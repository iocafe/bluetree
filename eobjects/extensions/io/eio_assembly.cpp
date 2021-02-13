/**

  @file    eio_assembly.cpp
  @brief   Assembly - collection of signals with specific functionality.
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
eioAssembly::eioAssembly(
    eObject *parent,
    e_oid oid,
    os_int flags)
    : eContainer(parent, oid, flags)
{
    initproperties();
    ns_create();
}


/**
****************************************************************************************************
  Virtual destructor.
****************************************************************************************************
*/
eioAssembly::~eioAssembly()
{
}


/**
****************************************************************************************************

  @brief Add the class to class list and class'es properties to it's property set.

  The eioAssembly::setupclass function adds the class to class list and class'es properties to
  it's property set. The class list enables creating new objects dynamically by class identifier,
  which is used for serialization reader functions. The property set stores static list of
  class'es properties and metadata for those.

****************************************************************************************************
*/
void eioAssembly::setupclass()
{
    const os_int cls = ECLASSID_EIO_ASSEMBLY;

    /* Add the class to class list.
     */
    os_lock();
    eclasslist_add(cls, (eNewObjFunc)OS_NULL, "eioAssembly", ECLASSID_CONTAINER);
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
void eioAssembly::onmessage(
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
    eContainer::onmessage(envelope);
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
eStatus eioAssembly::onpropertychange(
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
    return eContainer::onpropertychange(propertynr, x, flags);
}
