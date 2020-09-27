/**

  @file    eguiinitialize.cpp
  @brief   Initialize and shut down egui library.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "egui.h"


/**
****************************************************************************************************

  @brief Initialize eguilibrary for use.
  @anchor egui_initialize

  The egui_initialize() function initializes egui library for use. This function should
  be the first egui function called. The egui_shutdown() function cleans up resources
  used by the library.

  @return None.

****************************************************************************************************
*/
void egui_initialize()
{
    eGuiGlobal *eguiglobal;
    eThread *thread;

    /* Do nothing if the library has been initialized.
     */
    if (eglobal->eguiglobal) return;

   /* Clear the global strcture and mark initialized.
     */
    eguiglobal = (eGuiGlobal*)os_malloc(sizeof(eGuiGlobal), OS_NULL);
    eglobal->eguiglobal = eguiglobal;
    os_memclear(eguiglobal, sizeof(eGuiGlobal));

    thread = new eThread();
    eguiglobal->guilib_thread = thread;
    eguiglobal->gui_container = new eContainer(thread, EOID_GUI_CONTAINER);

    /* Initialize class list
     */
    eguiclasslist_initialize();

    /* Create econnect object to access IOCOM.
     */
    // eguiglobal->econnect = new ecRoot(thread, EOID_GUI_ECONNECT);
}


/**
****************************************************************************************************

  @brief Shut down egui library.
  @anchor egui_shutdown

  The egui_shutdown() function cleans up resources used by the egui library.

  @return  None.

****************************************************************************************************
*/
void egui_shutdown()
{
    /* Do nothing if the library has not been initialized.
     */
    if (eglobal->eguiglobal == OS_NULL) return;

    /* Release resources allocated for the class list.
     */
    eguiclasslist_release();

    /* Clean up
     */
    delete egui_get_container();
    delete egui_get_thread();

    os_free(eglobal->eguiglobal, sizeof(eGuiGlobal));
    eglobal->eguiglobal = OS_NULL;
}

