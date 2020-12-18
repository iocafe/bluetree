/**

  @file    emain.h
  @brief   Calling eobjects application entry point.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    25.12.2016

  This file implements osal_main() function, which intializes eobject library and calls emain()
  function to start the application.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef EMAIN_H_
#define EMAIN_H_
#include "eobjects.h"

/* If C++ compilation, all functions, etc. from this point on in this header file are
   plain C and must be left undecorated.
 */
OSAL_C_HEADER_BEGINS

/* Prototype for application's entry point function.
 */
eStatus emain(
    os_int argc,
    os_char *argv[]);

/* If C++ compilation, end the undecorated code.
 */
OSAL_C_HEADER_ENDS


/**
****************************************************************************************************

  @brief Call eobjects application entry point.
  @anchor osal_main

  Macro implementation of osal_main() function is part of entry sequence to eobjects console
  application. It is not used when eobject is used as library nor propably with GUI applications.

  @param   argc Number of command line arguments.
  @param   argv Array of string pointers, one for each command line argument plus.
                "-n=X" sets process number (device number) X, for example "-n=3".

  @return  Integer return value to caller, OSAL_SUCCESS (0) to indicate "all fine".

****************************************************************************************************
*/
#define EMAIN_CONSOLE_ENTRY(process_name) osalStatus osal_main(os_int argc, os_char *argv[]) { \
    osalStatus s; \
    eobjects_initialize(process_name, argc, argv, EOBJECTS_DEFAULT_INIT); \
    eprocess_create(); \
    s = (osalStatus)emain(argc, argv); \
    eprocess_close(); \
    eobjects_shutdown(); \
    return s; }

#endif
