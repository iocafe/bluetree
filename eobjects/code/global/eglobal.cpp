/**

  @file    eglobal.cpp
  @brief   Global structure.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    14.9.2020

  All global variables are placed in global structure, so it is eanough to pass to eglobal pointer
  to Windows DLL to share eccess to globals.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "eobjects.h"

/* Static structure for global variables.
 */
static eGlobal eglobal_buf;

/* Public pointer to global variable structure.
 */
eGlobal *eglobal = &eglobal_buf;

/* Default root directory.
 */
#ifndef EGLOBAL_ROOT_PATH
#define EGLOBAL_ROOT_PATH OSAL_FS_ROOT "coderoot/fsys"
#endif

/* Directory containing static application files.
 */
#ifndef EGLOBAL_APP_STATIC_DIR
#define EGLOBAL_APP_STATIC_DIR "apps/"
#endif

/* Default executable directory.
 */
#ifndef EGLOBAL_BIN_DIR
#define EGLOBAL_BIN_DIR OSAL_FS_ROOT "coderoot/bin/" OSAL_BIN_NAME
#endif

/* Default data directory.
 */
#ifndef EGLOBAL_DATA_DIR
#define EGLOBAL_DATA_DIR "data/"
#endif


/**
****************************************************************************************************

  @brief Initialize flat global structure.

  The eglobal_initialize function stores process name and identification number into
  global structure and sets up directory paths.

  @param  process_name Process name, like "grumpy".
  @param  process_nr Process identification number. Identifies the instances of the executable.
          Can be serial number or short string. Foe example "10" could identify the process
          as "grumpy_10".
  @return None.

****************************************************************************************************
*/
void eglobal_initialize(
    const os_char *process_name,
    os_int argc,
    os_char *argv[])
{
    const os_char *process_nr = OS_NULL;
    os_int i;

    /* Set default paths.
     */
    os_strncpy(eglobal->root_path, EGLOBAL_ROOT_PATH, EGLOBAL_PATH_SZ);
    os_strncpy(eglobal->app_static_dir, EGLOBAL_APP_STATIC_DIR, EGLOBAL_RELATIVE_PATH_SZ);
    os_strncat(eglobal->app_static_dir, process_name, EGLOBAL_RELATIVE_PATH_SZ);
    os_strncpy(eglobal->data_dir, EGLOBAL_DATA_DIR, EGLOBAL_RELATIVE_PATH_SZ);
    os_strncat(eglobal->data_dir, process_name, EGLOBAL_RELATIVE_PATH_SZ);
    os_strncat(eglobal->data_dir, process_nr, EGLOBAL_RELATIVE_PATH_SZ);
    os_strncpy(eglobal->bin_path, EGLOBAL_BIN_DIR, EGLOBAL_PATH_SZ);

    /* Check if we have process number or path modifiers.
     */
    for (i = 1; i < argc; i++) {
        if (!os_strncmp(argv[i], "-n=", 3)) {
            if (osal_char_isdigit(argv[i][3])) {
                process_nr = argv[i] + 3;
            }
        }
        else if (!os_strncmp(argv[i], "-p=", 3)) {
            os_strncpy(eglobal->root_path, argv[i], EGLOBAL_PATH_SZ);
        }
        else if (!os_strncmp(argv[i], "-b=", 3)) {
            os_strncpy(eglobal->bin_path, argv[i], EGLOBAL_PATH_SZ);
        }
    }

    /* Save process identification in global flat structure, so synchronization
       is not needed to access these.
     */
    os_strncpy(eglobal->process_name, process_name, EGLOBAL_PROCESS_NAME_SZ);
    os_strncpy(eglobal->process_nr, process_nr, EGLOBAL_PROCESS_NR_SZ);
    os_strncpy(eglobal->process_id, process_name, EGLOBAL_PROCESS_ID_SZ);
    os_strncat(eglobal->process_id, process_nr, EGLOBAL_PROCESS_ID_SZ);

}
