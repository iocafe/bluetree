/**

  @file    emaintain_thread.h
  @brief   Thread to maintain end points and connections.
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
#ifndef ENETMAINTAIN_THREAD_H_
#define ENETMAINTAIN_THREAD_H_
#include "extensions/netservice/enetservice.h"

class eNetService;

/**
****************************************************************************************************
  Defines, etc.
****************************************************************************************************
*/

/* Property numbers.
 */
#define ENETMAINTAINP_PUBLISH 10

/* Property names.
 */
extern const os_char
    enetmaintainp_publish[];


/**
****************************************************************************************************
  eNetMaintainThread class.
****************************************************************************************************
*/
class eNetMaintainThread : public eThread
{
public:
    /* Constructor.
     */
    eNetMaintainThread(
        eObject *parent = OS_NULL,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT);

    /* Virtual destructor.
     */
    virtual ~eNetMaintainThread();

    /* Casting eObject pointer to eNetMaintainThread pointer.
     */
    inline static eNetMaintainThread *cast(
        eObject *o)
    {
        e_assert_type(o, ECLASSID_NET_MAINTAIN_CLIENT)
        return (eNetMaintainThread*)o;
    }

    /* Get class identifier.
     */
    virtual os_int classid() {return ECLASSID_NET_MAINTAIN_CLIENT; }

    /* Static function to add class to propertysets and class list.
     */
    static void setupclass();

    /* Static constructor function for generating instance by class list.
     */
    static eNetMaintainThread *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new eNetMaintainThread(parent, id, flags);
    }

    /* Function to process incoming messages.
     */
    virtual void onmessage(
        eEnvelope *envelope);

    /* Called when property value changes.
     */
    virtual eStatus onpropertychange(
        os_int propertynr,
        eVariable *x,
        os_int flags);

    /* Set pointer to network service (eNetService is owned by eProcess, oe_lock() must
       be on to access.
     */
    inline void set_netservice(
        eNetService *netservice)
    {
        m_netservice = netservice;
    }

    /* Overloaded eThread function to initialize new thread.
     */
    virtual void initialize(
        eContainer *params = OS_NULL);

    /* Overloaded eThread function to perform thread specific cleanup when thread exists.
     */
    virtual void finish();

    /* Maintain LAN service UDP communication, thread main loop.
     */
    virtual void run();


protected:
    /**
    ************************************************************************************************
      Internal functions.
    ************************************************************************************************
    */

    /* Publish (initial or update) the end point information.
     */
    eStatus publish();


    /**
    ************************************************************************************************
      Member variables
    ************************************************************************************************
    */

    /** Set pointer to network service (eNetService is owned by eProcess, oe_lock() must
        be on to access.
     */
    eNetService *m_netservice;

    /** Value of ELIGHTHOUSEP_PUBLISH property. When changed, the end point information is
        published.
     */
    os_int m_publish_count;

    /** Publish function is not called immediately when m_publish_count changed, but instead
        m_publish is set. This allows multiple publish requests to result in one publish() call.
     */
    os_boolean m_publish;

    /** If data to publish was set successfully (something to publish)
     */
    // eStatus m_publish_status;
};

/* Start enet maintenance thread.
 */
void enet_start_maintain_thread(
    eNetService *netservice,
    os_int flags,
    eThreadHandle *maintain_thread_handle);


#endif
