/**

  @file    eio_mblk.cpp
  @brief   Object representing and IO memory block.
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
    eclasslist_add(cls, (eNewObjFunc)newobj, "eioMblk", ECLASSID_CONTAINER);
    addpropertys(cls, ECONTP_TEXT, econtp_text, "text", EPRO_PERSISTENT|EPRO_NOONPRCH);
    addpropertyb(cls, EIOP_CONNECTED, eiop_connected, OS_TRUE, "connected", EPRO_PERSISTENT);
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
void eioMblk::onmessage(
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
eStatus eioMblk::onpropertychange(
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

  The eioMblk::oncallback function

****************************************************************************************************
*/
eStatus eioMblk::oncallback(
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
void eioMblk::connected(
    struct eioMblkInfo *minfo)
{
    /* If we kno the memory block pointer.
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


/**
****************************************************************************************************

  @brief Flags the peristent object changed (needs to be saved).

  The eioMblk::touch function

****************************************************************************************************
*/
/* void eioMblk::touch()
{
    os_get_timer(&m_latest_touch);
    if (m_oldest_touch == 0) {
        m_oldest_touch = m_latest_touch;
    }

    set_timer(m_save_time);
}
*/

