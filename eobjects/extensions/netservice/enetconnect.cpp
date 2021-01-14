/**

  @file    enetconnect.cpp
  @brief   Connections to establish.
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


/**
****************************************************************************************************

  @brief Create "connect" table.

  The connect table specifies ecom and iocom connection to establish.

    "connect": [{
                "transport": "none",
                "parameters": "127.0.0.1:6367",
                "flags": "connect,up"
            },
            {
                "transport": "tls",
                "parameters": "*",
                "flags": "listen,down"
            },
            {
                "transport": "socket",
                "parameters": "*",
                "flags": "listen,down"
            }
        ],


****************************************************************************************************
*/
void eNetService::create_connect_table()
{
    eContainer *configuration, *columns;
    eVariable *column;

    m_connect = new ePersistent(this);
    m_connection_matrix = new eMatrix(m_connect);
    m_connection_matrix->addname("connect");
    m_connection_matrix->setpropertys(ETABLEP_TEXT, "connect to");

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
    column->addname("enable", ENAME_NO_MAP);
    column->setpropertys(EVARP_TEXT, "enable");
    column->setpropertyi(EVARP_TYPE, OS_BOOLEAN);
    column->setpropertyi(EVARP_DEFAULT, OS_TRUE);
    column->setpropertys(EVARP_TTIP,
        "Enable this row.");

    column = new eVariable(columns);
    column->addname("protocol", ENAME_NO_MAP);
    column->setpropertys(EVARP_TEXT, "protocol");
    column->setpropertyi(EVARP_TYPE, OS_STR);
    column->setpropertys(EVARP_ATTR, "list=\"ecom,iocom\"");
    column->setpropertys(EVARP_DEFAULT, "ecom");
    column->setpropertys(EVARP_TTIP,
        "Communication protocol.\n"
        "- \'ecom\': eobjects communication protocol (for glass user interface, etc).\n"
        "- \'iocom\': IO device communication protocol.\n");

    column = new eVariable(columns);
    column->addname("ip", ENAME_NO_MAP);
    column->setpropertys(EVARP_TEXT, "address/port");
    column->setpropertyi(EVARP_TYPE, OS_STR);
    column->setpropertys(EVARP_DEFAULT, "localhost");
    column->setpropertys(EVARP_TTIP,
        "IP andress and optional port number, COM port, or \'*\' to connect to addressed determined\n"
        "by lighthouse UDP multicasts. Examples: \'192.168.1.222\', \'192.168.1.222:666\', \'*\',\n"
        "or \'COM1:115200\'");

    column = new eVariable(columns);
    column->addname("name", ENAME_NO_MAP);
    column->setpropertys(EVARP_TEXT, "name");
    column->setpropertyi(EVARP_TYPE, OS_STR);
    column->setpropertys(EVARP_DEFAULT, "*");
    column->setpropertys(EVARP_TTIP,
        "Process name or IO network name to connect to. This can be a list, if detecting\n"
        "services by lighthouse. Wildcard \'*\' indicates that any name will be connected to.");

    column = new eVariable(columns);
    column->addname("transport", ENAME_NO_MAP);
    column->setpropertys(EVARP_TEXT, "transport");
    column->setpropertyi(EVARP_TYPE, OS_CHAR);
    column->setpropertys(EVARP_ATTR, "enum=\"1.SOCKET,2.TLS,3.SERIAL\"");
    column->setpropertyi(EVARP_DEFAULT, 2);
    column->setpropertys(EVARP_TTIP,
        "Transport to use.\n"
        "- \'SOCKET\': Plain socket connection, unsecured.\n"
        "- \'TLS\': TLS connection.\n"
        "- \'SERIAL\': Serial communication.\n");

    column = new eVariable(columns);
    column->addname("found", ENAME_NO_MAP);
    column->setpropertyi(EVARP_TYPE, OS_STR);
    column->setpropertys(EVARP_TEXT, "found");
    column->setpropertys(EVARP_ATTR, "nosave,rdonly");
    column->setpropertys(EVARP_TTIP,
        "Processes found by lighthouse.");

    column = new eVariable(columns);
    column->addname("active", ENAME_NO_MAP);
    column->setpropertyi(EVARP_TYPE, OS_INT);
    column->setpropertys(EVARP_TEXT, "active");
    column->setpropertys(EVARP_ATTR, "nosave,rdonly");
    column->setpropertys(EVARP_TTIP,
        "Number of active connections on resulting this end point.");

    column = new eVariable(columns);
    column->addname("tstamp", ENAME_NO_MAP);
    column->setpropertys(EVARP_TEXT, "last connection");
    column->setpropertyi(EVARP_TYPE, OS_LONG);
    column->setpropertys(EVARP_ATTR, "tstamp=\"yy,sec\",nosave,rdonly");
    column->setpropertys(EVARP_TTIP,
        "Time stamp of the last successfull connect.");

    /* ETABLE_ADOPT_ARGUMENT -> configuration will be released from memory.
     */
    m_connection_matrix->configure(configuration, ETABLE_ADOPT_ARGUMENT);
    m_connection_matrix->setflags(EOBJ_TEMPORARY_CALLBACK);

    m_connect->load_file("connect.eo");
    m_connect->setflags(EOBJ_TEMPORARY_CALLBACK);

    if (m_connection_matrix->nrows() == 0) {
        add_connect(OS_TRUE, "ecom", "localhost", "*", 1);
        add_connect(OS_FALSE, "ecom", "*", "*", 1);
        add_connect(OS_FALSE, "iocom", "*", "*", 1);
    }
}


/**
****************************************************************************************************

  @brief Add a row for a connection to "connections" table.

  The eNetService::add_connect function...

****************************************************************************************************
*/
void eNetService::add_connect(
    os_int enable,
    const os_char *protocol,
    const os_char *ip,
    const os_char *name,
    os_int transport,
    os_int row_nr)
{
    eContainer row;
    eVariable *element;

    if (row_nr > 0) {
        element = new eVariable(&row);
        element->addname("ix", ENAME_NO_MAP);
        element->setl(row_nr);
    }

    element = new eVariable(&row);
    element->addname("enable", ENAME_NO_MAP);
    element->setl(enable);

    if (protocol) {
        element = new eVariable(&row);
        element->addname("protocol", ENAME_NO_MAP);
        element->sets(protocol);
    }

    if (ip) {
        element = new eVariable(&row);
        element->addname("ip", ENAME_NO_MAP);
        element->sets(ip);
    }

    if (name) {
        element = new eVariable(&row);
        element->addname("name", ENAME_NO_MAP);
        element->sets(name);
    }

    element = new eVariable(&row);
    element->addname("transport", ENAME_NO_MAP);
    element->setl(transport);

    m_connection_matrix->insert(&row);
}



/**
****************************************************************************************************

  @brief Create and delete connections as needed.

  The eNetMaintainThread::maintain_connections() function is collects data from connect table and
  sets up connections for communication protocols.

  @return ESTATUS_SUCCESS if all is fine, oe ESTATUS_FAILED if notthing to publish.

****************************************************************************************************
*/
void eNetMaintainThread::maintain_connections()
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
