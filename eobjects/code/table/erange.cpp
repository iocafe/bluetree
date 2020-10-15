/**

  @file    erange.cpp
  @brief   Index range in where clause string.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    10.10.2020

  Parse index range fron string, generate index range string.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "eobjects.h"

/**
****************************************************************************************************

  @brief Parse index range from beginning of where clause string.

  The e_parse_index_range() function....

  @param  whereclause String starting with index range followed by actual where clause.
  @param  minix Pointer to start index,
  @param  minix Pointer to end index,
  @return Number of bytes parsed for index range (rest is for the actual where clause).
          Value 0 indicates that nothing was parsed (range unspecified).

****************************************************************************************************
*/
os_memsz e_parse_index_range(
    const os_char *whereclause,
    os_long *minix,
    os_long *maxix)
{
    const os_char *p;
    os_memsz count;

    if (whereclause == OS_NULL) {
        osal_debug_error("NULL where clause");
        return 0;
    }

    p = whereclause;
    while (osal_char_isspace(*p)) p++;
    if (*(p++) != '[') return 0;
    *minix = osal_str_to_int(p, &count);
    if (count <= 0) return 0;
    p += count;
    while (osal_char_isspace(*p)) p++;
    if (*p == ',') p++;
    *maxix = osal_str_to_int(p, &count);
    if (count <= 0) {
        *maxix = *minix;
    }
    else {
        p += count;
    }
    while (osal_char_isspace(*p)) p++;
    if (*(p++) != ']') return 0;
    while (osal_char_isspace(*p)) p++;
    return p - whereclause;
}
