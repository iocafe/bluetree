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
#include "eobjects.h"
#include "extensions/netservice/enetservice.h"


/**
****************************************************************************************************
  Constructor.
****************************************************************************************************
*/
eNetService::eNetService(
    eObject *parent,
    e_oid oid,
    os_int flags)
    : eThread(parent, oid, flags)
{
    m_persistent_accounts = OS_NULL;
    m_account_matrix = OS_NULL;
    initproperties();
}


/**
****************************************************************************************************
  Virtual destructor.
****************************************************************************************************
*/
eNetService::~eNetService()
{
}


/**
****************************************************************************************************

  @brief Clone object

  The eNetService::clone function clones and object including object's children.
  Names will be left detached in clone.

  @param  parent Parent for the clone.
  @param  oid Object identifier for the clone.
  @param  aflags 0 for default operation. EOBJ_NO_MAP not to map names.
  @return Pointer to the clone.

****************************************************************************************************
*/
eObject *eNetService::clone(
    eObject *parent,
    e_oid id,
    os_int aflags)
{
    eObject *clonedobj;
    clonedobj = new eNetService(parent, id == EOID_CHILD ? oid() : id, flags());
    clonegeneric(clonedobj, aflags|EOBJ_CLONE_ALL_CHILDREN);
    return clonedobj;
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
    os_unlock();
}

/* Overloaded eThread function to initialize new thread. Called after eNetService object is created.
 */
void eNetService::initialize(
    eContainer *params)
{

    ns_create();

    create_user_account_table();
    create_end_point_table();
    create_connection_table();

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

    /* Setup IO pins.
     */
    m_pins_header = prm->pins_header;
    pins_setup(m_pins_header, PINS_DEFAULT);

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
}

/* Overloaded eThread function to perform thread specific cleanup when threa exists.
   This is a "pair" to initialize function.
 */
void eNetService::finish()
{
    /* Remove eosal network event handler.
     */
    osal_set_net_event_handler(OS_NULL, this,
        OSAL_ADD_ERROR_HANDLER|OSAL_SYSTEM_ERROR_HANDLER);

#if 0
    /* Finished with lighthouse.
     */
    ioc_release_lighthouse_server(&m_lighthouse_server);

    /* Stop SPI and I2C threads.
     */
#if OSAL_MULTITHREAD_SUPPORT && (PINS_SPI || PINS_I2C)
    pins_stop_multithread_devicebus();
#endif

    pins_shutdown(m_pins_header);

    /* Release any memory allocated for node configuration.
    */
    ioc_release_node_config(&m_nodeconf);
#endif

    ioc_release_root(&m_root);
    eThread::finish();
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


/* Start network service.
 */
void enet_start_service(
    eThreadHandle *server_thread_handle)
{
    eNetService *net_service;

    /* Set up class for use.
     */
    eNetService::setupclass();

    /* Create and start net service thread to listen for incoming socket connections,
       name it "//netservice".
     */
    net_service = new eNetService();
    net_service->addname("//netservice");
    net_service->start(server_thread_handle);
}
