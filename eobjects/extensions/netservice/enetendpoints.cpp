/**

  @file    enetendpoints.cpp
  @brief   End points to listen to.
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

/* End point table column names.
 */
const os_char enet_endp_enable[] = "enable";
const os_char enet_endp_protocol[] = "protocol";
const os_char enet_endp_transport[] = "transport";
const os_char enet_endp_port[] = "port";
const os_char enet_endp_netname[] = "netname";


/**
****************************************************************************************************

  @brief Create "end point" table.

  The eNetService::create_user_account_table function...

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


  @param  flags Bit fields, ENET_DEFAULT_NO_END_POINTS flag checked by this function.


****************************************************************************************************
*/
void eNetService::create_end_point_table(
    os_int flags)
{
    eContainer *configuration, *columns;
    eVariable *column;
    os_boolean enable_by_default;

    m_end_points = new ePersistent(this);
    m_endpoint_matrix = new eMatrix(m_end_points);

    m_endpoint_matrix->addname("endpoints");
    m_endpoint_matrix->setpropertys(ETABLEP_TEXT, "endpoints");

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
    column->addname(enet_endp_enable, ENAME_NO_MAP);
    column->setpropertys(EVARP_TEXT, "enable");
    column->setpropertyi(EVARP_TYPE, OS_BOOLEAN);
    column->setpropertys(EVARP_TTIP,
        "Enable this end point");

    column = new eVariable(columns);
    column->addname(enet_endp_protocol, ENAME_NO_MAP);
    column->setpropertys(EVARP_TEXT, "protocol");
    column->setpropertyi(EVARP_TYPE, OS_STR);
    column->setpropertys(EVARP_ATTR, "list=\"ecom,iocom\"");
    column->setpropertys(EVARP_TTIP,
        "Listen for protocol.\n"
        "- \'eobjects\': eobjects communication protocol (for glass user interface, etc).\n"
        "- \'iocom\': IO device communication protocol.\n");

    column = new eVariable(columns);
    column->addname(enet_endp_transport, ENAME_NO_MAP);
    column->setpropertys(EVARP_TEXT, "transport");
    column->setpropertyi(EVARP_TYPE, OS_CHAR);
    column->setpropertys(EVARP_ATTR, "enum=\"1.SOCKET/IPv4,2.SOCKET/IPv6,3.TLS/IPv4,4.TLS/IPv6,5.SERIAL\"");
    column->setpropertys(EVARP_TTIP,
        "Transport to use.\n"
        "- \'SOCKET\': Plain socket connection, unsecured.\n"
        "- \'TLS\': TLS connection.\n"
        "- \'IPv4\' or 'IPv6\': Internet protocol, usually older \'IPv4\'.\n"
        "- \'SERIAL\': Serial communication.\n");

    column = new eVariable(columns);
    column->addname(enet_endp_port, ENAME_NO_MAP);
    column->setpropertys(EVARP_TEXT, "port");
    column->setpropertyi(EVARP_TYPE, OS_STR);
    column->setpropertys(EVARP_TTIP,
        "TCP port number to listen to. Typical:\n"
        "- \'6371\' eobjects socket.\n"
        "- \'6374\' eobjects TLS.\n"
        "- \'6368\' iocom socket.\n"
        "- \'6369\' iocom TLS.\n"
        "- \'COM1:115200\' serial port");

    /* column = new eVariable(columns);
    column->addname(enet_endp_iface, ENAME_NO_MAP);
    column->setpropertys(EVARP_TEXT, "interface");
    column->setpropertyi(EVARP_TYPE, OS_STR);
    column->setpropertys(EVARP_TTIP,
        "Bind the end point only to a specific network interface.");
     */

    /* column = new eVariable(columns);
    column->addname(enet_endp_netname, ENAME_NO_MAP);
    column->setpropertyi(EVARP_TYPE, OS_STR);
    column->setpropertys(EVARP_TEXT, "iocom network");
    column->setpropertys(EVARP_TTIP,
        "Device network name, used only with IOCOM protocol."); */

    /* column = new eVariable(columns);
    column->addname(enet_endp_serv_cert, ENAME_NO_MAP);
    column->setpropertys(EVARP_TEXT, "server certificate");
    column->setpropertyi(EVARP_TYPE, OS_STR);
    column->setpropertys(EVARP_TTIP,
        "Server certificate for this end point.\n"
        "If empty, the common cerver certificate is used."); */

    column = new eVariable(columns);
    column->addname("active", ENAME_NO_MAP);
    column->setpropertyi(EVARP_TYPE, OS_INT);
    column->setpropertys(EVARP_TEXT, "active connections");
    column->setpropertys(EVARP_ATTR, "nosave");
    column->setpropertys(EVARP_TTIP,
        "Number of active connections on this end point");

    column = new eVariable(columns);
    column->addname("tstamp", ENAME_NO_MAP);
    column->setpropertys(EVARP_TEXT, "last connection");
    column->setpropertyi(EVARP_TYPE, OS_LONG);
    column->setpropertys(EVARP_ATTR, "tstamp=\"yy,sec\",nosave");
    column->setpropertys(EVARP_TTIP,
        "Time stamp of the last successfull connect");

    /* ETABLE_ADOPT_ARGUMENT -> configuration will be released from memory.
     */
    m_endpoint_matrix->configure(configuration, ETABLE_ADOPT_ARGUMENT);
    m_endpoint_matrix->setflags(EOBJ_TEMPORARY_CALLBACK);

    m_end_points->load_file("endpoints.eo");
    m_end_points->setflags(EOBJ_TEMPORARY_CALLBACK);

    if (m_endpoint_matrix->nrows() == 0) {
        enable_by_default = (flags & ENET_DEFAULT_NO_END_POINTS) ? OS_FALSE : OS_TRUE;
        add_end_point(enable_by_default, "ecom", ENET_ENDP_TLS_IPV4,
            ENET_DEFAULT_TLS_PORT_STR);
        add_end_point(enable_by_default, "iocom", ENET_ENDP_TLS_IPV4,
            IOC_DEFAULT_TLS_PORT_STR);

/* TESTING */
add_end_point(OS_TRUE, "iocom", ENET_ENDP_SOCKET_IPV4,
    IOC_DEFAULT_SOCKET_PORT_STR, "iocafenet");

    }
}


/**
****************************************************************************************************

  @brief Add a line for an end point to "end point" table.

  The eNetService::add_end_point function...

  @param  protocol Protocol name to add.
  @param  transport_ix 1 = ENET_ENDP_SOCKET_IPV4, 2 = ENET_ENDP_SOCKET_IPV6, 3 = ENET_ENDP_TLS_IPV4,
          4 = ENET_ENDP_TLS_IPV6 or 5 = ENET_ENDP_SERIAL.

****************************************************************************************************
*/
void eNetService::add_end_point(
    os_int enable,
    const os_char *protocol,
    enetEndpTransportIx transport_ix,
    const os_char *port,
    const os_char *netname,
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

    element = new eVariable(&row);
    element->addname("protocol", ENAME_NO_MAP);
    element->sets(protocol);

    element = new eVariable(&row);
    element->addname("transport", ENAME_NO_MAP);
    element->setl(transport_ix);

    if (port) {
        element = new eVariable(&row);
        element->addname("port", ENAME_NO_MAP);
        element->sets(port);
    }

    /* if (netname) {
        element = new eVariable(&row);
        element->addname("netname", ENAME_NO_MAP);
        element->sets(netname);
    } */

    m_endpoint_matrix->insert(&row);
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
