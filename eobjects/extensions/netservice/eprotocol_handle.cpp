/**

  @file    eprotocolhandle.cpp
  @brief   Abstract communication protocol handle.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "extensions/netservice/enetservice.h"

/* Property names.
 */
const os_char
    eprohandp_isopen[] = "isopen";


/**
****************************************************************************************************
  Constructor.
****************************************************************************************************
*/
eProtocolHandle::eProtocolHandle(
    eObject *parent,
    e_oid oid,
    os_int flags)
    : eObject(parent, oid, flags)
{
}

