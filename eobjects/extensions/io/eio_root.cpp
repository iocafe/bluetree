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
    m_time_now = 0;

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
    eclasslist_add(cls, (eNewObjFunc)newobj, "eioRoot", ECLASSID_CONTAINER);
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
    minfo.eio_root = t;
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
        minfo.mblk = mblk;
        minfo.root = root;
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

            if (mblk) {
                t->connected(&minfo);
            }
            else {
                osal_debug_error("IOC_NEW_MEMORY_BLOCK: NULL mblk");
            }

            break;

        case IOC_MBLK_CONNECTED_AS_SOURCE:
        case IOC_MBLK_CONNECTED_AS_TARGET:
            break;

        case IOC_MEMORY_BLOCK_DELETED:
            if (mblk) {
                t->disconnected(&minfo);
            }
            else {
                osal_debug_error("IOC_MEMORY_BLOCK_DELETED: NULL mblk");
            }
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
eioMblk *eioRoot::connected(
    eioMblkInfo *minfo)
{
    eioNetwork *network;
    eioMblk *mblk;

    if (minfo->network_name == '\0') {
        return OS_NULL;
    }

    network = eioNetwork::cast(byname(minfo->network_name));
    if (network == OS_NULL) {
        network = new eioNetwork(this);
        network->addname(minfo->network_name);
    }

    mblk = network->connected(minfo);
    setpropertyl(EIOP_CONNECTED, OS_TRUE);
    return mblk;
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

  @brief Configure signal by information.

  The eioRoot::new_signal() function adds information about a signal based on "info" memory
  block configuration for the IO device network. If signal already exists, function jsut returns
  pointer to it. Synchronization ioc_lock() must be on when this function is called.

  @param   dnetwork Pointer to dynamic network structure.
  @param   signal_name Signal name.
  @param   mblk_name Memory block name.
  @param   device_name Device name.
  @param   device_nr Device number, if there are several same kind of IO devices, they must
           have different numbers.
  @param   addr Starting address of the signal in memory block.

  @param   n For strings n can be number of bytes in memory block for the string. For arrays n is
           number of elements reserved in memory block. Use value 1 for single variables.
  @param   ncolumns If a matrix of data is stored as an array, number of matrix columns.
           Otherwise value 1.
  @param   flags: OS_BOOLEAN, OS_CHAR, OS_UCHAR, OS_SHORT, OS_USHORT, OS_INT, OS_UINT,
           OS_LONG, OS_FLOAT, OS_DOUBLE, or OS_STR.


****************************************************************************************************
*/
void eioRoot::new_signal(
    eioMblkInfo *minfo,
    eioSignalInfo *sinfo)
{
    eioMblk *mblk;
    eioSignal *signal;
    eioDevice *device;
    eioGroup *group;
    eioVariable *variable;
    eContainer *esignals;
    const os_char *signal_name;

    mblk = connected(minfo);
    if (mblk == OS_NULL) {
        osal_debug_error_str("new_signal: Mblk could not be created: ", minfo->device_name);
        return;
    }

    /* Skip "set_" in signal name. We are merging in and out of parameter settings
       as one variable.
     */
    signal_name = sinfo->signal_name;
    if (!os_strncmp(signal_name, "set_", 4)) {
        signal_name += 4;
    }

    device = eioDevice::cast(mblk->grandparent());

    group = eioGroup::cast(device->byname(sinfo->group_name));
    if (group == OS_NULL) {
        group = new eioGroup(device);
        group->addname(sinfo->group_name);
    }

    variable = eioVariable::cast(group->byname(signal_name));
    if (variable == OS_NULL) {
        variable = new eioVariable(group);
        variable->addname(sinfo->signal_name);
        variable->setpropertys(EVARP_TEXT, signal_name);
    }

    esignals = mblk->esignals();
    signal = eioSignal::cast(esignals->byname(sinfo->signal_name));
    if (signal) if (signal->oid() != sinfo->addr) {
        delete signal;
        signal = OS_NULL;
    }
    if (signal == OS_NULL) {
        signal = new eioSignal(esignals, sinfo->addr);
        signal->addname(sinfo->signal_name);
    }
    signal->setup(variable, minfo, sinfo);
    variable->setup(signal, minfo, sinfo);
}


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
    eioSignal::setupclass();
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
