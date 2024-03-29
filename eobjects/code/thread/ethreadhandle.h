/**

  @file    ethreadhandle.h
  @brief   Thread handle class.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

  Thread handle is used for controlling threads from another thread.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef ETHREADHANDLE_H_
#define ETHREADHANDLE_H_
#include "eobjects.h"

/**
****************************************************************************************************
  eThread handle can be used to request child thread to exit and wait for it to join.
****************************************************************************************************
*/
class eThreadHandle : public eObject
{
    friend class eThread;

public:

    /**
    ************************************************************************************************
      Generic eObject functionality.
    ************************************************************************************************
    */

    /* Constructor.
     */
    eThreadHandle(
        eObject *parent = OS_NULL,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_EROOT_OPTIONAL);

    /* Virtual destructor.
     */
    virtual ~eThreadHandle();

    /* Casting eObject pointer to eThreadHandle pointer.
     */
    inline static eThreadHandle *cast(
        eObject *o)
    {
        e_assert_type(o, ECLASSID_THREAD_HANDLE)
        return (eThreadHandle*)o;
    }

    /* Get class identifier.
     */
    virtual os_int classid() {return ECLASSID_THREAD_HANDLE;}

    /* Static constructor function for generating instance by class list.
     */
    static eThreadHandle *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new eThreadHandle(parent, id, flags);
    }


    /**
    ************************************************************************************************
      Thread functions.
    ************************************************************************************************
    */

    /* Request to terminate a thread.
     */
    void terminate();

    /* Wait until thread has terminated.
     */
    void join();

    inline os_char *uniquename()
    {
        return m_unique_thread_name;
    }


private:
    /**
    ************************************************************************************************
      Internal functions
    ************************************************************************************************
    */
    inline void set_osal_handle(
        osalThread *h)
    {
        m_osal_handle = h;
    }

    void save_unique_thread_name(
        eThread *thread);


    /**
    ************************************************************************************************
      Member variables
    ************************************************************************************************
    */
    osalThread *m_osal_handle;

    os_char m_unique_thread_name[E_OIXSTR_BUF_SZ];
};

#endif
