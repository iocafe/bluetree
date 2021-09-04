/**

  @file    eprotocol_handle_switchbox.cpp
  @brief   SWITCHBOX protocol handle.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

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
    ioc_initialize_switchbox_root(&m_switchbox, 0 /* flags */);
    os_memclear(&m_epoint, sizeof(m_epoint));
    m_end_point_initialized = OS_FALSE;
    oixstr(m_path_to_self, sizeof(m_path_to_self));
}


/**
****************************************************************************************************
  Destructor.
****************************************************************************************************
*/
esboxProtocolHandle::~esboxProtocolHandle()
{
    close_endpoint();
    ioc_release_switchbox_root(&m_switchbox);
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


/* Listen for TCP port for switchbox connections.
 */
eStatus esboxProtocolHandle::listen(
    switchboxEndPointParams *prm)
{
    osalStatus ss;

    if (m_end_point_initialized) {
        close_endpoint();
    }

    ioc_initialize_switchbox_end_point(&m_epoint, &m_switchbox);
    ioc_set_switchbox_end_point_callback(&m_epoint, end_point_callback, this);
    m_end_point_initialized = OS_TRUE;

    ss = ioc_switchbox_listen(&m_epoint, prm);
    return ESTATUS_FROM_OSAL_STATUS(ss);
}


/**
****************************************************************************************************

  @brief Callback when an end point is actually listening, or dropped.

  The SWITCHBOX library calls thus function to inform application about successfully initialized and
  dropped end points. This function sets esboxProtocolHandle's "isopen" property.

  It uses complex way to set property:  we are now called by different thread which doesn't
  own esboxProtocolHandle and thus must not property directly. But path_to_self is simple C
  string, which is set when esboxProtocol handle is created and unchanged after that. It
  can be used by other threads as long as the protocol handle exists.

  @param   epoint Pointer to SWITCHBOX connection object.
  @param   event Reason for the callback, either IOC_CONNECTION_ESTABLISHED or
           IOC_CONNECTION_DROPPED.
  @param   context Callback context, here pointer to protocol handle.

****************************************************************************************************
*/
void esboxProtocolHandle::end_point_callback(
    struct switchboxEndPoint *epoint,
    switchboxEndPointEvent event,
    void *context)
{
    eProcess *process;
    os_boolean value;
    esboxProtocolHandle *p;
    OSAL_UNUSED(epoint);

    p = (esboxProtocolHandle*)context;
    switch (event) {
        case IOC_END_POINT_LISTENING:
            value = OS_TRUE;
            break;

        case IOC_END_POINT_DROPPED:
            value = OS_FALSE;
            break;

        default:
            return;
    }

    os_lock();
    process = eglobal->process;
    process->setpropertyl_msg(p->path_to_self(), value, eprohandp_isopen);
    os_unlock();
}


/* Finished with end point, at least for now. Close it and release resources.
 */
void esboxProtocolHandle::close_endpoint()
{
    if (m_end_point_initialized) {
        ioc_set_switchbox_end_point_callback(&m_epoint, OS_NULL, OS_NULL);

        while (ioc_terminate_switchbox_end_point_thread(&m_epoint) == OSAL_PENDING) {
            os_timeslice();
        }
        ioc_release_switchbox_end_point(&m_epoint);

        setpropertyi(EPROHANDP_ISOPEN, OS_FALSE);
        m_end_point_initialized = OS_FALSE;
    }
}


