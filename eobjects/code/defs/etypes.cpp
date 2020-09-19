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

bool erect_is_point_inside(
    eRect *r,
    os_int x,
    os_int y)
{
    if (r) {
        if (x >= r->x1 && x <= r->x2) {
            return (y >= r->y1 && y <= r->y2);
        }
    }

    return false;
}

