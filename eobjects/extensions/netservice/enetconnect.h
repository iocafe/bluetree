/**

  @file    enetconnect.h
  @brief   Connections to establish.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef ENETCONNECT_H_
#define ENETCONNECT_H_
#include "extensions/netservice/enetservice.h"

/* "connect to" table column names.
 */
extern const os_char enet_conn_enable[];
extern const os_char enet_conn_name[];
extern const os_char enet_conn_protocol[];
extern const os_char enet_conn_ip[];
extern const os_char enet_conn_transport[];


typedef enum {
    ENET_CONN_PROTOCOL,
    ENET_CONN_NAME,
    ENET_CONN_TRANSPORT,
    ENET_CONN_IP,

    ENET_CONN_PROTOCOL_HANDLE
}
enetConnItemId;

typedef enum {
    ENET_CONN_SOCKET = 1,
    ENET_CONN_TLS = 2,
    ENET_CONN_SERIAL = 3,
}
enetConnTransportIx;




#endif
