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

    /* Virtual destructor.
     */
    virtual ~eioProtocolHandle();

    /* Casting eObject pointer to eioProtocolHandle pointer.
     */
    inline static eioProtocolHandle *cast(
        eObject *o)
    {
        e_assert_type(o, ECLASSID_IOCOM_PROTOCOL_HANDLE)
        return (eioProtocolHandle*)o;
    }

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

    /* Called when property value changes.
     */
    virtual eStatus onpropertychange(
        os_int propertynr,
        eVariable *x,
        os_int flags);


    /**
    ************************************************************************************************
      Protocol handle functions.
    ************************************************************************************************
    */

    /* Start a connection or end point thread.
     */
    virtual void start_thread(
        eThread *t,
        const os_char *threadname);

    /* Terminate connection or end point thread.
     */
    virtual void terminate_thread();

    /* Get unique name of a connection or end point.
     */
    virtual const os_char *uniquename();

    /* Check if connection or end point is running.
     */
    virtual os_boolean isrunning();


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

    /** Communication channel open flag.
     */
    os_boolean m_is_open;
};


#endif
