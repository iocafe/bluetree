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
  Defines
****************************************************************************************************
*/

/* Property numbers.
 */
#define EPROHANDP_ISOPEN 10

/* Property names.
 */
extern const os_char
    eprohandp_isopen[];


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
    os_boolean isrunning();


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
