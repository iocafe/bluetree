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
    m_requested_columns = OS_NULL;
    m_table_configuration = OS_NULL;

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
                if (m_bflags & EBIND_CLIENT) {
                    disconnect();
                }
                else {
                    delete this;
                }
                return;

            case ECMD_RSET_SELECT:
                if (m_state == E_BINDING_OK) {
                    srvselect(envelope);
                }
                return;

            case ECMD_FWRD:
                update(envelope);
                return;

            case ECMD_ACK:
                ack(envelope);
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


/* If remotepath is OS_NULL last used name will be preserved/
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
        parameters->seto(ERSET_BINDING_REQUESTED_COLUMNS, m_requested_columns, ESET_STORE_AS_VARIABLE);
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

    /* Free memory allocated, if any
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

m_table_configuration->print_json();

    /* Complete the server end of binding and return.
     */
    srvbind_base(envelope, reply);

    // print_json();

    return;

notarget:
    /* Send "no target" reply message to indicate that recipient was not found.
     */
    if ((envelope->mflags() & EMSG_NO_REPLIES) == 0)
    {
        message (ECMD_NO_TARGET, envelope->source(),
            envelope->target(), OS_NULL, EMSG_DEFAULT);
    }
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

    /* Send select command with parameter (memory allocated for parameters released by this call)
     */
    message(ECMD_RSET_SELECT, m_bindpath ? m_bindpath : m_objpath,
        OS_NULL, parameters, EMSG_DEL_CONTENT);
}


/**
****************************************************************************************************

  @brief Select data.

  The eRowSetBinding::srvselect() function...


****************************************************************************************************
*/
void eRowSetBinding::srvselect(
    eEnvelope *envelope)
{
    eDBM *dbm;
    eVariable v;
    eSet *parameters;
    // eStatus s;

    parameters = eSet::cast(envelope->content());
    if (parameters == OS_NULL)
    {
        osal_debug_error("srvselect() failed: no parameters");
        goto getout;
    }

    dbm = eDBM::cast(grandparent());
    if (dbm == OS_NULL) goto getout;
    if (dbm->classid() != ECLASSID_DBM) goto getout;

    m_pstruct.limit = parameters->geti(ERSET_BINDING_LIMIT);
    m_pstruct.page_mode = parameters->geti(ERSET_BINDING_PAGE_MODE);
    m_pstruct.row_mode = parameters->geti(ERSET_BINDING_ROW_MODE);
    m_pstruct.tzone = parameters->geto_ptr(ERSET_BINDING_TZONE);
    if (m_pstruct.tzone) {
        m_pstruct.tzone = m_pstruct.tzone->clone(this, EOID_ITEM);
    }

    delete m_where_clause;
    m_where_clause = new eVariable(this);
    parameters->getv(ERSET_BINDING_WHERE_CLAUSE, m_where_clause);

    /* Select rows from table.
     */
    /*s = */
    dbm->select(m_where_clause->gets(),
        m_requested_columns, // ** ??  eContainer *columns,
        &m_pstruct,
        0 /* tflags = 0 ???????????????? */);

getout:
    return;
}


/**
****************************************************************************************************

  @brief Complete property binding at client end.

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
    eSet *parameters;
    eVariable v;

    parameters = eSet::cast(envelope->content());
    if (parameters == OS_NULL)
    {
#if OSAL_DEBUG
        osal_debug_error("cbindok() failed: no content");
#endif
        goto notarget;
    }

    /* If this server side is master at initialization, get property value.
     */
//        parameters->getv(EPR_BINDING_VALUE, &v);
//        binding_setproperty(&v);

notarget:

    /* Call base class to complete the binding.
     */
    cbindok_base(envelope);
}


/**
****************************************************************************************************

  @brief Mark property value changed.

  The eRowSetBinding::changed function marks a property value changed, so that it needs
  to forwarded. The function forwards the property value immediately, if flow control allows.
  Otherwise the property just remain marked to be forwarded.
  If property number given as argument is not for this binding, do nothing.

  @param propertynr Property number of the changed property.
  @param x Optional property value, used to save requerying it in common case.

  @return None.

****************************************************************************************************
*/
void eRowSetBinding::changed(
    os_int propertynr,
    eVariable *x,
    os_boolean delete_x)
{
    /* If not for this property, do nothing.
     */
//    if (propertynr != m_localpropertynr) return;

    /* Mark property value, etc changed. Forward immediately, if binding if flow
       control does not block it.
     */
    setchanged();
    forward(x, delete_x);
}


/**
****************************************************************************************************

  @brief Forward property value trough binding.

  The forward function sends value of a property if flow control allows.

  @param  x Variable containing value, if available.
  @param  delete_x Flag weather value should be deleted.
  @return None.

****************************************************************************************************
*/
void eRowSetBinding::forward(
    eVariable *x,
    os_boolean delete_x)
{
    eVariable *tmp;

    if (forwardnow())
    {

        if (x == OS_NULL)
        {
            tmp = new eVariable;
  //          binding_getproperty(tmp);

            message(ECMD_FWRD, m_bindpath, OS_NULL, tmp,
                EMSG_DEL_CONTENT /* EMSG_NO_ERROR_MSGS */);
        }
        else
        {
            /* Send data as ECMD_FWRD message.
             */
            message(ECMD_FWRD, m_bindpath, OS_NULL, x,
                delete_x ? EMSG_DEL_CONTENT : EMSG_DEFAULT  /* EMSG_NO_ERROR_MSGS */);
            x = OS_NULL;
        }

        /* Clear changed bit and increment acknowledge count.
         */
        forwarddone();
    }

    if (delete_x && x)
    {
        delete x;
    }
}


/**
****************************************************************************************************

  @brief Property value has been received from binding.

  The eRowSetBinding::update function...
  @return None.

****************************************************************************************************
*/
void eRowSetBinding::update(
    eEnvelope *envelope)
{
    // eVariable *x;

    // x = eVariable::cast(envelope->content());
    // binding_setproperty(x);
    sendack(envelope);
}


/**
****************************************************************************************************

  @brief Send acknowledge.

  The sendack function.

  @param  envelope Message envelope from server binding.
  @return None.

****************************************************************************************************
*/
void eRowSetBinding::sendack(
    eEnvelope *envelope)
{
    sendack_base(envelope);

    /* If this is server, if m_ackcount is not zero, mark changed.
     */
    if ((m_bflags & EBIND_CLIENT) == 0 && m_ackcount)
    {
        setchanged();
    }
}


/**
****************************************************************************************************

  @brief Acknowledge received.

  The ack function decrements acknowledge wait count and tries to send again.

  @param  envelope Message envelope from server binding.
  @return None.

****************************************************************************************************
*/
void eRowSetBinding::ack(
    eEnvelope *envelope)
{
    ack_base(envelope);
}
