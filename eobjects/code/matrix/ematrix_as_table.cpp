/**

  @file    ematrix_as_table.cpp
  @brief   Matrix class.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    10.10.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "eobjects.h"



/**
****************************************************************************************************

  @brief Configure matrix as table.

  The eMatrix::configure() function ...

  @param   configuration Table configuration, columns.

****************************************************************************************************
*/
void eMatrix::configure(
    eContainer *configuration,
    os_int tflags)
{
    eContainer *c;
    os_int nro_columns;

    c = firstc(EOID_TABLE_CONFIGURATION);
    if (c) {
        delete c;
        m_columns = OS_NULL;
    }

    if (configuration == OS_NULL) {
        osal_debug_error("eMatrix::configure: NULL configuration");
        return;
    }
    c = process_configuration(configuration, &nro_columns);

    if (c) {
        m_own_change++;
        resize(m_datatype, m_nrows, nro_columns);
        setpropertyl(EMTXP_DATATYPE, OS_OBJECT); // WE WILL WANT TO HAVE OTHER DATA TYPES AS WELL
        setpropertyl(EMTXP_NCOLUMNS, nro_columns);
        if (m_own_change <= 1) {
            setpropertyo(EMTXP_CONFIGURATION, c);
        }
        m_own_change--;
        m_columns = c->firstc(EOID_TABLE_COLUMNS);
    }
}


/**
****************************************************************************************************

  @brief Insert rows into table.

  The eMatrix::configure() function ...
  Row can be one row or container with multiple rows.

  @param   configuration

****************************************************************************************************
*/
void eMatrix::insert(
    eContainer *rows,
    os_int tflags)
{
    eContainer *row;

    if (rows == OS_NULL || m_columns == OS_NULL) {
        osal_debug_error("eMatrix::insert: Not configured as table or inserting NULL");
        return;
    }

    row = rows->firstc();
    if (row == OS_NULL) {
        insert_one_row(rows);
    }

    else {
        do {
            insert_one_row(row);
            row = row->nextc();
        }
        while (row);
    }
}

void eMatrix::insert_one_row(
    eContainer *row)
{
    eVariable *element, *index_element, *column;
    eName *name;
    os_char *namestr;
    os_long row_nr;
    os_int column_nr;

    index_element = find_index_element(row);
    if (index_element == OS_NULL) {
        osal_debug_error("eMatrix::insert: Matrix index column is not set");
        return;
    }

    row_nr = index_element->getl();

    for (element = row->firstv(); element; element = element->nextv())
    {
        if (element == index_element) {
            continue;
        }
        name = element->primaryname();
        if (name == OS_NULL) {
            osal_debug_error("eMatrix::insert: Element with no name");
            continue;
        }
        namestr = name->gets();
        column = eVariable::cast(m_columns->byname(namestr));
        if (column == OS_NULL) {
            osal_debug_error_str("eMatrix::insert: Table column not found: ", namestr);
            continue;
        }

        column_nr = column->oid();
        setv(row_nr - 1, column_nr, element);
    }

    /* Set flags column
     */
    setl(row_nr - 1, EMTX_FLAGS_COLUMN_NR, EMTX_FLAGS_ROW_OK);
}

eVariable *eMatrix::find_index_element(
    eContainer *row)
{
    eVariable *element, *index_column;
    eName *index_column_name, *column_name;

    index_column = m_columns->firstv();
    if (index_column == OS_NULL) {
        return OS_NULL;
    }

    /* Get index column name from configuration.
     */
    index_column_name = index_column->primaryname();
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


/* Update a row or rows of a table.
 */
eStatus eMatrix::update(
    os_char *whereclause,
    eContainer *row,
    os_int tflags)
{
    return select_update_remove(EMTX_UPDATE, whereclause, OS_NULL, tflags);
}

/* Remove rows from table.
 */
void eMatrix::remove(
    os_char *whereclause,
    os_int tflags)
{
    select_update_remove(EMTX_REMOVE, whereclause, OS_NULL, tflags);
}

/* Select rows from table.
 */
eStatus eMatrix::select(
    os_char *whereclause,
    etable_select_callback *callback,
    os_int tflags)
{
    return select_update_remove(EMTX_SELECT, whereclause, callback, tflags);
}



/* Select rows from table.
 */
eStatus eMatrix::select_update_remove(
    eMtxOp op,
    os_char *whereclause,
    etable_select_callback *callback,
    os_int tflags)
{
    eWhere *w;
    eMatrix *col_mtx;
    os_long minix, maxix;
    os_int row;
    os_memsz count;

    /* Get index range from beginning of where clause.
     */
    count = e_parse_index_range(whereclause, &minix, &maxix);
    if (count <= 0) {
        minix = 0;
        maxix = m_nrows - 1;
    }
    else {
        minix--;
        maxix--;
        whereclause += count;
    }

    /* Compile where clause and set column index for each varible in where clause
     */
    w = set_where(whereclause);
    col_mtx = new eMatrix(this, EOID_RITEM, EOBJ_TEMPORARY_ATTACHMENT);
    col_mtx->allocate(OS_SHORT, 1, 10);

    for (row = (os_int)minix; row <= maxix; row++)
    {
        /* Set values to where from row.
         */

        /* Evaluate where clause, skip row if no match.
         */
    }

    delete col_mtx;

    return ESTATUS_SUCCESS;
}
