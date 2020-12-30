/**

  @file    elighthouse_client.h
  @brief   Look out for device networks in the same LAN.
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
#ifndef ELIGHT_HOUSE_CLIENT_H_
#define ELIGHT_HOUSE_CLIENT_H_
#include "iocom.h"
#include "lighthouse.h"

/* Default socket port number for eobject communication. TCP ports 6371 - 6375 are unassigned.
 */
#define ENET_DEFAULT_SOCKET_PORT 6371
#define ENET_DEFAULT_SOCKET_PORT_STR "6371"

/**
****************************************************************************************************
  eLightHouseClient class.
****************************************************************************************************
*/
class eLightHouseClient : public eThread
{
public:
    /* Constructor.
     */
    eLightHouseClient(
        eObject *parent = OS_NULL,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT);

    /* Virtual destructor.
     */
    virtual ~eLightHouseClient();

    /* Clone object.
    virtual eObject *clone(
        eObject *parent,
        e_oid id = EOID_CHILD,
        os_int aflags = 0);
     */

    /* Casting eObject pointer to eLightHouseClient pointer.
     */
    inline static eLightHouseClient *cast(
        eObject *o)
    {
        e_assert_type(o, ECLASSID_LIGHT_HOUSE_CLIENT)
        return (eLightHouseClient*)o;
    }

    /* Get class identifier.
     */
    virtual os_int classid() {return ECLASSID_LIGHT_HOUSE_CLIENT; }

    /* Static function to add class to propertysets and class list.
     */
    static void setupclass();

    /* Static constructor function for generating instance by class list.
     */
    static eLightHouseClient *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new eLightHouseClient(parent, id, flags);
    }

    /* Overloaded eThread function to initialize new thread.
     */
    virtual void initialize(
        eContainer *params = OS_NULL);

    /* Overloaded eThread function to perform thread specific cleanup when threa exists.
     */
    virtual void finish();

    virtual void run();


protected:
    /**
    ************************************************************************************************
      Internal functions.
    ************************************************************************************************
    */

    static void callback(
        struct LighthouseClient *c,
        LightHouseClientCallbackData *data,
        void *context);

    /* Create "io device networks and processes" table.
     */
    void create_table();


    /**
    ************************************************************************************************
      Member variables
    ************************************************************************************************
    */

    /** eosal lighthouse client structure.
     */
    LighthouseClient m_lighthouse;

    /** Table showing eobjcts processess and iocom device networks.
     */
    eMatrix *m_matrix;

    /** Multicast counters by network service.
     */
    eContainer *m_counters;
};

/* Start light house client.
 */
void enet_start_lighthouse_client(
    eThreadHandle *lighthouse_client_thread_handle);


#endif
