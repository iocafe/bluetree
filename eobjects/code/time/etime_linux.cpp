/**

  @file    etime_linux.cpp
  @brief   Date and time, Linux specific code.
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
#if OSAL_LINUX
#include <time.h>

/* Convert UTC to local time.
 */
eStatus elocaltime(
    eLocalTime *local_time,
    os_long utc,
    eObject *tzone)
{
    os_long usec;
    struct tm result;
    eLocalTime lt;
    time_t ti;
    eStatus s;

    OSAL_UNUSED(tzone);

    os_memclear(&lt, sizeof(lt));
    os_memclear(&result, sizeof(result));

    usec = utc % 1000000;
    lt.millisecond = (os_short)usec / 1000;
    lt.microsecond = (os_short)usec % 1000;
    ti = utc / 1000000;

    s = localtime_r(&ti, &result) ? ESTATUS_SUCCESS :  ESTATUS_FAILED;

    lt.year = result.tm_year + 1900;
    lt.month = result.tm_mon + 1;
    lt.day = result.tm_mday;
    lt.weekday = result.tm_wday + 1;
    lt.yearday = result.tm_yday + 1;
    lt.hour = result.tm_hour;
    lt.minute = result.tm_min;
    lt.second = result.tm_sec;

    if (result.tm_isdst < 0) {
        lt.dst = E_UNKNOWN_DST;
    }
    else if (result.tm_isdst > 0) {
        lt.dst = E_IS_DST;
    }
    else {
        lt.dst = E_NOT_DST;
    }

    *local_time = lt;
    return s;
}

/* Convert local time to UTC. Returns -1 if failed.
 */
os_long emktime(
    eLocalTime *local_time,
    eObject *tzone)
{
    os_long usec;
    struct tm tm;
    eLocalTime lt;
    time_t ti;

    OSAL_UNUSED(tzone);

    lt = *local_time;
    os_memclear(&tm, sizeof(tm));
    tm.tm_year = lt.year - 1900;
    tm.tm_mon = lt.month - 1;
    tm.tm_mday = lt.day;
    tm.tm_hour = lt.hour;
    tm.tm_min = lt.minute;
    tm.tm_sec = lt.second;

    switch (lt.dst) {
        default:
        case E_UNKNOWN_DST: tm.tm_isdst = -1; break;
        case E_IS_DST: tm.tm_isdst = 1; break;
        case E_NOT_DST: tm.tm_isdst = 0; break;
    }

    ti = mktime(&tm);
    if (ti == -1) return -1;

    usec = lt.microsecond + 1000 * lt.millisecond;
    return 1000000L * (os_long)ti + usec;
}

#endif
