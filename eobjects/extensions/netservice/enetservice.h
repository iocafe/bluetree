/**

  @file    enetservice.h
  @brief   enet service implementation.
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
#ifndef ENETSERVICE_H_
#define ENETSERVICE_H_
#include "eobjects.h"
#include "extensions/netservice/eprotocol_handle.h"
#include "extensions/netservice/eprotocol_handle_ecom.h"
#include "extensions/netservice/enetparameters.h"
#include "extensions/netservice/enetendpoints.h"
#include "extensions/netservice/enetconnect.h"
#include "extensions/netservice/eprotocol.h"
#include "extensions/netservice/eprotocol_ecom.h"
#include "extensions/netservice/elighthouse_thread.h"
#include "extensions/netservice/enetmaintain_thread.h"
#include "extensions/iocom/eiocom.h"
#include "extensions/io/eio.h"
#include "extensions/switchbox/eswitchbox.h"

class eioRoot;

/**
****************************************************************************************************
  Defines, etc.
****************************************************************************************************
*/

/* Net service property numbers.
 */
#define ENETSERVP_ENDPOINT_CONFIG_CHANGE_COUNTER 10
#define ENETSERVP_ENDPOINT_CONFIG_COUNTER 15
#define ENETSERVP_CONNECT_CONFIG_CHANGE_COUNTER 20
#define ENETSERVP_LIGHTHOUSE_CHANGE_COUNTER 25

/* Net service property names.
 */
extern const os_char
    enetservp_endpoint_table_change_counter[],
    enetservp_endpoint_config_counter[],
    enetservp_connect_table_change_counter[],
    enetservp_lighthouse_change_counter[];

/* Default socket port number for eobject communication.
 */
#define ENET_DEFAULT_SOCKET_PORT 6371
#define ECOM_DEFAULT_SOCKET_PORT_STR "6371"

/* Flags for enet_start_service() function.
 */
#define ENET_ENABLE_IOCOM_CLIENT 1
#define ENET_ENABLE_ECOM_CLIENT 2
#define ENET_ENABLE_IOCOM_SERVICE 4
#define ENET_ENABLE_ECOM_SERVICE 8
#define ENET_ENABLE_IOCOM_SWITCHBOX_SERVICE 16
#define ENET_ENABLE_ECOM_SWITCHBOX_SERVICE 32
#define ENET_ENABLE_LIGHTHOUSE 64
#define ENET_DEFAULT_NO_END_POINTS 128

#define ENET_ENABLE_SERIAL_COM 1024
#define ENET_ENABLE_UNSECURED_SOCKETS 2048


/**
****************************************************************************************************
  eNetService class.
****************************************************************************************************
*/
class eNetService : public eObject
{
    friend class eLightHouseService;
    friend class eNetMaintainThread;

public:
    /* Constructor.
     */
    eNetService(
        eObject *parent = OS_NULL,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT);

    /* Virtual destructor.
     */
    virtual ~eNetService();

    /* Casting eObject pointer to eNetService pointer.
     */
    inline static eNetService *cast(
        eObject *o)
    {
        e_assert_type(o, ECLASSID_NETSERVICE)
        return (eNetService*)o;
    }

    /* Get class identifier.
     */
    virtual os_int classid() {return ECLASSID_NETSERVICE; }

    /* Static function to add class to propertysets and class list.
     */
    static void setupclass();

    /* Static constructor function for generating instance by class list.
     */
    static eNetService *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new eNetService(parent, id, flags);
    }

    /* Process a callback from a child object.
     */
    virtual eStatus oncallback(
        eCallbackEvent event,
        eObject *obj,
        eObject *appendix);

    /* Called after eNetService object is created to start it.
     */
    void start(
        os_int flags);

    /* Start closing net service (no process lock).
     */
    void finish();

    /* Get pointer to protocol container (used for start up only).
     */
    inline eContainer *protocols() {return m_protocols; }

    /* Get pointer to IOCOM root objects.
     */
    inline iocRoot *iocom_root() {return &m_iocom_root; }

protected:
    /**
    ************************************************************************************************
      Internal functions.
    ************************************************************************************************
    */

    /* Network event/error handler to move information by callbacks to messages.
     */
    static void net_event_handler(
        osalErrorLevel level,
        const os_char *module,
        os_int code,
        const os_char *description,
        void *context);

    /* Create "user accounts" table.
     */
    void create_user_account_table(
        os_int flags);

    /* Add new user account.
     */
    void add_user_account(
        os_int enable,
        const os_char *user_name,
        const os_char *password,
        const os_char *accept,
        os_int privileges,
        os_int row_nr = -1);

    /* Callback from IOCOM get user authorization.
     */
    static osalStatus authorize_iocom_user(
        struct iocRoot *root,
        iocAllowedNetworkConf *allowed_networks,
        iocUser *user_account,
        os_char *ip,
        void *context);

    /* Create "end point" table.
     */
    void create_end_point_table(
        os_int flags);

    /* Add a line for an end point to "end point" table.
     */
    void add_end_point(
        os_int enable,
        const os_char *protocol,
        enetEndpTransportIx transport_ix,
        const os_char *port,
        const os_char *netname = OS_NULL,
        os_int row_nr = -1);

    /* Create "connections" table.
     */
    void create_connect_table(
        os_int flags);

    /* Add a row for a connection to "connections" table.
     */
    void add_connect(
        os_int enable,
        const os_char *protocol,
        const os_char *ip,
        const os_char *name,
        os_int transport,
        os_int row_nr = -1);

    /* Create "io device networks and processes" table.
     */
    void create_lan_services_table();

    /* Create table of trusted client certificates.
     */
    void create_trusted_certificate_table();

    /* Create process status table.
     */
    void create_process_status_table();

    /* Create parameters for service.
     */
    void create_service_parameters(
        os_int flags);

    /* Parameter value has changed.
     */
    void parameter_changed(
        eObject *v);

    /**
    ************************************************************************************************
      Member variables
    ************************************************************************************************
    */

    /** IOCOM root object!
     */
    iocRoot m_iocom_root;

    /** IO object structure root.
     */
    eioRoot *m_eio_root;

    /** End point and connection management thread handle.
     */
    eThreadHandle m_maintain_thread_handle;

    /** Lighthouse thread handle.
     */
    eThreadHandle m_lighthouse_thread_handle;

    /** Container to hold protocol objects during startup before net maintain thread is created.
     */
    eContainer *m_protocols;

    /* User accounts table (matrix) and persistent object to contain it.
     */
    eMatrix *m_account_matrix;
    ePersistent *m_persistent_accounts;

    /* End point table (matrix) and persistent object to contain it.
     */
    eMatrix *m_endpoint_matrix;
    ePersistent *m_end_points;
    os_long m_end_points_config_counter;

    /* Connection table (matrix) and persistent object to contain it.
     */
    eMatrix *m_connect_to_matrix;
    ePersistent *m_connect;
    os_long m_connect_config_counter;

    /* Services table (matrix).
     */
    eMatrix *m_services_matrix;
    os_long m_lighthouse_change_counter;

    /* Trusted client certificate table (matrix) and persistent object to contain it.
     */
    eMatrix *m_trusted_matrix;
    ePersistent *m_persistent_trusted;

    /* Structure of server parameters and persistent object to contain these.
     */
    eNetServPrm m_parameters;
    ePersistent *m_persistent_parameters;

    /* Process status table.
     */
    eMatrix *m_status_matrix;
    ePersistent *m_persistent_status;
};


/* Create network service object.
 */
void enet_initialize_service();

/* Add a communication protocol for the network service object.
 */
void enet_add_protocol(
    eProtocol *protocol);

/* Start network service.
 */
void enet_start_service(
    os_int flags);

/* Shut down network service.
 */
void enet_stop_service();

#endif
