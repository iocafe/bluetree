/**

  @file    ecommands.h
  @brief   Enumeration of command identifiers.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    14.9.2020

  This header file defines command identifiers used by eobjects library.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef ECOMMANDS_H_
#define ECOMMANDS_H_
#include "eobjects.h"


/**
****************************************************************************************************

  @name Command identifiers for messages.

  Commend identifiers used in envelopes and in by message(), onmessage(), etc. functions.
  All command identifiers used by eobjects library are negative, range -1 ... -999 is reserved
  for these. Application may use positive command identifiers or negative command identifiers
  smaller than -999.

****************************************************************************************************
*/

/* Messaging, no target.
 */
#define ECMD_NO_TARGET -1

/* Generic error.
 */
#define ECMD_ERROR -2

/* Interrupt current operation.
 */
#define ECMD_INTERRUPT -3

/* Set property by message
 */
#define ECMD_SETPROPERTY -19

/* Binding related commands.
 */
#define ECMD_BIND -20
#define ECMD_BIND_RS -21
#define ECMD_BIND_REPLY -22
#define ECMD_UNBIND -23
#define ECMD_SRV_UNBIND -24
#define ECMD_REBIND -25
#define ECMD_FWRD -26
#define ECMD_ACK -27

/* Tables.
 */
#define ECMD_CONFIGURE_TABLE -30
#define ECMD_INSERT_ROWS_TO_TABLE -31
#define ECMD_REMOVE_ROWS_FROM_TABLE -32
#define ECMD_UPDATE_TABLE_ROWS -33

/* Timer commands (timer hit and set timer period).
 */
#define ECMD_TIMER -50
#define ECMD_SETTIMER -51

/* Browsing object tree.
 */
#define ECMD_INFO_REQUEST -60
#define ECMD_INFO_REPLY -61

/* Thread control, exit thread.
 */
#define ECMD_EXIT_THREAD -999


#endif
