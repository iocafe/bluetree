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
    : eVariable(parent, oid, flags)
{
    m_bound = OS_FALSE;
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
    eclasslist_add(cls, (eNewObjFunc)OS_NULL, "eioAssembly", ECLASSID_VARIABLE);
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
eStatus eioAssembly::onpropertychange(
    os_int propertynr,
    eVariable *x,
    os_int flags)
{
    eStatus s;
    os_long tstamp;
    os_int sbits;

    switch (propertynr)
    {
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
eStatus eioAssembly::simpleproperty(
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
eStatus eioAssembly::oncallback(
    eCallbackEvent event,
    eObject *obj,
    eObject *appendix)
{
    switch (event)
    {
        case ECALLBACK_SERVER_BINDING_CONNECTED:
        case ECALLBACK_SERVER_BINDING_DISCONNECTED:
            set_bound();
            break;

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
void eioAssembly::set_bound()
{
    os_boolean b;

    b = is_bound();
    if (b != m_bound) {
        setpropertyl(EIOP_BOUND, b);
    }
}
