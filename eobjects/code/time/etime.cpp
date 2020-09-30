/**

  @file    etime.cpp
  @brief   Date and time.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    29.9.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "eobjects.h"

/* Convert UTC to local time.
 */
eStatus elocaltime(
    eLocalTime *local_time,
    os_long utc,
    eObject *tzone)
{
    return ESTATUS_SUCCESS;
}

/* Convert local time to UTC.
 */
eStatus emktime(
    os_long *utc,
    eLocalTime *local_time,
    eObject *tzone)
{
    return ESTATUS_SUCCESS;
}

/* Generate time string from local time structure.
 */
eStatus etime_make_str(
    eLocalTime *local_time,
    os_char *buf,
    os_memsz buf_sz,
    eDateTimeFormat *format,
    os_int flags)
{
    return ESTATUS_SUCCESS;
}

/* Parse time string into local time structure.
 */
eStatus etime_parse_str(
    eLocalTime *local_time,
    const os_char *str,
    eDateTimeFormat *format)
{
    return ESTATUS_SUCCESS;
}

/* Generate date string from local time structure.
 */
eStatus edate_make_str(
    eLocalTime *local_time,
    os_char *buf,
    os_memsz buf_sz,
    eDateTimeFormat *format,
    os_int flags)
{
    return ESTATUS_SUCCESS;
}

/* Parse date string into local time structure.
 */
eStatus edate_parse_str(
    eLocalTime *local_time,
    const os_char *str,
    eDateTimeFormat *format)
{
    return ESTATUS_SUCCESS;
}
