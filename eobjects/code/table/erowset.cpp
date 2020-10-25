/**

  @file    erowset.cpp
  @brief   RowSet class.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    15.10.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "eobjects.h"


/* RowSet property names.
 */
const os_char
    ersetp_ncolumns[] = "ncolumns",
    ersetp_dbm_path[] = "dbmpath",
    ersetp_table_name[] = "table",
    ersetp_where_clause[] = "where",
    ersetp_requested_columns[] = "columns",
    ersetp_limit[] = "limit",
    ersetp_page_mode[] = "pagemode",
    ersetp_row_mode[] = "rowmode",
    ersetp_tzone[] = "tzone",
    ersetp_has_callback[] = "callback",
    ersetp_configuration[] = "configuration";

/**
****************************************************************************************************
  Constructor.
****************************************************************************************************
*/
eRowSet::eRowSet(
    eObject *parent,
    e_oid id,
    os_int flags)
    : eTable(parent, id, flags)
{
    /** Clear member variable
     */
    m_ncolumns = 0;
    m_configuration = OS_NULL;
    m_own_change = 0;
    m_dbm_path = OS_NULL;
    m_rebind = OS_FALSE;
    os_memclear(&m_prm, sizeof(eSelectParameters));
}


/**
****************************************************************************************************
  Virtual destructor.
****************************************************************************************************
*/
eRowSet::~eRowSet()
{
}


/**
****************************************************************************************************

  @brief Add eRowSet to class list.

  The eRowSet::setupclass function adds newobj function pointer to class list. This enables creating
  new objects dynamically by class identifier, which is used for serialization reader()
  functions.

  This needs to be called after eBuffer:setupclass(), there is a dependency in setup.

****************************************************************************************************
*/
void eRowSet::setupclass()
{
    const os_int cls = ECLASSID_ROWSET;

    /* Add the class to class list.
     */
    os_lock();
    eclasslist_add(cls, (eNewObjFunc)newobj, "eRowSet");
    addpropertyl(cls, ERSETP_NCOLUMNS, ersetp_ncolumns, "nro columns", EPRO_SIMPLE);

    addpropertys(cls, ERSETP_DBM_PATH, ersetp_dbm_path, "DBM path", EPRO_PERSISTENT|EPRO_SIMPLE);
    addpropertys(cls, ERSETP_TABLE_NAME, ersetp_table_name, "table", EPRO_PERSISTENT|EPRO_SIMPLE);
    addpropertys(cls, ERSETP_WHERE_CLAUSE, ersetp_where_clause, "where",
        EPRO_PERSISTENT|EPRO_SIMPLE);
    addproperty (cls, ERSETP_REQUESTED_COLUMNS, ersetp_requested_columns, "requested",
        EPRO_PERSISTENT|EPRO_SIMPLE);
    addpropertyl(cls, ERSETP_LIMIT, ersetp_limit, "limit", EPRO_PERSISTENT|EPRO_SIMPLE);
    addpropertyl(cls, ERSETP_PAGE_MODE, ersetp_page_mode, "page", EPRO_PERSISTENT|EPRO_SIMPLE);
    addpropertyl(cls, ERSETP_ROW_MODE, ersetp_row_mode, "row", EPRO_PERSISTENT|EPRO_SIMPLE);
    addproperty (cls, ERSETP_TZONE, ersetp_tzone, "tzone", EPRO_PERSISTENT|EPRO_SIMPLE);
    addpropertyb(cls, ERSETP_HAS_CALLBACK, ersetp_has_callback, "callback",
        EPRO_PERSISTENT|EPRO_SIMPLE);

    addproperty (cls, ERSETP_CONFIGURATION, ersetp_configuration, "configuration",
        EPRO_PERSISTENT|EPRO_SIMPLE);
    propertysetdone(cls);
    os_unlock();
}


/**
****************************************************************************************************

  @brief Clone object

  The eRowSet::clone function clones an eRowSet.

  @param  parent Parent for the clone.
  @param  id Object identifier for the clone.
  @param  aflags 0 for default operation. EOBJ_NO_MAP not to map names.
  @return Pointer to the clone.

****************************************************************************************************
*/
eObject *eRowSet::clone(
    eObject *parent,
    e_oid id,
    os_int aflags)
{
    eRowSet *clonedobj;

    clonedobj = new eRowSet(parent, id == EOID_CHILD ? oid() : id, flags());

    if (m_dbm_path) {
        clonedobj->m_dbm_path = eVariable::cast(m_dbm_path->clone(this,
            EOID_ITEM, EOBJ_TEMPORARY_ATTACHMENT));
    }
    if (m_prm.table_name) {
        clonedobj->m_prm.table_name = eVariable::cast(m_prm.table_name->clone(this,
            EOID_ITEM, EOBJ_TEMPORARY_ATTACHMENT));
    }
    clonedobj->m_prm.limit = m_prm.limit;
    clonedobj->m_prm.page_mode = m_prm.page_mode;
    clonedobj->m_prm.row_mode = m_prm.row_mode;
    if (m_prm.tzone) {
        clonedobj->m_prm.tzone = m_prm.tzone->clone(this, EOID_ITEM, EOBJ_TEMPORARY_ATTACHMENT);
    }

    clonegeneric(clonedobj, aflags|EOBJ_CLONE_ALL_CHILDREN);
    return clonedobj;
}


/**
****************************************************************************************************

  @brief Called to inform the class about property value change (override).

  The onpropertychange() function is called when class'es property changes, unless the
  property is flagged with EPRO_NOONPRCH.
  If property is flagged as EPRO_SIMPLE, this function shuold save the property value
  in class members and and return it when simpleproperty() is called.

  Notice for change logging: Previous value is still valid when this function is called.
  You can get the old value by calling property() function inside onpropertychange()
  function.

  @param   propertynr Property number of changed property.
  @param   x Variable containing the new value.
  @param   flags
  @return  If successfull, the function returns ESTATUS_SUCCESS (0). Nonzero return values do
           indicate that there was no property with given property number.

****************************************************************************************************
*/
eStatus eRowSet::onpropertychange(
    os_int propertynr,
    eVariable *x,
    os_int flags)
{
    eObject *o;

    switch (propertynr)
    {
        case ERSETP_NCOLUMNS: /* read only */
            break;

        case ERSETP_DBM_PATH:
            if (m_dbm_path == OS_NULL) {
                m_dbm_path = new eVariable(this, EOID_ITEM, EOBJ_TEMPORARY_ATTACHMENT);
            }
            m_dbm_path->setv(x);
            m_rebind = OS_TRUE;
            break;

        case ERSETP_TABLE_NAME:
            if (m_prm.table_name == OS_NULL) {
                m_prm.table_name = new eVariable(this, EOID_ITEM, EOBJ_TEMPORARY_ATTACHMENT);
            }
            m_prm.table_name->setv(x);
            m_rebind = OS_TRUE;
            break;

        case ERSETP_LIMIT:
            m_prm.limit = x->geti();
            break;

        case ERSETP_PAGE_MODE:
            m_prm.page_mode = x->geti();
            break;

        case ERSETP_ROW_MODE:
            m_prm.row_mode = x->geti();
            break;

        case ERSETP_TZONE:
            delete m_prm.tzone;
            m_prm.tzone = OS_NULL;
            o = x->geto();
            if (o) {
                m_prm.tzone = o->clone(this, EOID_ITEM, EOBJ_TEMPORARY_ATTACHMENT);
            }
            break;

        case ERSETP_CONFIGURATION: /* read only */
            break;

        default:
            return eObject::onpropertychange(propertynr, x, flags);
    }

    return ESTATUS_SUCCESS;
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
eStatus eRowSet::simpleproperty(
    os_int propertynr,
    eVariable *x)
{
    eRowSetBinding *binding;

    switch (propertynr)
    {
        case ERSETP_NCOLUMNS:
            if (m_ncolumns == 0) goto clear_x;
            x->setl(m_ncolumns);
            break;

        case ERSETP_DBM_PATH:
            x->setv(m_dbm_path);
            break;

        case ERSETP_TABLE_NAME:
            x->setv(m_prm.table_name);
            break;

        case ERSETP_WHERE_CLAUSE:
            binding = get_binding();
            if (binding == OS_NULL) goto clear_x;
            binding->propertyv(ERSETP_WHERE_CLAUSE, x);
            break;

        case ERSETP_REQUESTED_COLUMNS:
            binding = get_binding();
            if (binding == OS_NULL) goto clear_x;
            binding->propertyv(ERSETP_REQUESTED_COLUMNS, x);
            break;

        case ERSETP_LIMIT:
            if (m_prm.limit == 0) goto clear_x;
            x->setl(m_prm.limit);
            break;

        case ERSETP_PAGE_MODE:
            if (m_prm.page_mode == 0) goto clear_x;
            x->setl(m_prm.page_mode);
            break;

        case ERSETP_ROW_MODE:
            if (m_prm.row_mode == 0) goto clear_x;
            x->setl(m_prm.row_mode);
            break;

        case ERSETP_TZONE:
            x->seto(m_prm.tzone);
            break;

        case ERSETP_HAS_CALLBACK:
            if (m_prm.callback == OS_NULL) goto clear_x;
            x->setl(OS_TRUE);
            break;

        case ERSETP_CONFIGURATION:
            x->seto(m_configuration);
            break;

        default:
            return eTable::simpleproperty(propertynr, x);
    }
    return ESTATUS_SUCCESS;

clear_x:
    x->clear();
    return ESTATUS_SUCCESS;
}


#if E_SUPPROT_JSON
/**
****************************************************************************************************

  @brief Write rowset specific content to stream as JSON.

  The eRowSet::json_writer() function writes class specific object content to stream as JSON.

  @param  stream The stream to write to.
  @param  sflags Serialization flags. Typically EOBJ_SERIALIZE_DEFAULT.
  @param  indent Indentation depth, 0, 1... Writes 2x this spaces at beginning of a line.

  @return If successfull the function returns ESTATUS_SUCCESS (0). If writing object to stream
          fails, value ESTATUS_WRITING_OBJ_FAILED is returned. Assume that all nonzero values
          indicate an error.

****************************************************************************************************
*/
eStatus eRowSet::json_writer(
    eStream *stream,
    os_int sflags,
    os_int indent)
{
    os_boolean comma1, comma2;
    eVariable tmp;
    eMatrix *row;
    eObject *o;
    os_int column, type_id;
    os_boolean has_value;

    indent++;
    if (json_puts(stream, "[")) goto failed;
    comma1 = OS_FALSE;

    for (row = eMatrix::cast(first()); row; row = eMatrix::cast(row->next()))
    {
        if (row->classid() != ECLASSID_MATRIX) {
            osal_debug_error("Corrupted row set");
            goto failed;
        }

        if (comma1) {
            if (json_puts(stream, ",")) goto failed;
        }
        comma1 = OS_TRUE;

        if (json_indent(stream, indent, EJSON_NEW_LINE_BEFORE /* , &comma */)) goto failed;
        if (json_puts(stream, "[")) goto failed;
        comma2 = OS_FALSE;

        for (column = 0; column < m_ncolumns; column++)
        {
            if (comma2) {
                if (json_puts(stream, ",")) goto failed;
            }
            comma2 = OS_TRUE;

            has_value = row->getv(0, column, &tmp);
            if (has_value)
            {
                type_id = tmp.type();
                if (OSAL_IS_BOOLEAN_TYPE(type_id) ||
                    OSAL_IS_INTEGER_TYPE(type_id) ||
                    OSAL_IS_FLOAT_TYPE(type_id))
                {
                    if (json_puts(stream, tmp.gets())) goto failed;
                }
                else if (type_id == OS_OBJECT)
                {
                    o = tmp.geto();
                    if (o) {
                        if (o->json_write(stream, sflags, indent)) goto failed;
                    }
                    else {
                        if (json_putqs(stream, "")) goto failed;
                    }
                }
                else
                {
                    if (json_putqs(stream, tmp.gets())) goto failed;
                }
            }
            else {
                if (json_putqs(stream, "")) goto failed;
            }
        }

        if (json_puts(stream, "]")) goto failed;
    }

    if (json_indent(stream, --indent, EJSON_NEW_LINE_BEFORE /* , &comma */)) goto failed;
    if (json_puts(stream, "]")) goto failed;

    return ESTATUS_SUCCESS;

failed:
    return ESTATUS_FAILED;
}
#endif


/* Insert rows into table.
 * Row can be one row or container with multiple rows.
 */
void eRowSet::insert(
    eContainer *rows,
    os_int tflags)
{
    const os_char *table_name = OS_NULL;
    if (m_prm.table_name) {
        table_name = m_prm.table_name->gets();
    }
    etable_insert(this, m_dbm_path->gets(), table_name, rows, tflags);
}

/* Update a row or rows of a table.
 */
eStatus eRowSet::update(
    const os_char *where_clause,
    eContainer *row,
    os_int tflags)
{
    const os_char *table_name = OS_NULL;
    if (m_prm.table_name) {
        table_name = m_prm.table_name->gets();
    }
    etable_update(this, m_dbm_path->gets(), table_name, where_clause, row, tflags);
    return ESTATUS_SUCCESS;
}

/* Remove rows from table.
 */
void eRowSet::remove(
    const os_char *where_clause,
    os_int tflags)
{
    const os_char *table_name = OS_NULL;
    if (m_prm.table_name) {
        table_name = m_prm.table_name->gets();
    }
    etable_remove(this, m_dbm_path->gets(), table_name, where_clause, tflags);
}

/* Select rows from table.
 */
eStatus eRowSet::select(
    const os_char *where_clause,
    eContainer *columns,
    eSelectParameters *prm,
    os_int tflags)

{
    return ESTATUS_FAILED;
}


/**
****************************************************************************************************

  @brief Bind this row set to a remote table.

  The select() function creates binding to remote table and selects initial data.
  - Binding is recreated only if dbm path or table name have changed.

  @param  where_clause Where clause to select rows. If OS_NULL, no data is selected.
  @param  columns eContainer holding eVariable for each column to select.
  @param  bflags Combination of EBIND_DEFAULT (0), EBIND_NOFLOWCLT
          - EBIND_DEFAULT:  bind without special options.
          - EBIND_NOFLOWCLT: Disable flow control. Normally if property value changes
            faster than it can be transferred, some values are skipped. If EBIND_NOFLOWCLT
            flag is given, it disables flow control and every value is transferred without
            any limit to buffered memory use.
  @param  limit
  @param  bflags

****************************************************************************************************
*/
void eRowSet::select(
    const os_char *where_clause,
    eContainer *columns,
    os_int limit,
    os_int bflags)
{
    eContainer *bindings;
    eRowSetBinding *binding = OS_NULL;
    // eObject *o;

    if (m_dbm_path == OS_NULL) {
        osal_debug_error("eRowSet::select:DBM path not set");
        return;
    }

    if (m_callback == OS_NULL) {
        osal_debug_error("eRowSet::select:Row set callback function not set");
    }

    /* Limit is passed trough parameter structure, set as property.
     */
    setpropertyl(ERSETP_LIMIT, limit);

    /* Get or create bindings container.
     */
    bindings = firstc(EOID_BINDINGS);
    if (bindings == OS_NULL) {
        bindings = new eContainer(this, EOID_BINDINGS, EOBJ_IS_ATTACHMENT);
    }
    else {
        binding = eRowSetBinding::cast(bindings->first(EOID_TABLE_CLIENT_BINDING));
        if (m_rebind && binding)
        {
            delete binding;
            binding = OS_NULL;

        }
    }
    m_rebind = OS_FALSE;

    if (binding == OS_NULL)
    {
        /* Create binding
         */
        binding = new eRowSetBinding(bindings, EOID_TABLE_CLIENT_BINDING,
             EOBJ_NOT_CLONABLE | EOBJ_NOT_SERIALIZABLE);

        /* Bind the row set to table. This function will send message to remote object to bind.
         */
        binding->bind(m_dbm_path, columns, &m_prm, bflags);
    }

    /* If where clause is given as argument, initial data is wanted. Get it.
     */
    if (where_clause) {
        binding->select(where_clause, m_prm.limit,
            m_prm.page_mode, m_prm.row_mode, m_prm.tzone);
    }

    //binding->print_json();
}


/* Table structure has been received, store it and do callback.
 */
void eRowSet::client_binding_complete(
    eContainer *cont)
{
    eContainer *configuration, *columns;
    ersetCallbackInfo info;

    delete m_configuration;
    m_configuration = OS_NULL;
    m_ncolumns = 0;

    if (cont) {
        configuration = cont->firstc(EOID_TABLE_CONFIGURATION);
        if (configuration) {
            m_configuration = eContainer::cast(configuration->clone(this,
                EOID_TABLE_CONFIGURATION, EOBJ_TEMPORARY_ATTACHMENT));

            columns = m_configuration->firstc(EOID_TABLE_COLUMNS);
            if (columns) {
                m_ncolumns = columns->childcount();
            }
        }
    }

    /* Callback to inform application that binding is complete.
     */
    if (m_callback) {
        os_memclear(&info, sizeof(info));
        info.reason = ERSET_TABLE_BINDING_COMPLETE;
        m_callback(this, &info, m_context);
    }
}


/* All initial data has been received, move it from temporary sync storage to this row set
 * and do call back.
 */
void eRowSet::initial_data_complete(
    eContainer *sync_storage)
{
    eObject *o, *next_o;
    ersetCallbackInfo info;

    /* Delete old data
     */
    for (o = first(); o; o = next_o) {
        next_o = o->next();
        if (o->classid() == ECLASSID_MATRIX) {
            delete o;
        }
    }

    /* Adopt new data.
     */
    if (sync_storage) {
        for (o = sync_storage->first(); o; o = next_o) {
            next_o = o->next();
            if (o->classid() == ECLASSID_MATRIX) {
                o->adopt(this, EOID_ITEM);
            }
        }
    }

    /* Callback to inform application about initial data.
     */
    if (m_callback) {
        os_memclear(&info, sizeof(info));
        info.reason = ERSET_INITIAL_DATA_RECEIVED;
        m_callback(this, &info, m_context);
    }

print_json();
}


/* Gets pointer to the table binding or OS_NULL if none.
 */
eRowSetBinding *eRowSet::get_binding()
{
    eContainer *bindings;

    bindings = firstc(EOID_BINDINGS);
    if (bindings) {
        return eRowSetBinding::cast(bindings->first(EOID_TABLE_CLIENT_BINDING));
    }
    return OS_NULL;
}


