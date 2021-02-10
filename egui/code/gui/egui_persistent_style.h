/**

  @file    egui_persistent_style.h
  @brief   Save/load Dear ImGui style.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  ADOPTED FROM

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef EGUI_PERSISTENT_STYLE_H_
#define EGUI_PERSISTENT_STYLE_H_
#include "egui.h"

void ImGuiSetuptyle( bool bStyleDark_, float alpha_  );
bool ImGuiSaveStyle(const char* filename,const ImGuiStyle& style);
bool ImGuiLoadStyle(const char* filename,ImGuiStyle& style);

#endif
