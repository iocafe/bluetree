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

/* Limits for valid time stamp
 */
const os_long etimestamp_min = 1501557429043337L; /* ? */
const os_long etimestamp_max = 9601557429043337L; /* ? */


static eDateTimeFormat etime_default_format = {
    E_DATE_ORDER_MDY, ":", ".", "/"};


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
    eDateTimeFormat *format,
    os_memsz *n_chars_parsed)
{
    #define ETIMESTR_ITEMS 8
    os_short item[ETIMESTR_ITEMS];
    const os_char *p;
    os_memsz count, sep_len;
    os_int n;
    os_char *sep;

    if (format == OS_NULL) {
        format = &etime_default_format;
    }

    p = str;
    for (n = 0; n < ETIMESTR_ITEMS; n++) {
        item[n] = osal_str_to_int(p, &count);
        if (count == 0) break;
        p += count;
        sep = (n <= 1 ? format->time_sep : format->millisecond_sep);
        sep_len = os_strlen(sep) - 1;
        if (os_strncmp(p, sep, sep_len)) {
            n++;
            break;
        }
        p += sep_len;
    }

    if (n < 2) {
        if (n_chars_parsed) *n_chars_parsed = 0;
        return ESTATUS_FAILED;
    }

    local_time->hour = item[0];
    local_time->minute = item[1];
    if (n >= 3) local_time->second = item[2];
    if (n >= 4) local_time->millisecond = item[3];
    if (n >= 5) local_time->microsecond = item[4];

    if (n_chars_parsed) *n_chars_parsed = p - str;
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
    eDateTimeFormat *format,
    os_memsz *n_chars_parsed)
{
    #define EDATESTR_ITEMS 6
    os_short item[EDATESTR_ITEMS];
    const os_char *p;
    os_memsz count, sep_len;
    os_int n, i0, i1, i2;
    os_char *sep, used_sep_ch;
    eDateOrder dateorder;

    if (format == OS_NULL) {
        format = &etime_default_format;
    }

    p = str;
    used_sep_ch = 0;
    for (n = 0; n < EDATESTR_ITEMS; n++) {
        item[n] = osal_str_to_int(p, &count);
        if (count == 0) break;
        p += count;
        sep = format->date_sep;
        sep_len = os_strlen(sep) - 1;
        if (os_strncmp(p, sep, sep_len)) {
            if (*p == '/' || *p == '.') {
                used_sep_ch = *(p++);
            }
            else {
                n++;
                break;
            }
        }
        else {
            used_sep_ch = *sep;
            p += sep_len;
        }
    }

    if (n < 3) {
        if (n_chars_parsed) *n_chars_parsed = 0;
        return ESTATUS_FAILED;
    }

    /* Handle unspecified date order.
     */
    dateorder = format->dateorder;
    if (dateorder == E_UNDEFINED_DATE_ORDER)
    {
        if (item[0] >= 2000) {
            dateorder = E_DATE_ORDER_YMD;
        }
        else if (used_sep_ch == '.') {
            dateorder = E_DATE_ORDER_DMY;
        }
        else {
            dateorder = E_DATE_ORDER_MDY;
        }
    }

    switch (dateorder) {
        default:
        case E_DATE_ORDER_MDY: i0 = 2; i1 = 0; i2 = 1; break;
        case E_DATE_ORDER_DMY: i0 = 2; i1 = 1; i2 = 0; break;
        case E_DATE_ORDER_YMD: i0 = 0; i1 = 1; i2 = 2; break;
    }

    local_time->year = item[i0];
    if (local_time->year < 1000) local_time->year += 2000;
    local_time->month = item[i1];
    local_time->day = item[i2];

    if (n_chars_parsed) *n_chars_parsed = p - str;
    return ESTATUS_SUCCESS;
}

/* Convert UTC to timestamp string.
 */
eStatus etime_timestamp_str(
    os_long utc,
    eVariable *x,
    os_int tflags)
{
    eLocalTime localt;
    os_char buf[EDATETIME_STR_BUF_SZ];

    x->clear();
    if (utc == 0) {
        return ESTATUS_FAILED;
    }

    if (elocaltime(&localt, utc)) {
        return ESTATUS_FAILED;
    }

    if (edate_make_str(&localt, buf, sizeof(buf)) == ESTATUS_SUCCESS)
    {
        x->sets(buf);
        x->appends(" ");
    }

    if (etime_make_str(&localt, buf, sizeof(buf), OS_NULL, tflags)
        == ESTATUS_SUCCESS)
    {
        x->appends(buf);
    }

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

