/**

  @file    main.cpp
  @brief   Example code object properties.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    10.10.2020

  This example demonstrates how to create propertys.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "eobjects.h"

#include "connection.h"
#include "container.h"
#include "endpoint.h"
#include "names.h"
#include "properties.h"
#include "threads.h"
#include "variables.h"

/* If needed for the operating system, EOSAL_C_MAIN macro generates the actual C main() function.
   and macro EMAIN_CONSOLE_ENTRY eobjects specific osal_main() function which calls emain.
 */
EOSAL_C_MAIN
EMAIN_CONSOLE_ENTRY

/**
****************************************************************************************************

  @brief Application entry point.

  The emain() function is eobjects application's entry point. Creating emain() function
  is optional, you can use any entry point like standard main() and call initialization
  functions from there.

  @param   argc Number of command line arguments.
  @param   argv Array of string pointers, one for each command line argument. UTF8 encoded.

  @return  ESTATUS_SUCCESS if all fine, other values indicate an error.

****************************************************************************************************
*/
eStatus emain(
    os_int argc,
    os_char *argv[])
{
    property_example_4();

    return ESTATUS_SUCCESS;
}
