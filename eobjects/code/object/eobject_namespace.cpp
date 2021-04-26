/**

  @file    eobject_namespace.cpp
  @brief   eObject's name and name space related functions.
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

/* Name space identifiers as static strings. eobj_this_ns is default
   for ns_first and ns_firstv functions()
 */
const os_char eobj_process_ns[] = E_PROCESS_NS;
const os_char eobj_thread_ns[] = E_THREAD_NS;
const os_char eobj_parent_ns[] = E_PARENT_NS;
const os_char eobj_this_ns[] = E_THIS_NS;
const os_char eobj_no_ns[] = E_NO_NS;



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

  @brief Create name space for this object.

  The eObject::ns_create() function creates a name space for object. If object already
  has name space with matching ID, the function does nothing. If namespace with different
  exists, old one is deleted and new one created.

  @param   namespace_id Identifier for the name space.
  @return  None.

****************************************************************************************************
*/
void eObject::ns_create(
    const os_char *namespace_id)
{
    eNameSpace *ns;
    const os_char *nsid1, *nsid2;

    /* If object has already name space.
     */
    ns = eNameSpace::cast(first(EOID_NAMESPACE));
    if (ns)
    {
        nsid1 = namespace_id;
        if (nsid1 == OS_NULL) {
            nsid1 = "";
        }
        nsid2 = "";
        if (ns->namespaceid()) {
            nsid2 = ns->namespaceid()->gets();
        }

        /* If namespace identifier matches, just return.
         */
        if (!os_strcmp(nsid1, nsid2)) {
            return;
        }

        /* Delete old name space.
         */
        delete ns;
    }

    /* Create name space.
     */
    ns = new eNameSpace(this, EOID_NAMESPACE);
    if (namespace_id)
    {
        ns->setnamespaceid(namespace_id);
    }

    /* Remap names in child objects? Do we need this? In practise name space is created
       before placing children in?
     */
    /* if ((aflags & EOBJ_NO_MAP) == 0)
    {
        map(E_ATTACH_NAMES);
    } */
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
                if (!os_strcmp(namespace_id, eobj_no_ns)) {
                    return OS_NULL;
                }

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
           - ENAME_PERSISTENT   0
           - ENAME_TEMPORARY    0x00000001
           - ENAME_PRIMARY      0x00000002
           - ENAME_UNIQUE       0x00000004
           - ENAME_PARENT_NS    0
           - ENAME_PROCESS_NS   0x00000008
           - ENAME_THREAD_NS    0x00000010
           - ENAME_SPECIFIED_NS 0x00000020
           - ENAME_NO_NS        0x00000040
           - ENAME_THIS_NS      0x00000080
           - ENAME_NO_MAP       0x00001000

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
    if (flags & ENAME_TEMPORARY) {
        n->setflags(EOBJ_NOT_CLONABLE|EOBJ_NOT_SERIALIZABLE);
    }
    if (flags & ENAME_UNIQUE) {
        n->setflags(EOBJ_UNIQUE_NAME);
    }
    if (flags & ENAME_PRIMARY) {
        n->setflags(EOBJ_PRIMARY_NAME);
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
        else if (flags & ENAME_NO_NS)
        {
            namespace_id = eobj_no_ns;
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


/* Adds integer value as name to parent name space.
   This can be used to index large tables, typically by time stamp
 */
eName *eObject::addintname(
    os_long x,
    os_int flags)
{
    eName *n;

    /* Create name object and set integer value to it.
     */
    n = new eName(this, EOID_NAME);
    n->setl(x);

    /* Set flags for name, like persistancy.
     */
    if (flags & ENAME_TEMPORARY) {
        n->setflags(EOBJ_NOT_CLONABLE|EOBJ_NOT_SERIALIZABLE);
    }

    /* Set name space identifier.
     */
    n->setnamespaceid(eobj_parent_ns);

    /* Map name to namespace, unless disabled for now.
     */
    if ((flags & ENAME_NO_MAP) == 0)
    {
        n->mapname();
    }

    return n;
}

/**
****************************************************************************************************

  @brief Find object's first name.

  The eObject::primaryname() function gets first name given to this object.
  Object's first name may later on separated with alternate names with name flags. At this
  fime first name is simply name which was added first.

  @param   nstype -1 to return name for any name space, to select the name space, sete
           one of: ENAME_PARENT_NS, ENAME_PROCESS_NS, ENAME_THREAD_NS, ENAME_SPECIFIED_NS,
           ENAME_NO_NS, or ENAME_THIS_NS.

  @return  Pointer to first name (eName), or OS_NULL if none found.

****************************************************************************************************
*/
eName *eObject::primaryname(
    os_int nstype)
{
    eName *n;

    for (n = firstn(); n; n = n->nextn())
    {
        if (nstype == -1) {
            return n;
        }
        else if (nstype == n->ns_type())
        {
            return n;
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
    eObject *namedobj;
    os_int info;

    name = eName::cast(handle->m_object);
    namedobj = name->parent();
    if (namedobj) {
        ns = namedobj->findnamespace(name->namespaceid(), &info, this);
    }
    else {
        ns = OS_NULL;
    }

    if ((mflags & E_ATTACH_NAMES))
    {
        if (ns) {
            osal_debug_assert(ns);
            name->mapname2(ns, info);
        }
    }

    if (mflags & E_DETACH_FROM_NAMESPACES_ABOVE)
    {
        if (info & E_INFO_ABOVE_CHECKPOINT) {
            name->detach();
        }
    }
}


/**
****************************************************************************************************

  @brief Get object by name.

  The eObject::byname() function looks for name in this object's name space.

  @param   name Name to look for.
  @param   name_match OS_TRUE (default) to get next object pointer only if name x given as argument
           matches to name. OS_FALSE to get first object with name greater or equal to x argument.
  @return  If name is found, returns pointer to the named object. Otherwise if the function
           it returns OS_NULL.

****************************************************************************************************
*/
eObject *eObject::byname(
    const os_char *name,
    os_boolean name_match)
{
    eVariable namev;
    eName *nobj;
    eNameSpace *nspace;

    nspace = eNameSpace::cast(first(EOID_NAMESPACE));
    if (nspace)
    {
        namev.sets(name);
        nobj = nspace->findname(&namev, name_match);
        if (nobj) return nobj->parent();
    }
    return OS_NULL;
}


/**
****************************************************************************************************

  @brief Get object by integer name.

  The eObject::byname() function looks for integer name in this object's name space.
  Integer names are used to index data.

  String names are not mixed with integer names.

  @param   x Integer name value to look for.
  @param   name_match OS_TRUE (default) to get next object pointer only if name x given as argument
           matches to name. OS_FALSE to get first object with name greater or equal to x argument.
  @return  If name is found, returns pointer to "named" object. If name is not found, the function
           returns OS_NULL.

****************************************************************************************************
*/
eObject *eObject::byintname(
    os_long x,
    os_boolean name_match)
{
    eVariable namev;
    eName *nobj;
    eNameSpace *nspace;

    nspace = eNameSpace::cast(first(EOID_NAMESPACE));
    if (nspace)
    {
        namev.setl(x);
        nobj = nspace->findname(&namev, name_match);

        /* Move on until integer name found. Skip string names.
         */
        while (nobj) {
            if (nobj->type() == OS_LONG) {
                return nobj->parent();
            }
            nobj = nobj->ns_next(OS_FALSE);
        }
    }
    return OS_NULL;
}


