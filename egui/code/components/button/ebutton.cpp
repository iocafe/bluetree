/**

  @file    ebutton.cpp
  @brief   Push button or menu item.
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
  Constructor.
****************************************************************************************************
*/
eButton::eButton(
    eObject *parent,
    e_oid id,
    os_int flags)
    : eComponent(parent, id, flags)
{
    m_imgui_toggl = false;
    m_set_toggled = true;
}


/**
****************************************************************************************************
  Virtual destructor.
****************************************************************************************************
*/
eButton::~eButton()
{
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
eObject *eButton::clone(
    eObject *parent,
    e_oid id,
    os_int aflags)
{
    eButton *clonedobj;
    clonedobj = new eButton(parent, id == EOID_CHILD ? oid() : id, flags());
    clonegeneric(clonedobj, aflags);
    return clonedobj;
}


/**
****************************************************************************************************

  @brief Add eButton to class list and class'es properties to it's property set.

  The eButton::setupclass function adds eButton to class list and class'es properties to
  it's property set. The class list enables creating new objects dynamically by class identifier,
  which is used for serialization reader functions. The property set stores static list of
  class'es properties and metadata for those.

****************************************************************************************************
*/
void eButton::setupclass()
{
    const os_int cls = EGUICLASSID_BUTTON;

    os_lock();
    eclasslist_add(cls, (eNewObjFunc)newobj, "eButton");
    setupproperties(cls, ECOMP_NO_OPTIONAL_PROPERITES);
    addpropertys(cls, ECOMP_TEXT, ecomp_text, "text", EPRO_METADATA);
    addproperty (cls, ECOMP_VALUE, ecomp_value, "value");
    addpropertyl(cls, ECOMP_SETVALUE, ecomp_setvalue, OS_TRUE, "set value", EPRO_METADATA);
    addpropertys(cls, ECOMP_TARGET, ecomp_target, "target", EPRO_METADATA);

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
eStatus eButton::onpropertychange(
    os_int propertynr,
    eVariable *x,
    os_int flags)
{
    switch (propertynr)
    {
        case ECOMP_VALUE:
            m_set_toggled = true;
            break;

        case ECOMP_TEXT:
            m_text.clear();
            break;

        case ECOMP_SETVALUE:
            m_set_toggled = true;
            break;

        default:
            return eComponent::onpropertychange(propertynr, x, flags);
    }

    return ESTATUS_SUCCESS;
}


/**
****************************************************************************************************

  @brief Draw the component.

  The eButton::draw() function calls ImGui API to render the component.

  @param   Prm Drawing parameters.
  @return  The function return ESTATUS_SUCCESS if all is fine. Other values indicate that the
           component is no longer drawable or useful. This could be for example a pop up menu
           closed implicitely by clicking elsewhere.

****************************************************************************************************
*/
eStatus eButton::draw(
    eDrawParams& prm)
{
    eComponent *c;
    const os_char *label;
    ImVec2 sz;
    os_int cid;

    add_to_zorder(prm.window, prm.layer);

    if (m_set_toggled) {
        set_toggled();
        m_set_toggled = false;
    }

    ImVec2 cpos = ImGui::GetCursorScreenPos();
    m_rect.x1 = cpos.x;
    m_rect.y1 = cpos.y;

    sz.x = sz.y = 0;

    label = m_text.get(this, ECOMP_TEXT);

    /* If this component has children, it contains a sub menu.
     */
    c = firstcomponent();
    if (c) {
        if (ImGui::BeginMenu(label))
        {
            while (c) {
                c->draw(prm);
                c = c->nextcomponent();
            }

            ImGui::EndMenu();
        }
    }
    else {
        cid = parent()->classid();
        if (cid == EGUICLASSID_POPUP || cid == EGUICLASSID_BUTTON)
        {
            if (ImGui::MenuItem(label, "", &m_imgui_toggl)) {
                activate();
            }
        }
        else {
            if (m_imgui_toggl)
            {
                ImGui::PushID(label);
                ImGui::PushStyleColor(ImGuiCol_Button, ImGui::ColorConvertFloat4ToU32(ImVec4(0.3f, 0.7f, 0.5f, 1.0f)));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::ColorConvertFloat4ToU32(ImVec4(0.5f, 0.9f, 0.7f, 1.0f)));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::ColorConvertFloat4ToU32(ImVec4(0.3f, 0.7f, 0.5f, 1.0f)));
                ImGui::Button(label);
                if (ImGui::IsItemClicked(0))
                {
                    setpropertyl(ECOMP_VALUE, !propertyi(ECOMP_SETVALUE));
                }
                ImGui::PopStyleColor(3);
                ImGui::PopID();
            }
            else
            {
                ImGui::Button(label);
                if (ImGui::IsItemClicked(0)) {
                    eVariable tmp;
                    propertyv(ECOMP_SETVALUE, &tmp);
                    setpropertyv(ECOMP_VALUE, &tmp);
                    activate();
                }
            }
        }
    }

    sz = ImGui::GetItemRectSize();

    m_rect.x2 = m_rect.x1 + sz.x - 1;
    m_rect.y2 = m_rect.y1 + sz.y - 1;

    /* Let base class implementation handle the rest.
     */
    return eComponent::draw(prm);
}


/**
****************************************************************************************************

  @brief Component clicked.

  The eButton::on_click() function is called when a component is clicked. If the component
  processess the mouse click, it returns OS_TRUE. This indicates that the click has been
  processed. If it doesn't process the click, it call's eComponent base classess'es on_click()
  function to try if base class wants to process the click.
  When the mouse click is not processed, it is passed to parent object in z order.

  @param   prm Drawing parameters, notice especially edit_mode.
  @param   mouse_button_nr Which mouse button, for example EIMGUI_LEFT_MOUSE_BUTTON.

  @return  OS_TRUE if mouse click was processed by this component, or OS_FALSE if not.

****************************************************************************************************
*/
os_boolean eButton::on_click(
    eDrawParams& prm,
    os_int mouse_button_nr)
{
    if (!prm.edit_mode && mouse_button_nr == EIMGUI_LEFT_MOUSE_BUTTON) {
        return OS_TRUE;
    }
    return eComponent::on_click(prm, mouse_button_nr);
}


/**
****************************************************************************************************

  @brief Start editing value, toggle checkbox or show drop down list.

  The eButton::activate() function is called when a value is clicked, or key (for example
  spacebar) is hit to start editing the value. Actual operation depends on metadata, the
  function can either start value edit, toggle a checkbox or show drop down list.

  @return  None.

****************************************************************************************************
*/
void eButton::activate()
{
    eVariable target;

    propertyv(ECOMP_TARGET, &target);
    if (!target.isempty()){
        eVariable value;
        propertyv(ECOMP_SETVALUE, &value);
        setpropertyv_msg(target.gets(), &value);
    }
}


/**
****************************************************************************************************

  @brief Set value for ImGui toggle mark, when needed.

  The set_toggled() function is called when drawing to set value to determine value for
  m_imgui_toggl boolean. Pointer to this boolean is passed to the ImGui to inform wether
  to draw a check mark in menu button.
  @return  None.

****************************************************************************************************
*/
void eButton::set_toggled()
{
    eVariable tmp1, tmp2;

    propertyv(ECOMP_VALUE, &tmp1);
    propertyv(ECOMP_SETVALUE, &tmp2);

    m_imgui_toggl = tmp1.compare(&tmp2) ? false : true;
}
