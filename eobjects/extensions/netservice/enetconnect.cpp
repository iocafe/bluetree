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
#include "eobjects.h"
#include "extensions/netservice/enetservice.h"


/**
****************************************************************************************************

  @brief Create "connections" table.

  The eNetService::create_connect_table function...

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
    column->setpropertys(EVARP_TTIP,
        "Enable this connection.");

    column = new eVariable(columns);
    column->addname("protocol", ENAME_NO_MAP);
    column->setpropertys(EVARP_TEXT, "protocol");
    column->setpropertyi(EVARP_TYPE, OS_CHAR);
    column->setpropertys(EVARP_ATTR, "enum=\"1.eobjects,2.iocom\"");
    column->setpropertys(EVARP_TTIP,
        "Connect using this protocol.\n"
        "- \'eobjects\': eobjects communication protocol (for glass user interface, etc).\n"
        "- \'iocom\': IO device communication protocol.\n");

    column = new eVariable(columns);
    column->addname("connection", ENAME_NO_MAP);
    column->setpropertys(EVARP_TEXT, "connection");
    column->setpropertyi(EVARP_TYPE, OS_STR);
    column->setpropertys(EVARP_TTIP,
        "IP andress and optional TCP port number, or \'*\' to connect to addressed determined\n"
        "by lighthouse UDP multicasts. Examples: \'192.168.1.222\', \'192.168.1.222:666\', \'*\',\n"
        "or \'COM1:115200\'");

    column = new eVariable(columns);
    column->addname("devices", ENAME_NO_MAP);
    column->setpropertys(EVARP_TEXT, "devices");
    column->setpropertyi(EVARP_TYPE, OS_STR);
    column->setpropertys(EVARP_TTIP,
        "List of device to connect to. Wildcard \'*\' can be used.");

    column = new eVariable(columns);
    column->addname("network", ENAME_NO_MAP);
    column->setpropertys(EVARP_TEXT, "network");
    column->setpropertyi(EVARP_TYPE, OS_STR);
    column->setpropertys(EVARP_TTIP,
        "Device network name to connect to. Wildcard \'*\' to connect to any network.");

    column = new eVariable(columns);
    column->addname("transport", ENAME_NO_MAP);
    column->setpropertys(EVARP_TEXT, "transport");
    column->setpropertyi(EVARP_TYPE, OS_CHAR);
    column->setpropertys(EVARP_ATTR, "enum=\"1.SOCKET,2.TLS,3.SERIAL\"");
    column->setpropertys(EVARP_TTIP,
        "Transport to use.\n"
        "- \'SOCKET\': Plain socket connection, unsecured.\n"
        "- \'TLS\': TLS connection.\n"
        "- \'SERIAL\': Serial communication.\n");

    column = new eVariable(columns);
    column->addname("active", ENAME_NO_MAP);
    column->setpropertyi(EVARP_TYPE, OS_INT);
    column->setpropertys(EVARP_TEXT, "active");
    column->setpropertys(EVARP_ATTR, "nosave");
    column->setpropertys(EVARP_TTIP,
        "Number of active connections on resulting this end point");

    column = new eVariable(columns);
    column->addname("tstamp", ENAME_NO_MAP);
    column->setpropertys(EVARP_TEXT, "last connection");
    column->setpropertyi(EVARP_TYPE, OS_LONG);
    column->setpropertys(EVARP_ATTR, "tstamp=\"yy,sec\",nosave");
    column->setpropertys(EVARP_TTIP,
        "Time stamp of the last successfull connect");

    /* ETABLE_ADOPT_ARGUMENT -> configuration will be released from memory.
     */
    m_connection_matrix->configure(configuration, ETABLE_ADOPT_ARGUMENT);
    m_connection_matrix->setflags(EOBJ_TEMPORARY_CALLBACK);

    m_connect->load_file("connections.eo");

    if (m_connection_matrix->nrows() == 0) {
        add_connection(OS_TRUE, 1, "*", "*", 1);
        add_connection(OS_TRUE, 2, "*", "*", 1);
    }
}


/**
****************************************************************************************************

  @brief Add a row for a connection to "connections" table.

  The eNetService::add_connection function...

****************************************************************************************************
*/
void eNetService::add_connection(
    os_int enable,
    os_int protocol,
    const os_char *connection,
    const os_char *devices,
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

    element = new eVariable(&row);
    element->addname("protocol", ENAME_NO_MAP);
    element->setl(protocol);

    if (connection) {
        element = new eVariable(&row);
        element->addname("connection", ENAME_NO_MAP);
        element->sets(connection);
    }

    if (devices) {
        element = new eVariable(&row);
        element->addname("devices", ENAME_NO_MAP);
        element->sets(connection);
    }

    element = new eVariable(&row);
    element->addname("transport", ENAME_NO_MAP);
    element->setl(transport);

    m_connection_matrix->insert(&row);
}
