/**

  @file    eio_variable.cpp
  @brief   IO variable class.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    2.10.2020

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
eioVariable::eioVariable(
    eObject *parent,
    e_oid id,
    os_int flags)
    : eVariable(parent, id, flags)
{
    m_down_ref = OS_NULL;
    m_my_own_change = 0;
    m_value_set_by_user = OS_FALSE;
    m_bound = OS_FALSE;
}


/**
****************************************************************************************************

  @brief Add eioVariable to class list and class'es properties to it's property set.

  The eioVariable::setupclass function adds eioVariable to class list and class'es properties to
  it's property set. The class list enables creating new objects dynamically by class identifier,
  which is used for serialization reader functions. The property set stores static list of
  class'es properties and metadata for those.

****************************************************************************************************
*/
void eioVariable::setupclass()
{
    eVariable *v;
    const os_int cls = ECLASSID_EIO_VARIABLE;

    os_lock();
    eclasslist_add(cls, (eNewObjFunc)OS_NULL, "eioVariable", ECLASSID_VARIABLE);
    eVariable::setupproperties(cls);
    addproperty(cls, EVARP_SBITS, evarp_sbits, "state bits", EPRO_PERSISTENT|EPRO_SIMPLE);
    v = addproperty(cls, EVARP_TSTAMP, evarp_tstamp, "timestamp", EPRO_PERSISTENT|EPRO_SIMPLE);
    v->setpropertys(EVARP_ATTR, "tstamp=\"yy,msec\"");
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
eStatus eioVariable::onpropertychange(
    os_int propertynr,
    eVariable *x,
    os_int flags)
{
    eStatus s;

    switch (propertynr)
    {
        case EVARP_VALUE:
            m_value_set_by_user = !m_my_own_change;
            s = eVariable::onpropertychange(propertynr, x, flags);

            if (m_value_set_by_user) {
                down();
            }
            return s;

        case EIOP_BOUND:
            m_bound = (os_boolean)x->getl();
            break;

        default:
            return eVariable::onpropertychange(propertynr, x, flags);
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
eStatus eioVariable::simpleproperty(
    os_int propertynr,
    eVariable *x)
{
    switch (propertynr)
    {
        case EIOP_BOUND:
            x->setl(m_bound);
            break;

        default:
            return eVariable::simpleproperty(propertynr, x);
    }
    return ESTATUS_SUCCESS;
}


/**
****************************************************************************************************

  @brief Process a callback from a child object.

  This is used to maintain "bound" property of the IO variable, so that it is OS_TROE is
  someone is bound (looking at) the IO variable. This is used to delete disconnected
  iocDevice objects one they are no longer needed (bound).

****************************************************************************************************
*/
eStatus eioVariable::oncallback(
    eCallbackEvent event,
    eObject *obj,
    eObject *appendix)
{
    switch (event)
    {
        case ECALLBACK_SERVER_BINDING_CONNECTED:
        case ECALLBACK_SERVER_BINDING_DISCONNECTED:
            set_bound(event);
            return ESTATUS_SUCCESS;

        default:
            break;
    }

    /* If we need to pass callback to parent class.
     */
    if (flags() & (EOBJ_PERSISTENT_CALLBACK|EOBJ_TEMPORARY_CALLBACK)) {
        eVariable::oncallback(event, obj, appendix);
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
void eioVariable::set_bound(
    eCallbackEvent event)
{
    os_boolean b;
    eObject *gp;

    b = is_bound();
    if (b != m_bound) {
        setpropertyl(EIOP_BOUND, b);
        gp = grandparent();
        if (gp) gp->oncallback(event, parent(), OS_NULL);
    }
}


void eioVariable::setup(
    class eioSignal *signal,
    struct eioMblkInfo *minfo,
    struct eioSignalInfo *sinfo)
{
    os_short mblk_flags;
    os_int type_id;
    os_long min_value, max_value;
    os_boolean is_rdonly;

    mblk_flags = signal->mblk_flags();

    type_id = (sinfo->flags & OSAL_TYPEID_MASK);
    if (OSAL_IS_INTEGER_TYPE(type_id))
    {
        osal_type_range((osalTypeId)type_id, &min_value, &max_value);
        if (max_value > min_value) {
            setpropertyl(EVARP_MIN, min_value);
            setpropertyl(EVARP_MAX, max_value);
        }
        type_id = OS_LONG;
    }
    setpropertyl(EVARP_TYPE, type_id);

    if (mblk_flags & IOC_MBLK_DOWN) {
        if (m_down_ref == OS_NULL) {
            m_down_ref = new ePointer(this);
        }

        m_down_ref->set(signal);

        if (m_value_set_by_user && (mblk_flags & IOC_MBLK_UP) == 0) {
            down();
        }
    }

    is_rdonly = OS_TRUE;
    if (m_down_ref) if (m_down_ref->get()) {
        is_rdonly = OS_FALSE;
    }
    setpropertys(EVARP_ATTR, is_rdonly ? "rdonly" : "");
}

/* Adopts x.
 * os_lock() must be on when thi when this function is called.
 */
void eioVariable::up(eValueX *x)
{
    m_my_own_change++;
    setpropertyo(EVARP_VALUE, x, EMSG_DEL_CONTENT);
    m_my_own_change--;
}


void eioVariable::down()
{
    eioSignal *sig;

    if (m_down_ref) {
        sig = eioSignal::cast(m_down_ref->get());
        if (sig) {
            sig->down(this);
        }
    }
}
