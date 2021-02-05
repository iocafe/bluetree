/**

  @file    eio_group.cpp
  @brief   Object representing and IO group.
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
eioGroup::eioGroup(
    eObject *parent,
    e_oid oid,
    os_int flags)
    : eContainer(parent, oid, flags)
{
    initproperties();
}


/**
****************************************************************************************************
  Virtual destructor.
****************************************************************************************************
*/
eioGroup::~eioGroup()
{
}


/**
****************************************************************************************************

  @brief Clone object

  The eioGroup::clone function clones and object including object's children.
  Names will be left detached in clone.

  @param  parent Parent for the clone.
  @param  oid Object identifier for the clone.
  @param  aflags 0 for default operation. EOBJ_NO_MAP not to map names.
  @return Pointer to the clone.

****************************************************************************************************
*/
eObject *eioGroup::clone(
    eObject *parent,
    e_oid id,
    os_int aflags)
{
    eObject *clonedobj;
    clonedobj = new eioGroup(parent, id == EOID_CHILD ? oid() : id, flags());
    clonegeneric(clonedobj, aflags|EOBJ_CLONE_ALL_CHILDREN);
    return clonedobj;
}


/**
****************************************************************************************************

  @brief Add the class to class list and class'es properties to it's property set.

  The eioGroup::setupclass function adds the class to class list and class'es properties to
  it's property set. The class list enables creating new objects dynamically by class identifier,
  which is used for serialization reader functions. The property set stores static list of
  class'es properties and metadata for those.

****************************************************************************************************
*/
void eioGroup::setupclass()
{
    const os_int cls = ECLASSID_EIO_GROUP;

    /* Add the class to class list.
     */
    os_lock();
    eclasslist_add(cls, (eNewObjFunc)newobj, "eioGroup");
    addpropertys(cls, EIOP_TEXT, eiop_text, "text", EPRO_PERSISTENT);
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
void eioGroup::onmessage(
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
eStatus eioGroup::onpropertychange(
    os_int propertynr,
    eVariable *x,
    os_int flags)
{
    switch (propertynr)
    {
        case EIOP_TEXT:
            break;

        default:
            goto call_parent;
    }

    return ESTATUS_SUCCESS;

call_parent:
    return eContainer::onpropertychange(propertynr, x, flags);
}


/**
****************************************************************************************************

  @brief Process a callback from a child object.

  The eioGroup::oncallback function

****************************************************************************************************
*/
eStatus eioGroup::oncallback(
    eCallbackEvent event,
    eObject *obj,
    eObject *appendix)
{
    /* switch (event)
    {
        case ECALLBACK_VARIABLE_VALUE_CHANGED:
        case ECALLBACK_TABLE_CONTENT_CHANGED:
            touch();
            break;

        default:
            break;
    } */

    /* If we need to pass callback to parent class.
     */
    if (flags() & (EOBJ_PERSISTENT_CALLBACK|EOBJ_TEMPORARY_CALLBACK)) {
        eContainer::oncallback(event, obj, appendix);
    }

    return ESTATUS_SUCCESS;
}


/**
****************************************************************************************************

  @brief Flags the peristent object changed (needs to be saved).

  The eioGroup::touch function

****************************************************************************************************
*/
/* void eioGroup::touch()
{
    os_get_timer(&m_latest_touch);
    if (m_oldest_touch == 0) {
        m_oldest_touch = m_latest_touch;
    }

    set_timer(m_save_time);
}
*/
