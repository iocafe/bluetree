/**

  @file    eio_network.h
  @brief   Object representing and IO network.
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
#ifndef EIO_NETWORK_H_
#define EIO_NETWORK_H_
#include "extensions/io/eio.h"

struct eioMblkInfo;

/**
****************************************************************************************************
  eioNetwork is like a box of objects.
****************************************************************************************************
*/
class eioNetwork : public eContainer
{
public:
    /* Constructor.
     */
    eioNetwork(
        eObject *parent = OS_NULL,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT);

    /* Casting eObject pointer to eioNetwork pointer.
     */
    inline static eioNetwork *cast(
        eObject *o)
    {
        e_assert_type(o, ECLASSID_EIO_NETWORK)
        return (eioNetwork*)o;
    }

    /* Get class identifier.
     */
    virtual os_int classid() {return ECLASSID_EIO_NETWORK; }

    /* Static function to add class to propertysets and class list.
     */
    static void setupclass();

    /* Called when property value changes.
     */
    virtual eStatus onpropertychange(
        os_int propertynr,
        eVariable *x,
        os_int flags);


    /**
    ************************************************************************************************
      Maintain IO network hierarchy.
    ************************************************************************************************
    */
    eioMblk *connected(
        struct eioMblkInfo *minfo);

    eioDevice *get_device(
        const os_char *device_id);

    void disconnected(
        eioMblkInfo *minfo);
};

#endif
