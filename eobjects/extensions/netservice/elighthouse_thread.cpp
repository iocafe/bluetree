/**

  @file    elighthouse_thread.cpp
  @brief   Look out for device networks in the same LAN and announce services by UDP multicast.
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

/* Lighthouse UDP service property names.
 */
const os_char
    elighthousep_send_udp_multicasts[] = "send_multicasts",
    elighthousep_publish[] = "publish";

/**
****************************************************************************************************
  Constructor.
****************************************************************************************************
*/
eLightHouseService::eLightHouseService(
    eObject *parent,
    e_oid oid,
    os_int flags)
    : eThread(parent, oid, flags)
{
    m_netservice = OS_NULL;

    m_send_udp_multicasts = OS_FALSE;
    m_publish_count = 0;
    m_publish = OS_TRUE;
    m_publish_status = ESTATUS_FAILED;
    m_udp_send_initialized = OS_FALSE;

    m_counters = new eContainer(this);
    m_counters->ns_create();
    initproperties();
}


/**
****************************************************************************************************
  Virtual destructor.
****************************************************************************************************
*/
eLightHouseService::~eLightHouseService()
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
void eLightHouseService::setupclass()
{
    const os_int cls = ECLASSID_LIGHT_HOUSE_CLIENT;

    /* Add the class to class list.
     */
    os_lock();
    eclasslist_add(cls, (eNewObjFunc)newobj, "eLightHouseService");
    addpropertyb(cls, ELIGHTHOUSEP_SEND_UDP_MULTICASTS, elighthousep_send_udp_multicasts,
        OS_FALSE, "send UDP multicasts", EPRO_PERSISTENT);
    addpropertyb(cls, ELIGHTHOUSEP_PUBLISH, elighthousep_publish,
        OS_FALSE, "publish end point info", EPRO_DEFAULT);
    propertysetdone(cls);
    os_unlock();
}


/**
****************************************************************************************************

  @brief Function to process incoming messages.

  The eLightHouseService::onmessage function handles messages received by object. If this function
  doesn't process message, it calls parent class'es onmessage function.

  Send UDP broadcasts by timer hit.

  @param   envelope Message envelope. Contains command, target and source paths and
           message content, etc.
  @return  None.

****************************************************************************************************
*/
void eLightHouseService::onmessage(
    eEnvelope *envelope)
{
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
eStatus eLightHouseService::onpropertychange(
    os_int propertynr,
    eVariable *x,
    os_int flags)
{
    os_int count;

    switch (propertynr)
    {
        case ELIGHTHOUSEP_SEND_UDP_MULTICASTS:
            m_send_udp_multicasts = (os_boolean)x->getl();
            if (m_send_udp_multicasts) m_publish = OS_TRUE;
            break;

        case ELIGHTHOUSEP_PUBLISH:
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


/* Overloaded eThread function to initialize new thread. Called after eLightHouseService object is created.
 */
void eLightHouseService::initialize(
    eContainer *params)
{
    ioc_initialize_lighthouse_client(&m_client,
        OS_FALSE, /* is_ipv6 */
        OS_FALSE, /* is_tls */
        OS_NULL);

    ioc_set_lighthouse_client_callback(&m_client, callback, this);
}


/* Overloaded eThread function to perform thread specific cleanup when thread exists: Release
   resources allocated for lighthouse client. This is a "pair" to initialize function.
 */
void eLightHouseService::finish()
{
    ioc_release_lighthouse_client(&m_client);

    if (m_udp_send_initialized) {
        ioc_release_lighthouse_server(&m_server);
        m_udp_send_initialized = OS_FALSE;
    }
}


/**
****************************************************************************************************

  @brief Maintain LAN service UDP communication, thread main loop.

  The eLightHouseService::run() function keeps eobjects messaging alive, sends and listens for
  light house UDP multicasts. This thread handles all lighthouse UDP communication.
  The data to be published (sent as UDP messages) is collected from "endpoint" table,
  and received data is stored in "LAN services" table. Both these tables are global
  and owned by eProcess.

****************************************************************************************************
*/
void eLightHouseService::run()
{
    osalStatus s;

    while (OS_TRUE)
    {
        alive(EALIVE_RETURN_IMMEDIATELY);
        if (exitnow()) break;

        if (m_send_udp_multicasts) {
            if (m_publish) {
                if (!m_udp_send_initialized) {
                    ioc_initialize_lighthouse_server(&m_server, 10);
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
            ioc_release_lighthouse_server(&m_server);
            m_udp_send_initialized = OS_FALSE;
            timer(0);
        }

        s = ioc_run_lighthouse_client(&m_client, m_trigger);
        if (s != OSAL_SUCCESS) {
            if (s != OSAL_PENDING) {
osal_debug_error_int("ioc_run_lighthouse_client failed, s=", s);
            }
            os_sleep(500);
        }
    }
}


/**
****************************************************************************************************

  @brief Process received endpoint information, callback.

  The eLightHouseService::callback() is callback from iocom library's ioc_run_lighthouse_client()
  function. It is called when information about and endpoint is received by UDP multicast.
  The data structure as argument contains the information about the end point:
    - IO network or process name.
    - Communication protocol supported by the endpoint.
    - IO address and port numbers for plain socket and TLS.
    - Nickname of IO device or process.

  @param   c Pointer to the iocom light house client object structure.
  @param   data Structure containing information about the endpoint.
  @param   context Pointer to eobjects light house object.

****************************************************************************************************
*/
void eLightHouseService::callback(
    LighthouseClient *c,
    LightHouseClientCallbackData *data,
    void *context)
{
    eLightHouseService *ec;
    eContainer *row;
    eVariable *element, *where, *counter, tmp;
    os_long ti;
    const os_char *protocol_long;
    os_char nbuf[OSAL_NBUF_SZ];

    ec = (eLightHouseService*)context;
    if (data->network_name == OS_NULL) return;

    tmp.sets(data->network_name);
    tmp.appends("-");
    tmp.appends(data->protocol);
    counter = eVariable::cast(ec->m_counters->byname(tmp.gets()));
    if (counter) {
        if (counter->getl() == data->counter) {
            return;
        }
    }
    else {
        counter = new eVariable(ec->m_counters);
        counter->addname(tmp.gets());
    }
    counter->setl(data->counter);

    row = new eContainer(ec, EOID_TEMPORARY, EOBJ_TEMPORARY_ATTACHMENT);

    element = new eVariable(row);
    element->addname("name", ENAME_NO_MAP);
    element->sets(data->network_name);

    element = new eVariable(row);
    element->addname("nick", ENAME_NO_MAP);
    element->sets(data->nickname);

    element = new eVariable(row);
    element->addname("protocol", ENAME_NO_MAP);
    if (!os_strcmp(data->protocol, "i")) {
        protocol_long = "iocom";
    }
    else if (!os_strcmp(data->protocol, "o")) {
        protocol_long = "ecom";
    }
    else {
        protocol_long = data->protocol;
    }
    element->sets(protocol_long);

    element = new eVariable(row);
    element->addname("ip", ENAME_NO_MAP);
    element->sets(data->ip_addr);

    element = new eVariable(row);
    element->addname("tlsport", ENAME_NO_MAP);
    if (data->tls_port_nr) element->setl(data->tls_port_nr);

    element = new eVariable(row);
    element->addname("tcpport", ENAME_NO_MAP);
    if (data->tcp_port_nr) element->setl(data->tcp_port_nr);

    element = new eVariable(row);
    element->addname("tstamp", ENAME_NO_MAP);
    ti = etime();
    element->setl(ti);

    where = new eVariable(ec, EOID_TEMPORARY, EOBJ_TEMPORARY_ATTACHMENT);

    /* Remove rows with time stamps more than 5 seconds to future or
       older than 10 minutes.
     */
    osal_int_to_str(nbuf, sizeof(nbuf), ti + 5000000);
    where->sets("tstamp>");
    where->appends(nbuf);
#if 1
    osal_int_to_str(nbuf, sizeof(nbuf), ti - 600L * 1000000L);
    where->appends(" OR tstamp<");
    where->appends(nbuf);
#endif
    etable_remove(ec, "//netservice/services", OS_NULL, where->gets());

    /* Add row to the LAN services table.
     */
    where->sets("name=\'");
    where->appends(data->network_name);
    where->appends("\'");
    where->appends("AND protocol=\'");
    where->appends(protocol_long);
    where->appends("\'");
    etable_update(ec, "//netservice/services", OS_NULL, where->gets(), row,
        ETABLE_ADOPT_ARGUMENT|ETABLE_INSERT_OR_UPDATE);

    delete where;
}


/**
****************************************************************************************************

  @brief Send end point information out as UDP multicast.

  The eLightHouseService::run_server function is called periodically (about once every 4 seconds)
  to send end point information as UDP multicast. The function is called also if end point
  table is modified. It calls iocom librarie's ioc_run_lighthouse_server() function to
  send the broadcast, set up earlier by LightHouseService::publish() function.

****************************************************************************************************
*/
void eLightHouseService::run_server()
{
    os_timer ti;
    osalStatus s;

    if (m_udp_send_initialized) {
        os_get_timer(&ti);
        s = ioc_run_lighthouse_server(&m_server, &ti);
        if (s != OSAL_SUCCESS && s != OSAL_PENDING) {
            ioc_release_lighthouse_server(&m_server);
            m_udp_send_initialized = OS_FALSE;
        }
    }
}


/**
****************************************************************************************************

  @brief Publish (initial or update) the end point information.

  The eLightHouseService::publish() function is collects data from endpoint table and
  sets up data within iocom's LighthouseServer structure. This function doesn't send
  actual UDP multivasts, eLightHouseService::run_server() does that.

  @return ESTATUS_SUCCESS if all is fine, oe ESTATUS_FAILED if notthing to publish.

****************************************************************************************************
*/
eStatus eLightHouseService::publish()
{
    eMatrix *m;
    eObject *conf, *columns, *col;
    eContainer *localvars, *list, *item;
    eVariable *v;
    const os_char *protocol_short;
    os_int enable_col, protocol_col, transport_col, port_col; //, netname_col;
    os_int h, y, is_ipv6, is_tls, item_id, port_nr;
    os_int tls_port, tcp_port;
    enetEndpTransportIx transport_ix;
    eVariable protocol, tmp;
    os_char buf[OSAL_NETWORK_NAME_SZ], *p;
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


    ioc_lighthouse_start_endpoints(&m_server, "manteli");

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

        ioc_lighthouse_add_endpoint(&m_server, p,
            protocol_short, tls_port, tcp_port, is_ipv6);

        s = ESTATUS_SUCCESS;
    }

    delete localvars;
    return s;

getout:
    os_unlock();
    delete localvars;
    osal_debug_error("eLightHouseService::publish failed");
    return ESTATUS_FAILED;
}


/**
****************************************************************************************************

  @brief Create "LAN services" table.

  The eNetService::create_table function creates a global "LAN services" table, which displays
  services within the local are network segment. This table is used for both user information
  and to automatically generate connections, etc. The eLightHouseService::callback function
  populates this table.

****************************************************************************************************
*/
void eNetService::create_services_table()
{
    eContainer *configuration, *columns;
    eVariable *column;

    m_services_matrix = new eMatrix(this);
    m_services_matrix->addname("services");
    m_services_matrix->setpropertys(ETABLEP_TEXT, "LAN");

    configuration = new eContainer(this);
    columns = new eContainer(configuration, EOID_TABLE_COLUMNS);
    columns->addname("columns", ENAME_NO_MAP);

    /* For matrix as a table row number is always the first column in configuration.
     */
    column = new eVariable(columns);
    column->addname("ix", ENAME_NO_MAP);
    column->setpropertys(EVARP_TEXT, "row");
    column->setpropertyi(EVARP_TYPE, OS_INT);

    column = new eVariable(columns);
    column->addname("name", ENAME_NO_MAP);
    column->setpropertys(EVARP_TEXT, "name");
    column->setpropertyi(EVARP_TYPE, OS_STR);
    column->setpropertys(EVARP_TTIP,
        "IOCOM device network or eobjects process name");

    column = new eVariable(columns);
    column->addname("nick", ENAME_NO_MAP);
    column->setpropertys(EVARP_TEXT, "nickname");
    column->setpropertyi(EVARP_TYPE, OS_STR);
    column->setpropertys(EVARP_TTIP,
        "IO device's or process'es nickname. Helps user to identify devices, not used by sofware");

    column = new eVariable(columns);
    column->addname("protocol", ENAME_NO_MAP);
    column->setpropertys(EVARP_TEXT, "protocol");
    column->setpropertyi(EVARP_TYPE, OS_STR);
    column->setpropertys(EVARP_ATTR, "list=\"ecom,iocom\"");
    column->setpropertys(EVARP_TTIP,
        "Protocols, one of.\n"
        "- \'ecom\': eobjects communication protocol (for glass user interface, etc).\n"
        "- \'iocom\': IO device communication protocol.\n");

    column = new eVariable(columns);
    column->addname("ip", ENAME_NO_MAP);
    column->setpropertys(EVARP_TEXT, "IP address");
    column->setpropertyi(EVARP_TYPE, OS_STR);
    column->setpropertys(EVARP_TTIP,
        "Listening IP address");

    column = new eVariable(columns);
    column->addname("tlsport", ENAME_NO_MAP);
    column->setpropertys(EVARP_TEXT, "TLS port");
    column->setpropertyi(EVARP_TYPE, OS_INT);
    column->setpropertys(EVARP_TTIP,
        "Listening secure TLS socket port number.");

    column = new eVariable(columns);
    column->addname("tcpport", ENAME_NO_MAP);
    column->setpropertys(EVARP_TEXT, "TCP port");
    column->setpropertyi(EVARP_TYPE, OS_INT);
    column->setpropertys(EVARP_TTIP,
        "Listening TCP socket port number (not secured).");

    column = new eVariable(columns);
    column->addname("tstamp", ENAME_NO_MAP);
    column->setpropertys(EVARP_TEXT, "timestamp");
    column->setpropertyi(EVARP_TYPE, OS_LONG);
    column->setpropertys(EVARP_ATTR, "tstamp=\"yy,sec\",nosave");
    column->setpropertys(EVARP_TTIP,
        "Time when this information was last updated");

    /* ETABLE_ADOPT_ARGUMENT -> configuration will be released from memory.
     */
    m_services_matrix->configure(configuration, ETABLE_ADOPT_ARGUMENT);
    m_services_matrix->setflags(EOBJ_TEMPORARY_CALLBACK);
}


/**
****************************************************************************************************

  @brief Create "LAN services" table.

  The enet_start_lighthouse_thread function adds lighthouse service class, creates a light house
  thread object and starts to run it. This function is called by eNetService::start().

****************************************************************************************************
*/
void enet_start_lighthouse_thread(
    eNetService *netservice,
    os_int flags,
    eThreadHandle *lighthouse_client_thread_handle)
{
    struct eLightHouseService *lighthouse;

    /* Set up class for use.
     */
    eLightHouseService::setupclass();

    /* Create and start thread to listen for lighthouse UDP multicasts,
       name it "_lighthouse" in process name space.
     */
    lighthouse = new eLightHouseService();
    lighthouse->addname("//_lighthouse");
    lighthouse->set_netservice(netservice);
    lighthouse->bind(ELIGHTHOUSEP_SEND_UDP_MULTICASTS, "//netservice/servprm/lighthouseserv");
    lighthouse->bind(ELIGHTHOUSEP_PUBLISH, "//netservice", enetservp_endpoint_change_counter);
    lighthouse->start(lighthouse_client_thread_handle);
}
