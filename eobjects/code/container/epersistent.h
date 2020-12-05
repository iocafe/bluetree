/**

  @file    econtainer.h
  @brief   Simple object container.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  The container object is like a box holding a set of child objects.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef EPERSISTENT_H_
#define EPERSISTENT_H_
#include "eobjects.h"


/**
****************************************************************************************************
  Defines
****************************************************************************************************
*/

/* Persistent object property numbers.
 */
#define EPERP_PATH 10

/* Persistent object property names.
 */
extern const os_char
    eperp_path[];


/**
****************************************************************************************************
  ePersistent is like a box of objects.
****************************************************************************************************
*/
class ePersistent : public eContainer
{
public:
    /* Constructor.
     */
    ePersistent(
        eObject *parent = OS_NULL,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT);

    /* Virtual destructor.
     */
    virtual ~ePersistent();

    /* Clone object.
     */
    virtual eObject *clone(
        eObject *parent,
        e_oid id = EOID_CHILD,
        os_int aflags = 0);

    /* Casting eObject pointer to ePersistent pointer.
     */
    inline static ePersistent *cast(
        eObject *o)
    {
        e_assert_type(o, ECLASSID_PERSISTENT)
        return (ePersistent*)o;
    }

    /* Get class identifier.
     */
    virtual os_int classid() {return ECLASSID_PERSISTENT; }

    /* Static function to add class to propertysets and class list.
     */
    static void setupclass();

    /* Static constructor function for generating instance by class list.
     */
    static ePersistent *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new ePersistent(parent, id, flags);
    }
};

#endif
