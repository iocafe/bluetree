/**

  @file    ecomponent.cpp
  @brief   Abstract GUI component.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  Base call for all GUI components (widgets)...

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "eguilib.h"


/* GUI property names.
 */
os_char
    eguip_value[] = "x",
    eguip_digs[] = "x.digs",
    eguip_text[] = "x.text",
    eguip_unit[] = "x.unit",
    eguip_min[] = "x.min",
    eguip_max[] = "x.max",
    eguip_type[] = "x.type",
    eguip_attr[] = "x.attr",
    eguip_default[] = "x.default",
    eguip_gain[] = "x.gain",
    eguip_offset[] = "x.offset",
    eguip_quality[] = "x.quality",
    eguip_timestamp[] = "x.timestamp",
    eguip_conf[] = "conf"; /* This MUST not start with "x." */


/**
****************************************************************************************************

  @brief Constructor.

  X...

  @return  None.

****************************************************************************************************
*/
eGui::eGui(
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

  @brief Virtual destructor.

  X...

  @return  None.

****************************************************************************************************
*/
eGui::~eGui()
{
    /* Release any allocated memory.
     */
    // clear();
}


/**
****************************************************************************************************

  @brief Clone object

  The clone function clones the GUI and clonable attachments. Names will be left detached
  in clone if EOBJ_NO_MAP flag is given.

  @param  parent Parent for the clone.
  @param  id Object identifier for the clone.
  @param  aflags 0 for default operation. EOBJ_NO_MAP not to map names.
  @return Pointer to the clone.

****************************************************************************************************
*/
eObject *eGui::clone(
    eObject *parent,
    e_oid id,
    os_int aflags)
{
    eGui *clonedobj;
    clonedobj = new eGui(parent, id == EOID_CHILD ? oid() : id, flags());

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

  @brief Add eGui to class list and class'es properties to it's property set.

  The eGui::setupclass function adds eGui to class list and class'es properties to
  it's property set. The class list enables creating new objects dynamically by class identifier,
  which is used for serialization reader functions. The property set stores static list of
  class'es properties and metadata for those.

****************************************************************************************************
*/
void eGui::setupclass()
{
    const os_int cls = EGUICLASSID_GUI;

    /* Add the class to class list.
     */
    os_lock();
    eclasslist_add(cls, (eNewObjFunc)newobj, "eGui");
    setupproperties(cls);
    propertysetdone(cls);
    os_unlock();
}


/**
****************************************************************************************************

  @brief Add class'es properties to property set.

  The eGui::setupproperties is helper function for setupclass, it is called from both
  eGui class and derived classes like eName.

  Process mutex must be locked when calling this function.

****************************************************************************************************
*/
void eGui::setupproperties(
    os_int cls)
{
    eVariable *p;

    /* Order of these addproperty() calls is important, since eGui itself is used to
       describe the properties in property set. The property to set must be added to
       property set before setting value for it. There is trick with p to set text type
       after adding property type. This effects only eGui class.
     */
    p = addproperty(cls, EGUIP_TEXT, eguip_text, EPRO_METADATA|EPRO_NOONPRCH, "text");
    addpropertyl (cls, EGUIP_TYPE, eguip_type, EPRO_METADATA|EPRO_NOONPRCH, "type");
    p->setpropertyl(EGUIP_TYPE, OS_STR);

    addproperty (cls, EGUIP_VALUE, eguip_value, EPRO_PERSISTENT|EPRO_SIMPLE, "value");
    addproperty (cls, EGUIP_DEFAULT, eguip_default, EPRO_METADATA|EPRO_NOONPRCH, "default");
    addpropertyl(cls, EGUIP_DIGS, eguip_digs, EPRO_METADATA|EPRO_SIMPLE, "digs");
    addpropertys(cls, EGUIP_UNIT, eguip_unit, EPRO_METADATA|EPRO_NOONPRCH, "unit");
    addpropertyd(cls, EGUIP_MIN, eguip_min, EPRO_METADATA|EPRO_NOONPRCH, "min");
    addpropertyd(cls, EGUIP_MAX, eguip_max, EPRO_METADATA|EPRO_NOONPRCH, "max");
    addpropertyl(cls, EGUIP_ATTR, eguip_attr, EPRO_METADATA|EPRO_NOONPRCH, "attr");
    addpropertyd(cls, EGUIP_GAIN, eguip_gain, EPRO_METADATA|EPRO_NOONPRCH, "gain");
    addpropertyd(cls, EGUIP_OFFSET, eguip_offset, EPRO_METADATA|EPRO_NOONPRCH, "offset");
    addproperty (cls, EGUIP_QUALITY, eguip_quality, EPRO_METADATA|EPRO_NOONPRCH, "quality");
    addproperty (cls, EGUIP_TIMESTAMP, eguip_timestamp, EPRO_METADATA|EPRO_NOONPRCH, "timestamp");
    addproperty (cls, EGUIP_CONF, eguip_conf, EPRO_METADATA|EPRO_NOONPRCH, "conf");

}


/**
****************************************************************************************************

  @brief Get next child eGui object identified by oid.

  The eGui::nextv() function returns pointer to the next child object of this object.

  @param   id Object idenfifier. Default value EOID_CHILD specifies to count a child objects,
           which are not flagged as an attachment. Value EOID_ALL specifies to get count all
           child objects, regardless wether these are attachment or not. Other values
           specify object identifier, only children with that specified object identifier
           are searched for.

  @return  Pointer to the first child variable, or OS_NULL if none found.

****************************************************************************************************
*/
eGui *eGui::nextv(
    e_oid id)
{
    if (mm_handle == OS_NULL) return OS_NULL;
    eHandle *h = mm_handle->next(id);
    while (h)
    {
        if (h->object()->classid() == EGUICLASSID_GUI)
            return eGui::cast(h->object());

        h = h->next(id);
    }
    return OS_NULL;
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
  @return  None.

****************************************************************************************************
*/
void eGui::onpropertychange(
    os_int propertynr,
    eVariable *x,
    os_int flags)
{
    switch (propertynr)
    {
/*         case EGUIP_VALUE:
            setv(x);
            break;

        case EGUIP_DIGS:
            setdigs((os_int)x->getl());
            break; */

        default:
            /* eObject::onpropertychange(propertynr, x, flags); */
            break;
    }
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
eStatus eGui::simpleproperty(
    os_int propertynr,
    eVariable *x)
{
    switch (propertynr)
    {
/*
        case EGUIP_VALUE:
            x->setv(this);
            break;

        case EGUIP_DIGS:
            x->setl(digs());
            break;
*/

        default:
//            x->clear();
            /* return eObject::simpleproperty(propertynr, x); */
            return ESTATUS_NO_SIMPLE_PROPERTY_NR;
    }
    return ESTATUS_SUCCESS;
}
