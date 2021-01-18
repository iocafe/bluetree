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
const os_char enet_endp_ok[] = "ok";
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
    column->setpropertys(EVARP_ATTR, "rdonly");

    column = new eVariable(columns);
    column->addname(enet_endp_enable, ENAME_NO_MAP);
    column->setpropertys(EVARP_TEXT, "enable");
    column->setpropertyi(EVARP_TYPE, OS_BOOLEAN);
    column->setpropertyl(EVARP_DEFAULT, OS_TRUE);
    column->setpropertys(EVARP_TTIP,
        "Create end point for this row.");

    column = new eVariable(columns);
    column->addname(enet_endp_protocol, ENAME_NO_MAP);
    column->setpropertys(EVARP_TEXT, "protocol");
    column->setpropertyi(EVARP_TYPE, OS_STR);
    column->setpropertys(EVARP_ATTR, "list=\"ecom,iocom\"");
    column->setpropertys(EVARP_DEFAULT, "iocom");
    column->setpropertys(EVARP_TTIP,
        "Communication protocol.\n"
        "- \'ecom\': object based protocol (for glass user interface, etc).\n"
        "- \'iocom\': IO device communication.\n");

    column = new eVariable(columns);
    column->addname(enet_endp_transport, ENAME_NO_MAP);
    column->setpropertys(EVARP_TEXT, "transport");
    column->setpropertyi(EVARP_TYPE, OS_CHAR);
    column->setpropertys(EVARP_ATTR, "enum=\"1.SOCKET,2.TLS,3.SERIAL\"");
    column->setpropertyl(EVARP_DEFAULT, ENET_ENDP_TLS);
    column->setpropertys(EVARP_TTIP,
        "Transport:\n"
        "- \'SOCKET\': unsecured socket connection, .\n"
        "- \'TLS\': secure TLS connection.\n"
        "- \'SERIAL\': serial communication.\n");

    column = new eVariable(columns);
    column->addname(enet_endp_port, ENAME_NO_MAP);
    column->setpropertys(EVARP_TEXT, "port");
    column->setpropertyi(EVARP_TYPE, OS_STR);
    column->setpropertys(EVARP_DEFAULT, "*");
    column->setpropertys(EVARP_TTIP,
        "Port to listen, \'*\' defaults:\n"
        "- \'6371\': ecom socket.\n"
        "- \'6374\': ecom TLS.\n"
        "- \'6368\': ecom socket.\n"
        "- \'6369\': ecom TLS.\n"
        "- \'COM1:115200\' serial port\n"
        "Netwok interface can be specified for example \'192.168.1.222:6371\'.\n"
        "Use brackets around IP address to mark IPv6 address, for\n"
        "example \'[localhost]:12345\', or \'[]:12345\' for empty IP.");

    column = new eVariable(columns);
    column->addname(enet_endp_ok, ENAME_NO_MAP);
    column->setpropertyi(EVARP_TYPE, OS_BOOLEAN);
    column->setpropertys(EVARP_TEXT, "ready");
    column->setpropertys(EVARP_ATTR, "nosave,rdonly");
    column->setpropertys(EVARP_TTIP,
        "Checked if all is good and end point is listening.");


    /* column = new eVariable(columns);
    column->addname(enet_endp_netname, ENAME_NO_MAP);
    column->setpropertyi(EVARP_TYPE, OS_STR);
    column->setpropertys(EVARP_TEXT, "iocom network");
    column->setpropertys(EVARP_TTIP,
        "Device network name, used only with IOCOM protocol."); */

    /* column = new eVariable(columns);
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
        "Time stamp of the last successfull connect"); */

    /* ETABLE_ADOPT_ARGUMENT -> configuration will be released from memory.
     */
    m_endpoint_matrix->configure(configuration, ETABLE_ADOPT_ARGUMENT);
    m_endpoint_matrix->setflags(EOBJ_TEMPORARY_CALLBACK);

    m_end_points->load_file("endpoints.eo");
    m_end_points->setflags(EOBJ_TEMPORARY_CALLBACK);

    if (m_endpoint_matrix->nrows() == 0) {
        enable_by_default = (flags & ENET_DEFAULT_NO_END_POINTS) ? OS_FALSE : OS_TRUE;
        add_end_point(enable_by_default, "ecom", ENET_ENDP_TLS, "*");
        add_end_point(enable_by_default, "iocom", ENET_ENDP_TLS, "*");
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
    element->addname(enet_endp_enable, ENAME_NO_MAP);
    element->setl(enable);

    element = new eVariable(&row);
    element->addname(enet_endp_protocol, ENAME_NO_MAP);
    element->sets(protocol);

    element = new eVariable(&row);
    element->addname(enet_endp_transport, ENAME_NO_MAP);
    element->setl(transport_ix);

    if (port) {
        element = new eVariable(&row);
        element->addname(enet_endp_port, ENAME_NO_MAP);
        element->sets(port);
    }

    /* if (netname) {
        element = new eVariable(&row);
        element->addname(enet_endp_netname, ENAME_NO_MAP);
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
    eEndPointParameters prm;
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
        m->getv(ep_nr, port_col, &tmp);
        if (tmp.compare(v)) goto delete_it;

        os_unlock();
        continue;

delete_it:
        os_unlock();
        delete_ep(ep);

        /* Uncheck io in endpoints table.
         */
        tmp.setl(OS_FALSE);
        set_ep_status(ep_nr, enet_endp_ok, &tmp);

        changed = OS_TRUE;
    }

    /* Generate list of end points to add.
     */
    list = new eContainer(localvars);
    os_lock();
    h = m->nrows();
    for (ep_nr = 0; ep_nr < h; ep_nr++) {
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
        ep->setflags(EOBJ_PERSISTENT_CALLBACK);
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

        os_memclear(&prm, sizeof(prm));
        v = ep->firstv(ENET_ENDP_PORT);
        prm.port = v->gets();
        v = ep->firstv(ENET_ENDP_TRANSPORT);
        prm.transport = (enetEndpTransportIx)v->getl();
        handle = proto->new_end_point(ep_nr, &prm, &s);
        if (handle == OS_NULL) {
            osal_debug_error_str("unable to create end point: ", proto_name->gets());
            // update status in table, status s
            continue;
        }
        handle->adopt(ep, ENET_ENDP_PROTOCOL_HANDLE);
        handle->setflags(EOBJ_PERSISTENT_CALLBACK);

        /* Adopt, successfull created end point.
         */
        ep->adopt(m_end_points, ep_nr);
        changed = OS_TRUE;
    }

    /* Initiate end point information update in UDP multicasts.
     */
    if (changed) {
        setpropertyl(ENETMP_END_POINT_CONFIG_COUNT, ++m_end_point_config_count);
    }

    delete localvars;
    return;

getout_unlock:
    os_unlock();
    delete localvars;
    osal_debug_error("maintain_end_points() failed");
}


void eNetMaintainThread::delete_ep(
    eContainer *ep)
{
    eVariable *proto_name;
    eProtocol *proto;
    eProtocolHandle *handle;

    proto_name = ep->firstv(ENET_ENDP_PROTOCOL);
    proto = protocol_by_name(proto_name);
    if (proto == OS_NULL) return;

    handle = eProtocolHandle::cast(ep->first(ENET_ENDP_PROTOCOL_HANDLE));
    if (proto->is_end_point_running(handle))
    {
        proto->delete_end_pont(handle);
        while (proto->is_end_point_running(handle)) {
            os_timeslice();
        }
    }
    delete ep;
}

/* Update end point status "ok", etc.
 */
void eNetMaintainThread::ep_status_changed(
    eContainer *ep)
{
    eVariable *tmp;

    eProtocolHandle *handle;
    handle = eProtocolHandle::cast(ep->first(ENET_ENDP_PROTOCOL_HANDLE));

    tmp = new eVariable(ETEMPORARY);
    handle->propertyv(EPROHANDP_ISOPEN, tmp);
    set_ep_status(ep->oid(), enet_endp_ok, tmp);
    setpropertyl(ENETMP_END_POINT_CONFIG_COUNT, ++m_end_point_config_count);
    delete tmp;
}

void eNetMaintainThread::set_ep_status(
    os_int row_nr,
    const os_char *column_name,
    eVariable *value)
{
    eVariable *element, *where;
    eContainer *row;

    where = new eVariable(ETEMPORARY);
    row = new eContainer(ETEMPORARY);

    element = new eVariable(row);
    element->addname(column_name, ENAME_NO_MAP);
    element->setv(value);

    where->sets("[");
    where->appendl(row_nr + 1);
    where->appends("]");
    etable_update(this, "//netservice/endpoints", OS_NULL, where->gets(), row,
        ETABLE_ADOPT_ARGUMENT);

    delete where;
}
