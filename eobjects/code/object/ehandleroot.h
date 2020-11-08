/**

  @file    ehandleroot.h
  @brief   Global handle manager object.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  The handle root keeps track of handle tables and global free handles in them. Handles
  can be reserved by thread or an another root object. Handle root state is stored in
  eHandleRoot structure within eglobals.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef EHANDLEROOT_H_
#define EHANDLEROOT_H_
#include "eobjects.h"

class eHandleTable;

/** Maximum index for eHandleRoot's m_table array.
 */
#define EHANDLE_MAX_NRO_HANDLE_TABLES 0x2000

/**
****************************************************************************************************
  Handle root class.

  The handle eHandleRoot object manages holds pointers to allocated handle tables and maintains
  list of free handles, which are not reserved for any root object. There is one handle root
  object per process.
****************************************************************************************************
*/
typedef struct eHandleRoot
{
    /** Array of handle table pointers. Index is from 0 to EHANDLE_MAX_NRO_HANDLE_TABLES - 1.
     */
    eHandleTable *m_table[EHANDLE_MAX_NRO_HANDLE_TABLES];

    /** Number of allocated handle tables.
     */
    os_int m_nrotables;

    /** First free common handle (not reserved for any root object) in handle tables.
     */
    eHandle *m_first_free;
}
eHandleRoot;


/**
****************************************************************************************************
  Handle root functions.

  Function of the handle root is to allocate and manage handle tables, and allow (thread) root
  objects to reserve/release handles for use.
****************************************************************************************************
*/
/* Initialize handle tables.
 */
void ehandleroot_initialize();

/* Delete handle tables.
 */
void ehandleroot_shutdown();

/* Reserve handles for thread or another root object.
 */
eHandle *ehandleroot_reservehandles(
    os_int nro_handles);

/* Release handles from thread or another root object.
 */
eHandle *ehandleroot_releasehandles(
    eHandle *h,
    os_int nro_handles);

#endif
