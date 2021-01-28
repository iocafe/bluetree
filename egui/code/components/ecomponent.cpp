/**

  @file    ecomponent.cpp
  @brief   GUI component base class.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  Base class for all GUI components (widgets).

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
    ecomp_target[] = "target",
    ecomp_setvalue[] = "setvalue",
    ecomp_name[] = "name",
    ecomp_path[] = "path",
    ecomp_ipath[] = "ipath",
    ecomp_edit[] = "edit",
    ecomp_all[] = "all",
    ecomp_drop_down_list_select[] = "dropselect",
    ecomp_select[] = "_select",
    ecomp_command[] = "_command";


/**
****************************************************************************************************
  Constructor.
****************************************************************************************************
*/
eComponent::eComponent(
    eObject *parent,
    e_oid id,
    os_int flags)
    : eObject(parent, id, flags)
{
    m_select = OS_FALSE;
    m_popup_open = OS_FALSE;
    m_next_z = m_prev_z = OS_NULL;
    m_zlayer = 0;
}


/**
****************************************************************************************************
  Virtual destructor.
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
    eVariable *p;

    if (flags & ECOMP_VALUE_PROPERITES) {
        eVariable tmp;
        addproperty (cls, ECOMP_VALUE, ecomp_value, "value", EPRO_NOPACK);
        p = addpropertyl (cls, ECOMP_TYPE, ecomp_type, "type", EPRO_METADATA);
        emake_type_enum_str(&tmp);
        p->setpropertyv(ECOMP_ATTR, &tmp);

        addproperty (cls, ECOMP_TEXT, ecomp_text, "text", EPRO_METADATA);
        addproperty (cls, ECOMP_DEFAULT, ecomp_default, "default", EPRO_METADATA);
        addpropertyl(cls, ECOMP_DIGS, ecomp_digs, "digs", EPRO_METADATA);
        addpropertys(cls, ECOMP_UNIT, ecomp_unit, "unit", EPRO_METADATA);
        addpropertys(cls, ECOMP_ATTR, ecomp_attr, "attr", EPRO_METADATA);
    }

    if (flags & ECOMP_EXTRA_UI_PROPERITES) {
        addpropertys(cls, ECOMP_GROUP, ecomp_group, "group", EPRO_METADATA);
        addpropertyd(cls, ECOMP_MIN, ecomp_min, "min", EPRO_METADATA);
        addpropertyd(cls, ECOMP_MAX, ecomp_max, "max", EPRO_METADATA);
        addproperty (cls, ECOMP_ABBR, ecomp_abbr, "abbreviation", EPRO_METADATA);
        addproperty (cls, ECOMP_TTIP, ecomp_ttip, "tooltip", EPRO_METADATA);
        addpropertyd(cls, ECOMP_GAIN, ecomp_gain, "gain", EPRO_METADATA);
        addpropertyd(cls, ECOMP_OFFSET, ecomp_offset, "offset", EPRO_METADATA);
    }

    addpropertyb(cls, ECOMP_SELECT, ecomp_select, "select", EPRO_SIMPLE);
    addpropertyl(cls, ECOMP_COMMAND, ecomp_command, "command");
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

  @brief Get the topmost component in Z order which enclose (x, y) position.

  The eComponent::findcomponent function looks up the eWindow containing this component (this
  component can be the eWindow itsel) and searched for component.

  @param   pos ePos structure specifying point (x, y).
  @param   prm Structure holding rendering parameters.
  @param   drag_origin If this a check for drop, pointer to original dragged component.
           This can be used for type checking.

  @return  Pointer to the topmost component in Z order which encloses point (x, y).
           Returns OS_NULL if none of the components covers point (x,y) found.

****************************************************************************************************
*/
eComponent *eComponent::findcomponent(
    ePos pos,
    eDrawParams *prm,
    eComponent *drag_origin)
{
    eComponent *w, *c;

    w = window(EGUICLASSID_WINDOW);
    if (w == OS_NULL) return OS_NULL;

    c = w;
    do {
        c = c->m_prev_z;
        if (c->check_pos(pos, prm, drag_origin) != ECOMPO_IGNORE_MOUSE) {
            return c;
        }
    }
    while (c != w);

    return OS_NULL;
}


/**
****************************************************************************************************

  @brief Add this component to window's Z order

  Component will be added as new topmost component in Z order. Notice that Z order sorts
  components of window by "depth", from bottom to top. Z order is two directional linked list.

  @param   w Pointer to eWindow.
  @param   layer Layer. 0 for components in base layer. 1 for popup, 2 for poup up from popup.

****************************************************************************************************
*/
void eComponent::add_to_zorder(
    eWindow *w,
    os_int layer)
{
    eComponent *z;

    if (w == OS_NULL) return;

    if (classid() != EGUICLASSID_WINDOW)
    {
        z = w;
        while (z->m_prev_z->classid() != EGUICLASSID_WINDOW)
        {
            if (z->m_prev_z->m_zlayer <= layer) break;
            z = z->m_prev_z;
        }


        m_prev_z = z->m_prev_z;
        m_next_z = z;
        m_prev_z->m_next_z = this;
        z->m_prev_z = this;
        m_zlayer = layer;
    }
}


/**
****************************************************************************************************

  @brief Remove component from window's Z order

  The function detaches the component from two directional linked list Z order. This function
  needs to be called by component destructor, not to leave pointers hanging.

****************************************************************************************************
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


/**
****************************************************************************************************

  @brief Wipe out whole Z order

  The function detaches the all component in Z order from Z order.

****************************************************************************************************
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
    os_int command;

    switch (propertynr)
    {
        case ECOMP_SELECT:
            m_select = (os_boolean)x->getl();
            break;

        case ECOMP_COMMAND:
            command = x->geti();
            if (command == ECOMPO_NO_COMMAND) break;
            switch (command) {
                case ECOMPO_CUT:
                    on_delete();
                    break;

                default:
                    break;
            }
            setpropertyi(ECOMP_COMMAND, ECOMPO_NO_COMMAND);
            break;

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
        case ECOMP_SELECT:
            x->setl(m_select);
            break;

        default:
            return eObject::simpleproperty(propertynr, x);
    }
    return ESTATUS_SUCCESS;
}


/**
****************************************************************************************************

  @brief Write component to stream (place holder).

  The writer() function serializes component to stream. This eComponent::writer() is
  a place holder and must be overloaded to enable serialization for a class.

  This overloaded function writes only component specific content, use write() to save also
  class information, attachements, etc.

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
    osal_debug_error("eComponent::writer is not overloaded for serialization");
    return ESTATUS_WRITING_OBJ_FAILED;
}


/**
****************************************************************************************************

  @brief Read component from stream (place holder).

  The reader() function reads serialized component from stream. This eComponent::reader() is
  a place holder and must be overloaded to enable serialization for a class.

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
    osal_debug_error("eComponent::reader is not overloaded for serialization");
    return ESTATUS_READING_OBJ_FAILED;
}


/**
****************************************************************************************************

  @brief Base class support for drawing the component.

  The eComponent::draw() base class function should be called at end of a regular GUI component's
  draw function, but not from top level components like ePopup and eWindow.

  The function calls ImGui API to implement some generic component functionality,
  mostly popups at this time.

  @param   prm Drawing parameters.
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

    return ESTATUS_SUCCESS;
}


/**
****************************************************************************************************

  @brief Edit mode: Draw a rectangle around component

****************************************************************************************************
*/
void eComponent::draw_edit_mode_decorations(
    eDrawParams& prm,
    os_boolean mouse_over)
{
    ImDrawList* draw_list;
    ImVec2 top_left, bottom_right;
    ImU32 col;
    float thickness;
    float rounding = 4.0f;

    top_left.x = (os_float)m_rect.x1;
    top_left.y = (os_float)m_rect.y1;
    bottom_right.x = (os_float)m_rect.x2;
    bottom_right.y = (os_float)m_rect.y2;

    if (m_select || mouse_over) {
        thickness = 2.0f;
        rounding = 0.0f;
    }
    else {
        thickness = 1.0f;
        rounding = 0.0f;
    }

    if (mouse_over) {
        if (m_select) {
            col = IM_COL32(48, 48, 255, 250);
        }
        else {
            col = IM_COL32(128, 128, 128, 150);
            if (classid() == EGUICLASSID_WINDOW) {
                thickness = 1.0f;
                rounding = 0.0f;
                col = IM_COL32(128, 128, 128, 60);
            }
        }
    }
    else {
        col = m_select ? IM_COL32(48, 48, 255, 80) : IM_COL32(128, 128, 128, 60);
    }

    draw_list = ImGui::GetWindowDrawList();
    draw_list->AddRect(top_left, bottom_right, col,rounding,
        rounding > 0.0f ? ImDrawCornerFlags_All : ImDrawCornerFlags_None, thickness);
}


/**
****************************************************************************************************

  @brief Create popup window, often "select value" or "right click" popup menu.

  This function crate general purpose ePopup window for this GUI component. The popup window
  is used to show drop down lists, right click menu, etc.

  @return  Pointer to the new popup window.

****************************************************************************************************
*/
ePopup *eComponent::popup()
{
    ePopup *p;
    close_popup();

    p = new ePopup(this, EOID_GUI_POPUP, EOBJ_TEMPORARY_ATTACHMENT);
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
ePopup *eComponent::right_click_popup(
    eDrawParams& prm)
{
    ePopup *p;
    eButton *scope, *item;
    eWindow *w;

    p = popup();

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

    /* Edit mode component scope items.
     */
    if (prm.edit_mode) {
        add_popup_edit_mode_items(prm, p);
    }

    return p;
}


/**
****************************************************************************************************

  @brief Add edit mode items to right click popup.

  The add_popup_edit_mode_items functions adds edit mode items, like "cut", "copy", "paste"
  to right click popup.

  @param   prm Structure holding rendering parameters.
  @param   p Pointer to the right click popup window.
  @return  None.

****************************************************************************************************
*/
void eComponent::add_popup_edit_mode_items(
    eDrawParams& prm,
    ePopup *p)
{
    /* Generic component scope items
     */
    if (classid() != EGUICLASSID_WINDOW) {
        add_popup_item_command("cut", ECOMPO_CUT, p);
    }
}


/**
****************************************************************************************************
  Add command item to right click popup menu
****************************************************************************************************
*/
void eComponent::add_popup_item_command(
    const os_char *text,
    os_int command,
    ePopup *p)
{
    eButton *item;
    os_char buf[E_OIXSTR_BUF_SZ + 32];

    oixstr(buf, sizeof(buf));
    os_strncat(buf, "/_p/_command", sizeof(buf));

    item = new eButton(p);
    item->setpropertys(ECOMP_TEXT, text);
    item->setpropertyl(ECOMP_VALUE, ECOMPO_NO_COMMAND);
    item->setpropertyl(ECOMP_SETVALUE, command);
    item->setpropertys(ECOMP_TARGET, buf);
}


/**
****************************************************************************************************
  Add toggle item to right click popup menu
****************************************************************************************************
*/
void eComponent::add_popup_item_toggle(
    const os_char *text,
    os_int propertynr,
    const os_char *propertyname,
    ePopup *p)
{
    eButton *item;
    os_char buf[E_OIXSTR_BUF_SZ + 32];
    os_int state;

    oixstr(buf, sizeof(buf));
    os_strncat(buf, "/_p/", sizeof(buf));
    os_strncat(buf, propertyname, sizeof(buf));
    state = propertyi(propertynr);

    item = new eButton(p);
    item->setpropertys(ECOMP_TEXT, text);
    item->setpropertyl(ECOMP_VALUE, OS_FALSE);
    item->setpropertyl(ECOMP_SETVALUE, !state);
    item->setpropertys(ECOMP_TARGET, buf);
}


/**
****************************************************************************************************

  @brief Send message to object to request open content

  @param  path Path to the object to "open".
  @param  command EBROWSE_OPEN, EBROWSE_GRAPH, EBROWSE_PROPERTIES

****************************************************************************************************
*/
void eComponent::open_request(
    const os_char *path,
    os_int command)
{
    eContainer *context, *content;
    eVariable *v;

    content = new eContainer(ETEMPORARY);
    v = new eVariable(content, EOID_PARAMETER);
    v->setl(command);
    context = new eContainer(ETEMPORARY);
    v = new eVariable(context, EBROWSE_RIGHT_CLICK_SELECTIONS);
    v->setl(command);
    gui()->open_request(path, content, context);
    delete context;
    delete content;
}


/**
****************************************************************************************************

  @brief Create "select value from drop down list" popup.

  The add_popup_edit_mode_items functions adds edit mode items, like "cut", "copy", "paste"
  to right click popup.

  @param   list List holding values to select from.
  @param   propertyname Name of the property to set (if selected), for example "x".
  @param   value Current value of item. OS_NULL to get ECOMP_VALUE property.
  @return  Pointer to the new ePopup.

****************************************************************************************************
*/
ePopup *eComponent::drop_down_list(
    eContainer *list,
    const os_char *propertyname,
    eVariable *value)
{
    ePopup *p;
    eVariable *v, target, *tmpvalue = OS_NULL;
    eButton *b;
    os_int propertynr;
    os_char buf[E_OIXSTR_BUF_SZ];

    p = popup();

    if (list == OS_NULL) {
        osal_debug_error("No drop down list data");
        return p;
    }

    if (value == OS_NULL) {
        tmpvalue = new eVariable(list, EOID_ITEM, EOBJ_IS_ATTACHMENT);
        propertyv(ECOMP_VALUE, tmpvalue);
        value = tmpvalue;
    }

    oixstr(buf, sizeof(buf));
    target = buf;
    if (propertyname) {
        target += "/_p/";
        target += propertyname;
    }
    else {
        target += "/_p/x";
    }

    for (v = list->firstv(); v; v = v->nextv())
    {
        propertynr = v->oid();

        b = new eButton(p);
        b->setpropertyv(ECOMP_TEXT, v);
        if (propertynr == EOID_CHILD) { /* drop down list with strings */
            b->setpropertys(ECOMP_VALUE, value->gets());
            b->setpropertys(ECOMP_SETVALUE, v->gets());
        }
        else { /* drop down enum */
            b->setpropertyi(ECOMP_VALUE, value->geti());
            b->setpropertyi(ECOMP_SETVALUE, propertynr);
        }
        b->setpropertyv(ECOMP_TARGET, &target);
    }

    delete tmpvalue;
    return p;
}


/**
****************************************************************************************************

  @brief If this GUI component has popup open, close it.

  The eComponent::close_popup() function...

****************************************************************************************************
*/
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


/**
****************************************************************************************************

  @brief Delete has been selected from pop up menu.

  The eComponent::on_delete() function moves this component to gui's "delete later" list.

****************************************************************************************************
*/
void eComponent::on_delete()
{
    eGui *g;
    eWindow *w;
    ePointer *p, *next_p;
    eComponent *c;
    eContainer *select_list;

    w = (eWindow*)window(EGUICLASSID_WINDOW);
    g = gui();
    if (w == OS_NULL || g == OS_NULL) return;

    if (w->editmode())
    {
        if (!m_select) {
            w->select(this, EWINDOW_NEW_SELECTION);
        }

        select_list = w->get_select_list();
        for (p = (ePointer*)select_list->first(); p; p = next_p) {
            next_p = (ePointer*)p->next();
            c = (eComponent*)p->get();
            if (c) {
                g->delete_later(c);
            }
        }
    }
}


/**
****************************************************************************************************

  @brief Check if mouse position applies to this component.

  The eComponent::check_pos() function checks is mouse click, drag or drop applies to this
  component. The base class implementation simply checks is position is within the
  component's visible rectangle.

  @param   pos Mouse position (x,y) to check.
  @param   prm Structure holding rendering parameters.
  @param   drag_origin If this a check for drop, pointer to original dragged component.
           This can be used for type checking.

  @return  If position "is within" component, the function returns ECOMPO_POS_OK (nonzero).
           If not, ECOMPO_IGNORE_MOUSE (0).

****************************************************************************************************
*/
ecompoPosCheckRval eComponent::check_pos(
    ePos& pos,
    eDrawParams *prm,
    eComponent *drag_origin)
{
    OSAL_UNUSED(prm);
    OSAL_UNUSED(drag_origin);

    return erect_is_point_inside(visible_rect(), pos)
        ? ECOMPO_POS_OK : ECOMPO_IGNORE_MOUSE;
}


/**
****************************************************************************************************

  @brief GUI component's clicked, eComponent base class implementation.

  The eComponent::on_click() is called at mouse click to this component. The base class
  implementation takes care about edit mode selection and "right click" popup menu.

  @param   prm Structure holding rendering parameters.
  @param   mouse_button_nr Either EIMGUI_LEFT_MOUSE_BUTTON or EIMGUI_RIGHT_MOUSE_BUTTON.
  @return  OS_TRUE if click was consumed (processed) by the component, or OS_FALSE if not.
           If click is not processed, the click will be forwareded to GUI component below
           this one.

****************************************************************************************************
*/
os_boolean eComponent::on_click(
    eDrawParams& prm,
    os_int mouse_button_nr)
{
    eWindowSelect op;

    if (prm.edit_mode)
    {
        if (mouse_button_nr == EIMGUI_LEFT_MOUSE_BUTTON)
        {
            if (prm.mouse_click_keyboard_flags[mouse_button_nr] & EDRAW_LEFT_CTRL_DOWN)
            {
                op = m_select
                   ? EWINDOW_REMOVE_FROM_SELECTION
                   : EWINDOW_APPEND_TO_SELECTION;
            }
            else {
                op = EWINDOW_NEW_SELECTION;
            }

            prm.window->select(this, op);
            return OS_TRUE;
        }
    }

    if (prm.mouse_click[EIMGUI_RIGHT_MOUSE_BUTTON]) {
        right_click_popup(prm);
        return OS_TRUE;
    }

    return OS_FALSE;
}


/**
****************************************************************************************************

  @brief Drag desture detected, start to drag this component.

  The eComponent::on_start_drag() initiates dragging this component. It saves drag mode
  and ePointer to this component as drag origin.

  @param   prm Structure holding rendering parameters.
  @param   mouse_button_nr Either EIMGUI_LEFT_MOUSE_BUTTON or EIMGUI_RIGHT_MOUSE_BUTTON.
  @return  None.

****************************************************************************************************
*/
void eComponent::on_start_drag(
    eDrawParams& prm,
    os_int mouse_button_nr)
{
    eGuiDragMode drag_mode;

    if (prm.edit_mode)
    {
        if (!m_select) {
            prm.window->select(this, EWINDOW_NEW_SELECTION);
        }

        drag_mode = (prm.mouse_drag_keyboard_flags[mouse_button_nr] & EDRAW_LEFT_CTRL_DOWN)
            ? EGUI_DRAG_TO_COPY_COMPONENT : EGUI_DRAG_TO_MOVE_OR_COPY_COMPONENT;

        prm.gui->save_drag_origin(this, drag_mode);
    }
}


/**
****************************************************************************************************

  @brief Mouse dragging, we are copying/moving/modifying component(s).

  Not implemented, intended for visualization.

  @param   prm Structure holding rendering parameters.
  @param   mouse_button_nr Either EIMGUI_LEFT_MOUSE_BUTTON or EIMGUI_RIGHT_MOUSE_BUTTON.
  @param   drag_mode One of: EGUI_DRAG_TO_COPY_COMPONENT, EGUI_DRAG_TO_MOVE_OR_COPY_COMPONENT,
           or EGUI_DRAG_TO_MODIFY_COMPONENT

  @return  None.

****************************************************************************************************
*/
void eComponent::on_drag(
    eDrawParams& prm,
    os_int mouse_button_nr,
    eGuiDragMode drag_mode)
{
    OSAL_UNUSED(prm);
    OSAL_UNUSED(mouse_button_nr);
    OSAL_UNUSED(drag_mode);
}


/**
****************************************************************************************************

  @brief Mouse released to end drag, actually copy/move object or and modification.

  The eComponent::on_drop() is called to drop a dragged component when mouse drop gesture
  is detected. eComponent base class'es on_drop() handles mostly the edit mode, normal
  operation is class specfific.

  @param   prm Structure holding rendering parameters.
  @param   mouse_button_nr Either EIMGUI_LEFT_MOUSE_BUTTON or EIMGUI_RIGHT_MOUSE_BUTTON.
  @param   origin Pointer to dragged eComponent (origin), even the select list actually dropped.
  @param   drag_mode One of: EGUI_DRAG_TO_COPY_COMPONENT, EGUI_DRAG_TO_MOVE_OR_COPY_COMPONENT,
           or EGUI_DRAG_TO_MODIFY_COMPONENT

  @return  None.

****************************************************************************************************
*/
void eComponent::on_drop(
    eDrawParams& prm,
    os_int mouse_button_nr,
    eComponent *origin,
    eGuiDragMode drag_mode)
{
    eWindow *source_w, *destination_w;
    ePointer *p, *next_p;
    eComponent *c, *before_c, *cloned;
    eContainer *select_list;
    OSAL_UNUSED(mouse_button_nr);

    if ((drag_mode == EGUI_DRAG_TO_COPY_COMPONENT ||
         drag_mode == EGUI_DRAG_TO_MOVE_OR_COPY_COMPONENT) &&
         prm.edit_mode)
    {
        source_w = (eWindow*)origin->window(EGUICLASSID_WINDOW);
        destination_w = (eWindow*)window(EGUICLASSID_WINDOW);

        if (source_w != destination_w) {
            drag_mode = EGUI_DRAG_TO_COPY_COMPONENT;
        }

        /* Here we assume components on top of each others.
         */
        for (before_c = firstcomponent(); before_c; before_c = before_c->nextcomponent()) {
            if (prm.mouse_pos.y < (before_c->rect().y1 + before_c->rect().y2)/2) {
                break;
            }
        }

        select_list = source_w->get_select_list();
        for (p = (ePointer*)select_list->first(); p; p = next_p) {
            next_p = (ePointer*)p->next();
            c = (eComponent*)p->get();
            if (c == OS_NULL) {
                continue;
            }
            if (isdecendentof(c)) {
                continue;
            }

            /* Always clone, to get rid of temporary state.
             */
            cloned = eComponent::cast(c->clone(this));
            if (before_c) {
                cloned->adoptat(before_c);
                before_c = cloned->nextcomponent();
            }

            /* Delete original if moving.
             */
            if (drag_mode == EGUI_DRAG_TO_MOVE_OR_COPY_COMPONENT) {
                prm.gui->delete_later(c);
            }
        }
    }
}

