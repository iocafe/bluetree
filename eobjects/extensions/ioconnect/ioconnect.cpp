/**

  @file    ioconnect.h
  @brief   Connects to iocom device.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    14.9.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "ioconnect.h"

/* Variable property names.
 */
const os_char
    ioconp_path[] = "path";


/**
****************************************************************************************************
  Constructor.
****************************************************************************************************
*/
ioConnect::ioConnect(
    eObject *parent,
    e_oid id,
    os_int flags)
    : eObject(parent, id, flags)
{
    /* No type, number 2 digits after decimal point for doubles.
     */
    // m_vflags = OS_UNDEFINED_TYPE|(2 << EVAR_DDIGS_SHIFT);
    // m_value.valbuf.tmpstr = OS_NULL;
}


/**
****************************************************************************************************
  Virtual destructor.
****************************************************************************************************
*/
ioConnect::~ioConnect()
{
    /* Release any allocated memory.
     */
    // clear();
}


/**
****************************************************************************************************

  @brief Clone object

  The clone function clones the variable and clonable attachments. Names will be left detached
  in clone if EOBJ_NO_MAP flag is given.

  @param  parent Parent for the clone.
  @param  id Object identifier for the clone.
  @param  aflags 0 for default operation. EOBJ_NO_MAP not to map names.
  @return Pointer to the clone.

****************************************************************************************************
*/
eObject *ioConnect::clone(
    eObject *parent,
    e_oid id,
    os_int aflags)
{
    ioConnect *clonedobj;
    clonedobj = new ioConnect(parent, id == EOID_CHILD ? oid() : id, flags());

    /** Copy variable value.
     */
    // clonedobj->setv(this);
    // clonedobj->setdigs(digs());

    /* Copy clonable attachments.
     */
    clonegeneric(clonedobj, aflags);
    return clonedobj;
}


/**
****************************************************************************************************

  @brief Add ioConnect to class list and class'es properties to it's property set.

  The ioConnect::setupclass function adds ioConnect to class list and class'es properties to
  it's property set. The class list enables creating new objects dynamically by class identifier,
  which is used for serialization reader functions. The property set stores static list of
  class'es properties and metadata for those.

****************************************************************************************************
*/
void ioConnect::setupclass()
{
    const os_int cls = IOCONNCLASSID_CONNECT;

    /* Add the class to class list.
     */
    os_lock();
    eclasslist_add(cls, (eNewObjFunc)newobj, "ioConnect");
    setupproperties(cls);
    propertysetdone(cls);
    os_unlock();
}


/**
****************************************************************************************************

  @brief Add class'es properties to property set.

  The ioConnect::setupproperties is helper function for setupclass, it is called from both
  ioConnect class and derived classes like eName.

  Process mutex must be locked when calling this function.

****************************************************************************************************
*/
void ioConnect::setupproperties(
    os_int cls)
{
    // eVariable *p;

    /* Order of these addproperty() calls is important, since ioConnect itself is used to
       describe the properties in property set. The property to set must be added to
       property set before setting value for it. There is trick with p to set text type
       after adding property type. This effects only ioConnect class.
     */
    /* p = */ addproperty(cls, IOCONP_TEXT, ioconp_text, "text", EPRO_METADATA|EPRO_NOONPRCH);
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
eStatus ioConnect::onpropertychange(
    os_int propertynr,
    eVariable *x,
    os_int flags)
{
    switch (propertynr)
    {
/*         case IOCONP_VALUE:
            setv(x);
            break;

        case IOCONP_DIGS:
            setdigs((os_int)x->getl());
            break; */

        default:
            return eObject::onpropertychange(propertynr, x, flags);
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
eStatus ioConnect::simpleproperty(
    os_int propertynr,
    eVariable *x)
{
    switch (propertynr)
    {
/*
        case IOCONP_VALUE:
            x->setv(this);
            break;

        case IOCONP_DIGS:
            x->setl(digs());
            break;
*/

        default:
            return eObject::simpleproperty(propertynr, x);
    }
    return ESTATUS_SUCCESS;
}
