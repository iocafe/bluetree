/**

  @file    matrix.cpp
  @brief   Example code about using eobjects variable class.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  This example demonstrates how to use dynamically typed variable class eVariable.

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
    os_int i, x, y;

    for (i = 0; i<1000; i++) {
        x = osal_rand(0, 100);
        y = osal_rand(0, 100);
        mtx.setl(y, x, osal_rand(0, 100));
    }

    mtx.print_json();
}
