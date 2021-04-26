/**

  @file    edirectory.cpp
  @brief   Directory related functions.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "eobjects.h"


/**
****************************************************************************************************

  @brief Create directories.
  @anchor osal_mkdir

  The emkdir() function creates directories. Necessary parent directories are created.

  @param  path Path to directory (EMKDIR_DIR_PATH flag) or path to file (EMKDIR_FILE_PATH flag).
  @param  flags EMKDIR_DIR_PATH or EMKDIR_FILE_PATH.
  @return If successful, the function returns ESTATUS_SUCCESS (0). Other return values
          indicate an error. If directory already exists?

****************************************************************************************************
*/
eStatus emkdir(
    const os_char *path,
    os_int flags)
{
    const os_char *p, *e;
    eVariable part;
    eStatus rval = ESTATUS_SUCCESS;
    osalStatus s;

    p = os_strchr(path, '/');
    if (p == OS_NULL) return  ESTATUS_FAILED;
    p++;

    while (OS_TRUE)
    {
        e = os_strchr(p, '/');
        if (e == OS_NULL) {
            if (flags & EMKDIR_FILE_PATH) break;
            e = os_strchr(p, '\0');
        }
        part.clear();
        part.appends_nbytes(path, e - path);
        if (!part.isempty()) {
            s = osal_mkdir(part.gets(), 0);
            rval = ESTATUS_FROM_OSAL_STATUS(s);
            if (s) break;
        }

        if (*e == '\0') break;
        p = e + 1;
    }

    return rval;
}
