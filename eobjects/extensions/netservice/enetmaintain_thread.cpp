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
    enetmaintainp_config_counter[] = "epconfigcnt";

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
    m_protocols = new eContainer(this, EOID_ITEM, EOBJ_IS_ATTACHMENT);
    m_protocols->ns_create();
    m_end_point_config_count = 0;

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
        -1, "update end points and connections", EPRO_DEFAULT);
    addpropertyl(cls, ENETMAINTAINP_CONFIG_COUNTER, enetmaintainp_config_counter,
        0, "end point configuration trigger", EPRO_DEFAULT);
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
                // maintain_end_points();
                m_publish_count = count;
                m_publish = OS_TRUE;
                os_get_timer(&m_publish_timer);
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
    }

    /* End points are closed here explicitely to make sure that os_lock() doesn't
     * cause deadlock.
     */
    for (ep = m_end_points->firstc(); ep; ep = next_ep)
    {
        next_ep = ep->nextc();
        delete_ep(ep);

        /* proto_name = ep->firstv(ENET_ENDP_PROTOCOL);
        proto = protocol_by_name(proto_name);
        if (proto == OS_NULL) continue;
        handle = eProtocolHandle::cast(ep->first(ENET_ENDP_PROTOCOL_HANDLE));

        if (proto->is_end_point_running(handle))
        {
            proto->delete_end_pont(handle);
            while (proto->is_end_point_running(handle)) {
                os_timeslice();
            }
        }
        delete ep; */
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
    os_boolean changed = OS_FALSE;

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
        if (proto == OS_NULL) {
            osal_debug_error_str("Program error, unknown proto ", proto_name->gets());
            delete ep;
            continue;
        }
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
        /* handle = eProtocolHandle::cast(ep->first(ENET_ENDP_PROTOCOL_HANDLE));
        if (proto->is_end_point_running(handle))
        {
            proto->delete_end_pont(handle);
            while (proto->is_end_point_running(handle)) {
                os_timeslice();
            }
        }
        delete ep; */

        delete_ep(ep);
        changed = OS_TRUE;
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
            osal_debug_error_str("Unknown protocol: ", proto_name->gets());
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
        ep->adopt(m_end_points, ep_nr);
        changed = OS_TRUE;
    }

    /* Initiate end point information update in UDP multicasts.
     */
    if (changed) {
        setpropertyl(ENETMAINTAINP_CONFIG_COUNTER, ++m_end_point_config_count);
    }

    delete localvars;
    return;

getout_unlock:
    os_unlock();
    delete localvars;
    osal_debug_error("eNetMaintainThread::publish failed");
}

void eNetMaintainThread::delete_ep(
    eContainer *ep)
{
    eVariable *proto_name;
    eProtocol *proto;
    eProtocolHandle *handle;

    proto_name = ep->firstv(ENET_ENDP_PROTOCOL);
    proto = protocol_by_name(proto_name);
    if (proto == OS_NULL) return;

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

    /* Set up class for use.
     */
    eNetMaintainThread::setupclass();

    /* Create and start thread to listen for maintain UDP multicasts,
       name it "_maintain" in process name space.
     */
    maintain = new eNetMaintainThread();
    maintain->addname("//_netmaintain");
    maintain->set_netservice(netservice);
    maintain->bind(ENETMAINTAINP_PUBLISH, "//netservice",
        enetservp_endpoint_table_change_counter);
    maintain->bind(ENETMAINTAINP_CONFIG_COUNTER, "//netservice",
        enetservp_endpoint_config_counter, EBIND_CLIENTINIT);
    while ((proto = (eProtocol*)netservice->protocols()->first())) {
        maintain->add_protocol(proto);
    }
    maintain->start(maintain_thread_handle);
}

