/**

  @file    edirectory.cpp
  @brief   Directory related functions.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef EDIRECTORY_H_
#define EDIRECTORY_H_
#include "eobjects.h"


/**
****************************************************************************************************
  Defines
****************************************************************************************************
*/

/* Flags for emkdir function.
 */
#define EMKDIR_DIR_PATH 1
#define EMKDIR_FILE_PATH 2

/* Create directory and necessary parent directories if these do not exist already.
 */
eStatus emkdir(
    const os_char *path,
    os_int flags);



#endif
