/**

  @file    eprotocol_switchbox.cpp
  @brief   SWITCHBOX protocol management.
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
#include "extensions/switchbox/eswitchbox.h"

/**
****************************************************************************************************
  Constructor.
****************************************************************************************************
*/
esboxProtocol::esboxProtocol(
    eObject *parent,
    e_oid oid,
    os_int flags)
    : eProtocol(parent, oid, flags)
{
    m_switchbox_root = OS_NULL;
}


/**
****************************************************************************************************
  Virtual destructor.
****************************************************************************************************
*/
esboxProtocol::~esboxProtocol()
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
void esboxProtocol::setupclass()
{
    const os_int cls = ECLASSID_SWITCHBOX_PROTOCOL;

    /* Add the class to class list.
     */
    os_lock();
    eclasslist_add(cls, (eNewObjFunc)newobj, "esboxProtocol", ECLASSID_PROTOCOL);
    os_unlock();
}


/**
****************************************************************************************************

  @brief Initialize communication protocol (base class implementation).

  The add_protocol() function...

****************************************************************************************************
*/
eStatus esboxProtocol::initialize_protocol(
    class eNetService *netservice,
    void *parameters)
{
    esboxProtocol::setupclass();
    esboxProtocolHandle::setupclass();

    m_switchbox_root = OS_NULL; // netservice->switchbox_root();
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
void esboxProtocol::shutdown_protocol()
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
eProtocolHandle *esboxProtocol::new_end_point(
    os_int ep_nr,
    eEndPointParameters *parameters,
    eStatus *s)
{
    const os_char *prmstr;
    const osalStreamInterface *iface;
    os_short cflags;

    /* Get SWITCHBOX transport interface and flags.
     */
    switch (parameters->transport) {
        case ENET_ENDP_SOCKET: iface = OSAL_SOCKET_IFACE; cflags = IOC_SOCKET; break;
        case ENET_ENDP_TLS:    iface = OSAL_TLS_IFACE;    cflags = IOC_SOCKET; break;
        case ENET_ENDP_SERIAL: iface = OSAL_SERIAL_IFACE; cflags = IOC_SERIAL; break;
        default:
            *s = ESTATUS_FAILED;
            osal_debug_error_int("Unknown transport for switchbox end point: ", parameters->transport);
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
eProtocolHandle *esboxProtocol::new_connection(
    eVariable *con_name,
    eConnectParameters *parameters,
    eStatus *s)
{
    const os_char *prmstr;
    const osalStreamInterface *iface;
    os_short cflags;

    /* Get SWITCHBOX transport interface and flags.
     */
    switch (parameters->transport) {
        case ENET_CONN_SOCKET: iface = OSAL_SOCKET_IFACE; cflags = IOC_SOCKET; break;
        case ENET_CONN_TLS:    iface = OSAL_TLS_IFACE;    cflags = IOC_SOCKET; break;
        case ENET_CONN_SERIAL: iface = OSAL_SERIAL_IFACE; cflags = IOC_SERIAL; break;
        default:
            *s = ESTATUS_FAILED;
            osal_debug_error_int("Unknown transport for switchbox connection: ", parameters->transport);
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
eProtocolHandle *esboxProtocol::new_con_helper(
    const os_char *prmstr,
    const osalStreamInterface *iface,
    os_short cflags,
    eStatus *s)
{
    iocEndPoint *ep;
    iocEndPointParams epprm;
    iocConnection *con;
    iocConnectionParams conprm;
    esboxProtocolHandle *p;
    osalStatus ss;

    *s = ESTATUS_SUCCESS;
    p = new esboxProtocolHandle(ETEMPORARY);

    if ((cflags & (IOC_SOCKET|IOC_LISTENER)) == (IOC_SOCKET|IOC_LISTENER))
    {
        ep = p->epoint();
        p->mark_switchbox_end_point(OS_TRUE);
        ioc_initialize_end_point(ep, m_switchbox_root);
        ioc_set_end_point_callback(ep, end_point_callback, p);
        os_memclear(&epprm, sizeof(epprm));
        epprm.iface = iface;
        epprm.flags = cflags;
        epprm.parameters = prmstr;
        ss = ioc_listen(ep, &epprm);
        if (ss) *s = ESTATUS_FROM_OSAL_STATUS(ss);
    }
    else
    {
        con = p->con();
        p->mark_switchbox_end_point(OS_FALSE);
        ioc_initialize_connection(con, m_switchbox_root);
        ioc_set_connection_callback(con, connection_callback, p);
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

  @brief Callback when connection is established or dropped.

  The SWITCHBOX library calls thus function to inform application about new and dropped connections.
  This function sets esboxProtocolHandle's "isopen" property.

  It uses complex way to set property:  we are now called by different thread which doesn't
  own esboxProtocolHandle and thus must not property directly. But path_to_self is simple C
  string, which is set when esboxProtocol handle is created and unchanged after that. It
  can be used by other threads as long as the protocol handle exists.

  @param   con Pointer to SWITCHBOX connection object.
  @param   event Reason for the callback, either IOC_CONNECTION_ESTABLISHED or
           IOC_CONNECTION_DROPPED.
  @param   context Callback context, here pointer to protocol handle.

****************************************************************************************************
*/
void esboxProtocol::connection_callback(
    struct iocConnection *con,
    iocConnectionEvent event,
    void *context)
{
    eProcess *process;
    os_boolean value;
    esboxProtocolHandle *p;
    OSAL_UNUSED(con);

    p = (esboxProtocolHandle*)context;
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

  @brief Callback when an end point is actually listening, or dropped.

  The SWITCHBOX library calls thus function to inform application about succesfully initialized and
  dropped end points. This function sets esboxProtocolHandle's "isopen" property.
  See esboxProtocol::connection_callback().

  @param   epoint Pointer to SWITCHBOX connection object.
  @param   event Reason for the callback, either IOC_CONNECTION_ESTABLISHED or
           IOC_CONNECTION_DROPPED.
  @param   context Callback context, here pointer to protocol handle.

****************************************************************************************************
*/
void esboxProtocol::end_point_callback(
    struct iocEndPoint *epoint,
    iocEndPointEvent event,
    void *context)
{
    eProcess *process;
    os_boolean value;
    esboxProtocolHandle *p;
    OSAL_UNUSED(epoint);

    p = (esboxProtocolHandle*)context;
    switch (event) {
        case IOC_END_POINT_LISTENING:
            value = OS_TRUE;
            break;

        case IOC_END_POINT_DROPPED:
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
void esboxProtocol::delete_end_point(
    eProtocolHandle *handle)
{
    esboxProtocolHandle *p;
    iocEndPoint *ep;

    if (handle == OS_NULL) return;
    p = (esboxProtocolHandle*)handle;

    if (!p->is_switchbox_end_point()) {
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
void esboxProtocol::delete_connection(
    eProtocolHandle *handle)
{
    esboxProtocolHandle *p;
    iocConnection *con;

    if (handle == OS_NULL) return;
    p = (esboxProtocolHandle*)handle;
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
eStatus esboxProtocol::activate_connection(
    eProtocolHandle *handle,
    eConnectParameters *parameters)
{
    return ESTATUS_SUCCESS;
}


/**
****************************************************************************************************

  Deacivate a connection, not used for switchbox.

****************************************************************************************************
*/
void esboxProtocol::deactivate_connection(
    eProtocolHandle *handle)
{
}


