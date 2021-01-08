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

  @brief Publish (initial or update) the end point information.

  The eNetMaintainThread::publish() function is collects data from endpoint table and
  sets up data within iocom's LighthouseServer structure. This function doesn't send
  actual UDP multivasts, eNetMaintainThread::run_server() does that.

  @return ESTATUS_SUCCESS if all is fine, oe ESTATUS_FAILED if notthing to publish.

****************************************************************************************************
*/
eStatus eNetMaintainThread::publish()
{
    eMatrix *m;
    eObject *conf, *columns, *col;
    eContainer *localvars, *list, *item;
    eVariable *v;
    // const os_char *protocol_short;
    os_int enable_col, protocol_col, transport_col, port_col; //, netname_col;
    os_int h, y, is_ipv6, is_tls, item_id, port_nr;
    // os_int tls_port, tcp_port;
    enetEndpTransportIx transport_ix;
    eVariable protocol, tmp;
    // os_char buf[OSAL_NETWORK_NAME_SZ], *p;
    eStatus s = ESTATUS_FAILED;

    localvars = new eContainer(ETEMPORARY);
    list = new eContainer(localvars);
    // list->ns_create();
    os_lock();

    m = m_netservice->m_endpoint_matrix;
    conf = m->configuration();
    if (conf == OS_NULL) goto getout;
    columns = conf->first(EOID_TABLE_COLUMNS);
    if (columns == OS_NULL) goto getout;

    col = columns->byname(enet_endp_enable);
    if (col == OS_NULL) goto getout;
    enable_col = col->oid();
    col = columns->byname(enet_endp_protocol);
    if (col == OS_NULL) goto getout;
    protocol_col = col->oid();
    col = columns->byname(enet_endp_transport);
    if (col == OS_NULL) goto getout;
    transport_col = col->oid();
    col = columns->byname(enet_endp_port);
    if (col == OS_NULL) goto getout;
    port_col = col->oid();
    /* col = columns->byname(enet_endp_netname);
    if (col == OS_NULL) goto getout;
    netname_col = col->oid(); */

    h = m->nrows();
    for (y = 0; y<h; y++) {
        if ((m->geti(y, EMTX_FLAGS_COLUMN_NR) & EMTX_FLAGS_ROW_OK) == 0) continue;
        if (m->geti(y, enable_col) == 0) continue;

        m->getv(y, protocol_col, &protocol);
        port_nr = m->geti(y, port_col);
        if (port_nr <= 0) continue;

        transport_ix = (enetEndpTransportIx)m->geti(y, transport_col);
        switch (transport_ix) {
            case ENET_ENDP_SOCKET_IPV4: is_tls = 0; is_ipv6 = 0; break;
            case ENET_ENDP_SOCKET_IPV6: is_tls = 0; is_ipv6 = 1; break;
            case ENET_ENDP_TLS_IPV4:    is_tls = 1; is_ipv6 = 0; break;
            case ENET_ENDP_TLS_IPV6:    is_tls = 1; is_ipv6 = 1; break;
            default: goto goon;
        }

        item_id = is_ipv6;
        for (item = list->firstc(item_id); item; item = item->nextc(item_id))
        {
            /* Make sure that network name matches. If not skip compare row.
             */
            v = item->firstv(ENET_ENDP_PROTOCOL);
            if (protocol.compare(v)) {
                continue;
            }

/* Make sure that interface matches. If not skip compare row.
 */

/* If iocom, make sure that network name matches. If not skip compare row.
            if (protocol_ix == ENET_ENDP_IOCOM) {
            }
 */

            v = item->firstv(is_tls ? ENET_ENDP_TLS_PORT : ENET_ENDP_TCP_PORT);
            if (v == OS_NULL) {
                v = new eVariable(item, is_tls ? ENET_ENDP_TLS_PORT : ENET_ENDP_TCP_PORT);
                v->setl(port_nr);
                goto goon;
            }
            if (v->getl() == port_nr) goto goon;
        }

        item = new eContainer(list, item_id);
        v = new eVariable(item, ENET_ENDP_PROTOCOL);
        v->setv(&protocol);
        v = new eVariable(item, is_tls ? ENET_ENDP_TLS_PORT : ENET_ENDP_TCP_PORT);
        v->setl(port_nr);
        v = new eVariable(item, ENET_ENDP_IPV6);
        v->setl(is_ipv6 ? OS_TRUE : OS_FALSE);
        /* v = new eVariable(item, ENET_ENDP_NETNAME);
        m->getv(y, netname_col, v); */
goon:;
    }

    os_unlock();

#if 0
    ioc_maintain_start_endpoints(&m_server, "manteli");

    for (item = list->firstc(); item; item = item->nextc()) {
        tls_port = 0;
        v = item->firstv(ENET_ENDP_TLS_PORT);
        if (v) tls_port = v->getl();
        tcp_port = 0;
        v = item->firstv(ENET_ENDP_TCP_PORT);
        if (v) tcp_port = v->getl();

        is_ipv6 = OS_FALSE;
        v = item->firstv(ENET_ENDP_IPV6);
        if (v) is_ipv6 = v->getl();

        v = item->firstv(ENET_ENDP_PROTOCOL);
        protocol.setv(v);

        if (!os_strcmp(protocol.gets(), "iocom")) {
            os_strncpy(buf, eglobal->process_name, sizeof(buf));
            os_strncat(buf, "net", sizeof(buf));
            p = buf;
            protocol_short = "i";
        }
        else {
            p = eglobal->process_nr ? eglobal->process_id : eglobal->process_name;
            protocol_short = protocol.gets();
            if (!os_strcmp(protocol.gets(), "ecom")) {
                protocol_short = "o";
            }
        }

        ioc_maintain_add_endpoint(&m_server, p,
            protocol_short, tls_port, tcp_port, is_ipv6);

        s = ESTATUS_SUCCESS;
    }
#endif

    delete localvars;
    return s;

getout:
    os_unlock();
    delete localvars;
    osal_debug_error("eNetMaintainThread::publish failed");
    return ESTATUS_FAILED;
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
    struct eNetMaintainThread *maintain;

    /* Set up class for use.
     */
    eNetMaintainThread::setupclass();

    /* Create and start thread to listen for maintain UDP multicasts,
       name it "_maintain" in process name space.
     */
    maintain = new eNetMaintainThread();
    maintain->addname("//_netmaintain");
    maintain->set_netservice(netservice);
    // maintain->bind(ELIGHTHOUSEP_SEND_UDP_MULTICASTS, "//netservice/servprm/maintainserv");
    maintain->bind(ELIGHTHOUSEP_PUBLISH, "//netservice", enetservp_endpoint_change_counter);
    maintain->start(maintain_thread_handle);
}

