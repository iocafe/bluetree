/**

  @file    eprotocol_handle_switchbox.cpp
  @brief   SWITCHBOX protocol handle.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "extensions/switchbox/eswitchbox.h"


/**
****************************************************************************************************
  Constructor.
****************************************************************************************************
*/
esboxProtocolHandle::esboxProtocolHandle(
    eObject *parent,
    e_oid oid,
    os_int flags)
    : eProtocolHandle(parent, oid, flags)
{
    os_memclear(&m_switchbox, sizeof(m_switchbox));
    m_is_switchbox_end_point = OS_FALSE;
    oixstr(m_path_to_self, sizeof(m_path_to_self));
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
void esboxProtocolHandle::setupclass()
{
    const os_int cls = ECLASSID_SWITCHBOX_PROTOCOL_HANDLE;
    eVariable *p;

    /* Add the class to class list.
     */
    os_lock();
    eclasslist_add(cls, (eNewObjFunc)newobj, "esboxProtocolHandle", ECLASSID_PROTOCOL_HANDLE);
    p = addpropertyb(cls, EPROHANDP_ISOPEN, eprohandp_isopen, "is open", EPRO_SIMPLE);
    p->setpropertys(EVARP_ATTR, "rdonly");
    propertysetdone(cls);
    os_unlock();
}
