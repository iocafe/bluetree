/**

  @file    eguioid.h
  @brief   Enumeration of egui object identifiers.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  This header file defines object identifiers used by egui library.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef EGUIOID_H_
#define EGUIOID_H_
#include "egui.h"

/**
****************************************************************************************************

  @name Object identifiers used by egui library.

  The object identifiers from -99 to -1 are reserved for eobject library. Idenfifiers from
  -199 to -100 are reserved for egui library. Idenfifiers from -1099 to -1000 are for special
  use markings.

****************************************************************************************************
*/

/** GUI root object container. The same process can run multiple GUI objects.
 */
#define EOID_GUI_CONTAINER -100

/* econnect library root object, provides API to IOCOM library.
 */
#define EOID_GUI_ECONNECT -101

/* Window and popup are handled differently from regular GUI components.
 */
#define EOID_GUI_WINDOW -102
#define EOID_GUI_POPUP -103
#define EOID_GUI_COMPONENT -104

/* Misc objects used by components
 */
#define EOID_GUI_SELECTED -110
#define EOID_GUI_TO_BE_DELETED -111


#endif
