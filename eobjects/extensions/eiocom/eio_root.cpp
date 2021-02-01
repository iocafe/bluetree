/**

  @file    enetservice.cpp
  @brief   enet service implementation.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "extensions/eiocom/eiocom.h"

#if 0


/**
****************************************************************************************************
  Constructor.
****************************************************************************************************
*/
eioRoot::eioRoot(
    eObject *parent,
    e_oid oid,
    os_int flags)
    : eObject(parent, oid, flags)
{
    m_services_matrix = OS_NULL;
    m_lighthouse_change_counter = 0;

    m_iocom_root = OS_NULL;
    m_netservice = OS_NULL;

//    initproperties();
}


/**
****************************************************************************************************
  Virtual destructor.
****************************************************************************************************
*/
eioRoot::~eioRoot()
{
    /* Remove eosal network event handler.
     */
//    osal_set_net_event_handler(OS_NULL, this,
 //       OSAL_ADD_ERROR_HANDLER|OSAL_SYSTEM_ERROR_HANDLER);

#if 0
    /* Release any memory allocated for node configuration.
    */
    ioc_release_node_config(&m_nodeconf);
#endif

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
void eioRoot::setupclass()
{
    const os_int cls = ECLASSID_NETSERVICE;

    /* Add the class to class list.
     */
    os_lock();
    propertysetdone(cls);
    os_unlock();
}


/**
****************************************************************************************************

  @brief Create Start network service.

  Called after eioRoot object is created to create data structures and start operation.

  @param  flags Bit fields, combination of ENET_DEFAULT_NO_END_POINTS, ENET_ENABLE_IOCOM_CLIENT,
          ENET_ENABLE_EOBJECTS_CLIENT, ENET_ENABLE_IOCOM_SERVICE and ENET_ENABLE_EOBJECTS_SERVICE.

****************************************************************************************************
*/
void eioRoot::start(
    os_int flags)
{
    ns_create();

}


/* Start closing net service (no process lock).
 */
void eioRoot::finish()
{
}


/**
****************************************************************************************************

  @brief Process a callback from a child object.

  The NetService::oncallback function

****************************************************************************************************
*/
eStatus eioRoot::oncallback(
    eCallbackEvent event,
    eObject *obj,
    eObject *appendix)
{
#if 0
    OSAL_UNUSED(obj);

    if (event == ECALLBACK_PERSISTENT_CHANGED)
    {
        if (obj == m_end_points) {
            setpropertyl(EIO_ROOTP_ENDPOINT_CONFIG_CHANGE_COUNTER, ++m_end_points_config_counter);
        }

        if (obj == m_connect) {
            setpropertyl(EIO_ROOTP_CONNECT_CONFIG_CHANGE_COUNTER, ++m_connect_config_counter);
        }
    }

    if (event == ECALLBACK_TABLE_CONTENT_CHANGED)
    {
        if (obj == m_services_matrix) {
            setpropertyl(EIO_ROOTP_LIGHTHOUSE_CHANGE_COUNTER, ++m_lighthouse_change_counter);
        }
    }

    /* If we need to pass callback to parent class.
     */
    if (flags() & (EOBJ_PERSISTENT_CALLBACK|EOBJ_TEMPORARY_CALLBACK)) {
        eObject::oncallback(event, obj, appendix);
    }
#endif
    return ESTATUS_SUCCESS;
}


void eioRoot::set_netservice(
    struct eNetService *netservice)
{
    m_netservice = netservice;
    m_iocom_root = &netservice->m_iocom_root;
    m_netservice->m_eio = this;
}



/* Error handler to move information provided by error handler callbacks to network state structure.
 * This can be called by any thread: The function just converts callbacks to messages.
 */
void eioRoot::net_event_handler(
    osalErrorLevel level,
    const os_char *module,
    os_int code,
    const os_char *description,
    void *context)
{


}

#endif
