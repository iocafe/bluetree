/**

  @file    elogindata.h
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
#pragma once
#ifndef ELOGIN_DATA_H_
#define ELOGIN_DATA_H_

/* Current login data. This is set up as basic C structure instead of eobjects data structure
   to make doubly sure that this data cannot be accessed by browsing, etc. generic method.
 */
typedef struct {
    os_char user_name[OSAL_LONG_USER_NAME_SZ];
    os_char password[OSAL_SECRET_STR_SZ];
    os_boolean display_row;
    os_boolean save_password;
}
eLoginRow;

#define ELOGIN_MAX_ROWS  4

typedef struct
{
    /* Log in rows (pre filles user names and perhaps passwords).
     */
    eLoginRow rows[ELOGIN_MAX_ROWS];

    /* Currently selected row.
     */
    os_int selected_row;

    /* To validate that this structure is loaded and decrypted correctly.
     */
    os_ushort checksum;
}
eLoginData;

/* Load all login data from hard drive (AES decrypt).
 */
eStatus elogint_load(
    eLoginData *data);

/* Save all login data from hard drive (AES encrypt).
 */
eStatus elogint_save(
    eLoginData *data);

/* Set default login data into data structure.
 */
void elogin_defaults(
    eLoginData *data);

/* Save active user name and password.
 */
void elogin_set(
    const os_char *user_name,
    const os_char *password);

/* Load login data and set active user name and password.
 */
void elogin_initialize();

#endif
