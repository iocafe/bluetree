/**

  @file    properties1.cpp
  @brief   Example code object properties.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

  This example demonstrates how to create a property and sen messages to it.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "eobjects.h"
#include "properties.h"


/**
****************************************************************************************************
  Property example 2.
****************************************************************************************************
*/
void property_example_2()
{
    eVariable v, u;

    v.setpropertys(EVARP_VALUE, "ulle");
    osal_console_write("\nv.gets() = ");
    osal_console_write(v.gets());

    v.setpropertyd(EVARP_VALUE, 10.22);
    osal_console_write("\nv.gets() = ");
    osal_console_write(v.gets());

    v.setpropertyd(EVARP_DIGS, 5);
    osal_console_write("\nv.gets() = ");
    osal_console_write(v.gets());

    v.setpropertys(EVARP_TEXT, "nasse");
    v.propertyv(EVARP_TEXT, &u);
    osal_console_write("\nv.propertyv(EVARP_TEXT, &u), u.gets() = ");
    osal_console_write(u.gets());

    osal_console_write("\n");
}
