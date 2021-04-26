/**

  @file    ehandletable.cpp
  @brief   Handle table
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

  The handle table contains always EHANDLE_TABLE_LEN handles. Each handles is either in
  linked list of global friee handles, or reserved by thread.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "eobjects.h"


/**
****************************************************************************************************

  @brief Object constructor.

  See cache_aligned_allocator for possible future optimization. Practically L1 cache line
  is 64 bytes. If m_handle table starts at memory address dividable by 64, things should be
  a bit faster. Check cache_aligned_allocator.

  @param oix First object index.

  The eHandleTable constructor creates an empty table in which all handles are linked in order
  for be used as initial free chain.

****************************************************************************************************
*/
eHandleTable::eHandleTable(
    e_oix oix)
{
    eHandle
        *h;

    os_int
        count;

    h = m_handle;

    /* First N - 1 */
    count = EHANDLE_TABLE_LEN - 1;
    while (--count)
    {
        h->m_right = h + 1;
        h->m_oix = oix++;
        h->m_ucnt = 0;
        h++;
    }

    /* Last */
    h->m_right = OS_NULL;
    h->m_oix = oix;
    h->m_ucnt = 0;
}
