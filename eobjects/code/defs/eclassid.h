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

#define ECLASSID_OBJECT 20
#define ECLASSID_ROOT 21
#define ECLASSID_PROCESS 22
#define ECLASSID_THREAD 23
#define ECLASSID_THREAD_HANDLE 24
#define ECLASSID_QUEUE 27
#define ECLASSID_CONNECTION 30
#define ECLASSID_ENDPOINT 31
#define ECLASSID_TIMER 32
#define ECLASSID_TABLE 33
#define ECLASSID_WHERE 34
#define ECLASSID_BINDING 35
#define ECLASSID_SYNCHRONIZED 36
#define ECLASSID_SYNC_CONNECTOR 37

#define ECLASSID_STREAM 40
#define ECLASSID_BUFFERED_STREAM 41
#define ECLASSID_OSSTREAM 42

#define ECLASSID_FILE_SYSTEM 50
#define ECLASSID_NETSERVICE 51
#define ECLASSID_LIGHT_HOUSE_CLIENT 52
#define ECLASSID_NET_MAINTAIN_CLIENT 53
#define ECLASSID_PROTOCOL 54
#define ECLASSID_PROTOCOL_HANDLE 55
#define ECLASSID_ECOM_PROTOCOL 56


/* egui property numbers start from 64 */

/* First class id reserved for applications. All positive 32 bit integers
   starting from ECLASSID_APP_BASE can be used by application.
 */
#define ECLASSID_APP_BASE 1000


#endif
