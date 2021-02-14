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

    /* Prepare a newly created brick buffer assembly for use.
     */
    virtual eStatus setup(
        eioAssemblyParams *prm,
        iocRoot *iocom_root);

    eStatus try_finalize_setup();

    virtual void run(os_long ti);

    eStatus get();


protected:
    /**
    ************************************************************************************************
      Internal functions.
    ************************************************************************************************
    */

    eStatus try_signal_setup(
        iocSignal *sig,
        const os_char *name,
        const os_char *mblk_name);

    /* Release all resource allocated for the brick buffer.
     */
    void cleanup();

    /* Clear all brick buffer member variables.
     */
    void clear_member_variables();

    /**
    ************************************************************************************************
      Member variables
    ************************************************************************************************
    */

    /** IOCOM brick_buffer object.
     */
    iocBrickBuffer m_brick_buffer;

    /* Signal structures.
     */
    iocSignal m_sig_cmd;
    iocSignal m_sig_select;
    iocSignal m_sig_err;
    iocSignal m_sig_cs;
    iocSignal m_sig_buf;
    iocSignal m_sig_head;
    iocSignal m_sig_tail;
    iocSignal m_sig_state;

    /** Memory block handles.
     */
    iocHandle m_h_exp;
    iocHandle m_h_imp;

    /** Identifiers for the brick_buffer.
     */
    iocIdentifiers m_exp_ids;
    iocIdentifiers m_imp_ids;

    os_char m_prefix[IOC_SIGNAL_NAME_SZ];

    /** The "is_device" flag indicates that this python code is acting as device end
        of brick data transfer, and the other end is controlling the transfer.
        If not set, the python code is controlling the transfer.
     */
    os_boolean m_is_device;

    /** The "from_device" sets transfer direction. If this flag is set, the transfer is
        from device to controller, otherwise from controller to the device. The "is_device"
        flag sets if this python code is device or controller.
     */
    os_boolean m_from_device;

    /** The "flag_buffer" flag indicates flat buffer transfer. If not set, the ring buffer
        transfer is used.
     */
    os_boolean m_flat_buffer;

    /** This is camera which outputs eBitmaps.
     */
    os_boolean m_is_camera;

    /** Variable holding output state.
     */
    eVariable *m_output;
};

#endif
