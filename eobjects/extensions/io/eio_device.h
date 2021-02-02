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
  Defines
****************************************************************************************************
*/


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

    /* Virtual destructor.
     */
    virtual ~eioDevice();

    /* Clone object.
     */
    virtual eObject *clone(
        eObject *parent,
        e_oid id = EOID_CHILD,
        os_int aflags = 0);

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

    /* Static constructor function for generating instance by class list.
     */
    static eioDevice *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new eioDevice(parent, id, flags);
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


protected:
    /**
    ************************************************************************************************
      Internal functions.
    ************************************************************************************************
    */

    /* Flags the peristent object changed (needs to be saved).
     */
    // void touch();

    /**
    ************************************************************************************************
      Member variables
    ************************************************************************************************
    */
};

#endif
