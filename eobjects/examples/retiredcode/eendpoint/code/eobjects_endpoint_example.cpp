/**

  @file    eobjects_endpoint_example.cpp
  @brief   Example code about connecting two processes.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

  This demonstrates creating an end point which listens for socket connections.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "eobjects.h"
#include "eobjects_endpoint_example.h"

/* Generate entry code for console application.
 */
EMAIN_CONSOLE_ENTRY("endpointtest")

/**
****************************************************************************************************

  @brief Application entry point.

  The emain() function is eobjects application's entry point.

  @param   argc Number of command line arguments.
  @param   argv Array of string pointers, one for each command line argument. UTF8 encoded.

  @return  None.

****************************************************************************************************
*/
os_int emain(
    os_int argc,
    os_char *argv[])
{
    endpoint_example_1();

    return 0;
}
