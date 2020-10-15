/**

  @file    matrix_as_table2.cpp
  @brief   Storing data as matrix.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    10.10.2020

  This tests is about using matrix directly as a table. Direct use as table may seem unnecessary,
  but the table interface to matrix makes sense once table is accessed over message transport.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "eobjects.h"
#include "matrix.h"

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


void matrix_as_table_example2()
{
    eMatrix mtx;

    configure_columns(mtx);
    insert_row(mtx, 3, "Mechanical Tiger");
    insert_row(mtx, 4, "Jack the Bouncer");

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
