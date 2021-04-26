/**

  @file    eroot.cpp
  @brief   Root helper object.
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
#ifndef EROOT_H_
#define EROOT_H_
#include "eobjects.h"

/**
****************************************************************************************************

****************************************************************************************************
*/
class eRoot : public eObject
{
    /**
    ************************************************************************************************
      Generic eObject functionality.
    ************************************************************************************************
    */
public:
    /* Constructor.
     */
    eRoot(
        eObject *parent = OS_NULL,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT);

    /* Virtual destructor.
     */
    virtual ~eRoot();

    /* Casting eObject pointer to eRoot pointer.
     */
    inline static eRoot *cast(
        eObject *o)
    {
        e_assert_type(o, ECLASSID_ROOT)
        return (eRoot*)o;
    }

    /* Get class identifier.
     */
    virtual os_int classid() {return ECLASSID_ROOT;}

    /* Static function to add class to propertysets and class list.
     */
    static void setupclass();

    /* Static constructor function for generating instance by class list.
     */
    /* static eRoot *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new eRoot(parent, id, flags);
    } */


    /**
    ************************************************************************************************
      eRoot specific
    ************************************************************************************************
    */
    /* Reserve and initialize handle for object obj.
     */
    void newhandle(
        eObject *obj,
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_EROOT_OPTIONAL);

    /* Free handle of object obj.
     */
    void freehandle(
        eHandle *handle);


protected:

    /**
    ************************************************************************************************
      Member variables
    ************************************************************************************************
    */
    /** Pointer to first free handle.
     */
    eHandle *m_first_free_handle;

    /** Number of handles to reserve at once. Initially reserve for example two handles at the time,
        grow the number when more handles are allocated until maximum limit reached.
     */
    os_int m_reserve_at_once;

    /** Number of free handles.
     */
    os_int m_free_handle_count;
};

#endif
