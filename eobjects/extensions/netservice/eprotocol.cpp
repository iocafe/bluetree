/**

  @file    eprotocol.cpp
  @brief   Abstract communication protocol as seen by eNetService.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  Related to: Network connnection and protocol management interface.

  Abstract communication protocol interface is used by eNetService to manage end points and
  connections. This is the base class, protocol specific derived class will map eNetService
  calls like "create end point" to communication library functions.

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
eProtocol::eProtocol(
    eObject *parent,
    e_oid oid,
    os_int flags)
    : eObject(parent, oid, flags)
{
}


/**
****************************************************************************************************
  Virtual destructor.
****************************************************************************************************
*/
eProtocol::~eProtocol()
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
void eProtocol::setupclass()
{
    const os_int cls = ECLASSID_PROTOCOL;

    /* Add the class to class list.
     */
    os_lock();
    eclasslist_add(cls, (eNewObjFunc)newobj, "eProtocol");
    os_unlock();
}


/**
****************************************************************************************************

  @brief Initialize communication protocol (base class implementation).

  The add_protocol() function...

****************************************************************************************************
*/
eStatus eProtocol::initialize_protocol(
    void *parameters)
{
    return ESTATUS_SUCCESS;
}


/**
****************************************************************************************************

  @brief Finished with communication protocol, clean up (base class implementation).

  The shutdown_protocol() function...
  Called by destructor.

****************************************************************************************************
*/
void eProtocol::shutdown_protocol()
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
eProtocolHandle *eProtocol::new_end_point(
    os_int ep_nr,
    eEndPointParameters *parameters,
    eStatus *s)
{
    OSAL_UNUSED(ep_nr);
    OSAL_UNUSED(parameters);

    *s = ESTATUS_NOT_SUPPORTED;
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
void eProtocol::delete_end_pont(
    eProtocolHandle *handle)
{
    if (handle) {
        handle->terminate_thread();
    }
}


/**
****************************************************************************************************

  @brief Check end point status.

  The is_end_point_running() function checks if a specific end point is running.

  @param   handle   End point handle as returned by new_end_point().
  @return  OS_TRUE if end point is running, OS_FALSE if not.

****************************************************************************************************
*/
os_boolean eProtocol::is_end_point_running(
    eProtocolHandle *handle)
{
    if (handle) {
        return handle->isrunning();
    }
    return OS_FALSE;
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
eProtocolHandle *eProtocol::new_connection(
    eVariable *con_name,
    eConnectParameters *parameters,
    eStatus *s)
{
    OSAL_UNUSED(con_name);
    OSAL_UNUSED(parameters);

    *s = ESTATUS_NOT_SUPPORTED;
    return OS_NULL;
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
void eProtocol::delete_connection(
    eProtocolHandle *handle)
{
    if (handle) {
        handle->terminate_thread();
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
eStatus eProtocol::activate_connection(
    eProtocolHandle *handle,
    eConnectParameters *parameters)
{
    OSAL_UNUSED(handle);
    OSAL_UNUSED(parameters);

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
void eProtocol::deactivate_connection(
    eProtocolHandle *handle)
{
    OSAL_UNUSED(handle);
}


/**
****************************************************************************************************

  @brief Check connection status.

  The is_connection_running() function checks if a specific connection object exists.

  @param   handle   Connection handle as returned by new_connection().
  @return  OS_TRUE if end point is running, OS_FALSE if not.

****************************************************************************************************
*/
os_boolean eProtocol::is_connection_running(
    eProtocolHandle *handle)
{
    if (handle) {
        return handle->isrunning();
    }
    return OS_FALSE;
}

