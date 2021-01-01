/**

  @file    esynchronized.cpp
  @brief   Synchronized data exchange.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    21.10.2020

  When transferring large amount of data, it is sometimes necessary to divide the data
  into pieces and transfer these as received. Typically thread sending the data is
  in loop to collect the data, and should not process messages (not able to receive
  acknowledge as regular message).

  To make this work, an intermediate eSyncConnector object created under eProcess.
  Data will be sent and received data acknowledged tough the eSyncConnector object.
  This provides flow controlled data transfer.

  The eSynchronized object is created on in thread sending messages. It is used to manage
  the eSyncConnector in process memory tree, pass data to/from ir and take care of
  thread synchronization.

  The same mechanism can be used to implement "function calls" trough messaging. A thread
  sending a message to other thread, process, etc, and halting until reply is received.

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
    m_context = OS_NULL;
    m_synchronize = OS_FALSE;
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
    finish_sync_transfer(OS_TRUE);
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
    os_unlock();
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

  @brief Initialize eSynchronized for data transfer.

  The eSynchronized::initialize_synch_transfer function.

  If this is run by process, no synchronization is done, but data is sent by the synchronization
  object.

  @param  path Path to remote object.
  @return None.

****************************************************************************************************
*/
void eSynchronized::initialize_synch_transfer(
    const os_char *path)
{
    eContainer *connectors;
    eSyncConnector *connector;
    eThread *t;

    if (m_path) {
        osal_debug_error("initialize_synch_transfer: Function called twice");
        finish_sync_transfer(OS_TRUE);
    }

    /* If this object is run process.
     */
    m_synchronize = OS_FALSE;
    t = thread();
    if (t) {
        m_synchronize = (os_boolean)(t->classid() != ECLASSID_PROCESS);
    }

    m_path = new eVariable(this);
    m_path->sets(path);

    if (m_synchronize) {
        m_ref = new ePointer(this);
        m_event = osal_event_create();

        os_lock();

        connectors = eProcess::sync_connectors();
        connector = new eSyncConnector(connectors);
        connector->set_sync_event(m_event);
        m_ref->set(connector);

        os_unlock();
    }
    else {
        m_context = new eVariable(this);
        m_context->sets("pc");
        m_context->appendl(osal_rand(1, 100000));
    }
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

    if (m_path == OS_NULL) {
        return;
    }

    if (m_synchronize)
    {
        os_lock();
        connector = m_ref->get();
        delete connector;
        os_unlock();
        delete m_ref;
        m_ref = OS_NULL;
        osal_event_delete(m_event);
        m_event = OS_NULL;
    }
    else {
        delete m_context;
        m_context = OS_NULL;
    }

    delete m_path;
    m_path = OS_NULL;
}


/**
****************************************************************************************************

  @brief Send a message, synchronization.

  The eSynchronized::synch_send() function sends an envelope as message. The message envelope
  (or clone of it) will be recieved as onmessage call by remote object.

  The envelope object given as argument is adopted/deleted by this function.

  @param   envelope Message envelope to send. Contains command, target and source paths and
           message content, etc. Envelope pointer will not be valid after this function call.
           Context and target will be set by this function.
  @return  The function returns ESTATUS_SUCCESS if all is fine. Other return values indicate
           that there is no connection to the receiving object.

****************************************************************************************************
*/
eStatus eSynchronized::synch_send(
    eEnvelope *envelope)
{
    eSyncConnector *connector;
    eStatus s;

    /* Make sure that this has been initialize_synch_transfer() has been called.
     */
    if (m_path == OS_NULL) {
        osal_debug_error("synch_send: not initialized");
        return ESTATUS_FAILED;
    }

    /* Set target path.
     */
    envelope->settarget(m_path);

    if (m_synchronize)
    {
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

        if (connector->failed()) {
            os_unlock();
            return ESTATUS_FAILED;
        }

        /* Send message trough connector.
         */
        s = connector->send_message(envelope);

        /* End thread sync.
         */
        os_unlock();
    }
    else {
        envelope->setcontext(m_context);
        envelope->addmflags(EMSG_NO_REPLIES);
        message(envelope);
        s = ESTATUS_SUCCESS;
    }

    return s;
}


/**
****************************************************************************************************

  @brief Check for received reply messages.

  The sync_receive function

  @return envelope Message envelope.

****************************************************************************************************
*/
eEnvelope *eSynchronized::sync_receive(
    eObject *parent)
{
    eSyncConnector *connector;
    eEnvelope *envelope;

    /* Make sure that this has been initialize_synch_transfer() has been called.
     */
    if (m_path == OS_NULL) {
        osal_debug_error("sync_receive: not initialized");
        return OS_NULL;
    }

    if (!m_synchronize) {
        return OS_NULL;
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
        osal_debug_error("sync_receive: connector object has been deleted?");
        os_unlock();
        return OS_NULL;
    }

    if (connector->failed()) {
        os_unlock();
        return OS_NULL;
    }

    /* Check for received reply messages.
     */
    envelope = connector->get_received_message(parent);

    /* End thread sync and return count.
     */
    os_unlock();

    return envelope;
}


/**
****************************************************************************************************

  @brief Get number of messages send minus number of messages received.

  The function is used:
  - To check if remote object has acknowledged N enough received envelopes to send more data.
  - To check if a reply has been received from remote object.

  @return  "In air count": Number of synchronized data transfer messages which have been sent
           but not yet acknowledged or replied. -1 if failed. If the transfer is not synchronized,
           then the function returns 0.

****************************************************************************************************
*/
os_int eSynchronized::in_air_count()
{
    eSyncConnector *connector;
    os_int count;
    os_boolean failed;

    /* Make sure that this has been initialize_synch_transfer() has been called.
     */
    if (m_path == OS_NULL) {
        osal_debug_error("in_air_count: not initialized");
        return -1;
    }

    if (!m_synchronize) {
        return 0;
    }

    /* Start thread sync.
     */
    os_lock();

    /* Get pointer to sync connector object within process. If we do not have it, it
       has been deleted under process?
     */
    connector = eSyncConnector::cast(m_ref->get());
    if (connector == OS_NULL) {
        osal_debug_error("in_air_count: connector object has been deleted?");
        os_unlock();
        return -1;
    }

    if (connector->failed()) {
        os_unlock();
        return -1;
    }

    /* Get "in air count".
     */
    count = connector->in_air_count();
    failed = connector->failed();

    /* End thread sync and return count.
     */
    os_unlock();
    return failed ? -1 : count;
}


/**
****************************************************************************************************

  @brief Wait until "in air count" is less or equal than argument count.

  The functions wait until "in air count" is less or equal than argument "count", or timeout
  elapses. This function is used:
  - To wait until remote object has acknowledged N enough received envelopes to send more data.
  - To wait until a reply has been received from remote object.

  If the transnsfer is not synchronized, the function returns always ESTATUS_SUCCESS.

  @param  count "in air count to wait for".
  @param  timeout_ms Wait timeout. To wait infinetly give OSAL_EVENT_INFINITE (-1) here.
          To check "in air count" without waiting set timeout_ms to 0.

  @return ESTATUS_SUCCESS when "in air count" is less or equal to count, ESTATUS_TIMEOUT
          if the function timed out, or ESTATUS_FAILED if the operation has failed.

****************************************************************************************************
*/
eStatus eSynchronized::sync_wait(
    os_int count,
    os_long timeout_ms)
{
    os_int c;

    if (!m_synchronize) {
        return ESTATUS_SUCCESS;
    }

    while (OS_TRUE) {
        c = in_air_count();
        if (c == -1) return ESTATUS_FAILED;
        if (in_air_count() <= count) break;

        if (osal_event_wait(m_event, timeout_ms)) {
            return ESTATUS_TIMEOUT;
        }
    }

    return ESTATUS_SUCCESS;
}


