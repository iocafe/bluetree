/**

  @file    eprotocol_handle_iocom.cpp
  @brief   IOCOM protocol handle.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "extensions/iocom/eiocom.h"


/**
****************************************************************************************************
  Constructor.
****************************************************************************************************
*/
eioProtocolHandle::eioProtocolHandle(
    eObject *parent,
    e_oid oid,
    os_int flags)
    : eProtocolHandle(parent, oid, flags)
{
    os_memclear(&m_iocom, sizeof(m_iocom));
    m_is_iocom_end_point = OS_FALSE;
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
void eioProtocolHandle::setupclass()
{
    const os_int cls = ECLASSID_IOCOM_PROTOCOL_HANDLE;
    eVariable *p;

    /* Add the class to class list.
     */
    os_lock();
    eclasslist_add(cls, (eNewObjFunc)newobj, "eioProtocolHandle", ECLASSID_PROTOCOL_HANDLE);
    p = addpropertyb(cls, EPROHANDP_ISOPEN, eprohandp_isopen, "is open", EPRO_SIMPLE);
    p->setpropertys(EVARP_ATTR, "rdonly");
    propertysetdone(cls);
    os_unlock();
}
