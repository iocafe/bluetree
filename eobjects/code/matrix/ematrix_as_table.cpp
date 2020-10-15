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
    const os_char *whereclause,
    eContainer *row,
    os_int tflags)
{
    return select_update_remove(EMTX_UPDATE, whereclause, OS_NULL, tflags);
}

/* Remove rows from table.
 */
void eMatrix::remove(
    const os_char *whereclause,
    os_int tflags)
{
    select_update_remove(EMTX_REMOVE, whereclause, OS_NULL, tflags);
}

/* Select rows from table.
 */
eStatus eMatrix::select(
    const os_char *whereclause,
    etable_select_callback *callback,
    os_int tflags)
{
    return select_update_remove(EMTX_SELECT, whereclause, callback, tflags);
}



/* Select rows from table.
 */
eStatus eMatrix::select_update_remove(
    eMtxOp op,
    const os_char *whereclause,
    etable_select_callback *callback,
    os_int tflags)
{
    eWhere *w = OS_NULL;
    os_int *col_mtx = OS_NULL;
    os_memsz col_mtx_sz = 0;
    eContainer *vars = OS_NULL;
    eVariable *v, *u;
    eName *name;
    os_long minix, maxix;
    os_int row, i, col_nr, nvars;
    os_memsz count;

    if (m_columns == OS_NULL) {
        osal_debug_error("eMatrix::select_update_remove: Not configured");
        return ESTATUS_FAILED;
    }

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
    if (whereclause) if (*whereclause) {
        w = set_where(whereclause);
        if (w == OS_NULL) {
            return ESTATUS_FAILED;
        }
        nvars = w->nvars();
        if (nvars > 0) {
            col_mtx_sz = nvars * sizeof(os_int);
            col_mtx = (os_int*)os_malloc(col_mtx_sz, OS_NULL);

            vars = w->variables();
            if (vars) {
                for (v = vars->firstv(), i = 0; v; v = v->nextv(), i++)
                {
                    col_nr = -1;
                    name = v->primaryname();
                    if (name) {
                        u = eVariable::cast(m_columns->byname(name->gets()));
                        if (u) {
                            col_nr = u->oid();
                        }
                    }
                    col_mtx[i] = col_nr;
                }
            }
        }
    }

    /* Process rows by row.
     */
    for (row = (os_int)minix; row <= maxix; row++)
    {
        /* If row has been deleted
         */
        if ((getl(row, EMTX_FLAGS_COLUMN_NR) & EMTX_FLAGS_ROW_OK) == 0)
            continue;

        /* If we have where clause
         */
        if (vars) {
            /* Set values to where from row.
             */
            for (v = vars->firstv(), i = 0; v; v = v->nextv(), i++) {

                col_nr = col_mtx[i];
                if (col_nr == EMTX_FLAGS_COLUMN_NR) {
                    v->setl(row + 1);
                }
                else {
                    getv(row, col_nr, v);
                }
            }
        }
        if (w) {
            /* Evaluate where clause, skip operation on row if no match.
             */
            if (w->evaluate()) {
                continue;
            }
        }

        /* Do the operation.
         */
        switch (op) {
            case EMTX_UPDATE:
                break;

            case EMTX_REMOVE:
                clear(row);
                break;

            case EMTX_SELECT:
                break;
        }
    }

    if (col_mtx_sz) {
        os_free(col_mtx, col_mtx_sz);
    }
    return ESTATUS_SUCCESS;
}
