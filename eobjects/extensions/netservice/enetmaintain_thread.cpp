/**

  @file    emaintain_thread.cpp
  @brief   Thread to maintain end points and connections.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

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
    enetmp_end_pont_table_modif_count[] = "publish",
    enetmp_end_point_config_count[] = "epconfigcnt",
    enetmp_connect_table_modif_count[] = "connect",
    enetmp_lighthouse_change_count[] = "lighthouse";

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
    m_end_point_table_modif_count = -1;
    m_configure_end_points = OS_FALSE;
    m_end_point_config_timer = 0;
    m_end_point_config_count = 0;

    m_end_points = new eContainer(this);
    m_end_points->setflags(EOBJ_PERSISTENT_CALLBACK);
    m_protocols = new eContainer(this, EOID_ITEM, EOBJ_IS_ATTACHMENT);
    m_protocols->ns_create();
    m_connect_table_modif_count = -1;
    m_configure_connections = OS_FALSE;
    m_trigger_connect_check_by_lighthouse = OS_FALSE;
    m_connect_timer = 0;
    m_connections = new eContainer(this);
    m_connections->setflags(EOBJ_PERSISTENT_CALLBACK);
    m_connections->ns_create();
    m_timer_ms = 0;
    m_lighthouse_modif_count = 0;

    initproperties();
    ns_create();
    m_connections->addname("connections");
    m_end_points->addname("endpoints");
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
    eclasslist_add(cls, (eNewObjFunc)newobj, "eNetMaintainThread", ECLASSID_THREAD);
    addpropertyl(cls, ENETMP_END_POINT_TABLE_MODIF_COUNT, enetmp_end_pont_table_modif_count,
        -1, "end point table modif count", EPRO_DEFAULT);
    addpropertyl(cls, ENETMP_END_POINT_CONFIG_COUNT, enetmp_end_point_config_count,
        0, "end point config count", EPRO_NOONPRCH);
    addpropertyl(cls, ENETMP_CONNECT_TABLE_MODIF_COUNT, enetmp_connect_table_modif_count,
        -1, "connect table modif count", EPRO_DEFAULT);
    addpropertyl(cls, ENETMP_LIGHTHOUSE_CHANGE_COUNT, enetmp_lighthouse_change_count,
        0, "lighthouse change count", EPRO_DEFAULT);
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
        case ENETMP_END_POINT_TABLE_MODIF_COUNT:
            count = x->geti();
            if (count != m_end_point_table_modif_count) {
                m_end_point_table_modif_count = count;
                m_configure_end_points = OS_TRUE;
                os_get_timer(&m_end_point_config_timer);
                set_timer(100);
            }
            break;

        case ENETMP_CONNECT_TABLE_MODIF_COUNT:
            count = x->geti();
            if (count != m_connect_table_modif_count) {
                m_connect_table_modif_count = count;
                m_configure_connections = OS_TRUE;
                os_get_timer(&m_connect_timer);
                set_timer(100);
            }
            break;

        case ENETMP_LIGHTHOUSE_CHANGE_COUNT            :
            count = x->geti();
            if (count != m_lighthouse_modif_count) {
                m_lighthouse_modif_count = count;
                if (m_trigger_connect_check_by_lighthouse) {
                    m_configure_connections = OS_TRUE;
                    os_get_timer(&m_connect_timer);
                    set_timer(100);
                }
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
    create_socket_list_table();
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
        alive();
        if (exitnow()) {
            break;
        }

        if (m_configure_end_points) if (os_has_elapsed(&m_end_point_config_timer, 90))
        {
            maintain_end_points();
            m_configure_end_points = OS_FALSE;
        }

        if (m_configure_connections)
            if (os_has_elapsed(&m_connect_timer, 90) && m_netservice->m_connect_to_matrix)
        {
            merge_to_socket_list();
            maintain_connections();
            m_configure_connections = OS_FALSE;
        }

        if (!m_configure_end_points &&
            !m_configure_connections)
        {
            set_timer(0);
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
    switch (event)
    {
        case ECALLBACK_STATUS_CHANGED:
            if (appendix == OS_NULL) break;
            if (obj == m_end_points) {
                ep_status_changed(eContainer::cast(appendix));
            }
            else if (obj == m_connections) {
                con_status_changed(eContainer::cast(appendix));
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


/**
****************************************************************************************************

  @brief Set timer period, how often to recive timer messages.

  This function sets how oftern onmessage() is called with ECMD_TIMER argument by timer.
  Call this function instead of calling timer() directlt to avois repeated set or clear
  of the timer period.

  @param timer_ms Timer hit period in milliseconds, repeated. Set 0 to disable timer.

****************************************************************************************************
*/
void eNetMaintainThread::set_timer(
    os_int timer_ms)
{
    if (timer_ms != m_timer_ms) {
        m_timer_ms = timer_ms;
        timer(timer_ms);
    }
}


void eNetMaintainThread::add_protocol(
    eProtocol *proto)
{
    proto->adopt(m_protocols);
    proto->addname(proto->protocol_name());
    if (proto->protocol_name2()) {
        proto->addname(proto->protocol_name2());
    }
}


eProtocol *eNetMaintainThread::protocol_by_name(
    eVariable *proto_name)
{
    return (eProtocol*)m_protocols->byname(proto_name->gets());
}


/**
****************************************************************************************************

  @brief Create thread to main net service.

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
    maintain->bind(ENETMP_END_POINT_TABLE_MODIF_COUNT, netservice_name,
        enetservp_endpoint_table_change_counter);
    maintain->bind(ENETMP_END_POINT_CONFIG_COUNT, netservice_name,
        enetservp_endpoint_config_counter, EBIND_CLIENTINIT);
    maintain->bind(ENETMP_CONNECT_TABLE_MODIF_COUNT, netservice_name,
        enetservp_connect_table_change_counter);
    maintain->bind(ENETMP_LIGHTHOUSE_CHANGE_COUNT, netservice_name,
        enetservp_lighthouse_change_counter);

    while ((proto = (eProtocol*)netservice->protocols()->first())) {
        maintain->add_protocol(proto);
    }
    maintain->start(maintain_thread_handle);
}

