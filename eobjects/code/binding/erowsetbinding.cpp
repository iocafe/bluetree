/**

  @file    erowsetbinding.cpp
  @brief   Binding row set to table.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "eobjects.h"


/**
****************************************************************************************************

  @brief Property binding class constructor.

  Clear member variables.
  @return  None.

****************************************************************************************************
*/
eRowSetBinding::eRowSetBinding(
    eObject *parent,
    e_oid id,
    os_int flags)
    : eBinding(parent, id, flags)
{
    /* Clear member variables.
     */
    // m_pset = OS_NULL;
    os_memclear(&m_pstruct, sizeof(eSelectParameters));
    m_where_clause = OS_NULL;
    m_where = OS_NULL;
    m_minix = OS_LONG_MIN;
    m_maxix = OS_LONG_MAX;
    m_requested_columns = OS_NULL;
    m_table_configuration = OS_NULL;
    m_sync_transfer = OS_NULL;
    m_sync_transfer_mtx_nr = 0;
    m_sync_storage = OS_NULL;
    m_trigged_changes = OS_NULL;

    /* Row set bindings cannot be cloned or serialized.
     */
    setflags(EOBJ_NOT_CLONABLE|EOBJ_NOT_SERIALIZABLE);
}


/**
****************************************************************************************************

  @brief Virtual destructor.

  If connected, disconnect binding. Release all resources allocated for the binging.
  @return  None.

****************************************************************************************************
*/
eRowSetBinding::~eRowSetBinding()
{
}


/**
****************************************************************************************************

  @brief Add the class to class list and class'es properties to it's property set.

  The eVariable::setupclass function adds the class to class list and class'es properties to
  it's property set. The class list enables creating new objects dynamically by class identifier,
  which is used for serialization reader functions. The property set stores static list of
  class'es properties and metadata for those.

****************************************************************************************************
*/
void eRowSetBinding::setupclass()
{
    const os_int cls = ECLASSID_ROW_SET_BINDING;

    /* Add the class to class list.
     */
    os_lock();
    eclasslist_add(cls, (eNewObjFunc)newobj, "eRowSetBinding");
    addpropertys(cls, ERSETP_DBM_PATH, ersetp_dbm_path, "DBM path", EPRO_PERSISTENT|EPRO_SIMPLE);
    addpropertys(cls, ERSETP_TABLE_NAME, ersetp_table_name, "table", EPRO_PERSISTENT|EPRO_SIMPLE);
    addpropertys(cls, ERSETP_WHERE_CLAUSE, ersetp_where_clause, "where", EPRO_PERSISTENT|EPRO_SIMPLE);
    addproperty (cls, ERSETP_REQUESTED_COLUMNS, ersetp_requested_columns, "requested", EPRO_PERSISTENT|EPRO_SIMPLE);
    addpropertyl(cls, ERSETP_LIMIT, ersetp_limit, "limit", EPRO_PERSISTENT|EPRO_SIMPLE);
    addpropertyl(cls, ERSETP_PAGE_MODE, ersetp_page_mode, "page", EPRO_PERSISTENT|EPRO_SIMPLE);
    addpropertyl(cls, ERSETP_ROW_MODE, ersetp_row_mode, "row", EPRO_PERSISTENT|EPRO_SIMPLE);
    addproperty (cls, ERSETP_TZONE, ersetp_tzone, "tzone", EPRO_PERSISTENT|EPRO_SIMPLE);
    addpropertys(cls, EBINDP_BFLAGS, ebindp_bflags, "b_flags", EPRO_PERSISTENT|EPRO_SIMPLE);
    addpropertys(cls, EBINDP_STATE, ebindp_state, "state", EPRO_PERSISTENT|EPRO_SIMPLE);
    addproperty (cls, ERSETP_CONFIGURATION, ersetp_configuration, "configuration", EPRO_PERSISTENT|EPRO_SIMPLE);
    propertysetdone(cls);
    os_unlock();
}


/**
****************************************************************************************************

  @brief Function to process incoming messages.

  The eRowSetBinding::onmessage function handles messages received by object. If this function
  doesn't process message, it calls parent class'es onmessage function.

  @param   envelope Message envelope. Contains command, target and source paths and
           message content, etc.
  @return  None.

****************************************************************************************************
*/
void eRowSetBinding::onmessage(
    eEnvelope *envelope)
{
    eDBM *dbm;

    /* If at final destination for the message.
     */
    if (*envelope->target()=='\0')
    {
        switch (envelope->command())
        {
            case ECMD_BIND_REPLY:
                cbindok(this, envelope);
                return;

            case ECMD_UNBIND:
            case ECMD_SRV_UNBIND:
            case ECMD_NO_TARGET:
                /* Client side: Disconnect this eRowSetBinding but do not delete it from memory
                 * so it can be reconnected.
                 */
                if (m_bflags & EBIND_CLIENT) {
                    disconnect();
                }

                /* Server side: Delete this eRowSetBinding and refresh trigger data.
                 */
               else {
                    dbm = srv_dbm();
                    delete this;
                    dbm->generate_trigger_data();
                }
                return;

            case ECMD_RSET_SELECT:
                if (m_state == E_BINDING_OK) {
                    srvselect(envelope);
                }
                return;

            case ECMD_TABLE_DATA_TRANSFER:
                table_data_received(envelope);
                return;

            case ECMD_OK:
                initial_data_complete();
                return;

            case ECMD_REBIND:
                bind2(OS_NULL);
                return;
        }
    }

    /* Call parent class'es onmessage.
     */
    eBinding::onmessage(envelope);
}


/**
****************************************************************************************************

  @brief Get value of simple property (override).

  The simpleproperty() function stores current value of simple property into variable x.

  @param   propertynr Property number to get.
  @param   x Variable into which to store the property value.
  @return  If property with property number was stored in x, the function returns
           ESTATUS_SUCCESS (0). Nonzero return values indicate that property with
           given number was not among simple properties.

****************************************************************************************************
*/
eStatus eRowSetBinding::simpleproperty(
    os_int propertynr,
    eVariable *x)
{
    switch (propertynr)
    {
        case EBINDP_BFLAGS:
            x->sets((m_bflags & EBIND_BIND_ROWSET) ? "rowset" : "property");
            x->appends((m_bflags & EBIND_CLIENT) ? ",client" : ",server");
            if (m_bflags & EBIND_INTERTHREAD) x->appends(",interthread");
            x->appends((m_bflags & EBIND_TEMPORARY) ? ",temporary" : ",pesistent");
            break;

        case EBINDP_STATE:
            switch (m_state)
            {
                case E_BINDING_UNUSED: x->sets("disconnected"); break;
                case E_BINDING_NOW: x->sets("connecting"); break;
                case E_BINDING_OK: x->sets("connected"); break;
                default: x->sets("?"); break;
            }
            break;

        case ERSETP_DBM_PATH:
            if (m_objpath == OS_NULL) goto clear_x;
            x->sets(m_objpath);
            break;

        case ERSETP_TABLE_NAME:
            x->setv(m_pstruct.table_name);
            break;

        case ERSETP_WHERE_CLAUSE:
            x->setv(m_where_clause);
            break;

        case ERSETP_REQUESTED_COLUMNS:
            x->seto(m_requested_columns);
            break;

        case ERSETP_LIMIT:
            if (m_pstruct.limit == 0) goto clear_x;
            x->setl(m_pstruct.limit);
            break;

        case ERSETP_PAGE_MODE:
            if (m_pstruct.page_mode == 0) goto clear_x;
            x->setl(m_pstruct.page_mode);
            break;

        case ERSETP_ROW_MODE:
            if (m_pstruct.row_mode == 0) goto clear_x;
            x->setl(m_pstruct.row_mode);
            break;

        case ERSETP_TZONE:
            x->seto(m_pstruct.tzone);
            break;

        default:
            return eBinding::simpleproperty(propertynr, x);
    }
    return ESTATUS_SUCCESS;

clear_x:
    x->clear();
    return ESTATUS_SUCCESS;
}


/**
****************************************************************************************************

  @brief Bind this object's property to remote property.

  The eObject::bind() function creates binding to remote property. When two variables are bound
  together, they have the same value. When the other changes, so does the another. Bindings
  work over messaging, thus binding work as well between objects in same thread or objects in
  different computers.

  @param  localpropertyno This object's propery number to bind.
  @param  remotepath Path to remote object to bind to.
  @param  remoteproperty Name of remote property to bind. If OS_NULL variable value
          is assumed.
  @param  bflags Combination of EBIND_DEFAULT (0), EBIND_NOFLOWCLT
          - EBIND_DEFAULT:  bind without special options.
          - EBIND_NOFLOWCLT: Disable flow control.
  @return None.

****************************************************************************************************
*/
void eRowSetBinding::bind(
    eVariable *dbm_path,
    eContainer *columns,
    eSelectParameters *prm,
    os_int bflags)
{
    /* Free memory allocated, if any
     */
    delete m_pstruct.table_name;
    delete m_pstruct.tzone;

    /* Save bind parameters.
     */
    os_memcpy(&m_pstruct, prm, sizeof(eSelectParameters));
    if (m_pstruct.table_name) {
        m_pstruct.table_name = eVariable::cast(m_pstruct.table_name->clone(this));
    }
    if (m_pstruct.tzone) {
        m_pstruct.tzone = m_pstruct.tzone->clone(this);
    }

    /* Store columns
     */
    delete m_requested_columns;
    m_requested_columns = OS_NULL;
    if (columns) {
        m_requested_columns = eContainer::cast(columns->clone(this));
    }

    /* Save flags and mark as client end.
     */
    m_bflags = bflags | EBIND_CLIENT;

    /* Initiate binding.
     */
    bind2(dbm_path->gets());
}


/* Send first message to initiate row set binding (client)
 * If remotepath is OS_NULL last used name will be preserved
 */
void eRowSetBinding::bind2(
    const os_char *remotepath)
{
    eSet *parameters;

    /* Generate eSet to hold select parameters.
     */
    parameters = new eSet(this);
    parameters->setl(ERSET_BINDING_FLAGS, (m_bflags & EBIND_SER_MASK) | EBIND_BIND_ROWSET);
    if (m_pstruct.table_name) {
        parameters->setv(ERSET_BINDING_TABLE_NAME, m_pstruct.table_name);
    }
    if (m_requested_columns) {
        parameters->seto(ERSET_BINDING_REQUESTED_COLUMNS,
            m_requested_columns, ESET_STORE_AS_VARIABLE);
    }

    /* Call base class to do binding.
     */
    eBinding::bind_base(remotepath, parameters, OS_TRUE);
}


/**
****************************************************************************************************

  @brief Create server end property binding.

  The eRowSetBinding::srvbind() function...

  @param  envelope Receecived ECMD_BIND_RS command envelope.
  @return None.

****************************************************************************************************
*/
void eRowSetBinding::srvbind(
    eObject *obj,
    eEnvelope *envelope)
{
    eSet *parameters;
    eDBM *dbm;
    eContainer *reply;

    parameters = eSet::cast(envelope->content());
    if (parameters == OS_NULL)
    {
        osal_debug_error("srvbind() failed: no parameters");
        goto notarget;
    }

    /* Free memory allocated for table name and time zone, if any.
     */
    delete m_pstruct.table_name;
    delete m_pstruct.tzone;

    /* Store parameters.
     */
    os_memclear(&m_pstruct, sizeof(eSelectParameters));
    m_bflags = parameters->geti(ERSET_BINDING_FLAGS);
    m_pstruct.table_name = new eVariable(this);
    parameters->getv(ERSET_BINDING_TABLE_NAME, m_pstruct.table_name);

    /* Store columns
     */
    delete m_requested_columns;
    m_requested_columns = eContainer::cast(parameters->geto_ptr(ERSET_BINDING_REQUESTED_COLUMNS));
    if (m_requested_columns) {
        m_requested_columns = eContainer::cast(m_requested_columns->clone(this, EOID_ITEM));
    }

    /* Set EBIND_TEMPORARY, and EBIND_INTERTHREAD if envelope has been moved
       from thread to another.
     */
    m_bflags |= EBIND_TEMPORARY;
    if (envelope->mflags() & EMSG_INTERTHREAD)
    {
        m_bflags |= EBIND_INTERTHREAD;
    }

    /* If subproperties are requested, list ones matching in both ends.
       Store initial property value, unless clientmaster.
     */
    reply = new eContainer(this);

    dbm = eDBM::cast(obj);

    /* Solve wild cards in requested columns -> resolved_columns
     */
    if (m_table_configuration == OS_NULL) {
        m_table_configuration = new eContainer(this, EOID_TABLE_CONFIGURATION);
        m_table_configuration->ns_create();
    }
    dbm->solve_table_configuration(m_table_configuration, m_requested_columns, m_pstruct.table_name);

    /* Send table congiguration */
    m_table_configuration->clone(reply);
    // m_table_configuration->print_json();

    /* Complete the server end of binding and return.
     */
    srvbind_base(envelope, reply);

    return;

notarget:
    /* Send "no target" reply message to indicate that recipient was not found.
     */
    notarget(envelope);
}


/**
****************************************************************************************************

  @brief Select data from table (client).

  The select function request selected data from a table and initiatializes a selection.
  The row set (grandparent of this binding) will receive updates for.

  @param  where_clause Which rows to select.
  @param  remotepath Path to remote object to bind to.
  @param  prm Parameter structure, limit, page_mode, row_mode;
.
  @return None.

****************************************************************************************************
*/
void eRowSetBinding::select(
    const os_char *where_clause,
    os_int limit,
    os_int page_mode,
    os_int row_mode,
    eObject *tzone)
{
    eSet *parameters;

    /* Save select parameters.
     */
    if (m_pstruct.limit != limit)
    {
        m_pstruct.limit = limit;
    }
    if (page_mode >= 0 && page_mode != m_pstruct.page_mode) {
        m_pstruct.page_mode = page_mode;
    }
    if (row_mode >= 0 && row_mode != m_pstruct.row_mode) {
        m_pstruct.row_mode = row_mode;
    }
    if (tzone) {
        delete m_pstruct.tzone;
        m_pstruct.tzone = tzone->clone(this);
    }

    /* Generate eSet to hold select parameters.
     */
    parameters = new eSet(this);
    if (m_pstruct.limit) {
        parameters->setl(ERSET_BINDING_LIMIT, m_pstruct.limit);
    }
    if (m_pstruct.page_mode) {
        parameters->setl(ERSET_BINDING_PAGE_MODE, m_pstruct.page_mode);
    }
    if (m_pstruct.row_mode) {
        parameters->setl(ERSET_BINDING_ROW_MODE, m_pstruct.row_mode);
    }
    if (m_pstruct.tzone) {
        parameters->seto(ERSET_BINDING_TZONE, m_pstruct.tzone);
    }
    if (where_clause) {
        parameters->sets(ERSET_BINDING_WHERE_CLAUSE, where_clause);
    }
    if (m_pstruct.table_name) {
        parameters->setv(ERSET_BINDING_TABLE_NAME, m_pstruct.table_name);
    }

    /* If we have storage for synchronized data transfer, empty it */
    if (m_sync_storage) {
        m_sync_storage->clear();
    }

    /* Send select command with parameter (memory allocated for parameters released by this call)
     */
    message(ECMD_RSET_SELECT, m_bindpath ? m_bindpath : m_objpath,
        OS_NULL, parameters, EMSG_DEL_CONTENT);
}


/**
****************************************************************************************************

  @brief Select data from underlying table (server).

  The eRowSetBinding::srvselect() function...


****************************************************************************************************
*/
void eRowSetBinding::srvselect(
    eEnvelope *envelope)
{
    eDBM *dbm;
    eVariable v;
    eSet *parameters;
    os_char *where_clause;
    eContainer *columns;
    os_memsz count;
    eStatus s;

    parameters = eSet::cast(envelope->content());
    if (parameters == OS_NULL)
    {
        osal_debug_error("srvselect() failed: no parameters");
        goto getout;
    }

    dbm = srv_dbm();
    if (dbm == OS_NULL) goto getout;
    if (m_table_configuration == OS_NULL) goto getout;
    columns = m_table_configuration->firstc(EOID_TABLE_COLUMNS);
    if (columns == OS_NULL) goto getout;

    m_pstruct.limit = parameters->geti(ERSET_BINDING_LIMIT);
    m_pstruct.page_mode = parameters->geti(ERSET_BINDING_PAGE_MODE);
    m_pstruct.row_mode = parameters->geti(ERSET_BINDING_ROW_MODE);
    m_pstruct.tzone = parameters->geto_ptr(ERSET_BINDING_TZONE);
    if (m_pstruct.tzone) {
        m_pstruct.tzone = m_pstruct.tzone->clone(this, EOID_ITEM);
    }

    if (m_where_clause == OS_NULL) {
        m_where_clause = new eVariable(this);
    }
    parameters->getv(ERSET_BINDING_WHERE_CLAUSE, m_where_clause);

    /* Asterix '*' as where clause is all rows, same as empty where clause.
     */
    where_clause = m_where_clause->gets();
    if (!os_strcmp(where_clause, "*") || *where_clause == '\0') {
        where_clause = OS_NULL;
    }

    /* Get index range from beginning of where clause.
     */
    count = e_parse_index_range(where_clause, &m_minix, &m_minix);
    if (count <= 0) {
        m_minix = OS_LONG_MIN;
        m_maxix = OS_LONG_MAX;
    }
    else {
        where_clause += count;
    }

    if (where_clause) {
        if (m_where == OS_NULL) {
            m_where = new eWhere(this);
        }

        s = m_where->compile(where_clause);
        if (s) {
            osal_debug_error_str("Where clause syntax error: ", where_clause);
            delete m_where;
            m_where = OS_NULL;
        }
    }
    else {
        delete m_where;
        m_where = OS_NULL;
    }

    /* Refresh eDBM trigger data with updated where clause, etc.
     */
    dbm->generate_trigger_data();

    /* Setup synchronized transfer.
     */
    m_sync_transfer = new eSynchronized(this);
    m_sync_transfer->initialize_synch_transfer(m_bindpath);
    m_sync_transfer_mtx_nr = 0;

    /* Set callback function to process select results.
     */
    m_pstruct.callback = srvselect_callback;
    m_pstruct.context = this;

    /* Select rows from table.
     */
    dbm->select(m_where_clause->gets(),
        columns, &m_pstruct, 0 /* tflags = 0 ???????????????? */);

    /* Send ECMD_OK as reply to indicate that the selection has completed.
     */
    message(ECMD_OK, envelope->source(),
        envelope->target(), OS_NULL, EMSG_DEFAULT, envelope->context());

    /* Wait for rest of messages (to avoid notarget warnings on acknowledges), and cleanup.
     */
    m_sync_transfer->sync_wait(0, m_timeout_ms);
    delete (m_sync_transfer);
    m_sync_transfer = OS_NULL;

getout:
    return;
}


/**
****************************************************************************************************

  @brief Callback to process srvselect() results (server).

  The eRowSetBinding::srvselect() function...

  @param data eMatrix holding one or more rows of matrix data. Deleted by this function.

  @return  The function returns ESTATUS_SUCCESS is all is fine. Other return values indicate
           an error and transfer is interrupted.

****************************************************************************************************
*/
eStatus eRowSetBinding::srvselect_callback(
    eTable *t,
    eMatrix *data,
    eObject *context)
{
    eEnvelope *envelope;
    eSynchronized *sync;
    eContainer *cont;
    eStatus s;

    eRowSetBinding *b;
    b = (eRowSetBinding*)context;
    sync = b->m_sync_transfer;

    /* Wait until we have received acknolegement for all but three packages sent.
     * If timeout or other error, interrupt the transfer.
     */
    s = sync->sync_wait(3, m_timeout_ms);
    if (s != ESTATUS_SUCCESS) {
        return s;
    }

    /* Generate envelope for this to send.
     */
    envelope = new eEnvelope(b);
    envelope->setcommand(ECMD_TABLE_DATA_TRANSFER);
    cont = new eContainer(b);
    data->adopt(cont, b->m_sync_transfer_mtx_nr++);
    envelope->setcontent(cont, EMSG_DEL_CONTENT);

    /* The envelope object given as argument is adopted/deleted by the synch_send function.
     */
    s = sync->synch_send(envelope);

    return s;
}


/**
****************************************************************************************************

  @brief Complete property binding at client end (client).

  The eRowSetBinding::cbindok() function...

  @param  obj Pointer to object being bound.
  @param  envelope The enveloped returned from server end as ECMD_BIND_REPLY.
  @return None.

****************************************************************************************************
*/
void eRowSetBinding::cbindok(
    eObject *obj,
    eEnvelope *envelope)
{
    eContainer *reply;
    eRowSet *rset;

    reply = eContainer::cast(envelope->content());
    osal_debug_assert(reply != OS_NULL);

    rset = client_rowset();
    if (rset && reply) {
        rset->client_binding_complete(reply);
    }

    /* Call base class to complete the binding.
     */
    cbindok_base(envelope);
}


/**
****************************************************************************************************

  @brief Some or all selected data data received, save it (client).

  The table_data_received function...

****************************************************************************************************
*/
void eRowSetBinding::table_data_received(
    eEnvelope *envelope)
{
    eObject *o;
    eVariable *tmp;
    eContainer *cont;
    eMatrix *mtx, *m;
    os_int nrows, ncols, r, c;
    osalTypeId dtype;

    if (m_sync_storage == OS_NULL) {
        m_sync_storage = new eContainer(this);
    }

    cont = eContainer::cast(envelope->content());
    if (cont == OS_NULL) goto getout;

    tmp = new eVariable(this);
    for (o = cont->first(); o; o = o->next()) {
        if (o->classid() != ECLASSID_MATRIX) {
            continue;
        }
        mtx = eMatrix::cast(o);

        if (o->oid() == 0) {
            m_sync_storage->clear();
        }

        nrows = mtx->nrows();
        ncols = mtx->ncolumns();
        dtype = mtx->datatype();

        for (r = 0; r < nrows; r++) {
            m = new eMatrix(m_sync_storage);
            m->allocate(dtype, 1, ncols);
            for (c = 0; c < ncols; c++) {
                mtx->getv(r, c, tmp);
                m->setv(0, c, tmp);
            }
        }
    }
    delete tmp;

getout:
    message (ECMD_ACK, envelope->source(), envelope->target(),
        OS_NULL, EMSG_KEEP_CONTEXT, envelope->context());
}


/**
****************************************************************************************************

  @brief Inform row set that all initial data has been received (client).

  The initial_data_complete function...

****************************************************************************************************
*/
void eRowSetBinding::initial_data_complete()
{
    eRowSet *rset;

    rset = client_rowset();
    if (rset) {
        rset->initial_data_complete(m_sync_storage);
    }
}


/* Get pointer to eDBM object (server)
 */
eDBM *eRowSetBinding::srv_dbm()
{
    eDBM *dbm;
    os_int cid;

    dbm = (eDBM*)grandparent();
    if (dbm) {
        cid = dbm->classid();
        if (cid == ECLASSID_DBM || cid == ECLASSID_MATRIX) {
            return dbm;
        }
    }

    osal_debug_error("srv_dbm: Grandparent is not eDBM");
    return OS_NULL;
}


/* Get pointer to eRowSet object (client)
 */
eRowSet *eRowSetBinding::client_rowset()
{
    eRowSet *rowset;
    os_int cid;

    rowset = (eRowSet*)grandparent();
    if (rowset) {
        cid = rowset->classid();
        if (cid == ECLASSID_ROWSET) {
            return rowset;
        }
    }

    osal_debug_error("client_rowset: Grandparent is not eRowSet");
    return OS_NULL;
}

/* Append "remove row" to trig data to send to row set.
 */
void eRowSetBinding::trigdata_append_remove(
    os_long ix_value)
{
    if (m_trigged_changes == OS_NULL)
    {
        m_trigged_changes = new eContainer(this);
    }
}

/* Append "insert or update row" to trig data to send to row set.
 */
void eRowSetBinding::trigdata_append_insert_or_update(
    os_long ix_value,
    eContainer *trigger_columns,
    eDBM *dbm)
{
    eContainer *vars, *list;
    eVariable *v, *tc;
    eName *n;
    eMatrix *m;
    os_int col_nr;

    if (m_where) {
        vars = m_where->variables();
        if (vars && trigger_columns)
        {
            for (v = vars->firstv(); v; v = v->nextv()) {
                n = v->primaryname();
                if (n == OS_NULL) continue;
                tc = eVariable::cast(trigger_columns->byname(n->gets()));
                if (tc) {
                    v->setv(tc);
                }
                else {
                    v->clear();
                }
            }

            if (m_where->evaluate()) {
                return;
            }
        }
    }

    if (m_trigged_changes == OS_NULL)
    {
        m_trigged_changes = new eContainer(this);
    }

    m = new eMatrix(m_trigged_changes);
    list = columns();
    m->allocate(OS_OBJECT, 1, list->childcount()); // ??????????????????? CHECK CAN DATA TYPE BE OPTIMIZED
    if (list) {
        for (v = list->firstv(), col_nr = 0; v; v = v->nextv(), col_nr++) {
            n = v->primaryname();
            if (n == OS_NULL) continue;

            tc = eVariable::cast(trigger_columns->byname(n->gets()));
            if (tc) {
                m->setv(0, col_nr, tc);
            }
        }
    }
}


/* Send and clear trig data.
 */
void eRowSetBinding::trigdata_send()
{

}


/**
****************************************************************************************************

  @brief Get the next row set binding identified by oid.

  The eRowSetBinding::nextrb() function returns pointer to the next row set binding.

  @param   id EOID_TABLE_CLIENT_BINDING to loop trough client bindinds or EOID_TABLE_SERVER_BINDING
           to loop trough server bindings.

  @return  Pointer to the eRowSetBinding, or OS_NULL if none found.

****************************************************************************************************
*/
eRowSetBinding *eRowSetBinding::nextrb(
    e_oid id)
{
    if (mm_handle == OS_NULL) return OS_NULL;
    eHandle *h = mm_handle->next(id);
    while (h)
    {
        if (h->object()->classid() == ECLASSID_ROW_SET_BINDING)
            return eRowSetBinding::cast(h->object());

        h = h->next(id);
    }
    return OS_NULL;
}
