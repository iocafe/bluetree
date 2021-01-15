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
#define ENETMAINTAINP_CONFIG_COUNTER 20
#define ENETMAINTAINP_CONNECT 30

/* Property names.
 */
extern const os_char
    enetmaintainp_publish[],
    enetmaintainp_config_counter[],
    enetmaintainp_connect[];


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

    /* Adopt communication protocol object.
     */
    void add_protocol(
        eProtocol *proto);

protected:
    /**
    ************************************************************************************************
      Internal functions.
    ************************************************************************************************
    */

    /* Create and delete end points.
     */
    void maintain_end_points();

    /* Create and delete connections.
     */
    void maintain_connections();

    /* Delete end point (stops end point thread and cleans up)
     */
    void delete_ep(
        eContainer *ep);

    /* Delete connection (stops connection thread and cleans up)
     */
    void delete_con(
        eContainer *con);

    /* Get protocol by name
     */
    eProtocol *protocol_by_name(
        eVariable *proto_name);



    /**
    ************************************************************************************************
      Member variables
    ************************************************************************************************
    */

    /** Set pointer to network service (eNetService is owned by eProcess, oe_lock() must
        be on to access.
     */
    eNetService *m_netservice;

    /** Container to hold protocol objects.
     */
    eContainer *m_protocols;

/* --- Maintain end points --- */

    /** List of running end point, data about those.
     */
    eContainer *m_end_points;

    /** Value of ELIGHTHOUSEP_PUBLISH property. When changed, the end points are to be configured.
     */
    os_int m_publish_count;

    /** The maintain_end_points() function is not called immediately when m_publish_count
        changes, but instead m_publish is set. This allows multiple publish requests to result
        in one maintain_end_points() call.
     */
    os_boolean m_publish;

    /** Timer value when m_publish flag was set.
     */
    os_long m_publish_timer;

    /** End point configuration counter.
     */
    os_long m_end_point_config_count;

/* --- Maintain connections --- */
    /** Value of ELIGHTHOUSEP_CONNECT property. When changed, the connections are to be configured..
     */
    os_int m_connect_count;

    /** The maintain_connections() function is not called immediately when m_connect_count
        changes, but instead m_connect is set. This allows multiple connect requests to
        result in one maintain_connections() call.
     */
    os_boolean m_connect;

    /** Timer value when m_connect flag was set.
     */
    os_long m_connect_timer;

    /** List of running connections, data about those.
     */
    eContainer *m_connections;

};

/* Start enet maintenance thread.
 */
void enet_start_maintain_thread(
    eNetService *netservice,
    os_int flags,
    eThreadHandle *maintain_thread_handle);


#endif
