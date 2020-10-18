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

/* Table property names.
 */
const os_char
    emtxp_configuration[] = "configuration";


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
    eVariable *src_column, *next_src_column;
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

        for (src_column = src_columns->firstv(), column_nr0 = 0;
             src_column;
             src_column = next_src_column, column_nr0++)
        {
            next_src_column =  src_column->nextv();
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
    const os_char *whereclause)
{
    eObject *o;
    eWhere *w;

    o = first(EOID_TABLE_WHERE);
    delete o;

    w = new eWhere(this, EOID_TABLE_WHERE, EOBJ_TEMPORARY_ATTACHMENT);
    if (w->compile(whereclause))
    {
        osal_debug_error_str("Where clause syntax error: ", whereclause);
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


#if 0
/* Convert select parameters from c-struct to set.
 */
void eselect_struct_to_set(
    eSet *set,
    eSelectParameters *prm)
{
    if (prm->limit) {
        set->setl(ESELECT_LIMIT, prm->limit);
    }

    if (prm->page_mode) {
        set->setl(ESELECT_PAGE_MODE, prm->page_mode);
    }

    if (prm->row_mode) {
        set->setl(ESELECT_ROW_MODE, prm->row_mode);
    }

    if (prm->tzone) {
        set->seto(ESELECT_TZONE, prm->tzone);
    }
}

/* Convert select parameters from set to c-struct.
 */
void eselect_struct_to_set(
    eSelectParameters *prm,
    eSet *set,
    eObject *parent)
{
    eVariable *tzone_var;

    prm->limit = set->getl(ESELECT_LIMIT);
    prm->page_mode = set->getl(ESELECT_PAGE_MODE);
    prm->row_mode = set->getl(ESELECT_ROW_MODE);

    tzone_var = new eVariable(parent, EOID_ITEM, EOBJ_TEMPORARY_ATTACHMENT);
    set->getv(ESELECT_TZONE, tzone_var);
    prm->tzone = tzone_var->geto();
}
#endif
