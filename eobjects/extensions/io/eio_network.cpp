/**

  @file    eio_network.cpp
  @brief   Object representing and IO network.
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
eioNetwork::eioNetwork(
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

  @brief Add the class to class list and class'es properties to it's property set.

  The eioNetwork::setupclass function adds the class to class list and class'es properties to
  it's property set. The class list enables creating new objects dynamically by class identifier,
  which is used for serialization reader functions. The property set stores static list of
  class'es properties and metadata for those.

****************************************************************************************************
*/
void eioNetwork::setupclass()
{
    const os_int cls = ECLASSID_EIO_NETWORK;

    /* Add the class to class list.
     */
    os_lock();
    eclasslist_add(cls, (eNewObjFunc)OS_NULL, "eioNetwork", ECLASSID_CONTAINER);
    addpropertys(cls, ECONTP_TEXT, econtp_text, "text", EPRO_PERSISTENT|EPRO_NOONPRCH);
    addpropertyb(cls, EIOP_CONNECTED, eiop_connected, OS_TRUE, "connected", EPRO_PERSISTENT);
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
eStatus eioNetwork::onpropertychange(
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
  Create IO network objects to represent connection.
****************************************************************************************************
*/
eioMblk *eioNetwork::connected(
    struct eioMblkInfo *minfo)
{
    eioDevice *device;
    eioMblk *mblk;
    os_char buf[IOC_DEVICE_ID_SZ], nbuf[OSAL_NBUF_SZ];

    if (minfo->device_name[0] == '\0') {
        return OS_NULL;
    }

    os_strncpy(buf, minfo->device_name, sizeof(buf));
    osal_int_to_str(nbuf, sizeof(nbuf), minfo->device_nr);
    os_strncat(buf, nbuf, sizeof(buf));

    device = get_device(buf);

    mblk = device->connected(minfo);
    setpropertyl(EIOP_CONNECTED, OS_TRUE);
    return mblk;
}


eioDevice *eioNetwork::get_device(
    const os_char *device_id)
{
    eioDevice *device;

    device = eioDevice::cast(byname(device_id));
    if (device == OS_NULL) {
        eVariable tmp;
        device = new eioDevice(this);
        tmp = device_id;
        tmp += " IO device";
        device->setpropertyv(ECONTP_TEXT, &tmp);
        device->addname(device_id);
    }
    return device;
}


/**
****************************************************************************************************
  Mark IO network objects to disconnected and delete unused ones.
****************************************************************************************************
*/
void eioNetwork::disconnected(
    eioMblkInfo *minfo)
{
    eioDevice *device;
    os_char buf[IOC_DEVICE_ID_SZ], nbuf[OSAL_NBUF_SZ];

    os_strncpy(buf, minfo->device_name, sizeof(buf));
    osal_int_to_str(nbuf, sizeof(nbuf), minfo->device_nr);
    os_strncat(buf, nbuf, sizeof(buf));

    device = eioDevice::cast(byname(buf));
    if (device) {
        device->disconnected(minfo);
    }

    for (device = eioDevice::cast(first());
         device;
         device = eioDevice::cast(device->next()))
    {
        if (device->propertyl(EIOP_CONNECTED)) {
            return;
        }
    }

    setpropertyl(EIOP_CONNECTED, OS_FALSE);
}

