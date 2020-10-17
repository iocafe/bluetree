/**

  @file    etablemessages.h
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
#pragma once
#ifndef ETABLEMESSAGES_H_
#define ETABLEMESSAGES_H_
#include "eobjects.h"

/* Configure the table.
 */
void etable_configure(
    eObject *t,
    const os_char *dbmpath,
    eContainer *configuration,
    os_int tflags = 0);

/* Insert rows into table.
 * Row can be one row or container with multiple rows.
 */
void etable_insert(
    eObject *t,
    const os_char *dbmpath,
    eContainer *rows,
    os_int tflags = 0);

/* Update a row or rows of a table or insert a row to the table.
 */
eStatus etable_update(
    eObject *t,
    const os_char *dbmpath,
    const os_char *whereclause,
    eContainer *row,
    os_int tflags = 0);

/* Remove rows from table.
 */
void etable_remove(
    eObject *t,
    const os_char *dbmpath,
    const os_char *whereclause,
    os_int tflags = 0);

/* Select rows from table.
 */
eStatus etable_select(
    eObject *t,
    const os_char *dbmpath,
    const os_char *whereclause,
    eContainer *columns,
    eDBM *dbm,
    os_int tflags = 0);

#endif
