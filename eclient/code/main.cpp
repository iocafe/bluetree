/**

  @file    main.cpp
  @brief   Program entry point, eclient.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    2.8.2020

  Code here is general program setup code. It initializes iocom library to be used as automation
  device controller. This example code uses eosal functions everywhere, including the program
  entry point osal_main(). If you use iocom library from an existing program, just call library
  iocom functions from C or C++ code and ignore "framework style" code here.

  The Buster conroller example here uses static IO device configuration. This means that
  communication signal map from IO board JSON files, etc, is compiled into Buster's code ->
  run time matching IO signal at IO device and in Buster is by address and type, not by signal name.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "eclient.h"

int duudeli(void);


/* If needed for the operating system, EOSAL_C_MAIN macro generates the actual C main() function.
   and macro EMAIN_CONSOLE_ENTRY eobjects specific osal_main() function which calls emain.
 */
EOSAL_C_MAIN
EMAIN_CONSOLE_ENTRY

/**
****************************************************************************************************

  @brief The controller program entry point.

  Initialize IOCOM and start the IO controller application.

  - osal_simulated_loop: When emulating micro-controller on PC, run loop. Just save context
    pointer on real micro-controller.

  @oaran   argc Number of command line arguments (PC only)
  @oaran   argv Array of command line argument pointers (PC only)
  @return  ESTATUS_SUCCESS if all fine, other values indicate an error.

****************************************************************************************************
*/
eStatus emain(
    os_int argc,
    os_char *argv[])
{
    eGui *gui;
    eThreadHandle client_thread_handle;
    eStatus s;

//    duudeli(); return OSAL_SUCCESS;

    egui_initialize();
    s = eimgui_initialize();
    if (s) {
        osal_debug_error("ImGui initialization failed");
        egui_shutdown();
        return ESTATUS_FAILED;
    }

    /* Manage network connections.
     */
    enet_start_client(&client_thread_handle);

    gui = new eGui(egui_get_container());
    gui->setup_desktop_application();
    gui->run();

    client_thread_handle.terminate();
    client_thread_handle.join();

    eimgui_shutdown();
    egui_shutdown();

    return ESTATUS_SUCCESS;
}
