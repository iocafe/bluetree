/**

  @file    eprotocol.h
  @brief   Abstract communication protocol as seen by eNetService.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  Abstract communication protocol interface is used by eNetService to manage end points and
  connections. This is the base class, protocol specific derived class will map eNetService
  calls like "create end point" to communication library functions.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef EPROTOCOL_H_
#define EPROTOCOL_H_
#include "extensions/netservice/enetservice.h"


/**
****************************************************************************************************
  Defines
****************************************************************************************************
*/

/* Property numbers.
 */
#define EPROTOCOLP_PATH 10

/* Property names.
 */
extern const os_char
    eprotocolp_path[];


/**
****************************************************************************************************
  eProtocol class.
****************************************************************************************************
*/
class eProtocol : public eObject
{
public:
    /* Constructor.
     */
    eProtocol(
        eObject *parent = OS_NULL,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT);

    /* Virtual destructor.
     */
    virtual ~eProtocol();

    /* Casting eObject pointer to eProtocol pointer.
     */
    inline static eProtocol *cast(
        eObject *o)
    {
        e_assert_type(o, ECLASSID_PROTOCOL)
        return (eProtocol*)o;
    }

    /* Get class identifier.
     */
    virtual os_int classid() {return ECLASSID_PROTOCOL; }

    /* Static function to add class to propertysets and class list.
     */
    static void setupclass();

    /* Static constructor function for generating instance by class list.
     */
    static eProtocol *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new eProtocol(parent, id, flags);
    }

    /* Called when property value changes.
     */
    virtual eStatus onpropertychange(
        os_int propertynr,
        eVariable *x,
        os_int flags);

    /**
    ************************************************************************************************
      Base class protocol related functions.
    ************************************************************************************************
    */
    virtual eStatus initialize_protocol(
        void *parameters);

    virtual void shutdown_protocol();

    virtual eStatus new_end_point(
        void *parameters);

    virtual eStatus new_connection(
        void *parameters);

protected:
    /**
    ************************************************************************************************
      Internal functions.
    ************************************************************************************************
    */


    /**
    ************************************************************************************************
      Member variables
    ************************************************************************************************
    */

};


#endif
