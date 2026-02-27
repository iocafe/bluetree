/**

  @file    eprotocol_handle_switchbox.h
  @brief   SWITCHBOX protocol handle.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef EPROTOCOL_HANDLE_SWITCHBOX_H_
#define EPROTOCOL_HANDLE_SWITCHBOX_H_
#include "extensions/netservice/enetservice.h"

/**
****************************************************************************************************
  Defines
****************************************************************************************************
*/


/**
****************************************************************************************************
  esboxProtocolHandle class.
****************************************************************************************************
*/
class esboxProtocolHandle : public eProtocolHandle
{
public:
    /* Constructor.
     */
    esboxProtocolHandle(
        eObject *parent = OS_NULL,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT);

    /* Destructor.
     */
    ~esboxProtocolHandle();

    /* Get class identifier.
     */
    virtual os_int classid() {return ECLASSID_SWITCHBOX_PROTOCOL_HANDLE; }

    /* Static function to add class to propertysets and class list.
     */
    static void setupclass();

    /* Static constructor function for generating instance by class list.
     */
    static esboxProtocolHandle *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new esboxProtocolHandle(parent, id, flags);
    }

    /**
    ************************************************************************************************
      Protocol handle functions.
    ************************************************************************************************
    */

    virtual os_boolean started() {return m_is_open; } // ??????????????????????????????? DO NOT RETURN THIS, RETURN IF COMMUNICATION HAS BEEN SET UP

    eStatus listen(
        switchboxEndPointParams *prm);

    void close_endpoint();

    inline const os_char *path_to_self() { return m_path_to_self; }


protected:

    /* Callback when an end point is actually listening, or dropped.
     */
    static void end_point_callback(
        struct switchboxEndPoint *epoint,
        switchboxEndPointEvent event,
        void *context);

    /**
    ************************************************************************************************
      Member variables
    ************************************************************************************************
    */
    switchboxRoot m_switchbox;
    switchboxEndPoint m_epoint;
    os_boolean m_end_point_initialized;
    os_char m_path_to_self[E_OIXSTR_BUF_SZ];
};


#endif
