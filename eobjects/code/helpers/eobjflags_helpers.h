/**

  @file    eobjflags_helpers.h
  @brief   Convert various flags to strings.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    2.1.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef EOBJFLAGS_HELPERS_H_
#define EOBJFLAGS_HELPERS_H_
#include "eobjects.h"

/* Convert eObject flags to string.
 */
void eobjflags_to_str(
    eVariable *strvar,
    os_uint flags);

#endif
