/**

  @file    erange.h
  @brief   Index range in where clause string.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

  Parse index range fron string, generate index range string.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef ERANGE_H_
#define ERANGE_H_
#include "eobjects.h"

/* Parse index range from beginning of where clause string.
 */
os_memsz e_parse_index_range(
    const os_char *whereclause,
    os_long *minix,
    os_long *maxix);


#endif
