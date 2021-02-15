/**

  @file    eimgui_iface.h
  @brief   API to access graphics backend from egui code.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  API for interfacing with imgui, mouse and keyboard, operating system windows, graphics rendering
  pipeline, etc.

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


/* Constant EIMGUI_LEFT_MOUSE_BUTTON and ImGuiMouseButton_Left are 0, and promiced by ImGui to remain so.
   We use the same indexed, so defined here as numbers. Same goes for EIMGUI_RIGHT_MOUSE_BUTTON and
   ImGuiMouseButton_Right, these are 1. This software doesn't support middle mouse button (it supports
   mouse wheel), so number of buttons for this software is defined as 2.
 */
#define EIMGUI_LEFT_MOUSE_BUTTON 0
#define EIMGUI_RIGHT_MOUSE_BUTTON 1
#define EIMGUI_NRO_MOUSE_BUTTONS 2


/**
****************************************************************************************************
  Initialization
****************************************************************************************************
*/

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

/**
****************************************************************************************************
  Viewports
****************************************************************************************************
*/

/* For type checking
 */
typedef struct eViewPort
{
    int dulle;
}
eViewPort;

eViewPort *eimgui_open_viewport();
void egui_close_viewport(eViewPort *viewport);


/**
****************************************************************************************************
  Rendering
****************************************************************************************************
*/

eStatus eimgui_start_frame(eViewPort *viewport);

void eimgui_finish_frame(eViewPort *viewport);


/**
****************************************************************************************************
  Textures
****************************************************************************************************
*/

/* Load a bitmap to graphics card (as texture).
 */
eStatus eimgui_upload_texture_to_grahics_card(
    const os_uchar *bitmap_data,
    os_int bitmap_width,
    os_int bitmap_height,
    osalBitmapFormat bitmap_format,
    os_int byte_width,
    ImTextureID *textureID);

/* Delete a texture (bitmap) from graphics card.
 */
void eimgui_delete_texture_on_grahics_card(
    ImTextureID textureID);

#endif
