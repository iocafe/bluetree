/**

  @file    elighthouse_client.cpp
  @brief   Look out for device networks in the same LAN.
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
eLightHouseClient::eLightHouseClient(
    eObject *parent,
    e_oid oid,
    os_int flags)
    : eThread(parent, oid, flags)
{
//    initproperties();
}


/**
****************************************************************************************************
  Virtual destructor.
****************************************************************************************************
*/
eLightHouseClient::~eLightHouseClient()
{
}


/**
****************************************************************************************************

  @brief Clone object

  The eLightHouseClient::clone function clones and object including object's children.
  Names will be left detached in clone.

  @param  parent Parent for the clone.
  @param  oid Object identifier for the clone.
  @param  aflags 0 for default operation. EOBJ_NO_MAP not to map names.
  @return Pointer to the clone.

****************************************************************************************************
*/
/* eObject *eLightHouseClient::clone(
    eObject *parent,
    e_oid id,
    os_int aflags)
{
    eObject *clonedobj;
    clonedobj = new eLightHouseClient(parent, id == EOID_CHILD ? oid() : id, flags());
    clonegeneric(clonedobj, aflags|EOBJ_CLONE_ALL_CHILDREN);
    return clonedobj;
} */


/**
****************************************************************************************************

  @brief Add the class to class list and class'es properties to it's property set.

  The eVariable::setupclass function adds the class to class list and class'es properties to
  it's property set. The class list enables creating new objects dynamically by class identifier,
  which is used for serialization reader functions. The property set stores static list of
  class'es properties and metadata for those.

****************************************************************************************************
*/
void eLightHouseClient::setupclass()
{
    const os_int cls = ECLASSID_LIGHT_HOUSE_CLIENT;

    /* Add the class to class list.
     */
    os_lock();
    eclasslist_add(cls, (eNewObjFunc)newobj, "eLightHouseClient");
    os_unlock();
}

/* Overloaded eThread function to initialize new thread. Called after eLightHouseClient object is created.
 */
void eLightHouseClient::initialize(
    eContainer *params)
{
    ns_create(); // ???????????????

    ioc_initialize_lighthouse_client(&m_lighthouse,
        OS_FALSE, /* is_ipv6 */
        OS_FALSE, /* is_tls */
        OS_NULL);

    ioc_set_lighthouse_client_callback(&m_lighthouse, callback, this);
}


/* Overloaded eThread function to perform thread specific cleanup when threa exists: Release
   resources allocated for lighthouse client. This is a "pair" to initialize function.
 */
void eLightHouseClient::finish()
{
    ioc_release_lighthouse_client(&m_lighthouse);
}

/* Listen for lighthouse USP multicasts.
 */
void eLightHouseClient::run()
{
    osalStatus s;

    while (OS_TRUE)
    {
        alive(EALIVE_RETURN_IMMEDIATELY);
        if (exitnow()) break;

        s = ioc_run_lighthouse_client(&m_lighthouse, m_trigger);
        if (s != OSAL_SUCCESS) {
            if (s != OSAL_PENDING) {
osal_debug_error_int("ioc_run_lighthouse_client failed, s=", s);
            }
            os_sleep(500);
        }

osal_debug_error("XXX");
    }
}

/* Callback by the same thread which calls ioc_run_lighthouse_client()
 */
void eLightHouseClient::callback(
    LighthouseClient *c,
    os_char *ip_addr,
    os_int tls_port_nr,
    os_int tcp_port_nr,
    os_char *network_name,
    void *context)
{
    osal_debug_error_str("HERE 1 ", ip_addr);
    osal_debug_error_str("HERE 2 ", network_name);
    osal_debug_error_int("HERE 3 ", tls_port_nr);
    osal_debug_error_int("HERE 4 ", osal_rand(1, 1000));
}

/* Start light house client.
 */
void enet_start_lighthouse_client(
    eThreadHandle *lighthouse_client_thread_handle)
{
    struct eLightHouseClient *lighthouse;

    /* Set up class for use.
     */
    eLightHouseClient::setupclass();

    /* Create and start thread to listen for lighthouse UDP multicasts,
       name it "//lookout".
     */
    lighthouse = new eLightHouseClient();
    lighthouse->addname("//lookout");
    lighthouse->start(lighthouse_client_thread_handle);
}
