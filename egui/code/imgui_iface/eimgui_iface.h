/**

  @file    eimgui_iface.h
  @brief   ImGUI library initialization and shut down for egui.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  API for interfacing with imgui, mouse and keyboard and operating system windows.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef IMGUI_IFACE_H_
#define IMGUI_IFACE_H_
#include "egui.h"


/* For type checking
 */
typedef struct eViewPort
{
    int dulle;
}
eViewPort;


#define EIMGUI_LEFT_MOUSE_BUTTON ImGuiMouseButton_Left
#define EIMGUI_RIGHT_MOUSE_BUTTON ImGuiMouseButton_Right

/* Initialize ImGUI for use.
 */
eStatus eimgui_initialize();

/* Shut down ImGUI.
 */
void eimgui_shutdown();

/* Set operating system window title.
 */
void eimgui_set_window_title(
    os_char *title);


eViewPort *eimgui_open_viewport();
void egui_close_viewport(eViewPort *viewport);


eStatus eimgui_start_frame(eViewPort *viewport);

void eimgui_finish_frame(eViewPort *viewport);


#endif
