/**

  @file    imguistyleserializer.h
  @brief   Save/load Dear ImGui style.
  @author  Many authors, MIT license
  @version 1.0
  @date    26.4.2021

****************************************************************************************************
*/
#pragma once
#ifndef IMGUISTYLESERIALIZER_H_
#define IMGUISTYLESERIALIZER_H_
#include "imgui.h"

void ImGuiSetuptyle( bool bStyleDark, float alpha);
bool ImGuiSaveStyle(const char* filename,const ImGuiStyle& style);
bool ImGuiLoadStyle(const char* filename,ImGuiStyle& style);

#endif
