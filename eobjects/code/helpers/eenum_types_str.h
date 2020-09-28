/**

  @file    eenum_types_str.h
  @brief   Create string to enumerate data types in "attr" property.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  The enumeration allows equi, etc, to show drop down list, etc, to select data type.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef EENUM_TYPES_STR_H_
#define EENUM_TYPES_STR_H_
#include "eobjects.h"

/* Get user interface string label for type number
 */
const os_char *etype_to_label(
    osalTypeId type_id);

/* Generate enumeration of types for "attr" property.
 */
void emake_type_enum_str(
    eVariable *str,
    os_boolean append_it = OS_FALSE);

#endif
