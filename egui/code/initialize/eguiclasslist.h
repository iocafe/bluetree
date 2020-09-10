/**

  @file    eguiclasslist.h
  @brief   Class list of egui library.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    10.9.2020

  Maintain list of classes which can be created dynamically by class ID.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef EGUICLASSLIST_H_
#define EGUICLASSLIST_H_
#include "eobjects.h"

/* Initialize class list.
 */
void eguiclasslist_initialize();

/* Release resources allocated for the class list.
 */
void eguiclasslist_release();

#endif
