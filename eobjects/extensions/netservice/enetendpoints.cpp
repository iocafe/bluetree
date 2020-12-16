/**

  @file    enetendpoints.cpp
  @brief   Ens points to listen to.
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


****************************************************************************************************
*/
void eNetService::create_end_point_table()
{
    eContainer *configuration, *columns;
    eVariable *column;

    m_end_points = new ePersistent(this);
    m_endpoint_matrix = new eMatrix(m_end_points);
    m_endpoint_matrix->addname("endpoints");

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
    column->addname("protocol", ENAME_NO_MAP);
    column->setpropertys(EVARP_TEXT, "protocol");
    column->setpropertyi(EVARP_TYPE, OS_CHAR);
    column->setpropertys(EVARP_ATTR, "enum=\"1.eobjects,2.iocom\"");
    column->setpropertys(EVARP_TTIP,
        "Listen for protocol.\n"
        "- \'none\': Connections by this user are not allowed.\n"
        "- \'eobjects\': eobjects communication protocol (for glass user interface, etc).\n"
        "- \'iocom\': IO device communication protocol.\n");

    column = new eVariable(columns);
    column->addname("tansport", ENAME_NO_MAP);
    column->setpropertys(EVARP_TEXT, "tansport");
    column->setpropertyi(EVARP_TYPE, OS_CHAR);
    column->setpropertys(EVARP_ATTR, "enum=\"0.none,1.socket,2.TLS,3.serial\"");
    column->setpropertys(EVARP_TTIP,
        "Listen for protocol.\n"
        "- \'none\': Connections by this user are not allowed.\n"
        "- \'eobjects\': eobjects communication protocol (for glass user interface, etc).\n"
        "- \'iocom\': IO device communication protocol.\n");

    column = new eVariable(columns);
    column->addname("port", ENAME_NO_MAP);
    column->setpropertys(EVARP_TEXT, "port/iface");
    column->setpropertyi(EVARP_TYPE, OS_STR);
    column->setpropertys(EVARP_TTIP,
        "TCP port number proceeded with IP . Examples: \'6666\',\n"
        "\'192.168.1.222:666\', or \'COM1:115200\'");

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

    m_persistent_accounts->load_file("endpoints.eo");

    if (m_endpoint_matrix->nrows() == 0) {
        add_end_point(1, 1, "5999");
        add_end_point(2, 1, IOC_DEFAULT_SOCKET_PORT_STR);
    }
}


void eNetService::add_end_point(
    os_int protocol,
    os_int transport,
    const os_char *port,
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
    element->addname("protocol", ENAME_NO_MAP);
    element->setl(protocol);

    element = new eVariable(&row);
    element->addname("transport", ENAME_NO_MAP);
    element->setl(transport);

    if (port) {
        element = new eVariable(&row);
        element->addname("port", ENAME_NO_MAP);
        element->sets(port);
    }

    m_endpoint_matrix->insert(&row);
}
