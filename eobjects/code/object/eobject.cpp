/**

  @file    eobject.cpp
  @brief   Object base class.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  The eObject is base class for all objects. Functions to manage object hierarchy, object
  identification and serialization.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/

/**
****************************************************************************************************

  A red-black tree is a type of self-balancing binary search tree typically used to implement
  associative arrays. It has O(log n) worst-case time for each operation and is quite efficient
  in practice. Unfortunately, it's also quite complex to implement, requiring a number of subtle
  cases for both insertion and deletion.


****************************************************************************************************
*/
#include "eobjects.h"



/**
****************************************************************************************************

  @brief Object constructor.

  The eObject constructor creates an empty object. This constructor is called when object of
  any eobject library is constructed.

  @param   parent Pointer to parent object, or OS_NULL if the object is an orphan. If parent
           object is specified, the object will be deleted when parent is deleted.
  @param   id Object identifier for new object. If not specified, defauls to EOID_ITEM (-1),
           which is generic list item.

  @param   flags

  @return  None.

****************************************************************************************************
*/
eObject::eObject(
    eObject *parent,
    e_oid id,
    os_int flags)
{
    eRoot *root;

    mm_handle = OS_NULL;
    mm_parent = parent;

    /* If this if not primitive object?
     */
    if ((flags & EOBJ_EROOT_OPTIONAL) == 0 || parent != OS_NULL)
    {
        flags &= EOBJ_CLONE_MASK;

        /* No parent, allocate root object?
         */
        if (parent == OS_NULL)
        {
            makeroot(id, flags);
        }

        /* If not root object constructor?
           Otherwise normal child object.  Copy parent's root object pointer
           and allocate handle for the new child object object.
        */
        else if (id != EOID_ROOT_HELPER)
        {
            /* If parent has no root helper object, create one
             */
            if (parent->mm_handle == OS_NULL)
            {
                parent->makeroot(EOID_ITEM, EOBJ_DEFAULT);
            }

            root = parent->mm_handle->m_root;
            root->newhandle(this, parent, id, flags);
        }
    }
}


/**
****************************************************************************************************

  @brief Create root helper object and handles for root and root helper.

  The eObject::makeroot() function...
  This object is pointer to tree root (not helper).

  @return  None.

****************************************************************************************************
*/
void eObject::makeroot(
    e_oid id,
    os_int flags)
{
    eRoot *root;

    /* Allocate root helper object.
    */
    root = new eRoot(this, EOID_ROOT_HELPER, EOBJ_TEMPORARY_ATTACHMENT);

    /* Allocate handle for this object
     */
    root->newhandle(this, OS_NULL, id, flags);

    /* Allocate handle for the root helper object.
     */
    root->newhandle(root, this, EOID_ROOT_HELPER, EOBJ_TEMPORARY_ATTACHMENT);
}


/**
****************************************************************************************************

  @brief Object destructor.

  The eObject destructor releases all child objects and removes object from parent's child
  structure,

  @return  None.

****************************************************************************************************
*/
eObject::~eObject()
{
    eObject *bindings, *b, *next_b, *root;

    /* if (m_parent)
    {
        m_parent->onchilddetach();
    } */

    /* Delete child objects.
     */
    if (mm_handle)
    {
        os_lock();

        bindings = first(EOID_BINDINGS);
        if (bindings) {
            for (b = bindings->first(); b; b = next_b) {
                next_b = b->next();
                delete b;
            }
        }

        if (mm_handle)
        {
            mm_handle->delete_children();
        }

        root = first(EOID_ROOT_HELPER);

        if (mm_parent)
        {
            if (mm_parent->mm_handle) {
                mm_parent->mm_handle->rbtree_remove(mm_handle);
            }
#if OSAL_DEBUG
            else {
                osal_debug_error("mm_parent->mm_handle is NULL");
            }
#endif
        }

        /* Handle no longer needed.
         */
        if (mm_handle->m_root) {
            mm_handle->m_root->freehandle(mm_handle);
        }
#if OSAL_DEBUG
        else {
            osal_debug_error("mm_handle->m_root is NULL");
        }
#endif
        if (root) {
            delete root;
            mm_handle->m_root = OS_NULL;
        }

        os_unlock();
    }
}


/**
****************************************************************************************************

  @brief Clone object

  The eObject::clone function is base class only. Cloning eObject base class is not supported.

  @param  parent Parent for the clone.
  @param  id Object identifier for the clone. Set EOID_CHILD to use the same indentifier as the
          original object.
  @param  aflags
          - EOBJ_NO_MAP not to map names.
          - EOBJ_CLONE_ALL_CHILDREN to clone all children, not just attachments.
          - EOBJ_NO_CLONED_NAMES Do not clone object names.
          - EOBJ_CUST_FLAGSx Set a custom flag.
          - EOBJ_IS_ATTACHMENT Mark cloned object as attachment.
          - EOBJ_NOT_CLONABLE Mark cloned object as not clonable.
          - EOBJ_NOT_SERIALIZABLE Mark cloned object as not serializable.
  @return Pointer to the new clone.

****************************************************************************************************
*/
eObject *eObject::clone(
    eObject *parent,
    e_oid id,
    os_int aflags)
{
    osal_debug_error("clone() not supported for the class");
    return OS_NULL;
}


/**
****************************************************************************************************

  @brief Helper function for clone functionality

  The eObject::clonegeneric is helper function to implement cloning object. It copies attachments
  or all child objects and maps names to name space. This depends on flags.

  @param  clonedobj Pointer to cloned object being created.
  @param  aflags
          - EOBJ_NO_MAP not to map names.
          - EOBJ_CLONE_ALL_CHILDREN to clone all children, not just attachments. Except ones
            specifically flagged EOBJ_NOT_CLONABLE.
          - EOBJ_NO_CLONED_NAMES Do not clone object names.
          - EOBJ_CUST_FLAGSx Set a custom flag.
          - EOBJ_IS_ATTACHMENT Mark cloned object as attachment.
          - EOBJ_NOT_CLONABLE Mark cloned object as not clonable.
          - EOBJ_NOT_SERIALIZABLE Mark cloned object as not serializable.

  @return None.

****************************************************************************************************
*/
void eObject::clonegeneric(
    eObject *clonedobj,
    os_int aflags)
{
    eHandle *handle;

    /* If there is no handle pointer, there can be no children to clone.
     */
    if (mm_handle == OS_NULL) return;

    /* Copy clonable attachments or all clonable object.
     */
    for (handle = mm_handle->first(EOID_ALL);
         handle;
         handle = handle->next(EOID_ALL))
    {
        if (((handle->m_oflags & EOBJ_IS_ATTACHMENT) ||
             (aflags & EOBJ_CLONE_ALL_CHILDREN)) &&
             (handle->m_oflags & EOBJ_NOT_CLONABLE) == 0)
        {
            if (aflags & EOBJ_NO_CLONED_NAMES) {
                if (handle->oid() == EOID_NAME) continue;
            }
            handle->m_object->clone(clonedobj, handle->oid(), EOBJ_NO_MAP);
        }
    }

    /* If flags have been specified for top level object.
     */
    clonedobj->setflags(aflags & EOBJ_CLONE_ARG_AFLAGS_MASK);

    /* If we have name space, but it was not clonable, then create empty name space.
     */
    if (flags() & EOBJ_HAS_NAMESPACE) {
        if (clonedobj->first(EOID_NAMESPACE) == OS_NULL && first(EOID_NAMESPACE)) {
            clonedobj->ns_create();
        }
    }

    /* Map names to name spaces.
     */
    if ((aflags & EOBJ_NO_MAP) == 0) {
        clonedobj->map(E_ATTACH_NAMES);
    }

}


/**
****************************************************************************************************

  @brief Get class name.

  The eObject::classname function returns clas name matching to class id (cid).

  @param   cid Class ifentifier to look for.
  @return  Class name, or OS_NULL if none found.

****************************************************************************************************
*/
const os_char *eObject::classname()
{
    return eclasslist_classname(classid());
}


/**
****************************************************************************************************

  @brief Allocate new object of any listed class.

  The eObject::newobject function looks from global class list by class identifier. If static
  constructor member function corresponding to classid given as argument is found, then an
  object of that class is created as child object if this object.

  @param   cid Class identifier, specifies what kind of object to create.
  @param   oid Object identifier for the new object.
  @return  Pointer to newly created child object, or OS_NULL if none was found.

****************************************************************************************************
*/
eObject *eObject::newobject(
    eObject *parent,
    os_int cid,
    e_oid id,
    os_int flags)
{
    eNewObjFunc func;

    /* Look for static constructor by class identifier. If not found, return OS_NULL.
     */
    func = eclasslist_newobj(cid);
    if (func == OS_NULL) return OS_NULL;

    /* Create new object of the class.
     */
    return func(parent, id, flags);
}


#if EOVERLOAD_NEW_AND_DELETE
/**
****************************************************************************************************

  @brief Overloaded new operator.

  The new operator maps object memory allocation to OSAL function os_malloc().

  @param   size Number of bytes to allocate.
  @return  Pointer to allocated memory block.

****************************************************************************************************
*/
void *eObject::operator new(
    size_t size)
{
    os_char *buf;

    size += sizeof(os_memsz);
    buf = os_malloc((os_memsz)size, OS_NULL);

    *(os_memsz*)buf = (os_memsz)size;

    return buf + sizeof(os_memsz);
}
#endif

#if EOVERLOAD_NEW_AND_DELETE
/**
****************************************************************************************************

  @brief Overloaded delete operator.

  The delete operator maps freeing object memory to OSAL function os_free().

  @param   buf Pointer to memory block to free.
  @return  None.

****************************************************************************************************
*/
void eObject::operator delete(
    void *buf)
{
    if (buf)
    {
        buf = (os_char*)buf - sizeof(os_memsz);
        os_free(buf, *(os_memsz*)buf);
    }
}
#endif


/**
****************************************************************************************************

  @brief Get number of children.

  The eObject::childcount() function returns pointer number of children. Argument oid specified
  wether to count attachment in or to count children only with specific id.

  @param   oid Object idenfifier. Default value EOID_CHILD specifies to count a child objects,
           which are not flagged as an attachment. Value EOID_ALL specifies to get count all
           child objects, regardless wether these are attachment or not. Other values
           specify object identifier, only children with that specified object identifier
           are counted.

  @return  Number of child objects.

****************************************************************************************************
*/
    /* os_long childcount(
        e_oid id = EOID_CHILD)
    {
        if (mm_handle) return mm_handle->childcount(oid);
        return 0;
    } */


/**
****************************************************************************************************

  @brief Convert oix and ucnt to string.

  The eObject::oixstr function creates string which contains both object index and use counter
  of this object. This string can be used as unique identifier of an object/

  Example strings:
  - "@17_3" oix=15, ucnt = 3
  - "@15" oix=15, ucnt = 0

  @param  buf Buffer for resulting string. Recommended size is E_OIXSTR_BUF_SZ.
  @return Size of buffer in bytes.

****************************************************************************************************
*/
void eObject::oixstr(
    os_char *buf,
    os_memsz bufsz)
{
    os_int pos, ucnt;

    osal_debug_assert(mm_handle);

    pos = 0;
    buf[pos++] = '@';
    pos += (os_int)osal_int_to_str(buf+pos, bufsz-pos, mm_handle->oix()) - 1;
    if (pos < bufsz - 1)
    {
        ucnt = mm_handle->ucnt();
        if (ucnt)
        {
            buf[pos++] = '_';
            pos += (os_int)osal_int_to_str(buf+pos, bufsz-pos, mm_handle->ucnt()) - 1;
        }
    }
}


/**
****************************************************************************************************

  @brief Parse oix and ucnt to string.

  The eObject::oixparse function parses object index and use counter from string. See
  eObject::oixstr() function.

  @param  str Pointer to string to parse.
   buf Buffer for resulting string. Recommended size is E_OIXSTR_BUF_SZ.
  @return Number of characters parsed to skip over. Zero if the function failed.

****************************************************************************************************
*/
os_short eObject::oixparse(
    os_char *str,
    e_oix *oix,
    os_int *ucnt)
{
    os_memsz count;
    os_char *p;

    p = str;

    if (*(p++) != '@') goto failed;

    *oix = (e_oix)osal_str_to_int(p, &count);
    if (count == 0) goto failed;
    p += count;
    if (*p != '_')
    {
        *ucnt = 0;
        return (os_short)(p - str);
    }
    p++;

    *ucnt = (os_int)osal_str_to_int(p, &count);
    p += count;
    return (os_short)(p - str);

failed:
    *oix = 0;
    *ucnt = 0;
    return 0;
}


/**
****************************************************************************************************

  @brief Get thread object.

  The eObject::thread() function returns pointer to eThread object to which this object belongs
  to. If this object doesn'is not part of thread's tree structure, the function returns OS_NULL.

  @return  Pointer to eThread, or OS_NULL if none.

****************************************************************************************************
*/
eThread *eObject::thread()
{
    if (mm_handle)
    {
        eObject *o = mm_handle->m_root->parent();
        osal_debug_assert(o);

        /* If this is thread, return pointer.
         */
        if (o->isthread()) return (eThread*)o;
    }
    return OS_NULL;
}


/**
****************************************************************************************************

  @brief Get parent object of specific class and/or identified by specific oid.

  The eObject::parent() function returns pointer to the parent object of specific class or
  object identifier. Either cid, oid, or both, can be used to select the parent.

  @param   cid Class idenfifier. Set ECLASSID_NONE (0) to ignore the class when finding the
           right parent. For example EGUICLASSID_WINDOW.
  @param   id Object idenfifier. Default value EOID_ALL causes function to ignore object
           identifier when finding the parent.
  @param   check_this If true, the function can also return pointer to this object if it
           matches the search criteria (not necessarily an ancestor). If false, this object
           is not checked.

  @return  Pointer to the parent, grandparent, great grandparent... matching to search criteria.

****************************************************************************************************
*/
eObject *eObject::parent(
    os_int cid,
    e_oid id,
    bool check_this)
{
    eObject *obj;

    if (check_this) {
        obj = this;
    }
    else {
        obj = parent();
    }

    while (obj) {
        if ((obj->classid() == cid || cid == ECLASSID_NONE) &&
            (obj->oid() == id || id == EOID_ALL))
        {
            return obj;
        }
        obj = obj->parent();
    }

    return OS_NULL;
}


/**
****************************************************************************************************

  @brief Check is object my ancestor.

  The eObject::isdecendentof() function check if this object is a decendent of specified
  ansestor.

  @param   ancestor Pointer to ancestor object. If OS_NULL, the function returns OS_FALSE.

  @return  OS_TRUE if this is decendent of ancestor, OS_FALSE if not. If ancestor equals this
           object, the function returns OS_TRUE.

****************************************************************************************************
*/
os_boolean eObject::isdecendentof(
    eObject *ancestor)
{
    eObject *o = this;
    while (ancestor != o) {
        if (o == OS_NULL) {
            return OS_FALSE;
        }
        o = o->parent();
    }

    return OS_TRUE;
}


/**
****************************************************************************************************

  @brief Get first child object identified by oid.

  The eObject::first() function returns pointer to the first child object of this object.

  @param   id Object idenfifier. Default value EOID_CHILD specifies to count a child objects,
           which are not flagged as an attachment. Value EOID_ALL specifies to get count all
           child objects, regardless wether these are attachment or not. Other values
           specify object identifier, only children with that specified object identifier
           are searched for.

  @return  Pointer to the first child object, or OS_NULL if none found.

****************************************************************************************************
*/
eObject *eObject::first(
    e_oid id)
{
    if (mm_handle == OS_NULL) return OS_NULL;
    eHandle *h = mm_handle->first(id);
    if (h == OS_NULL) return OS_NULL;
    return h->m_object;
}


/**
****************************************************************************************************

  @brief Get the first child variable identified by oid.

  The eObject::firstv() function returns pointer to the first child variable of this object.

  @param   id Object idenfifier. Default value EOID_CHILD specifies to count a child objects,
           which are not flagged as an attachment. Value EOID_ALL specifies to get count all
           child objects, regardless wether these are attachment or not. Other values
           specify object identifier, only children with that specified object identifier
           are searched for.

  @return  Pointer to the first child variable. Or OS_NULL if none found.

****************************************************************************************************
*/
eVariable *eObject::firstv(
    e_oid id)
{
    if (mm_handle == OS_NULL) return OS_NULL;
    eHandle *h = mm_handle->first(id);
    while (h)
    {
        if (h->m_object->classid() == ECLASSID_VARIABLE)
            return eVariable::cast(h->m_object);

        h = h->next(id);
    }
    return OS_NULL;
}


/**
****************************************************************************************************

  @brief Get the first child matrix identified by oid.

  The eObject::firstm() function returns pointer to the first child eMatrix.

  @param   id Object idenfifier. Default value EOID_CHILD specifies to count a child objects,
           which are not flagged as an attachment. Value EOID_ALL specifies to get count all
           child objects, regardless wether these are attachment or not. Other values
           specify object identifier, only children with that specified object identifier
           are searched for.

  @return  Pointer to the first child matrix. Or OS_NULL if none found.

****************************************************************************************************
*/
eMatrix *eObject::firstm(
    e_oid id)
{
    if (mm_handle == OS_NULL) return OS_NULL;
    eHandle *h = mm_handle->first(id);
    while (h)
    {
        if (h->m_object->classid() == ECLASSID_MATRIX)
            return eMatrix::cast(h->m_object);

        h = h->next(id);
    }
    return OS_NULL;
}


/**
****************************************************************************************************

  @brief Get the first child set by oid.

  The eObject::firsts() function returns pointer to the first child set (eSet) of this object.

  @param   id Object idenfifier. Default value EOID_CHILD specifies to count a child objects,
           which are not flagged as an attachment. Value EOID_ALL specifies to get count all
           child objects, regardless wether these are attachment or not. Other values
           specify object identifier, only children with that specified object identifier
           are searched for.

  @return  Pointer to the first child eSet. Or OS_NULL if none found.

****************************************************************************************************
*/
eSet *eObject::firsts(
    e_oid id)
{
    if (mm_handle == OS_NULL) return OS_NULL;
    eHandle *h = mm_handle->first(id);
    while (h)
    {
        if (h->m_object->classid() == ECLASSID_SET)
            return eSet::cast(h->m_object);

        h = h->next(id);
    }
    return OS_NULL;
}


/**
****************************************************************************************************

  @brief Get the first child container identified by oid.

  The eObject::firstc() function returns pointer to the first child container of this object.

  @param   id Object idenfifier. Default value EOID_CHILD specifies to count a child objects,
           which are not flagged as an attachment. Value EOID_ALL specifies to get count all
           child objects, regardless wether these are attachment or not. Other values
           specify object identifier, only children with that specified object identifier
           are searched for.

  @return  Pointer to the first child container. Or OS_NULL if none found.

****************************************************************************************************
*/
eContainer *eObject::firstc(
    e_oid id)
{
    if (mm_handle == OS_NULL) return OS_NULL;
    eHandle *h = mm_handle->first(id);
    while (h)
    {
        if (h->object()->classid() == ECLASSID_CONTAINER)
            return eContainer::cast(h->m_object);

        h = h->next(id);
    }
    return OS_NULL;
}


/**
****************************************************************************************************

  @brief Get last child object identified by oid.

  The eObject::last() function returns pointer to the last child object of this object.

  @param   id Object idenfifier. Default value EOID_CHILD specifies to count a child objects,
           which are not flagged as an attachment. Value EOID_ALL specifies to get count all
           child objects, regardless wether these are attachment or not. Other values
           specify object identifier, only children with that specified object identifier
           are searched for.

  @return  Pointer to the last child object, or OS_NULL if none found.

****************************************************************************************************
*/
eObject *eObject::last(
    e_oid id)
{
    if (mm_handle == OS_NULL) return OS_NULL;
    eHandle *h = mm_handle->last(id);
    if (h == OS_NULL) return OS_NULL;
    return h->m_object;
}


/**
****************************************************************************************************

  @brief Get next child object identified by oid.

  The eObject::next() function returns pointer to the next object.

  @param   id Object idenfifier. Default value EOID_CHILD specifies to count a child objects,
           which are not flagged as an attachment. Value EOID_ALL specifies to get count all
           child objects, regardless wether these are attachment or not. Other values
           specify object identifier, only children with that specified object identifier
           are searched for.

  @return  Pointer to next object, or OS_NULL if none found.

****************************************************************************************************
*/
eObject *eObject::next(
    e_oid id)
{
    if (mm_handle == OS_NULL) return OS_NULL;
    eHandle *h = mm_handle->next(id);
    if (h == OS_NULL) return OS_NULL;
    return h->m_object;
}


/**
****************************************************************************************************

  @brief Get previous object identified by oid.

  The eObject::prev() function returns pointer to the previous object.

  @param   id Object idenfifier. Default value EOID_CHILD specifies to count a child objects,
           which are not flagged as an attachment. Value EOID_ALL specifies to get count all
           child objects, regardless wether these are attachment or not. Other values
           specify object identifier, only children with that specified object identifier
           are searched for.

  @return  Pointer to previous object, or OS_NULL if none found.

****************************************************************************************************
*/
eObject *eObject::prev(
    e_oid id)
{
    if (mm_handle == OS_NULL) return OS_NULL;
    eHandle *h = mm_handle->prev(id);
    if (h == OS_NULL) return OS_NULL;
    return h->m_object;
}


/**
****************************************************************************************************

  @brief Adopt this eObject as child of parent.

  The eObject::adopt() function moves on object from it's position in tree structure to
  an another.

  @param   parent Pointer to new parent object, or if EOBJ_BEFORE_THIS flags is given, the sibling.
  @param   id EOID_CHILD object identifier unchanged.
  @param   aflags
          - EOBJ_BEFORE_THIS Adopt before this object.
          - EOBJ_NO_MAP not to map names.
          - EOBJ_CUST_FLAGSx Set a custom flag.
          - EOBJ_IS_ATTACHMENT Mark cloned object as attachment.
          - EOBJ_NOT_CLONABLE Mark cloned object as not clonable.
          - EOBJ_NOT_SERIALIZABLE Mark cloned object as not serializable.
  @return  None.

****************************************************************************************************
*/
void eObject::adopt(
    eObject *parent,
    e_oid id,
    os_int aflags)
{
    os_boolean sync;
    eHandle *childh, *parenth;
    eObject *before = OS_NULL;
    os_int mapflags;

    /* Make sure that parent object is already part of tree structure.
     */
    if (parent->mm_handle == OS_NULL)
    {
        /* Allocate root helper object hand two handles.
         */
        parent->makeroot(EOID_TEMPORARY, EOBJ_DEFAULT);

        /* WAS:
        osal_debug_error("adopt(): parent object is not part of tree");
        return; */
    }

    if (aflags & EOBJ_BEFORE_THIS)
    {
        before = parent;
        parent = before->parent();
        id = before->oid();
    }

    if (mm_handle == OS_NULL)
    {
        sync = OS_FALSE; // || m_root->is_process ???????????????????????????????????????????????????????????????????????
        if (sync) os_lock();

        mm_parent = parent;
        if (before) {
            parent->mm_handle->m_root->newhandle(this, before, id, EOBJ_BEFORE_THIS);
        }

        else {
            parent->mm_handle->m_root->newhandle(this, parent, id, 0);
        }

        if (sync) os_unlock();
    }

    else
    {
#if EOBJECT_DBTREE_DEBUG
        verify_whole_tree();
        parent->verify_whole_tree();
#endif

        childh = mm_handle;
        parenth = parent->mm_handle;

        /* Synchronize if adopting from three structure to another.
         */
        sync = (parenth->m_root != childh->m_root);

        if (sync) {
            os_lock();
        }

        /* If not already at right place?
         */
        if (this != before) {
            /* Detach names of child object and it's childen from name spaces
               above this object in tree structure.
             */
            map(E_DETACH_FROM_NAMESPACES_ABOVE);

            if (mm_parent)
            {
                mm_parent->mm_handle->rbtree_remove(childh);
            }

            mm_parent = parent;

            if (id != EOID_CHILD) childh->m_oid = id;
            childh->m_oflags |= EOBJ_IS_RED;
            childh->m_left = childh->m_right = childh->m_up = OS_NULL;

            if (before) {
                parenth->rbtree_insert_at(childh, before->mm_handle);
            }
            else {
                parenth->rbtree_insert(childh);
            }
        }

        /* Map names back: If not disabled by user flag EOBJ_NO_MAP, then attach all names of
           child object (this) and it's childen to name spaces. If a name is already mapped,
           it is not remapped.
           If we are adoprion from a=one tree structure to another (sync is on), we need to set
           m_root pointer (pointer to eRoot of a tree structure)to all child objects.
         */
        mapflags = sync ? E_SET_ROOT_POINTER : 0;
        if ((aflags & EOBJ_NO_MAP) == 0)
        {
            mapflags |= E_ATTACH_NAMES;
        }

        if (mapflags)
        {
            childh->m_root = parenth->m_root;
            map(E_ATTACH_NAMES|E_SET_ROOT_POINTER);
        }

#if EOBJECT_DBTREE_DEBUG
        parent->verify_whole_tree();
#endif

        if (sync) os_unlock();
    }

    /* If flags have been specified for top level object.
     */
    aflags &= EOBJ_CLONE_ARG_AFLAGS_MASK;
    if (aflags) {
        setflags(aflags);
    }
}
