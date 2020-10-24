/**

  @file    ematrix_as_table.cpp
  @brief   Table API for the eMatrix class.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    15.10.2020

  eMatrix class code to implement table API. Direct use as table may seem unnecessary,
  but the table interface to matrix makes sense once table is accessed over message transport.

  The table API:
  - configure: Configure table columns, initial rows, etc.
  - insert: Insert a row or rows to table.
  - remove: Remove rows from table.
  - update: Update table row or rows.
  - select: Select data from table.

  Notes:
  - flags column Is always the first matrix column. It is used to mark which matrix rows contain
    data and which are to be treated as empty. It is invisible to upper level of code. The
    matrix row number 1... is presented to upper levels as colum 0.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "eobjects.h"

/**
****************************************************************************************************

  @brief Configure matrix as table (Table interface function).

  The eMatrix::configure() function configures matrix as table. This function
  - Stores column configuration.
  - Sets matrix size and data type - DATA TYPE IS NOW FIXED OS_OBJECT
  - Adds initial data rows to empty matrix.

  @param   configuration Table configuration, columns.
  @param   tflags Set 0 for default configuration. Set ETABLE_ADOPT_ARGUMENT to adopt/delete
           configuration. If set the configuration object pointer must not be used after the
           function call returns.

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
    c = process_configuration(configuration, &nro_columns, tflags);

    if (c) {
        m_own_change++;
        resize(m_datatype, m_nrows, nro_columns);
        // setpropertyl(EMTXP_DATATYPE, OS_OBJECT); // WE WILL WANT TO HAVE OTHER DATA TYPES AS WELL
        setpropertyl(EMTXP_NCOLUMNS, nro_columns);
        if (m_own_change <= 1) {
            setpropertyo(EMTXP_CONFIGURATION, c);
        }
        m_own_change--;
        m_columns = c->firstc(EOID_TABLE_COLUMNS);
    }
}

/* Get pointer to table configuration (eContainer).
 */
eContainer *eMatrix::configuration()
{
    return firstc(EOID_TABLE_CONFIGURATION);
}


/**
****************************************************************************************************

  @brief Insert rows into table (Table interface function).

  The eMatrix::insert() function inserts one or more new rows to table.

  @param   rows For single for: eContainer holding a eVariables for each element to set.
           Multiple rows: eContainer holding a eContainers for each row to insert. Each row
           container contains eVariable for each element to set.
  @param   tflags Reserved for future, set 0 for now.

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
        insert_one_row(rows, -1);
    }

    else {
        do {
            insert_one_row(row, -1);
            row = row->nextc();
        }
        while (row);
    }
}


/**
****************************************************************************************************

  @brief Insert single rows into table (helper function).

  This is a helper function for update and insert to actually set data to one single row
  of the matrix.

  @param   row eContainer holding a eVariables for each element to set.
  @param   use_row_nr Row number to update. This tells the function which matrix row is being
           updated. If "row" argument specifies new row number, then this row is moved.
           Insert function sets here -1 to indicate that we are not updating a row.
  @param   tflags Reserved for future, set 0 for now.

****************************************************************************************************
*/
void eMatrix::insert_one_row(
    eContainer *row,
    os_int use_row_nr)
{
    eVariable *element, *index_element, *column;
    eName *name;
    os_char *namestr;
    os_long row_nr;
    os_int column_nr;

    index_element = find_index_element(row);
    if (index_element == OS_NULL) {
        /* If row number is unspecified, use first free
         */
        if (use_row_nr < 0) {
            for (row_nr = 0; row_nr < m_nrows; row_nr++) {
                if ((getl(row_nr, EMTX_FLAGS_COLUMN_NR) & EMTX_FLAGS_ROW_OK) == 0) {
                    break;
                }
            }
        }
        else {
            row_nr = use_row_nr;
        }
    }
    else {
        row_nr = index_element->getl() - 1;
        if (row_nr < 0) {
            if (use_row_nr < 0) {
                for (row_nr = 0; row_nr < m_nrows; row_nr++) {
                    if ((getl(row_nr, EMTX_FLAGS_COLUMN_NR) & EMTX_FLAGS_ROW_OK) == 0) {
                        break;
                    }
                }
            }
            else {
                row_nr = use_row_nr;
            }
        }
        else if (use_row_nr >= 0 && row_nr != use_row_nr) {
            copy_row(row_nr, use_row_nr);
            clear_row(use_row_nr);
        }
    }

    /* If we insert new row, clear any old data
     */
    if (use_row_nr < 0) {
        clear_row(row_nr);
    }

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
        setv(row_nr, column_nr, element);
    }

    /* Set flags column
     */
    setl(row_nr, EMTX_FLAGS_COLUMN_NR, EMTX_FLAGS_ROW_OK);
}


/**
****************************************************************************************************

  @brief Find index column eVariable from row to insert (helper function).

  @param   row eContainer holding a eVariables for each element.
  @return  Pointer to eVariable within row which contains index value, if any.
           OS_NULL if row doesn't contain index.

****************************************************************************************************
*/
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


/**
****************************************************************************************************

  @brief Update a row or rows of a table (Table interface function).

  @param   whereclause String containing range and/or actual where clause. This selects which
           rows are updated.
  @param   row A row of updated data. eContainer holding an eVariable for each element (column)
           to update. eVariable name is column name.
  @param   tflags Reserved for future, set 0 for now.

  @return  OSAL_SUCCESS if ok.

****************************************************************************************************
*/
eStatus eMatrix::update(
    const os_char *whereclause,
    eContainer *row,
    os_int tflags)
{
    return select_update_remove(EMTX_UPDATE, whereclause, row, OS_NULL, tflags);
}


/**
****************************************************************************************************

  @brief Remove rows from the table (Table interface function).

  @param   whereclause String containing range and/or actual where clause. This selects which
           rows are to be removed.
  @param   tflags Reserved for future, set 0 for now.

****************************************************************************************************
*/
void eMatrix::remove(
    const os_char *whereclause,
    os_int tflags)
{
    select_update_remove(EMTX_REMOVE, whereclause, OS_NULL, OS_NULL, tflags);
}


/**
****************************************************************************************************

  @brief Select rows from table (Table interface function).

  Selects data from table according. Rows to be selected are specified by where clause.
  Column which to get are listed in "columns" list. The selected data is returned trough
  the callback function.

  @param   whereclause String containing range and/or actual where clause. This selects which
           rows are updated.
  @param   columns List of columns to get. eContainer holding an eVariable for each column
           to select. eVariable name is column name, or column name can also be stored as
           variable value.

  @param   tflags Reserved for future, set 0 for now.

  @return  OSAL_SUCCESS if ok.

****************************************************************************************************
*/
eStatus eMatrix::select(
    const os_char *whereclause,
    eContainer *columns,
    eSelectParameters *prm,
    os_int tflags)
{
    return select_update_remove(EMTX_SELECT, whereclause, columns, prm, tflags);
}


/**
****************************************************************************************************

  @brief Select, update or remove rows from table (internal).

  @param   op What to do: EMTX_UPDATE, EMTX_REMOVE or EMTX_SELECT.
  @param   whereclause String containing range and/or actual where clause.
  @param   cont A row of updated data or eContainer holding columns to select.
  @param   callback Pointer to callback function which will receive the data. The
           callback function may be called multiple times to receive data as matrices with
           N data rows in each. N is chosen for efficiency.
  @param   context Application specific context pointer to pass to callback function.
  @param   tflags Reserved for future, set 0 for now.

  @return  OSAL_SUCCESS if ok. Other return values indicate an error or interrupted data transfer.

****************************************************************************************************
*/
eStatus eMatrix::select_update_remove(
    eMtxOp op,
    const os_char *whereclause,
    eContainer *cont,
    eSelectParameters *prm,
    os_int tflags)
{
    eWhere *w = OS_NULL;
    os_int *col_mtx = OS_NULL, *sel_mtx = OS_NULL;
    os_memsz col_mtx_sz = 0, sel_mtx_sz = 0;
    eContainer *vars = OS_NULL;
    eVariable *v, *u, *tmp = OS_NULL;
    eName *name;
    os_char *namestr;
    eMatrix *m;
    ePointer *ref = OS_NULL;
    os_long minix, maxix;
    os_int row_nr, i, col_nr, nvars, nro_selected_cols;
    os_memsz count;
    eStatus s, rval = ESTATUS_SUCCESS;
    os_boolean eval_error_reported = OS_FALSE;

    if (m_columns == OS_NULL) {
        osal_debug_error("eMatrix::select_update_remove: Not configured");
        return ESTATUS_FAILED;
    }

    /* Asterix '*' as where clause is all rows, same as empty where clause.
     */
    if (!os_strcmp(whereclause, "*")) {
        whereclause = OS_NULL;
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
                        namestr = name->gets();
                        u = eVariable::cast(m_columns->byname(namestr));
                        if (u) {
                            col_nr = u->oid();
                        }
                        else {
                            osal_debug_error_str("Where clause contains column name: ", namestr);
                        }
                    }
                    col_mtx[i] = col_nr;
                }
            }
        }
    }

    /* Set up for select.
     */
    nro_selected_cols = 0;
    if (op == EMTX_SELECT) {
        ref = new ePointer(this, EOID_ITEM, EOBJ_TEMPORARY_ATTACHMENT);
        tmp = new eVariable(this, EOID_ITEM, EOBJ_TEMPORARY_ATTACHMENT);

        if (cont != NULL) {
            for (v = cont->firstv(); v; v = v->nextv()) {
                nro_selected_cols++;
            }

            if (nro_selected_cols) {
                sel_mtx_sz = nro_selected_cols * sizeof(os_int);
                sel_mtx = (os_int*)os_malloc(sel_mtx_sz, OS_NULL);
            }

            for (v = cont->firstv(), i = 0; v; v = v->nextv(), i++)
            {
                col_nr = -1;
                name = v->primaryname();
                if (name) {
                    namestr = name->gets();
                }
                else {
                    namestr = v->gets();
                }
                u = eVariable::cast(m_columns->byname(namestr));
                if (u) {
                    col_nr = u->oid();
                }
                sel_mtx[i] = col_nr;
            }
        }
    }

    /* Process rows by row.
     */
    for (row_nr = (os_int)minix; row_nr <= maxix; row_nr++)
    {
        /* If row has been deleted
         */
        if ((getl(row_nr, EMTX_FLAGS_COLUMN_NR) & EMTX_FLAGS_ROW_OK) == 0)
            continue;

        /* If we have where clause
         */
        if (vars) {
            /* Set values to where from row.
             */
            for (v = vars->firstv(), i = 0; v; v = v->nextv(), i++) {
                col_nr = col_mtx[i];
                if (col_nr == EMTX_FLAGS_COLUMN_NR) {
                    v->setl(row_nr + 1);
                }
                else if (col_nr >= 0) {
                    getv(row_nr, col_nr, v);
                }
            }
        }
        if (w) {
            /* Evaluate where clause, skip operation on row if no match.
             */
            s = w->evaluate();
            if (s) {
                if (s != ESTATUS_FALSE && !eval_error_reported)
                {
                    rval = s;
                    osal_debug_error_str("Where clause failed: ", whereclause);
                    eval_error_reported = OS_TRUE;
                }
                continue;
            }
        }

        /* Do the operation.
         */
        switch (op) {
            case EMTX_UPDATE:
                insert_one_row(cont, row_nr);
                break;

            case EMTX_REMOVE:
                clear_row(row_nr);
                break;

            case EMTX_SELECT:
                m = new eMatrix(this, EOID_ITEM, EOBJ_TEMPORARY_ATTACHMENT);
                ref->set(m);

                /* Set values to selected row to return.
                 */
                if (nro_selected_cols > 0) {
                    m->allocate(datatype(), 1, nro_selected_cols);

                    for (i = 0; i < nro_selected_cols; i++) {
                        col_nr = sel_mtx[i];
                        if (col_nr == EMTX_FLAGS_COLUMN_NR) {
                            m->setl(0, i, row_nr + 1);
                        }
                        else if (col_nr >= 0) {
                            getv(row_nr, col_nr, tmp);
                            m->setv(0, i, tmp);
                        }
                    }
                }

                /* No columns specified, return whole row.
                 */
                else {
                    m->allocate(datatype(), 1, m_ncolumns);
                    for (i = 0; i < m_ncolumns; i++) {
                        if (i == EMTX_FLAGS_COLUMN_NR) {
                            m->setl(0, i, row_nr + 1);
                        }
                        else {
                            getv(row_nr, i, tmp);
                            m->setv(0, i, tmp);
                        }
                    }
                }

                /* Callback.
                 */
                if (prm) if (prm->callback) {
                    rval = prm->callback(this, m, prm->context);
                    if (rval) goto getout;
                }

                /* Clean up in case callback did not adopt the matrix.
                 */
                delete ref->get();
                break;
        }
    }

getout:
    if (col_mtx_sz) {
        os_free(col_mtx, col_mtx_sz);
    }
    if (sel_mtx_sz){
        os_free(sel_mtx, sel_mtx_sz);
    }
    delete ref;
    delete tmp;
    return rval;
}


/**
****************************************************************************************************

  @brief Pass messages to DBM object.

  The function makes sure that DBM object exist as attachment to the matrix and passes
  table related messages table to it. The DBM object manages the matrix trough table interface.

  @param   envelope Message envelope. Contains command, target and source paths and
           message content, etc.

****************************************************************************************************
*/
void eMatrix::dbm_message(
    eEnvelope *envelope)
{
    eObject *dbm;

    dbm = first(EOID_DBM);
    if (dbm == OS_NULL) {
        dbm = new eDBM(this, EOID_DBM, EOBJ_TEMPORARY_ATTACHMENT);
    }
    dbm->onmessage(envelope);
}
