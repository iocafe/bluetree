/**

  @file    eprotocol_iocom.cpp
  @brief   IOCOM  protocol management.
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
#include "extensions/iocom/eiocom.h"

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
    eclasslist_add(cls, (eNewObjFunc)newobj, "eioProtocol", ECLASSID_PROTOCOL);
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
    addname(protocol_name());
    return ESTATUS_SUCCESS;
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
        case ENET_ENDP_SOCKET: iface = OSAL_SOCKET_IFACE; cflags = IOC_SOCKET; break;
        case ENET_ENDP_TLS:    iface = OSAL_TLS_IFACE;    cflags = IOC_SOCKET; break;
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
        case ENET_CONN_SOCKET: iface = OSAL_SOCKET_IFACE; cflags = IOC_SOCKET; break;
        case ENET_CONN_TLS:    iface = OSAL_TLS_IFACE;    cflags = IOC_SOCKET; break;
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


/**
****************************************************************************************************

  @brief Helper function for new_connection() and new_end_point().

  The new_con_helper() function...

****************************************************************************************************
*/
eProtocolHandle *eioProtocol::new_con_helper(
    const os_char *prmstr,
    const osalStreamInterface *iface,
    os_short cflags,
    eStatus *s)
{
    iocEndPoint *ep;
    iocEndPointParams epprm;
    iocConnection *con;
    iocConnectionParams conprm;
    eioProtocolHandle *p;
    osalStatus ss;

    *s = ESTATUS_SUCCESS;
    p = new eioProtocolHandle(ETEMPORARY);

    if ((cflags & (IOC_SOCKET|IOC_LISTENER)) == (IOC_SOCKET|IOC_LISTENER))
    {
        ep = p->epoint();
        p->mark_iocom_end_point(OS_TRUE);
        ioc_initialize_end_point(ep, m_iocom_root);
        os_memclear(&epprm, sizeof(epprm));
        epprm.iface = iface;
        epprm.flags = cflags;
        epprm.parameters = prmstr;
        ss = ioc_listen(ep, &epprm);
        if (ss) *s = ESTATUS_FROM_OSAL_STATUS(ss);

p->setpropertyi(EPROHANDP_ISOPEN, OS_TRUE);
    }
    else
    {
        con = p->con();
        p->mark_iocom_end_point(OS_FALSE);
        ioc_initialize_connection(con, m_iocom_root);
        ioc_set_connection_callback(con, connection_callback, p);
        os_memclear(&conprm, sizeof(conprm));
        conprm.iface = iface;
        conprm.flags = cflags;
        conprm.parameters = prmstr;
        ss = ioc_connect(con, &conprm);
        if (ss) *s = ESTATUS_FROM_OSAL_STATUS(ss);
    }

//    p->setpropertyi(EPROHANDP_ISOPEN, OS_TRUE);
    return p;
}

/* Callback when connection is established or dropped.
 */
void eioProtocol::connection_callback(
    struct iocConnection *conf,
    iocConnectionEvent event,
    void *context)
{
    eProcess *process;
    os_boolean value;
    eioProtocolHandle *p;
    p = (eioProtocolHandle*)context;

    /* Complex way to set property, we are now called by different thread which doesn't
       own eioProtocolHandle and thus must not property directly.
     */
    switch (event) {
        case IOC_CONNECTION_ESTABLISHED:
            value = OS_TRUE;
            break;

        case IOC_CONNECTION_DROPPED:
            value = OS_FALSE;
            break;

        default:
            return;
    }

    os_lock();
    process = eglobal->process;
    process->setpropertyl_msg(p->path_to_self(), value, eprohandp_isopen);
    os_unlock();
}


/**
****************************************************************************************************

  @brief Delete an end point.

  The delete_end_point() function deletes an end point created by new_end_point() call. This
  function releases all resources associated with the end point. Notice that closing listening
  socket may linger a while in underlyin OS.

  @param   handle   End point handle as returned by new_end_point().

****************************************************************************************************
*/
void eioProtocol::delete_end_point(
    eProtocolHandle *handle)
{
    eioProtocolHandle *p;
    iocEndPoint *ep;

    if (handle == OS_NULL) return;
    p = (eioProtocolHandle*)handle;

    if (!p->is_iocom_end_point()) {
        delete_connection(handle);
        return;
    }

    if (p->isrunning()) {
        ep = p->epoint();

        while (ioc_terminate_end_point_thread(ep)) {
            os_timeslice();
        }

        ioc_release_end_point(p->epoint());
        p->setpropertyi(EPROHANDP_ISOPEN, OS_FALSE);
    }
}


/**
****************************************************************************************************

  @brief Delete a connection.

  The delete_connection() function deletes a connection created by new_connection() call. This
  function releases all resources associated with the end point. Notice that closing listening
  socket may linger a while in underlyin OS.

  @param   handle   Connection handle as returned by new_connection().

****************************************************************************************************
*/
void eioProtocol::delete_connection(
    eProtocolHandle *handle)
{
    eioProtocolHandle *p;
    iocConnection *con;

    if (handle == OS_NULL) return;
    p = (eioProtocolHandle*)handle;
    if (p->isrunning()) {
        con = p->con();

        while (ioc_terminate_connection_thread(con)) {
            os_timeslice();
        }
        ioc_release_connection(p->con());
        p->setpropertyi(EPROHANDP_ISOPEN, OS_FALSE);
    }
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
    return ESTATUS_SUCCESS;
}


/**
****************************************************************************************************

  @brief Deacivate a connection.

  The deactivate_connection() function disables a connection object so that it is inactive
  and does not run actual communication. But it doesn't change eConnection parameters
  or stored client binding data.

  @param   handle   Connection handle as returned by new_connection().

****************************************************************************************************
*/
void eioProtocol::deactivate_connection(
    eProtocolHandle *handle)
{
    /* eioProtocolHandle *p;

    if (handle == OS_NULL) return;
    p = (eioProtocolHandle*)handle; */
}


