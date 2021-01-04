/**

  @file    elighthouse_client.cpp
  @brief   Look out for device networks in the same LAN.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "eobjects.h"
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
    os_timer ti;
    osalStatus s;

    /* If at final destination for the message.
     */
    if (*envelope->target()=='\0')
    {
        switch (envelope->command())
        {
            case ECMD_TIMER:
                if (m_udp_send_initialized) {
                    os_get_timer(&ti);
                    s = ioc_run_lighthouse_server(&m_server, &ti);
                    if (s != OSAL_SUCCESS && s != OSAL_PENDING) {
                        stop_udp_multicasts();
                        m_udp_send_initialized = OS_FALSE;
                    }
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
        stop_udp_multicasts();
        m_udp_send_initialized = OS_FALSE;
    }
}

/* Listen for lighthouse USP multicasts.
 */
void eLightHouseService::run()
{
    osalStatus s;

    while (OS_TRUE)
    {
        alive(EALIVE_RETURN_IMMEDIATELY);
        if (exitnow()) break;

        s = ioc_run_lighthouse_client(&m_client, m_trigger);
        if (s != OSAL_SUCCESS) {
            if (s != OSAL_PENDING) {
osal_debug_error_int("ioc_run_lighthouse_client failed, s=", s);
            }
            os_sleep(500);
        }

osal_debug_error("XXX");

        if (m_send_udp_multicasts) {
            if (m_publish) {
                m_publish = OS_FALSE;
                if (m_udp_send_initialized) {
                    stop_udp_multicasts();
                    m_udp_send_initialized = OS_FALSE;
                }

                publish();
                m_udp_send_initialized = OS_TRUE;
                timer(5000);
            }
        }
        else if (m_udp_send_initialized)
        {
            stop_udp_multicasts();
            m_udp_send_initialized = OS_FALSE;
            timer(0);
        }
    }
}

/* Callback by the same thread which calls ioc_run_lighthouse_client()
 */
void eLightHouseService::callback(
    LighthouseClient *c,
    LightHouseClientCallbackData *data,
    void *context)
{
    eLightHouseService *ec;
    eContainer *row;
    eVariable *element, *where, *counter;

    ec = (eLightHouseService*)context;
    if (data->network_name == OS_NULL) return;

    osal_debug_error_str("HERE 1 ", data->ip_addr);
    osal_debug_error_str("HERE 2 ", data->network_name);
    osal_debug_error_int("HERE 3 ", data->tls_port_nr);
    osal_debug_error_int("HERE 4 ", osal_rand(1, 1000));

    counter = eVariable::cast(ec->m_counters->byname(data->network_name));
    if (counter) {
        if (counter->getl() == data->counter) {
            osal_debug_error_int("repeated lightcouse counter ", data->counter);
            return;
        }
    }
    else {
        counter = new eVariable(ec->m_counters);
        counter->addname(data->network_name);
    }
    counter->setl(data->counter);

    row = new eContainer(ec, EOID_TEMPORARY, EOBJ_TEMPORARY_ATTACHMENT);

    element = new eVariable(row);
    element->addname("name", ENAME_NO_MAP);
    element->sets(data->network_name);

    element = new eVariable(row);
    element->addname("protocol", ENAME_NO_MAP);
    if (!os_strcmp(data->protocol, "i")) {
        element->sets("iocom");
    }
    else if (!os_strcmp(data->protocol, "o")) {
        element->sets("eobjects");
    }
    else {
        element->sets(data->protocol);
    }

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
    element->setl(etime());

    where = new eVariable(ec, EOID_TEMPORARY, EOBJ_TEMPORARY_ATTACHMENT);
    where->appends("name=\'");
    where->appends(data->network_name);
    where->appends("\'");

    etable_update(ec, "//netservice/services", OS_NULL, where->gets(), row,
        ETABLE_ADOPT_ARGUMENT|ETABLE_INSERT_OR_UPDATE);

    delete where;
}


/* Publish (initial or update) the end point information.
 */
void eLightHouseService::publish()
{
    eMatrix *m;
    eObject *conf, *columns, *col;
    os_int enable_col, h, y;
    osalLighthouseInfo lighthouse_info;

    os_memclear(&lighthouse_info, sizeof(lighthouse_info));

    os_lock();

    m = m_netservice->m_endpoint_matrix;
    conf = m->configuration();
    if (conf == OS_NULL) goto getout;
    columns = conf->first(EOID_TABLE_COLUMNS);
    if (columns == OS_NULL) goto getout;

    col = columns->byname("enable");
    if (col == OS_NULL) goto getout;
    enable_col = col->oid();

    h = m->nrows();

    for (y = 0; y<h; y++) {
        if ((m->geti(y, EMTX_FLAGS_COLUMN_NR) & EMTX_FLAGS_ROW_OK) == 0) continue;
        if (m->geti(y, enable_col) == 0) continue;


    }

    os_unlock();

    ioc_initialize_lighthouse_server(&m_server, "abba", &lighthouse_info, "manteli");

    return;

getout:
    os_unlock();
    osal_debug_error("eLightHouseService::publish failed");
}


void eLightHouseService::stop_udp_multicasts()
{
    ioc_release_lighthouse_server(&m_server);
}


/**
****************************************************************************************************

  @brief Create "io device networks and processes" table.

  The eNetService::create_table function...

****************************************************************************************************
*/
void eNetService::create_services_table()
{
    eContainer *configuration, *columns;
    eVariable *column;

    m_services_matrix = new eMatrix(this);
    m_services_matrix->addname("services");
    m_services_matrix->setpropertys(ETABLEP_TEXT, "services in local network");

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
    column->addname("protocol", ENAME_NO_MAP);
    column->setpropertys(EVARP_TEXT, "protocol");
    column->setpropertyi(EVARP_TYPE, OS_CHAR);
    column->setpropertys(EVARP_ATTR, "enum=\"1.eobjects,2.iocom\"");
    column->setpropertys(EVARP_TTIP,
        "Protocols, one of.\n"
        "- \'eobjects\': eobjects communication protocol (for glass user interface, etc).\n"
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


/* Start light house service.
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


