/**

  @file    eguiclassid.h
  @brief   Enumeration of egui class identifiers.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    14.9.2020

  This header file defines class identifiers used by egui library.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef EGUICLASSID_H_
#define EGUICLASSID_H_
#include "egui.h"


/**
****************************************************************************************************

  @name Class identifiers used by egui library.

  Each serializable eobjects library class has it's own class identifier. These may not be
  modified, would break serialization.

****************************************************************************************************
*/

#define ECLASSID_EGUI_BASE 128

#define EGUICLASSID_MOUSE_MESSAGE       (ECLASSID_EGUI_BASE + 1)
#define EGUICLASSID_KEYBOARD_MESSAGE    (ECLASSID_EGUI_BASE + 2)
#define EGUICLASSID_GUI                 (ECLASSID_EGUI_BASE + 3)
#define EGUICLASSID_TABLE_COLUMN        (ECLASSID_EGUI_BASE + 4)

#define EGUICLASSID_BEGIN_COMPONENTS    (ECLASSID_EGUI_BASE + 10)
#define EGUICLASSID_COMPONENT           (EGUICLASSID_BEGIN_COMPONENTS + 0)
#define EGUICLASSID_WINDOW              (EGUICLASSID_BEGIN_COMPONENTS + 1)
#define EGUICLASSID_POPUP               (EGUICLASSID_BEGIN_COMPONENTS + 2)
#define EGUICLASSID_LINE_EDIT           (EGUICLASSID_BEGIN_COMPONENTS + 3)
#define EGUICLASSID_TREE_NODE           (EGUICLASSID_BEGIN_COMPONENTS + 4)
#define EGUICLASSID_BUTTON              (EGUICLASSID_BEGIN_COMPONENTS + 5)
#define EGUICLASSID_TABLE_VIEW          (EGUICLASSID_BEGIN_COMPONENTS + 6)
#define EGUICLASSID_PARAMETER_LIST      (EGUICLASSID_BEGIN_COMPONENTS + 7)
#define EGUICLASSID_CAMERA_VIEW         (EGUICLASSID_BEGIN_COMPONENTS + 8)

#define EGUICLASSID_GAME_CONTROLLER     (EGUICLASSID_BEGIN_COMPONENTS + 20)

#define EGUICLASSID_LOGIN_DIALOG        (EGUICLASSID_BEGIN_COMPONENTS + 50)

// #define EGUICLASSID_IOC_CHECKBOX        (EGUICLASSID_BEGIN_COMPONENTS + 40)
#define EGUICLASSID_END_COMPONENTS      (EGUICLASSID_BEGIN_COMPONENTS + 80)

#define EGUICLASSID_IS_COMPONENT(cid) \
    ((cid) >= EGUICLASSID_BEGIN_COMPONENTS && (cid) <= EGUICLASSID_END_COMPONENTS)

/* FOR 3D extension
#define ECLASSID_EGUI_3D_BASE 196
#define EGUICLASSID_OBJECT3D       (ECLASSID_EGUI_3D_BASE + 1)
#define EGUICLASSID_MOVINGOBJECT3D (ECLASSID_EGUI_3D_BASE + 2)
#define EGUICLASSID_MESH3D         (ECLASSID_EGUI_3D_BASE + 3)
#define EGUICLASSID_TERRAIN3D      (ECLASSID_EGUI_3D_BASE + 4)
#define EGUICLASSID_WORLD3D        (ECLASSID_EGUI_3D_BASE + 5)
*/

#endif
