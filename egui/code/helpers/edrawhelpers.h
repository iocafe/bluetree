/**

  @file    edrawhelpers.h
  @brief   Helper functions for rendering components.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    15.9.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "egui.h"
#pragma once
#ifndef EDRAWHELPERS_H_
#define EDRAWHELPERS_H_
#include "egui.h"

#define EDRAW_VALUE_DEFAULT 0
#define EDRAW_VALUE_TABLE 1

#define EDRAW_TTIP_DEFAULT 0
#define EDRAW_TTIP_CELL_VALUE 1
#define EDRAW_TTIP_PATH 2
#define EDRAW_TTIP_IPATH 4

void edraw_value(
    eVariable *value,
    os_int state_bits,
    eComponent *compo, // for redirs, tool tip, etc
    eAttrBuffer& attr,
    os_int value_w = -1,
    eRect *r = OS_NULL,
    os_int dflags = EDRAW_VALUE_DEFAULT);

void edraw_tooltip(
    eObject *obj,
    eVariable *value,
    const os_char *otext,
    const os_char *flagstr,
    eAttrBuffer& oattr,
    os_int flags);

#endif
