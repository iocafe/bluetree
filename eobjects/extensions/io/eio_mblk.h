/**

  @file    eio_mblk.h
  @brief   Object representing and IO memory block.
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
#ifndef EIO_MBLK_H_
#define EIO_MBLK_H_
#include "extensions/io/eio.h"


/**
****************************************************************************************************
  Defines
****************************************************************************************************
*/


/**
****************************************************************************************************
  eioMblk is like a box of objects.
****************************************************************************************************
*/
class eioMblk : public eContainer
{
public:
    /* Constructor.
     */
    eioMblk(
        eObject *parent = OS_NULL,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT);

    /* Virtual destructor.
     */
    virtual ~eioMblk();

    /* Casting eObject pointer to eioMblk pointer.
     */
    inline static eioMblk *cast(
        eObject *o)
    {
        e_assert_type(o, ECLASSID_EIO_MBLK)
        return (eioMblk*)o;
    }

    /* Get class identifier.
     */
    virtual os_int classid() {return ECLASSID_EIO_MBLK; }

    /* Static function to add class to propertysets and class list.
     */
    static void setupclass();

    /* Static constructor function for generating instance by class list.
     */
    static eioMblk *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new eioMblk(parent, id, flags);
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

    /* A callback by a child object.
     */
    virtual eStatus oncallback(
        eCallbackEvent event,
        eObject *obj,
        eObject *appendix);

    /**
    ************************************************************************************************
      Maintain IO network hierarchy.
    ************************************************************************************************
    */
    void connected(
        struct eioMblkInfo *minfo);

    void disconnected(
        eioMblkInfo *minfo);

    eContainer *esignals();

    inline iocHandle *handle_ptr() {return &m_handle;}
    inline os_short mblk_flags() {return m_mblk_flags; }

protected:
    /**
    ************************************************************************************************
      Internal functions.
    ************************************************************************************************
    */

    static void callback(
        struct iocHandle *handle,
        os_int start_addr,
        os_int end_addr,
        os_ushort flags,
        void *context);

    /**
    ************************************************************************************************
      Member variables
    ************************************************************************************************
    */
    eioRoot *m_eio_root;
    iocHandle m_handle;
    os_boolean m_handle_set;

    os_short m_mblk_flags;       /* Memory block flags, bit fields: IOC_MBLK_DOWN, IOC_MBLK_UP. */

    eContainer *m_esignals;
};

#endif
