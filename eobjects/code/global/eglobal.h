/**

  @file    eglobal.h
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
#pragma once
#ifndef EGLOBAL_H_
#define EGLOBAL_H_
#include "eobjects.h"

class eThreadHandle;
class eConsole;
class eNetService;

/* Space allocation for process name, nr, id, etc. strings.
 */
#define EGLOBAL_PROCESS_NAME_SZ 16
#define EGLOBAL_PROCESS_ID_SZ 32

/* Space allocation for absolute and relative paths.
 */
#ifndef EGLOBAL_PATH_SZ
#define EGLOBAL_PATH_SZ 128
#endif
#ifndef EGLOBAL_RELATIVE_PATH_SZ
#define EGLOBAL_RELATIVE_PATH_SZ 64
#endif


/**
****************************************************************************************************

  @brief Global structure.

  A flag C structure for global parameters. Excluding pointers, these are not modified at run
  time and can be accessed without synchronization.

****************************************************************************************************
*/
typedef struct eGlobal
{
    /** Flags indicating that eobjects library has been initialized.
     */
    os_boolean initialized;

    eHandleRoot hroot;

    /** Root container for global objects.
     */
    eContainer *root;

    /** Empty variable which does not belong to any thread.
     */
    eVariable *empty;

    /** Container for class list.
     */
    eContainer *classlist;

    /** Container for property sets.
     */
    eContainer *propertysets;

    /** Pointer to process thread handle.
     */
    eThreadHandle *processhandle;

    /** Pointer to process object.
     */
    eProcess *process;

    /** Process name space
     */
    eNameSpace *process_ns;

    /** Pointer to timer thread handle
     */
    eThreadHandle *timerhandle;

    /** Network service object (netservice extension). OS_NULL if not used.
     */
    eNetService *netservice;

    /** Name of the process, like "grumpy".
     */
    os_char process_name[EGLOBAL_PROCESS_NAME_SZ];

    /** Process idenfification number (device number, can be serial number).
     */
    os_int process_nr;

    /** Process identification, process name and identification name together
     */
    os_char process_id[EGLOBAL_PROCESS_ID_SZ];

    /** Path to root data directory (operating system path).
     */
    os_char root_path[EGLOBAL_PATH_SZ];

    /** Static application specific file directory. Relative within root directory.
        This doesn't contain product number.
     */
    os_char app_static_dir[EGLOBAL_RELATIVE_PATH_SZ];

    /** Application's dynamic data directory (relative within root directory).
        This contains application name and product number.
     */
    os_char data_dir[EGLOBAL_RELATIVE_PATH_SZ];

    /** Executables, operating system path to the directory.
     */
    os_char bin_path[EGLOBAL_PATH_SZ];

    /** This is special for eMatrix clas. Memory manager and eBuffer implementation
        are tested at startup, how much memory we actually get when we request
        OEMATRIX_APPROX_BUF_SZ bytes.
     */
    os_int matrix_buffer_allocation_sz;

    /** Pointer to global structure for egui library.
     */
    struct eGuiGlobal *eguiglobal;

    /** Initialization flags saved by eobjects_inititalize() for eobjects_shutdown()
     */
    os_int eobjects_init_flags;
}
eGlobal;

/* Global pointer to global structure.
 */
extern eGlobal *eglobal;

inline eNameSpace *eglobal_process_ns()
{
    osal_debug_assert(eglobal->process_ns);
    return eglobal->process_ns;
}

/* os_lock() must be called when using the returned pointer.
 */
inline eContainer *eglobal_root()
{
    osal_debug_assert(eglobal->root);
    return eglobal->root;
}

/* THIS SHOULD BE AS FAST FUNCTION AS POSSIBLE
 */
inline eHandle *eget_handle(
    e_oix oix)
{
    return eglobal->hroot.m_table[oix >> EHANDLE_HANDLE_BITS]->m_handle + (oix & EHANDLE_TABLE_MASK);
}


/* Initialize flat global structure.
 */
void eglobal_initialize(
    const os_char *process_name,
    os_int argc,
    os_char *argv[]);

/* Generate full operating system path to data file.
 */
void eglobal_make_full_data_file_path(
    const os_char *file_name,
    eVariable *full_path);

#endif
