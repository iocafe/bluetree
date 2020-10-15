/**

  @file    matrix.cpp
  @brief   Storing data as matrix.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    10.10.2020

  Matrix is collection of elements organized in rows and columns. The code here fills matrix
  with data.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "eobjects.h"
#include "matrix.h"

void matrix_example1()
{
    eMatrix mtx;
    eVariable value;
    os_int i, x, y;

    const os_int w = 8, h = 12;

    /* Allocating matrix in advance is optional, but will make memory allocation more efficient.
       Data type: OS_FLOAT matrix can store floating point number in each element.
       Specifying data type allows more efficient storage and serialization. Empty values
       are marked with "" in JSON.
     */
    mtx.allocate(OS_FLOAT, h, w);
    for (i = 0; i<200; i++) {
        x = osal_rand(0, w - 1);
        y = osal_rand(0, h - 1);
        mtx.setd(y, x, 0.01 * osal_rand(0, 1000));
    }

    osal_console_write("Matrix as JSON:\n\n");
    mtx.print_json();

    /* Set up matrix to store any data type and store strings to it. Data type OS_OBJECT
       is the default data type if mtx.allocate() is not called: Each element has it's
       own type and data types can be mixed within matrix.
     */
    mtx.allocate(OS_OBJECT, h, w);
    for (i = 0; i<1000; i++) {
        x = osal_rand(0, w - 1);
        y = osal_rand(0, h - 1);
        value = "s";
        value.appendl(osal_rand(0, 100));
        mtx.setv(y, x, &value);
    }

    osal_console_write("\n\nMatrix as JSON:\n\n");
    mtx.print_json();

    osal_console_write("\n");
}
