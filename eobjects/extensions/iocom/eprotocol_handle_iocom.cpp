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
    os_memclear(&m_iocom, sizeof(m_iocom)); //  eiocomStateStruct m_iocom;
    m_is_open = OS_FALSE;
    // m_isrunning = OS_FALSE;
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
    p = addpropertyb(cls, EPROHANDP_ISOPEN, eprohandp_isopen, OS_FALSE, "is open", EPRO_DEFAULT);
    p->setpropertys(EVARP_ATTR, "rdonly");
    propertysetdone(cls);
    os_unlock();
}


/**
****************************************************************************************************

  @brief Called to inform the class about property value change (override).

  The onpropertychange() function is called when class'es property changes, unless the
  property is flagged with EPRO_NOONPRCH.
  If property is flagged as EPRO_SIMPLE, this function shuold save the property value
  in class members and and return it when simpleproperty() is called.

  @param   propertynr Property number of changed property.
  @param   x Variable containing the new value.
  @param   flags
  @return  If successfull, the function returns ESTATUS_SUCCESS (0). Nonzero return values do
           indicate that there was no property with given property number.

****************************************************************************************************
*/
eStatus eioProtocolHandle::onpropertychange(
    os_int propertynr,
    eVariable *x,
    os_int flags)
{
    os_boolean is_open;

    switch (propertynr)
    {
        case EPROHANDP_ISOPEN:
            is_open = x->geti();
            if (is_open != m_is_open) {
                m_is_open = is_open;
                docallback(ECALLBACK_STATUS_CHANGED);
            }
            return ESTATUS_SUCCESS;

        default:
            break;
    }

    return eObject::onpropertychange(propertynr, x, flags);
}

os_boolean eioProtocolHandle::isrunning()
{
    return m_iocom.con.worker.thread_running;
}

