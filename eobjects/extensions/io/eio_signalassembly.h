/**

  @file    eio_signalassembly.h
  @brief   Collection of signals as assembly, like game controller.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef EIO_SIGNALASSEMBLY_H_
#define EIO_SIGNALASSEMBLY_H_
#include "extensions/io/eio.h"

/**
****************************************************************************************************
  eioSignalAssembly is marker fo collection os signals.
****************************************************************************************************
*/
class eioSignalAssembly : public eioAssembly
{
public:
    /* Constructor.
     */
    eioSignalAssembly(
        eObject *parent = OS_NULL,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT);

    /* Casting eObject pointer to eioSignalAssembly pointer.
     */
    inline static eioSignalAssembly *cast(
        eObject *o)
    {
        e_assert_type(o, ECLASSID_EIO_SIGNAL_ASSEMBLY)
        return (eioSignalAssembly*)o;
    }

    /* Get class identifier.
     */
    virtual os_int classid() {return ECLASSID_EIO_SIGNAL_ASSEMBLY; }

    /* Static function to add class to propertysets and class list.
     */
    static void setupclass();


    /**
    ************************************************************************************************
      Assembly specific functions.
    ************************************************************************************************
    */

    /* Prepare a newly created signal for use.
     */
    virtual eStatus setup(
        eioAssemblyParams *prm,
        iocRoot *iocom_root);

    virtual void run(os_long ti);

protected:
    /**
    ************************************************************************************************
      Internal functions.
    ************************************************************************************************
    */

    /* Collect information about this object for tree browser.
     */
    virtual void object_info(
        eVariable *item,
        eVariable *name,
        eSet *appendix,
        const os_char *target);

    /* Information for opening the object has been requested, send it.
     */
    virtual void send_open_info(
        eEnvelope *envelope);

    /* Helper for send_open_info()
     */
    void send_open_info_helper(
        eioDevice *device,
        os_int property_nr,
        eContainer *reply);
};

#endif
