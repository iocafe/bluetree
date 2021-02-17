/**

  @file    eio_signalassembly.cpp
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
#include "extensions/io/eio.h"


/**
****************************************************************************************************
  Constructor.
****************************************************************************************************
*/
eioSignalAssembly::eioSignalAssembly(
    eObject *parent,
    e_oid oid,
    os_int flags)
    : eioAssembly(parent, oid, flags)
{
    initproperties();
    ns_create();
}


/**
****************************************************************************************************
  Virtual destructor.
****************************************************************************************************
*/
eioSignalAssembly::~eioSignalAssembly()
{
}


/**
****************************************************************************************************

  @brief Add the class to class list and class'es properties to it's property set.

  The eioSignalAssembly::setupclass function adds the class to class list and class'es properties to
  it's property set. The class list enables creating new objects dynamically by class identifier,
  which is used for serialization reader functions. The property set stores static list of
  class'es properties and metadata for those.

****************************************************************************************************
*/
void eioSignalAssembly::setupclass()
{
    const os_int cls = ECLASSID_EIO_SIGNAL_ASSEMBLY;
    eVariable *v;

    os_lock();
    eclasslist_add(cls, (eNewObjFunc)OS_NULL, "eioSignalAssembly", ECLASSID_EIO_ASSEMBLY);
    addpropertys(cls, EVARP_TEXT, evarp_text, "text", EPRO_PERSISTENT|EPRO_NOONPRCH);
    addpropertys(cls, EVARP_VALUE, evarp_value, "value", EPRO_SIMPLE|EPRO_NOONPRCH);
    addpropertyb(cls, EIOP_BOUND, eiop_bound, "bound", EPRO_SIMPLE|EPRO_RDONLY);
    addpropertys(cls, EIOP_ASSEMBLY_TYPE, eiop_assembly_type, "assembly type", EPRO_PERSISTENT|EPRO_NOONPRCH);
    addpropertys(cls, EIOP_ASSEMBLY_EXP, eiop_assembly_exp, "exp", EPRO_PERSISTENT|EPRO_NOONPRCH);
    addpropertys(cls, EIOP_ASSEMBLY_IMP, eiop_assembly_imp, "imp", EPRO_PERSISTENT|EPRO_NOONPRCH);
    addpropertys(cls, EIOP_ASSEMBLY_PREFIX, eiop_assembly_prefix, "prefix", EPRO_PERSISTENT|EPRO_NOONPRCH);
    v = addpropertyl(cls, EIOP_ASSEMBLY_TIMEOUT, eiop_assembly_timeout, "timeout", EPRO_PERSISTENT|EPRO_NOONPRCH);
    v->setpropertys(EVARP_UNIT, "ms");
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
eStatus eioSignalAssembly::onpropertychange(
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
eStatus eioSignalAssembly::simpleproperty(
    os_int propertynr,
    eVariable *x)
{
    switch (propertynr)
    {
        case EVARP_VALUE:
            // x->setv(m_output);
            break;

        default:
            return eioAssembly::simpleproperty(propertynr, x);
    }
    return ESTATUS_SUCCESS;
}


/**
****************************************************************************************************

  @brief Prepare a newly created signal assembly assembly for use.

  The eioSignalAssembly::setup function...

****************************************************************************************************
*/
eStatus eioSignalAssembly::setup(
    eioAssemblyParams *prm,
    iocRoot *iocom_root)
{
    OSAL_UNUSED(iocom_root);
    setpropertys(EVARP_TEXT, prm->name);
    setpropertys(EIOP_ASSEMBLY_TYPE, prm->type_str);
    setpropertys(EIOP_ASSEMBLY_EXP, prm->exp_str);
    setpropertys(EIOP_ASSEMBLY_IMP, prm->imp_str);
    setpropertys(EIOP_ASSEMBLY_PREFIX, prm->prefix);
    if (prm->timeout_ms) {
        setpropertyl(EIOP_ASSEMBLY_TIMEOUT, prm->timeout_ms);
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
/* eStatus eioSignalAssembly::try_signal_setup(
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
} */


/**
****************************************************************************************************

  @brief Call repeatedly

  lock must be on

****************************************************************************************************
*/
void eioSignalAssembly::run(os_long ti)
{
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
void eioSignalAssembly::object_info(
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
void eioSignalAssembly::send_open_info(
    eEnvelope *envelope)
{
    eContainer *reply;
    eVariable *item, tmp;
    // eName *name;
    // eioDevice *device;

    /* Brick buffer title text has device name and signal assembly name.
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

    item = new eVariable(reply, ECLASSID_EIO_SIGNAL_ASSEMBLY);
    item->sets("_p/x");

    /* Send reply to caller
     */
    message(ECMD_OPEN_REPLY, envelope->source(),
        envelope->target(), reply, EMSG_DEL_CONTENT, envelope->context());
}
