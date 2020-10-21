/**

  @file    esynchronized.h
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
#pragma once
#ifndef ESYNCHRONIZED_H_
#define ESYNCHRONIZED_H_
#include "eobjects.h"


/**
****************************************************************************************************

  @brief Table binding class.

  The eSynchronized is class derived from eBinding. It implements property binding specific
  functionality.


****************************************************************************************************
*/
class eSynchronized : public eObject
{
    /**
    ************************************************************************************************

      @name Generic object functionality.

      These functions enable using objects of this class as generic eObjects.

    ************************************************************************************************
    */
public:
    /* Constructor.
     */
    eSynchronized(
        eObject *parent = OS_NULL,
        e_oid id = EOID_RITEM,
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

    /* Process received messages
     */
    virtual void onmessage(
        eEnvelope *envelope);


    /**
    ************************************************************************************************

      @name Property binding functions

      These functions implement property finding functionality.

    ************************************************************************************************
    */


    /**
    ************************************************************************************************

      @name Synchchronized

    ************************************************************************************************
    */

    void initialize_synch_transfer(
        const os_char *path);

    void finish_sync_transfer();

    /* Send message.
     */
    eStatus synch_send(
        eEnvelope *envelope);

    eStatus sync_receive(
        eEnvelope *envelope);

    /* Get number of messages send minus number of messages received.
     */
    os_int sync_in_air_count();

    /* Wait until "in air count" is less or equal than argument.
     */
    eStatus sync_wait(
        os_int in_air_count,
        os_long timeout_ms);

protected:


    /**
    ************************************************************************************************

      @name Member variables.

      The member variables hold information where to bind (for client binding) and current
      binding state.

    ************************************************************************************************
    */

    eVariable m_path;

    osalEvent m_event;
};

#endif
