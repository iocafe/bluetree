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
#include "extensions/netservice/enetservice.h"

/**
****************************************************************************************************
  Constructor.
****************************************************************************************************
*/
ecomProtocol::ecomProtocol(
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
ecomProtocol::~ecomProtocol()
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
void ecomProtocol::setupclass()
{
    const os_int cls = ECLASSID_ECOM_PROTOCOL;

    /* Add the class to class list.
     */
    os_lock();
    eclasslist_add(cls, (eNewObjFunc)newobj, "ecomProtocol");
    os_unlock();
}


/**
****************************************************************************************************

  @brief Initialize communication protocol (base class implementation).

  The add_protocol() function...

****************************************************************************************************
*/
eStatus ecomProtocol::initialize_protocol(
    class eNetService *netservice,
    void *parameters)
{
    ecomProtocol::setupclass();
    ecomProtocolHandle::setupclass();

    return eProtocol::initialize_protocol(netservice, parameters);
}


/**
****************************************************************************************************

  @brief Finished with communication protocol, clean up (base class implementation).

  The shutdown_protocol() function...
  Called by destructor.

****************************************************************************************************
*/
void ecomProtocol::shutdown_protocol()
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
eProtocolHandle *ecomProtocol::new_end_point(
    os_int ep_nr,
    eEndPointParameters *parameters,
    eStatus *s)
{
    eProtocolHandle *p;
    eThread *t;
    eVariable tmp;
    const os_char *transport_name, *un;

    /* Name of the transport.
     */
    switch (parameters->transport) {
        case ENET_ENDP_SOCKET:
            transport_name = "socket";
            break;

        case ENET_ENDP_TLS:
            transport_name = "tls";
            break;

        case ENET_ENDP_SERIAL:
            transport_name = "serial";
            break;

        default:
            *s = ESTATUS_FAILED;
            osal_debug_error_int("Unknown end point transport: ", parameters->transport);
            return OS_NULL;
    }

    /* Create and start end point thread to listen for incoming socket connections,
       name it "myendpoint".
     */
    t = new eEndPoint();
    p = new ecomProtocolHandle(ETEMPORARY);
    tmp.sets("ecom_ep");
    tmp.appendl(ep_nr + 1);
    tmp.appends("_");
    tmp.appends(transport_name);
    p->start_thread(t, tmp.gets());

    /* Bind property handles "is open" property to end point's same property.
     */
    un = p->uniquename();
    p->bind(EPROHANDP_ISOPEN, un, eendpp_isopen, EBIND_TEMPORARY);

    /* Set end point parameters as string (transport, IP address, TCP port, etc).
     */
    tmp.sets(transport_name);
    tmp.appends(":");
    tmp.appends(parameters->port);
    setpropertys_msg(un, tmp.gets(), eendpp_ipaddr);

    *s = ESTATUS_SUCCESS;
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
eProtocolHandle *ecomProtocol::new_connection(
    eVariable *con_name,
    eConnectParameters *parameters,
    eStatus *s)
{
    eProtocolHandle *p;
    eThread *t;
    eVariable tmp;
    const os_char *un;

    /* Create and start end point thread to listen for incoming socket connections,
       name it "myendpoint".
     */
    t = new eConnection();
    p = new ecomProtocolHandle(ETEMPORARY);
    p->start_thread(t, con_name->gets());

    /* Bind property handles "is open" property to connection's same property.
     */
    un = p->uniquename();
    p->bind(EPROHANDP_ISOPEN, un, econnp_isopen, EBIND_TEMPORARY);

    /* Set connect parameters as string (transport, IP address, TCP port, etc).
     */
    make_connect_parameter_string(&tmp, parameters);
    setpropertys_msg(un, tmp.gets(), econnp_ipaddr);

    *s = ESTATUS_SUCCESS;
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
eStatus ecomProtocol::activate_connection(
    eProtocolHandle *handle,
    eConnectParameters *parameters)
{
    eVariable tmp;
    const os_char *un;

    if (handle == OS_NULL || parameters == OS_NULL) {
        return ESTATUS_FAILED;
    }

    make_connect_parameter_string(&tmp, parameters);
    un = handle->uniquename();
    setpropertys_msg(un, tmp.gets(), econnp_ipaddr);
    setpropertyl_msg(un, OS_TRUE, econnp_enable);

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
void ecomProtocol::deactivate_connection(
    eProtocolHandle *handle)
{
    const os_char *un;

    if (handle == OS_NULL) {
        return;
    }

    un = handle->uniquename();
    setpropertyl_msg(un, OS_FALSE, econnp_enable);
}


/**
****************************************************************************************************

  @brief Generate string containing transport, IP address and port.

  The make_connect_parameter_string() function....

****************************************************************************************************
*/
void ecomProtocol::make_connect_parameter_string(
    eVariable *parameter_str,
    eConnectParameters *parameters)
{
    const os_char *transport_name;

    /* Name of the transport.
     */
    switch (parameters->transport) {
        case ENET_CONN_SOCKET:
            transport_name = "socket";
            break;

        case ENET_CONN_TLS:
            transport_name = "tls";
            break;

        case ENET_CONN_SERIAL:
            transport_name = "serial";
            break;

        default:
            transport_name = "unknown";
            osal_debug_error_int("Unknown connection transport: ", parameters->transport);
            break;
    }

    /* Set end point parameters as string (transport, IP address, TCP port, etc).
     */
    parameter_str->sets(transport_name);
    parameter_str->appends(":");
    parameter_str->appends(parameters->parameters);
}
