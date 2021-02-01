/**

  @file    eio_root.h
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
#ifndef EIO_ROOT_H_
#define EIO_ROOT_H_
#include "extensions/eiocom/eiocom.h"

#if 0

class eNetService;

/**
****************************************************************************************************
  Defines, etc.
****************************************************************************************************
*/



/**
****************************************************************************************************
  eioRoot class.
****************************************************************************************************
*/
class eioRoot : public eObject
{
    friend class eLightHouseService;
    friend class eNetMaintainThread;

public:
    /* Constructor.
     */
    eioRoot(
        eObject *parent = OS_NULL,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT);

    /* Virtual destructor.
     */
    virtual ~eioRoot();

    /* Casting eObject pointer to eioRoot pointer.
     */
    inline static eioRoot *cast(
        eObject *o)
    {
        e_assert_type(o, ECLASSID_EIO_ROOT)
        return (eioRoot*)o;
    }

    /* Get class identifier.
     */
    virtual os_int classid() {return ECLASSID_EIO_ROOT; }

    /* Static function to add class to propertysets and class list.
     */
    static void setupclass();

    /* Static constructor function for generating instance by class list.
     */
    static eioRoot *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new eioRoot(parent, id, flags);
    }

    /* Process a callback from a child object.
     */
    virtual eStatus oncallback(
        eCallbackEvent event,
        eObject *obj,
        eObject *appendix);

    /* Called after eioRoot object is created to start it.
     */
    void start(
        os_int flags);

    /* Start closing net service (no process lock).
     */
    void finish();

    inline struct eNetService *netservice()
    {
        return m_netservice;
    }

    void set_netservice(
        class eNetService *netservice);


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

    /* Create "io device networks and processes" table.
     */
    void create_services_table();


    /**
    ************************************************************************************************
      Member variables
    ************************************************************************************************
    */

    /** IOCOM root object (owned by netservice)!
     */
    iocRoot *m_iocom_root;

    class eNetService *m_netservice;

    /* Services table (matrix).
     */
    eMatrix *m_services_matrix;
    os_long m_lighthouse_change_counter;
};


#endif

#endif
