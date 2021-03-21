/**

  @file    main.cpp
  @brief   Glass user interface client application entry point.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    2.11.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "glass.h"
#include "tableview_test_data.h"

int duudeli(void);


/* EOSAL_C_MAIN macro generates the actual C main() function and macro EMAIN_CONSOLE_ENTRY
   the "eobjects" specific osal_main() function which calls emain.
 */
EOSAL_C_MAIN
EMAIN_CONSOLE_ENTRY("glass")

/**
****************************************************************************************************

  @brief Glass user interface client application entry point.

  @oaran   argc Number of command line arguments (PC only)
  @oaran   argv Array of command line argument pointers (PC only)
  @return  ESTATUS_SUCCESS if all fine, other values indicate an error.

****************************************************************************************************
*/
eStatus emain(
    os_int argc,
    os_char *argv[])
{
    eThreadHandle fsys_thread_handle;
    eGui *gui;
    eStatus s;
    eStatus rval = ESTATUS_SUCCESS;
    OSAL_UNUSED(argc);
    OSAL_UNUSED(argv);

    // duudeli(); return ESTATUS_SUCCESS;
    // tableview_test_start();

    /* Initialize/load user login data.
     */
    elogin_initialize();

    efsys_expose_directory("//fsys", eglobal->root_path, &fsys_thread_handle);

    egui_initialize();
    s = eimgui_initialize();
    if (s) {
        osal_debug_error("ImGui initialization failed");
        rval = ESTATUS_FAILED;
        goto getout;
    }

    enet_initialize_service();
    os_lock(); /* root pointer used */
    enet_add_protocol(new ecomProtocol(eglobal_root()));
    enet_add_protocol(new eioProtocol(eglobal_root()));
    os_unlock();
    enet_start_service(
        ENET_DEFAULT_NO_END_POINTS|
        ENET_ENABLE_IOCOM_CLIENT|
        ENET_ENABLE_EOBJECTS_CLIENT|
        ENET_ENABLE_IOCOM_SERVICE|
        ENET_ENABLE_LIGHTHOUSE|
        /* |ENET_ENABLE_EOBJECTS_SERVICE */
        ENET_ENABLE_SERIAL_COM|
        ENET_ENABLE_UNSECURED_SOCKETS);

    gui = new eGui(egui_get_container());
    gui->setup_desktop_application();
    gui->run();

    enet_stop_service();

    eimgui_shutdown();

getout:
    egui_shutdown();

    fsys_thread_handle.terminate();
    fsys_thread_handle.join();

    // tableview_test_end();

    return rval;
}

