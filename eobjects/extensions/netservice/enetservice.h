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

/**
****************************************************************************************************
  eNetService class.
****************************************************************************************************
*/
class eNetService : public eThread
{
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

    /* Clone object.
     */
    virtual eObject *clone(
        eObject *parent,
        e_oid id = EOID_CHILD,
        os_int aflags = 0);

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

    virtual void initialize(
        eContainer *params = OS_NULL);

protected:
    /**
    ************************************************************************************************
      Internal functions.
    ************************************************************************************************
    */

    /* Create "user accounts" table.
     */
    void create_user_account_table();

    void add_user_account(
        const os_char *user_name,
        const os_char *password,
        os_int accept,
        os_int privileges,
        os_int row_nr = -1);

    /* Create "end point" table.
     */
    void create_end_point_table();

    /* Add a line for an end point to "end point" table.
     */
    void add_end_point(
        os_int enable,
        os_int protocol,
        os_int transport,
        const os_char *port,
        os_int row_nr = -1);

    /* Create "connections" table.
     */
    void create_connection_table();

    /* Add a row for a connection to "connections" table.
     */
    void add_connection(
        os_int enable,
        os_int protocol,
        const os_char *connection,
        const os_char *devices,
        os_int transport,
        os_int row_nr = -1);


    /**
    ************************************************************************************************
      Member variables
    ************************************************************************************************
    */

    /** Persistent object to hold user accounts table.
     */
    ePersistent *m_persistent_accounts;

    /** User accounts table (matrix).
     */
    eMatrix *m_account_matrix;

    /** Persistent object to hold end point table.
     */
    ePersistent *m_end_points;

    /** End point table (matrix).
     */
    eMatrix *m_endpoint_matrix;

    /** Persistent object to hold the connection table.
     */
    ePersistent *m_connections;

    /** Connection table (matrix).
     */
    eMatrix *m_connection_matrix;
};

/* Start network service.
 */
void enet_start_server(
    eThreadHandle *server_thread_handle);

#endif
