/**

  @file    eclasslist.h
  @brief   Class list.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

  Maintain list of classes which can be created dynamically by class ID.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef ECLASSLIST_H_
#define ECLASSLIST_H_
#include "eobjects.h"


typedef eObject *(*eNewObjFunc)(
    eObject *parent,
    e_oid id,
    os_int flags);

/* Add class to class list.
 */
void eclasslist_add(
    os_int cid,
    eNewObjFunc nfunc,
    const os_char *classname,
    os_int baseclass_id = ECLASSID_OBJECT);

/* Get static object constuctor function pointer by class ID.
 */
eNewObjFunc eclasslist_newobj(
    os_int cid);

/* Check if a class is derived from a base class, or instance of the base class.
 */
os_boolean eclasslist_isinstanceof(
    os_int this_cid,
    os_int base_class_cid);

/* Get class name.
 */
os_char *eclasslist_classname(
    os_int cid);

/* Initialize class list.
 */
void eclasslist_initialize();

/* release class list.
 */
void eclasslist_release();

#endif
