/**

  @file    emaintain_thread.cpp
  @brief   Thread to maintain end points and connections.
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

/* Property names.
 */
const os_char
    enetmaintainp_publish[] = "publish",
    enetmaintainp_config_counter[] = "epconfigcnt",
    enetmaintainp_connect[] = "connect";

/**
****************************************************************************************************
  Constructor.
****************************************************************************************************
*/
eNetMaintainThread::eNetMaintainThread(
    eObject *parent,
    e_oid oid,
    os_int flags)
    : eThread(parent, oid, flags)
{
    m_netservice = OS_NULL;
    m_publish_count = -1;
    m_publish = OS_FALSE;
    m_end_points = new eContainer(this);
    m_end_points->setflags(EOBJ_PERSISTENT_CALLBACK);
    m_protocols = new eContainer(this, EOID_ITEM, EOBJ_IS_ATTACHMENT);
    m_protocols->ns_create();
    m_end_point_config_count = 0;
    m_connect_count = -1;
    m_connect = OS_FALSE;
    m_connections = new eContainer(this);

    initproperties();
}


/**
****************************************************************************************************
  Virtual destructor.
****************************************************************************************************
*/
eNetMaintainThread::~eNetMaintainThread()
{
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
void eNetMaintainThread::setupclass()
{
    const os_int cls = ECLASSID_NET_MAINTAIN_CLIENT;

    /* Add the class to class list.
     */
    os_lock();
    eclasslist_add(cls, (eNewObjFunc)newobj, "eNetMaintainThread");
    addpropertyl(cls, ENETMAINTAINP_PUBLISH, enetmaintainp_publish,
        -1, "update end points", EPRO_DEFAULT);
    addpropertyl(cls, ENETMAINTAINP_CONFIG_COUNTER, enetmaintainp_config_counter,
        0, "end point configuration trigger", EPRO_DEFAULT);
    addpropertyl(cls, ENETMAINTAINP_CONNECT, enetmaintainp_connect,
        -1, "update connections", EPRO_DEFAULT);
    propertysetdone(cls);
    os_unlock();
}


/**
****************************************************************************************************

  @brief Function to process incoming messages.

  The eNetMaintainThread::onmessage function handles messages received by object. If this function
  doesn't process message, it calls parent class'es onmessage function.

  Send UDP broadcasts by timer hit.

  @param   envelope Message envelope. Contains command, target and source paths and
           message content, etc.
  @return  None.

****************************************************************************************************
*/
void eNetMaintainThread::onmessage(
    eEnvelope *envelope)
{
    /* If at final destination for the message.
     */
    if (*envelope->target()=='\0')
    {
        switch (envelope->command())
        {
            case ECMD_TIMER: /* No need to do anything, timer is used just to break event wait */
                return;

            default:
                break;
        }
    }

    /* Call parent class'es onmessage.
     */
    eThread::onmessage(envelope);
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
eStatus eNetMaintainThread::onpropertychange(
    os_int propertynr,
    eVariable *x,
    os_int flags)
{
    os_int count;

    switch (propertynr)
    {
        case ENETMAINTAINP_PUBLISH:
            count = x->geti();
            if (count != m_publish_count) {
                m_publish_count = count;
                m_publish = OS_TRUE;
                os_get_timer(&m_publish_timer);
                timer(100);
            }
            break;

        case ENETMAINTAINP_CONNECT:
            count = x->geti();
            if (count != m_connect_count) {
                m_connect_count = count;
                m_connect = OS_TRUE;
                os_get_timer(&m_connect_timer);
                timer(100);
            }
            break;

        default:
            return eObject::onpropertychange(propertynr, x, flags);
    }

    return ESTATUS_SUCCESS;
}


/* Overloaded eThread function to initialize new thread. Called after eNetMaintainThread object is created.
 */
void eNetMaintainThread::initialize(
    eContainer *params)
{
}


/* Overloaded eThread function to perform thread specific cleanup when thread exists: Release
   resources allocated for maintain client. This is a "pair" to initialize function.
 */
void eNetMaintainThread::finish()
{
}


/**
****************************************************************************************************

  @brief Maintain connections and end points, thread main loop.

  The eNetMaintainThread::run() function
  The data is collected from "connect" and "endpoint" tables, both these tables are global
  and owned by eProcess.

****************************************************************************************************
*/
void eNetMaintainThread::run()
{
    eContainer *ep, *next_ep;
    eContainer *con, *next_con;

    while (OS_TRUE)
    {
        alive(EALIVE_RETURN_IMMEDIATELY);
        if (exitnow()) {
            break;
        }

        if (m_publish) if (os_has_elapsed(&m_publish_timer, 100))
        {
            maintain_end_points();
            timer(0);
            m_publish = OS_FALSE;
        }

        if (m_connect) if (os_has_elapsed(&m_connect_timer, 100))
        {
            maintain_connections();
            timer(0);
            m_connect = OS_FALSE;
        }
    }

    /* Connections and end points are closed here explicitely to make sure
       that os_lock() doesn't cause deadlock.
     */
    for (con = m_connections->firstc(); con; con = next_con)
    {
        next_con = con->nextc();
        delete_con(con);
    }
    for (ep = m_end_points->firstc(); ep; ep = next_ep)
    {
        next_ep = ep->nextc();
        delete_ep(ep);
    }
}






/**
****************************************************************************************************

  @brief Process a callback from a child object.

  The ePersistent::oncallback function

****************************************************************************************************
*/
eStatus eNetMaintainThread::oncallback(
    eCallbackEvent event,
    eObject *obj,
    eObject *appendix)
{
    // os_int nr;

    switch (event)
    {
        case ECALLBACK_STATUS_CHANGED:
            if (appendix == OS_NULL) break;
            // nr = appendix->oid();
            if (obj == m_end_points) {
                ep_status_changed(eContainer::cast(appendix));
            }
            break;

        default:
            break;
    }

    /* If we need to pass callback to parent class.
     */
    if (flags() & (EOBJ_PERSISTENT_CALLBACK|EOBJ_TEMPORARY_CALLBACK)) {
        eThread::oncallback(event, obj, appendix);
    }

    return ESTATUS_SUCCESS;
}


void eNetMaintainThread::add_protocol(
    eProtocol *proto)
{
    proto->adopt(m_protocols);
    proto->addname(proto->protocol_name());
}


eProtocol *eNetMaintainThread::protocol_by_name(
    eVariable *proto_name)
{
    return (eProtocol*)m_protocols->byname(proto_name->gets());
}


/**
****************************************************************************************************

  @brief Create "LAN services" table.

  The enet_start_maintain_thread function adds maintain service class, creates a light house
  thread object and starts to run it. This function is called by eNetService::start().

****************************************************************************************************
*/
void enet_start_maintain_thread(
    eNetService *netservice,
    os_int flags,
    eThreadHandle *maintain_thread_handle)
{
    eNetMaintainThread *maintain;
    eProtocol *proto;
    const os_char netservice_name[] = "//netservice";

    /* Create and start thread to listen for maintain UDP multicasts,
       name it "_maintain" in process name space.
     */
    maintain = new eNetMaintainThread();
    maintain->addname("//_netmaintain");
    maintain->set_netservice(netservice);
    maintain->bind(ENETMAINTAINP_PUBLISH, netservice_name,
        enetservp_endpoint_table_change_counter);
    maintain->bind(ENETMAINTAINP_CONFIG_COUNTER, netservice_name,
        enetservp_endpoint_config_counter, EBIND_CLIENTINIT);
    maintain->bind(ENETMAINTAINP_CONNECT, netservice_name,
        enetservp_connect_table_change_counter);

    while ((proto = (eProtocol*)netservice->protocols()->first())) {
        maintain->add_protocol(proto);
    }
    maintain->start(maintain_thread_handle);
}

