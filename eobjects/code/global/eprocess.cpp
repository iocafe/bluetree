/**

  @file    eprocess.cpp
  @brief   eProcess class implementation.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "eobjects.h"


/**
****************************************************************************************************

  @brief Constructor.

  X...

  @return  None.

****************************************************************************************************
*/
eProcess::eProcess(
    eObject *parent,
    e_oid id,
    os_int flags)
    : eThread(parent, id, flags)
{
    eglobal->process_ns = new eNameSpace(this, EOID_NAMESPACE);

    m_sync_connectors = new eContainer(this);
    m_sync_connectors->addname("sync_connectors", ENAME_NO_MAP);
}


/**
****************************************************************************************************

  @brief Virtual destructor.

  X...

  @return  None.

****************************************************************************************************
*/
eProcess::~eProcess()
{
}


/**
****************************************************************************************************

  @brief Add eProcess to class list and class'es properties to it's property set.

  The eProcess::setupclass function adds eThread to class list and class'es properties to
  it's property set. The class list enables creating new objects dynamically by class identifier,
  which is used for serialization reader functions. The property set stores static list of
  class'es properties and metadata for those.

****************************************************************************************************
*/
void eProcess::setupclass()
{
    const os_int cls = ECLASSID_PROCESS;

    /* Add the class to class list.
     */
    os_lock();
    eclasslist_add(cls, OS_NULL, "eProcess", ECLASSID_THREAD);
    os_unlock();
}


void eProcess::initialize(eContainer *params)
{
    osal_console_write("initializing process\n");
}

void eProcess::run()
{
    while (!exitnow())
    {
        /* Wait for thread to be trigged.
         */
        osal_event_wait(trigger(), OSAL_EVENT_INFINITE);

        /* Process messages
         */
        os_lock();
        alive(EALIVE_RETURN_IMMEDIATELY);
        os_unlock();
    }
}

void eProcess::onmessage(
    eEnvelope *envelope)
{
    eThread::onmessage(envelope);
}

/* Get pointer to eSyncConnector objects for synchronized data transfers.
 */
eContainer *eProcess::sync_connectors()
{
    return eglobal->process->m_sync_connectors;
}


/**
****************************************************************************************************

  @brief Create eProcess object and start  thread to run it.

  The eprocess_create() function...

  @return  None.

****************************************************************************************************
*/
void eprocess_create()
{
    eProcess
        *process;

    eTimer
        *tim;

    eThreadHandle
        *processhandle,
        *timerhandle;

    if (eglobal->processhandle == OS_NULL)
    {
        /* Create process object and start thread to run it. After this process pointer is useless.
         */
        process = new eProcess();
        processhandle = new eThreadHandle();
        process->start(processhandle);

        /* Create thread which runs timers. After this tim pointer is useless.
         */
        tim = new eTimer();
        timerhandle = new eThreadHandle();
        tim->start(timerhandle);

        /* Add as global process only when process has been created.
         */
        os_lock();
        eglobal->process = process;
        eglobal->processhandle = processhandle;
        eglobal->timerhandle = timerhandle;
        os_unlock();
    }
}


/**
****************************************************************************************************

  @brief Terminate eProcess thread and clean up.

  The eprocess_close() function...

  @return  None.

****************************************************************************************************
*/
void eprocess_close()
{
    if (eglobal->processhandle)
    {
        /* Request timer thread to exit and wait for thread to terminate.
         */
        eglobal->timerhandle->terminate();
        eglobal->timerhandle->join();

        /* Request process thread to exit and wait for thread to terminate.
         */
        eglobal->processhandle->terminate();
        eglobal->processhandle->join();

        eglobal->processhandle = OS_NULL;
    }
}


