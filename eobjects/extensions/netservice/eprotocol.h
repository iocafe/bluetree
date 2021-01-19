/**

  @file    eprotocol.h
  @brief   Abstract communication protocol as seen by eNetService.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  Related to: Network connnection and protocol management interface.

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
typedef struct
{
    enetEndpTransportIx transport;

    const os_char *port;
}
eEndPointParameters;


typedef struct
{
    enetConnTransportIx transport;

    const os_char *parameters;
}
eConnectParameters;

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

    /**
    ************************************************************************************************
      Protocol related functions, base class.
    ************************************************************************************************
    */

    /* Get protocol name.
     */
    virtual const os_char *protocol_name() {return "none"; }

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
        eEndPointParameters *parameters,
        eStatus *s);

    /* Delete an end point.
     */
    virtual void delete_end_pont(
        eProtocolHandle *handle);

    /* Check end point status (running).
     */
    virtual os_boolean is_end_point_running(
        eProtocolHandle *handle);

    /* Create a new connection using this protocol.
     */
    virtual eProtocolHandle *new_connection(
        os_int conn_nr,
        eConnectParameters *parameters,
        eStatus *s);

    /* Delete a connection.
     */
    virtual void delete_connection(
        eProtocolHandle *handle);

    /* Check connection status.
     */
    virtual os_boolean is_connection_running(
        eProtocolHandle *handle);

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
