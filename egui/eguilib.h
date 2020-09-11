/**

  @file    eguilib.h
  @brief   Main egui library header file.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  This egui library base main header file. If further includes rest of egui headers.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef EGUILIB_H_
#define EGUILIB_H_

#define EOID_ROOT 9999

/* Include freeglut header. Static link.
 */
// #define FREEGLUT_STATIC
// #include <GL/freeglut.h>

/* Include objects headers.
 */
#include "eobjects.h"

/* Initialize the egui library for use.
 */
void egui_initialize(
    void *reserved);

/* Shut down the egui library.
*/
void egui_shutdown();

/* Include Dear ImGUI header.
 */
#include "code/imgui/imgui.h"

/* Include egui headers.
 */
#include "code/defs/eguiclassid.h"
#include "code/defs/eguioid.h"
#include "code/initialize/eguiglobal.h"
#include "code/initialize/eguiclasslist.h"
#include "code/imgui_iface/imgui_iface.h"
#include "code/userinput/emouse.h"
#include "code/userinput/ekeyboard.h"
#include "code/component/ecomponent.h"
#include "code/gui/egui.h"


#endif
