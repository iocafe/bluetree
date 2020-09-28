/**

  @file    egui.h
  @brief   Main egui library header file.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    15.9.2020

  This egui library base main header file. If further includes rest of egui headers.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef EGUI_H_
#define EGUI_H_

#define EOID_ROOT 9999

/* Include library headers.
 */
#include "eobjects.h"
#include "econnect.h"

/* Initialize the egui library for use.
 */
void egui_initialize();

/* Shut down the egui library.
*/
void egui_shutdown();

/* Get main gui thread object.
 */
#define egui_get_thread() (eglobal->eguiglobal->guilib_thread)

/* Get GUI container which holds eGui objects if these are run by same thread.
 */
#define egui_get_container() (eglobal->eguiglobal->gui_container)

/* Get econnect object used for IOCOM connections.
 */
#define egui_get_econnect() (eglobal->eguiglobal->econnect)

/* Include Dear ImGUI header.
 */
#include "code/imgui/imgui.h"

/* Include egui headers.
 */
#include "code/defs/eguiclassid.h"
#include "code/defs/eguioid.h"
#include "code/initialize/eguiglobal.h"
#include "code/initialize/eguiclasslist.h"
#include "code/imgui_iface/eimgui_iface.h"
#include "code/userinput/emouse.h"
#include "code/userinput/ekeyboard.h"
#include "code/components/eautolabel.h"
#include "code/components/eeditbuffer.h"
#include "code/components/estrbuffer.h"
#include "code/components/eattrbuffer.h"
#include "code/components/ecomponent.h"
#include "code/components/window/ewindow.h"
#include "code/components/popup/epopup.h"
#include "code/components/lineedit/elineedit.h"
#include "code/components/button/ebutton.h"
#include "code/components/treenode/etreenode.h"

#include "code/ioc_components/eioc_checkbox.h"
#include "code/gui/eguiroot.h"


#endif
