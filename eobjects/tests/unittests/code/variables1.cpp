/**

  @file    variables.cpp
  @brief   Example code about using eobjects variable class.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

  This example demonstrates how to use dynamically typed variable class eVariable.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "eobjects.h"
#include "variables.h"

void variables_example1()
{
    eVariable
        var,
        dar;

    os_long
        u = 22;

    dar = var = "abba ";
    dar = 10;
    dar = u;
    osal_console_write(dar.gets());
    osal_console_write("\n");

    var += dar;
    var += " ja nasse";
    var += " ja pulla";

    osal_console_write(var.gets());
    osal_console_write("\n");

    dar = 1110.018;
    dar.setdigs(5);
    dar += var;
    osal_console_write(dar.gets());
    osal_console_write("\n");
}
