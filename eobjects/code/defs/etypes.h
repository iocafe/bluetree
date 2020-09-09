/**

  @file    etypes.h
  @brief   Commonly used data types and structure.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef ETYPES_H_
#define ETYPES_H_
#include "eobjects.h"

typedef struct eRect
{
    os_int x1, y1, x2, y2;
}
eRect;

typedef struct eSize
{
    os_int w, h;
}
eSize;

typedef struct ePos
{
    os_int x, y;
}
ePos;

#endif
