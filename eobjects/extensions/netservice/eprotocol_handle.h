/**

  @file    eprotocolhandle.h
  @brief   Abstract communication protocol handle.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef EPROTOCOL_HANDLE_H_
#define EPROTOCOL_HANDLE_H_
#include "extensions/netservice/enetservice.h"



/**
****************************************************************************************************
  eProtocolHandle class.
****************************************************************************************************
*/
class eProtocolHandle : public eObject
{
public:
    /* Constructor.
     */
    eProtocolHandle(
        eObject *parent = OS_NULL,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT);

    /* Virtual destructor.
     */
    virtual ~eProtocolHandle();

    /* Clone object.
     */
    virtual eObject *clone(
        eObject *parent,
        e_oid id = EOID_CHILD,
        os_int aflags = 0);

    /* Casting eObject pointer to eProtocolHandle pointer.
     */
    inline static eProtocolHandle *cast(
        eObject *o)
    {
        e_assert_type(o, ECLASSID_PROTOCOL_HANDLE)
        return (eProtocolHandle*)o;
    }

    /* Get class identifier.
     */
    virtual os_int classid() {return ECLASSID_PROTOCOL_HANDLE; }

    /* Static function to add class to propertysets and class list.
     */
    static void setupclass();

    /* Static constructor function for generating instance by class list.
     */
    static eProtocolHandle *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new eProtocolHandle(parent, id, flags);
    }


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
