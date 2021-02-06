/**

  @file    eio_thread.h
  @brief   Thread to run the IO.
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
#ifndef EIO_THREAD_H_
#define EIO_THREAD_H_
#include "extensions/io/eio.h"


/**
****************************************************************************************************
  Defines, etc.
****************************************************************************************************
*/

/* Property numbers.
 */
/* #define EIOTHREADP_END_POINT_TABLE_MODIF_COUNT 10
#define EIOTHREADP_END_POINT_CONFIG_COUNT 15
#define EIOTHREADP_CONNECT_TABLE_MODIF_COUNT 20
#define EIOTHREADP_LIGHTHOUSE_CHANGE_COUNT 25
*/

/* Property names.
 */
/* extern const os_char
    eiothreadp_end_pont_table_modif_count[],
    eiothreadp_end_point_config_count[],
    eiothreadp_connect_table_modif_count[],
    eiothreadp_lighthouse_change_count[]; */


/**
****************************************************************************************************
  eioThread class.
****************************************************************************************************
*/
class eioThread : public eThread
{
public:
    /* Constructor.
     */
    eioThread(
        eObject *parent = OS_NULL,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT);

    /* Virtual destructor.
     */
    virtual ~eioThread();

    /* Casting eObject pointer to eioThread pointer.
     */
    inline static eioThread *cast(
        eObject *o)
    {
        e_assert_type(o, ECLASSID_EIO_THREAD)
        return (eioThread*)o;
    }

    /* Get class identifier.
     */
    virtual os_int classid() {return ECLASSID_EIO_THREAD; }

    /* Static function to add class to propertysets and class list.
     */
    static void setupclass();

    /* Static constructor function for generating instance by class list.
     */
    static eioThread *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new eioThread(parent, id, flags);
    }

    /* Function to process incoming messages.
     */
    virtual void onmessage(
        eEnvelope *envelope);

    /* Called when property value changes.
     */
/*     virtual eStatus onpropertychange(
        os_int propertynr,
        eVariable *x,
        os_int flags); */

    /* Set pointer to network service (eNetService is owned by eProcess, oe_lock() must
       be on to access.
     */
    inline void set_iocom_root(
        iocRoot *iocom_root)
    {
        m_iocom_root = iocom_root;
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

    /* Set timer period, how often to recive timer messages.
     */
    void set_timer(
        os_int timer_ms);

    /**
    ************************************************************************************************
      Member variables
    ************************************************************************************************
    */

    /** IOCOM root object!
     */
    iocRoot *m_iocom_root;

    /** Set pointer to network service (eNetService is owned by eProcess, oe_lock() must
        be on to access.
     */
//    eNetService *m_netservice;


};

/* Start enet maintenance thread.
 */
void eio_start_thread(
    eioRoot *eio_root,
    eThreadHandle *io_thread_handle);


#endif
