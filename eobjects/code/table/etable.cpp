/**

  @file    etable.cpp
  @brief   Table interface.
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

  @brief Constructor.

  X...

  @return  None.

****************************************************************************************************
*/
eTable::eTable(
    eObject *parent,
    e_oid id,
    os_int flags)
    : eObject(parent, id, flags)
{
}


/**
****************************************************************************************************

  @brief Virtual destructor.

  X...

  @return  None.

****************************************************************************************************
*/
eTable::~eTable()
{
}


/**
****************************************************************************************************

  @brief Add eTable to class list.

  The eTable::setupclass function adds newobj function pointer to class list. This enables creating
  new objects dynamically by class identifier, which is used for serialization reader()
  functions.

****************************************************************************************************
*/
void eTable::setupclass()
{
    const os_int cls = ECLASSID_TABLE;

    /* Add the class to class list.
     */
    os_lock();
    eclasslist_add(cls, (eNewObjFunc)newobj, "eTable");
    os_unlock();
}


/**
****************************************************************************************************

  @brief Process configuration to make sure it is in useful format.

  The eTable::process_configuration processes configuration so that object identifier is
  column number for each column variable, and row data is stripped. The function ensures
  that name spaces are present and names are mapped into these.

  @param   configuration Configuration input. The configuration returned by this function
           is almost same as conguraration here, just with modification listed above.
  @param   nro_columns Pointer to integer where to store number of colums.
  @param   tflags Set 0 for default configuration. Set ETABLE_ADOPT_ARGUMENT to adopt/delete
           configuration. If set the configuration object pointer must not be used after the
           function call returns;

  @return  Pointer to the modified configuratiion stored as temporary attachment to this object
           (identifier EOID_TABLE_CONFIGURATION).

****************************************************************************************************
*/
eContainer *eTable::process_configuration(
    eContainer *configuration,
    os_int *nro_columns,
    os_int tflags)
{
    eContainer *dst_configuration;
    eContainer *src_columns, *dst_columns;
    eVariable *src_column, *next_src_column, *first_src_column, *v;
    eName *n;
    os_int column_nr0;

    *nro_columns = 0;

    dst_configuration = new eContainer(this, EOID_TABLE_CONFIGURATION, EOBJ_TEMPORARY_ATTACHMENT);
    dst_configuration->ns_create();

    src_columns = configuration->firstc(EOID_TABLE_COLUMNS);
    if (src_columns == OS_NULL) {
        src_columns = eContainer::cast(configuration->byname("columns"));
    }

    if (src_columns) {
        dst_columns = new eContainer(dst_configuration, EOID_TABLE_COLUMNS);
        dst_columns->ns_create();

        first_src_column = src_columns->firstv();
        for (src_column = first_src_column, column_nr0 = 0;
             src_column;
             src_column = next_src_column, column_nr0++)
        {
            next_src_column =  src_column->nextv();

            /* Index is always first column, at least for now.
             */
            if (src_column == first_src_column) {
                n = src_column->primaryname();
                if (n) {
                    v = new eVariable(dst_configuration, EOID_TABLE_IX_COLUMN_NAME);
                    v->setv(n);
                }
            }

            if (tflags & ETABLE_ADOPT_ARGUMENT) {
                src_column->adopt(dst_columns, column_nr0);
            }
            else {
                src_column->clone(dst_columns, column_nr0);
            }
        }
        *nro_columns = column_nr0;
    }

    if (tflags & ETABLE_ADOPT_ARGUMENT) {
        delete configuration;
    }

    return dst_configuration;
}


/* Alloctaes eWhere object as child of this object. Set where clause. Compiles it.
  @return ESTATUS_SUCCESS if ok.
 */
eWhere *eTable::set_where(
    const os_char *where_clause)
{
    eWhere *w;

    w = eWhere::cast(first(EOID_TABLE_WHERE));
    if (w == OS_NULL) {
        w = new eWhere(this, EOID_TABLE_WHERE, EOBJ_TEMPORARY_ATTACHMENT);
    }
    if (w->compile(where_clause))
    {
        osal_debug_error_str("Where clause syntax error: ", where_clause);
        delete w;
        return OS_NULL;
    }

    return w;
}


/* Get pointer to eWhere object, set by setwhere() function.
 */
/* eWhere *eTable::get_where()
{
    return eWhere::cast(first(EOID_TABLE_WHERE));
} */


/**
****************************************************************************************************

  @brief Find index column eVariable from row to insert (helper function).

  @param   row eContainer holding a eVariables for each element.
  @return  Pointer to eVariable within row which contains index value, if any.
           OS_NULL if row doesn't contain index.

****************************************************************************************************
*/
eVariable *eTable::find_index_element(
    eContainer *row)
{
    eVariable *element;
    eName *index_column_name, *column_name;

    /* Get index column name from configuration.
     */
    index_column_name = find_index_column_name();
    if (index_column_name == OS_NULL) {
        return OS_NULL;
    }

    for (element = row->firstv(); element; element = element->nextv())
    {
        column_name = element->primaryname();
        if (!index_column_name->compare(column_name)) {
            break;
        }
    }

    return element;
}
