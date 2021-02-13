/**

  @file    eio_brickbuf.cpp
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
#include "extensions/io/eio.h"


/**
****************************************************************************************************
  Constructor.
****************************************************************************************************
*/
eioBrickBuffer::eioBrickBuffer(
    eObject *parent,
    e_oid oid,
    os_int flags)
    : eioAssembly(parent, oid, flags)
{
    clear_member_variables();
    initproperties();
    ns_create();
}


/**
****************************************************************************************************
  Virtual destructor.
****************************************************************************************************
*/
eioBrickBuffer::~eioBrickBuffer()
{
    cleanup();
}


/**
****************************************************************************************************

  @brief Add the class to class list and class'es properties to it's property set.

  The eioBrickBuffer::setupclass function adds the class to class list and class'es properties to
  it's property set. The class list enables creating new objects dynamically by class identifier,
  which is used for serialization reader functions. The property set stores static list of
  class'es properties and metadata for those.

****************************************************************************************************
*/
void eioBrickBuffer::setupclass()
{
    const os_int cls = ECLASSID_EIO_BRICK_BUFFER;

    /* Add the class to class list.
     */
    os_lock();
    eclasslist_add(cls, (eNewObjFunc)newobj, "eioBrickBuffer", ECLASSID_EIO_ASSEMBLY);
    addpropertys(cls, ECONTP_TEXT, econtp_text, "text", EPRO_PERSISTENT|EPRO_NOONPRCH);
    propertysetdone(cls);
    os_unlock();
}


/**
****************************************************************************************************

  @brief Function to process incoming messages.

  The eTreeNode::onmessage function handles messages received by object. If this function
  doesn't process message, it calls parent class'es onmessage function.

  @param   envelope Message envelope. Contains command, target and source paths and
           message content, etc.
  @return  None.

****************************************************************************************************
*/
void eioBrickBuffer::onmessage(
    eEnvelope *envelope)
{
    /* If at final destination for the message.
     */
    /* if (*envelope->target()=='\0' && envelope->command() == ECMD_TIMER)
    {
        check_save_timer();
        return;
    } */

    /* Default thread message processing.
     */
    eioAssembly::onmessage(envelope);
}


/**
****************************************************************************************************

  @brief Called to inform the class about property value change (override).

  The onpropertychange() function is called when class'es property changes, unless the
  property is flagged with EPRO_NOONPRCH.
  If property is flagged as EPRO_SIMPLE, this function shuold save the property value
  in class members and and return it when simpleproperty() is called.

  Notice for change logging: Previous value is still valid when this function is called.
  You can get the old value by calling property() function inside onpropertychange()
  function.

  @param   propertynr Property number of changed property.
  @param   x Variable containing the new value.
  @param   flags
  @return  If successfull, the function returns ESTATUS_SUCCESS (0). Nonzero return values do
           indicate that there was no property with given property number.

****************************************************************************************************
*/
eStatus eioBrickBuffer::onpropertychange(
    os_int propertynr,
    eVariable *x,
    os_int flags)
{
/*     switch (propertynr)
    {
        default:
            goto call_parent;
    }

    return ESTATUS_SUCCESS;

call_parent:
*/
    return eioAssembly::onpropertychange(propertynr, x, flags);
}


/**
****************************************************************************************************

  @brief Prepare a newly created brick buffer assembly for use.

  The eioBrickBuffer::setup function...

****************************************************************************************************
*/
eStatus eioBrickBuffer::setup(
    eioAssemblyParams *prm,
    iocRoot *iocom_root)
{
    iocStreamerSignals sig;
    const char *p;

    /* Start from beginning, clean all.
     */
    cleanup();

    /* Determine flags
     */
    m_is_device = OS_FALSE;
    m_from_device = OS_TRUE;
    p = prm->type_str;
    if (!os_strcmp(p, "cam_flat")) {
        m_flat_buffer = OS_TRUE;
    }
    else if (os_strcmp(p, "lcam_flat")) {
        m_flat_buffer = OS_TRUE;
    }
    else if (!os_strcmp(p, "cam_ring")) {
        m_flat_buffer = OS_FALSE;
    }
    else if (os_strcmp(p, "lcam_right")) {
        m_flat_buffer = OS_FALSE;
    }
    else {
        osal_debug_error_str("eioBrickBuffer: Unknown assembly type: ", p);
        return ESTATUS_FAILED;
    }

    m_sig_cmd.handle = &m_h_imp;
    m_sig_select.handle = &m_h_imp;
    m_sig_err.handle = &m_h_exp;
    m_sig_state.handle = &m_h_exp;

    if (m_from_device) {
        m_sig_buf.handle = &m_h_exp;
        m_sig_cs.handle = &m_h_exp;
        m_sig_head.handle = &m_h_exp;
        m_sig_tail.handle = &m_h_imp;
    }
    else {
        m_sig_buf.handle = &m_h_imp;
        m_sig_cs.handle = &m_h_imp;
        m_sig_head.handle = &m_h_imp;
        m_sig_tail.handle = &m_h_exp;
    }

    ioc_iopath_to_identifiers(iocom_root, &m_exp_ids, prm->exp_str, IOC_EXPECT_MEMORY_BLOCK);
    ioc_iopath_to_identifiers(iocom_root, &m_imp_ids, prm->imp_str, IOC_EXPECT_MEMORY_BLOCK);

    os_strncpy(m_prefix, prm->prefix, sizeof(m_prefix));

    /* Initialize brick buffer (does not allocate any memory yet)
    */
    os_memclear(&sig, sizeof(iocStreamerSignals));
    sig.to_device = !m_from_device;
    sig.flat_buffer = m_flat_buffer;
    sig.cmd = &m_sig_cmd;
    sig.select = &m_sig_select;
    sig.err = &m_sig_err;
    sig.cs = &m_sig_cs;
    sig.state = &m_sig_state;
    sig.buf = &m_sig_buf;
    sig.head = &m_sig_head;
    sig.tail = &m_sig_tail;

    ioc_initialize_brick_buffer(&m_brick_buffer, &sig, iocom_root, prm->timeout_ms,
        m_is_device ? IOC_BRICK_DEVICE : IOC_BRICK_CONTROLLER);

    return ESTATUS_SUCCESS;
}


/**
****************************************************************************************************

  Try to setup signal stucture for use.

  Lock must be on.

  @param   sig Pointer to signal structure to set up.
  @param   name Signal name without prefix.
  @param   identifiers Specifies block to use.
  @return  OSAL_SUCCESS if all is successfull, OSAL_STATUS_FAILED otherwise.

****************************************************************************************************
*/
eStatus eioBrickBuffer::try_signal_setup(
    iocSignal *sig,
    const os_char *name,
    const os_char *mblk_name)
{
    eioDevice *device;
    eioMblk *mblk;
    eioSignal *eiosig;
    iocHandle *handle, *srchandle;

    os_char signal_name[IOC_SIGNAL_NAME_SZ];

    device = eioDevice::cast(grandparent());
    mblk = eioMblk::cast(device->byname(mblk_name));
    if (mblk == OS_NULL) return ESTATUS_FAILED;

    os_strncpy(signal_name, m_prefix, sizeof(signal_name));
    os_strncat(signal_name, name, sizeof(signal_name));

    eiosig = eioSignal::cast(mblk->byname(signal_name));
    if (eiosig == OS_NULL) return ESTATUS_FAILED;

    sig->addr = eiosig->io_addr();
    sig->n = eiosig->io_n();
    sig->flags = eiosig->io_flags();

    handle = sig->handle;
    if (handle->mblk) return ESTATUS_SUCCESS;

    if (m_h_exp.mblk) {
        ioc_release_handle(&m_h_exp);
    }

    srchandle = mblk->handle_ptr();
    if (srchandle->mblk == OS_NULL) return ESTATUS_FAILED;
    ioc_duplicate_handle(handle, srchandle);
    return ESTATUS_SUCCESS;
}


/**
****************************************************************************************************

  Try to finalize all needed signal structures.

  Lock must be on
  @return  None.

****************************************************************************************************
*/
eStatus eioBrickBuffer::try_finalize_setup()
{
    /* If setup is already good. We check state and cmd because they are in different
       memory blocks and last to be set up.
     */
    if (m_sig_state.handle->mblk && m_sig_state.flags &&
        m_sig_cmd.handle->mblk && m_sig_cmd.flags)
    {
        return ESTATUS_SUCCESS;
    }
    m_sig_state.flags = 0;
    m_sig_cmd.flags = 0;

    if (!m_flat_buffer) {
        if (try_signal_setup(&m_sig_select, "select", m_imp_ids.mblk_name))
            goto getout;
    }
    if (try_signal_setup(&m_sig_err, "err", m_exp_ids.mblk_name)) goto getout;

    if (m_from_device) {
        if (try_signal_setup(&m_sig_cs, "cs", m_exp_ids.mblk_name)) goto getout;
        if (try_signal_setup(&m_sig_buf, "buf", m_exp_ids.mblk_name)) goto getout;
        if (try_signal_setup(&m_sig_head, "head", m_exp_ids.mblk_name)) goto getout;
        if (!m_flat_buffer) {
            if (try_signal_setup(&m_sig_tail, "tail", m_imp_ids.mblk_name))
                goto getout;
        }
    }
    else {
        if (try_signal_setup(&m_sig_cs, "cs", m_imp_ids.mblk_name)) goto getout;
        if (try_signal_setup(&m_sig_buf, "buf", m_imp_ids.mblk_name)) goto getout;
        if (try_signal_setup(&m_sig_head, "head", m_imp_ids.mblk_name)) goto getout;
        if (!m_flat_buffer) {
            if (try_signal_setup(&m_sig_tail, "tail", m_exp_ids.mblk_name))
                goto getout;
        }
    }

    if (try_signal_setup(&m_sig_state, "state", m_exp_ids.mblk_name)) goto getout;
    if (try_signal_setup(&m_sig_cmd, "cmd", m_imp_ids.mblk_name)) goto getout;

    return ESTATUS_SUCCESS;

getout:
    return ESTATUS_FAILED;
}

/**
****************************************************************************************************

  @brief Release all resource allocated for the brick buffer.

  The eioBrickBuffer::cleanup( function...

****************************************************************************************************
*/
void eioBrickBuffer::cleanup()
{
    ioc_release_brick_buffer(&m_brick_buffer);

    if (m_h_exp.mblk) {
        ioc_release_handle(&m_h_exp);
    }

    if (m_h_imp.mblk) {
        ioc_release_handle(&m_h_imp);
    }

    clear_member_variables();
}


/**
****************************************************************************************************

  @brief Clear all brick buffer member variables.

  The eioBrickBuffer::cleanup( function...

****************************************************************************************************
*/
void eioBrickBuffer::clear_member_variables()
{
    os_memclear(&m_brick_buffer, sizeof(m_brick_buffer));

    os_memclear(&m_sig_cmd, sizeof(m_sig_cmd));
    os_memclear(&m_sig_select, sizeof(m_sig_select));
    os_memclear(&m_sig_err, sizeof(m_sig_err));
    os_memclear(&m_sig_cs, sizeof(m_sig_cs));
    os_memclear(&m_sig_buf, sizeof(m_sig_buf));
    os_memclear(&m_sig_head, sizeof(m_sig_head));
    os_memclear(&m_sig_tail, sizeof(m_sig_tail));
    os_memclear(&m_sig_state, sizeof(m_sig_state));

    os_memclear(&m_h_exp, sizeof(m_h_exp));
    os_memclear(&m_h_imp, sizeof(m_h_imp));

    os_memclear(&m_exp_ids, sizeof(m_exp_ids));
    os_memclear(&m_imp_ids, sizeof(m_imp_ids));

    os_memclear(m_prefix, sizeof(m_prefix));

    m_is_device = OS_FALSE;
    m_from_device = OS_TRUE;
    m_flat_buffer = OS_TRUE;
}
