/**

  @file    eiocom.h
  @brief   iocom within eobjects.
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
#ifndef EIOCOM_H_
#define EIOCOM_H_
#include "extensions/netservice/enetservice.h"
// #include "extensions/eiocom/eio_root.h"
#include "extensions/eiocom/eprotocol_handle_iocom.h"
#include "extensions/eiocom/eprotocol_iocom.h"

class eNetService;

/**
****************************************************************************************************
  Defines, etc.
****************************************************************************************************
*/



/**
****************************************************************************************************
  Initialization, shutdown.
****************************************************************************************************
*/
#if 0

/* Initialize iocom data structures, etc.
 */
void eio_initialize(
    class eNetService *net_service);

/* Release resources allocated for iocom data structures.
 */
void eio_shutdown();

#endif

#endif
