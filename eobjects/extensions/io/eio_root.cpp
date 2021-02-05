/**

  @file    eio_root.cpp
  @brief   Object representing and IO root.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "extensions/io/eio.h"


/**
****************************************************************************************************
  Constructor.
****************************************************************************************************
*/
eioRoot::eioRoot(
    eObject *parent,
    e_oid oid,
    os_int flags)
    : eContainer(parent, oid, flags)
{
    initproperties();
    ns_create();
}


/**
****************************************************************************************************
  Virtual destructor.
****************************************************************************************************
*/
eioRoot::~eioRoot()
{
}


/**
****************************************************************************************************

  @brief Clone object

  The eioRoot::clone function clones and object including object's children.
  Names will be left detached in clone.

  @param  parent Parent for the clone.
  @param  oid Object identifier for the clone.
  @param  aflags 0 for default operation. EOBJ_NO_MAP not to map names.
  @return Pointer to the clone.

****************************************************************************************************
*/
eObject *eioRoot::clone(
    eObject *parent,
    e_oid id,
    os_int aflags)
{
    eObject *clonedobj;
    clonedobj = new eioRoot(parent, id == EOID_CHILD ? oid() : id, flags());
    clonegeneric(clonedobj, aflags|EOBJ_CLONE_ALL_CHILDREN);
    return clonedobj;
}


/**
****************************************************************************************************

  @brief Add the class to class list and class'es properties to it's property set.

  The eioRoot::setupclass function adds the class to class list and class'es properties to
  it's property set. The class list enables creating new objects dynamically by class identifier,
  which is used for serialization reader functions. The property set stores static list of
  class'es properties and metadata for those.

****************************************************************************************************
*/
void eioRoot::setupclass()
{
    const os_int cls = ECLASSID_EIO_ROOT;

    /* Add the class to class list.
     */
    os_lock();
    eclasslist_add(cls, (eNewObjFunc)newobj, "eioRoot");
    addpropertys(cls, EIOP_TEXT, eiop_text, "text", EPRO_PERSISTENT);
    propertysetdone(cls);
    os_unlock();
}


/**
****************************************************************************************************

  @brief Function to process incoming messages.

  The eTreeNode::onmessage function handles messages received by object. If this function
  doesn't process message, it calls parent class'es onmessage function.

  @param   envelope Message envelope. Contains command, target and source paths and
           message content, etc.
  @return  None.

****************************************************************************************************
*/
void eioRoot::onmessage(
    eEnvelope *envelope)
{
    /* If at final destination for the message.
     */
    /* if (*envelope->target()=='\0' && envelope->command() == ECMD_TIMER)
    {
        check_save_timer();
        return;
    } */

    /* Default thread message processing.
     */
    eContainer::onmessage(envelope);
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
eStatus eioRoot::onpropertychange(
    os_int propertynr,
    eVariable *x,
    os_int flags)
{
    switch (propertynr)
    {
        case EIOP_TEXT:
            break;

        default:
            goto call_parent;
    }

    return ESTATUS_SUCCESS;

call_parent:
    return eContainer::onpropertychange(propertynr, x, flags);
}


/**
****************************************************************************************************

  @brief Process a callback from a child object.

  The eioRoot::oncallback function

****************************************************************************************************
*/
eStatus eioRoot::oncallback(
    eCallbackEvent event,
    eObject *obj,
    eObject *appendix)
{
    /* switch (event)
    {
        case ECALLBACK_VARIABLE_VALUE_CHANGED:
        case ECALLBACK_TABLE_CONTENT_CHANGED:
            touch();
            break;

        default:
            break;
    } */


    /* If we need to pass callback to parent class.
     */
    if (flags() & (EOBJ_PERSISTENT_CALLBACK|EOBJ_TEMPORARY_CALLBACK)) {
        eContainer::oncallback(event, obj, appendix);
    }

    return ESTATUS_SUCCESS;
}


/**
****************************************************************************************************

  @brief Connect root object to IOCOM.

  The eioRoot::setup function...

  @param   iocom_root Pointer to IOCOM root object.

****************************************************************************************************
*/
void eioRoot::setup(
    iocRoot *iocom_root)
{
    m_iocom_root = iocom_root;
    ioc_set_root_callback(iocom_root, io_root_callback, this);
}


/**
****************************************************************************************************

  @brief Root callback function (process network and device connect/disconnect, etc).

  The eioRoot::io_root_callback function is called by IOCOM library when a new thread, etc
  is created.

  This function can be called by any thread, and thus use os_lock() to synchronize access
  to global objects.

****************************************************************************************************
*/
void eioRoot::io_root_callback(
    struct iocRoot *root,
    iocEvent event,
    struct iocDynamicNetwork *dnetwork,
    struct iocMemoryBlock *mblk,
    void *context)
{
    eioRoot *t = (eioRoot*)context;
    eioMblkInfo minfo;

    os_memclear(&minfo, sizeof(minfo));
    minfo.network_name = osal_str_empty;
    if (dnetwork)
    {
        minfo.network_name = dnetwork->network_name;
    }
    if (mblk)
    {
#if IOC_MBLK_SPECIFIC_DEVICE_NAME
        minfo.network_name = mblk->network_name;
        minfo.device_name = mblk->device_name;
        minfo.device_nr = mblk->device_nr;
#else
        minfo.network_name = root->network_name;
        minfo.device_name = root->device_name;
        minfo.device_nr = root->device_nr;
#endif
        minfo.mblk_name = mblk->mblk_name;
    }
    else
    {
        minfo.device_name = osal_str_empty;
        minfo.device_nr = 0;
        minfo.mblk_name = osal_str_empty;
    }

    os_lock();
    switch (event)
    {
        case IOC_NEW_MEMORY_BLOCK:
            t->network_connected(&minfo);
            break;

        case IOC_MBLK_CONNECTED_AS_SOURCE:
        case IOC_MBLK_CONNECTED_AS_TARGET:
            break;

        default:
        case IOC_MEMORY_BLOCK_DELETED:
            break;

        case IOC_NEW_NETWORK:
            // t->network_connected(&minfo);
            break;

        case IOC_NETWORK_DISCONNECTED:
            // t->network_disconnected(&minfo);
            break;

        case IOC_NEW_DEVICE:
        case IOC_DEVICE_DISCONNECTED:
            break;
    }
    os_unlock();
}


/**
****************************************************************************************************

  @brief Find or create a IO network object.

  The eioRoot::network_connected function checks if a network exists. If so it makes sure that
  network is set as connected returns pointer to network object. If not, it creates new
  network object and marks it connected.

  returns Pointer to network object OS_NULL if network name is empty.

****************************************************************************************************
*/
eioNetwork *eioRoot::network_connected(
    eioMblkInfo *minfo)
{
    eioNetwork *network;

    if (minfo->network_name == '\0') {
        return OS_NULL;
    }

    network = eioNetwork::cast(byname(minfo->network_name));
    if (network) {
        network->setpropertyl(EIOP_CONNECTED, OS_TRUE);
    }
    else {
        network = new eioNetwork(this);
        network->addname(minfo->network_name);
    }

    network->device_connected(minfo);
    return network;
}


/**
****************************************************************************************************

  @brief Mark network object disconnected and delete it, if it is unused.

  The eioRoot::network_disconnected function...

  returns Pointer to disconnected network object. OS_NULL if network with given name does no
          longer exist.

****************************************************************************************************
*/
eioNetwork *eioRoot::network_disconnected(
    eioMblkInfo *minfo)
{
    eioNetwork *network;

    network = eioNetwork::cast(byname(minfo->network_name));
    if (network) {
        network->setpropertyl(EIOP_CONNECTED, OS_FALSE);
        return network;
    }
    return OS_NULL;
}


/**
****************************************************************************************************

  @brief Flags the peristent object changed (needs to be saved).

  The eioRoot::touch function

****************************************************************************************************
*/
/* void eioRoot::touch()
{
    os_get_timer(&m_latest_touch);
    if (m_oldest_touch == 0) {
        m_oldest_touch = m_latest_touch;
    }

    set_timer(m_save_time);
}
*/



/**
****************************************************************************************************

  @brief Initialize IO network structure classes.

  The eioRoot::touch function

****************************************************************************************************
*/
void eio_initialize()
{
    eioRoot::setupclass();
    eioNetwork::setupclass();
    eioDevice::setupclass();
    eioMblk::setupclass();
    eioGroup::setupclass();
    eioVariable::setupclass();
}
