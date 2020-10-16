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
    ersetp_nrows[] = "nrows",
    ersetp_ncolumns[] = "ncolumns";


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
    /** Initial size is 0, 0
     */
    m_nrows = m_ncolumns = 0;

    m_columns = OS_NULL;
    m_own_change = 0;
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
    addpropertyl(cls, ERSETP_NROWS, ersetp_nrows, "nro rows", EPRO_PERSISTENT|EPRO_SIMPLE);
    addpropertyl(cls, ERSETP_NCOLUMNS, ersetp_ncolumns, "nro columns", EPRO_PERSISTENT|EPRO_SIMPLE);
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
    clonedobj->m_ncolumns = m_ncolumns;
    clonedobj->m_nrows = m_nrows;

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
    eObject *configuration;

    switch (propertynr)
    {
        case ERSETP_NROWS:
            if (m_own_change == 0) {
                m_nrows = x->getl();
            }
            break;

        case ERSETP_NCOLUMNS:
            if (m_own_change == 0) {
                m_ncolumns = x->getl();
            }
            break;

        case ERSETP_CONFIGURATION:
            if (m_own_change == 0) {
                configuration = x->geto();
                if (configuration) if (configuration->classid() == ECLASSID_CONTAINER) {
                    m_own_change++;
                    configure((eContainer*)configuration);
                    m_own_change--;
                }
            }
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
    eContainer *c;

    switch (propertynr)
    {
        case ERSETP_NROWS:
            x->setl(m_nrows);
            break;

        case ERSETP_NCOLUMNS:
            x->setl(m_ncolumns);
            break;

        case ERSETP_CONFIGURATION:
            c = firstc(EOID_TABLE_CONFIGURATION);
            x->seto(c);
            break;

        default:
            return eObject::simpleproperty(propertynr, x);
    }
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
#if 0
    os_boolean comma1, comma2;
    eVariable tmp;
    os_int row, column, type_id;
    os_boolean has_value;

    indent++;
    if (json_puts(stream, "[")) goto failed;
    comma1 = OS_FALSE;
    for (row = 0; row < m_nrows; row++)
    {

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

            /* If this is a table, we want to show row number instead of flags column.
             */
            if (m_columns && column == ERSET_FLAGS_COLUMN_NR) {
                // setl(row_nr, ERSET_FLAGS_COLUMN_NR, ERSET_FLAGS_ROW_OK);
                tmp.setl(row + 1);
                has_value = OS_TRUE;
            }
            else {
                has_value = getv(row, column, &tmp);
            }

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
                    if (json_putqs(stream, "?")) goto failed;
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
#endif
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

}

/* Update a row or rows of a table.
 */
eStatus eRowSet::update(
    const os_char *whereclause,
    eContainer *row,
    os_int tflags)
{
    return ESTATUS_FAILED;
}

/* Remove rows from table.
 */
void eRowSet::remove(
    const os_char *whereclause,
    os_int tflags)
{
}

/* Select rows from table.
 */
eStatus eRowSet::select(
    const os_char *whereclause,
    eContainer *columns,
    etable_select_callback *callback,
    eObject *context,
    os_int tflags)

{
    return ESTATUS_FAILED;
}
