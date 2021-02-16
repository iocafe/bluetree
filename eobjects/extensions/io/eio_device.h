/**

  @file    eio_device.h
  @brief   Object representing and IO device.
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
#ifndef EIO_DEVICE_H_
#define EIO_DEVICE_H_
#include "extensions/io/eio.h"

/**
****************************************************************************************************
  eioDevice is like a box of objects.
****************************************************************************************************
*/
class eioDevice : public eContainer
{
public:
    /* Constructor.
     */
    eioDevice(
        eObject *parent = OS_NULL,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT);

    /* Casting eObject pointer to eioDevice pointer.
     */
    inline static eioDevice *cast(
        eObject *o)
    {
        e_assert_type(o, ECLASSID_EIO_DEVICE)
        return (eioDevice*)o;
    }

    /* Get class identifier.
     */
    virtual os_int classid() {return ECLASSID_EIO_DEVICE; }

    /* Static function to add class to propertysets and class list.
     */
    static void setupclass();

    /* Called when property value changes.
     */
    virtual eStatus onpropertychange(
        os_int propertynr,
        eVariable *x,
        os_int flags);

    /* Get value of simple property.
     */
    virtual eStatus simpleproperty(
        os_int propertynr,
        eVariable *x);

    /* Process a callback from a child object.
     */
    eStatus oncallback(
        eCallbackEvent event,
        eObject *obj,
        eObject *appendix);

    /**
    ************************************************************************************************
      IO device specific functions.
    ************************************************************************************************
    */
    eioMblk *connected(
        struct eioMblkInfo *minfo);

    void disconnected(
        eioMblkInfo *minfo);

    eContainer *io();
    eContainer *assemblies();
    inline eContainer *mblks() {return m_mblks; }

protected:
    /**
    ************************************************************************************************
      Internal functions.
    ************************************************************************************************
    */

    /* Decide value for "bound" flag.
     */
    void set_bound(
        eCallbackEvent event);

    /**
    ************************************************************************************************
      Member variables
    ************************************************************************************************
    */
    eContainer *m_mblks;
    eContainer *m_io;
    eContainer *m_assemblies;

    /* Someone is bound to (looking at) this device.
     */
    os_boolean m_bound;

    /* This object is connected to IOCOM device.
     */
    os_boolean m_connected;

};

#endif
