/**

  @file    ethread.cpp
  @brief   Thread class.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

  The thread object is the root of thread's object tree.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "eobjects.h"

/** Parameter structure for creating thread.
 */
typedef struct
{
    /* Pointer to the thread object
     */
    eThread *thread;

    /** Parameters for new thread.
     */
    eContainer *params;
}
eThreadParameters;


/* Forward referred static functions.
 */
static void ethread_func(
    void *prm,
    osalEvent done);


/**
****************************************************************************************************
  Constructor.
****************************************************************************************************
*/
eThread::eThread(
    eObject *parent,
    e_oid id,
    os_int flags)
    : eObject(parent, id, flags)
{
    /* Create thread triggger.
     */
    m_trigger = osal_event_create(OSAL_EVENT_SET_AT_EXIT);

    /* Create message queue for incoming messages. This is not child of the thread object.
     */
    m_message_queue = new eContainer(OS_NULL, EOID_INTERNAL, EOBJ_TEMPORARY_ATTACHMENT);

    m_exit_requested = OS_FALSE;
}


/**
****************************************************************************************************
  Virtual destructor.
****************************************************************************************************
*/
eThread::~eThread()
{
    eObject *o, *next_o, *bindings;

    /* Delete children before deleting message queue. Delete first regular child objects,
       then attachments. Leave eRoot to be for now.
     */
    bindings = first(EOID_BINDINGS);
    if (bindings) {
        for (o = bindings->first(); o; o = next_o) {
            next_o = o->next();
            delete o;
        }
    }
    for (o = first(); o; o = next_o) {
        next_o = o->next();
        delete o;
    }
    for (o = first(EOID_ALL); o; o = next_o) {
        next_o = o->next(EOID_ALL);
        if (o->oid() != EOID_ROOT_HELPER) {
            delete o;
        }
    }

    /* Delete the message queue. Not child of the tread, so this needs to be deleted explicitely.
     */
    delete m_message_queue;
    m_message_queue = OS_NULL;

    /* Release thread triggger.
     */
    osal_event_delete(m_trigger);
}


/**
****************************************************************************************************

  @brief Add eThread to class list and class'es properties to it's property set.

  The eThread::setupclass function adds eThread to class list and class'es properties to
  it's property set. The class list enables creating new objects dynamically by class identifier,
  which is used for serialization reader functions. The property set stores static list of
  class'es properties and metadata for those.

****************************************************************************************************
*/
void eThread::setupclass()
{
    const os_int cls = ECLASSID_THREAD;

    /* Add the class to class list.
     */
    os_lock();
    eclasslist_add(cls, (eNewObjFunc)newobj, "eThread");
    os_unlock();
}


/**
****************************************************************************************************

  @brief Function to process incoming messages.

  The eThread::onmessage function handles messages received by the thread object.

  @param   envelope Message envelope. Contains command, target and source paths and
           message content, etc.
  @return  None.

****************************************************************************************************
*/
void eThread::onmessage(
    eEnvelope *envelope)
{
    os_char
        *target;

    target = envelope->target();

    if (*target == '\0')
    {
        switch (envelope->command())
        {
            case ECMD_EXIT_THREAD:
                m_exit_requested = OS_TRUE;
                return;
        }
    }

    eObject::onmessage(envelope);
}


/**
****************************************************************************************************

  @brief Start thread.

  After calling this funcion, eThread pointer thiso cannot be used from calling thread.

  @return  None.

****************************************************************************************************
*/
void eThread::start(
    eThreadHandle *thandle,
    eContainer *params)
{
    eThreadParameters prmstruct;
    osalThread *handle;

    /* Save unique name into handle for controlling thread trough handle.
     */
    if (thandle)
    {
        thandle->save_unique_thread_name(this);
    }

    os_memclear(&prmstruct, sizeof(prmstruct));
    prmstruct.thread = this;
    if (params)
    {
        prmstruct.params = eContainer::cast(params->clone(this, EOID_INTERNAL));
    }

    /* Initialize the thread.
     */
    initialize(prmstruct.params);

    if (thandle)
    {
        handle = osal_thread_create(ethread_func, &prmstruct, OS_NULL, OSAL_THREAD_ATTACHED);
        thandle->set_osal_handle(handle);
    }
    else
    {
        osal_thread_create(ethread_func, &prmstruct, OS_NULL, OSAL_THREAD_DETACHED);
    }
}


/**
****************************************************************************************************

  @brief Thread 1 entry point function.

  The my_thread_1_func() function is called to start the thread.

  @param   prm Pointer to parameters for new thread. This pointer must can be used only
           before "done" event is set. This can be OS_NULL if no parameters are needed.
  @param   done Event to set when parameters have been copied to entry point
           functions own memory.

  @return  None.

****************************************************************************************************
*/
static void ethread_func(
    void *prm,
    osalEvent done)
{
    eThreadParameters prmstruct;

    /* Copy parameters to local stack
     */
    os_memcpy(&prmstruct, prm, sizeof(eThreadParameters));

    /* Let thread which created this one proceed.
     */
    osal_event_set(done);

    /* Run the thread.
     */
    prmstruct.thread->run();

    /* Finish with thread
     */
    prmstruct.thread->finish();

    /* Delete the thread object.
     */
    delete prmstruct.thread;
}


void eThread::run()
{
    while (!exitnow())
    {
        alive();
    }
}


/**
****************************************************************************************************

  @brief Place an envelope to thread's message queue

  The eThread::queue function...

  Process mutex must be locked when calling this function!!!

  @param  envelope Pointer to envelope. Envelope will be adopted by this function.
  @return None.

****************************************************************************************************
*/
void eThread::queue(
    eEnvelope *envelope,
    os_boolean delete_envelope)
{
    if (delete_envelope) {
        envelope->adopt(m_message_queue, EOID_ITEM, EOBJ_NO_MAP);
    }
    else {
        envelope->clone(m_message_queue, EOID_ITEM, EOBJ_NO_MAP);
    }
    osal_event_set(m_trigger);
}


/**
****************************************************************************************************

  @brief Process messages.

  The alive function processed messages incoming to thread. It takes a message
  item at a time and and forwards those.

  @return None.

****************************************************************************************************
*/
void eThread::alive(
    os_int flags)
{
    eEnvelope *envelope;

    /* Wait for thread to be trigged. Always clear the event, even we would not be waiting.
     */
    osal_event_wait(m_trigger, flags & EALIVE_WAIT_FOR_EVENT
        ? OSAL_EVENT_INFINITE : OSAL_EVENT_NO_WAIT);

    while (!exitnow())
    {
        /* Synchronize and get message (envelope) from queue.
         */
        os_lock();
        envelope = eEnvelope::cast(m_message_queue->first());
        if (envelope)
        {
            envelope->adopt(this, EOID_CHILD, EOBJ_NO_MAP);

            /* Flag that envelope has been moved from thread to another.
             */
            envelope->addmflags(EMSG_INTERTHREAD);
        }
        os_unlock();

        /* If no message, do nothing more.
         */
        if (envelope == OS_NULL) return;

        /* Call message processing.
         */
        onmessage(envelope);

        /* Finished with envelope.
         */
        delete envelope;
    }
}
