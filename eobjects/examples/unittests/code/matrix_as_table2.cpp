/**

  @file    matrix_as_table2.cpp
  @brief   Unit test, eMatrix class implementation of table API.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    15.10.2020

  A matrix can be used directly as a table. Direct use as table may seem unnecessary,
  but the table interface to matrix makes sense once table is accessed over message transport.

  The table API:
  - configure: Configure table columns, initial rows, etc.
  - insert: Insert a row or rows to table.
  - remove: Remove rows from table.
  - update: Update table row or rows.
  - select: Select data from table.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "eobjects.h"
#include "matrix.h"

/* Prototypes of forward referred static functions.
 */
static void configure_columns(
    eMatrix& mtx);

static void insert_row(
    eMatrix& mtx,
    os_int rownr,
    const os_char *text);

static void remove_row(
    eMatrix& mtx,
    os_int rownr);

static void remove_row2(
    eMatrix& mtx);

static void update_row(
    eMatrix& mtx,
    os_int rownr,
    const os_char *text);

static void select_rows(
    eMatrix& mtx);

static void select_callback(
    eTable *t,
    eMatrix *data,
    eObject *context);



void matrix_as_table_2()
{
    eMatrix mtx;

    configure_columns(mtx);
    insert_row(mtx, 3, "Mechanical Tiger");
    insert_row(mtx, 4, "Jack the Bouncer");
    insert_row(mtx, 16, "Silly Creeper");
    insert_row(mtx, 14, "Astounding Apple");

    select_rows(mtx);

    remove_row(mtx, 14);
    remove_row2(mtx);
    update_row(mtx, 3, "Mighty Mechanical Squirrel");

    osal_console_write("\n\nMatrix as JSON:\n\n");
    mtx.print_json();

    osal_console_write("\n");
}


static void configure_columns(
    eMatrix& mtx)
{
    eContainer *configuration, *columns;
    eVariable *column;

    configuration = new eContainer();
    columns = new eContainer(configuration, EOID_TABLE_COLUMNS);
    columns->addname("columns", ENAME_NO_MAP);

    /* For matrix as a table row number is always the first column in configuration.
     */
    column = new eVariable(columns);
    column->addname("ix", ENAME_NO_MAP);
    column->setpropertys(EVARP_TEXT, "rivi");

    column = new eVariable(columns);
    column->addname("connected", ENAME_NO_MAP);
    column->setpropertyi(EVARP_TYPE, OS_STR);

    column = new eVariable(columns);
    column->addname("connectto", ENAME_NO_MAP);

    mtx.configure(configuration);
    delete configuration;
}


static void insert_row(
    eMatrix& mtx,
    os_int rownr,
    const os_char *text)
{
    eContainer row;
    eVariable *element;

    element = new eVariable(&row);
    element->addname("ix", ENAME_NO_MAP);
    element->setl(rownr);

    element = new eVariable(&row);
    element->addname("connected", ENAME_NO_MAP);
    element->setl(OS_TRUE);

    element = new eVariable(&row);
    element->addname("connectto", ENAME_NO_MAP);
    element->sets(text);

    mtx.insert(&row);
}

static void remove_row(
    eMatrix& mtx,
    os_int rownr)
{
    eVariable where;

    where = "[";
    where += rownr;
    where += "]";
    mtx.remove(where.gets());
}

static void remove_row2(
    eMatrix& mtx)
{
    mtx.remove("connectto<\'Mechanical\'");
}

static void update_row(
    eMatrix& mtx,
    os_int rownr,
    const os_char *text)
{
    eContainer row;
    eVariable *element;
    eVariable where;

    where = "[";
    where += rownr;
    where += "]";

    element = new eVariable(&row);
    element->addname("connectto", ENAME_NO_MAP);
    element->sets(text);

    element = new eVariable(&row);
    element->addname("ix", ENAME_NO_MAP);
    element->setl(12);

    mtx.update(where.gets(), &row);
}

static void select_rows(
    eMatrix& mtx)
{
    eVariable where;
    eContainer columns;
    eVariable *element;

    element = new eVariable(&columns);
    element->addname("connectto", ENAME_NO_MAP);

    where = "[";
    where += 10;
    where += ",";
    where += 20;
    where += "]";
    mtx.select(where.gets(), &columns, select_callback, OS_NULL);
}

static void select_callback(
    eTable *t,
    eMatrix *data,
    eObject *context)
{
    eVariable tmp;
    os_int ncols, nrows, c, r;

    ncols = data->ncolumns();
    nrows = data->nrows();

    for (r = 0; r<nrows; r++) {
        for (c = 0; c<ncols; c++) {
            data->getv(r, c, &tmp);
            osal_debug_error_str("Element: ", tmp.gets());
        }
    }
}

