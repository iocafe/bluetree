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

/* "connect to" table column names.
 */
const os_char enet_conn_enable[] = "enable";
const os_char enet_conn_name[] = "name";
const os_char enet_conn_protocol[] = "protocol";
const os_char enet_conn_ip[] = "ip";
const os_char enet_conn_transport[] = "transport";

/**
****************************************************************************************************

  @brief Create "connect to" table.

  The connect table specifies ecom and iocom connection to establish. This table can be edited
  by user.

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
    m_connect_to_matrix = new eMatrix(m_connect);
    m_connect_to_matrix->addname("connect");
    m_connect_to_matrix->setpropertys(ETABLEP_TEXT, "connect to");

    configuration = new eContainer(this);
    columns = new eContainer(configuration, EOID_TABLE_COLUMNS);
    columns->addname("columns", ENAME_NO_MAP);

    /* For matrix as a table row number is always the first column in configuration.
     */
    column = new eVariable(columns);
    column->addname("ix", ENAME_NO_MAP);
    column->setpropertys(EVARP_TEXT, "row");
    column->setpropertyi(EVARP_TYPE, OS_INT);
    column->setpropertys(EVARP_ATTR, "rdonly");

    column = new eVariable(columns);
    column->addname(enet_conn_enable, ENAME_NO_MAP);
    column->setpropertys(EVARP_TEXT, "enable");
    column->setpropertyi(EVARP_TYPE, OS_BOOLEAN);
    column->setpropertyi(EVARP_DEFAULT, OS_TRUE);
    column->setpropertys(EVARP_TTIP,
        "Enable this row.");

    column = new eVariable(columns);
    column->addname("name", ENAME_NO_MAP);
    column->setpropertys(EVARP_TEXT, enet_conn_name);
    column->setpropertyi(EVARP_TYPE, OS_STR);
    column->setpropertys(EVARP_DEFAULT, "*");
    column->setpropertys(EVARP_TTIP,
        "Process name or IO network name to connect to. This can be a list, if detecting\n"
        "services by lighthouse. Wildcard \'*\' indicates that any name will be connected to.");

    column = new eVariable(columns);
    column->addname(enet_conn_protocol, ENAME_NO_MAP);
    column->setpropertys(EVARP_TEXT, "protocol");
    column->setpropertyi(EVARP_TYPE, OS_STR);
    column->setpropertys(EVARP_ATTR, "list=\"ecom,iocom\"");
    column->setpropertys(EVARP_DEFAULT, "ecom");
    column->setpropertys(EVARP_TTIP,
        "Communication protocol.\n"
        "- \'ecom\': eobjects communication protocol (for glass user interface, etc).\n"
        "- \'iocom\': IO device communication protocol.\n");

    column = new eVariable(columns);
    column->addname(enet_conn_ip, ENAME_NO_MAP);
    column->setpropertys(EVARP_TEXT, "address/port");
    column->setpropertyi(EVARP_TYPE, OS_STR);
    column->setpropertys(EVARP_DEFAULT, "localhost");
    column->setpropertys(EVARP_TTIP,
        "IP andress and optional port number, COM port, or \'*\' to connect to addressed determined\n"
        "by lighthouse UDP multicasts. Examples: \'192.168.1.222\', \'192.168.1.222:666\', \'*\',\n"
        "or \'COM1:115200\'");

    column = new eVariable(columns);
    column->addname(enet_conn_transport, ENAME_NO_MAP);
    column->setpropertys(EVARP_TEXT, "transport");
    column->setpropertyi(EVARP_TYPE, OS_CHAR);
    column->setpropertys(EVARP_ATTR, "enum=\"1.SOCKET,2.TLS,3.SERIAL\"");
    column->setpropertyi(EVARP_DEFAULT, 2);
    column->setpropertys(EVARP_TTIP,
        "Transport to use.\n"
        "- \'SOCKET\': Plain socket connection, unsecured.\n"
        "- \'TLS\': TLS connection.\n"
        "- \'SERIAL\': Serial communication.\n");

#if 0
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
        "Number of active connections on resulting this row.");

    column = new eVariable(columns);
    column->addname("tstamp", ENAME_NO_MAP);
    column->setpropertys(EVARP_TEXT, "last connection");
    column->setpropertyi(EVARP_TYPE, OS_LONG);
    column->setpropertys(EVARP_ATTR, "tstamp=\"yy,sec\",nosave,rdonly");
    column->setpropertys(EVARP_TTIP,
        "Time stamp of the last successfull connect.");
#endif

    /* ETABLE_ADOPT_ARGUMENT -> configuration will be released from memory.
     */
    m_connect_to_matrix->configure(configuration, ETABLE_ADOPT_ARGUMENT);
    m_connect_to_matrix->setflags(EOBJ_TEMPORARY_CALLBACK);

    m_connect->load_file("connect.eo");
    m_connect->setflags(EOBJ_TEMPORARY_CALLBACK);

    if (m_connect_to_matrix->nrows() == 0) {
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
    element->addname(enet_conn_enable, ENAME_NO_MAP);
    element->setl(enable);

    if (name) {
        element = new eVariable(&row);
        element->addname(enet_conn_name, ENAME_NO_MAP);
        element->sets(name);
    }

    if (protocol) {
        element = new eVariable(&row);
        element->addname(enet_conn_protocol, ENAME_NO_MAP);
        element->sets(protocol);
    }

    if (ip) {
        element = new eVariable(&row);
        element->addname(enet_conn_ip, ENAME_NO_MAP);
        element->sets(ip);
    }

    element = new eVariable(&row);
    element->addname(enet_conn_transport, ENAME_NO_MAP);
    element->setl(transport);

    m_connect_to_matrix->insert(&row);
}


/**
****************************************************************************************************

  @brief Create "socket list" table.

  The socket list has one row for each socket (or other connection) to create.

  It is a temporary table created by merging "connect to" and "LAN services" tables to create
  connection list where each row represents a process to connect to.

  The source tables "connect to" and "LAN services" do belong to eNetService (eProcess) and thus
  os_lock() must be on when accessing these. The "LAN services can also have wildcard IP
  addressess where one tow represents connection to multiple processedd.

  The connect processess table belongs to eNetMaintainThread, no lock needed. It contains
  precisely one for row each socket or other connection to create.


****************************************************************************************************
*/
void eNetMaintainThread::create_socket_list_table()
{
    eContainer *configuration, *columns;
    eVariable *column;

    m_socket_list_matrix = new eMatrix(this);
    m_socket_list_matrix->addname("socketlist");
    m_socket_list_matrix->setpropertys(ETABLEP_TEXT, "socket list");

    configuration = new eContainer(this);
    columns = new eContainer(configuration, EOID_TABLE_COLUMNS);
    columns->addname("columns", ENAME_NO_MAP);

    /* For matrix as a table row number is always the first column in configuration.
     */
    column = new eVariable(columns);
    column->addname("ix", ENAME_NO_MAP);
    column->setpropertys(EVARP_TEXT, "row");
    column->setpropertyi(EVARP_TYPE, OS_INT);
    column->setpropertys(EVARP_ATTR, "rdonly");

    column = new eVariable(columns);
    column->addname("name", ENAME_NO_MAP);
    column->setpropertys(EVARP_TEXT, enet_conn_name);
    column->setpropertyi(EVARP_TYPE, OS_STR);
    column->setpropertys(EVARP_ATTR, "rdonly");
    column->setpropertys(EVARP_TTIP,
        "Process or IO network name to connect to. This can be a list, if detecting\n"
        "services by lighthouse. Wildcard \'*\' indicates that any name will be connected to.");

    column = new eVariable(columns);
    column->addname(enet_conn_protocol, ENAME_NO_MAP);
    column->setpropertys(EVARP_TEXT, "protocol");
    column->setpropertyi(EVARP_TYPE, OS_STR);
    column->setpropertys(EVARP_ATTR, "rdonly");
    column->setpropertys(EVARP_TTIP,
        "Communication protocol.\n"
        "- \'ecom\': eobjects communication protocol (for glass user interface, etc).\n"
        "- \'iocom\': IO device communication protocol.\n");

    column = new eVariable(columns);
    column->addname(enet_conn_ip, ENAME_NO_MAP);
    column->setpropertys(EVARP_TEXT, "address/port");
    column->setpropertyi(EVARP_TYPE, OS_STR);
    column->setpropertys(EVARP_ATTR, "rdonly");
    column->setpropertys(EVARP_TTIP,
        "IP andress and port number, COM port");

    column = new eVariable(columns);
    column->addname(enet_conn_transport, ENAME_NO_MAP);
    column->setpropertys(EVARP_TEXT, "transport");
    column->setpropertyi(EVARP_TYPE, OS_CHAR);
    column->setpropertys(EVARP_ATTR, "enum=\"1.SOCKET,2.TLS,3.SERIAL\",rdonly");
    column->setpropertys(EVARP_TTIP,
        "Transport to use.\n"
        "- \'SOCKET\': unsecured socket.\n"
        "- \'TLS\': secure TLS socket.\n"
        "- \'SERIAL\': serial communication.\n");

    /* ETABLE_ADOPT_ARGUMENT -> configuration will be released from memory.
     */
    m_socket_list_matrix->configure(configuration, ETABLE_ADOPT_ARGUMENT);
}


/**
****************************************************************************************************

  @brief Create list of sockets to create.

  Merges "connect" to and "LAN services" tables to create list of sockets (or other connections)
  to create.

  Source tables belong to eNetService (eProcess) and thus os_lock() must be on when accessing
  these. Destination table belongs to eNetMaintainThread, no lock needed.

****************************************************************************************************
*/
void eNetMaintainThread::merge_to_socket_list()
{
    eMatrix *m, *lh;
    eContainer *localvars, *rows, *conf, *columns;
    eContainer *addr_blocklist, *name_blocklist;
    eVariable *namelist, *protocol, *ip;
    os_char *ip_and_port_str, name_str[IOC_DEVICE_ID_SZ], ip_str[OSAL_IPADDR_SZ];
    const os_char *p, *lh_name_str, *lh_ip_str, *lh_nick_str;
    os_int enable_col, name_col, protocol_col, transport_col, ip_col, port_nr;
    eVariable *lh_name, *lh_nick, *lh_protocol, *lh_ip;
    os_int lh_name_col, lh_nick_col, lh_protocol_col, lh_ip_col, lh_tlsport_col, lh_tcpport_col;
    os_int h, lh_h, con_nr, i;
    enetConnTransportIx transport_ix;
    os_boolean is_ipv6;

    /* Keep track if remakr
     */
    m_trigger_connect_check_by_lighthouse = OS_FALSE;

    localvars = new eContainer(ETEMPORARY);
    namelist = new eVariable(localvars);
    protocol = new eVariable(localvars);
    ip = new eVariable(localvars);
    lh_name = new eVariable(localvars);
    lh_nick = new eVariable(localvars);
    lh_protocol = new eVariable(localvars);
    lh_ip = new eVariable(localvars);

    os_lock();

    /* Get "connect to" matrix column numbers.
     */
    m = m_netservice->m_connect_to_matrix;
    conf = m->configuration();
    if (conf == OS_NULL) goto getout_unlock;
    columns = conf->firstc(EOID_TABLE_COLUMNS);
    if (columns == OS_NULL) goto getout_unlock;
    enable_col = etable_column_ix(enet_conn_enable, columns);
    name_col = etable_column_ix(enet_conn_name, columns);
    protocol_col = etable_column_ix(enet_conn_protocol, columns);
    ip_col = etable_column_ix(enet_conn_ip, columns);
    transport_col = etable_column_ix(enet_conn_transport, columns);

    /* Get "LAN services" matrix column numbers.
     */
    lh = m_netservice->m_services_matrix;
    conf = lh->configuration();
    if (conf == OS_NULL) goto getout_unlock;
    columns = conf->firstc(EOID_TABLE_COLUMNS);
    lh_name_col = etable_column_ix(enet_lansrv_name, columns);
    lh_nick_col = etable_column_ix(enet_lansrv_nick, columns);
    lh_protocol_col = etable_column_ix(enet_lansrv_protocol, columns);
    lh_ip_col = etable_column_ix(enet_lansrv_ip, columns);
    lh_tlsport_col = etable_column_ix(enet_lansrv_tlsport, columns);
    lh_tcpport_col = etable_column_ix(enet_lansrv_tcpport, columns);

    /* Generate block lists.
     */
    addr_blocklist  = new eContainer(localvars);
    name_blocklist  = new eContainer(localvars);
    addr_blocklist->ns_create();
    name_blocklist->ns_create();
    os_unlock();

    /* Remove all rows from socket list.
     */
    m_socket_list_matrix->remove("1");

    rows = new eContainer(localvars);
    os_lock();
    h = m->nrows();
    lh_h = lh->nrows();
    for (con_nr = 0; con_nr < h; con_nr++) {
        if ((m->geti(con_nr, EMTX_FLAGS_COLUMN_NR) & EMTX_FLAGS_ROW_OK) == 0) continue;
        if (m->geti(con_nr, enable_col) == 0) continue;

        m->getv(con_nr, name_col, namelist);
        if (namelist->isempty() || namelist->type() != OS_STR) {
            namelist->sets("*");
        }
        m->getv(con_nr, ip_col, ip);
        m->getv(con_nr, protocol_col, protocol);
        transport_ix = (enetConnTransportIx)m->getl(con_nr, transport_col);

        /* Get ip address string, port number (0 if not set) and is_tls flag
         */
        ip_and_port_str = ip->gets();
        port_nr = 0;
        if (transport_ix == ENET_CONN_SOCKET || transport_ix == ENET_CONN_TLS)
        {
            osal_socket_get_ip_and_port(ip_and_port_str, ip_str, -(os_memsz)sizeof(ip_str),
                &port_nr, &is_ipv6, OSAL_STREAM_CONNECT, 0);
            if (ip_str[0] == '\0') {
                os_strncpy(ip_str, "*", sizeof(ip_str));
            }

            /* If we have no port number, use default for the protocol.
             */
            if (port_nr == 0) {
                if (!os_strcmp(protocol->gets(), "ecom")) {
                    port_nr = (transport_ix == ENET_CONN_TLS ? ENET_DEFAULT_TLS_PORT : ENET_DEFAULT_SOCKET_PORT);
                }
                else {
                    port_nr = (transport_ix == ENET_CONN_TLS ? IOC_DEFAULT_TLS_PORT : IOC_DEFAULT_SOCKET_PORT);
                }
            }
        }
        else {
            os_strncpy(ip_str, ip_and_port_str, sizeof(ip_str));
        }

        p = namelist->gets();
        while (OS_TRUE) {
            if (osal_str_list_iter(name_str, sizeof(name_str), &p, OSAL_STRING_DEFAULT)) {
                break;
            }
            if (name_str[0] == '\0') continue;

            /* If we have real IP address, add it. We can have only one name.
             */
            if (os_strcmp(ip_str, "*"))
            {
                add_socket_to_list(name_str, protocol, transport_ix, ip_str, port_nr,
                    is_ipv6, rows, addr_blocklist, name_blocklist);
                break;
            }

            /* No light house for serial communication, etc.
             */
            if (transport_ix != ENET_CONN_SOCKET && transport_ix != ENET_CONN_TLS) {
                continue;
            }

            /* We need to redu merge when lighthouse data is received.
             */
            m_trigger_connect_check_by_lighthouse = OS_TRUE;

            /* No ip set, we use lighthouse.
             */
            for (i = 0; i < lh_h; i++) {
                lh->getv(i, lh_name_col, lh_name);
                lh_name_str = lh_name->gets();
                lh->getv(i, lh_nick_col, lh_nick);
                lh_nick_str = lh_nick->gets();
                lh->getv(i, lh_ip_col, lh_ip);
                lh_ip_str = lh_ip->gets();

                /* Skip ones which do not match wildcard name or no matching protocol.
                 */
                if (!osal_pattern_match(lh_name_str, name_str, 0) &&
                    !osal_pattern_match(lh_nick_str, name_str, 0))
                {
                    continue;
                }
                lh->getv(i, lh_protocol_col, lh_protocol);
                if (lh_protocol->compare(protocol)) continue;

                if (!osal_pattern_match(lh_ip_str, ip_str, 0))
                {
                    continue;
                }

                if (transport_ix == ENET_CONN_TLS) {
                    port_nr = lh->geti(i, lh_tlsport_col);
                }
                else {
                    port_nr = lh->geti(i, lh_tcpport_col);
                }

                if (port_nr) {
                    add_socket_to_list(lh_name_str, protocol, transport_ix, lh_ip_str, port_nr,
                        is_ipv6, rows, addr_blocklist, name_blocklist);
                }
            }
        }
    }
    os_unlock();

    if (rows->firstc()) {
        m_socket_list_matrix->insert(rows, ETABLE_ADOPT_ARGUMENT);
    }

    delete localvars;
    return;

getout_unlock:
    os_unlock();
    delete localvars;
    osal_debug_error("maintain_connections() failed");
}



/* Add a row to socket list.
 */
void eNetMaintainThread::add_socket_to_list(
    const os_char *name,
    eVariable *protocol,
    enetConnTransportIx transport_ix,
    const os_char *ip,
    os_int port_nr,
    os_boolean is_ipv6,
    eContainer *rows,
    eContainer *addr_blocklist,
    eContainer *name_blocklist)
{
    eContainer *row, *c;
    eVariable *v;

    if (name_blocklist->byname(name)) {
        return;
    }

    row = new eContainer(rows);

    /* Set address column, merge ip and port
     */
    v = new eVariable(row);
    v->addname(enet_conn_ip, ENAME_NO_MAP);
    v->sets(ip);
    v->appends(":");
    v->appendl(port_nr);

    if (addr_blocklist->byname(v->gets())) {
        delete row;
        return;
    }

    /* Update block lists.
     */
    c = new eContainer(name_blocklist);
    c->addname(v->gets());

    if (os_strcmp(name, "*")) {
        c = new eContainer(name_blocklist);
        c->addname(name);
    }

    /* Set rest of columns.
     */
    v = new eVariable(row);
    v->addname(enet_conn_name, ENAME_NO_MAP);
    v->sets(name);

    v = new eVariable(row);
    v->addname(enet_conn_protocol, ENAME_NO_MAP);
    v->setv(protocol);

    v = new eVariable(row);
    v->addname(enet_conn_transport, ENAME_NO_MAP);
    v->setl(transport_ix);
}


/**
****************************************************************************************************

  @brief Create and delete connections as needed.

  The eNetMaintainThread::maintain_connections() function processes socket list and creates,
  deletes or updates socket connections.

****************************************************************************************************
*/
void eNetMaintainThread::maintain_connections()
{
    eProtocol *proto;
    eProtocolHandle *handle;
    eConnectParameters prm;
    eMatrix *m;
    eObject *conf;
    eContainer *localvars, *columns, *con, *next_con, *index, *c, *next_c;
    eVariable *con_name, *ip, *protocol;
    eVariable *ip_p, *protocol_p, *transport_p;
    os_int protocol_col, transport_col, ip_col;
    os_int h, con_nr;
    enetConnTransportIx transport_ix;
    eStatus s;

    localvars = new eContainer(ETEMPORARY);
    con_name = new eVariable(localvars);
    ip = new eVariable(localvars);
    protocol = new eVariable(localvars);

    m = m_socket_list_matrix;
    conf = m->configuration();
    if (conf == OS_NULL) goto getout;
    columns = conf->firstc(EOID_TABLE_COLUMNS);
    if (columns == OS_NULL) goto getout;

    // name_col = etable_column_ix(enet_conn_name, columns);
    protocol_col = etable_column_ix(enet_conn_protocol, columns);
    ip_col = etable_column_ix(enet_conn_ip, columns);
    transport_col = etable_column_ix(enet_conn_transport, columns);

    /* Make index "name->row nr" for socket list.
     */
    index = new eContainer(localvars);
    index->ns_create();
    h = m->nrows();
    for (con_nr = 0; con_nr < h; con_nr++)
    {
        if ((m->geti(con_nr, EMTX_FLAGS_COLUMN_NR) & EMTX_FLAGS_ROW_OK) == 0) continue;

        m->getv(con_nr, ip_col, ip);
        m->getv(con_nr, protocol_col, protocol);
        transport_ix = (enetConnTransportIx)m->getl(con_nr, transport_col);
        make_connection_name(con_name, protocol, ip, transport_ix);

        c = new eContainer(index, con_nr);
        c->addname(con_name->gets());
    }

    /* Deactivate connections which are no longer needed.
     */
    for (con = m_connections->firstc(); con; con = next_con)
    {
        next_con = con->nextc();

        ip_p = con->firstv(ENET_CONN_IP);
        protocol_p = con->firstv(ENET_CONN_PROTOCOL);
        transport_p = con->firstv(ENET_CONN_TRANSPORT);
        if (ip_p == OS_NULL || protocol_p == OS_NULL || transport_p == OS_NULL) {
            osal_debug_error("maintain_connections error 1");
            continue;
        }
        transport_ix = (enetConnTransportIx)transport_p->getl();
        make_connection_name(con_name, protocol_p, ip_p, transport_ix);

        /* If connection is still needed, do not deactivate.
         */
        if (index->byname(con_name->gets())) continue;

        proto = protocol_by_name(protocol_p);
        handle = eProtocolHandle::cast(con->first(ENET_CONN_PROTOCOL_HANDLE));
        osal_debug_assert(proto != OS_NULL);
        osal_debug_assert(handle != OS_NULL);

        if (proto->is_connection_running(handle)) {
            proto->deactivate_connection(handle);
        }
        else {
            delete con;
        }
    }

    /* Update exisiting connections. Loop using socket list index.
     */
    for (c = index->firstc(); c; c = next_c)
    {
        next_c = c->nextc();

        /* Row of socket list table is:
         */
        con_nr = c->oid();

        /* Make connection name
         */
        m->getv(con_nr, ip_col, ip);
        m->getv(con_nr, protocol_col, protocol);
        transport_ix = (enetConnTransportIx)m->getl(con_nr, transport_col);
        make_connection_name(con_name, protocol, ip, transport_ix);

        /* If we already have this connection, activate it. Either
         */
        con = eContainer::cast(m_connections->byname(con_name->gets()));
        if (con) {
            proto = protocol_by_name(protocol);
            handle = eProtocolHandle::cast(con->first(ENET_CONN_PROTOCOL_HANDLE));
            osal_debug_assert(proto != OS_NULL);
            osal_debug_assert(handle != OS_NULL);

            os_memclear(&prm, sizeof(prm));
            prm.parameters = ip->gets();
            prm.transport = transport_ix;

            if (proto->is_connection_running(handle)) {
                s = proto->activate_connection(handle, &prm);
                if (s) {
                    osal_debug_error_int("proto->activate_connection: ", s);
                }

                /* Remove from connection index (just small speed optimization)
                 */
                delete c;
            }
            else {
                delete con;
            }
        }
    }

    /* Create new connections. Loop using socket list index.
     */
    for (c = index->firstc(); c; c = next_c)
    {
        next_c = c->nextc();

        /* Row of socket list table is:
         */
        con_nr = c->oid();

        /* Make connection name
         */
        m->getv(con_nr, ip_col, ip);
        m->getv(con_nr, protocol_col, protocol);
        transport_ix = (enetConnTransportIx)m->getl(con_nr, transport_col);
        make_connection_name(con_name, protocol, ip, transport_ix);

        /* If we do not have this connection, create it.
         */
        if (m_connections->byname(con_name->gets()) == OS_NULL)
        {
            os_memclear(&prm, sizeof(prm));
            prm.parameters = ip->gets();
            prm.transport = transport_ix;

            proto = protocol_by_name(protocol);
            if (proto == OS_NULL) {
                osal_debug_error_str("new_connection: unknown protocol: ", protocol->gets());
                continue;
            }

            handle = proto->new_connection(con_name, &prm, &s);
            if (s) {
                osal_debug_error_int("proto->new_connection: ", s);
                delete handle;
                continue;
            }

            con = new eContainer(m_connections);
            con->addname(con_name->gets());
            ip_p = new eVariable(con, ENET_CONN_IP);
            ip_p->setv(ip);
            protocol_p = new eVariable(con, ENET_CONN_PROTOCOL);
            protocol_p->setv(protocol);
            transport_p = new eVariable(con, ENET_CONN_TRANSPORT);
            transport_p->setl(transport_ix);
            handle->adopt(con, ENET_CONN_PROTOCOL_HANDLE);
        }
    }

    delete localvars;
    return;

getout:
    delete localvars;
    osal_debug_error("maintain_connections() failed");
}

/* Generate name for a connection.
 * Connection name is used to identify connection to specific process (ip and port)
   @param   con_name Pointer to eVariable where to store resulting connection name.
 */
void eNetMaintainThread::make_connection_name(
    eVariable *con_name,
    eVariable *protocol,
    eVariable *ip,
    enetConnTransportIx transport_ix)
{
    os_char *p, c, buf[OSAL_IPADDR_AND_PORT_SZ + 64], *d, *buf_stop;
    const os_char *transport_name;

    con_name->setv(protocol);
    switch (transport_ix)
    {
        case ENET_CONN_SOCKET: transport_name = "socket_"; break;
        case ENET_CONN_TLS:    transport_name = "tls_"; break;
        case ENET_CONN_SERIAL: transport_name = "serial_"; break;
        default: transport_name = "_unknown_"; break;
    }

    con_name->appends("_c");
    con_name->appends(transport_name);
    p = ip->gets();
    d = buf;
    buf_stop = buf + sizeof(buf) - 6;
    while ((c = *p) != '\0' && d < buf_stop) {
        if (osal_char_isprint(c))
        {
            switch (c) {
                case '[':
                    *(d++) = '_';
                    *(d++) = 't';
                    *(d++) = 'l';
                    *(d++) = 's';
                    *(d++) = '_';
                    break;

                case ']':
                    break;

                case '.':
                    *(d++) = '.';
                    break;

                default:
                    if (osal_char_isaplha(c) || osal_char_isdigit(c)) {
                        *(d++) = c;
                    }
                    else {
                        *(d++) = '_';
                    }
                    break;
            }
        }
        else {
            *(d++) = '_';
        }
        p++;
    }
    *(d++) = '\0';
    con_name->appends(buf);
}


void eNetMaintainThread::delete_con(
    eContainer *con)
{
    eVariable *proto_name;
    eProtocol *proto;
    eProtocolHandle *handle;

    proto_name = con->firstv(ENET_CONN_PROTOCOL);
    proto = protocol_by_name(proto_name);
    if (proto == OS_NULL) return;

    handle = eProtocolHandle::cast(con->first(ENET_CONN_PROTOCOL_HANDLE));
    if (proto->is_connection_running(handle))
    {
        proto->delete_connection(handle);
        while (proto->is_connection_running(handle)) {
            os_timeslice();
        }
    }
    delete con;
}
