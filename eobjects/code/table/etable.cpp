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


/* Generic table attribute list.
 */
const etableConfAttr etable_attrs[]
    = {{"text", ETABLEP_TEXT, ETABLE_BASIC_ATTR_GROUP}};

/* Number of genertic table attributes.
 */
const os_int etable_nro_attrs = (sizeof(etable_attrs)/sizeof(etableConfAttr));


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


/* Add generic table properties to derived class'es property set.
 */
void eTable::add_generic_table_properties(
    os_int cls,
    os_int group_flags)
{
    os_int i;

    for (i = 0; i < etable_nro_attrs; i++) {
        if (group_flags & etable_attrs[i].group_flags)
        {
            addproperty(cls, etable_attrs[i].property_nr, etable_attrs[i].attr_name,
                etable_attrs[i].attr_name, EPRO_PERSISTENT);
        }
    }
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

    dst_configuration = new eContainer(this, EOID_TABLE_CONFIGURATION, EOBJ_IS_ATTACHMENT);
    dst_configuration->ns_create();

    src_columns = configuration->firstc(EOID_TABLE_COLUMNS);
    if (src_columns == OS_NULL) {
        src_columns = eContainer::cast(configuration->byname("columns"));
    }

    if (src_columns) {
        dst_columns = new eContainer(dst_configuration, EOID_TABLE_COLUMNS);
        dst_columns->addname("columns", ENAME_PRIMARY|ENAME_NO_MAP);
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


/* Store generic table configuration attributes in table properties.
 */
void eTable::process_configuration_attribs(
    eContainer *configuration,
    os_int group_flags)
{
    eContainer *src_attrs;
    eVariable *src_attr;
    eName *n;
    os_char *namestr;
    os_int id, i;

    src_attrs = configuration->firstc(EOID_TABLE_ATTR);
    if (src_attrs == OS_NULL) {
        src_attrs = eContainer::cast(configuration->byname("attr"));
    }
    if (src_attrs) {
        for (src_attr = src_attrs->firstv(); src_attr; src_attr = src_attr->nextv())
        {
            id = src_attr->oid();
            n = src_attr->primaryname();
            namestr = OS_NULL;
            if (n) namestr = n->gets();

            for (i = 0; i < etable_nro_attrs; i++)
            {
                if (group_flags & etable_attrs[i].group_flags) {
                    if (id == etable_attrs[i].property_nr ||
                        !os_strcmp(namestr, etable_attrs[i].attr_name))
                    {
                        setpropertyv(etable_attrs[i].property_nr, src_attr);
                    }
#if OSAL_DEBUG
                    else {
                        osal_debug_error_int("eTable: Unknown config attr ", id);
                    }
#endif
                }
            }
        }
    }
}


/* Set generic table attributes from properties to table configuration.
 */
void eTable::add_attribs_to_configuration(
    eContainer *configuration,
    os_int group_flags)
{
    eContainer *attrs;
    eVariable *attr;
    eVariable v;
    os_int i;

    attrs = configuration->firstc(EOID_TABLE_ATTR);
    if (attrs == OS_NULL) {
        attrs = eContainer::cast(configuration->byname("attr"));
    }
    if (attrs == OS_NULL) {
        attrs = new eContainer(configuration, EOID_TABLE_ATTR);
        attrs->addname("attr", ENAME_PRIMARY|ENAME_NO_MAP);
        attrs->ns_create();
    }

    for (i = 0; i < etable_nro_attrs; i++) {
        if (group_flags & etable_attrs[i].group_flags) {
            propertyv(etable_attrs[i].property_nr, &v);
            if (!v.isempty()) {
                attr = new eVariable(attrs, etable_attrs[i].property_nr);
                attr->addname(etable_attrs[i].attr_name, ENAME_PRIMARY|ENAME_NO_MAP);
                attr->setv(&v);
            }
        }
    }
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
