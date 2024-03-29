/**

  @file    eio_group.h
  @brief   Object representing and IO group.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef EIO_GROUP_H_
#define EIO_GROUP_H_
#include "extensions/io/eio.h"

/**
****************************************************************************************************
  eioGroup is like a box of objects.
****************************************************************************************************
*/
class eioGroup : public eContainer
{
public:
    /* Constructor.
     */
    eioGroup(
        eObject *parent = OS_NULL,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT);

    /* Casting eObject pointer to eioGroup pointer.
     */
    inline static eioGroup *cast(
        eObject *o)
    {
        e_assert_type(o, ECLASSID_EIO_GROUP)
        return (eioGroup*)o;
    }

    /* Get class identifier.
     */
    virtual os_int classid() {return ECLASSID_EIO_GROUP; }

    /* Static function to add class to propertysets and class list.
     */
    static void setupclass();
};

#endif
