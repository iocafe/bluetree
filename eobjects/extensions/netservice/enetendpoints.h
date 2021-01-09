/**

  @file    enetendpoints.h
  @brief   End points to listen to.
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
#ifndef ENETENDPOINTS_H_
#define ENETENDPOINTS_H_
#include "extensions/netservice/enetservice.h"

/* End point table column names.
 */
extern const os_char enet_endp_enable[];
extern const os_char enet_endp_protocol[];
extern const os_char enet_endp_transport[];
extern const os_char enet_endp_port[];
extern const os_char enet_endp_netname[];

typedef enum {
    ENET_ENDP_PROTOCOL,
    ENET_ENDP_TLS_PORT,
    ENET_ENDP_TCP_PORT,
    ENET_ENDP_IPV6,
    ENET_ENDP_NETNAME,

    ENET_ENDP_TRANSPORT,
    ENET_ENDP_PORT,
    ENET_ENDP_PROTOCOL_HANDLE
}
enetEndpItemId;


typedef enum {
    ENET_ENDP_SOCKET_IPV4 = 1,
    ENET_ENDP_SOCKET_IPV6,
    ENET_ENDP_TLS_IPV4,
    ENET_ENDP_TLS_IPV6,
    ENET_ENDP_SERIAL
}
enetEndpTransportIx;

#endif
