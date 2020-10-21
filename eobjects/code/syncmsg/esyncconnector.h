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

  @brief Table binding class.

  The eSyncConnector is class derived from eBinding. It implements property binding specific
  functionality.


****************************************************************************************************
*/
class eSyncConnector : public eObject
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
    eSyncConnector(
        eObject *parent = OS_NULL,
        e_oid id = EOID_RITEM,
        os_int flags = EOBJ_DEFAULT);

    /* Virtual destructor.
     */
    virtual ~eSyncConnector();

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

      @name functions

    ************************************************************************************************
    */

    /* Get number of unacknowledged messages (number of messages send - number of messages received)
     */
    inline os_int in_air_count() {return m_in_air_count;}

    /* Increment "in air count", called when message has been sent.
     */
    inline void increment_in_air_count() {m_in_air_count++;}


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

    os_int m_in_air_count;


};

#endif