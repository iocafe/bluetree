/**

  @file    eio_signalassembly.h
  @brief   Collection of signals as assembly, like game controller.
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

    /* Virtual destructor.
     */
    virtual ~eioSignalAssembly();

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

    /* Called when property value changes.
     */
    virtual eStatus onpropertychange(
        os_int propertynr,
        eVariable *x,
        os_int flags);

    /* Get value of simple property (override).
     */
    eStatus simpleproperty(
        os_int propertynr,
        eVariable *x);

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

    /* eStatus try_signal_setup(
        iocSignal *sig,
        const os_char *name,
        const os_char *mblk_name); */


    /* Collect information about this object for tree browser.
     */
    virtual void object_info(
        eVariable *item,
        eVariable *name,
        eSet *appendix,
        const os_char *target);

    virtual void send_open_info(
        eEnvelope *envelope);

    /**
    ************************************************************************************************
      Member variables
    ************************************************************************************************
    */

    /** Identifiers for the brick_buffer.
     */
    // iocIdentifiers m_exp_ids;
    // iocIdentifiers m_imp_ids;
};

#endif
