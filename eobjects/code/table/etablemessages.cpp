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

/* Forward referred static functions.
 */
static eContainer *etable_prepare_msg(
    eObject *t,
    eContainer *item,
    e_oid id,
    os_int tflags);


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

    content = etable_prepare_msg(t, configuration, EOID_TABLE_CONFIGURATION, tflags);
    t->message(ECMD_CONFIGURE_TABLE, dbmpath, OS_NULL, content,
        EMSG_DEL_CONTENT|EMSG_NO_REPLIES);
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
    eContainer *content;

    content = etable_prepare_msg(t, rows, EOID_TABLE_CONTENT, tflags);
    t->message(ECMD_INSERT_ROWS_TO_TABLE, dbmpath, OS_NULL, content,
        EMSG_DEL_CONTENT|EMSG_NO_REPLIES);
}


/* Update a row or rows of a table or insert a row to the table.
 */
void etable_update(
    eObject *t,
    const os_char *dbmpath,
    const os_char *whereclause,
    eContainer *row,
    os_int tflags)
{
    eContainer *content;
    eVariable *v;

    content = etable_prepare_msg(t, row, EOID_TABLE_CONTENT, tflags);
    v = new eVariable(content, EOID_TABLE_WHERE);
    v->sets(whereclause);

    t->message(ECMD_UPDATE_TABLE_ROWS, dbmpath, OS_NULL, content,
        EMSG_DEL_CONTENT|EMSG_NO_REPLIES);
}


/* Remove rows from table.
 */
void etable_remove(
    eObject *t,
    const os_char *dbmpath,
    const os_char *whereclause,
    os_int tflags)
{
    eContainer *content;
    eVariable *v;

    content = etable_prepare_msg(t, OS_NULL, 0, tflags);
    v = new eVariable(content, EOID_TABLE_WHERE);
    v->sets(whereclause);

    t->message(ECMD_REMOVE_ROWS_FROM_TABLE, dbmpath, OS_NULL, content,
        EMSG_DEL_CONTENT|EMSG_NO_REPLIES);
}


/* Prepare message to send.
 */
static eContainer *etable_prepare_msg(
    eObject *t,
    eContainer *item,
    e_oid id,
    os_int tflags)
{
    eContainer *content;
    eVariable *f;

    content = new eContainer(t, EOID_ITEM, EOBJ_TEMPORARY_ATTACHMENT);
    if (item) {
        if (tflags & ETABLE_ADOPT_ARGUMENT) {
            item->adopt(content, id, EOBJ_NO_MAP);
        }
        else {
            item->clone(content, id, EOBJ_NO_MAP);
        }
    }

    tflags &= ETABLE_SERIALIZED_FLAGS_MASK;
    if (tflags) {
        f = new eVariable(content, EOID_FLAGS);
        f->setl(tflags & ETABLE_SERIALIZED_FLAGS_MASK);
    }

    return content;
}
