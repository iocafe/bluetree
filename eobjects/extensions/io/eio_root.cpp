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
    os_memclear(&m_io_thread_handle, sizeof(m_io_thread_handle));

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
    addpropertyb(cls, EIOP_CONNECTED, eiop_connected, OS_TRUE, "connected", EPRO_PERSISTENT);
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
            if (!os_strcmp(minfo.mblk_name, "info")) {
                ioc_add_callback(&mblk->handle, info_callback, context);
            }

            t->connected(&minfo);
            break;

        case IOC_MBLK_CONNECTED_AS_SOURCE:
        case IOC_MBLK_CONNECTED_AS_TARGET:
            break;

        case IOC_MEMORY_BLOCK_DELETED:
            t->disconnected(&minfo);
            break;

        default:
             break;
    }
    os_unlock();
}


/**
****************************************************************************************************
  Create IO network objects to represent connection.
****************************************************************************************************
*/
void eioRoot::connected(
    eioMblkInfo *minfo)
{
    eioNetwork *network;

    if (minfo->network_name == '\0') {
        return;
    }

    network = eioNetwork::cast(byname(minfo->network_name));
    if (network == OS_NULL) {
        network = new eioNetwork(this);
        network->addname(minfo->network_name);
    }

    network->connected(minfo);
    setpropertyl(EIOP_CONNECTED, OS_TRUE);
}


/**
****************************************************************************************************
  Mark IO network objects to disconnected and delete unused ones.
****************************************************************************************************
*/
void eioRoot::disconnected(
    eioMblkInfo *minfo)
{
    eioNetwork *network;

    network = eioNetwork::cast(byname(minfo->network_name));
    if (network) {
        network->disconnected(minfo);
    }

    for (network = eioNetwork::cast(first());
         network;
         network = eioNetwork::cast(network->next()))
    {
        if (network->propertyl(EIOP_CONNECTED)) {
            return;
        }
    }

    setpropertyl(EIOP_CONNECTED, OS_FALSE);
}


/**
****************************************************************************************************

  @brief Callback function to add dynamic device information.

  The eioRoot::info_callback() function is called when device information data is received from
  connection or when connection status changes.

  @param   handle Memory block handle.
  @param   start_addr Address of first changed byte.
  @param   end_addr Address of the last changed byte.
  @param   flags Reserved  for future.
  @param   context Application specific pointer passed to this callback function.

  @return  None.

****************************************************************************************************
*/
void eioRoot::info_callback(
    struct iocHandle *handle,
    os_int start_addr,
    os_int end_addr,
    os_ushort flags,
    void *context)
{
    eioRoot *t = (eioRoot*)context;
    iocRoot *root;
    iocMemoryBlock *mblk;
    osalJsonIndex jindex;
    osalStatus s;
    // iocAddDinfoState state;
    OSAL_UNUSED(start_addr);
    OSAL_UNUSED(flags);
    OSAL_UNUSED(context);

    /* If actual data received (not connection status change).
     */
    if (end_addr < 0) return;

    /* Get memory block pointer and start synchronization.
     */
    mblk = ioc_handle_lock_to_mblk(handle, &root);
    if (mblk == OS_NULL) return;

    {
        ioc_add_dynamic_info(handle, OS_FALSE);
    }

    s = osal_create_json_indexer(&jindex, mblk->buf, mblk->nbytes, 0);
    if (s) goto getout;

    /* s = ioc_dinfo_process_block(droot, &state, osal_str_empty, &jindex);
    if (s) goto getout; */

    ioc_unlock(root);
    os_lock();



    os_unlock();
    mblk = ioc_handle_lock_to_mblk(handle, &root);
    if (mblk == OS_NULL) return;

    /* Informn application about new networks and devices.
     */
    /* if (state.dnetwork->new_network)
    {
        ioc_new_root_event(root, IOC_NEW_NETWORK, state.dnetwork, OS_NULL, root->callback_context);
        state.dnetwork->new_network = OS_FALSE;
    }
    ioc_new_root_event(root, IOC_NEW_DEVICE, state.dnetwork, mblk, root->callback_context);
    */

    /* Flag for basic server (iocBServer). Check for missing certificate chain and
       flash program versions.
     */
    // root->check_cert_chain_etc = OS_TRUE;

getout:
    ioc_unlock(root);
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

  @brief Initialize IO network structure classes and start IO thread.

****************************************************************************************************
*/
eioRoot *eio_initialize(
    iocRoot *iocom_root,
    eObject *parent)
{
    eioRoot *eio_root;

    eioRoot::setupclass();
    eioNetwork::setupclass();
    eioDevice::setupclass();
    eioMblk::setupclass();
    eioGroup::setupclass();
    eioVariable::setupclass();
    eioThread::setupclass();

    eio_root = new eioRoot(parent);
    eio_root->addname("//io");
    eio_root->setup(iocom_root);

    eio_start_thread(eio_root, &eio_root->m_io_thread_handle);

    return eio_root;
}

/**
****************************************************************************************************

  @brief Stop IO thread.

****************************************************************************************************
*/
void eio_stop_io_thread(
    eioRoot *eio_root)
{
    /* Stop network maintenance thread.
     */
    eio_root->m_io_thread_handle.terminate();
    eio_root->m_io_thread_handle.join();
}
