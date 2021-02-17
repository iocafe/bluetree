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
    m_bound = OS_FALSE;
    m_connected = OS_FALSE;
    initproperties();
    ns_create();
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
    eclasslist_add(cls, (eNewObjFunc)OS_NULL, "eioDevice", ECLASSID_CONTAINER);
    addpropertys(cls, ECONTP_TEXT, econtp_text, "text", EPRO_PERSISTENT|EPRO_NOONPRCH);
    addpropertyb(cls, EIOP_CONNECTED, eiop_connected, "connected", EPRO_SIMPLE|EPRO_RDONLY);
    addpropertyb(cls, EIOP_BOUND, eiop_bound, "bound", EPRO_SIMPLE|EPRO_RDONLY);
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
            m_connected = (os_boolean)x->getl();
            break;

        case EIOP_BOUND:
            m_bound = (os_boolean)x->getl();
            break;

        default:
            return eContainer::onpropertychange(propertynr, x, flags);
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
eStatus eioDevice::simpleproperty(
    os_int propertynr,
    eVariable *x)
{
    switch (propertynr)
    {
        case EIOP_CONNECTED:
            x->setl(m_connected);
            break;

        case EIOP_BOUND:
            x->setl(m_bound);
            break;

        default:
            return eContainer::simpleproperty(propertynr, x);
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


/**
****************************************************************************************************
  Get "assemblies" container, create it if it doesn't exist.
****************************************************************************************************
*/
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


/**
****************************************************************************************************

  @brief Process a callback from a child object.

  This is used to maintain "bound" property of the IO variable, so that it is OS_TROE is
  someone is bound (looking at) the IO variable. This is used to delete disconnected
  iocDevice objects one they are no longer needed (bound).

****************************************************************************************************
*/
eStatus eioDevice::oncallback(
    eCallbackEvent event,
    eObject *obj,
    eObject *appendix)
{
    switch (event)
    {
        case ECALLBACK_SERVER_BINDING_CONNECTED:
        case ECALLBACK_SERVER_BINDING_DISCONNECTED:
            if (obj == m_io ||
                obj == m_assemblies)
            {
                set_bound(event);
            }
            return ESTATUS_SUCCESS;

        default:
            break;
    }

    /* If we need to pass callback to parent class.
     */
    if (flags() & (EOBJ_PERSISTENT_CALLBACK|EOBJ_TEMPORARY_CALLBACK)) {
        return eContainer::oncallback(event, obj, appendix);
    }
    return ESTATUS_SUCCESS;
}


/**
****************************************************************************************************

  @brief Decide value for "bound" flag.

  This function is called by oncallback() when server side binding is established or
  disconnected. The task of this function is to maintain is_bound flag.

****************************************************************************************************
*/
void eioDevice::set_bound(
    eCallbackEvent event)
{
    eObject *item;
    os_boolean b;

    b = OS_FALSE;
    for (item = m_io->first(); item && !b; item = item->next()) {
        if (!item->isinstanceof(ECLASSID_EIO_VARIABLE)) continue;
        b = item->propertyl(EIOP_BOUND);
    }
    for (item = m_assemblies->first(); item && !b; item = item->next()) {
        if (!item->isinstanceof(ECLASSID_EIO_ASSEMBLY)) continue;
        b = item->propertyl(EIOP_BOUND);
    }

    /*
    if (!b) b = is_bound();
     */

    if (b != m_bound) {
        setpropertyl(EIOP_BOUND, b);
        // gp = grandparent();
        // if (gp) gp->oncallback(event, parent(), OS_NULL);
    }
}
