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
#include "extensions/netservice/enetservice.h"


/* Net service property names.
 */
const os_char
    enetservp_endpoint_change_counter[] = "epchangecnt";


/**
****************************************************************************************************
  Constructor.
****************************************************************************************************
*/
eNetService::eNetService(
    eObject *parent,
    e_oid oid,
    os_int flags)
    : eObject(parent, oid, flags)
{
    m_persistent_accounts = OS_NULL;
    m_account_matrix = OS_NULL;
    m_end_points = OS_NULL;
    m_endpoint_matrix = OS_NULL;
    m_connect = OS_NULL;
    m_connection_matrix = OS_NULL;
    m_services_matrix = OS_NULL;
    m_trusted_matrix = OS_NULL;
    m_persistent_trusted = OS_NULL;
    m_persistent_serv_prm = OS_NULL;
    m_end_points_config_counter = 0;
    os_memclear(&m_serv_prm, sizeof(eNetServPrm));

    initproperties();
}


/**
****************************************************************************************************
  Virtual destructor.
****************************************************************************************************
*/
eNetService::~eNetService()
{
    /* Remove eosal network event handler.
     */
    osal_set_net_event_handler(OS_NULL, this,
        OSAL_ADD_ERROR_HANDLER|OSAL_SYSTEM_ERROR_HANDLER);

#if 0
    /* Finished with lighthouse.
     */
    ioc_release_lighthouse_server(&m_lighthouse_server);

    /* Release any memory allocated for node configuration.
    */
    ioc_release_node_config(&m_nodeconf);
#endif

    ioc_release_root(&m_root);
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
void eNetService::setupclass()
{
    const os_int cls = ECLASSID_NETSERVICE;

    /* Add the class to class list.
     */
    os_lock();
    eclasslist_add(cls, (eNewObjFunc)newobj, "eNetService");
    addpropertyl(cls, ENETSERVP_ENDPOINT_CHANGE_COUNTER, enetservp_endpoint_change_counter,
        0, "end point change counter", EPRO_DEFAULT|EPRO_NOONPRCH);
    propertysetdone(cls);
    os_unlock();
}


/**
****************************************************************************************************

  @brief Create Start network service.

  Called after eNetService object is created to create data structures and start operation.

  @param  flags Bit fields, combination of ENET_DEFAULT_NO_END_POINTS, ENET_ENABLE_IOCOM_CLIENT,
          ENET_ENABLE_EOBJECTS_CLIENT, ENET_ENABLE_IOCOM_SERVICE and ENET_ENABLE_EOBJECTS_SERVICE.

****************************************************************************************************
*/
void eNetService::start(
    os_int flags)
{
    ns_create();

    if (flags & (ENET_ENABLE_IOCOM_SERVICE | ENET_ENABLE_EOBJECTS_SERVICE)) {
        create_user_account_table();
        create_end_point_table(flags);
        create_service_parameters(flags);
    }
    if (flags & (ENET_ENABLE_IOCOM_CLIENT | ENET_ENABLE_EOBJECTS_CLIENT)) {
        create_connect_table();
        create_trusted_certificate_table();
    }
    create_services_table();

    /* Setup eosal network event handler callback to keep track of errors and network state.
     */
    osal_set_net_event_handler(net_event_handler, this,
        OSAL_ADD_ERROR_HANDLER|OSAL_SYSTEM_ERROR_HANDLER);

    /* Initialize communication root object.
     */
    ioc_initialize_root(&m_root);

#if 0
    /* Use devicedir library for development testing, initialize.
     */
    io_initialize_device_console(&m_console, &m_root);

    /* Load device/network configuration and device/user account congiguration
       (persistent storage is typically either file system or micro-controller's flash).
       Defaults are set in network-defaults.json and in account-defaults.json.
     */
    ioc_load_node_config(&m_nodeconf, prm->network_defaults,
        prm->network_defaults_sz, device_name, IOC_LOAD_PBNR_NODE_CONF);
    m_device_id = ioc_get_device_id(&m_nodeconf);
    ioc_set_iodevice_id(&m_root, device_name, m_device_id->device_nr,
        m_device_id->password, m_device_id->network_name);

    ioc_initialize_dynamic_root(&m_root);

    /* Set callback function to receive information about new dynamic memory blocks.
     */
    // ioc_set_root_callback(&iocom_root, app_root_callback, OS_NULL);

    /* Get service TCP port number and transport (IOC_TLS_SOCKET or IOC_TCP_SOCKET).
     */
    m_connconf = ioc_get_connection_conf(&m_nodeconf);
    ioc_get_lighthouse_info(m_connconf, &m_lighthouse_server_info);
#endif

    /* Start the light house service as separate thread. This must be after parmaters
     * haven been created so binding succeed.
     */
    enet_start_lighthouse_thread(this, flags, &m_lighthouse_client_thread_handle);
}


/* Start closing net service (no process lock).
 */
void eNetService::finish()
{
    /* Stop light house client.
     */
    m_lighthouse_client_thread_handle.terminate();
    m_lighthouse_client_thread_handle.join();
}


/**
****************************************************************************************************

  @brief Process a callback from a child object.

  The NetService::oncallback function

****************************************************************************************************
*/
eStatus eNetService::oncallback(
    eCallbackEvent event,
    eObject *obj,
    eObject *appendix)
{
    OSAL_UNUSED(obj);

    if (event == ECALLBACK_PERSISTENT_CHANGED)
    {
        if (obj == m_end_points) {
            setpropertyl(ENETSERVP_ENDPOINT_CHANGE_COUNTER, ++m_end_points_config_counter);
        }
    }

    /* If we need to pass callback to parent class.
     */
    if (flags() & (EOBJ_PERSISTENT_CALLBACK|EOBJ_TEMPORARY_CALLBACK)) {
        eObject::oncallback(event, obj, appendix);
    }

    return ESTATUS_SUCCESS;
}



/* Error handler to move information provided by error handler callbacks to network state structure.
 * This can be called by any thread: The function just converts callbacks to messages.
 */
void eNetService::net_event_handler(
    osalErrorLevel level,
    const os_char *module,
    os_int code,
    const os_char *description,
    void *context)
{

}


/**
****************************************************************************************************

  @brief Start network service.

  Setup network service class and creates global network service object.

  @param  flags Bit fields, combination of ENET_ENABLE_IOCOM_CLIENT, ENET_ENABLE_EOBJECTS_CLIENT,
          ENET_ENABLE_IOCOM_SERVICE and ENET_ENABLE_EOBJECTS_SERVICE.

****************************************************************************************************
*/
void enet_start_service(
    os_int flags)
{
    eNetService::setupclass();

    os_lock();
    eNetService *netservice = new eNetService(eglobal->process);
    netservice->addname("//netservice");
    eglobal->netservice = netservice;
    netservice->start(flags);
    os_unlock();
}


/* Shut down network service.
 * Deletes global network service object.
 */
void enet_stop_service()
{
    if (eglobal->netservice) {
        eglobal->netservice->finish();
        os_lock();
        delete eglobal->netservice;
        eglobal->netservice = OS_NULL;
        os_unlock();
    }
}
