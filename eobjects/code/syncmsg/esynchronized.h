/**

  @file    esynchronized.h
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
#pragma once
#ifndef ESYNCHRONIZED_H_
#define ESYNCHRONIZED_H_
#include "eobjects.h"

/**
****************************************************************************************************
  Synchronized data transfer object for application.
****************************************************************************************************
*/
class eSynchronized : public eObject
{
public:

    /**
    ************************************************************************************************
      Generic eObject functionality.
    ************************************************************************************************
    */
    /* Constructor.
     */
    eSynchronized(
        eObject *parent = OS_NULL,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT);

    /* Virtual destructor.
     */
    virtual ~eSynchronized();

    /* Casting eObject pointer to eSynchronized pointer.
     */
    inline static eSynchronized *cast(
        eObject *o)
    {
        e_assert_type(o, ECLASSID_SYNCHRONIZED)
        return (eSynchronized*)o;
    }

    /* Get class identifier.
     */
    virtual os_int classid() {return ECLASSID_SYNCHRONIZED; }

    /* Static function to add class to propertysets and class list.
     */
    static void setupclass();

    /* Static constructor function for generating instance by class list.
     */
    static eSynchronized *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new eSynchronized(parent, id, flags);
    }

    /* Get value of simple property.
     */
    virtual eStatus simpleproperty(
        os_int propertynr,
        eVariable *x);


    /**
    ************************************************************************************************
      Synchronized data transfer functions.
    ************************************************************************************************
    */
    /* Initialize eSynchronized for data transfer.
     */
    void initialize_synch_transfer(
        const os_char *path);

    /* Finished with synchronized transfer object, clean up.
     */
    void finish_sync_transfer(
        os_boolean abort);

    /* Send message with syncronization.
     */
    eStatus synch_send(
        eEnvelope *envelope);

    /* Check for received reply messages.
     */
    eEnvelope *sync_receive(
        eObject *parent);

    /* Get number of messages send minus number of messages received.
     */
    os_int in_air_count();

    /* Wait until "in air count" is less or equal than argument.
     */
    eStatus sync_wait(
        os_int in_air_count,
        os_long timeout_ms);

protected:

    /**
    ************************************************************************************************
      Member variables.
    ************************************************************************************************
    */
    eVariable *m_path;
    ePointer *m_ref;
    osalEvent m_event;
};

#endif
