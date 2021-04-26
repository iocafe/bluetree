/**

  @file    eprotocol_iocom.cpp
  @brief   IOCOM protocol management.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

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
    addname(protocol_name2());
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
    eioProtocolHandle *p;
    os_short cflags;
    os_char hostbuf[OSAL_HOST_BUF_SZ];
    OSAL_UNUSED(ep_nr);

    /* Get IOCOM transport interface and flags.
     */
    prmstr = parameters->port;
    if (parameters->protocol_flags & EPROTO_PRM_CONNECT_IOCOM_TO_SWITCHBOX)
    {
        iface = IOC_SWITCHBOX_SOCKET_IFACE;
        cflags = IOC_SOCKET;

        osal_socket_embed_default_port(parameters->port,
            hostbuf, sizeof(hostbuf), IOC_DEFAULT_IOCOM_SWITCHBOX_TLS_PORT);
        prmstr = hostbuf;

    }
    else switch (parameters->transport)
    {
        case ENET_ENDP_SOCKET:
            iface = OSAL_SOCKET_IFACE;
            cflags = IOC_SOCKET;
            break;

        case ENET_ENDP_TLS:
            iface = OSAL_TLS_IFACE;
            cflags = IOC_SOCKET;
            break;

        case ENET_ENDP_SERIAL:
            iface = OSAL_SERIAL_IFACE;
            cflags = IOC_SERIAL;
            break;

        default:
            *s = ESTATUS_FAILED;
            osal_debug_error_int("Unknown transport for iocom end point: ", parameters->transport);
            return OS_NULL;
    }

    cflags |= IOC_LISTENER|IOC_DYNAMIC_MBLKS|IOC_CREATE_THREAD;
    p = new eioProtocolHandle(ETEMPORARY);
    *s = new_con_helper(p, OS_NULL, prmstr, iface, cflags);
    return p;
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
    const osalStreamInterface *iface;
    eioProtocolHandle *p;
    os_short cflags;
    OSAL_UNUSED(con_name);

    /* Get IOCOM transport interface and flags.
     */
    switch (parameters->transport) {
        case ENET_CONN_SOCKET:
            iface = OSAL_SOCKET_IFACE;
            cflags = IOC_SOCKET;
            break;

        case ENET_CONN_TLS:
            iface = OSAL_TLS_IFACE;
            cflags = IOC_SOCKET;
            break;

        case ENET_CONN_SERIAL:
            iface = OSAL_SERIAL_IFACE;
            cflags = IOC_SERIAL;
            break;

        default:
            *s = ESTATUS_FAILED;
            osal_debug_error_int("Unknown transport for iocom connection: ", parameters->transport);
            return OS_NULL;
    }

    cflags |= IOC_DYNAMIC_MBLKS|IOC_CREATE_THREAD;
    p = new eioProtocolHandle(ETEMPORARY);
    *s = new_con_helper(p, parameters->name, parameters->parameters, iface, cflags);
    return p;
}


/**
****************************************************************************************************

  @brief Helper function for new_connection() and new_end_point().

  The new_con_helper() function...

****************************************************************************************************
*/
eStatus eioProtocol::new_con_helper(
    eioProtocolHandle *p,
    const os_char *name,
    const os_char *prmstr,
    const osalStreamInterface *iface,
    os_short cflags)
{
    iocEndPoint *ep;
    iocEndPointParams epprm;
    iocConnection *con;
    iocConnectionParams conprm;
    osalStatus ss;
    eStatus s;

    s = ESTATUS_SUCCESS;

    if ((cflags & (IOC_SOCKET|IOC_LISTENER)) == (IOC_SOCKET|IOC_LISTENER))
    {
        ep = p->epoint();
        p->mark_iocom_handle_type(EIO_HANDLE_END_POINT);
        ioc_initialize_end_point(ep, m_iocom_root);
        ioc_set_end_point_callback(ep, end_point_callback, p);
        os_memclear(&epprm, sizeof(epprm));
        epprm.iface = iface;
        epprm.flags = cflags;
        epprm.parameters = prmstr;
        ss = ioc_listen(ep, &epprm);
        if (ss) s = ESTATUS_FROM_OSAL_STATUS(ss);
    }
    else
    {
        eVariable user_override;
        os_char password[OSAL_SECRET_STR_SZ];

        con = p->con();
        p->mark_iocom_handle_type(EIO_HANDLE_CONNECTION);
        ioc_initialize_connection(con, m_iocom_root);
        ioc_set_connection_callback(con, connection_callback, p);
        os_memclear(&conprm, sizeof(conprm));
        conprm.iface = iface;
        conprm.flags = cflags;
        conprm.parameters = prmstr;

        /* If we have GUI user login, use it for iocom connections.
         */
        if (eglobal->active_login.user_name[0]) {
            user_override = eglobal->active_login.user_name;
            conprm.password_override = eglobal->active_login.password;
            if (!os_strcmp(conprm.password_override, "*") ||
                !os_strcmp(conprm.password_override, ""))
            {
                osal_get_auto_password(password, sizeof(password));
                conprm.password_override = password;
            }
        }

        /* Otherwise use process ID and auto password for connections.
         */
        else {
            user_override = eglobal->process_id;
            osal_get_auto_password(password, sizeof(password));
            conprm.password_override = password;
        }

        /* If we have network name, and user name override doesn't contain network
           name, append network name.
         */
        if (name) if (os_strcmp(name, "*")) if (os_strchr(user_override.gets(), '.') == OS_NULL)
        {
            user_override += ".";
            user_override += name;
        }
        conprm.user_override = user_override.gets();

        ss = ioc_connect(con, &conprm);
        if (ss) s = ESTATUS_FROM_OSAL_STATUS(ss);
    }

    return s;
}


/**
****************************************************************************************************

  @brief Callback when connection is established or dropped.

  The IOCOM library calls thus function to inform application about new and dropped connections.
  This function sets eioProtocolHandle's "isopen" property.

  It uses complex way to set property:  we are now called by different thread which doesn't
  own eioProtocolHandle and thus must not property directly. But path_to_self is simple C
  string, which is set when eioProtocol handle is created and unchanged after that. It
  can be used by other threads as long as the protocol handle exists.

  @param   con Pointer to IOCOM connection object.
  @param   event Reason for the callback, either IOC_CONNECTION_ESTABLISHED or
           IOC_CONNECTION_DROPPED.
  @param   context Callback context, here pointer to protocol handle.

****************************************************************************************************
*/
void eioProtocol::connection_callback(
    struct iocConnection *con,
    iocConnectionEvent event,
    void *context)
{
    eProcess *process;
    os_boolean value;
    eioProtocolHandle *p;
    OSAL_UNUSED(con);

    p = (eioProtocolHandle*)context;
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

  The IOCOM library calls thus function to inform application about succesfully initialized and
  dropped end points. This function sets eioProtocolHandle's "isopen" property.
  See eioProtocol::connection_callback().

  @param   epoint Pointer to IOCOM connection object.
  @param   event Reason for the callback, either IOC_CONNECTION_ESTABLISHED or
           IOC_CONNECTION_DROPPED.
  @param   context Callback context, here pointer to protocol handle.

****************************************************************************************************
*/
void eioProtocol::end_point_callback(
    struct iocEndPoint *epoint,
    iocEndPointEvent event,
    void *context)
{
    eProcess *process;
    os_boolean value;
    eioProtocolHandle *p;
    OSAL_UNUSED(epoint);

    p = (eioProtocolHandle*)context;
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
void eioProtocol::delete_end_point(
    eProtocolHandle *handle)
{
    eioProtocolHandle *p;
    iocEndPoint *ep;

    if (handle == OS_NULL) return;
    p = (eioProtocolHandle*)handle;

    switch (p->handle_type())
    {
        case EIO_HANDLE_CONNECTION:
            delete_connection(handle);
            break;

        case EIO_HANDLE_END_POINT:
            ep = p->epoint();

            while (ioc_terminate_end_point_thread(ep) == OSAL_PENDING) {
                os_timeslice();
            }

            ioc_release_end_point(p->epoint());
            p->setpropertyi(EPROHANDP_ISOPEN, OS_FALSE);
            p->mark_iocom_handle_type(EIO_HANDLE_UNUSED);
            break;

        default:
            break;
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
    if (p->handle_type() == EIO_HANDLE_CONNECTION)
    {
        con = p->con();

        while (ioc_terminate_connection_thread(con) == OSAL_PENDING) {
            os_timeslice();
        }
        ioc_release_connection(p->con());
        p->setpropertyi(EPROHANDP_ISOPEN, OS_FALSE);
        p->mark_iocom_handle_type(EIO_HANDLE_UNUSED);
    }
}


/**
****************************************************************************************************

  @brief Reactivate a deactivated connection or modify parameters.

  This function is used to pause communication or modify existing connection parameters so that
  connection can be resumed without losing binding state.

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
    // eioProtocol::delete_connection(handle);
}


