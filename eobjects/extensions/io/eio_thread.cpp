/**

  @file    eio_thread.cpp
  @brief   Thread to run the IO.
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
eioThread::eioThread(
    eObject *parent,
    e_oid oid,
    os_int flags)
    : eThread(parent, oid, flags)
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
void eioThread::setupclass()
{
    const os_int cls = ECLASSID_EIO_THREAD;

    /* Add the class to class list.
     */
    os_lock();
    eclasslist_add(cls, (eNewObjFunc)OS_NULL, "eioThread");
    os_unlock();
}


/**
****************************************************************************************************

  @brief Function to process incoming messages.

  The eioThread::onmessage function handles messages received by object. If this function
  doesn't process message, it calls parent class'es onmessage function.

  Send UDP broadcasts by timer hit.

  @param   envelope Message envelope. Contains command, target and source paths and
           message content, etc.
  @return  None.

****************************************************************************************************
*/
void eioThread::onmessage(
    eEnvelope *envelope)
{
    /* If at final destination for the message.
     */
    if (*envelope->target()=='\0')
    {
        switch (envelope->command())
        {
            case ECMD_TIMER: /* No need to do anything, timer is used just to break event wait */
                return;

            default:
                break;
        }
    }

    /* Call parent class'es onmessage.
     */
    eThread::onmessage(envelope);
}


/**
****************************************************************************************************

  @brief Maintain connections and end points, thread main loop.

  The eioThread::run() function
  The data is collected from "connect" and "endpoint" tables, both these tables are global
  and owned by eProcess.

****************************************************************************************************
*/
void eioThread::run()
{
    timer(2000);

    while (OS_TRUE)
    {
        alive();
        if (exitnow()) {
            break;
        }

        ioc_receive_all(m_iocom_root);
        ioc_send_all(m_iocom_root);

osal_debug_error("HERE IO THREAD LOOP")        ;
    }
}


/**
****************************************************************************************************

  @brief Start IO thread.

****************************************************************************************************
*/
void eio_start_thread(
    eioRoot *eio_root,
    eThreadHandle *io_thread_handle)
{
    eioThread *t;

    /* Create and start thread to listen for t UDP multicasts,
       name it "_t" in process name space.
     */
    t = new eioThread();
    t->addname("//_iothread");

    t->set_iocom_root(eio_root->iocom_root());
    t->start(io_thread_handle);
}

