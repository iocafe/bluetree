/**

  @file    eprotocol_ecom.h
  @brief   The eobjects library communication protocol management, serialized objects.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  Related to: Network connnection and protocol management interface.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef EPROTOCOL_COM_H_
#define EPROTOCOL_COM_H_
#include "extensions/netservice/enetservice.h"


/**
****************************************************************************************************
  Defines
****************************************************************************************************
*/



/**
****************************************************************************************************
  eComProtocol class.
****************************************************************************************************
*/
class eComProtocol : public eProtocol
{
public:
    /* Constructor.
     */
    eComProtocol(
        eObject *parent = OS_NULL,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT);

    /* Virtual destructor.
     */
    virtual ~eComProtocol();

    /* Casting eObject pointer to eComProtocol pointer.
     */
    inline static eComProtocol *cast(
        eObject *o)
    {
        e_assert_type(o, ECLASSID_ECOM_PROTOCOL)
        return (eComProtocol*)o;
    }

    /* Get class identifier.
     */
    virtual os_int classid() {return ECLASSID_ECOM_PROTOCOL; }

    /* Static function to add class to propertysets and class list.
     */
    static void setupclass();

    /* Static constructor function for generating instance by class list.
     */
    static eComProtocol *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new eComProtocol(parent, id, flags);
    }

    /**
    ************************************************************************************************
      Protocol related functions, derived ecom protocol class.
    ************************************************************************************************
    */

    /* Get protocol name.
     */
    virtual const os_char *protocol_name() {return "ecom"; }

    /* Initialize communication protocol
     */
    virtual eStatus initialize_protocol(
        void *parameters);

    /* Finished with communication protocol, clean up
     */
    virtual void shutdown_protocol();

    /* Create a new end point to listen for this protocol.
     */
    virtual eProtocolHandle *new_end_point(
        os_int ep_nr,
        void *parameters,
        eStatus *s);

    /* Create a new connection using this protocol.
     */
    virtual eProtocolHandle *new_connection(
        os_int conn_nr,
        void *parameters,
        eStatus *s);


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
