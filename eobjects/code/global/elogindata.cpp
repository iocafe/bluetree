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

/* Forward referred static functions.
 */
static void elogin_get_path(
    os_char *path,
    os_memsz path_sz);


/**
****************************************************************************************************

  @brief Load all login data from hard drive (AES decrypt).

  The elogint_load() function loads the login data from hard drive and verifies the checksum.
  If loading fails, or checksum doesn't match, then the data structure is filed by elogin_delaults().

  @param   data Pointer to data structure where to place the loaded data.
  @param   ESTATUS_SUCCESS if data was loaded successfully.

****************************************************************************************************
*/
eStatus elogin_load(
    eLoginData *data)
{
    os_char path[OSAL_PERSISTENT_MAX_PATH];
    os_uchar encrypted[sizeof(eLoginData)];
    os_ushort saved_checksum, calculated_checksum;
    osalStatus s;
    os_memsz n_read;

    /* Initialize with defaults
     */
    elogin_defaults(data);

    /* Make path
     */
    elogin_get_path(path, sizeof(path));

    /* Try to load the file.
     */
    s = os_read_file(path, (os_char*)encrypted, sizeof(encrypted), &n_read, OS_FILE_DEFAULT);
    if (s) {
        osal_debug_error_str("Login data not loaded: ", path);
        goto failed;
    }

    /* Encrypt the login data. This is not bullet proof, just in case operating system
       security should be used to safeguard "secret" files.
     */
    osal_aes_crypt(encrypted, sizeof(encrypted), (os_uchar*)data, sizeof(eLoginData),
        eglobal->active_login.crypt_key, OSAL_AES_DECRYPT);

    /* Verify checksum. If load failed or checksum doesn't match, set defaults.
     */
    saved_checksum = data->checksum;
    data->checksum = 0;
    calculated_checksum = os_checksum((const os_char*)data, sizeof(eLoginData), OS_NULL);
    if (calculated_checksum != saved_checksum) {
        osal_debug_error_str("Login data checksum mismatch: ", path);
        goto failed;
    }

    return ESTATUS_FROM_OSAL_STATUS(s);

failed:
    elogin_defaults(data);
    return ESTATUS_FAILED;
}


/**
****************************************************************************************************

  @brief Save all login data from hard drive (AES encrypt).

  The elogint_save() function calculates checksum and saves the login data to hard drive.

  @param   data Pointer to data to save. This function modifies checksum member.
  @param   ESTATUS_SUCCESS if data was loaded successfully.

****************************************************************************************************
*/
eStatus elogint_save(
    eLoginData *data)
{
    os_char path[OSAL_PERSISTENT_MAX_PATH];
    os_uchar encrypted[sizeof(eLoginData)];
    osalStatus s;

    /* Calculate checksum.
     */
    data->checksum = 0;
    data->checksum = os_checksum((const os_char*)data, sizeof(eLoginData), OS_NULL);

    /* Generate path where to save the file.
     */
    elogin_get_path(path, sizeof(path));

    /* Encrypt the login data. This is not bullet proof, just in case operating system
       security should be used to safeguard "secret" files.
     */
    osal_aes_crypt((os_uchar*)data, sizeof(eLoginData), encrypted, sizeof(encrypted),
        eglobal->active_login.crypt_key, OSAL_AES_ENCRYPT);

    /* Save to file.
     */
    s = os_write_file(path, (os_char*)encrypted, sizeof(encrypted), OS_FILE_DEFAULT);
    if (s) {
        emkdir(path, EMKDIR_FILE_PATH);
        s = os_write_file(path, (os_char*)encrypted, sizeof(encrypted), OS_FILE_DEFAULT);
    }
    if (s) {
        osal_debug_error_str("Saving login data failed: ", path);
    }

    return ESTATUS_FROM_OSAL_STATUS(s);
}


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


/**
****************************************************************************************************

  @brief Save active user name and password.

  The elogin_set() function sets active user name and password. Connections from this point
  on use this as login.

****************************************************************************************************
*/
void elogin_set(
    const os_char *user_name,
    const os_char *password)
{
    os_memclear(eglobal->active_login.user_name, OSAL_LONG_USER_NAME_SZ);
    os_strncpy(eglobal->active_login.user_name, user_name, OSAL_LONG_USER_NAME_SZ);
    os_memclear(eglobal->active_login.password, OSAL_SECRET_STR_SZ);
    os_strncpy(eglobal->active_login.password, password, OSAL_SECRET_STR_SZ);
}


/**
****************************************************************************************************

  @brief Load login data from hard drive and set active login.

  The elogin_initialize() loads login data from hard drive by elogint_load() function, and sets
  active user login.

  @return  OS_TRUE if login, etc, dialog should be opened to ask for password. OS_FALSE if login
           seems complete.

****************************************************************************************************
*/
os_boolean elogin_initialize()
{
    eLoginData data;

    /* Generate encryption key. We use CPUID/computer identification for encryption.
     */
    osal_initialize_aes_crypt_key(eglobal->active_login.crypt_key, OS_TRUE);

    /* Load the login data, use defaults if this fails.
     */
    elogin_load(&data);

    /* Set active login.
     */
    return elogin_set_data(&data);
}


/**
****************************************************************************************************

  @brief Save active user name and password by login data.

  The elogin_set_data() function sets active user name and password using login data as source
  structure.

  @param data Login data structure.

****************************************************************************************************
*/
os_boolean elogin_set_data(
    eLoginData *data)
{
    os_int row, i;
    os_boolean rval = OS_FALSE;

    /* Select row to use.
     */
    row = data->selected_row;
    if (row < 0 || row >= ELOGIN_MAX_ROWS) {
        rval = OS_TRUE;
    }
    else if (data->rows[row].display_row == 0 ||
        data->rows[row].user_name[0] == '\0' ||
        data->rows[row].password[0] == '\0')
    {
        rval = OS_TRUE;
    }
    if (rval) {
        row = -1;
        for (i = ELOGIN_MAX_ROWS - 1; i >= 0; i--) {
            if (data->rows[i].display_row &&
                data->rows[i].user_name[0] != '\0' &&
                data->rows[i].password[0] != '\0')
            {
                row = i;
                break;
            }
        }
        data->selected_row = row;
    }

    /* Set active login
     */
    if (row >= 0) {
        elogin_set(data->rows[row].user_name, data->rows[row].password);
    }

    /* Fallback to guest/pass (useful only for testing).
     */
    else {
        elogin_set("guest", "pass");
    }

    return rval;
}


/**
****************************************************************************************************

  @brief Get path to login data file.

  @param  path Pointer to buffer where to store the path.
  @param  path_sz Buffer size in bytes.

****************************************************************************************************
*/
static void elogin_get_path(
    os_char *path,
    os_memsz path_sz)
{
    os_strncpy(path, eglobal->root_path, path_sz);
    os_strncat(path, "/", path_sz);
    os_strncat(path, eglobal->data_dir, path_sz);
    os_strncat(path, "/_secret/_login.ec", path_sz);
}
