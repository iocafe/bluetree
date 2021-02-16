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
    m_output = new eVariable(this);
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
    eclasslist_add(cls, (eNewObjFunc)OS_NULL, "eioBrickBuffer", ECLASSID_EIO_ASSEMBLY);
    addpropertys(cls, EVARP_TEXT, evarp_text, "text", EPRO_PERSISTENT|EPRO_NOONPRCH);
    addpropertys(cls, EVARP_VALUE, evarp_value, "value", EPRO_SIMPLE|EPRO_NOONPRCH);
    addpropertyb(cls, EIOP_BOUND, eiop_bound, "bound", EPRO_SIMPLE);
    propertysetdone(cls);

    propertysetdone(cls);
    os_unlock();
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
    switch (propertynr)
    {
        case EVARP_VALUE:
            break;

        default:
            return eioAssembly::onpropertychange(propertynr, x, flags);
    }

    return ESTATUS_SUCCESS;
}


/**
****************************************************************************************************

  @brief Get value of simple property (override).

  The simpleproperty() function stores current value of simple property into variable x.

  @param   propertynr Property number to get.
  @param   x Variable into which to store the property value.
  @return  If property with property number was stored in x, the function returns
           ESTATUS_SUCCESS (0). Nonzero return values indicate that property with
           given number was not among simple properties.

****************************************************************************************************
*/
eStatus eioBrickBuffer::simpleproperty(
    os_int propertynr,
    eVariable *x)
{
    switch (propertynr)
    {
        case EVARP_VALUE:
            x->setv(m_output);
            break;

        default:
            return eioAssembly::simpleproperty(propertynr, x);
    }
    return ESTATUS_SUCCESS;
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
    eioRoot *root;
    const char *p;

    /* Start from beginning, clean all.
     */
    cleanup();

    /* Determine flags
     */
    m_is_device = OS_FALSE;
    m_from_device = OS_TRUE;
    m_is_camera = OS_FALSE;
    p = prm->type_str;
    if (!os_strcmp(p, "cam_flat")) {
        m_flat_buffer = OS_TRUE;
        m_is_camera = OS_TRUE;
    }
    else if (os_strcmp(p, "lcam_flat")) {
        m_flat_buffer = OS_TRUE;
    }
    else if (!os_strcmp(p, "cam_ring")) {
        m_flat_buffer = OS_FALSE;
        m_is_camera = OS_TRUE;
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

    root = eioRoot::cast(grandparent()->grandparent());
    if (root) {
        root->assembly_to_run_list(this, OS_TRUE);
    }

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
    eContainer *mblks;
    eioMblk *mblk;
    eioSignal *eiosig;
    iocHandle *handle, *srchandle;

    os_char signal_name[IOC_SIGNAL_NAME_SZ];

    device = eioDevice::cast(grandparent());
    mblks = device->mblks();
    if (mblks == OS_NULL) return ESTATUS_FAILED;
    mblk = eioMblk::cast(mblks->byname(mblk_name));
    if (mblk == OS_NULL) return ESTATUS_FAILED;

    os_strncpy(signal_name, m_prefix, sizeof(signal_name));
    os_strncat(signal_name, name, sizeof(signal_name));

    eiosig = eioSignal::cast(mblk->esignals()->byname(signal_name));
    if (eiosig == OS_NULL) return ESTATUS_FAILED;

    sig->addr = eiosig->io_addr();
    sig->n = eiosig->io_n();
    sig->flags = eiosig->io_flags();

    handle = sig->handle;
    if (handle->mblk) return ESTATUS_SUCCESS;

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

  @brief Call repeatedly

  lock must be on

****************************************************************************************************
*/
void eioBrickBuffer::run(os_long ti)
{
    os_boolean get_data;

    get_data = m_from_device;
    if (m_is_device) get_data = !get_data;

    if (get_data) {
        /* Enable or disable reciving data, if someone is bound to the outout.
         */
        ioc_brick_set_receive(&m_brick_buffer, is_bound());

        /* Receive data
         */
        get();
    }
}


/**
****************************************************************************************************

  @brief Get brick from this buffer.

****************************************************************************************************
*/
eStatus eioBrickBuffer::get()
{
    eBitmap *bitmap;
    iocBrickHdr *hdr;
    os_uchar *data, *dst;
    os_memsz buf_sz, data_sz;
    osalBitmapFormat format;
    os_uchar compression;
    os_int width, height, y;
    os_int pixel_nbytes, dst_row_nbytes, src_row_nbytes, copy_nbytes;
    osalStatus s;

    /* Setup all signals, if we have not done that already.
     */
    if (try_finalize_setup()) {
        return ESTATUS_PENDING;
    }

    /* Receive data, return ESTATUS_SUCCESS if we got no data.
     */
    s = ioc_run_brick_receive(&m_brick_buffer);
    buf_sz = m_brick_buffer.buf_sz;
    if (s != OSAL_COMPLETED || buf_sz <= (os_memsz)sizeof(iocBrickHdr)) {
        return ESTATUS_FROM_OSAL_STATUS(s);
    }

    data = m_brick_buffer.buf + sizeof(iocBrickHdr);
    data_sz = buf_sz - sizeof(iocBrickHdr);
    hdr = (iocBrickHdr*)m_brick_buffer.buf;

    format = (osalBitmapFormat)hdr->format;
    compression = hdr->compression;
    width = (os_int)ioc_get_brick_hdr_int(hdr->width, IOC_BRICK_DIM_SZ);
    height = (os_int)ioc_get_brick_hdr_int(hdr->height, IOC_BRICK_DIM_SZ);

    if (m_is_camera) { // camera
        bitmap = new eBitmap(ETEMPORARY);
        bitmap->allocate(format, width, height, EBITMAP_NO_NEW_MEMORY_ALLOCATION);

        if (compression == IOC_UNCOMPRESSED)
        {
            /* Handle bitmap row alignment when copying.
             */
            pixel_nbytes = bitmap->pixel_nbytes();
            dst_row_nbytes = bitmap->row_nbytes();
            src_row_nbytes = pixel_nbytes * width;
            if (pixel_nbytes == 3) {
                src_row_nbytes = (src_row_nbytes + 1) / 2;
                src_row_nbytes *= 2;
            }
            dst = bitmap->ptr();

            if (src_row_nbytes == dst_row_nbytes) {
                os_memcpy(dst, data, height * src_row_nbytes);
            }
            else {
                copy_nbytes = dst_row_nbytes;
                if (src_row_nbytes < copy_nbytes) copy_nbytes = src_row_nbytes;

                for (y = 0; y < height; y++) {
                    os_memcpy(dst, data, copy_nbytes);
                    os_memclear(dst + copy_nbytes, dst_row_nbytes - copy_nbytes);
                    dst += dst_row_nbytes;
                    data += src_row_nbytes;
                }
            }
        }
        else if (compression & IOC_JPEG)
        {
            bitmap->set_jpeg_data(data, data_sz, OS_FALSE);
        }
        else
        {
            osal_debug_error_int("unsupported brick compression = ", compression);
            return ESTATUS_FAILED;
        }

        /* Set output and forward property value to bindings, if any.
         */
        m_output->seto(bitmap, OS_TRUE);
        forwardproperty(EVARP_VALUE, m_output, OS_NULL, 0);
    }

    return ESTATUS_SUCCESS;
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
    m_is_camera = OS_FALSE;
}


/**
****************************************************************************************************

  @brief Collect information about this object for tree browser.

  The eVariable::object_info function fills in item (eVariable) to contain information
  about this object for tree browser view.

  @param   item Pointer to eVariable to set up with object information.
  @param   name Object's name if known. OS_NULL if object is not named or name is
           unknown at this time.
  @param   appendix Pointer to eSet into which to store property flags. The stored property
           flags indicate if object has namespace, children, or properties.
  @param   target Path "within object" when browsing a tree which is not made out
           of actual eObjects. For example OS file system directory.

****************************************************************************************************
*/
void eioBrickBuffer::object_info(
    eVariable *item,
    eVariable *name,
    eSet *appendix,
    const os_char *target)
{
    eioAssembly::object_info(item, name, appendix, target);
    appendix->setl(EBROWSE_RIGHT_CLICK_SELECTIONS, EBROWSE_CAMERA);
}


/**
****************************************************************************************************

  @brief Information for opening object has been requested, send it.

  The object has received ECMD_INFO request and it needs to return back information
  for opening the object.

  @param   envelope Message envelope. Contains command, target and source paths and
           message content, etc.
  @return  None.

****************************************************************************************************
*/
void eioBrickBuffer::send_open_info(
    eEnvelope *envelope)
{
    eContainer *reply;
    eVariable *item, tmp;
    // eName *name;
    // eioDevice *device;

    /* Brick buffer title text has device name and brick buffer name.
     */
    /* device = eioDevice::cast(grandparent());
    name = device->primaryname();
    if (name) {
        tmp = *name;
        tmp += " ";
    }
    name = primaryname();
    if (name) {
        tmp += *name;
    } */
    propertyv(EVARP_TEXT, &tmp);

    /* Show properties regardless of command.
     */
    reply = new eContainer(this, EOID_ITEM, EOBJ_IS_ATTACHMENT);
    reply->setpropertyv(ECONTP_TEXT, &tmp);

    /* Open as "camera view" fron the browser.
     */
    item = new eVariable(reply, EOID_PARAMETER);
    item->setl(EBROWSE_CAMERA);

    item = new eVariable(reply, ECLASSID_EIO_BRICK_BUFFER);
    item->sets("_p/x");

    /* Send reply to caller
     */
    message(ECMD_OPEN_REPLY, envelope->source(),
        envelope->target(), reply, EMSG_DEL_CONTENT, envelope->context());
}
