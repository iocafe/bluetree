/**

  @file    eprotocol.cpp
  @brief   Abstract communication protocol as seen by eNetService.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  Related to: Network connnection and protocol management interface.

  Abstract communication protocol interface is used by eNetService to manage end points and
  connections. This is the base class, protocol specific derived class will map eNetService
  calls like "create end point" to communication library functions.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "extensions/netservice/enetservice.h"

/**
****************************************************************************************************
  Constructor.
****************************************************************************************************
*/
eProtocol::eProtocol(
    eObject *parent,
    e_oid oid,
    os_int flags)
    : eObject(parent, oid, flags)
{
}


/**
****************************************************************************************************

  @brief Check end point status.

  The is_end_point_running() function checks if a specific end point is running.

  @param   handle   End point handle as returned by new_end_point().
  @return  OS_TRUE if end point is running, OS_FALSE if not.

****************************************************************************************************
*/
os_boolean eProtocol::is_end_point_running(
    eProtocolHandle *handle)
{
    if (handle) {
        return handle->isrunning();
    }
    return OS_FALSE;
}


/**
****************************************************************************************************

  @brief Check connection status.

  The is_connection_running() function checks if a specific connection object exists.

  @param   handle   Connection handle as returned by new_connection().
  @return  OS_TRUE if end point is running, OS_FALSE if not.

****************************************************************************************************
*/
os_boolean eProtocol::is_connection_running(
    eProtocolHandle *handle)
{
    if (handle) {
        return handle->isrunning();
    }
    return OS_FALSE;
}

