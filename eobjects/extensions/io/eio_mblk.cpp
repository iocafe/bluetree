/**

  @file    eio_mblk.cpp
  @brief   Object representing and IO memory block.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

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
eioMblk::eioMblk(
    eObject *parent,
    e_oid oid,
    os_int flags)
    : eContainer(parent, oid, flags)
{
    os_memclear(&m_handle, sizeof(m_handle));
    m_handle_set = OS_FALSE;
    m_esignals = OS_NULL;
    m_mblk_flags = 0;
    m_eio_root = OS_NULL;
    m_connected = OS_FALSE;

    initproperties();
    ns_create();
}


/**
****************************************************************************************************
  Virtual destructor.
****************************************************************************************************
*/
eioMblk::~eioMblk()
{
    if (m_handle_set) {
        ioc_remove_callback(&m_handle, callback, this);
        ioc_release_handle(&m_handle);
        m_handle_set = OS_FALSE;
    }
}


/**
****************************************************************************************************

  @brief Add the class to class list and class'es properties to it's property set.

  The eioMblk::setupclass function adds the class to class list and class'es properties to
  it's property set. The class list enables creating new objects dynamically by class identifier,
  which is used for serialization reader functions. The property set stores static list of
  class'es properties and metadata for those.

****************************************************************************************************
*/
void eioMblk::setupclass()
{
    const os_int cls = ECLASSID_EIO_MBLK;

    /* Add the class to class list.
     */
    os_lock();
    eclasslist_add(cls, (eNewObjFunc)OS_NULL, "eioMblk", ECLASSID_CONTAINER);
    addpropertys(cls, ECONTP_TEXT, econtp_text, "text", EPRO_PERSISTENT|EPRO_NOONPRCH);
    addpropertyb(cls, EIOP_CONNECTED, eiop_connected, "connected", EPRO_SIMPLE|EPRO_RDONLY);
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
eStatus eioMblk::onpropertychange(
    os_int propertynr,
    eVariable *x,
    os_int flags)
{
    switch (propertynr)
    {
        case EIOP_CONNECTED:
            m_connected = (os_boolean)x->getl();
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
eStatus eioMblk::simpleproperty(
    os_int propertynr,
    eVariable *x)
{
    switch (propertynr)
    {
        case EIOP_CONNECTED:
            x->setl(m_connected);
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
void eioMblk::connected(
    struct eioMblkInfo *minfo)
{
    /* If we know the memory block pointer.
     */
    if (minfo->mblk) {
        /* Save/update memory block flags.
         */
        m_mblk_flags = minfo->mblk->flags;
        m_eio_root = minfo->eio_root;

        /* Set callback function, when "info" block is received.
         */
        if (os_strcmp(minfo->mblk_name, "info"))
        {
            if (m_handle_set) {
                ioc_remove_callback(&m_handle, callback, this);
                ioc_release_handle(&m_handle);
            }

            ioc_setup_handle(&m_handle, minfo->root, minfo->mblk);
            ioc_add_callback(&m_handle, callback, this);
            m_handle_set = OS_TRUE;
        }
    }

    /* Mark connected.
     */
    setpropertyl(EIOP_CONNECTED, OS_TRUE);
}


/**
****************************************************************************************************
  Mark IO network objects to disconnected and delete unused ones.
****************************************************************************************************
*/
void eioMblk::disconnected(
    eioMblkInfo *minfo)
{
    eObject *sig;

    /* Try read all "up" signals to mark these disconnected
     */
    if ((mblk_flags() & IOC_MBLK_DOWN) == 0 && m_esignals) {
        for (sig = m_esignals->first(); sig; sig = sig->next())
        {
            if (sig->classid() == ECLASSID_EIO_SIGNAL) {
                ((eioSignal*)sig)->up();
            }
        }
    }

    if (m_handle_set) {
        ioc_remove_callback(&m_handle, callback, this);
        ioc_release_handle(&m_handle);
        m_handle_set = OS_FALSE;
    }

    setpropertyl(EIOP_CONNECTED, OS_FALSE);
}


/**
****************************************************************************************************
  Get pointer to signal container, create if needed.
****************************************************************************************************
*/
eContainer *eioMblk::esignals()
{
    if (m_esignals == OS_NULL) {
        m_esignals = new eContainer(this);
        m_esignals->addname("signals");
        m_esignals->setpropertys(ECONTP_TEXT, "signals");
        m_esignals->ns_create();
    }
    return m_esignals;
}


void eioMblk::callback(
    struct iocHandle *handle,
    os_int start_addr,
    os_int end_addr,
    os_ushort flags,
    void *context)
{
    eObject *f, *l, *sig;
    eioMblk *t;
    eioSignal *p;
    eContainer *esignals;

    if ((flags & (IOC_MBLK_CALLBACK_RECEIVE|IOC_MBLK_CALLBACK_WRITE_TRIGGER|
        IOC_MBLK_CALLBACK_RECEIVE_TRIGGER)) == 0)
    {
        return;
    }

    os_lock();

    t = (eioMblk*)context;

    if (flags & (IOC_MBLK_CALLBACK_WRITE_TRIGGER|IOC_MBLK_CALLBACK_RECEIVE_TRIGGER))
    {
        if (t->m_eio_root) {
            t->m_eio_root->trig_io();
        }
    }

    if (flags & IOC_MBLK_CALLBACK_RECEIVE) {
        esignals = t->m_esignals;
        if (esignals == OS_NULL) {
            goto getout;
        }

        f = esignals->first(start_addr, OS_FALSE);
        if (f) {
            f = f->prev();
        }
        if (f == OS_NULL) {
            f = esignals->first(0, OS_FALSE);
            if (f == OS_NULL) goto getout;
        }

        l = esignals->first(end_addr, OS_FALSE);
        if (l == OS_NULL) {
            l = esignals->last();
            if (l == OS_NULL) goto getout;
        }

        sig = f;
        while (sig)
        {
            if (sig->classid() == ECLASSID_EIO_SIGNAL)
            {
                p = (eioSignal*)sig;
                if (ioc_is_my_address(p->iosignal(), start_addr, end_addr)) {
                    p->up();
                }
            }

            if (sig == l) {
                break;
            }
            sig = sig->next();
        }
    }
getout:
    os_unlock();
    return;
}
