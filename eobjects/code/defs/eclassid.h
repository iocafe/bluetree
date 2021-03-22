/**

  @file    eclassid.h
  @brief   Enumeration of class identifiers.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    14.9.2020

  This header file defines class identifiers used by eobjects library.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef ECLASSID_H_
#define ECLASSID_H_
#include "eobjects.h"

/**
****************************************************************************************************

  @name Class identifiers used by eobjects library.

  Each serializable eobjects library class has it's own class identifier. These may not be
  modified, would break serialization.

****************************************************************************************************
*/

#define ECLASSID_NONE 0
#define ECLASSID_CONTAINER 1
#define ECLASSID_VARIABLE 2
#define ECLASSID_NAME 3
#define ECLASSID_MATRIX 4
#define ECLASSID_ENVELOPE 5
#define ECLASSID_SET 6
#define ECLASSID_VALUEX 7
#define ECLASSID_PROPERTY_BINDING 8
#define ECLASSID_POINTER 9
#define ECLASSID_BUFFER 11
#define ECLASSID_ROWSET 12
#define ECLASSID_PERSISTENT 13
#define ECLASSID_ROW_SET_BINDING 14
#define ECLASSID_DBM 15
#define ECLASSID_NAMESPACE 16
#define ECLASSID_BITMAP 17

#define ECLASSID_OBJECT 20
#define ECLASSID_ROOT 21
#define ECLASSID_PROCESS 22
#define ECLASSID_THREAD 23
#define ECLASSID_THREAD_HANDLE 24
#define ECLASSID_QUEUE 25
#define ECLASSID_CONNECTION 26
#define ECLASSID_ENDPOINT 27
#define ECLASSID_TIMER 28
#define ECLASSID_TABLE 29
#define ECLASSID_WHERE 30
#define ECLASSID_BINDING 31
#define ECLASSID_SYNCHRONIZED 32
#define ECLASSID_SYNC_CONNECTOR 33

#define ECLASSID_STREAM 65
#define ECLASSID_BUFFERED_STREAM 66
#define ECLASSID_OSSTREAM 67

#define ECLASSID_PROTOCOL_HANDLE 70
#define ECLASSID_PROTOCOL 71
#define ECLASSID_ECOM_PROTOCOL_HANDLE 72
#define ECLASSID_ECOM_PROTOCOL 73
#define ECLASSID_IOCOM_PROTOCOL 74
#define ECLASSID_IOCOM_PROTOCOL_HANDLE 75
#define ECLASSID_SWITCHBOX_PROTOCOL 76
#define ECLASSID_SWITCHBOX_PROTOCOL_HANDLE 77

#define ECLASSID_EIO_ROOT 80
#define ECLASSID_EIO_NETWORK 81
#define ECLASSID_EIO_DEVICE 82
#define ECLASSID_EIO_MBLK 83
#define ECLASSID_EIO_GROUP 84
#define ECLASSID_EIO_VARIABLE 85
#define ECLASSID_EIO_SIGNAL 86
#define ECLASSID_EIO_ASSEMBLY 90
#define ECLASSID_EIO_BRICK_BUFFER 91
#define ECLASSID_EIO_SIGNAL_ASSEMBLY 92
#define ECLASSID_EIO_THREAD 99

#define ECLASSID_FILE_SYSTEM 100
#define ECLASSID_NETSERVICE 101
#define ECLASSID_LIGHT_HOUSE_CLIENT 102
#define ECLASSID_NET_MAINTAIN_CLIENT 103


/* egui property numbers start from 128 */

/* First class id reserved for applications. All positive 32 bit integers
   starting from ECLASSID_APP_BASE can be used by application.
 */
#define ECLASSID_APP_BASE 1000


#endif
