/**

  @file    eacknowledge.h
  @brief   Flow control of potentially large data amounts.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  When transferring large amount of data, it is sometimes necessary to divide the data
  into pieces and transfer these as received. Typically thread sending the data is
  in loop to collect the data, and should not process messages.

  To make this work, an intermediate eAcknowledge object created under eProcess.
  Data will be sent and received data acknowledged by this object. This provides
  flow controlled data transfer.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef EACKNOWLEDGE_H_
#define EACKNOWLEDGE_H_
#include "eobjects.h"


/**
****************************************************************************************************

  @brief Table binding class.

  The eAcknowledge is class derived from eBinding. It implements property binding specific
  functionality.


****************************************************************************************************
*/
class eAcknowledge : public eObject
{
    /**
    ************************************************************************************************

      @name Generic object functionality.

      These functions enable using objects of this class as generic eObjects.

    ************************************************************************************************
    */
    /*@{*/
public:
    /* Constructor.
     */
    eAcknowledge(
        eObject *parent = OS_NULL,
        e_oid id = EOID_RITEM,
        os_int flags = EOBJ_DEFAULT);

    /* Virtual destructor.
     */
    virtual ~eAcknowledge();

    /* Casting eObject pointer to eAcknowledge pointer.
     */
    inline static eAcknowledge *cast(
        eObject *o)
    {
        e_assert_type(o, ECLASSID_ACKNOWLEDGE)
        return (eAcknowledge*)o;
    }

    /* Get class identifier.
     */
    virtual os_int classid() {return ECLASSID_ACKNOWLEDGE; }

    /* Static function to add class to propertysets and class list.
     */
    static void setupclass();

    /* Static constructor function for generating instance by class list.
     */
    static eAcknowledge *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new eAcknowledge(parent, id, flags);
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

    /*@}*/


    /**
    ************************************************************************************************

      @name Property binding functions

      These functions implement property finding functionality.

    ************************************************************************************************
    */
    /*@{*/


    /**
    ************************************************************************************************

      @name Global acknowledge functions

    ************************************************************************************************
    */

    /* static os_long start_controlled_transfer();
    static os_long send(envelpoe);
    static os_long receive(envelpoe);
    static os_long unacknowledged_count(envelpoe);
    static os_long finish_controlled_transfer(envelpoe);
    */

protected:

    /*@}*/

    /**
    ************************************************************************************************

      @name Member variables.

      The member variables hold information where to bind (for client binding) and current
      binding state.

    ************************************************************************************************
    */
    /*@{*/





    /*@}*/




};

#endif
