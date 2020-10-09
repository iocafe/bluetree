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

/* Name space identifiers as static strings. eobj_this_ns is default
   for ns_first and ns_firstv functions()
 */
const os_char eobj_process_ns[] = E_PROCESS_NS;
const os_char eobj_thread_ns[] = E_THREAD_NS;
const os_char eobj_parent_ns[] = E_PARENT_NS;
const os_char eobj_this_ns[] = E_THIS_NS;


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
    flags &= EOBJ_CLONE_MASK;

    mm_parent = parent;

    /* If this if not primitive object?
     */
    if (id != EOID_ITEM || parent != OS_NULL)
    {
        /* No parent, allocate root object?
         */
        if (parent == OS_NULL)
        {
            /* Allocate root helper object hand two handles.
             */
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
    /* if (m_parent)
    {
        m_parent->onchilddetach();
    } */

    /* Delete child objects.
     */
    if (mm_handle)
    {
        os_lock();

        /* if (mm_handle->m_parent)
        if (mm_parent)
            */
        if (mm_handle)
        {
            mm_handle->delete_children();
        }

        if ((flags() & EOBJ_FAST_DELETE) == 0)
        {
            /* If handle has parent, remove from parent's children.
             */
            /* if (mm_handle->m_parent)
            {
                mm_handle->m_parent->rbtree_remove(mm_handle);
            } */
            if (mm_parent)
            {
                mm_parent->mm_handle->rbtree_remove(mm_handle);
            }

            /* Handle no longer needed.
             */
            mm_handle->m_root->freehandle(mm_handle);

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
  @param  aflags EOBJ_NO_AFLAGS (0) for default operation.
          - EOBJ_NO_MAP not to map names.
          - EOBJ_CLONE_ALL_CHILDREN to clone all children, not just attachments.
          - EOBJ_NO_CLONED_NAMES Do not clone object names.
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
  @param  aflags EOBJ_NO_AFLAGS (0) for default operation.
          - EOBJ_NO_MAP not to map names.
          - EOBJ_CLONE_ALL_CHILDREN to clone all children, not just attachments.
          - EOBJ_NO_CLONED_NAMES Do not clone object names.

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

    /* Map names to name spaces.
     */
    if ((aflags & EOBJ_NO_MAP) == 0)
    {
        map(E_ATTACH_NAMES);
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
    while (ancestor != this) {
        if (ancestor == OS_NULL) {
            return OS_FALSE;
        }
        ancestor = ancestor->parent();
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

  @brief Get the first child name identified by oid.

  The eObject::firstn() function returns pointer to the first child name of this object.

  @param   id Object idenfifier. Default value EOID_CHILD specifies to count a child objects,
           which are not flagged as an attachment. Value EOID_ALL specifies to get count all
           child objects, regardless wether these are attachment or not. Other values
           specify object identifier, only children with that specified object identifier
           are searched for.

  @return  Pointer to the first child name. Or OS_NULL if none found.

****************************************************************************************************
*/
eName *eObject::firstn(
    e_oid id)
{
    if (mm_handle == OS_NULL) return OS_NULL;
    eHandle *h = mm_handle->first(id);
    while (h)
    {
        if (h->object()->classid() == ECLASSID_NAME)
            return eName::cast(h->m_object);

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

  @brief Adopt obeject as child.

  The eObject::adopt() function moves on object from it's position in tree structure to
  an another.

  @param   id EOID_CHILD object identifier unchanged.
  @param   aflags EOBJ_BEFORE_THIS Adopt before this object. EOBJ_NO_MAP not to map names.
  @return  None.

****************************************************************************************************
*/
void eObject::adopt(
    eObject *child,
    e_oid id,
    os_int aflags)
{
    os_boolean sync;
    eHandle *childh;
    os_int mapflags;

    /* Make sure that parent object is already part of tree structure.
     */
    if (mm_handle == OS_NULL)
    {
        osal_debug_error("adopt(): parent object is not part of tree");
        return;
    }

    if (child->mm_handle == OS_NULL)
    {
        sync = OS_FALSE; // || m_root->is_process ???????????????????????????????????????????????????????????????????????
        if (sync) os_lock();

        child->mm_parent = this;
        mm_handle->m_root->newhandle(child, this, id, 0);

        if (sync) os_unlock();
    }

    else
    {
// child->mm_handle->verify_whole_tree();
// mm_handle->verify_whole_tree();

        // Detach names

        childh = child->mm_handle;

        /* Synchronize if adopting from three structure to another.
         */
        sync = (mm_handle->m_root != childh->m_root);

        if (sync)
        {
            os_lock();
        }

        /* Detach names of child object and it's childen from name spaces
           above this object in tree structure.
         */
        child->map(E_DETACH_FROM_NAMESPACES_ABOVE);

        /* if (childh->m_parent)
        {
            childh->m_parent->rbtree_remove(childh);
        } */
        if (child->mm_parent)
        {
            child->mm_parent->mm_handle->rbtree_remove(childh);

        }

        child->mm_parent = this;

        if (id != EOID_CHILD) childh->m_oid = id;
        childh->m_oflags |= EOBJ_IS_RED;
        childh->m_left = childh->m_right = childh->m_up = OS_NULL;
        mm_handle->rbtree_insert(childh);
        /* childh->m_parent = mm_handle; */

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
            childh->m_root = mm_handle->m_root;
            child->map(E_ATTACH_NAMES|E_SET_ROOT_POINTER);
        }

// mm_root->mm_handle->verify_whole_tree();


        if (sync) os_unlock();
    }
}


/**
****************************************************************************************************

  @brief Create name space for this object.

  The eObject::ns_create() function creates a name space for object. If object already
  has name space...

  @param   namespace_id Identifier for the name space.
  @return  None.

****************************************************************************************************
*/
void eObject::ns_create(
    const os_char *namespace_id)
{
    eNameSpace *ns;

    /* If object has already name space.
     */
    ns = eNameSpace::cast(first(EOID_NAMESPACE));
    if (ns)
    {
        /* If namespace identifier matches, just return.
         */
        if (ns->namespaceid())
        {
            if (!os_strcmp(namespace_id, ns->namespaceid()->gets()))
                return;
        }

        /* Delete old name space.
           We should keep ot if we want to have multiple name spaces???
         */
        delete ns;
    }

    /* Create name space.
     */
    // ns = eNameSpace::newobj(this, EOID_NAMESPACE);
    ns = new eNameSpace(this, EOID_NAMESPACE);
    if (namespace_id)
    {
        ns->setnamespaceid(namespace_id);
    }

    /* Remap names in child objects ??? Do we need this. In practise name space is created
       before placing children in?
     */
}


/**
****************************************************************************************************

  @brief Delete name space of this object.

  The eObject::ns_delete() function deletes name space of this object.

  @return  None.

****************************************************************************************************
*/
void eObject::ns_delete()
{
    delete first(EOID_NAMESPACE);
}


/**
****************************************************************************************************

  @brief Find eName by name value and namespace identifier.

  The eObject::ns_first() function finds the first eName object matching to name. If name
  is OS_NULL, the function returns the first name in namespace (if any).
  Name may contain name space identifier, for example "myid/myname" in which case the name
  space identifier given as argument is ignored.

  @param   name Pointer to eVariable or eName, holding name to search for. Name can be prefixed
           by name space identifier. If OS_NULL, then pointer to first name in name space
           is returned.
  @param   Name space identifier string. OS_NULL to specify name space of this object.
  @return  Pointer to name. OS_NULL if none found.

****************************************************************************************************
*/
eName *eObject::ns_first(
    const os_char *name,
    const os_char *namespace_id)
{
    if (name == OS_NULL)
    {
        return ns_firstv(OS_NULL, namespace_id);
    }
    else
    {
        eVariable n;
        n = name;
        return ns_firstv(&n, namespace_id);
    }
}


/**
****************************************************************************************************

  @brief Find eName by name and name space.

  The eObject::ns_firstv() function finds the first eName object matching to name. If name
  is OS_NULL, the function returns the first name in namespace (if any).
  Name may contain name space identifier, for example "myid/myname" in which case the name
  space identifier given as argument is ignored.

  @param   name String name to search for. Name can be prefixed by name space identifier.
           If OS_NULL, then pointer to first name in name space is returned.
  @param   Name space identifier string. OS_NULL to specify name space of this object.
  @return  Pointer to name. OS_NULL if none found.

****************************************************************************************************
*/
eName *eObject::ns_firstv(
    eVariable *name,
    const os_char *namespace_id)
{
    eNameSpace *ns;
    eName *n;
    os_char *p, *q;
    eVariable *tmp_name, *tmp_id;

    tmp_name = tmp_id = OS_NULL;

    /* String type may contain name space prefix, check for it.
     */
    if (name) if (name->type() == OS_STR)
    {
        p = name->gets();
        q = os_strechr(p, '/');
        if (q)
        {
            tmp_id = new eVariable;
            tmp_id->sets(p, q-p);
            namespace_id = tmp_id->gets();

            tmp_name = new eVariable;
            tmp_name->sets(q+1);
            name = tmp_name;
        }
    }

    /* Find name space.
     */
    ns = findnamespace(namespace_id);
    if (ns == OS_NULL) return OS_NULL;

    /* Find name in name space.
     */
    n = ns->findname(name);

    /* Cleanup and return.
     */
    if (tmp_name)
    {
        delete tmp_name;
        delete tmp_id;
    }
    return n;
}


/**
****************************************************************************************************

  @brief Find object by name.

  The eObject::ns_get() function finds the first named object matching to name in speficied
  namespace. If name is OS_NULL, the function returns the first name in namespace
  (if any).  Name may contain name space identifier, for example "myid/myname" in which case
  the name space identifier given as argument is ignored.

  @param   name Pointer to eVariable or eName, holding name to search for. Name can be prefixed
           by name space identifier. If OS_NULL, then pointer to first name in name space
           is returned.
  @param   namespace_id Name space identifier string. OS_NULL to specify name space of this object.
  @param   cid Specifies class for object to look for. Specify ECLASSID_OBJECT to search
           for object of any class. For example ECLASSID_VARIABLE searches for only eVariable
           objects, or ECLASSID_CONTAINER only containers.
  @return  Pointer to the first object with matching name and class found. OS_NULL if none found.

****************************************************************************************************
*/
eObject *eObject::ns_get(
    const os_char *name,
    const os_char *namespace_id,
    os_int cid)
{
    eName *n;
    eObject *p;

    n  = ns_first(name, namespace_id);
    while (n)
    {
        p = n->parent();
        if (cid == ECLASSID_OBJECT || p->classid() == cid) return p;
        n = n->ns_next();
    }

    return OS_NULL;
}


eVariable *eObject::ns_getv(
    const os_char *name,
    const os_char *namespace_id)
{
    return eVariable::cast(ns_get(name, namespace_id, ECLASSID_VARIABLE));
}


eContainer *eObject::ns_getc(
    const os_char *name,
    const os_char *namespace_id)
{
    return eContainer::cast(ns_get(name, namespace_id, ECLASSID_CONTAINER));
}


/**
****************************************************************************************************

  @brief Find name space by naespace identifier.

  The eObject::findnamespace() function adds name to this object and maps it to name space.

  - When looking for parent namespace with "..", the function returns next namespace ABOVE this
    object. If this object has name space, it will not be returned.
  - When searching by name space identifier, the first name with matching identifier is returned.
    This can be object's (this) own name space.

  @param  namespace_id Identifier for the name space. OS_NULL refers to first parent name space,
          regardless of name space identifier.
  @param  info Pointer where to set information bits. OS_NULL if not needed.
          E_INFO_PROCES_NS bit indicates that the name space is process namespace.
          E_INFO_ABOVE_CHECKPOINT bit indicates that name space is above check point in tree
          structure.
  @param  checkpoint Pointer to object in tree structure to check if name space is above this one.
          OS_NULL if not needed.
  @return Pointer to name space, eNameSpace class. OS_NULL if none found.

****************************************************************************************************
*/
eNameSpace *eObject::findnamespace(
    const os_char *namespace_id,
    os_int *info,
    eObject *checkpoint)
{
    eNameSpace *ns;
    eHandle *h, *ns_h;
    os_boolean getparent;

    /* No information yet.
     */
    if (info) *info = 0;

    /* If name space id NULL, it is same as parent name space.
     */
    if (namespace_id == OS_NULL)
    {
        namespace_id = E_PARENT_NS;
    }

    switch (*namespace_id)
    {
        /* If name space id refers to process name space, just return pointer to it.
         */
        case '/':
            if (info) *info = E_INFO_PROCES_NS|E_INFO_ABOVE_CHECKPOINT;
            return eglobal->process_ns;

        /* If thread name space, just return pointer to the name space.
         */
        case '\0':
            if (info) *info = E_INFO_ABOVE_CHECKPOINT;
            return eNameSpace::cast(mm_handle->m_root->first(EOID_NAMESPACE));

        default:
            if (!os_strcmp(namespace_id, "."))
            {
                if ((flags() & EOBJ_HAS_NAMESPACE) == 0) return OS_NULL;
                return eNameSpace::cast(first(EOID_NAMESPACE));
            }
            else
            {
                getparent = (os_boolean)!os_strcmp(namespace_id, "..");
            }
            break;
    }

    /* Look upwards for parent or matching name space.
     */
    h = getparent ? (mm_parent ? mm_parent->mm_handle : OS_NULL) : mm_handle;
    /* h = getparent ? mm_handle->parent() : mm_handle; */
    while (h)
    {
        if (h->flags() & EOBJ_HAS_NAMESPACE)
        {
            /* Support multiple name spaces per object.
             */
            ns_h = h->first(EOID_NAMESPACE);
            while (ns_h)
            {
                if (getparent)
                    return eNameSpace::cast(ns_h->object());

                /* If name space has identifier, and it matches?
                 */
                if (namespace_id)
                {
                    ns = eNameSpace::cast(ns_h->object());
                    if (ns) if (ns->namespaceid())
                    {
                        if (!os_strcmp(namespace_id, ns->namespaceid()->gets()))
                            return ns;
                    }
                }
                ns_h = ns_h->next(EOID_NAMESPACE);
            }
        }

        if (info) if (h->m_object == checkpoint) *info |= E_INFO_ABOVE_CHECKPOINT;
        /* h = h->parent(); */
        if (h->m_object->mm_parent == OS_NULL) break;
        h = h->m_object->mm_parent->mm_handle;
    }

    return OS_NULL;
}


/**
****************************************************************************************************

  @brief Name this object.

  The eObject::addname() function adds name to this object and maps it to name space.

  @param   name Name for this object. Name may start with name space identifier separated with
           slash from actual name. If name space is identified as part of name, the namespace_id
           argument will be ignored.
  @param   namespace_id Identifier for the name space. OS_NULL is first parent's name space.
  @param   flags
  @return  Pointer to newly created name, eName class.

****************************************************************************************************
*/
eName *eObject::addname(
    const os_char *name,
    os_int flags,
    const os_char *namespace_id)
{
    eName *n;
    os_char buf[128];
    const os_char *p;
    os_memsz sz;

    /* Create name object.
     */
    n = new eName(this, EOID_NAME);

    /* Set flags for name, like persistancy.
     */
    if (flags & ENAME_TEMPORARY)
    {
        n->setflags(EOBJ_NOT_CLONABLE|EOBJ_NOT_SERIALIZABLE);
    }

    /* If name space is not given as argument, decide by flags.
     */
    if (namespace_id == OS_NULL)
    {
        namespace_id = eobj_parent_ns;

        /* If name starts with namespace id.
         */
        if (name)
        {
            if (name[0] == '/')
            {
                if (name[1] == '/')
                {
                    namespace_id = eobj_process_ns;
                    name += 2;
                    goto namespace_selected;
                }
                else
                {
                    namespace_id = eobj_thread_ns;
                    name++;
                    goto namespace_selected;
                }
            }
            else if (name[0] == '.')
            {
                if (name[1] == '/')
                {
                    namespace_id = eobj_this_ns;
                    name += 2;
                    goto namespace_selected;
                }
                else if (name[1] == '.') if (name[2] == '/')
                {
                    namespace_id = eobj_parent_ns;
                    name += 3;
                    goto namespace_selected;
                }
            }

            p = os_strchr((char*)name, '/');
            if (p) {
                sz = p - name + 1;
                if (sz > (os_memsz)sizeof(buf)) sz = sizeof(buf);
                os_strncpy(buf, name, sz);
                namespace_id = buf;
                name = p + 1;
                goto namespace_selected;
            }
        }

        if (flags & ENAME_PROCESS_NS)
        {
            namespace_id = eobj_process_ns;
        }
        else if (flags & ENAME_THREAD_NS)
        {
            namespace_id = eobj_thread_ns;
        }
        else if (flags & ENAME_THIS_NS)
        {
            namespace_id = eobj_this_ns;
        }
    }
namespace_selected:

    /* Set name string, if any.
     */
    if (name) n->sets(name);

    /* Set name space identifier.
     */
    n->setnamespaceid(namespace_id);

    /* Map name to namespace, unless disabled for now.
     */
    if ((flags & ENAME_NO_MAP) == 0)
    {
        n->mapname();
    }

    /* Return pointer to name.
     */
    return n;
}



/**
****************************************************************************************************

  @brief Find object's first name.

  The eObject::firstname() function gets first name given to this object.
  Object's first name may later on separated with alternate names with name flags. At this
  fime first name is simply name which was added first.

  @return  Pointer to first name (eName), or OS_NULL if none found.

****************************************************************************************************
*/
eName *eObject::firstname()
{
    eObject *n;

    for (n = first(EOID_NAME); n; n = n->next(EOID_NAME)) {
        if (n->classid() == ECLASSID_NAME) {
            return eName::cast(n);
        }
    }
    return OS_NULL;
}



/**
****************************************************************************************************

  @brief Attach/detach names in tree sturcture to name spaces. Set eRoot pointers.

  The eObject::map() function is attaches/detaches names from name spaces, and sets used
  eRoot object pointer for child objects. The functionality is controlled by flags.
  This is function is mostly used to remap names when an object (tree structure) is adopted
  from one thread to another. And when queuing messages for threads and outgoing connections.

  @param   flags
           - E_ATTACH_NAMES: Attach all names of child object (this) and it's childen to
             name spaces. If a name is already mapped, it is not remapped.

           - E_SET_ROOT_POINTER: Copy m_root pointer (pointer to eRoot of a tree structure)
             from child object (this) to all child objects of it.

           - E_DETACH_FROM_NAMESPACES_ABOVE: Detach names of child object (this) and it's
             childen from name spaces above this object in tree structure.

  @return  None

****************************************************************************************************
*/
void eObject::map(
    os_int mflags)
{
    osal_debug_assert(mm_handle != OS_NULL);

    /* Handle special case when this is name
     */
    if (mm_handle->oid() == EOID_NAME &&
       (mflags & (E_ATTACH_NAMES|E_DETACH_FROM_NAMESPACES_ABOVE)))
    {
        mapone(mm_handle, mflags);
    }

    /* Map all child objects.
     */
    map2(mm_handle, mflags);
}

void eObject::map2(
    eHandle *handle,
    os_int mflags)
{
    eHandle
        *childh;

    for (childh = handle->first(EOID_ALL);
         childh;
         childh = childh->next(EOID_ALL))
    {
        if (mflags & E_SET_ROOT_POINTER)
        {
            childh->m_root = handle->m_root;
        }

        /* If this is name which needs to be attached or detached, do it.
         */
        if (childh->oid() == EOID_NAME &&
           (mflags & (E_ATTACH_NAMES|E_DETACH_FROM_NAMESPACES_ABOVE)))
        {
            mapone(childh, mflags);
        }

        /* If this has children, process those.
         */
        if (childh->m_children)
        {
            map2(childh, mflags);
        }
    }
}


/**
****************************************************************************************************

  @brief Attach or detach a name to a name space.

  The eObject::mapone() function attaches or detaches a name to/from name space.

  @return  None.

****************************************************************************************************
*/
void eObject::mapone(
    eHandle *handle,
    os_int mflags)
{
    eName *name;
    eNameSpace *ns;
    os_int info;

    name = eName::cast(handle->m_object);

    ns = handle->m_object->findnamespace(name->namespaceid(), &info, this);

    if ((mflags & E_ATTACH_NAMES))
    {
        osal_debug_assert(ns);
        if (ns) name->mapname2(ns, info);
    }

    if (mflags & E_DETACH_FROM_NAMESPACES_ABOVE)
    {
        if (info & E_INFO_ABOVE_CHECKPOINT)
        {
            name->detach();
        }
    }
}


/**
****************************************************************************************************

  @brief Get object by name.

  The eObject::byname() function looks for name in this object's name space.

  @param  name Name to look for.
  @return If name is found, returns pointer to namef object. Otherwise the function returns OS_NULL.

****************************************************************************************************
*/
eObject *eObject::byname(
    const os_char *name)
{
    eVariable namev;
    eName *nobj;
    eNameSpace *nspace;

    nspace = eNameSpace::cast(first(EOID_NAMESPACE));
    if (nspace)
    {
        namev.sets(name);
        nobj = nspace->findname(&namev);
        if (nobj) return nobj->parent();
    }
    return OS_NULL;
}


/**
****************************************************************************************************

  @brief Write object to stream.

  The eObject::write() function writes object with class information, attachments, etc to
  the stream.

  @param  stream The stream to write to.
  @param  sflags Serialization flags. EOBJ_SERIALIZE_DEFAULT

  @return If successfull the function returns ESTATUS_SUCCESS (0). If writing object to stream
          fails, value ESTATUS_WRITING_OBJ_FAILED is returned. Assume that all nonzero values
          indicate an error.

****************************************************************************************************
*/
eStatus eObject::write(
    eStream *stream,
    os_int sflags)
{
    eObject *child;
    os_long n_attachements;

    /* Write class identifier, object identifier and persistant object flags.
     */
    if (*stream << classid()) goto failed;
    if (*stream << oid()) goto failed;
    if (*stream << flags() & (EOBJ_SERIALIZATION_MASK)) goto failed;

    /* Calculate and write number of attachments.
       HERE WE SHOULD USE HANDLES FOR SPEED
     */
    n_attachements = 0;
    for (child = first(EOID_ALL); child; child = child->next(EOID_ALL))
    {
        if (child->isserattachment()) n_attachements++;
    }
    if (*stream << n_attachements) goto failed;

    /* Write the object content.
     */
    if (writer(stream, sflags)) goto failed;

    /* Write attachments.
       HERE WE SHOULD USE HANDLES FOR SPEED
     */
    for (child = first(EOID_ALL); child; child = child->next(EOID_ALL))
    {
        if (child->isserattachment())
        {
            if (child->write(stream, sflags)) goto failed;
        }
    }

    /* Object succesfully written.
     */
    return ESTATUS_SUCCESS;

    /* Writing object failed.
     */
failed:
    return ESTATUS_WRITING_OBJ_FAILED;
}


/**
****************************************************************************************************

  @brief Read object from stream.

  The eObject::read() function reads class information, etc from the stream, creates new
  child object and reads child object content and attachments.

  @param  stream The stream to write to.
  @param  sflags Serialization flags. EOBJ_SERIALIZE_DEFAULT

  @return If successfull the function returns pointer to te new child object.
          If reading object from stream fails, value OS_NULL is returned.

****************************************************************************************************
*/
eObject *eObject::read(
    eStream *stream,
    os_int sflags)
{
    os_int cid, oid, oflags;
    os_long n_attachements, i;
    eObject *child;

    /* Read class identifier, object identifier, persistant object flags
       and number of attachments.
     */
    if (*stream >> cid) goto failed;
    if (*stream >> oid) goto failed;
    if (*stream >> oflags) goto failed;
    if (*stream >> n_attachements) goto failed;

    /* Generate new object.
     */
    child = newchild(cid, oid);
    if (child == OS_NULL) goto failed;

    /* Set flags.
     */
    child->setflags(oflags);

    /* Read the object content.
     */
    if (child->reader(stream, sflags)) goto failed;

    /* Read attachments.
     */
    for (i = 0; i<n_attachements; i++)
    {
        if (read(stream, sflags) == OS_NULL) goto failed;
    }

    /* Object succesfully read, return pointer to it.
     */
    return child;

    /* Reading object failed.
     */
failed:
    return OS_NULL;
}


