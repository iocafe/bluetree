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
#include "extensions/eiocom/eiocom.h"


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
    m_isrunning = OS_FALSE;
}


/**
****************************************************************************************************
  Virtual destructor.
****************************************************************************************************
*/
eioProtocolHandle::~eioProtocolHandle()
{
    terminate_thread();
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
    eclasslist_add(cls, (eNewObjFunc)newobj, "eioProtocolHandle");
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

  Notice for change logging: Previous value is still valid when this function is called.
  You can get the old value by calling property() function inside onpropertychange()
  function.

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



/**
****************************************************************************************************

  @brief Start a connection or end point thread.

  The eioProtocolHandle::start_thread names and starts running pre created eThread object as
  separate thread.

  @param   t Thread object to start.
  @param   threadname Name to give to thread in process name space.

****************************************************************************************************
*/
void eioProtocolHandle::start_thread(
    eThread *t,
    const os_char *threadname)
{
}


/**
****************************************************************************************************

  @brief Terminate connection or end point thread.

  If there is a connection or end point running for the protocol handle, the function
  sends terminate request to the thread and waits for the thread to exit.

****************************************************************************************************
*/
void eioProtocolHandle::terminate_thread()
{
}


/**
****************************************************************************************************

  @brief Get unique name of a connection or end point.

  The eioProtocolHandle::uniquename() function returns unique name of a connection or end point.
  The unique name is typically used to set or modify properties of running object.

  @return  Unique name of connection or end point object in process name space. OS_NULL if
           there is no such thread running for the protocol handle.

****************************************************************************************************
*/
const os_char *eioProtocolHandle::uniquename()
{
    return OS_NULL;
}

