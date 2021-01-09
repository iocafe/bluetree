/**

  @file    eprotocol.cpp
  @brief   Abstract communication protocol as seen by eNetService.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

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

/* File system property names.
 */
const os_char
    eprotocolp_path[] = "path";

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
    initproperties();
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
    addpropertys(cls, EPROTOCOLP_PATH, eprotocolp_path, "/coderoot/fsys", "path", EPRO_DEFAULT);
    propertysetdone(cls);
    os_unlock();
}


/**
****************************************************************************************************

  @brief Called to inform the class about property value change (override).

  The onpropertychange() function is called when class'es property changes, unless the
  property is flagged with EPRO_NOONPRCH.

  @param   propertynr Property number of changed property.
  @param   x Variable containing the new value.
  @param   flags
  @return  If successfull, the function returns ESTATUS_SUCCESS (0). Nonzero return values do
           indicate that there was no property with given property number.

****************************************************************************************************
*/
eStatus eProtocol::onpropertychange(
    os_int propertynr,
    eVariable *x,
    os_int flags)
{
    switch (propertynr)
    {
        case EPROTOCOLP_PATH: /* Read only for sandbox security */
            break;

        default:
            return eObject::onpropertychange(propertynr, x, flags);
    }

    return ESTATUS_SUCCESS;
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
    eProtocol::setupclass();

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

  @brief Create a new end point.

  The new_end_point() function...

****************************************************************************************************
*/
eProtocolHandle *eProtocol::new_end_point(
    os_int ep_nr,
    void *parameters,
    eStatus *s)
{
    OSAL_UNUSED(ep_nr);
    OSAL_UNUSED(parameters);

    *s = ESTATUS_NOT_SUPPORTED;
    return OS_NULL;
}

void eProtocol::delete_end_pont(
    eProtocolHandle *handle)
{
}

eStatus eProtocol::is_end_point_running(
    eProtocolHandle *handle)
{
    return ESTATUS_SUCCESS;
}

/**
****************************************************************************************************

  @brief Create a new connection.

  The new_connection() function...

****************************************************************************************************
*/
eProtocolHandle *eProtocol::new_connection(
    void *parameters,
    os_int conn_nr,
    eStatus *s)
{
    OSAL_UNUSED(conn_nr);
    OSAL_UNUSED(parameters);

    *s = ESTATUS_NOT_SUPPORTED;
    return OS_NULL;
}


void eProtocol::delete_connection(
    eProtocolHandle *handle)
{
}

eStatus eProtocol::is_connection_running(
    eProtocolHandle *handle)
{
    return ESTATUS_SUCCESS;
}

