/**

  @file    eliststr_helpers.h
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
#pragma once
#ifndef ELISTSTR_HELPERS_H_
#define ELISTSTR_HELPERS_H_
#include "eobjects.h"

/* Append string item x to list. If list is not empty, separator is appended before x.
 */
void eliststr_appeneds(
    eVariable *list,
    const os_char *x,
    const os_char *sep = OS_NULL);

/* See eliststr_appened().
 */
void eliststr_appenedv(
    eVariable *list,
    eVariable *x,
    const os_char *sep = OS_NULL);

#endif
