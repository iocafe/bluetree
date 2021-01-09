/**

  @file    eprotocolhandle.cpp
  @brief   Abstract communication protocol handle.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "extensions/netservice/enetservice.h"

/* File system property names.
 */
const os_char
    eprotocolp_path[] = "path";

/**
****************************************************************************************************
  Constructor.
****************************************************************************************************
*/
eProtocolHandle::eProtocolHandle(
    eObject *parent,
    e_oid oid,
    os_int flags)
    : eObject(parent, oid, flags)
{
}


/**
****************************************************************************************************
  Virtual destructor.
****************************************************************************************************
*/
eProtocolHandle::~eProtocolHandle()
{
}


/**
****************************************************************************************************

  @brief Clone the object

  The eProtocolHandle::clone function clones an eMatrix.

  @param  parent Parent for the clone.
  @param  id Object identifier for the clone.
  @param  aflags 0 for default operation. EOBJ_NO_MAP not to map names.
  @return Pointer to the clone.

****************************************************************************************************
*/
eObject *eProtocolHandle::clone(
    eObject *parent,
    e_oid id,
    os_int aflags)
{
    eProtocolHandle *clonedobj;

    clonedobj = new eProtocolHandle(parent, id == EOID_CHILD ? oid() : id, flags());

    clonegeneric(clonedobj, aflags);
    return clonedobj;
}


/**
****************************************************************************************************

  @brief Add the class to class list and class'es properties to it's property set.

  The eVariable::setupclass function adds the class to class list and class'es properties to
  it's property set. The class list enables creating new objects dynamically by class identifier,
  which is used for serialization reader functions. The property set stores static list of
  class'es properties and metadata for those.

****************************************************************************************************
*/
void eProtocolHandle::setupclass()
{
    const os_int cls = ECLASSID_PROTOCOL_HANDLE;

    /* Add the class to class list.
     */
    os_lock();
    eclasslist_add(cls, (eNewObjFunc)newobj, "eProtocolHandle");
    os_unlock();
}
