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
eComProtocol::eComProtocol(
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
eComProtocol::~eComProtocol()
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
void eComProtocol::setupclass()
{
    const os_int cls = ECLASSID_ECOM_PROTOCOL;

    /* Add the class to class list.
     */
    os_lock();
    eclasslist_add(cls, (eNewObjFunc)newobj, "eComProtocol");
    os_unlock();
}


/**
****************************************************************************************************

  @brief Initialize communication protocol (base class implementation).

  The add_protocol() function...

****************************************************************************************************
*/
eStatus eComProtocol::initialize_protocol(
    void *parameters)
{
    eComProtocol::setupclass();

    return ESTATUS_SUCCESS;
}


/**
****************************************************************************************************

  @brief Finished with communication protocol, clean up (base class implementation).

  The shutdown_protocol() function...
  Called by destructor.

****************************************************************************************************
*/
void eComProtocol::shutdown_protocol()
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
eProtocolHandle *eComProtocol::new_end_point(
    os_int ep_nr,
    void *parameters,
    eStatus *s)
{
    eProtocolHandle *p;
    eThread *t;

    OSAL_UNUSED(ep_nr);
    OSAL_UNUSED(parameters);

    /* Create and start end point thread to listen for incoming socket connections,
       name it "myendpoint".
     */
    t = new eEndPoint();
    p = new eProtocolHandle(ETEMPORARY);
    p->start_thread(t, "myendpoint");

    setpropertys_msg(p->uniquename(),
         "socket::" IOC_DEFAULT_SOCKET_PORT_STR, eendpp_ipaddr);

    *s = ESTATUS_SUCCESS;
    return p;
}


/**
****************************************************************************************************

  @brief Create a new connection using this protocol.

  The new_connection() function creates new connection with this protocol. Notice that this
  function will return quite immediately, and connection object gets created even there
  may not be physical transport or other end is down for the moment.

  @param   conn_nr Connection number. Unique number within process which can be used to create
           connection thread name, etc. In practice this is connection table's row number.
           There is no must to use this number, it is just for easy identification purposes.
  @param   parameters Structure containing parameters for the connection point.
  @param   s Pointer to eStatus for function return code. If successfull *s is set to
           ESTATUS_SUCCESS. Other values indicate an error.
  @return  Newly allocated protocol handle, used to delete the end point or to check it's status.
           This should be adopted to application data structure. eProtocolHandle handle may
           contain protocol specific content, which should be ignored by calling application.
           If the function fails, the return value is OS_NULL.

****************************************************************************************************
*/
eProtocolHandle *eComProtocol::new_connection(
    os_int conn_nr,
    void *parameters,
    eStatus *s)
{
    eProtocolHandle *p;
    eThread *t;

    OSAL_UNUSED(conn_nr);
    OSAL_UNUSED(parameters);

    /* Create and start end point thread to listen for incoming socket connections,
       name it "myendpoint".
     */
    t = new eConnection();
    p = new eProtocolHandle(ETEMPORARY);
    p->start_thread(t, "myconnection");

    setpropertys_msg(p->uniquename(),
         "socket:localhost", econnp_ipaddr);

    *s = ESTATUS_SUCCESS;
    return p;
}


