/**

  @file    esynchronized.cpp
  @brief   Synchronized data exchange.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    21.10.2020

  When transferring large amount of data, it is sometimes necessary to divide the data
  into pieces and transfer these as received. Typically thread sending the data is
  in loop to collect the data, and should not process messages.

  To make this work, an intermediate eSynchronized object created under eProcess.
  Data will be sent and received data acknowledged by this object. This provides
  flow controlled data transfer.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "eobjects.h"


/**
****************************************************************************************************

  @brief Property binding class constructor.

  Clear member variables.
  @return  None.

****************************************************************************************************
*/
eSynchronized::eSynchronized(
    eObject *parent,
    e_oid id,
    os_int flags)
    : eObject(parent, id, flags)
{
    /* Row set bindings cannot be cloned or serialized.
     */
    setflags(EOBJ_NOT_CLONABLE|EOBJ_NOT_SERIALIZABLE);

    /* Clear member variables.
     */
    m_path = OS_NULL;
    m_ref = OS_NULL;
    m_event = OS_NULL;
}


/**
****************************************************************************************************

  @brief Virtual destructor.

  If connected, disconnect binding. Release all resources allocated for the binging.
  @return  None.

****************************************************************************************************
*/
eSynchronized::~eSynchronized()
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
void eSynchronized::setupclass()
{
    const os_int cls = ECLASSID_SYNCHRONIZED;

    /* Add the class to class list.
     */
    os_lock();
    eclasslist_add(cls, (eNewObjFunc)newobj, "eSynchronized");
    //propertysetdone(cls);
    os_unlock();
}


/**
****************************************************************************************************

  @brief Function to process incoming messages.

  The eSynchronized::onmessage function handles messages received by object. If this function
  doesn't process message, it calls parent class'es onmessage function.

  @param   envelope Message envelope. Contains command, target and source paths and
           message content, etc.
  @return  None.

****************************************************************************************************
*/
void eSynchronized::onmessage(
    eEnvelope *envelope)
{
    /* If at final destination for the message.
     */
    if (*envelope->target()=='\0')
    {
        switch (envelope->command())
        {
            case ECMD_BIND_REPLY:
                return;

            case ECMD_UNBIND:
            case ECMD_SRV_UNBIND:
            case ECMD_NO_TARGET:
                return;

            case ECMD_FWRD:
                return;

            case ECMD_ACK:
                return;

            case ECMD_REBIND:
                return;
        }
    }

    /* Call parent class'es onmessage.
     */
    eObject::onmessage(envelope);
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
eStatus eSynchronized::simpleproperty(
    os_int propertynr,
    eVariable *x)
{
    switch (propertynr)
    {
        /* case ERSETP_LIMIT:
            if (m_pstruct.limit == 0) goto clear_x;
            x->setl(m_pstruct.limit);
            break; */

        default:
            return eObject::simpleproperty(propertynr, x);
    }
    return ESTATUS_SUCCESS;

// clear_x:
    x->clear();
    return ESTATUS_SUCCESS;
}


/**
****************************************************************************************************

  @brief Initialize synchronized transfer object.

  The eSynchronized::initialize_synch_transfer function.

  @param  path Path to remote object.
  @return None.

****************************************************************************************************
*/
void eSynchronized::initialize_synch_transfer(
    const os_char *path)
{
    if (m_ref) {
        osal_debug_error("initialize_synch_transfer: Function called twice");
        finish_sync_transfer(OS_TRUE);
    }

    m_event = osal_event_create();
    m_path = new eVariable(this);
    m_path->sets(path);
    m_ref = new ePointer(this);

    os_lock();

    // connector = new eSyncConnector(process?);
    // m_ref->set(connector);

    os_unlock();
}


/**
****************************************************************************************************

  @brief Finished with synchronized transfer object, clean up.

  The eSynchronized::finish_sync_transfer function.

  @param  abort
  @return None.

****************************************************************************************************
*/
void eSynchronized::finish_sync_transfer(
        os_boolean abort)
{
    eObject *connector;

    if (m_ref == OS_NULL) {
        osal_debug_error("finish_sync_transfer: Sync transfer has bit been initialized");
        finish_sync_transfer(OS_TRUE);
    }

    os_lock();
    connector = m_ref->get();
    delete connector;
    os_unlock();

    delete m_path;
    delete m_ref;
    osal_event_delete(m_event);
}


/**
****************************************************************************************************

  @brief Send a message usint synchnronization.

  The eSynchronized::synch_send() function sends an envelope as message. The message envelope
  (or clone of it) will be recieved as onmessage call by remote object.

  The envelope object given as argument is adopted/deleted by this function.

  @param   envelope Message envelope to send. Contains command, target and source paths and
           message content, etc. Envelope pointer will not be valid after this function call.
  @return  The function returns ESTATUS_SUCCESS if all is fine. Other return values indicate
           that there is no connection to the receiving object.

****************************************************************************************************
*/
eStatus eSynchronized::synch_send(
    eEnvelope *envelope)
{
    eSyncConnector *connector;

    /* Make sure that this has been initialize_synch_transfer() has been called.
     */
    if (m_ref == OS_NULL) {
        osal_debug_error("synch_send: not initialized");
        return ESTATUS_FAILED;
    }

    /* Start thread sync.
     */
    os_lock();

    /* Get pointer to sync connector object within process. If we do not have it, it
       has been deleted under process?
     */
    connector = eSyncConnector::cast(m_ref->get());
    if (connector == OS_NULL)
    {
        osal_debug_error("synch_send: connector object has been deleted?");
        os_unlock();
        return ESTATUS_FAILED;
    }

     /* Make sure that this has been initialize_synch_transfer() has been called.
     */
    // connector->message();

    // Increment in air count.
    connector->increment_in_air_count();

    /* End thread sync.
     */
    os_unlock();
}


/**
****************************************************************************************************

  @brief Check for received reply messages.

  The sync_receive function.

  @return envelope Message envelope.

****************************************************************************************************
*/
eEnvelope *eSynchronized::sync_receive(
    eObject *parent)
{
    return OS_NULL;
}


/**
****************************************************************************************************

  @brief Get number of messages send minus number of messages received.

  The

  @return "In air count".

****************************************************************************************************
*/
os_int eSynchronized::in_air_count()
{
    eSyncConnector *connector;
    os_int count;

    /* Make sure that this has been initialize_synch_transfer() has been called.
     */
    if (m_ref == OS_NULL) {
        osal_debug_error("in_air_count: not initialized");
        return 0;
    }

    /* Start thread sync.
     */
    os_lock();

    /* Get pointer to sync connector object within process. If we do not have it, it
       has been deleted under process?
     */
    connector = eSyncConnector::cast(m_ref->get());
    if (connector == OS_NULL)
    {
        osal_debug_error("in_air_count: connector object has been deleted?");
        os_unlock();
        return 0;
    }

    /* Get "in air count".
     */
    count = connector->in_air_count();

    /* End thread sync and return count.
     */
    os_unlock();
    return count;
}



/**
****************************************************************************************************

  @brief Wait until "in air count" is less or equal than argument.

  The

  @param  in_air_count
  @param  timeout_ms
  @return None.

****************************************************************************************************
*/
eStatus eSynchronized::sync_wait(
    os_int in_air_count,
    os_long timeout_ms)
{
    return ESTATUS_FAILED;
}


