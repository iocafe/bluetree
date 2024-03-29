/**

  @file    eprotocol_switchbox.h
  @brief   SWITCHBOX protocol management.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

  Related to: Network connnection and protocol management interface.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef ESBOX_PROTOCOL_H_
#define ESBOX_PROTOCOL_H_
#include "extensions/netservice/enetservice.h"


/**
****************************************************************************************************
  Defines
****************************************************************************************************
*/



/**
****************************************************************************************************
  esboxProtocol class.
****************************************************************************************************
*/
class esboxProtocol : public eProtocol
{
public:
    /* Constructor.
     */
    esboxProtocol(
        eObject *parent = OS_NULL,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT);

    /* Virtual destructor.
     */
    virtual ~esboxProtocol();

    /* Casting eObject pointer to esboxProtocol pointer.
     */
    inline static esboxProtocol *cast(
        eObject *o)
    {
        e_assert_type(o, ECLASSID_SWITCHBOX_PROTOCOL)
        return (esboxProtocol*)o;
    }

    /* Get class identifier.
     */
    virtual os_int classid() {return ECLASSID_SWITCHBOX_PROTOCOL; }

    /* Static function to add class to propertysets and class list.
     */
    static void setupclass();

    /* Static constructor function for generating instance by class list.
     */
    static esboxProtocol *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new esboxProtocol(parent, id, flags);
    }

    /**
    ************************************************************************************************
      Protocol related functions, derived ecom protocol class.
    ************************************************************************************************
    */

    /* Get protocol name.
     */
    virtual const os_char *protocol_name() {return "ioswitchbox"; }
    virtual const os_char *protocol_name2() {return "eswitchbox"; }

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

    /* Delete an end point.
     */
    virtual void delete_end_point(
        eProtocolHandle *handle);

    /* Create a new connection using this protocol.
     */
    virtual eProtocolHandle *new_connection(
        eVariable *con_name,
        eConnectParameters *parameters,
        eStatus *s);

    /* Delete a connection.
     */
    virtual void delete_connection(
        eProtocolHandle *handle);

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

    /* Helper function for new_connection() and new_end_point().
    eProtocolHandle *new_con_helper(
        const os_char *prmstr,
        const osalStreamInterface *iface,
        os_short cflags,
        eStatus *s);
     */


    /**
    ************************************************************************************************
      Member variables
    ************************************************************************************************
    */
};


#endif
