/**

  @file    eint2str.h
  @brief   Convert integer to string, options for date/time strings, etc.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    30.9.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eosal and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef EINT2STR_H_
#define EINT2STR_H_
#include "eobjects.h"

/* Convert integer to string, without terminating the string and padding option.
 */
os_char *eint2str(
    os_char *dst,
    os_long x,
    os_memsz max_chars,
    os_int min_width = 0,
    os_char pad_char = ' ');

#endif
