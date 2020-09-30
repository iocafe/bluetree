/**

  @file    etime.h
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
#pragma once
#ifndef ETIME_H_
#define ETIME_H_
#include "eobjects.h"


/**
****************************************************************************************************

  Local time structure.

    - year: 2020 -
    - month:  1 - 12
    - day: 1 - 31
    - weekday: 1 - 7
    - hour: 0 - 23
    - minute: 0 - 59
    - second: 0 - 59
    - millisecond: 0 - 999
    - dst: 1 = daylight saving in effect. 2 = not in effect, 0 = unknown.

****************************************************************************************************
*/
typedef struct eLocalTime
{
    os_short year;
    os_char month;
    os_char day;
    os_char weekday;

    os_char hour;
    os_char minute;
    os_char second;
    os_short millisecond;
    os_char dst;
}
eLocalTime;

/* Order of date fields.
 */
typedef enum {
    E_DATE_ORDER_MDY = 1,
    E_DATE_ORDER_DMY = 2,
    E_DATE_ORDER_YMD = 3
}
eDateOrder;

/* Recommended buffer size for date or time string.
 */
#define EDATETIME_STR_BUF_SZ 16

/* Field separator string size allocation.
 */
#define EDATETIME_SEP_SZ 4

/* Structure to hold locale specific date and time formatting.
 */
typedef struct eDateTimeFormat
{
    eDateOrder dateorder;

    os_char time_sep[EDATETIME_SEP_SZ];
    os_char millisecond_sep[EDATETIME_SEP_SZ];
    os_char date_sep[EDATETIME_SEP_SZ];
}
eDateTimeFormat;

/* Flags for etime_make_str() and etime_parse_str() functions.
 */
#define ETIMESTR_DEFAULT 0
#define ETIMESTR_SECONDS 1
#define ETIMESTR_MILLISECONDS 2
#define ETIMESTR_ZERO_FILL 4
#define ETIMESTR_SPACE_FILL 8

/* Flags for edate_make_str() and edate_parse_str() functions.
 */
#define EDATESTR_DEFAULT 0
#define EDATESTR_FOUR_DIGIT_YEAR 1

/* Get current time as UTC from computer's clock.
 */
inline os_long etime()
{
    os_int64 ts;
    os_time(&ts);
    return ts;
}

/* Set computer's clock.
 */
inline eStatus etime_set(
    os_long utc)
{
    os_int64 ts;
    ts = utc;
    return (eStatus)os_settime(&ts);
}

/* Convert UTC to local time.
 */
eStatus elocaltime(
    eLocalTime *local_time,
    os_long utc,
    eObject *tzone = OS_NULL);

/* Convert local time to UTC.
 */
eStatus emktime(
    os_long *utc,
    eLocalTime *local_time,
    eObject *tzone = OS_NULL);

/* Generate time string from local time structure.
 */
eStatus etime_make_str(
    eLocalTime *local_time,
    os_char *buf,
    os_memsz buf_sz,
    eDateTimeFormat *format = OS_NULL,
    os_int flags = ETIMESTR_SECONDS);

/* Parse time string into local time structure.
 */
eStatus etime_parse_str(
    eLocalTime *local_time,
    const os_char *str,
    eDateTimeFormat *format = OS_NULL);

/* Generate date string from local time structure.
 */
eStatus edate_make_str(
    eLocalTime *local_time,
    os_char *buf,
    os_memsz buf_sz,
    eDateTimeFormat *format = OS_NULL,
    os_int flags = EDATESTR_DEFAULT);

/* Parse date string into local time structure.
 */
eStatus edate_parse_str(
    eLocalTime *local_time,
    const os_char *str,
    eDateTimeFormat *format = OS_NULL);

#endif
