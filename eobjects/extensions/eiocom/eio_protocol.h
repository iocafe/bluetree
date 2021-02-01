/**

  @file    eio_protocol.h
  @brief   IOCOM protocol management.
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
#ifndef EIO_PROTOCOL_H_
#define EIO_PROTOCOL_H_
#include "extensions/netservice/enetservice.h"


/**
****************************************************************************************************
  Defines
****************************************************************************************************
*/



/**
****************************************************************************************************
  eioProtocol class.
****************************************************************************************************
*/
class eioProtocol : public eProtocol
{
public:
    /* Constructor.
     */
    eioProtocol(
        eObject *parent = OS_NULL,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT);

    /* Virtual destructor.
     */
    virtual ~eioProtocol();

    /* Casting eObject pointer to eioProtocol pointer.
     */
    inline static eioProtocol *cast(
        eObject *o)
    {
        e_assert_type(o, ECLASSID_EIO_PROTOCOL)
        return (eioProtocol*)o;
    }

    /* Get class identifier.
     */
    virtual os_int classid() {return ECLASSID_EIO_PROTOCOL; }

    /* Static function to add class to propertysets and class list.
     */
    static void setupclass();

    /* Static constructor function for generating instance by class list.
     */
    static eioProtocol *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new eioProtocol(parent, id, flags);
    }

    /**
    ************************************************************************************************
      Protocol related functions, derived ecom protocol class.
    ************************************************************************************************
    */

    /* Get protocol name.
     */
    virtual const os_char *protocol_name() {return "iocom"; }

    /* Initialize communication protocol
     */
    virtual eStatus initialize_protocol(
        class eNetService *netservice,
        void *parameters);

    /* Finished with communication protocol, clean up
     */
    virtual void shutdown_protocol();

    /* Create a new end point to listen for this protocol.
     */
    virtual eProtocolHandle *new_end_point(
        os_int ep_nr,
        eEndPointParameters *parameters,
        eStatus *s);

    /* Create a new connection using this protocol.
     */
    virtual eProtocolHandle *new_connection(
        eVariable *con_name,
        eConnectParameters *parameters,
        eStatus *s);

    /* Reactivate a deactivated connection or modify parameters.
     */
    virtual eStatus activate_connection(
        eProtocolHandle *handle,
        eConnectParameters *parameters);

    /* Deacivate a connection.
     */
    virtual void deactivate_connection(
        eProtocolHandle *handle);

protected:
    /**
    ************************************************************************************************
      Internal functions.
    ************************************************************************************************
    */

    void make_connect_parameter_string(
        eVariable *parameter_str,
        eConnectParameters *parameters);


    /**
    ************************************************************************************************
      Member variables
    ************************************************************************************************
    */

};


#endif
