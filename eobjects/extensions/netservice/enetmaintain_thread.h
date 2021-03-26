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
#define ENETMP_END_POINT_TABLE_MODIF_COUNT 10
#define ENETMP_END_POINT_CONFIG_COUNT 15
#define ENETMP_CONNECT_TABLE_MODIF_COUNT 20
#define ENETMP_LIGHTHOUSE_CHANGE_COUNT 25

/* Property names.
 */
extern const os_char
    enetmp_end_pont_table_modif_count[],
    enetmp_end_point_config_count[],
    enetmp_connect_table_modif_count[],
    enetmp_lighthouse_change_count[];


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


    /* Delete end point (stops end point thread and cleans up)
     */
    void delete_ep(
        eContainer *ep);

    /* End point status change.
     */
    void ep_status_changed(
        eContainer *ep);

    void set_ep_status(
        os_int row_nr,
        const os_char *column_name,
        eVariable *value);


    /* Create internal "socket list" table.
     */
    void create_socket_list_table();

    /* Merge "connect to" and "LAN services" into socket list.
     */
    void merge_to_socket_list();

    /* Add a row to socket list.
     */
    void add_socket_to_list(
        const os_char *name,
        eVariable *protocol,
        enetConnTransportIx transport_ix,
        const os_char *ip,
        os_int port_nr,
        os_boolean is_ipv6,
        eContainer *rows,
        eContainer *addr_blocklist,
        eContainer *name_blocklist,
        os_int contable_row);

    /* Create and delete connections.
     */
    void maintain_connections();

    /* Generate name for a connection.
     */
    void make_connection_name(
        eVariable *con_name,
        eVariable *name,
        eVariable *protocol,
        eVariable *ip,
        enetConnTransportIx transport_ix);

    /* Delete connection (stops connection thread and cleans up)
     */
    void delete_con(
        eContainer *con);

    void con_status_changed(
        eContainer *con);

    void set_con_status(
        os_int slist_row,
        os_int contab_row,
        const os_char *column_name,
        os_boolean value);

    /* A callback by a child object.
     */
    virtual eStatus oncallback(
        eCallbackEvent event,
        eObject *obj,
        eObject *appendix);

    /* Set timer period, how often to recive timer messages.
     */
    void set_timer(
        os_int timer_ms);

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
        The maintain_end_points() function is not called immediately when m_publish_count
        changes, but instead m_publish is set. This allows multiple publish requests to result
        in one maintain_end_points() call.
        Timer value when m_publish flag was set.
     */
    os_int m_end_point_table_modif_count;
    os_boolean m_configure_end_points;
    os_long m_end_point_config_timer;

    os_int m_end_point_config_count;

    os_long m_connect_table_modif_count;
    os_boolean m_configure_connections;
    os_long m_connect_timer;

    os_boolean m_trigger_connect_check_by_lighthouse;
    os_long m_lighthouse_modif_count;

    /* The "connect processess" matrix hold one row for each socket to create.
     */
    eMatrix *m_socket_list_matrix;

    /** Current periodic of timer messages.
     */
    os_int m_timer_ms;

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
