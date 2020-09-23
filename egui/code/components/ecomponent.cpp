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
#include "egui.h"

/* Variable property names.
 */
const os_char
    ecomp_path[] = "path",
    ecomp_ipath[] = "ipath";


/**
****************************************************************************************************

  @brief Constructor.

  X...

  @return  None.

****************************************************************************************************
*/
eComponent::eComponent(
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
eComponent::~eComponent()
{
    /* Release any allocated memory.
     */
    // clear();
}


/**
****************************************************************************************************

  @brief Add eComponent to class list and class'es properties to it's property set.

  The eComponent::setupclass function adds eComponent to class list and class'es properties to
  it's property set. The class list enables creating new objects dynamically by class identifier,
  which is used for serialization reader functions. The property set stores static list of
  class'es properties and metadata for those.

****************************************************************************************************
*/
void eComponent::setupclass()
{
    const os_int cls = EGUICLASSID_COMPONENT;

    /* Add the class to class list.
     */
    os_lock();
    eclasslist_add(cls, (eNewObjFunc)newobj, "eComponent");
    setupproperties(cls, ECOMP_NO_OPTIONAL_PROPERITES);
    propertysetdone(cls);
    os_unlock();
}


/**
****************************************************************************************************

  @brief Add class'es properties to property set.

  The eComponent::setupproperties is helper function for setupclass, it is called from both
  eComponent class and derived classes like eName.

  Process mutex must be locked when calling this function.

****************************************************************************************************
*/
void eComponent::setupproperties(
    os_int cls,
    os_int flags)
{
    eVariable *p;

    /* Order of these addproperty() calls is important, since eComponent itself is used to
       describe the properties in property set. The property to set must be added to
       property set before setting value for it. There is trick with p to set text type
       after adding property type. This effects only eComponent class.
     */
    p = addproperty(cls, ECOMP_TEXT, ecomp_text, EPRO_METADATA, "text");

    if (flags & ECOMP_LINCONV_PROPERITES) {
        addpropertyl (cls, ECOMP_TYPE, ecomp_type, EPRO_METADATA, "type");
        p->setpropertyl(ECOMP_TYPE, OS_STR);
        addproperty (cls, ECOMP_VALUE, ecomp_value, EPRO_SIMPLE, "value");
        addproperty (cls, ECOMP_DEFAULT, ecomp_default, EPRO_METADATA, "default");
        addpropertyl(cls, ECOMP_DIGS, ecomp_digs, EPRO_METADATA|EPRO_SIMPLE, "digs");
        addpropertys(cls, ECOMP_UNIT, ecomp_unit, EPRO_METADATA, "unit");
        addpropertyd(cls, ECOMP_MIN, ecomp_min, EPRO_METADATA, "min");
        addpropertyd(cls, ECOMP_MAX, ecomp_max, EPRO_METADATA, "max");
    }

    if (flags & ECOMP_EXTRA_UI_PROPERITES) {
        addproperty(cls, ECOMP_ABBR, ecomp_abbr, EPRO_METADATA, "abbreviation");
        addproperty(cls, ECOMP_TTIP, ecomp_ttip, EPRO_METADATA, "tooltip");
    }

    addpropertyl(cls, ECOMP_ATTR, ecomp_attr, EPRO_METADATA, "attr");

    if (flags & ECOMP_LINCONV_PROPERITES) {
        addpropertyd(cls, ECOMP_GAIN, ecomp_gain, EPRO_METADATA, "gain");
        addpropertyd(cls, ECOMP_OFFSET, ecomp_offset, EPRO_METADATA, "offset");
    }

    if (flags & ECOMP_VALUE_STATE_PROPERITES) {
        addproperty (cls, ECOMP_SBITS, ecomp_sbits, EPRO_DEFAULT, "state bits");
        addproperty (cls, ECOMP_TSTAMP, ecomp_tstamp, EPRO_DEFAULT, "timestamp");
    }

    if (flags & ECOMP_CONF_PROPERITES) {
        addproperty (cls, ECOMP_CONF, ecomp_conf, EPRO_METADATA, "conf");
    }

    if (flags & ECOMP_CONF_PATH) {
        addproperty (cls, ECOMP_PATH, ecomp_path, EPRO_METADATA, "path");
    }
    if (flags & ECOMP_CONF_IPATH) {
        addproperty (cls, ECOMP_IPATH, ecomp_ipath, EPRO_METADATA, "ipath");
    }
}


/**
****************************************************************************************************

  @brief Get the first child component identified by oid.

  The eComponent::firstcomponent() function returns pointer to the next EGUI component.

  @param   id Object idenfifier. Default value EOID_CHILD specifies to count a child objects,
           which are not flagged as an attachment. Value EOID_ALL specifies to get count all
           child objects, regardless wether these are attachment or not. Other values
           specify object identifier, only children with that specified object identifier
           are searched for.

  @return  Pointer to the first child component, or OS_NULL if none found.

****************************************************************************************************
*/
eComponent *eComponent::firstcomponent(
    e_oid id)
{
    eObject *o;
    os_int cid;

    o = first(id);
    if (o == OS_NULL) {
        return OS_NULL;
    }

    cid = o->classid();
    if (EGUICLASSID_IS_COMPONENT(cid)) {
        return (eComponent*)o;
    }

    eHandle *h = o->handle()->next(id);
    while (h)
    {
        cid = h->object()->classid();
        if (EGUICLASSID_IS_COMPONENT(cid))
        {
            return (eComponent*)h->object();
        }

        h = h->next(id);
    }
    return OS_NULL;
}


/**
****************************************************************************************************

  @brief Get next component identified by oid.

  The eComponent::nextcomponent() function returns pointer to the next EGUI component.

  @param   id Object idenfifier. Default value EOID_CHILD specifies to count a child objects,
           which are not flagged as an attachment. Value EOID_ALL specifies to get count all
           child objects, regardless wether these are attachment or not. Other values
           specify object identifier, only children with that specified object identifier
           are searched for.

  @return  Pointer to the next component, or OS_NULL if none found.

****************************************************************************************************
*/
eComponent *eComponent::nextcomponent(
    e_oid id)
{
    os_int cid;

    if (mm_handle == OS_NULL) return OS_NULL;
    eHandle *h = mm_handle->next(id);
    while (h)
    {
        cid = h->object()->classid();
        if (cid >= EGUICLASSID_BEGIN_COMPONENTS &&
            cid <= EGUICLASSID_END_COMPONENTS)
        {
            return (eComponent*)h->object();
        }

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
  @return  If successfull, the function returns ESTATUS_SUCCESS (0). Nonzero return values do
           indicate that there was no property with given property number.

****************************************************************************************************
*/
eStatus eComponent::onpropertychange(
    os_int propertynr,
    eVariable *x,
    os_int flags)
{
    switch (propertynr)
    {
/*         case ECOMP_VALUE:
            setv(x);
            break;

        case ECOMP_DIGS:
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
eStatus eComponent::simpleproperty(
    os_int propertynr,
    eVariable *x)
{
    switch (propertynr)
    {
/*
        case ECOMP_VALUE:
            x->setv(this);
            break;

        case ECOMP_DIGS:
            x->setl(digs());
            break;
*/

        default:
            return eObject::simpleproperty(propertynr, x);
    }
    return ESTATUS_SUCCESS;
}


/**
****************************************************************************************************

  @brief Write variable to stream.

  The eComponent::writer() function serialized variable to stream. This writes only variable
  specific content, use eObject::write() to save also class information, attachements, etc.

  @param  stream The stream to write to.
  @param  flags Serialization flags.

  @return If successfull the function returns ESTATUS_SUCCESS (0). If writing object to stream
          fails, value ESTATUS_WRITING_OBJ_FAILED is returned. Assume that all nonzero values
          indicate an error.

****************************************************************************************************
*/
eStatus eComponent::writer(
    eStream *stream,
    os_int flags)
{
#if 0
    /* Version number. Increment if new serialized items are to the object,
       and check for new version's items in read() function.
     */
    const os_int version = 0;

    /* Begin the object and write version number.
     */
    if (stream->write_begin_block(version)) goto failed;

    /* Write type and number of decimal digits in flags.
     */
    if (*stream << (m_vflags & EVAR_SERIALIZATION_MASK)) goto failed;

    /* Write the value, if any.
     */
    switch (type())
    {
        case OS_LONG:
            if (*stream << m_value.valbuf.v.l) goto failed;
            break;

        case OS_DOUBLE:
            if (*stream << m_value.valbuf.v.d) goto failed;
            break;

        case OS_STR:
            if (m_vflags & EVAR_STRBUF_ALLOCATED)
            {
                if (*stream << m_value.strptr.used - 1) goto failed;
                if (stream->write(m_value.strptr.ptr, m_value.strptr.used - 1)) goto failed;
            }
            else
            {
                if (*stream << m_value.strbuf.used - 1) goto failed;
                if (stream->write(m_value.strbuf.buf, m_value.strbuf.used - 1)) goto failed;
            }
            break;

        case OS_OBJECT:
            if (m_value.valbuf.v.o->write(stream, flags)) goto failed;
            break;

        default:
            break;
    }

    /* End the object.
     */
    if (stream->write_end_block()) goto failed;

    /* Object succesfully written.
     */
    return ESTATUS_SUCCESS;

    /* Writing object failed.
     */
failed:
#endif
    return ESTATUS_WRITING_OBJ_FAILED;
}


/**
****************************************************************************************************

  @brief Read variable from stream.

  The eComponent::reader() function reads serialized variable from stream.
  This function reads only object content. To read whole object including attachments, names,
  etc, use eObject::read().

  @param  stream The stream to read from.
  @param  flags Serialization flags.

  @return If successfull the function returns ESTATUS_SUCCESS (0). If writing object to stream
          fails, value ESTATUS_READING_OBJ_FAILED is returned. Assume that all nonzero values
          indicate an error.

****************************************************************************************************
*/
eStatus eComponent::reader(
    eStream *stream,
    os_int flags)
{
#if 0
    /* Version number. Used to check which versions item's are in serialized data.
     */
    os_int version;

    os_short vflags;

    os_long sz;

    /* Release any allocated memory.
     */
    clear();

    /* Read object start mark and version number.
     */
    if (stream->read_begin_block(&version)) goto failed;

    /* Read type and number of decimal digits in flags.
     */
    if (*stream >> vflags) goto failed;

    /* Read the value, if any.
     */
    switch (vflags & EVAR_TYPE_MASK)
    {
        case OS_LONG:
            if (*stream >> m_value.valbuf.v.l) goto failed;
            break;

        case OS_DOUBLE:
            if (*stream >> m_value.valbuf.v.d) goto failed;
            break;

        case OS_STR:
            if (*stream >> sz) goto failed;

            /* If string fits into small buffer, copy it and save used size.
               Leave space for '\0' character (sz doesn not include '\0').
             */
            if (sz < EVARIABLE_STRBUF_SZ)
            {
                if (stream->read(m_value.strbuf.buf, sz)) goto failed;
                m_value.strbuf.buf[sz] = '\0';
                m_value.strbuf.used = (os_uchar)sz;
            }

            /* Otherwise we need to allocate buffer for long string. Allocate buffer, copy data in,
               save allocated size and used size. Set EVAR_STRBUF_ALLOCATED flag to indicate that
               buffer was allocated.
             */
            else
            {
                m_value.strptr.ptr = os_malloc(sz+1, &m_value.strptr.allocated);
                if (stream->read(m_value.strptr.ptr, sz)) goto failed;
                m_value.strptr.ptr[sz] = '\0';
                m_value.strptr.used = sz+1;
                m_vflags |= EVAR_STRBUF_ALLOCATED;
            }
            break;

        case OS_OBJECT:
            m_value.valbuf.v.o = read(stream, flags);
            if (m_value.valbuf.v.o == OS_NULL) goto failed;
            break;

        default:
            break;
    }

    /* Store data type and decimal digits.
     */
    m_vflags &= ~EVAR_SERIALIZATION_MASK;
    m_vflags |= (vflags & EVAR_SERIALIZATION_MASK);

    /* End the object.
     */
    if (stream->read_end_block()) goto failed;

    /* Object succesfully read.
     */
    return ESTATUS_SUCCESS;

    /* Reading object failed.
     */
failed:
#endif
    return ESTATUS_READING_OBJ_FAILED;
}


/**
****************************************************************************************************

  @brief Base class support for dwaring the component.

  The eComponent::draw() base class function should be called at end of a regular GUI component's
  draw function, but not from top level components like ePopup and eWindow.

  The function calls ImGui API to implement some generic component functionality,
  like right clicks, drag and drop in edit mode, etc.

  @param   Prm Drawing parameters.
  @return  The function return ESTATUS_SUCCESS if all is fine. Other values indicate that the
           component is no longer drawable or useful. This could be for example a pop up menu
           closed implicitely by clicking elsewhere.

****************************************************************************************************
*/
eStatus eComponent::draw(
    eDrawParams& prm)
{
    eRect visible_rect;
    eObject *o;
    bool popup_drawn;

    // Union component rect and parent clip to get visible rect !!!!!!!!!!!!!!!!!!!!!!
    visible_rect = m_rect;

    // And make sure item is in Z order

    if (prm.right_click) {
        if (erect_is_point_inside(&visible_rect, prm.io->MousePos.x, prm.io->MousePos.y))
        {
            popup();
        }
    }

    if (m_popup_open)
    {
        popup_drawn = false;
        for (o = first(EOID_GUI_POPUP); o; o = o->next(EOID_GUI_POPUP))
        {
            if (EGUICLASSID_IS_COMPONENT(o->classid())) {
                if (((eComponent*)o)->draw(prm) == ESTATUS_SUCCESS) {
                    popup_drawn = true;
                }
            }
        }

        if (!popup_drawn) {
            close_popup();
        }
    }

    return ESTATUS_SUCCESS;
}


ePopup *eComponent::popup()
{
    ePopup *p;

    close_popup();

    p = new ePopup(this, EOID_GUI_POPUP,
        EOBJ_TEMPORARY_ATTACHMENT );

    p->open_popup();
    m_popup_open = true;

    return p;
}


void eComponent::close_popup()
{
    eObject *o;

    if (m_popup_open)
    {
        while ((o = first(EOID_GUI_POPUP))) {
            delete o;
        }
        m_popup_open = false;
    }
}
