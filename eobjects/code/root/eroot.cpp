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
#include "eobjects.h"


/**
****************************************************************************************************

  @brief Constructor.

  X...

  @return  None.

****************************************************************************************************
*/
eRoot::eRoot(
    eObject *parent,
    e_oid id,
    os_int flags)
    : eObject(parent, id, flags)
{
    /* No free handles reserved yet. Set number of handles to reserver to zero.
       This means that empty default is used.
     */
    m_first_free_handle = OS_NULL;
    m_free_handle_count = 0;
    m_reserve_at_once = 1;
}


/**
****************************************************************************************************

  @brief Virtual destructor.

  X...

  @return  None.

****************************************************************************************************
*/
eRoot::~eRoot()
{
    ehandleroot_releasehandles(m_first_free_handle, -1);
}


/**
****************************************************************************************************

  @brief Add eRoot to class list and class'es properties to it's property set.

  The eRoot::setupclass function adds eThread to class list and class'es properties to
  it's property set. The class list enables creating new objects dynamically by class identifier,
  which is used for serialization reader functions. The property set stores static list of
  class'es properties and metadata for those.

****************************************************************************************************
*/
void eRoot::setupclass()
{
    const os_int cls = ECLASSID_ROOT;

    /* Add the class to class list.
     */
    os_lock();
    eclasslist_add(cls, OS_NULL, "eRoot");
    os_unlock();
}



/**
****************************************************************************************************

  @brief Reserve and initialize handle for object.

  param oid Object identifier, for example EOID_ITEM.

  @return  None.

****************************************************************************************************
*/
void eRoot::newhandle(
    eObject *obj,
    eObject *parent,
    e_oid id,
    os_int flags)
{
    eHandle *handle;
    eObject *before;

    /* If we have no free handles, allocate more. Incse number of handles to
       allocate at once.
     */
    if (m_first_free_handle == OS_NULL)
    {
        if (m_reserve_at_once < 4)
        {
            m_reserve_at_once = 4;
        }
        else if (m_reserve_at_once < 16)
        {
            m_reserve_at_once = 16;
        }
        else if (m_reserve_at_once <= 64)
        {
            m_reserve_at_once *= 2;
        }
        m_first_free_handle = ehandleroot_reservehandles(m_reserve_at_once);
        m_free_handle_count += m_reserve_at_once;
    }

    /* Remove handle to use from chain of free handles.
     */
    handle = m_first_free_handle;
    m_first_free_handle = handle->right();
    m_free_handle_count--;

    /* Save object identifier, clear flags, mark new node as red,
       join to tree hierarchy, no children yet.
     */
    handle->clear(obj, id, flags & ~EOBJ_BEFORE_THIS);
    handle->m_root = this;

    obj->mm_handle = handle;
    if (parent)
    {
        /* If parent object is given, join the new object
           to red black tree of parent's children.
         */
        if (flags & EOBJ_BEFORE_THIS) {
            before = parent;
            parent = before->parent();
            parent->mm_handle->rbtree_insert_at(handle, before->mm_handle);
        }
        else {
            parent->mm_handle->rbtree_insert(handle);
        }
    }
}


/* Close handle of object obj.
 */
void eRoot::freehandle(
    eHandle *handle)
{
    /* Join to chain of free handles.
     */
    handle->m_right = m_first_free_handle;
    handle->m_root = OS_NULL;
    handle->m_object = OS_NULL;
    handle->ucnt_mark_unused();

    m_first_free_handle = handle;
    m_free_handle_count++;

    if (m_free_handle_count > 2*m_reserve_at_once)
    {
        m_first_free_handle = ehandleroot_releasehandles(m_first_free_handle, m_reserve_at_once);
        m_free_handle_count -= m_reserve_at_once;
    }
}
