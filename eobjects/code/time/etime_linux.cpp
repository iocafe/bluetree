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
#if OSAL_LINUX

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

#endif
