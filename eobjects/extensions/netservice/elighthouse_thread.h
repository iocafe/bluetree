/**

  @file    elighthouse_thread.h
  @brief   Look out for device networks in the same LAN and announce services by UDP multicast.
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
#ifndef ELIGHTHOUSE_THREAD_H_
#define ELIGHTHOUSE_THREAD_H_
#include "extensions/netservice/enetservice.h"
#include "lighthouse.h"

class eNetService;

/**
****************************************************************************************************
  Defines, etc.
****************************************************************************************************
*/

/* Lighthouse UDP service property numbers.
 */
#define ELIGHTHOUSEP_SEND_UDP_MULTICASTS 10
#define ELIGHTHOUSEP_PUBLISH 15

/* Lighthouse UDP service property names.
 */
extern const os_char
    elighthousep_send_udp_multicasts[],
    elighthousep_publish[];

/* "LAN services" table column names.
 */
extern const os_char enet_lansrv_name[];
extern const os_char enet_lansrv_nick[];
extern const os_char enet_lansrv_protocol[];
extern const os_char enet_lansrv_ip[];
extern const os_char enet_lansrv_tlsport[];
extern const os_char enet_lansrv_tcpport[];
extern const os_char enet_lansrv_tstamp[];


/**
****************************************************************************************************
  eLightHouseService class.
****************************************************************************************************
*/
class eLightHouseService : public eThread
{
public:
    /* Constructor.
     */
    eLightHouseService(
        eObject *parent = OS_NULL,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT);

    /* Casting eObject pointer to eLightHouseService pointer.
     */
    inline static eLightHouseService *cast(
        eObject *o)
    {
        e_assert_type(o, ECLASSID_LIGHT_HOUSE_CLIENT)
        return (eLightHouseService*)o;
    }

    /* Get class identifier.
     */
    virtual os_int classid() {return ECLASSID_LIGHT_HOUSE_CLIENT; }

    /* Static function to add class to propertysets and class list.
     */
    static void setupclass();

    /* Static constructor function for generating instance by class list.
     */
    static eLightHouseService *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new eLightHouseService(parent, id, flags);
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

    /* Set pointer to network service (eNetService is owned by eProcess, os_lock() must
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

    /* Process received endpoint information, callback.
     */
    static void callback(
        struct LighthouseClient *c,
        LightHouseClientCallbackData *data,
        void *context);

    /* Send end point information out as UDP multicast.
     */
    void run_server();

    /* Set timer period, how often to recive timer messages.
     */
    void set_timer(
        os_int timer_ms);

    /* Publish (initial or update) the end point information.
     */
    eStatus publish();


    /**
    ************************************************************************************************
      Member variables
    ************************************************************************************************
    */

    /** Set pointer to network service (eNetService is owned by eProcess, os_lock() must
        be on to access.
     */
    eNetService *m_netservice;

/* CLIENT ************************** */
    /** eosal lighthouse client structure.
     */
    LighthouseClient m_client;

    /** Multicast counters by network service.
     */
    eContainer *m_counters;

/* SERVICE ************************* */

    /** eosal lighthouse server structure.
     */
    LighthouseServer m_server;

    /** Server side IDP multicasts are enabled.
     */
    os_boolean m_send_udp_multicasts;

    /** Value of ELIGHTHOUSEP_PUBLISH property. When changed, the end point information is
        published.
     */
    os_int m_publish_count;

    /** Publish function is not called immediately when m_publish_count changed, but instead
        m_publish is set. This allows multiple publish requests to result in one publish() call.
     */
    os_boolean m_publish;

    os_timer m_publish_timer;

    /** OS_TRUE if initialized for sending UDP multicasts.
     */
    os_boolean m_udp_send_initialized;

    /** If data to publish was set successfully (something to publish)
     */
    eStatus m_publish_status;

    /** Current periodic of timer messages.
     */
    os_int m_timer_ms;

    os_boolean m_run_server_now;
};

/* Start light house thread.
 */
void enet_start_lighthouse_thread(
    eNetService *netservice,
    os_int flags,
    eThreadHandle *lighthouse_thread_handle);


#endif
