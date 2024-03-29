/**

  @file    eprotocol_handle_ecom.h
  @brief   ECOM communication protocol handle.
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
#ifndef EPROTOCOL_HANDLE_ECOM_H_
#define EPROTOCOL_HANDLE_ECOM_H_
#include "extensions/netservice/enetservice.h"

/**
****************************************************************************************************
  Defines
****************************************************************************************************
*/



/**
****************************************************************************************************
  ecomProtocolHandle class.
****************************************************************************************************
*/
class ecomProtocolHandle : public eProtocolHandle
{
public:
    /* Constructor.
     */
    ecomProtocolHandle(
        eObject *parent = OS_NULL,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT);

    /* Virtual destructor.
     */
    virtual ~ecomProtocolHandle();

    /* Get class identifier.
     */
    virtual os_int classid() {return ECLASSID_ECOM_PROTOCOL_HANDLE; }

    /* Static function to add class to propertysets and class list.
     */
    static void setupclass();

    /* Static constructor function for generating instance by class list.
     */
    static ecomProtocolHandle *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new ecomProtocolHandle(parent, id, flags);
    }


    /**
    ************************************************************************************************
      Protocol handle functions.
    ************************************************************************************************
    */

    /* Start a connection or end point thread.
     */
    void start_thread(
        eThread *t,
        const os_char *threadname);

    /* Terminate connection or end point thread.
     */
    void terminate_thread();

    /* Get unique name of a connection or end point.
     */
    const os_char *uniquename();

    /* Check if connection or end point is running.
     */
    virtual os_boolean started();


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

    /** Thread handle. Connections and end points typically run in their own thread.
        OS_NULL if thread has not been started.
     */
    eThreadHandle *m_threadhandle;

    /** Name of commection or end point in process name space.
     */
    eVariable *m_threadname;
};


#endif
