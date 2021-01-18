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
    eMatrix *m;
    eContainer *localvars, *row, *rows, *conf, *columns;
    eVariable *name, *protocol, *transport, *ip;
    os_int enable_col, name_col, protocol_col, transport_col, ip_col;
    os_int h, con_nr;

    localvars = new eContainer(ETEMPORARY);

    /* Get "connect to" matrix column numbers.
     */
    os_lock();
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
    os_unlock();

    m_socket_list_matrix->remove("1");

    rows = new eContainer(localvars);
    os_lock();
    h = m->nrows();
    for (con_nr = 0; con_nr < h; con_nr ++) {
        if ((m->geti(con_nr, EMTX_FLAGS_COLUMN_NR) & EMTX_FLAGS_ROW_OK) == 0) continue;
        if (m->geti(con_nr, enable_col) == 0) continue;

        row = new eContainer(rows);

        name = new eVariable(row);
        name->addname(enet_conn_name, ENAME_NO_MAP);
        m->getv(con_nr, name_col, name);

        protocol = new eVariable(row);
        protocol->addname(enet_conn_protocol, ENAME_NO_MAP);
        m->getv(con_nr, protocol_col, protocol);

        transport = new eVariable(row);
        transport->addname(enet_conn_transport, ENAME_NO_MAP);
        m->getv(con_nr, transport_col, transport);

        ip = new eVariable(row);
        ip->addname(enet_conn_ip, ENAME_NO_MAP);
        m->getv(con_nr, ip_col, ip);
    }
    os_unlock();

    m_socket_list_matrix->insert(rows, ETABLE_ADOPT_ARGUMENT);


    delete localvars;
    return;

getout_unlock:
    os_unlock();
    delete localvars;
    osal_debug_error("maintain_connections() failed");
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
    eContainer *localvars, *list, *con, *next_con;
    eVariable *v, *proto_name;
    const os_char *p;
    os_int enable_col, name_col, protocol_col, transport_col, ip_col;
    os_int h, con_nr;
    eVariable tmp;
    eStatus s;
    os_boolean changed = OS_FALSE;

    localvars = new eContainer(ETEMPORARY);

    os_lock();
    m = m_netservice->m_connect_to_matrix;
    conf = m->configuration();
    if (conf == OS_NULL) goto getout_unlock;
    columns = conf->first(EOID_TABLE_COLUMNS);
    if (columns == OS_NULL) goto getout_unlock;
    col = columns->byname(enet_conn_enable);
    if (col == OS_NULL) goto getout_unlock;
    enable_col = col->oid();

    col = columns->byname(enet_conn_name);
    if (col == OS_NULL) goto getout_unlock;
    name_col = col->oid();

    col = columns->byname(enet_conn_protocol);
    if (col == OS_NULL) goto getout_unlock;
    protocol_col = col->oid();

    col = columns->byname(enet_conn_ip);
    if (col == OS_NULL) goto getout_unlock;
    ip_col = col->oid();

    col = columns->byname(enet_conn_transport);
    if (col == OS_NULL) goto getout_unlock;
    transport_col = col->oid();
    os_unlock();

    /* Merge connect to and LAN services tables to create connection
       list where each row represents a process to connect to.
       Source tables belong to eNetService (eProcess) and thus os_lock()
       must be on when accessing these. Destination table belongs to
       eNetMaintainThread, no lock needed.
     */


    /* Deactivate connections which are no longer needed or have changed.
     */
    for (con = m_connections->firstc(); con; con = next_con)
    {
        next_con = con->nextc();
        con_nr = con->oid();
        proto_name = con->firstv(ENET_CONN_PROTOCOL);
        proto = protocol_by_name(proto_name);
        if (proto == OS_NULL) {
            osal_debug_error_str("Program error, unknown proto ", proto_name->gets());
            delete con;
            continue;
        }
        handle = eProtocolHandle::cast(con->first(ENET_CONN_PROTOCOL_HANDLE));
        if (!proto->is_connection_running(handle)) continue;

        os_timeslice();
        os_lock();

        if ((m->geti(con_nr, EMTX_FLAGS_COLUMN_NR) & EMTX_FLAGS_ROW_OK) == 0) goto delete_it;
        if (m->geti(con_nr, enable_col) == 0) goto delete_it;
        m->getv(con_nr, protocol_col, &tmp);
        if (tmp.compare(proto_name)) goto delete_it;
        v = con->firstv(ENET_CONN_TRANSPORT);
        m->getv(con_nr, transport_col, &tmp);
        if (tmp.compare(v)) goto delete_it;
        v = con->firstv(ENET_CONN_IP);
        m->getv(con_nr, ip_col, &tmp);
        if (tmp.compare(v)) goto delete_it;
        v = con->firstv(ENET_CONN_NAME);
        if (v) {
            /* MISSING: We need wildcard match !!!!!!!!!!!
             */
            m->getv(con_nr, name_col, &tmp);
            p = tmp.gets();
            if (*p != '\0' && os_strstr(p, "*", OSAL_STRING_SEARCH_ITEM_NAME) == OS_NULL)
            {
                if (os_strstr(p, v->gets(), OSAL_STRING_SEARCH_ITEM_NAME) == OS_NULL) {
                    if (tmp.compare(v)) goto delete_it;
                }
            }
        }

        os_unlock();
        continue;

delete_it:
        os_unlock();
        deactivate_con(con);
        changed = OS_TRUE;
    }

    /* Generate list of connections to add.
     */
    list = new eContainer(localvars);
    os_lock();
    h = m->nrows();
    for (con_nr = 0; con_nr < h; con_nr ++) {
        if ((m->geti(con_nr, EMTX_FLAGS_COLUMN_NR) & EMTX_FLAGS_ROW_OK) == 0) continue;
        if (m->geti(con_nr, enable_col) == 0) continue;
        if (m_connections->first(con_nr)) continue;

        con = new eContainer(list, con_nr);
        v = new eVariable(con, ENET_CONN_NAME);
        m->getv(con_nr, name_col, v);
        v = new eVariable(con, ENET_CONN_PROTOCOL);
        m->getv(con_nr, protocol_col, v);
        v = new eVariable(con, ENET_CONN_TRANSPORT);
        m->getv(con_nr, transport_col, v);
        v = new eVariable(con, ENET_CONN_IP);
        m->getv(con_nr, ip_col, v);
    }
    os_unlock();

    /* Add connections (no lock).
     */
    for (con = list->firstc(); con; con = next_con)
    {
        next_con = con->nextc();
        con_nr = con->oid();
        proto_name = con->firstv(ENET_CONN_PROTOCOL);
        proto = protocol_by_name(proto_name);
        if (proto == OS_NULL) {
            osal_debug_error_str("Unknown protocol: ", proto_name->gets());
            // update status in table
            continue;
        }

        handle = proto->new_connection(con_nr, OS_NULL, &s);
        if (handle == OS_NULL) {
            osal_debug_error_str("unable to create connection: ", proto_name->gets());
            // update status in table, status s
            continue;
        }
        handle->adopt(con, ENET_CONN_PROTOCOL_HANDLE);

        /* Adopt, successfull created connection.
         */
        con->adopt(m_connections, con_nr);
        changed = OS_TRUE;
    }

    delete localvars;
    return;

getout_unlock:
    os_unlock();
    delete localvars;
    osal_debug_error("maintain_connections() failed");
}


void eNetMaintainThread::deactivate_con(
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
