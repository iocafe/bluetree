/**

  @file    eio_device.cpp
  @brief   Object representing and IO device.
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
eioDevice::eioDevice(
    eObject *parent,
    e_oid oid,
    os_int flags)
    : eContainer(parent, oid, flags)
{
    m_mblks = m_io = m_assemblies = OS_NULL;;
    initproperties();
    ns_create();
}


/**
****************************************************************************************************
  Virtual destructor.
****************************************************************************************************
*/
eioDevice::~eioDevice()
{
}


/**
****************************************************************************************************

  @brief Add the class to class list and class'es properties to it's property set.

  The eioDevice::setupclass function adds the class to class list and class'es properties to
  it's property set. The class list enables creating new objects dynamically by class identifier,
  which is used for serialization reader functions. The property set stores static list of
  class'es properties and metadata for those.

****************************************************************************************************
*/
void eioDevice::setupclass()
{
    const os_int cls = ECLASSID_EIO_DEVICE;

    /* Add the class to class list.
     */
    os_lock();
    eclasslist_add(cls, (eNewObjFunc)newobj, "eioDevice", ECLASSID_CONTAINER);
    addpropertys(cls, ECONTP_TEXT, econtp_text, "text", EPRO_PERSISTENT|EPRO_NOONPRCH);
    addpropertyl(cls, EIOP_CONNECTED, eiop_connected, OS_TRUE, "connected", EPRO_PERSISTENT);
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
void eioDevice::onmessage(
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
eStatus eioDevice::onpropertychange(
    os_int propertynr,
    eVariable *x,
    os_int flags)
{
    switch (propertynr)
    {
        case EIOP_CONNECTED:
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

  The eioDevice::oncallback function

****************************************************************************************************
*/
eStatus eioDevice::oncallback(
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
  Create IO network objects to represent connection.
****************************************************************************************************
*/
eioMblk *eioDevice::connected(
    struct eioMblkInfo *minfo)
{
    eioMblk *mblk;

    if (minfo->mblk_name[0] == '\0') {
        return OS_NULL;
    }

    if (m_mblks == OS_NULL) {
        eVariable tmp;
        eName *name;

        m_mblks = new eContainer(this);

        name = primaryname();
        if (name) {
            tmp = *name;
            tmp += " mblks";
            m_mblks->setpropertyv(ECONTP_TEXT, &tmp);
        }
        m_mblks->addname("mblks");
        m_mblks->ns_create();
    }

    mblk = eioMblk::cast(m_mblks->byname(minfo->mblk_name));
    if (mblk == OS_NULL) {
        mblk = new eioMblk(m_mblks);
        mblk->addname(minfo->mblk_name);
    }

    mblk->connected(minfo);
    setpropertyl(EIOP_CONNECTED, OS_TRUE);
    return mblk;
}


/**
****************************************************************************************************
  Mark IO network objects to disconnected and delete unused ones.
****************************************************************************************************
*/
void eioDevice::disconnected(
    eioMblkInfo *minfo)
{
    eioMblk *mblk;

    if (m_mblks == OS_NULL) {
        return;
    }

    mblk = eioMblk::cast(m_mblks->byname(minfo->mblk_name));
    if (mblk) {
        mblk->disconnected(minfo);
    }

    for (mblk = eioMblk::cast(m_mblks->first());
         mblk;
         mblk = eioMblk::cast(mblk->next()))
    {
        if (mblk->propertyl(EIOP_CONNECTED)) {
            return;
        }
    }

    setpropertyl(EIOP_CONNECTED, OS_FALSE);
}

eContainer *eioDevice::io()
{
    if (m_io == OS_NULL)
    {
        eVariable tmp;
        eName *name;

        m_io = new eContainer(this);
        m_io->addname("io");
        m_io->ns_create();

        name = primaryname();
        if (name) {
            tmp = *name;
            tmp += " IO";
            m_io->setpropertyv(ECONTP_TEXT, &tmp);
        }
    }
    return m_io;
}

eContainer *eioDevice::assemblies()
{
    if (m_assemblies == OS_NULL)
    {
        eVariable tmp;
        eName *name;

        m_assemblies = new eContainer(this);
        m_assemblies->addname("assembly");
        m_assemblies->ns_create();

        name = primaryname();
        if (name) {
            tmp = *name;
            tmp += " assy";
            m_assemblies->setpropertyv(ECONTP_TEXT, &tmp);
        }

    }
    return m_assemblies;
}
