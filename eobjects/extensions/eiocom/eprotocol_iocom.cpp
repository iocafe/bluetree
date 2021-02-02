/**

  @file    eprotocol_ecom.cpp
  @brief   The eobjects library communication protocol management, serialized objects.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  Related to: Network connnection and protocol management interface.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "extensions/eiocom/eiocom.h"

/**
****************************************************************************************************
  Constructor.
****************************************************************************************************
*/
eioProtocol::eioProtocol(
    eObject *parent,
    e_oid oid,
    os_int flags)
    : eProtocol(parent, oid, flags)
{
    m_iocom_root = OS_NULL;
}


/**
****************************************************************************************************
  Virtual destructor.
****************************************************************************************************
*/
eioProtocol::~eioProtocol()
{
    shutdown_protocol();
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
void eioProtocol::setupclass()
{
    const os_int cls = ECLASSID_IOCOM_PROTOCOL;

    /* Add the class to class list.
     */
    os_lock();
    eclasslist_add(cls, (eNewObjFunc)newobj, "eioProtocol");
    os_unlock();
}


/**
****************************************************************************************************

  @brief Initialize communication protocol (base class implementation).

  The add_protocol() function...

****************************************************************************************************
*/
eStatus eioProtocol::initialize_protocol(
    class eNetService *netservice,
    void *parameters)
{
    eioProtocol::setupclass();
    eioProtocolHandle::setupclass();

    m_iocom_root = netservice->iocom_root();

    return eProtocol::initialize_protocol(netservice, parameters);
}


/**
****************************************************************************************************

  @brief Finished with communication protocol, clean up (base class implementation).

  The shutdown_protocol() function...
  Called by destructor.

****************************************************************************************************
*/
void eioProtocol::shutdown_protocol()
{
}


/**
****************************************************************************************************

  @brief Create a new end point to listen for this protocol.

  The new_end_point() function creates an end point to listen for specific protocol connections.

  @param   ep_nr End point number. Unique number within process which can be used to create
           end point thread name, etc. In practice this is end point setup table's row number.
           There is no must to use this number, it is just for easy identification purposes.
  @param   parameters Structure containing parameters for the end point.
  @param   s Pointer to eStatus for function return code. If successfull *s is set to
           ESTATUS_SUCCESS. Other values indicate an error.
  @return  Newly allocated protocol handle, used to delete the end point or to check it's status.
           This should be adopted to application data structure. eProtocolHandle handle may
           contain protocol specific content, which should be ignored by calling application.
           If the function fails, the return value is OS_NULL.

****************************************************************************************************
*/
eProtocolHandle *eioProtocol::new_end_point(
    os_int ep_nr,
    eEndPointParameters *parameters,
    eStatus *s)
{
    const os_char *prmstr;
    const osalStreamInterface *iface;
    os_short cflags;

    /* Get IOCOM transport interface and flags.
     */
    switch (parameters->transport) {
        case ENET_ENDP_SOCKET: iface = OSAL_TLS_IFACE;    cflags = IOC_SOCKET; break;
        case ENET_ENDP_TLS:    iface = OSAL_SOCKET_IFACE; cflags = IOC_SOCKET; break;
        case ENET_ENDP_SERIAL: iface = OSAL_SERIAL_IFACE; cflags = IOC_SERIAL; break;
        default:
            *s = ESTATUS_FAILED;
            osal_debug_error_int("Unknown transport for iocom end point: ", parameters->transport);
            return OS_NULL;
    }

    prmstr = parameters->port;
    cflags |= IOC_LISTENER|IOC_DYNAMIC_MBLKS|IOC_CREATE_THREAD;
    return new_con_helper(prmstr, iface, cflags, s);
}


/**
****************************************************************************************************

  @brief Create a new connection using this protocol.

  The new_connection() function creates new connection with this protocol. Notice that this
  function will return quite immediately, and connection object gets created even there
  may not be physical transport or other end is down for the moment.

  @param   con_name Connection name. Identifier created from IP address, port, protocol and
           transport.
  @param   parameters Structure containing parameters for the connection point.
  @param   s Pointer to eStatus for function return code. If successfull *s is set to
           ESTATUS_SUCCESS. Other values indicate an error.
  @return  Newly allocated protocol handle, used to delete the end point or to check it's status.
           This should be adopted to application data structure. eProtocolHandle handle may
           contain protocol specific content, which should be ignored by calling application.
           If the function fails, the return value is OS_NULL.

****************************************************************************************************
*/
eProtocolHandle *eioProtocol::new_connection(
    eVariable *con_name,
    eConnectParameters *parameters,
    eStatus *s)
{
    const os_char *prmstr;
    const osalStreamInterface *iface;
    os_short cflags;

    /* Get IOCOM transport interface and flags.
     */
    switch (parameters->transport) {
        case ENET_CONN_SOCKET: iface = OSAL_TLS_IFACE;    cflags = IOC_SOCKET; break;
        case ENET_CONN_TLS:    iface = OSAL_SOCKET_IFACE; cflags = IOC_SOCKET; break;
        case ENET_CONN_SERIAL: iface = OSAL_SERIAL_IFACE; cflags = IOC_SERIAL; break;
        default:
            *s = ESTATUS_FAILED;
            osal_debug_error_int("Unknown transport for iocom connection: ", parameters->transport);
            return OS_NULL;
    }

    prmstr = parameters->parameters;
    cflags |= IOC_DYNAMIC_MBLKS|IOC_CREATE_THREAD;
    return new_con_helper(prmstr, iface, cflags, s);
}


eProtocolHandle *eioProtocol::new_con_helper(
    const os_char *prmstr,
    const osalStreamInterface *iface,
    os_short cflags,
    eStatus *s)
{
    iocEndPoint *ep = OS_NULL;
    iocEndPointParams epprm;
    iocConnection *con = OS_NULL;
    iocConnectionParams conprm;
    eProtocolHandle *p;
    osalStatus ss;

    *s = ESTATUS_SUCCESS;

    p = new eioProtocolHandle(ETEMPORARY);

    if ((cflags & (IOC_SOCKET|IOC_LISTENER)) == (IOC_SOCKET|IOC_LISTENER))
    {
        ep = ioc_initialize_end_point(OS_NULL, m_iocom_root);
        os_memclear(&epprm, sizeof(epprm));
        epprm.iface = iface;
        epprm.flags = cflags;
        epprm.parameters = prmstr;
        ss = ioc_listen(ep, &epprm);
        if (ss) *s = ESTATUS_FROM_OSAL_STATUS(ss);
    }
    else
    {
        con = ioc_initialize_connection(OS_NULL, m_iocom_root);
        os_memclear(&conprm, sizeof(conprm));
        conprm.iface = iface;
        conprm.flags = cflags;
        conprm.parameters = prmstr;
        ss = ioc_connect(con, &conprm);
        if (ss) *s = ESTATUS_FROM_OSAL_STATUS(ss);
    }

    return p;
}


/**
****************************************************************************************************

  @brief Reactivate a deactivated connection or modify parameters.

  This function is used to pause communication or modify existing connection parameters so that
  connection can be resumed without losing binding state.

  ecom specific: Difference between first deleting a connection and creating new one, compared
  to deactivating/reactivating it is: The connection object is never deleted, and binding
  information stored in connection objects is preserved. If connection comes back existsing
  binding from client to server do restored.

  @param   handle   Connection handle as returned by new_connection().
  @param   parameters Structure containing parameters for the connection point.
  @return  Pointer to eStatus for function return code. If successfull, the function returns
           ESTATUS_SUCCESS. Other values indicate an error.

****************************************************************************************************
*/
eStatus eioProtocol::activate_connection(
    eProtocolHandle *handle,
    eConnectParameters *parameters)
{
    /* eVariable tmp;
    const os_char *un; */

    if (handle == OS_NULL || parameters == OS_NULL) {
        return ESTATUS_FAILED;
    }

    /* make_connect_parameter_string(&tmp, parameters);
    un = handle->uniquename();
    setpropertys_msg(un, tmp.gets(), econnp_ipaddr);
    setpropertyl_msg(un, OS_TRUE, econnp_enable); */

    return ESTATUS_SUCCESS;
}


/**
****************************************************************************************************

  @brief Deacivate a connection.

  The deactivate_connection() function disables a connection object so that it is inavtive
  and does not run actual communication. But it doesn't change eConnection parameters
  or stored client binding data.

  @param   handle   Connection handle as returned by new_connection().

****************************************************************************************************
*/
void eioProtocol::deactivate_connection(
    eProtocolHandle *handle)
{
    /* const os_char *un;

    if (handle == OS_NULL) {
        return;
    } */

    /* un = handle->uniquename();
    setpropertyl_msg(un, OS_FALSE, econnp_enable); */
}


