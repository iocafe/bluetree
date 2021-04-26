/**

  @file    enicevalue.h
  @brief   Format value nicely for UI display.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "egui.h"
#pragma once
#ifndef ENICEVALUE_H_
#define ENICEVALUE_H_
#include "egui.h"

/* Format value nicely for UI display.
 */
void enice_value_for_ui(
    eVariable *value,
    eObject *obj,
    eAttrBuffer *attr);

void enice_ui_value_to_internal_type(
    eVariable *value,
    eVariable *new_value,
    eObject *obj,
    eAttrBuffer *attr);

#endif
