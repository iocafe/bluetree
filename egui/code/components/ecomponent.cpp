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
    ecomp_ipath[] = "ipath",
    ecomp_setvalue[] = "setvalue",
    ecomp_target[] = "target",
    ecomp_edit[] = "edit",
    ecomp_refresh[] = "refresh",
    ecomp_all[] = "all";


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
    m_popup_open = OS_FALSE;
    m_next_z = m_prev_z = OS_NULL;
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
    remove_from_zorder();
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
    eVariable *vtype;

    addpropertys(cls, ECOMP_TEXT, ecomp_text, "text", EPRO_METADATA);

    if (flags & ECOMP_VALUE_PROPERITES) {
        vtype = addpropertyl (cls, ECOMP_TYPE, ecomp_type, "type", EPRO_METADATA);
        addproperty (cls, ECOMP_VALUE, ecomp_value, "value");
        addproperty (cls, ECOMP_DEFAULT, ecomp_default, "default", EPRO_METADATA);
        addpropertyl(cls, ECOMP_DIGS, ecomp_digs, "digs", EPRO_METADATA);
        addpropertys(cls, ECOMP_UNIT, ecomp_unit, "unit", EPRO_METADATA);
        addpropertyd(cls, ECOMP_MIN, ecomp_min, "min", EPRO_METADATA);
        addpropertyd(cls, ECOMP_MAX, ecomp_max, "max", EPRO_METADATA);

        {
            eVariable tmp;
            emake_type_enum_str(&tmp);
            vtype->setpropertyv(ECOMP_ATTR, &tmp);
        }
    }

    if (flags & ECOMP_EXTRA_UI_PROPERITES) {
        addproperty(cls, ECOMP_ABBR, ecomp_abbr, "abbreviation", EPRO_METADATA);
        addproperty(cls, ECOMP_TTIP, ecomp_ttip, "tooltip", EPRO_METADATA);
    }

    addpropertys(cls, ECOMP_ATTR, ecomp_attr, "attr", EPRO_METADATA);

    if (flags & ECOMP_VALUE_PROPERITES) {
        addpropertyd(cls, ECOMP_GAIN, ecomp_gain, "gain", EPRO_METADATA);
        addpropertyd(cls, ECOMP_OFFSET, ecomp_offset, "offset", EPRO_METADATA);
    }

    if (flags & ECOMP_CONF_PROPERITES) {
        addproperty (cls, ECOMP_CONF, ecomp_conf, "conf", EPRO_METADATA);
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


/* Get topmost component in Z orderr which encloses (x, y) position.
 * Returns OS_NULL if none found.
 */
eComponent *eComponent::findcomponent(
        ePos pos)
{
    eComponent *w, *c;

    w = window(EGUICLASSID_WINDOW);
    if (w == OS_NULL) return OS_NULL;

    c = w;
    do {
        c = c->m_prev_z;
        if (erect_is_point_inside(c->visible_rect(), pos)) {
            if (check_click(pos) != ECOMPO_CLICK_IGNORE) return c;
        }
    }
    while (c != w);

    return OS_NULL;
}

/* Add component to window's Z order
 */
void eComponent::add_to_zorder(eWindow *w)
{
    if (w == OS_NULL) return;

    if (classid() != EGUICLASSID_WINDOW)
    {
        m_prev_z = w->m_prev_z;
        m_next_z = w;
        m_prev_z->m_next_z = this;
        w->m_prev_z = this;
    }
}


/* Remove component from window's Z order
 */
void eComponent::remove_from_zorder()
{
    if (classid() != EGUICLASSID_WINDOW && m_next_z)
    {
        m_prev_z->m_next_z = m_next_z;
        m_next_z->m_prev_z = m_prev_z;
        m_next_z = m_prev_z = OS_NULL;
    }
}

/* Wipe out whole Z order
 */
void eComponent::clear_zorder()
{
    eComponent *c, *next_c;

    c = this;
    do {
        next_c = c->m_next_z;
        c->remove_from_zorder();
        if (c == next_c) break;
        c = next_c;
    }
    while(c);
}


/**
****************************************************************************************************

  @brief Get component's parent window.

  The eComponent::window() function returns pointer to the parent window (eWindow or
  ePopup). If this object is pointer window, the pointer to itself is returned

  @param   cid Set 0 to get pointer to either eWindow or ePopup. EGUICLASSID_WINDOW
           to get eWindow pointer only (if ePopup, the function returns os_NULL).
           EGUICLASSID_POPUP to get ePopup. Tree above first eWindow or ePopup is not
           checked, regardless of value of cid parameter.

  @return  Pointer to the parent window, or OS_NULL if there is this component is not enclosed
           within a parent window.

****************************************************************************************************
*/
eComponent *eComponent::window(
    os_int cid)
{
    eObject *obj;
    os_int id;

    obj = this;
    while (obj) {
        id = obj->classid();
        if (id == EGUICLASSID_WINDOW || id == EGUICLASSID_POPUP)
        {
            if (cid == 0 || cid == id) {
                return (eComponent*)obj;
            }
            return OS_NULL;
        }
        obj = obj->parent();
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
    eObject *o;
    bool popup_drawn;

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

    /* In edit mode, draw decorations.
     */
    if (prm.edit_mode) {
        draw_edit_mode_decorations();
    }

    return ESTATUS_SUCCESS;
}


/**
****************************************************************************************************

  @brief Edit mode: Draw a rectangle around component

****************************************************************************************************
*/
void eComponent::draw_edit_mode_decorations()
{
    ImDrawList* draw_list;
    ImVec2 top_left, bottom_right;
    ImU32 col;

    top_left.x = m_rect.x1;
    top_left.y = m_rect.y1;
    bottom_right.x = m_rect.x2;
    bottom_right.y = m_rect.y2;

    draw_list = ImGui::GetWindowDrawList();
    col = IM_COL32(255, 80, 80, 50);
    draw_list->AddRect(top_left, bottom_right, col);
}


ePopup *eComponent::popup()
{
    ePopup *p;

    close_popup();

    p = new ePopup(this, EOID_GUI_POPUP,
        EOBJ_TEMPORARY_ATTACHMENT );

    m_popup_open = true;

    return p;
}


/**
****************************************************************************************************

  @brief Generate right click popup menu.

  A derived component class may override this function to add it's own component specific
  items to right click popup menu. Overridden function should call the base class'es
  function to generate the popup menu and then add it's own items using returned ePopup
  pointer.

  @return  Pointer to the new right click popup window.

****************************************************************************************************
*/
ePopup *eComponent::right_click_popup()
{
    ePopup *p;
    eButton *scope, *item;
    eWindow *w;

    p = popup();

    /* Generic component scope items.
     */

    /* Window scope items (also for popups).
     */
    scope = new eButton(p);
    w = (eWindow*)window(EGUICLASSID_WINDOW);
    if (w) {
        scope->setpropertys(ECOMP_TEXT, "window");

        item = new eButton(scope);
        item->setpropertys(ECOMP_TEXT, "edit window");

        item->setpropertyl(ECOMP_VALUE, OS_FALSE);
        item->setpropertyl(ECOMP_SETVALUE, !w->editmode());
        item->setpropertys(ECOMP_TARGET, "window/_p/edit");
    }

    /* GUI scope items.
     */
    scope = new eButton(p);
    scope->setpropertys(ECOMP_TEXT, "gui");

    item = new eButton(scope);
    item->setpropertys(ECOMP_TEXT, "settings");
    item->setpropertys(ECOMP_VALUE, "guisettings");
    item->setpropertys(ECOMP_TARGET, "gui/_p/open");

    return p;
}


ePopup *eComponent::drop_down_list(
    eContainer *list)
{
    ePopup *p;
    eVariable *v, target;
    eButton *b;
    os_int propertynr, value;
    os_char buf[E_OIXSTR_BUF_SZ];

    p = popup();

    if (list == OS_NULL) {
        osal_debug_error("No drop down list data");
        return p;
    }

    value = propertyi(ECOMP_VALUE);

    oixstr(buf, sizeof(buf));
    target = buf;
    target += "/_p/x";

    for (v = list->firstv(); v; v = v->nextv())
    {
        propertynr = v->oid();

        b = new eButton(p);
        b->setpropertyv(ECOMP_TEXT, v);
        b->setpropertyi(ECOMP_VALUE, value);
        b->setpropertyi(ECOMP_SETVALUE, propertynr);
        b->setpropertyv(ECOMP_TARGET, &target);
    }

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


/* Drag desture detected, we are starting to drag this component.
 */
void eComponent::on_start_drag(
    eDrawParams& prm,
    ePos& mouse_down_pos)
{
    prm.gui->save_drag_origin(this, EGUI_DRAG_TO_COPY_COMPONENT);
}

/* Mouse dragging, we are copying/moving/mofifying component(s).
 */
void eComponent::on_drag(
    eDrawParams& prm,
    eGuiDragMode drag_mode,
    ePos& mouse_pos)
{

}

/* Mouse released to end drag, actually copy/move object or and modification.
 */
void eComponent::on_drop(
    eDrawParams& prm,
    eGuiDragMode drag_mode,
    ePos& mouse_up_pos)
{

}
