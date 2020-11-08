/**

  @file    ehandletable.h
  @brief   Handle table
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  The handle table contains always EHANDLE_TABLE_LEN handles. Each handles is either in
  linked list of global friee handles, or reserved by thread.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef EHANDLETABLE_H_
#define EHANDLETABLE_H_
#include "eobjects.h"

/** Number of bits in object index specifying handle index. This and EHANDLE_TABLE_LEN must match.
 */
#define EHANDLE_HANDLE_BITS 14

/** Maximum index for eHandle's m_handle array. s
 */
#define EHANDLE_TABLE_LEN 0x4000

/** EHANDLE_TABLE_MASK, EHANDLE_TABLE_LEN and EHANDLE_HANDLE_BITS must match to each others.
 */
#define EHANDLE_TABLE_MASK 0x3FFF


/**
****************************************************************************************************
  Handle table class.

  The eHandleTable contains fixed length array of eHandle object.
****************************************************************************************************
*/
class eHandleTable
{
public:
    eHandleTable(e_oix oix);

    inline eHandle *firsthandle() {return m_handle;}

    /** Handle array.
     */
    eHandle m_handle[EHANDLE_TABLE_LEN];
};

#endif

