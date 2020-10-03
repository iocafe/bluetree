/**

  @file    eliststr_helpers.cpp
  @brief   Helper functions for list manipulation.
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


/* Append string item x to list. If list is not empty, separator is appended before x.
   If sep is OS_NULL, the "\n" separator is used.
 */
void eliststr_appeneds(
    eVariable *list,
    const os_char *x,
    const os_char *sep)
{
    if (sep == OS_NULL) sep = "\n";

    if (!list->isempty()) {
        list->appends(sep);
    }
    list->appends(x);
}

/* See eliststr_appened().
 */
void eliststr_appenedv(
    eVariable *list,
    eVariable *x,
    const os_char *sep)
{
    eliststr_appeneds(list, x->gets(), sep);
}
