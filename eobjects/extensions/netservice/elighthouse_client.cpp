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


/**
****************************************************************************************************
  Constructor.
****************************************************************************************************
*/
eLightHouseClient::eLightHouseClient(
    eObject *parent,
    e_oid oid,
    os_int flags)
    : eThread(parent, oid, flags)
{
    m_counters = new eContainer(this);
    m_counters->ns_create();
}


/**
****************************************************************************************************
  Virtual destructor.
****************************************************************************************************
*/
eLightHouseClient::~eLightHouseClient()
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
void eLightHouseClient::setupclass()
{
    const os_int cls = ECLASSID_LIGHT_HOUSE_CLIENT;

    /* Add the class to class list.
     */
    os_lock();
    eclasslist_add(cls, (eNewObjFunc)newobj, "eLightHouseClient");
    os_unlock();
}

/* Overloaded eThread function to initialize new thread. Called after eLightHouseClient object is created.
 */
void eLightHouseClient::initialize(
    eContainer *params)
{
    ioc_initialize_lighthouse_client(&m_lighthouse,
        OS_FALSE, /* is_ipv6 */
        OS_FALSE, /* is_tls */
        OS_NULL);

    ioc_set_lighthouse_client_callback(&m_lighthouse, callback, this);
}


/* Overloaded eThread function to perform thread specific cleanup when thread exists: Release
   resources allocated for lighthouse client. This is a "pair" to initialize function.
 */
void eLightHouseClient::finish()
{
    ioc_release_lighthouse_client(&m_lighthouse);
}

/* Listen for lighthouse USP multicasts.
 */
void eLightHouseClient::run()
{
    osalStatus s;

    while (OS_TRUE)
    {
        alive(EALIVE_RETURN_IMMEDIATELY);
        if (exitnow()) break;

        s = ioc_run_lighthouse_client(&m_lighthouse, m_trigger);
        if (s != OSAL_SUCCESS) {
            if (s != OSAL_PENDING) {
osal_debug_error_int("ioc_run_lighthouse_client failed, s=", s);
            }
            os_sleep(500);
        }

osal_debug_error("XXX");
    }
}

/* Callback by the same thread which calls ioc_run_lighthouse_client()
 */
void eLightHouseClient::callback(
    LighthouseClient *c,
    LightHouseClientCallbackData *data,
    void *context)
{
    eLightHouseClient *ec;
    eContainer *row;
    eVariable *element, *where, *counter;

    ec = (eLightHouseClient*)context;
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
    column->setpropertys(EVARP_TEXT, "tlsport");
    column->setpropertyi(EVARP_TYPE, OS_INT);
    column->setpropertys(EVARP_TTIP,
        "Listening secure TLS socket port number.");

    column = new eVariable(columns);
    column->addname("tcpport", ENAME_NO_MAP);
    column->setpropertys(EVARP_TEXT, "tcpport");
    column->setpropertyi(EVARP_TYPE, OS_INT);
    column->setpropertys(EVARP_TTIP,
        "Listening TCP socket port number (not secured).");

    column = new eVariable(columns);
    column->addname("tstamp", ENAME_NO_MAP);
    column->setpropertys(EVARP_TEXT, "time stamp");
    column->setpropertyi(EVARP_TYPE, OS_LONG);
    column->setpropertys(EVARP_ATTR, "tstamp=\"yy,sec\",nosave");
    column->setpropertys(EVARP_TTIP,
        "Time when this information was last updated");

    /* ETABLE_ADOPT_ARGUMENT -> configuration will be released from memory.
     */
    m_services_matrix->configure(configuration, ETABLE_ADOPT_ARGUMENT);
    m_services_matrix->setflags(EOBJ_TEMPORARY_CALLBACK);
}


/* Start light house client.
 */
void enet_start_lighthouse_client(
    eThreadHandle *lighthouse_client_thread_handle)
{
    struct eLightHouseClient *lighthouse;

    /* Set up class for use.
     */
    eLightHouseClient::setupclass();

    /* Create and start thread to listen for lighthouse UDP multicasts,
       name it "_lighthouse" in process name space.
     */
    lighthouse = new eLightHouseClient();
    lighthouse->addname("//_lighthouse");
    lighthouse->start(lighthouse_client_thread_handle);
}
