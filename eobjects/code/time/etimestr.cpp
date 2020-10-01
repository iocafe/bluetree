/**

  @file    etimestr.cpp
  @brief   Date and time strings.
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

static eDateTimeFormat etime_default_format = {
    E_DATE_ORDER_YMD, ":", ".", "/"};


static void estr_append_item(
    os_char **p,
    const os_char *sep,
    os_int sep_len,
    os_int x,
    os_int w);


/* Generate time string from local time structure.
 */
eStatus etime_make_str(
    eLocalTime *local_time,
    os_char *buf,
    os_memsz buf_sz,
    eDateTimeFormat *format,
    os_int flags)
{
    os_int needed, min_width, sep_len, ms_sep_len;
    os_char *p, pad_char;

    if (format == OS_NULL) {
        format = &etime_default_format;
    }

    sep_len = (os_int)os_strlen(format->time_sep) - 1;
    ms_sep_len = (os_int)os_strlen(format->millisecond_sep)  - 1;
    needed = 6 + 2 * sep_len + 1;

    if (flags & (ETIMESTR_MILLISECONDS|ETIMESTR_MICROSECONDS)) needed += 3 + ms_sep_len;
    if (flags & ETIMESTR_MILLISECONDS) needed += 3 + ms_sep_len;

    if (buf_sz < needed) {
        osal_debug_error("Too small time string buffer");
        buf[0] = '\0';
        return ESTATUS_FAILED;
    }

    pad_char = ' ';
    min_width = 0;
    if (flags & (ETIMESTR_ZERO_FILL|ETIMESTR_SPACE_FILL)) {
        min_width = 2;
        if (flags & ETIMESTR_ZERO_FILL) pad_char = '0';
    }

    p = eint2str(buf, local_time->hour, 2, min_width, pad_char);
    estr_append_item(&p, format->time_sep, sep_len, local_time->minute, 2);

    if (flags & (ETIMESTR_SECONDS|ETIMESTR_MILLISECONDS|ETIMESTR_MICROSECONDS))
    {
        estr_append_item(&p, format->time_sep, sep_len, local_time->second, 2);

        if (flags & (ETIMESTR_MILLISECONDS|ETIMESTR_MICROSECONDS)) {
            estr_append_item(&p, format->millisecond_sep, ms_sep_len, local_time->millisecond, 3);

            if (flags & ETIMESTR_MICROSECONDS) {
                estr_append_item(&p, format->millisecond_sep, ms_sep_len, local_time->microsecond, 3);
            }
        }
    }

    if (p) {
        *p = '\0';
        return ESTATUS_SUCCESS;
    }

    buf[buf_sz-1] = '\0';
    return ESTATUS_FAILED;
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
    os_char *date_sep, *p, pad_char;
    os_int sep_len, year_len, year, needed, min_width;

    if (format == OS_NULL) {
        format = &etime_default_format;
    }

    date_sep = format->date_sep;
    sep_len = os_strlen(date_sep) - 1;

    year = local_time->year;
    year_len = 4;
    if ((flags & EDATESTR_FOUR_DIGIT_YEAR) == 0) {
        year %= 100;
        year_len = 2;
    }

    needed = 4 + year_len + 2 * sep_len + 1;
    if (buf_sz < needed) {
        osal_debug_error("Too small time string buffer");
        buf[0] = '\0';
        return ESTATUS_FAILED;
    }

    pad_char = ' ';
    min_width = 0;
    if (flags & (ETIMESTR_ZERO_FILL|ETIMESTR_SPACE_FILL)) {
        min_width = (format->dateorder == E_DATE_ORDER_YMD) ? 4 : 2;
        if (flags & ETIMESTR_ZERO_FILL) pad_char = '0';
    }

    if (flags & EDATESTR_YEAR) {
        p = eint2str(buf, local_time->year, 4, 4, ' ');
    }
    else if (flags & EDATESTR_MONTH) {
        p = eint2str(buf, local_time->month, 2, min_width, pad_char);
    }
    else if (flags & EDATESTR_WEEKDAY) {
        p = eint2str(buf, local_time->weekday, 2, min_width, pad_char);
    }
    else
    {
        switch (format->dateorder)
        {
            default:
            case E_DATE_ORDER_MDY:
                p = eint2str(buf, local_time->month, 2, min_width, pad_char);
                estr_append_item(&p, date_sep, sep_len, local_time->day, 2);
                estr_append_item(&p, date_sep, sep_len, year, year_len);
                break;

            case E_DATE_ORDER_DMY:
                p = eint2str(buf, local_time->day, 2, min_width, pad_char);
                estr_append_item(&p, "", 0, local_time->day, 2);
                estr_append_item(&p, date_sep, sep_len, local_time->month, 2);
                estr_append_item(&p, date_sep, sep_len, year, year_len);
                break;

            case E_DATE_ORDER_YMD:
                p = eint2str(buf, year, year_len, min_width, pad_char);
                estr_append_item(&p, date_sep, sep_len, local_time->month, 2);
                estr_append_item(&p, date_sep, sep_len, local_time->day, 2);
                break;
        }
    }

    if (p) {
        *p = '\0';
        return ESTATUS_SUCCESS;
    }

    buf[buf_sz-1] = '\0';
    return ESTATUS_FAILED;
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


static void estr_append_item(
    os_char **p,
    const os_char *sep,
    os_int sep_len,
    os_int x,
    os_int w)
{
    if (sep_len && *p) {
        os_memcpy(*p, sep, sep_len);
        *p += sep_len;
    }
    *p = eint2str(*p, x, w, w, '0');
}
