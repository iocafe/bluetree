/**

  @file    ethread.h
  @brief   Thread class.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    10.10.2020

  The thread object is the root of thread's object tree.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef ETHREAD_H_
#define ETHREAD_H_
#include "eobjects.h"

/* Flags for alive() function.
 */
#define EALIVE_WAIT_FOR_EVENT 1
#define EALIVE_RETURN_IMMEDIATELY 0


/**
****************************************************************************************************
  eThread is top of the tread's object tree.
****************************************************************************************************
*/
class eThread : public eObject
{
public:
    /**
    ************************************************************************************************
      Generic eObject functionality.
    ************************************************************************************************
    */
    /* Constructor.
     */
    eThread(
        eObject *parent = OS_NULL,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT);

    /* Virtual destructor.
     */
    virtual ~eThread();

    /* Casting eObject pointer to eThread pointer.
        */
    inline static eThread *cast(
        eObject *o)
    {
        if (o)
        {
            if (o->isthread()) return (eThread*)o;
            osal_debug_assert(0);
        }
        return OS_NULL;
    }

    /* Get class identifier.
     */
    virtual os_int classid() {return ECLASSID_THREAD; }

    /* Static function to add class to propertysets and class list.
     */
    static void setupclass();

    /* Return OS_TRUE if object is thread (derived).
     */
    virtual os_boolean isthread()
    {
        return OS_TRUE;
    }

    /* Static constructor function for generating instance by class list.
     */
    static eThread *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new eThread(parent, id, flags);
    }

    virtual void onmessage(
        eEnvelope *envelope);

    inline osalEvent trigger() {return m_trigger;}


    /**
    ************************************************************************************************
      Virtual functions to overload in derived thread class.
    ************************************************************************************************
    */

    /* Overload this function to perform thread specific initialization when thread starts.
       This is called when a thread is started by start() function. Start function does not
       return (waits for event) until initialize returns.
     */
    virtual void initialize(
        eContainer *params = OS_NULL) {}

    /* Default run function, which just calls alive (waits for thread events).
       This function can be overloaded, for example to wait for received data from
       socket. Timer mechanism should be used to implement periodic functionality
       instead of overloading run for this purpose.
     */
    virtual void run();

    /* Overload this function to perform thread specific cleanup when thread exists. This
       is a "pair" to initialize function.
     */
    virtual void finish() {}


    /**
    ************************************************************************************************
      Thread class functions
    ************************************************************************************************
    */
    /* Create operating system thread to run the the eThread. T
     */
    void start(
        eThreadHandle *thandle = OS_NULL,
        eContainer *params = OS_NULL);

    /* Check if thread exit is requested. This can be called from derived thread class
       implementetion to check if thread exit has been requested.
     */
    inline os_boolean exitnow()
    {
        return m_exit_requested || !osal_go();
    }

    /* Get next message to thread to process.
     */
    void queue(
        eEnvelope *envelope,
        os_boolean delete_envelope = OS_TRUE);

    /* Check for messages received by the thread and pass these on as onmessage() calls
       to objects.
     */
    void alive(
        os_int flags = EALIVE_WAIT_FOR_EVENT);


protected:

    /**
    ************************************************************************************************
      Member variables.
    ************************************************************************************************
    */

    /* Thread triggger.
     */
    osalEvent m_trigger;

    /* Message queue for incoming messages.
     */
    eContainer *m_message_queue;

    /* Exit requested
     */
    os_boolean m_exit_requested;
};

#endif
