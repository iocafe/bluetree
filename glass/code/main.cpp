/**

  @file    main.cpp
  @brief   Program entry point, eclient.
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


/* If needed for the operating system, EOSAL_C_MAIN macro generates the actual C main() function.
   and macro EMAIN_CONSOLE_ENTRY eobjects specific osal_main() function which calls emain.
 */
EOSAL_C_MAIN
EMAIN_CONSOLE_ENTRY("glass")

/**
****************************************************************************************************

  @brief The eclient program entry point.

  Initialize esal, eobjects and iocom. Start the eclient application.

  @oaran   argc Number of command line arguments (PC only)
  @oaran   argv Array of command line argument pointers (PC only)
  @return  ESTATUS_SUCCESS if all fine, other values indicate an error.

****************************************************************************************************
*/
eStatus emain(
    os_int argc,
    os_char *argv[])
{
    eThreadHandle client_thread_handle;
    eThreadHandle server_thread_handle;
    eThreadHandle fsys_thread_handle;
    eGui *gui;
    eStatus s;

//    duudeli(); return ESTATUS_SUCCESS;

    tableview_test_start();

    efsys_expose_directory("//fsys", OSAL_FS_ROOT "coderoot", &fsys_thread_handle);

    egui_initialize();
    s = eimgui_initialize();
    if (s) {
        osal_debug_error("ImGui initialization failed");
        egui_shutdown();
        return ESTATUS_FAILED;
    }

    enet_start_server(&server_thread_handle);
    enet_start_client(&client_thread_handle);

    gui = new eGui(egui_get_container());
    gui->setup_desktop_application();
    gui->run();

    server_thread_handle.terminate();
    server_thread_handle.join();
    client_thread_handle.terminate();
    client_thread_handle.join();

    eimgui_shutdown();
    egui_shutdown();

    fsys_thread_handle.terminate();
    fsys_thread_handle.join();

    tableview_test_end();

    return ESTATUS_SUCCESS;
}
