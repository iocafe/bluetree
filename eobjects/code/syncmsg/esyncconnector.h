/**

  @file    esyncconnector.h
  @brief   Synchronized transfer helper object in eProcess tree.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    21.10.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef ESYNCCONNECTOR_H_
#define ESYNCCONNECTOR_H_
#include "eobjects.h"


/**
****************************************************************************************************
  Synchronized connector in process'es memory tree.
****************************************************************************************************
*/
class eSyncConnector : public eObject
{
public:
    /**
    ************************************************************************************************
      Generic eObject functionality.
    ************************************************************************************************
    */
    /* Constructor.
     */
    eSyncConnector(
        eObject *parent = OS_NULL,
        e_oid id = EOID_RITEM,
        os_int flags = EOBJ_DEFAULT);

    /* Casting eObject pointer to eSyncConnector pointer.
     */
    inline static eSyncConnector *cast(
        eObject *o)
    {
        e_assert_type(o, ECLASSID_SYNC_CONNECTOR)
        return (eSyncConnector*)o;
    }

    /* Get class identifier.
     */
    virtual os_int classid() {return ECLASSID_SYNC_CONNECTOR; }

    /* Static function to add class to propertysets and class list.
     */
    static void setupclass();

    /* Static constructor function for generating instance by class list.
     */
    static eSyncConnector *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new eSyncConnector(parent, id, flags);
    }

    /* Get value of simple property.
     */
    virtual eStatus simpleproperty(
        os_int propertynr,
        eVariable *x);

    /* Process received messages
     */
    virtual void onmessage(
        eEnvelope *envelope);


    /**
    ************************************************************************************************
      Synchronized data transfer connector functions
    ************************************************************************************************
    */
    /* Set synchronization event.
     */
    void set_sync_event(osalEvent e) {m_event = e;}

    /* Get number of unacknowledged messages (number of messages send - number of messages received)
     */
    inline os_int in_air_count() {return m_in_air_count;}

    /* Increment "in air count", called when message has been sent.
     */
    inline void increment_in_air_count() {m_in_air_count++;}

    /* Check if this operation has failed.
     */
    inline os_boolean failed() {return m_failed;}

    eStatus send_message(
        eEnvelope *envelope);

    /* Check for received reply messages.
     */
    eEnvelope *get_received_message(
        eObject *parent);

protected:

    /**
    ************************************************************************************************
      Member variables.
    ************************************************************************************************
    */
    osalEvent m_event;
    eContainer *m_queue;
    volatile os_int m_in_air_count;
    volatile os_boolean m_failed;
    eVariable *m_context;
};

#endif
