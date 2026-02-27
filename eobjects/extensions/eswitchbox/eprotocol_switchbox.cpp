/**

  @file    eprotocol_switchbox.cpp
  @brief   SWITCHBOX protocol management.
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
#include "extensions/eswitchbox/eswitchbox.h"

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
    switchboxEndPointParams prm;
    esboxProtocolHandle *p;

    /* Get switchbox TCP port, interface and flags.
     */
    os_memclear(&prm, sizeof(prm));
    prm.flags = IOC_SOCKET|IOC_CREATE_THREAD;
    prm.parameters = parameters->port;

    if (parameters->protocol_flags & EPROTO_PRM_SWITCHBOX_IOCOM_ENDPOINT) {
        prm.default_port = IOC_DEFAULT_IOCOM_SWITCHBOX_TLS_PORT;
    }
    else if (parameters->protocol_flags & EPROTO_PRM_SWITCHBOX_ECOM_ENDPOINT) {
        prm.default_port = IOC_DEFAULT_ECOM_SWITCHBOX_TLS_PORT;
    }
    else {
        prm.default_port = 9128;
        osal_debug_error("default port not selected by protocol_flags, using 9128");
    }

    switch (parameters->transport) {
        /* case ENET_ENDP_SOCKET: prm.iface = OSAL_SOCKET_IFACE; break; */
        case ENET_ENDP_TLS:    prm.iface = OSAL_TLS_IFACE;    break;
        default:
            *s = ESTATUS_NOT_SUPPORTED;
            osal_debug_error_int("Unknown switchbox transport: ", parameters->transport);
            return OS_NULL;
    }

    p = new esboxProtocolHandle(ETEMPORARY);
    *s = p->listen(&prm);
    if (*s) {
        osal_debug_error_str("Failed to create switchbox endpoint: ", parameters->port);
        delete p;
        return OS_NULL;
    }

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
eProtocolHandle *esboxProtocol::new_connection(
    eVariable *con_name,
    eConnectParameters *parameters,
    eStatus *s)
{
    *s = ESTATUS_NOT_SUPPORTED;
    osal_debug_error("Switchbox doesn't support connect, only end points");
    return OS_NULL;
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

    if (handle == OS_NULL) return;
    p = (esboxProtocolHandle*)handle;
    p->close_endpoint();
}


/**
****************************************************************************************************

  Delete a connection, not used for switchbox.

****************************************************************************************************
*/
void esboxProtocol::delete_connection(
    eProtocolHandle *handle)
{
}


/**
****************************************************************************************************

  Reactivate connection or modify parameters, not used for switchbox.

  @return  Always ESTATUS_NOT_SUPPORTED.

****************************************************************************************************
*/
eStatus esboxProtocol::activate_connection(
    eProtocolHandle *handle,
    eConnectParameters *parameters)
{
    return ESTATUS_NOT_SUPPORTED;
}


/**
****************************************************************************************************

  Deactivate a connection, not used for switchbox.

****************************************************************************************************
*/
void esboxProtocol::deactivate_connection(
    eProtocolHandle *handle)
{
}
