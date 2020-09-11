/**

  @file    imgui_iface.h
  @brief   ImGUI library initialization and shut down for egui.
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
#ifndef IMGUI_IFACE_H_
#define IMGUI_IFACE_H_
#include "eguilib.h"

/* Initialize ImGUI for use.
 */
eStatus egui_initialize_imgui();

/* Shut down ImGUI.
 */
void egui_shutdown_imgui();

#endif
