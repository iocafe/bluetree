/**

  @file    enetservice.cpp
  @brief   enet service implementation.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

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
    enetservp_endpoint_table_change_counter[] = "eptabccnt",
    enetservp_endpoint_config_counter[] = "erecongcnt",
    enetservp_connect_table_change_counter[] = "contabccnt",
    enetservp_lighthouse_change_counter[] = "lhcnt";


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
    m_connect_to_matrix = OS_NULL;
    m_services_matrix = OS_NULL;
    m_trusted_matrix = OS_NULL;
    m_persistent_trusted = OS_NULL;
    m_persistent_parameters = OS_NULL;
    m_end_points_config_counter = 0;
    m_connect_config_counter = 0;
    m_lighthouse_change_counter = 0;
    os_memclear(&m_parameters, sizeof(m_parameters));
    os_memclear(&m_iocom_root, sizeof(m_iocom_root));

    addname("//netservice");
    ns_create();

    m_protocols = new eContainer(this, EOID_ITEM, EOBJ_IS_ATTACHMENT);
    m_protocols->addname("protocols");
    m_protocols->ns_create();

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
    addpropertyl(cls, ENETSERVP_ENDPOINT_CONFIG_CHANGE_COUNTER, enetservp_endpoint_table_change_counter,
        0, "end point table change counter", EPRO_DEFAULT|EPRO_NOONPRCH);
    addpropertyl(cls, ENETSERVP_ENDPOINT_CONFIG_COUNTER, enetservp_endpoint_config_counter,
        0, "end point config counter", EPRO_DEFAULT|EPRO_NOONPRCH);
    addpropertyl(cls, ENETSERVP_CONNECT_CONFIG_CHANGE_COUNTER, enetservp_connect_table_change_counter,
        0, "connect table change counter", EPRO_DEFAULT|EPRO_NOONPRCH);
    addpropertyl(cls, ENETSERVP_LIGHTHOUSE_CHANGE_COUNTER, enetservp_lighthouse_change_counter,
        0, "LAN services change count", EPRO_NOONPRCH);

    propertysetdone(cls);
    os_unlock();
}


/**
****************************************************************************************************

  @brief Create Start network service.

  Called after eNetService object is created to create data structures and start operation.

  @param  flags Bit fields, combination of ENET_DEFAULT_NO_END_POINTS, ENET_ENABLE_IOCOM_CLIENT,
          ENET_ENABLE_ECOM_CLIENT, ENET_ENABLE_IOCOM_SERVICE, ENET_ENABLE_ECOM_SERVICE,
          and ENET_ENABLE_LIGHTHOUSE.

****************************************************************************************************
*/
void eNetService::start(
    os_int flags)
{
    os_char buf[OSAL_NETWORK_NAME_SZ];

    create_process_status_table();
    create_service_parameters(flags);
    if (flags & (ENET_ENABLE_IOCOM_SERVICE | ENET_ENABLE_ECOM_SERVICE)) {
        create_user_account_table(flags);
        create_end_point_table(flags);
        ioc_enable_user_authentication(&m_iocom_root, authorize_iocom_user, this);
    }
    if (flags & (ENET_ENABLE_IOCOM_CLIENT | ENET_ENABLE_ECOM_CLIENT)) {
        create_connect_table(flags);
        create_trusted_certificate_table();
    }
    if (flags & ENET_ENABLE_LIGHTHOUSE) {
        create_lan_services_table();
    }

    /* Setup eosal network event handler callback to keep track of errors and network state.
     */
    osal_set_net_event_handler(net_event_handler, this,
        OSAL_ADD_ERROR_HANDLER|OSAL_SYSTEM_ERROR_HANDLER);

    /* Initialize iocom communication root object and iocom protocol related stuff.
     */
    ioc_initialize_root(&m_iocom_root, IOC_USE_EOSAL_MUTEX);

    os_strncpy(buf, eglobal->process_id, sizeof(buf));
    // os_strncat(buf, "N", sizeof(buf));
    ioc_set_iodevice_id(&m_iocom_root, eglobal->process_name, eglobal->process_nr,
        OS_NULL, buf);

    m_eio_root = eio_initialize(&m_iocom_root, this);

    /* Start the connection and end point management as separate thread. This must be after parmaters
     * haven been created so binding succeed.
     */
    enet_start_maintain_thread(this, flags, &m_maintain_thread_handle);

    /* Start the light house service as separate thread. This must be after parmaters
     * haven been created so binding succeed.
     */
    if (flags & ENET_ENABLE_LIGHTHOUSE) {
        enet_start_lighthouse_thread(this, flags, &m_lighthouse_thread_handle);
    }
}


/* Start closing net service (no process lock).
 */
void eNetService::finish()
{
    /* Stop light house thread.
     */
    m_lighthouse_thread_handle.terminate();
    m_lighthouse_thread_handle.join();

    /* Stop network maintenance thread.
     */
    m_maintain_thread_handle.terminate();
    m_maintain_thread_handle.join();

    /* Remove eosal network event handler.
     */
    osal_set_net_event_handler(OS_NULL, this,
        OSAL_ADD_ERROR_HANDLER|OSAL_SYSTEM_ERROR_HANDLER);

    eio_stop_io_thread(m_eio_root);

    delete m_eio_root;
    m_eio_root = OS_NULL;

    ioc_release_root(&m_iocom_root);
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
            setpropertyl(ENETSERVP_ENDPOINT_CONFIG_CHANGE_COUNTER, ++m_end_points_config_counter);
        }

        if (obj == m_connect) {
            setpropertyl(ENETSERVP_CONNECT_CONFIG_CHANGE_COUNTER, ++m_connect_config_counter);
        }
    }

    else if (event == ECALLBACK_TABLE_CONTENT_CHANGED)
    {
        if (obj == m_services_matrix) {
            setpropertyl(ENETSERVP_LIGHTHOUSE_CHANGE_COUNTER, ++m_lighthouse_change_counter);
        }
    }

    else if (event == ECALLBACK_VARIABLE_VALUE_CHANGED)
    {
        if (obj == m_persistent_parameters) {
            parameter_changed(appendix);
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

  @brief Create global eNetService object.

  Setup network service class and creates global network service object.
  Call to this function must be followed by enet_add_protocol() calls and one
  enet_start_service() call.

****************************************************************************************************
*/
void enet_initialize_service()
{
    eNetService::setupclass();
    eNetMaintainThread::setupclass();
    eProtocol::setupclass();
    eProtocolHandle::setupclass();

    os_lock();
    eNetService *netservice = new eNetService(eglobal->process);
    eglobal->netservice = netservice;
    os_unlock();
}


/**
****************************************************************************************************

  @brief Add a communication protocol for eNetService object.

  @param  protocol Communication protocol object.

****************************************************************************************************
*/
void enet_add_protocol(
    eProtocol *protocol)
{
    os_lock();
    protocol->adopt(eglobal->netservice->protocols());
    protocol->initialize_protocol(eglobal->netservice, OS_NULL);
    os_unlock();
}


/**
****************************************************************************************************

  @brief Start the network service.

  Start running eNetService task.

  @param  flags Bit fields, combination of ENET_ENABLE_IOCOM_CLIENT, ENET_ENABLE_ECOM_CLIENT,
          ENET_ENABLE_IOCOM_SERVICE, ENET_ENABLE_ECOM_SERVICE, ENET_ENABLE_LIGHTHOUSE.

****************************************************************************************************
*/
void enet_start_service(
    os_int flags)
{
    os_lock();
    eglobal->netservice->start(flags);
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
