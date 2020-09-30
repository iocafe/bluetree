/**

  @file    eint2str.cpp
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
#include "eobjects.h"


/**
****************************************************************************************************

  @brief Convert integer to string, without terminating the string and padding option.
  @anchor eint2str

  The eint2strstr() function converts an integer x to string and stores the resulting
  string into dst. Terminating '\0' is NOT stored.
  The resulting string can be padded to specific minimum width by setting "min_width" and
  "pad_char".
  If the buffer is too small to hold the resulting string, the function returns NULL.

  @param dst Pointer where to store the resulting characters.
  @param x Integer value to convert to string
  @param max_chars Maximum number of characters to store. This is to prevent buffer overflow.
  @param min_width Minimum number of characters in the resulting string. This is used to
         indicate zero or space padding to get fixed with string. Set 0 if padding is not needed.
  @param pad_char Pad character, typically zero '0' or space ' '.

  @return  Pointer to character position just after appended characters. OS_NULL if max_chars
           would have been exceeded.

****************************************************************************************************
*/
os_char *eint2str(
    os_char *dst,
    os_long x,
    os_memsz max_chars,
    os_int min_width,
    os_char pad_char)
{
    os_char nbuf[OSAL_NBUF_SZ], *e;
    os_int bytes, count, i;

    if (dst == OS_NULL) return OS_NULL;

    e = dst + max_chars;

    /* If we may be padding with something else but space, handle '-' sign first.
     */
    if (x < 0 && pad_char != ' ') {
        x = -x;
        max_chars--;
        min_width--;
        if (dst >= e) return OS_NULL;
        *(dst++) = '-';
    }

    bytes = (os_int)osal_int_to_str(nbuf, sizeof(nbuf), x) - 1;

    count = min_width - bytes;
    while (count-- > 0) {
        if (dst >= e) return OS_NULL;
        *(dst++) = pad_char;
    }

    for (i = 0; i<bytes; i++) {
        if (dst >= e) return OS_NULL;
        *(dst++) = nbuf[i];
    }

    return dst;
}
