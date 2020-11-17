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

  @param  process_name Application name, important. This is used by communication to identify this
          application. Choose unique short name without special characters.
          Only 'a' - 'z' and 'A' - 'Z' characters. Max 15 characters + terminating '\0'.
  @param  flags EOBJECTS_DEFAULT_INIT (0) Default initialization.
          Flag EOBJECTS_NO_NETWORK_INIT disables network and iocom initialization. This
          can be used if eosal network is initialized beforehand. Not recommended, gets complex.
  @return None.

****************************************************************************************************
*/
void eobjects_initialize(
    const os_char *process_name,
    os_int flags)
{
    osalSecurityConfig security;
    osPersistentParams persistentprm;

    /* Do nothing if the library has been initialized.
     */
    if (eglobal->initialized) return;

    /* Clear the global strcture, mark initialized and save process name.
     */
    os_memclear(eglobal, sizeof(eGlobal));
    eglobal->initialized = OS_TRUE;
    os_strncpy(eglobal->process_name, process_name, ENET_PROCESS_NAME_SZ);

    /* Initialize handle tables.
     */
    ehandleroot_initialize();

    /* Initialize class list
     */
    eclasslist_initialize();

    /* Initialize network
     */
    if ((flags & EOBJECTS_NO_NETWORK_INIT) == 0)
    {
        /* Setup error handling. Here we select to keep track of network state. We could also
           set application specific error handler callback by calling osal_set_error_handler().
         */
        osal_initialize_net_state();

        /* Initialize persistent storage (typically flash is running in micro-controller)
         */
        os_memclear(&persistentprm, sizeof(persistentprm));
        persistentprm.device_name = process_name;
        os_persistent_initialze(&persistentprm);

#if OSAL_TLS_SUPPORT
        os_memclear(&security, sizeof(security));
        // security->certs_dir
        osal_tls_initialize(OS_NULL, 0, OS_NULL, 0, &security);
#else
        osal_socket_initialize(OS_NULL, 0, OS_NULL, 0);
#endif
#if OSAL_SERIAL_SUPPORT
        osal_serial_initialize();
#endif
    }

    /* Save initialization flags for eobjects_shutdown()
     */
    eglobal->eobjects_init_flags = flags;
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

    if ((eglobal->eobjects_init_flags & EOBJECTS_NO_NETWORK_INIT) == 0)
    {
#if OSAL_TLS_SUPPORT
        osal_tls_shutdown();
#else
        osal_socket_shutdown();
#endif

#if OSAL_SERIAL_SUPPORT
        osal_serial_shutdown();
#endif
        os_persistent_shutdown();
    }

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

