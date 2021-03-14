/**

  @file    elogindata.c
  @brief   Saving/loading accessing user login data.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    14.9.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "eobjects.h"


/**
****************************************************************************************************

  @brief Set default login data into data structure.

  The elogin_defaults() function default user, passowrds, etc into data structure given as
  argument. This is used when UI, etc, is started for the first time, and if UI configuration
  decryption/checksum fails.

****************************************************************************************************
*/
void elogin_defaults(
    eLoginData *data)
{
    os_memclear(data, sizeof(eLoginData));
    data->selected_row = 0;

    os_strncpy(data->rows[0].user_name, "quest", OSAL_LONG_USER_NAME_SZ);
    os_strncpy(data->rows[0].password, "pass", OSAL_SECRET_STR_SZ);
    data->rows[0].display_row = OS_TRUE;
    data->rows[0].save_password = OS_TRUE;

    os_strncpy(data->rows[1].user_name, "user", OSAL_LONG_USER_NAME_SZ);
    data->rows[1].display_row = OS_TRUE;
    data->rows[1].save_password = OS_TRUE;

    os_strncpy(data->rows[2].user_name, "root", OSAL_LONG_USER_NAME_SZ);
    data->rows[2].display_row = OS_TRUE;
}

