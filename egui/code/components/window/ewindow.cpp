/**

  @file    ewindow.cpp
  @brief   GUI window class.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    15.9.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "egui.h"


/**
****************************************************************************************************

  @brief Constructor.

  X...

  @return  None.

****************************************************************************************************
*/
eWindow::eWindow(
    eObject *parent,
    e_oid id,
    os_int flags)
    : eComponent(parent, id, flags)
{
    m_autolabel_count = 0;
    m_edit_mode = OS_FALSE;
    m_next_z = m_prev_z = this;

    addname("window", ENAME_TEMPORARY, "gui");
    ns_create("window");

    m_select_list = new eContainer(this, EOID_GUI_SELECTED, EOBJ_TEMPORARY_ATTACHMENT);
    addname("../_select", ENAME_TEMPORARY);
}


/**
****************************************************************************************************

  @brief Virtual destructor.

  X...

  @return  None.

****************************************************************************************************
*/
eWindow::~eWindow()
{
    clear_zorder();
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
eObject *eWindow::clone(
    eObject *parent,
    e_oid id,
    os_int aflags)
{
    eWindow *clonedobj;
    clonedobj = new eWindow(parent, id == EOID_CHILD ? oid() : id, flags());
    clonegeneric(clonedobj, aflags);
    return clonedobj;
}


/**
****************************************************************************************************

  @brief Add eWindow to class list and class'es properties to it's property set.

  The eWindow::setupclass function adds eWindow to class list and class'es properties to
  it's property set. The class list enables creating new objects dynamically by class identifier,
  which is used for serialization reader functions. The property set stores static list of
  class'es properties and metadata for those.

****************************************************************************************************
*/
void eWindow::setupclass()
{
    const os_int cls = EGUICLASSID_WINDOW;

    os_lock();
    eclasslist_add(cls, (eNewObjFunc)newobj, "eWindow");
    addpropertys(cls, ECOMP_VALUE, ecomp_value, "title");
    addpropertyb(cls, ECOMP_EDIT, ecomp_edit, "edit");
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
eStatus eWindow::onpropertychange(
    os_int propertynr,
    eVariable *x,
    os_int flags)
{
    switch (propertynr)
    {
        case ECOMP_VALUE:
            m_label_title.clear();
            break;

        case ECOMP_EDIT: /* toggle edit mode */
            set_editmode(x->geti());
            break;
    }

    return eComponent::onpropertychange(propertynr, x, flags);
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
eStatus eWindow::simpleproperty(
    os_int propertynr,
    eVariable *x)
{
    return eComponent::simpleproperty(propertynr, x);
}


/**
****************************************************************************************************

  @brief Generating ImGui autolabel.

  The eWindow::make_autolabel generates unique nonzero numbers for ImGui labels.

  @return Unique number (we wish).

****************************************************************************************************
*/
os_long eWindow::make_autolabel()
{
    while (++m_autolabel_count == 0);
    return m_autolabel_count;
}


/**
****************************************************************************************************

  @brief Draw the component.

  The eWindow::draw() function calls ImGui API to render the component.

  @param   Prm Drawing parameters.
  @return  The function return ESTATUS_SUCCESS if all is fine. Other values indicate that the
           component is no longer drawable or useful. This could be for example a pop up menu
           closed implicitely by clicking elsewhere.

****************************************************************************************************
*/
eStatus eWindow::draw(
    eDrawParams& prm)
{
    eDrawParams wprm;
    eComponent *c;
    const os_char *label;
    ImVec2 pos, sz;
    os_int mouse_button_nr;
    os_boolean lock_window;

    wprm = prm;
    wprm.edit_mode = m_edit_mode;
    wprm.window = this;

    /* Decide if we need to lock window to place for drag and drop.
     */
    lock_window = OS_FALSE;
    if (wprm.mouse_left_press && wprm.edit_mode) {
        lock_window = erect_is_point_inside(m_rect, prm.mouse_left_press_pos);
    }

    /* Create a window.
     */
    label = m_label_title.get(this, ECOMP_VALUE);
    if (lock_window) {
        ImGui::Begin(label, NULL, ImGuiWindowFlags_NoMove);
    }
    else
    {
        ImGui::Begin(label);
        /*  ImGui::Begin(label, NULL, ImGuiWindowFlags_NoSavedSettings |ImGuiWindowFlags_NoMove
          | ImGuiWindowFlags_NoTitleBar); */
    }

    wprm.mouse_over_window = ImGui::IsWindowHovered();
    if (!wprm.mouse_over_window) {
        wprm.mouse_click[EIMGUI_LEFT_MOUSE_BUTTON] = OS_FALSE;
        wprm.mouse_click[EIMGUI_RIGHT_MOUSE_BUTTON] = OS_FALSE;
        wprm.mouse_drag_event[EIMGUI_LEFT_MOUSE_BUTTON] = OS_FALSE;
        wprm.mouse_drag_event[EIMGUI_RIGHT_MOUSE_BUTTON] = OS_FALSE;
        wprm.mouse_drop_event[EIMGUI_LEFT_MOUSE_BUTTON] = OS_FALSE;
        wprm.mouse_drop_event[EIMGUI_RIGHT_MOUSE_BUTTON] = OS_FALSE;
    }

    /* Draw child components and setup Z order.
     */
    clear_zorder();
    for (c = firstcomponent(EOID_GUI_COMPONENT); c; c = c->nextcomponent(EOID_GUI_COMPONENT))
    {
        c->draw(wprm);
    }

    /* This gets rectangle including window framing.
     */
    pos = ImGui::GetWindowPos();
    sz = ImGui::GetWindowSize();
    m_rect.x1 = pos.x;
    m_rect.y1 = pos.y;
    m_rect.x2 = m_rect.x1 + sz.x - 1;
    m_rect.y2 = m_rect.y1 + sz.y - 1;

    /* This is what we need, window content rectangle excluding framing.
     */
    pos = ImGui::GetWindowPos();
    sz = ImGui::GetWindowContentRegionMin();
    m_rect.x1 = sz.x + pos.x;
    m_rect.y1 = sz.y + pos.y;
    sz = ImGui::GetWindowContentRegionMax();
    m_rect.x2 = sz.x + pos.x - 1;
    m_rect.y2 = sz.y + pos.y - 1;

    /* test visualization
     * ImVec2 vMin(m_rect.x1, m_rect.y1);
    ImVec2 vMax(m_rect.x2, m_rect.y2);
    ImGui::GetForegroundDrawList()->AddRect( vMin, vMax, IM_COL32( 255, 0, 0, 255 ) );
    */

    /* Good way to visually test.
    ImVec2 vMin = ImGui::GetWindowContentRegionMin();
    ImVec2 vMax = ImGui::GetWindowContentRegionMax();
    vMin.x += ImGui::GetWindowPos().x;
    vMin.y += ImGui::GetWindowPos().y;
    vMax.x += ImGui::GetWindowPos().x;
    vMax.y += ImGui::GetWindowPos().y;
    ImGui::GetForegroundDrawList()->AddRect( vMin, vMax, IM_COL32( 255, 255, 0, 255 ) );
     */

    eComponent::draw(wprm);

    if (wprm.edit_mode) {
        draw_edit_mode_decorations(wprm);
    }

    if (prm.mouse_click[EIMGUI_RIGHT_MOUSE_BUTTON]) {
        open_popup(wprm);
    }

    for (mouse_button_nr = 0;
         mouse_button_nr < EIMGUI_NRO_MOUSE_BUTTONS;
         mouse_button_nr++)
    {
        if (prm.mouse_click[mouse_button_nr]) {
            click(wprm, mouse_button_nr);
        }

        if (prm.mouse_drag_event[mouse_button_nr]) {
            start_drag(wprm, mouse_button_nr);
        }

        if (prm.mouse_dragging[mouse_button_nr]) {
            start_drag(wprm, mouse_button_nr);
        }

        if (prm.mouse_drop_event[mouse_button_nr]) {
            drop(wprm, mouse_button_nr);
        }
    }

    /* Finished with the window.
     */
    ImGui::End();
    return ESTATUS_SUCCESS;
}

/**
****************************************************************************************************

  @brief Draw edit mode decorations, like component frames, etc.

  The eWindow::edit_mode_decorations()....

  @param   prm Drawing parameters.
  @return  None

****************************************************************************************************
*/
void eWindow::draw_edit_mode_decorations(
    eDrawParams& prm)
{
    eComponent *c, *mouse_over;

    if (prm.mouse_over_window) {
        mouse_over = findcomponent(prm.mouse_pos);
    }
    else {
        mouse_over = OS_NULL;
    }

    for (c = m_next_z; c; c = c->m_next_z)
    {
        c->draw_edit_mode_decorations(prm, (os_boolean)(mouse_over == c));
        if (c == this) break;
    }
}

void eWindow::open_popup(
    eDrawParams& prm)
{
    eComponent *c;
    c = findcomponent(prm.mouse_pos);
    if (c) {
        c->right_click_popup();
    }
}

void eWindow::click(
    eDrawParams& prm,
    os_int mouse_button_nr)
{
    eComponent *c;
    c = findcomponent(prm.mouse_pos);
    if (c) {
        c->on_click(prm, mouse_button_nr);
    }
}

void eWindow::start_drag(
    eDrawParams& prm,
    os_int mouse_button_nr)
{
    eComponent *c;
    c = findcomponent(prm.mouse_drag_start_pos[mouse_button_nr]);
    if (c) {
        c->on_start_drag(prm, mouse_button_nr, prm.mouse_drag_start_pos[mouse_button_nr]);
    }
}

void eWindow::drag(
    eDrawParams& prm,
    os_int mouse_button_nr)
{
    eComponent *c;
    c = prm.gui->get_drag_origin();
    if (c) {
        c->on_drag(prm, mouse_button_nr, prm.gui->get_drag_mode(), prm.mouse_pos);
    }
}


void eWindow::drop(
    eDrawParams& prm,
    os_int mouse_button_nr)
{
    eComponent *origin, *c;
    eGuiDragMode drag_mode;

    origin = prm.gui->get_drag_origin();
    if (origin == OS_NULL) {
        return;
    }

    drag_mode = prm.gui->get_drag_mode();
    if (drag_mode == EGUI_DRAG_TO_MODIFY_COMPONENT)
    {
        origin->on_drop(prm, mouse_button_nr, OS_NULL, drag_mode, prm.mouse_pos);
    }
    else {
        c = findcomponent(prm.mouse_pos);
        if (c) {
            c->on_drop(prm, mouse_button_nr, origin, drag_mode, prm.mouse_pos);
        }
    }

    prm.gui->save_drag_origin(OS_NULL);
}

/* Modify selecction list and select flags of components.
 */
void eWindow::select(eComponent *c,
    eWindowSelect op)
{
    ePointer *p, *next_p;
    eComponent *cc;
    os_boolean c_in_selection, is_c;

    /* Clearing selection is same as new empty selection.
     */
    if (op == EWINDOW_CLEAR_SELECTION) {
        op = EWINDOW_NEW_SELECTION;
        c = OS_NULL;
    }

    /* If we need to remove current selection
     */
    c_in_selection = OS_FALSE;
    for (p = (ePointer*)m_select_list->first(); p; p = next_p) {
        next_p = (ePointer*)p->next();
        if (p->classid() != ECLASSID_POINTER) continue;
        cc = (eComponent*)p->get();
        if (cc) {
            is_c = (os_boolean)(cc == c);
            c_in_selection |= is_c;

            switch (op) {
                default:
                case EWINDOW_NEW_SELECTION:
                    cc->setpropertyl(ECOMP_SELECT, is_c);
                    if (!is_c) delete p;
                    break;

                case EWINDOW_APPEND_TO_SELECTION:
                    if (is_c) {
                        cc->setpropertyl(ECOMP_SELECT, OS_TRUE);
                    }
                    else {
                        if (c->isdecendentof(cc) ||
                            cc->isdecendentof(c))
                        {
                            return;
                        }
                    }
                    break;

                case EWINDOW_REMOVE_FROM_SELECTION:
                    if (is_c) {
                        cc->setpropertyl(ECOMP_SELECT, OS_FALSE);
                        delete p;
                    }
                    break;
            }
        }
        else {
            /* Componen has been deleted but pointer is still in
               select list, just forget it.
             */
            delete p;
        }
    }

    /* If component is not in select list, we may need to add it.
     */
    if (!c_in_selection && c) {
        if (op == EWINDOW_NEW_SELECTION || op == EWINDOW_APPEND_TO_SELECTION)
        {
            p = new ePointer(m_select_list);
            p->set(c);
            c->setpropertyl(ECOMP_SELECT, OS_TRUE);
        }
    }
}


/**
****************************************************************************************************

  @brief Add eTreeNode to class list and class'es properties to it's property set.

  The eObject::object_info function fills in item (eVariable) to contain information
  about this object in tree browser view.

  @param   item Pointer to eVariable to set up with object information.
  @param   name Object's name if known. OS_NULL if object is not named or name is
           unknown at this time.
  @param   appendix Pointer to eSet into which to store property flags. The stored property
           flags indicate if object has namespace, children, or properties.

****************************************************************************************************
*/
void eWindow::object_info(
    eVariable *item,
    eVariable *name,
    eSet *appendix)
{
    eVariable tmp;
    eObject::object_info(item, name, appendix);

    propertyv(ECOMP_VALUE, &tmp);
    item->setv(&tmp, OS_TRUE);
}
