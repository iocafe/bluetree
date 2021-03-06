/**

  @file    eendpoint.h
  @brief   End point class.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    14.9.2020

  The eEndPoint is socket end point listening to specific TCP port for new connections.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef EENDPOINT_H_
#define EENDPOINT_H_
#include "eobjects.h"


/**
****************************************************************************************************
  Defines
****************************************************************************************************
*/

/* Enumeration of end point properties.
 */
#define EENDPP_CLASSID 2
#define EENDPP_IPADDR  4
#define EENDPP_ISOPEN  6

/* End point property names.
 */
extern const os_char
    eendpp_classid[],
    eendpp_ipaddr[],
    eendpp_isopen[];


/**
****************************************************************************************************

  @brief End point class.

  The eEndPoint is socket end point listening to specific TCP port for new connections.

****************************************************************************************************
*/
class eEndPoint : public eThread
{
public:
    /* Constructor.
     */
    eEndPoint(
        eObject *parent = OS_NULL,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT);

    /* Virtual destructor.
     */
    virtual ~eEndPoint();

    /* Casting eObject pointer to eEndPoint pointer.
     */
    inline static eEndPoint *cast(
        eObject *o)
    {
        e_assert_type(o, ECLASSID_ENDPOINT)
        return (eEndPoint*)o;
    }

    /* Get class identifier.
     */
    virtual os_int classid() {return ECLASSID_ENDPOINT; }

    /* Static function to add class to propertysets and class list.
     */
    static void setupclass();

    /* Static constructor function.
    */
    static eEndPoint *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new eEndPoint(parent, id, flags);
    }

    /* Called when property value changes.
     */
    virtual eStatus onpropertychange(
        os_int propertynr,
        eVariable *x,
        os_int flags);

    /* Get value of simple property.
     */
    virtual eStatus simpleproperty(
        os_int propertynr,
        eVariable *x);

    /* Initialize the object.
     */
    virtual void initialize(
        eContainer *params = OS_NULL);

    /* Run the end point: main loop to process thread events and accept connections.
     */
    virtual void run();

protected:

    /**
    ************************************************************************************************
      Protected member functions.
    ************************************************************************************************
    */

    /* Try to start listening socket port.
     */
    void open();

    /* Close the listening socket.
     */
    void close();


    /**
    ************************************************************************************************
      Member variables.
    ************************************************************************************************
    */

    /** Stream class identifier. Specifies stream class to use.
     */
    os_int m_stream_classid;

    /** IP address of the interface and port number to listen to.
        IP address can be empty to listen for any interface.
     */
    eVariable *m_ipaddr;

    /** Listening stream handle. OS_NULL if listening socket
        is not open.
     */
    eStream *m_stream;

    /** Accepted connection count.
     */
    os_long m_accept_count;

    /** End point object initailized flag.
     */
    os_boolean m_initialized;

    /** End point has been successfully opened.
     */
    os_boolean m_isopen;

    /** We tried to open socket port and failed.
     */
    os_boolean m_open_failed;
};

#endif
