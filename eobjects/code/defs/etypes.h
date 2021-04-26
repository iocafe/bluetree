/**

  @file    etypes.h
  @brief   Commonly used structures and related functions.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

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

/* Check if point x,y is within rectangle.
 */
os_boolean erect_is_xy_inside(
    eRect& r,
    os_int x,
    os_int y);

/* Check if point x,y is within rectangle.
 */
os_boolean erect_is_point_inside(
    eRect& r,
    ePos& p);

/* Make rectangle smaller by padding edges
 */
void erect_shrink(
    eRect& r,
    os_int n);

#endif
