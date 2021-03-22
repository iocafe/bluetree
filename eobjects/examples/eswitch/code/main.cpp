/**

  @file    main.cpp
  @brief   Switchbox example eswitch entry point.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    2.11.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "eswitch.h"


/* EOSAL_C_MAIN macro generates the actual C main() function and macro EMAIN_CONSOLE_ENTRY
   the "eobjects" specific osal_main() function which calls emain.
 */
EOSAL_C_MAIN
EMAIN_CONSOLE_ENTRY("eswitch")

/**
****************************************************************************************************

  @brief Switchbox example eswitch entry point.

  @oaran   argc Number of command line arguments (PC only)
  @oaran   argv Array of command line argument pointers (PC only)
  @return  ESTATUS_SUCCESS if all fine, other values indicate an error.

****************************************************************************************************
*/
eStatus emain(
    os_int argc,
    os_char *argv[])
{
    // eThreadHandle service_thread_handle;
    eThreadHandle fsys_thread_handle;
    OSAL_UNUSED(argc);
    OSAL_UNUSED(argv);

    efsys_expose_directory("//fsys", eglobal->root_path, &fsys_thread_handle);

    enet_initialize_service();
    os_lock(); /* root pointer used */
    enet_add_protocol(new ecomProtocol(eglobal_root()));
    /* enet_add_protocol(new eioProtocol(eglobal_root())); */
    enet_add_protocol(new esboxProtocol(eglobal_root()));
    os_unlock();
    enet_start_service(
        ENET_ENABLE_EOBJECTS_SERVICE|
        ENET_ENABLE_SWITCHBOX_SERVICE|
        ENET_ENABLE_UNSECURED_SOCKETS); /* allow unsecured for now, take this flag off for production */

    while (OS_TRUE) {
        os_sleep(1000);
    }

    enet_stop_service();

    fsys_thread_handle.terminate();
    fsys_thread_handle.join();

    return ESTATUS_SUCCESS;
}
