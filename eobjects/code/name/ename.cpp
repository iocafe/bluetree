/**

  @file    ename.cpp
  @brief   Name class.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  The names map to name space. These are used to represent tree structure of named objects.
  The eName class is derived from eVariable class.

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
eName::eName(
    eObject *parent,
    e_oid id,
    os_int flags)
    : eVariable(parent, id, flags)
{
    /* Clear member variables to initial state.
     */
    clear_members();
    m_ns_type = ENAME_PARENT_NS;
    m_namespace_id = OS_NULL;

    /* If this is name space.
     */
    if (id == EOID_NAME)
    {
        /* Flag this as attachment.
         */
        setflags(EOBJ_IS_ATTACHMENT);
    }
}


/**
****************************************************************************************************

  @brief Virtual destructor.

  X...

  @return  None.

****************************************************************************************************
*/
eName::~eName()
{
    /* Detach name from name space.
     */
    detach();
}


/**
****************************************************************************************************

  @brief Clone object

  The clone function clones the variable and clonable attachments. Names will be left detached
  in clone if EOBJ_NO_MAP flag is given.

  @param  parent Parent for the clone.
  @param  id Object identifier for the clone.
  @param  aflags 0 for default operation. EOBJ_NO_MAP not to map names.
  @return Pointer to the clone.

****************************************************************************************************
*/
eObject *eName::clone(
    eObject *parent,
    e_oid id,
    os_int aflags)
{
    eName *clonedobj;
    clonedobj = new eName(parent, id == EOID_CHILD ? oid() : id, flags());

    /* Copy variable value.
     */
    clonedobj->setv(this);
    clonedobj->setdigs(digs());

    /* Copy name space type/identifier.
     */
    if (m_namespace_id) {
        clonedobj->setnamespaceid(namespaceid());
    }
    clonedobj->m_ns_type = m_ns_type;

    /* Copy clonable attachments.
     */
    clonegeneric(clonedobj, aflags);
    return clonedobj;
}


/**
****************************************************************************************************

  @brief Add eName to class list and class'es properties to it's property set.

  The eName::setupclass function adds eName to class list and class'es properties to
  it's property set. The class list enables creating new objects dynamically by class identifier,
  which is used for serialization reader functions. The property set stores static list of
  class'es properties and metadata for those.

****************************************************************************************************
*/
void eName::setupclass()
{
    const os_int cls = ECLASSID_NAME;

    /* Add the class to class list.
     */
    os_lock();
    eclasslist_add(cls, (eNewObjFunc)newobj, "eName");
    eVariable::setupproperties(cls);
    propertysetdone(cls);
    os_unlock();
}


/**
****************************************************************************************************

  @brief Initialize member variables to known state.
  @return  None.

****************************************************************************************************
*/
void eName::clear_members()
{
    m_ileft = m_iright = m_iup = OS_NULL;
    m_namespace = OS_NULL;
    m_is_process_ns = OS_FALSE;
    ixsetred();
}


/**
****************************************************************************************************

  @brief Get next child name identified by oid.

  The eVariable::nextn() function returns pointer to the name of this object.

  @param   id Object idenfifier. Default value EOID_CHILD specifies to count a child objects,
           which are not flagged as an attachment. Value EOID_ALL specifies to get count all
           child objects, regardless wether these are attachment or not. Other values
           specify object identifier, only children with that specified object identifier
           are searched for.

  @return  Pointer to object's next name, or OS_NULL if none found.

****************************************************************************************************
*/
eName *eName::nextn(
    e_oid id)
{
    if (mm_handle == OS_NULL) return OS_NULL;
    eHandle *h = mm_handle->next(id);
    while (h)
    {
        if (h->object()->classid() == ECLASSID_NAME)
            return eName::cast(h->object());

        h = h->next(id);
    }
    return OS_NULL;
}


/**
****************************************************************************************************

  @brief Write name to stream.

  The eName::writer() function serializes eName to stream. This writes only content,
  use eObject::write() to save also class information, attachements, etc.

  @param  stream The stream to write to.
  @param  flags Serialization flags.

  @return If successfull the function returns ESTATUS_SUCCESS (0). If writing object to stream
          fails, value ESTATUS_WRITING_OBJ_FAILED is returned. Assume that all nonzero values
          indicate an error.

****************************************************************************************************
*/
eStatus eName::writer(
    eStream *stream,
    os_int flags)
{
    /* Version number. Increment if new serialized items are to the object,
       and check for new version's items in read() function.
     */
    const os_int version = 0;

    /* Begin the object and write version number.
     */
    if (stream->write_begin_block(version)) goto failed;

    /* Use base class'es function to do the work.
     */
    eVariable::writer(stream, flags);

    /* End the object.
     */
    if (stream->write_end_block()) goto failed;

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

  @brief Read name from stream.

  The eName::reader() function reads serialized name from stream. This function reads only content.
  To read whole object including attachments, names, etc, use eObject::read().

  @param  stream The stream to read from.
  @param  flags Serialization flags.

  @return If successfull the function returns ESTATUS_SUCCESS (0). If writing object to stream
          fails, value ESTATUS_READING_OBJ_FAILED is returned. Assume that all nonzero values
          indicate an error.

****************************************************************************************************
*/
eStatus eName::reader(
    eStream *stream,
    os_int flags)
{
    /* Version number. Used to check which versions item's are in serialized data.
     */
    os_int version;

    /* Read object start mark and version number.
     */
    if (stream->read_begin_block(&version)) goto failed;

    /* Use base class'es function to do the work.
     */
    eVariable::reader(stream, flags);

    /* End the object.
     */
    if (stream->read_end_block()) goto failed;

    /* Object succesfully read.
     */
    return ESTATUS_SUCCESS;

    /* Reading object failed.
     */
failed:
    return ESTATUS_READING_OBJ_FAILED;
}


/**
****************************************************************************************************

  @brief Get the next name in same namespace.

  The eName::ms_next() function returns pointer to next name in namespace.

  @param   name_match OS_TRUE (default) to get next name only if it is same name as this object.
           OS_FALSE to get next name in name space, regardless of the value.

  @return  Pointer to the next name within the name space, or OS_NULL if no more names.

****************************************************************************************************
*/
eName *eName::ns_next(
    os_boolean name_match)
{
    eName
        *n,
        *m;

    n = this;

    if (n->m_iright)
    {
        n = n->m_iright;
        while (n->m_ileft) n = n->m_ileft;

        if (!name_match) return n;

        return compare(n) ? OS_NULL : n;
    }
    else
    {
        m = n->m_iup;

        while (OS_TRUE)
        {
            if (m == OS_NULL) return OS_NULL;
            if (m->m_ileft == n) break;
            n = m;
            m = n->m_iup;
        }

        if (!name_match) return m;

        return compare(m) ? OS_NULL : m;
    }
}


/**
****************************************************************************************************

  @brief Get the previous name in same namespace.

  The eName::ms_prev() function returns pointer to previous name in namespace.

  @param   name_match OS_TRUE (default) to get prev name only if it is same name as this object.
           OS_FALSE to get prev name in name space, regardless of the value.

  @return  Pointer to the prev name in same namespace, or OS_NULL if no more matching names.

****************************************************************************************************
*/
eName *eName::ns_prev(
    os_boolean name_match)
{
    eName
        *n,
        *m;

    n = this;

    if (n->m_ileft)
    {
        n = n->m_ileft;
        while (n->m_iright) n = n->m_iright;

        if (!name_match) return n;

        return compare(n) ? OS_NULL : n;
    }
    else
    {
        m = n->m_iup;

        while (OS_TRUE)
        {
            if (m == OS_NULL) return OS_NULL;
            if (m->m_iright == n) break;
            n = m;
            m = n->m_iup;
        }

        if (!name_match) return m;

        return compare(m) ? OS_NULL : m;
    }
}


/**
****************************************************************************************************

  @brief Get name space identifier, if any, for the name.

  The eName::namespaceid function returns name space identifier as string. The name space
  identifier tells to which name space the name belongs to.

  @return  Name space identifier string.

****************************************************************************************************
*/
const os_char *eName::namespaceid()
{
    const os_char
        *namespace_id;

    switch (m_ns_type)
    {
        default:
        case ENAME_PARENT_NS:
            namespace_id = eobj_parent_ns;
            break;

        case ENAME_PROCESS_NS:
            namespace_id = eobj_process_ns;
            break;

        case ENAME_THREAD_NS:
            namespace_id = eobj_thread_ns;
            break;

        case ENAME_THIS_NS:
            namespace_id = eobj_this_ns;
            break;

        case ENAME_SPECIFIED_NS:
            namespace_id = m_namespace_id->gets();
            break;
    }

    return namespace_id;
}

/**
****************************************************************************************************

  @brief Set name space identifier, if any, for the name.

  The eName::setnamespaceid() function...

  @return None.

****************************************************************************************************
*/
void eName::setnamespaceid(
    const os_char *namespace_id)
{
    /* Clear old stuff if any
     */
    m_ns_type = ENAME_PARENT_NS;
    if (m_namespace_id)
    {
        delete m_namespace_id;
        m_namespace_id = OS_NULL;
    }

    if (namespace_id)
    {
        if (!os_strcmp(namespace_id, E_PROCESS_NS))
        {
            m_ns_type = ENAME_PROCESS_NS;
        }
        else if (!os_strcmp(namespace_id, E_THREAD_NS))
        {
            m_ns_type = ENAME_THREAD_NS;
        }
        else if (!os_strcmp(namespace_id, E_PARENT_NS))
        {
            m_ns_type = ENAME_PARENT_NS;
        }
        else if (!os_strcmp(namespace_id, eobj_this_ns))
        {
            m_ns_type = ENAME_THIS_NS;
        }
        else
        {
            m_ns_type = ENAME_SPECIFIED_NS;
            m_namespace_id = new eVariable(this, EOID_CHILD, EOBJ_TEMPORARY_ATTACHMENT);
            m_namespace_id->sets(namespace_id);
        }
    }
}


/**
****************************************************************************************************

  @brief Map the name to a name space.

  The eName::mapname() function maps name to name space.

  @return  ESTATUS_SUCCESS if successfull, other values indicate an error.

****************************************************************************************************
*/
eStatus eName::mapname()
{
    eNameSpace
        *ns;

    os_int
        info;

    /* If this name is mapped already, do nothing.
     */
    if (m_namespace) return ESTATUS_NAME_ALREADY_MAPPED;

    /* Find name space to map to. If none, return error.
     */
    ns = findnamespace(namespaceid(), &info);
    if (ns == OS_NULL) return ESTATUS_NAME_MAPPING_FAILED;

    return mapname2(ns, info);
}


/**
****************************************************************************************************

  @brief Map the name to a name space.

  The eName::mapname2() function maps name to name space given as argument.

  @return  ESTATUS_SUCCESS if successfull, other values indicate an error.

****************************************************************************************************
*/
eStatus eName::mapname2(
    eNameSpace *ns,
    os_int info)
{
    /* If this name is mapped already, do nothing.
     */
    if (m_namespace) return ESTATUS_NAME_ALREADY_MAPPED;

     /* Save pointer to name space.
     */
    m_namespace = ns;
    m_is_process_ns = (info & E_INFO_PROCES_NS) ? OS_TRUE : OS_FALSE;

    /* If process name space, synchronize.
     */
    if (m_is_process_ns) os_lock();

    /* Insert name to name space's red black tree.
     */
    ns->ixrbtree_insert(this);

    /* Finish with syncronization and return.
     */
    if (m_is_process_ns) os_unlock();
    return ESTATUS_SUCCESS;
}


/**
****************************************************************************************************

  @brief Detach name from name space.

  The eName::detach() function...

  @return  None.

****************************************************************************************************
*/
void eName::detach()
{
    /* If this name is not mapped, do nothing.
     */
    if (m_namespace == OS_NULL) return;

    /* If process name space, synchronize.
     */
    if (m_is_process_ns) os_lock();

    /* Insert name to name space's red black tree.
     */
    m_namespace->ixrbtree_remove(this);

    /* Finish with syncronization.
     */
    if (m_is_process_ns) os_unlock();

    /* Clear member variables to initial state.
     */
    clear_members();
}
