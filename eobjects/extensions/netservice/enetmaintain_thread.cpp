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
    enetmaintainp_publish[] = "publish";

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
    m_publish_count = 0;
    m_publish = OS_TRUE;
    m_end_points = new eContainer(this);
    m_protocols = new eContainer(this, EOID_ITEM, EOBJ_IS_ATTACHMENT);
    m_protocols->ns_create();

    // m_publish_status = ESTATUS_FAILED;
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
    addpropertyb(cls, ENETMAINTAINP_PUBLISH, enetmaintainp_publish,
        OS_FALSE, "update end points and connections", EPRO_DEFAULT);
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
#if 0
    /* If at final destination for the message.
     */
    if (*envelope->target()=='\0')
    {
        switch (envelope->command())
        {
            case ECMD_TIMER:
                if (m_publish_status == ESTATUS_SUCCESS) {
                    run_server();
                }
                return;

            default:
                break;
        }
    }
#endif
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
                maintain_end_points();
                m_publish_count = count;
                m_publish = OS_TRUE;
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

  @brief Maintain LAN service UDP communication, thread main loop.

  The eNetMaintainThread::run() function keeps eobjects messaging alive, sends and listens for
  light house UDP multicasts. This thread handles all maintain UDP communication.
  The data to be published (sent as UDP messages) is collected from "endpoint" table,
  and received data is stored in "LAN services" table. Both these tables are global
  and owned by eProcess.

****************************************************************************************************
*/
void eNetMaintainThread::run()
{
    while (OS_TRUE)
    {
        alive(EALIVE_RETURN_IMMEDIATELY);
        if (exitnow()) break;

os_sleep(100);
#if 0
        if (m_send_udp_multicasts) {
            if (m_publish) {
                if (!m_udp_send_initialized) {
                    m_udp_send_initialized = OS_TRUE;
                }

                m_publish_status = publish();
                m_publish = OS_FALSE;
                if (m_publish_status == ESTATUS_SUCCESS) {
                    run_server();
                    timer(4500);
                }
            }
        }
        else if (m_udp_send_initialized)
        {
            ioc_release_maintain_server(&m_server);
            m_udp_send_initialized = OS_FALSE;
            timer(0);
        }
#endif
    }
}


/**
****************************************************************************************************

  @brief Create and delete end points as needed.

  The eNetMaintainThread::maintain_end_points() function is collects data from endpoint table and
  sets up data end points for communication protocols.

  @return ESTATUS_SUCCESS if all is fine, oe ESTATUS_FAILED if notthing to publish.

****************************************************************************************************
*/
void eNetMaintainThread::maintain_end_points()
{
    eProtocol *proto;
    eProtocolHandle *handle;
    eMatrix *m;
    eObject *conf, *columns, *col;
    eContainer *localvars, *list, *ep, *next_ep;
    eVariable *v, *proto_name;
    os_int enable_col, protocol_col, transport_col, port_col;
    os_int h, ep_nr;
    eVariable tmp;
    eStatus s;

    localvars = new eContainer(ETEMPORARY);
    // list = new eContainer(localvars);
    // list->ns_create();

    os_lock();
    m = m_netservice->m_endpoint_matrix;
    conf = m->configuration();
    if (conf == OS_NULL) goto getout_unlock;
    columns = conf->first(EOID_TABLE_COLUMNS);
    if (columns == OS_NULL) goto getout_unlock;
    col = columns->byname(enet_endp_enable);
    if (col == OS_NULL) goto getout_unlock;
    enable_col = col->oid();
    col = columns->byname(enet_endp_protocol);
    if (col == OS_NULL) goto getout_unlock;
    protocol_col = col->oid();
    col = columns->byname(enet_endp_transport);
    if (col == OS_NULL) goto getout_unlock;
    transport_col = col->oid();
    col = columns->byname(enet_endp_port);
    if (col == OS_NULL) goto getout_unlock;
    port_col = col->oid();
    /* col = columns->byname(enet_endp_netname);
    if (col == OS_NULL) goto getout;
    netname_col = col->oid(); */
    os_unlock();

    /* Remove end points which are no longer needed or have changed.
     */
    for (ep = m_end_points->firstc(); ep; ep = next_ep)
    {
        next_ep = ep->nextc();
        ep_nr = ep->oid();
        proto_name = ep->firstv(ENET_ENDP_PROTOCOL);
        proto = protocol_by_name(proto_name);
        handle = eProtocolHandle::cast(ep->first(ENET_ENDP_PROTOCOL_HANDLE));
        if (!proto->is_end_point_running(handle)) continue;

        os_timeslice();
        os_lock();

        if ((m->geti(ep_nr, EMTX_FLAGS_COLUMN_NR) & EMTX_FLAGS_ROW_OK) == 0) goto delete_it;
        if (m->geti(ep_nr, enable_col) == 0) goto delete_it;
        m->getv(ep_nr, protocol_col, &tmp);
        if (tmp.compare(proto_name)) goto delete_it;
        v = ep->firstv(ENET_ENDP_TRANSPORT);
        m->getv(ep_nr, transport_col, &tmp);
        if (tmp.compare(v)) goto delete_it;
        v = ep->firstv(ENET_ENDP_PORT);
        m->getv(ep_nr, transport_col, &tmp);
        if (tmp.compare(v)) goto delete_it;

        os_unlock();
        continue;

delete_it:
        os_unlock();
        handle = eProtocolHandle::cast(ep->first(ENET_ENDP_PROTOCOL_HANDLE));
        if (proto->is_end_point_running(handle))
        {
            proto->delete_end_pont(handle);
            while (proto->is_end_point_running(handle)) {
                os_timeslice();
            }
        }
        delete ep;
    }

    /* Generate list of end points to add.
     */
    list = new eContainer(localvars);
    os_lock();
    h = m->nrows();
    for (ep_nr = 0; ep_nr < h; ep_nr ++) {
        if ((m->geti(ep_nr, EMTX_FLAGS_COLUMN_NR) & EMTX_FLAGS_ROW_OK) == 0) continue;
        if (m->geti(ep_nr, enable_col) == 0) continue;
        if (m_end_points->first(ep_nr)) continue;

        ep = new eContainer(list, ep_nr);
        v = new eVariable(ep, ENET_ENDP_PROTOCOL);
        m->getv(ep_nr, protocol_col, v);
        v = new eVariable(ep, ENET_ENDP_TRANSPORT);
        m->getv(ep_nr, transport_col, v);
        v = new eVariable(ep, ENET_ENDP_PORT);
        m->getv(ep_nr, port_col, v);
    }
    os_unlock();

    /* Add end points (no lock).
     */
    for (ep = list->firstc(); ep; ep = next_ep)
    {
        next_ep = ep->nextc();
        ep_nr = ep->oid();
        proto_name = ep->firstv(ENET_ENDP_PROTOCOL);
        proto = protocol_by_name(proto_name);
        if (proto == OS_NULL) {
            osal_debug_error_str("unknown protocol: ", proto_name->gets());
            // update status in table
            continue;
        }

        handle = proto->new_end_point(ep_nr, OS_NULL, &s);
        if (handle == OS_NULL) {
            osal_debug_error_str("unable to create end point: ", proto_name->gets());
            // update status in table, status s
            continue;
        }
        handle->adopt(ep, ENET_ENDP_PROTOCOL_HANDLE);

        /* Adopt, successfull created end point.
         */
        ep->adopt(m_end_points);
    }

    delete localvars;
    return;

getout_unlock:
    os_unlock();
    delete localvars;
    osal_debug_error("eNetMaintainThread::publish failed");
}


void eNetMaintainThread::add_protocol(
    eProtocol *proto)
{
    proto->adopt(m_protocols);
    proto->addname("iocom");
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

    /* Set up class for use.
     */
    eNetMaintainThread::setupclass();

    /* Create and start thread to listen for maintain UDP multicasts,
       name it "_maintain" in process name space.
     */
    maintain = new eNetMaintainThread();
    maintain->addname("//_netmaintain");
    maintain->set_netservice(netservice);
    // maintain->bind(ENETMAINTAINP_SEND_UDP_MULTICASTS, "//netservice/servprm/maintainserv");
    maintain->bind(ENETMAINTAINP_PUBLISH, "//netservice", enetservp_endpoint_change_counter);

    while ((proto = (eProtocol*)netservice->protocols()->first())) {
        maintain->add_protocol(proto);
    }
    maintain->start(maintain_thread_handle);
}

