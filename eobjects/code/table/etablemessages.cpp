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
 * @param   tflags ETABLE_ADOPT_ARGUMENT Adopt or delete configuration.
 */
void etable_configure(
    eObject *t,
    const os_char *dbmpath,
    eContainer *configuration,
    os_int tflags)
{
    eContainer *content;
    eVariable *f;

    content = new eContainer(t, EOID_ITEM, EOBJ_TEMPORARY_ATTACHMENT);
    if (tflags & ETABLE_ADOPT_ARGUMENT) {
        configuration->adopt(content, EOID_TABLE_CONFIGURATION, EOBJ_NO_MAP);
    }
    else {
        configuration->clone(content, EOID_TABLE_CONFIGURATION, EOBJ_NO_MAP);
    }
    f = new eVariable(content, EOID_FLAGS);
    f->setl(tflags & ETABLE_SERIALIZED_FLAGS_MASK);
    t->message(ECMD_CONFIGURE_TABLE, dbmpath, OS_NULL, content, EMSG_DEL_CONTENT|EMSG_NO_REPLIES);
}

/* Insert rows into table.
 * Row can be one row or container with multiple rows.
 */
void etable_insert(
    eObject *t,
    const os_char *dbmpath,
    eContainer *rows,
    os_int tflags)
{
    osal_debug_error("eTable::insert is not overloaded");
}

/* Update a row or rows of a table or insert a row to the table.
 */
eStatus etable_update(
    eObject *t,
    const os_char *dbmpath,
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
    const os_char *dbmpath,
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
