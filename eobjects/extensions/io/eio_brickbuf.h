/**

  @file    eio_brickbuf.h
  @brief   "Brick" data transfer, like camera images, etc.
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
#ifndef EIO_BRICK_BUFFER_H_
#define EIO_BRICK_BUFFER_H_
#include "extensions/io/eio.h"


/**
****************************************************************************************************
  Defines
****************************************************************************************************
*/


/**
****************************************************************************************************
  eioBrickBuffer is like a box of objects.
****************************************************************************************************
*/
class eioBrickBuffer : public eioAssembly
{
public:
    /* Constructor.
     */
    eioBrickBuffer(
        eObject *parent = OS_NULL,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT);

    /* Virtual destructor.
     */
    virtual ~eioBrickBuffer();

    /* Casting eObject pointer to eioBrickBuffer pointer.
     */
    inline static eioBrickBuffer *cast(
        eObject *o)
    {
        e_assert_type(o, ECLASSID_EIO_BRICK_BUFFER)
        return (eioBrickBuffer*)o;
    }

    /* Get class identifier.
     */
    virtual os_int classid() {return ECLASSID_EIO_BRICK_BUFFER; }

    /* Static function to add class to propertysets and class list.
     */
    static void setupclass();

    /* Static constructor function for generating instance by class list.
     */
    static eioBrickBuffer *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new eioBrickBuffer(parent, id, flags);
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
