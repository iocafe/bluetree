/**

  @file    eguiglobal.h
  @brief   Global structure of the egui library.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    10.9.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef EGUIGLOBAL_H_
#define EGUIGLOBAL_H_
#include "egui.h"

class ecRoot;

/**
****************************************************************************************************

  @brief Global structure.

  X...


****************************************************************************************************
*/
typedef struct eGuiGlobal
{
    /* Main guilib thread object.
     */
    eThread *guilib_thread;

    /* Container for eGui objects (multiple viewports in future?).
     */
    eContainer *gui_container;

    /* econnect library root object, API to IOCOM communication
     */
    ecRoot *econnect;
}
eGuiGlobal;

#endif
