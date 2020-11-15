/**

  @file    etypes.cpp
  @brief   Commonly used structures and related functions.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "eobjects.h"

os_boolean erect_is_xy_inside(
    eRect& r,
    os_int x,
    os_int y)
{
    if (x >= r.x1 && x <= r.x2) {
        return (os_boolean)(y >= r.y1 && y <= r.y2);
    }
    return OS_FALSE;
}

os_boolean erect_is_point_inside(
    eRect& r,
    ePos& p)
{
    return erect_is_xy_inside(r, p.x, p.y);
}

void erect_shrink(
    eRect& r,
    os_int n)
{
    r.x1 += n;
    r.y1 += n;
    r.x2 -= n;
    r.y2 -= n;
}

