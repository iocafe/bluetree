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
/*@{*/

/* Messaging, no target.
 */
#define ECMD_NO_TARGET -1

/* Set property by message
 */
#define ECMD_SETPROPERTY -19

/* Binding related commands.
 */
#define ECMD_BIND -20
#define ECMD_BIND_REPLY -21
#define ECMD_UNBIND -22
#define ECMD_SRV_UNBIND -23
#define ECMD_REBIND -24
#define ECMD_FWRD -25
#define ECMD_ACK -26

/* Tables.
 */
#define ECMD_CONFIGURE_TABLE -30

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


/*@}*/


#endif
