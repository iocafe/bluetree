/**

  @file    etablemessages.cpp
  @brief   Functions for constructing and sending table related messages.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    15.10.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "eobjects.h"

/* Configure the table.
 */
void etable_configure(
    eObject *t,
    const os_char *path,
    eContainer *configuration,
    os_int tflags)
{
    /* os_int mflags;

    mflags = (tflags & ETABLE_ADOPT_ARGUMENT) ?

    t->message(CONFIGURE, path, OS_NULL, mflags, context);
    */
    osal_debug_error("eTable::configure is not overloaded");
}

/* Insert rows into table.
 * Row can be one row or container with multiple rows.
 */
void etable_insert(
    eObject *t,
    const os_char *path,
    eContainer *rows,
    os_int tflags)
{
    osal_debug_error("eTable::insert is not overloaded");
}

/* Update a row or rows of a table or insert a row to the table.
 */
eStatus etable_update(
    eObject *t,
    const os_char *path,
    const os_char *whereclause,
    eContainer *row,
    os_int tflags)
{
    osal_debug_error("eTable::update is not overloaded");
    return ESTATUS_FAILED;
}

/* Remove rows from table.
 */
void etable_remove(
    eObject *t,
    const os_char *path,
    const os_char *whereclause,
    os_int tflags)
{
    osal_debug_error("eTable::remove is not overloaded");
}

/* Select rows from table.
 */
eStatus etable_select(
    eObject *t,
    const os_char *path,
    const os_char *whereclause,
    eContainer *columns,
    eDBM *dbm,
    os_int tflags)
{
    osal_debug_error("eTable::select is not overloaded");
    return ESTATUS_FAILED;
}
