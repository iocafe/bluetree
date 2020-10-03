/**

  @file    ecroot.cpp
  @brief   Root object.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    14.9.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "econnect.h"
#include "nodeconf.h"


/* GUI property names.
 */
const os_char
    ecrootp_value[] = "x",
    ecrootp_digs[] = "x.digs",
    ecrootp_text[] = "x.text",
    ecrootp_unit[] = "x.unit",
    ecrootp_min[] = "x.min",
    ecrootp_max[] = "x.max",
    ecrootp_type[] = "x.type",
    ecrootp_attr[] = "x.attr",
    ecrootp_default[] = "x.default",
    ecrootp_gain[] = "x.gain",
    ecrootp_offset[] = "x.offset",
    ecrootp_state_bits[] = "x.quality",
    ecrootp_timestamp[] = "x.timestamp",
    ecrootp_conf[] = "conf"; /* This MUST not start with "x." */


/**
****************************************************************************************************

  @brief Constructor.

  X...

  @return  None.

****************************************************************************************************
*/
ecRoot::ecRoot(
    eObject *parent,
    e_oid id,
    os_int flags)
    : eObject(parent, id, flags)
{
}


/**
****************************************************************************************************

  @brief Virtual destructor.

  X...

  @return  None.

****************************************************************************************************
*/
ecRoot::~ecRoot()
{
    shutdown();
}

/**
****************************************************************************************************

  @brief Clone object

  The clone function clones the GUI and clonable attachments. Names will be left detached
  in clone if EOBJ_NO_MAP flag is given.

  @param  parent Parent for the clone.
  @param  id Object identifier for the clone.
  @param  aflags 0 for default operation. EOBJ_NO_MAP not to map names.
  @return Pointer to the clone.

****************************************************************************************************
*/
eObject *ecRoot::clone(
    eObject *parent,
    e_oid id,
    os_int aflags)
{
    ecRoot *clonedobj;
    clonedobj = new ecRoot(parent, id == EOID_CHILD ? oid() : id, flags());

    /** Copy variable value.
     */
    // clonedobj->setv(this);
    // clonedobj->setdigs(digs());

    /* Copy clonable attachments.
     */
    clonegeneric(clonedobj, aflags);
    return clonedobj;
}


/**
****************************************************************************************************

  @brief Add ecRoot to class list and class'es properties to it's property set.

  The ecRoot::setupclass function adds ecRoot to class list and class'es properties to
  it's property set. The class list enables creating new objects dynamically by class identifier,
  which is used for serialization reader functions. The property set stores static list of
  class'es properties and metadata for those.

****************************************************************************************************
*/
void ecRoot::setupclass()
{
    const os_int cls = ECONNCLASSID_ROOT;

    /* Add the class to class list.
     */
    os_lock();
    eclasslist_add(cls, (eNewObjFunc)newobj, "ecRoot");
    setupproperties(cls);
    propertysetdone(cls);
    os_unlock();
}


/**
****************************************************************************************************

  @brief Add class'es properties to property set.

  The ecRoot::setupproperties is helper function for setupclass, it is called from both
  ecRoot class and derived classes like eName.

  Process mutex must be locked when calling this function.

****************************************************************************************************
*/
void ecRoot::setupproperties(
    os_int cls)
{
    eVariable *p;

    /* Order of these addproperty() calls is important, since ecRoot itself is used to
       describe the properties in property set. The property to set must be added to
       property set before setting value for it. There is trick with p to set text type
       after adding property type. This effects only ecRoot class.
     */
    p = addproperty(cls, ECROOTP_TEXT, ecrootp_text, "text", EPRO_METADATA|EPRO_NOONPRCH);
    addpropertyl (cls, ECROOTP_TYPE, ecrootp_type, "type", EPRO_METADATA|EPRO_NOONPRCH);
    p->setpropertyl(ECROOTP_TYPE, OS_STR);
}


/**
****************************************************************************************************

  @brief Called to inform the class about property value change (override).

  The onpropertychange() function is called when class'es property changes, unless the
  property is flagged with EPRO_NOONPRCH.
  If property is flagged as EPRO_SIMPLE, this function shuold save the property value
  in class members and and return it when simpleproperty() is called.

  Notice for change logging: Previous value is still valid when this function is called.
  You can get the old value by calling property() function inside onpropertychange()
  function.

  @param   propertynr Property number of changed property.
  @param   x Variable containing the new value.
  @param   flags
  @return  If successfull, the function returns ESTATUS_SUCCESS (0). Nonzero return values do
           indicate that there was no property with given property number.

****************************************************************************************************
*/
eStatus ecRoot::onpropertychange(
    os_int propertynr,
    eVariable *x,
    os_int flags)
{
    switch (propertynr)
    {
/*         case ECROOTP_VALUE:
            setv(x);
            break;

        case ECROOTP_DIGS:
            setdigs((os_int)x->getl());
            break; */

        default:
            return eObject::onpropertychange(propertynr, x, flags);
    }

    return ESTATUS_SUCCESS;
}


/**
****************************************************************************************************

  @brief Get value of simple property (override).

  The simpleproperty() function stores current value of simple property into variable x.

  @param   propertynr Property number to get.
  @param   x Variable into which to store the property value.
  @return  If property with property number was stored in x, the function returns
           ESTATUS_SUCCESS (0). Nonzero return values indicate that property with
           given number was not among simple properties.

****************************************************************************************************
*/
eStatus ecRoot::simpleproperty(
    os_int propertynr,
    eVariable *x)
{
    switch (propertynr)
    {
/*
        case ECROOTP_VALUE:
            x->setv(this);
            break;

        case ECROOTP_DIGS:
            x->setl(digs());
            break;
*/

        default:
            return eObject::simpleproperty(propertynr, x);
    }
    return ESTATUS_SUCCESS;
}


eStatus ecRoot::initialize(const os_char *device_name)
{
#if 0
    osPersistentParams persistentprm;
    iocDeviceId *device_id;
    iocConnectionConfig *connconf;
    osalSecurityConfig *security;
    iocNetworkInterfaces *nics;
    iocWifiNetworks *wifis;
    osalLighthouseInfo lighthouse_info;

    /* Setup error handling. Here we select to keep track of network state. We could also
       set application specific error handler callback by calling osal_set_error_handler().
     */
    osal_initialize_net_state();

    /* Initialize persistent storage
     */
    os_memclear(&persistentprm, sizeof(persistentprm));
    persistentprm.device_name = device_name;
    os_persistent_initialze(&persistentprm);

    /* Initialize communication root and dymanic structure data root objects.
     * This demo uses dynamic signal configuration.
     */
    ioc_initialize_root(&m_root);

    /* If we are using devicedir for development testing, initialize.
     */
    io_initialize_device_console(&ioconsole, &m_root);

    /* Load device/network configuration and device/user account congiguration
       (persistent storage is typically either file system or micro-controller's flash).
       Defaults are set in network-defaults.json and in account-defaults.json.
     */
    ioc_load_node_config(&app_device_conf, ioapp_network_defaults,
        sizeof(ioapp_network_defaults), device_name, IOC_LOAD_PBNR_NODE_CONF);
    device_id = ioc_get_device_id(&app_device_conf);
    ioc_set_iodevice_id(&m_root, device_name, device_id->device_nr,
        device_id->password, device_id->network_name);

    ioc_initialize_dynamic_root(&m_root);

    /* Get service TCP port number and transport (IOC_TLS_SOCKET or IOC_TCP_SOCKET).
     */
    connconf = ioc_get_connection_conf(&app_device_conf);
    ioc_get_lighthouse_info(connconf, &lighthouse_info);

    /* Create frank main object
     */
    app_root_obj = new AppRoot(device_name, device_id->device_nr, device_id->network_name,
        device_id->publish);

    /* Set callback function to receive information about new dynamic memory blocks.
     */
    ioc_set_root_callback(&m_root, app_root_callback, OS_NULL);

    /* Setup network interface configuration and initialize transport library. This is
       partyly ignored if network interfaces are managed by operating system
       (Linux/Windows,etc),
     */
    nics = ioc_get_nics(&app_device_conf);
    wifis = ioc_get_wifis(&app_device_conf);
    security = ioc_get_security_conf(&app_device_conf);
    osal_tls_initialize(nics->nic, nics->n_nics, wifis->wifi, wifis->n_wifi, security);
    osal_serial_initialize();

    /* Connect to network.
     */
    ioc_connect_node(&m_root, connconf, IOC_DYNAMIC_MBLKS|IOC_CREATE_THREAD);

    /* Initialize light house. Sends periodic UDP broadcards to so that this service
       can be detected in network.
     */
    ioc_initialize_lighthouse_server(&lighthouse, device_id->publish, &lighthouse_info, OS_NULL);
#endif

    return ESTATUS_SUCCESS;
}


void ecRoot::shutdown()
{
}
