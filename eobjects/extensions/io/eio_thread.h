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

    /* Static function to add class to propertysets and class list.
     */
    static void setupclass();

    /* Function to process incoming messages.
     */
    virtual void onmessage(
        eEnvelope *envelope);

    /* Set pointer to network service (eNetService is owned by eProcess, oe_lock() must
       be on to access.
     */
    inline void set_iocom_root(
        iocRoot *iocom_root)
    {
        m_iocom_root = iocom_root;
    }

    /* Save object hierarchy root. This is used only setting time stamps.
     */
    inline void set_eio_root(
        eioRoot *eio_root)
    {
        m_eio_root = eio_root;
    }

    /* RUN IOCOM communication, thread main loop.
     */
    virtual void run();


protected:
    /**
    ************************************************************************************************
      Internal functions.
    ************************************************************************************************
    */

    /**
    ************************************************************************************************
      Member variables
    ************************************************************************************************
    */

    /** IOCOM root object!
     */
    iocRoot *m_iocom_root;

    /** IO object hierarchy root (time stamps).
     */
    eioRoot *m_eio_root;
};


/* Start enet maintenance thread.
 */
void eio_start_thread(
    eioRoot *eio_root,
    eThreadHandle *io_thread_handle);


/* Stop IO thread.
 */
void eio_stop_io_thread(
    eioRoot *eio_root);

#endif
