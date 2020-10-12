/**

  @file    matrix_as_table.cpp
  @brief   Storing data as matrix.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    10.10.2020

  Matrix can

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "eobjects.h"
#include "matrix.h"

void matrix_as_table_example2()
{
    eMatrix mtx;
    eVariable value;
    eContainer *configuration, *columns;
    eVariable *column;

    // os_int i, x, y;
    //os_char nbuf[OSAL_NBUF_SZ];

    configuration = new eContainer();
    columns = new eContainer(configuration, EOID_TABLE_COLUMNS);
    columns->addname("columns", ENAME_PARENT_NS|ENAME_NO_MAP);

    column = new eVariable(columns);
    column->addname("ix", ENAME_PARENT_NS|ENAME_NO_MAP);
    column->setpropertys(EVARP_TEXT, "rivi");

    column = new eVariable(columns);
    column->addname("connected", ENAME_PARENT_NS|ENAME_NO_MAP);
    column->setpropertyi(EVARP_TYPE, OS_STR);

    column = new eVariable(columns);
    column->addname("connectto", ENAME_PARENT_NS|ENAME_NO_MAP);

    mtx.configure(configuration);

    /* Set up matrix to store any data type and store strings to it. Data type OS_OBJECT
       is the default data type if mtx.allocate() is not called: Each element has it's
       own type and data types can be mixed within matrix.
     */
/*     mtx.allocate(OS_OBJECT, h, w);
    for (i = 0; i<1000; i++) {
        x = osal_rand(0, w - 1);
        y = osal_rand(0, h - 1);
        osal_int_to_str(nbuf, sizeof(nbuf), osal_rand(0, 100));

        value = "s";
        value += nbuf;
        mtx.setv(y, x, &value);
    } */

    osal_console_write("\n\nMatrix as JSON:\n\n");
    mtx.print_json();

    osal_console_write("\n");
}
