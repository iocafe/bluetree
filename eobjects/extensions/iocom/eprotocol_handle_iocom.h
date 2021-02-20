/**

  @file    eprotocol_handle_iocom.h
  @brief   IOCOM protocol handle.
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
#ifndef EPROTOCOL_HANDLE_IOCOM_H_
#define EPROTOCOL_HANDLE_IOCOM_H_
#include "extensions/netservice/enetservice.h"

/**
****************************************************************************************************
  Defines
****************************************************************************************************
*/

typedef union {
    iocConnection con;
    iocEndPoint epoint;
}
eiocomStateStruct;



/**
****************************************************************************************************
  eioProtocolHandle class.
****************************************************************************************************
*/
class eioProtocolHandle : public eProtocolHandle
{
public:
    /* Constructor.
     */
    eioProtocolHandle(
        eObject *parent = OS_NULL,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT);

    /* Get class identifier.
     */
    virtual os_int classid() {return ECLASSID_IOCOM_PROTOCOL_HANDLE; }

    /* Static function to add class to propertysets and class list.
     */
    static void setupclass();

    /* Static constructor function for generating instance by class list.
     */
    static eioProtocolHandle *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new eioProtocolHandle(parent, id, flags);
    }

    /**
    ************************************************************************************************
      Protocol handle functions.
    ************************************************************************************************
    */

    virtual os_boolean isrunning() {return m_is_open; }
    inline iocEndPoint *epoint() {return &m_iocom.epoint; }
    inline iocConnection *con() {return &m_iocom.con; }

    inline void mark_iocom_end_point(os_boolean is_iocom_end_point)
        {m_is_iocom_end_point = is_iocom_end_point; }

    inline os_boolean is_iocom_end_point()
        {return m_is_iocom_end_point; }


protected:

    /**
    ************************************************************************************************
      Member variables
    ************************************************************************************************
    */
    eiocomStateStruct m_iocom;
    // os_boolean m_is_open;
    os_boolean m_is_iocom_end_point;

};


#endif
