/**

  @file    einitialize.cpp
  @brief   Initialize and shut down eobjects library.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "eobjects.h"


/**
****************************************************************************************************

  @brief Initialize eobject library for use.
  @anchor eobjects_initialize

  The eobjects_initialize() function initializes eobjects library for use. This function should
  be the first eobjects function called. The eobjects_shutdown() function cleans up resources
  used by the library.

  @param  reserved For now OS_NULL.
  @return None.

****************************************************************************************************
*/
void eobjects_initialize(
    void *reserved)
{
    /* Do nothing if the library has been initialized.
     */
    if (eglobal->initialized) return;

    /* Clear the global strcture and mark initialized.
     */
    os_memclear(eglobal, sizeof(eGlobal));
    eglobal->initialized = OS_TRUE;

    /* Initialize handle tables.
     */
    ehandleroot_initialize();

    /* Initialize class list
     */
    eclasslist_initialize();
}


/**
****************************************************************************************************

  @brief Shut down eobjects library.
  @anchor eobjects_shutdown

  The eobjects_shutdown() function cleans up resources used by the eobjects library.

  @return  None.

****************************************************************************************************
*/
void eobjects_shutdown()
{
    /* Do nothing if the library has not been initialized.
     */
    if (!eglobal->initialized) return;

    /* Release resources allocated for the class list.
     */
    eclasslist_release();

    /* Delete handle tables.
     */
    ehandleroot_shutdown();

    /* Mark eobjects library uninitialized.
     */
    eglobal->initialized = OS_FALSE;
}

