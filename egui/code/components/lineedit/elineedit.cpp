/**

  @file    elineedit.cpp
  @brief   Line edit with label and value.
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
eLineEdit::eLineEdit(
    eObject *parent,
    e_oid id,
    os_int flags)
    : eComponent(parent, id, flags)
{
    m_edit_value = false;
    m_prev_edit_value = false;
    m_set_checked = true;
    m_imgui_checked = false;
}


/**
****************************************************************************************************

  @brief Virtual destructor.

  X...

  @return  None.

****************************************************************************************************
*/
eLineEdit::~eLineEdit()
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
eObject *eLineEdit::clone(
    eObject *parent,
    e_oid id,
    os_int aflags)
{
    eLineEdit *clonedobj;
    clonedobj = new eLineEdit(parent, id == EOID_CHILD ? oid() : id, flags());
    clonegeneric(clonedobj, aflags);
    return clonedobj;
}


/**
****************************************************************************************************

  @brief Add eLineEdit to class list and class'es properties to it's property set.

  The eLineEdit::setupclass function adds eLineEdit to class list and class'es properties to
  it's property set. The class list enables creating new objects dynamically by class identifier,
  which is used for serialization reader functions. The property set stores static list of
  class'es properties and metadata for those.

****************************************************************************************************
*/
void eLineEdit::setupclass()
{
    const os_int cls = EGUICLASSID_LINE_EDIT;

    os_lock();
    eclasslist_add(cls, (eNewObjFunc)newobj, "eLineEdit");
    eComponent::setupproperties(cls, ECOMP_VALUE_PROPERITES|
        ECOMP_VALUE_STATE_PROPERITES|ECOMP_EXTRA_UI_PROPERITES);
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
eStatus eLineEdit::onpropertychange(
    os_int propertynr,
    eVariable *x,
    os_int flags)
{
    switch (propertynr)
    {
        case ECOMP_VALUE: /* clear label to display new text and proceed */
            m_label_value.clear();
            m_set_checked = true;
            break;

        case ECOMP_TEXT:
            m_text.clear();
            break;

        case ECOMP_UNIT:
            // m_unit.clear();
            m_attr.clear();
            break;

        case EVARP_DIGS:
        case EVARP_MIN:
        case EVARP_MAX:
        case EVARP_TYPE:
        case EVARP_ATTR:
            m_label_value.clear();
            m_attr.clear();
            m_set_checked = true;
            break;

        default:
            break;
    }

    return eComponent::onpropertychange(propertynr, x, flags);
}


/**
****************************************************************************************************

  @brief Draw the component.

  The eLineEdit::draw() function calls ImGui API to render the component.

  @param   Prm Drawing parameters.
  @return  The function return ESTATUS_SUCCESS if all is fine. Other values indicate that the
           component is no longer drawable or useful. This could be for example a pop up menu
           closed implicitely by clicking elsewhere.

****************************************************************************************************
*/
eStatus eLineEdit::draw(
    eDrawParams& prm)
{
    os_int edit_w, unit_w, total_w, unit_spacer, total_h, h;
    const os_char *value, *label, *unit;
    ImGuiInputTextFlags eflags;

    m_attr.for_variable(this);

    // ImVec2 c = ImGui::GetContentRegionAvail();
    // total_w = c.x;
    total_w = ImGui::GetContentRegionMax().x;


ImVec2 cpos = ImGui::GetCursorScreenPos();      // ImDrawList API uses screen coordinates!
    m_rect.x1 = cpos.x;
    m_rect.y1 = cpos.y;

    ImGui::TextUnformatted(m_text.get(this, ECOMP_TEXT));
    total_h = ImGui::GetItemRectSize().y;

    // int edit_w = ImGui::CalcItemWidth();
    // ImGui::SameLine(total_w - edit_w);
    // edit_w = total_w - 200;

    if (m_attr.showas() == E_SHOWAS_CHECKBOX) {
        edit_w = ImGui::GetFrameHeight();
    }
    else {
        edit_w = 200;
    }
    unit_w = 60;
    unit_spacer = 6;

    ImGui::SameLine(total_w - edit_w - unit_spacer - unit_w);
    ImGui::SetNextItemWidth(edit_w);

    if (m_edit_value) {
        label = m_label_edit.get(this);

        switch (m_attr.showas())
        {
            case E_SHOWAS_INTEGER_NUMBER:
            case E_SHOWAS_DECIMAL_NUMBER:
                eflags = ImGuiInputTextFlags_CharsDecimal|ImGuiInputTextFlags_EnterReturnsTrue;
                break;

            default:
                eflags = ImGuiInputTextFlags_EnterReturnsTrue;
                break;
        }

        ImGui::InputText(label, m_edit_buf.ptr(), m_edit_buf.sz(), eflags);
        if ((!ImGui::IsItemActive() || ImGui::IsItemDeactivatedAfterEdit()) && m_prev_edit_value)
        {
            eVariable value;
            propertyv(ECOMP_VALUE, &value);
            m_edit_value = false;
            if (os_strcmp(m_edit_buf.ptr(), value.gets())) {
                setpropertys(ECOMP_VALUE, m_edit_buf.ptr());
            }
        }
        else {
            if (!m_prev_edit_value) {
                ImGui::SetKeyboardFocusHere(-1);
                m_prev_edit_value = true;
            }
        }
    }
    else {
        value = m_label_value.get(this, ECOMP_VALUE, &m_attr);

        ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(1.0f, 0.5f));
        switch (m_attr.showas())
        {
            case E_SHOWAS_CHECKBOX:
                if (m_set_checked) {
                    set_checked();
                    m_set_checked = false;
                }

                label = m_label_edit.get(this);
                if (ImGui::Checkbox(label, &m_imgui_checked))
                {
                    activate();
                }
                break;

            default:
                ImGui::Button(value, ImVec2(edit_w, 0));
                if (ImGui::IsItemActive()) {
                    activate();
                }
                break;
        }
        ImGui::PopStyleVar();
        h = ImGui::GetItemRectSize().y;
        if (h > total_h) total_h = h;
    }

    unit = m_unit.get(this, ECOMP_UNIT);
    if (*unit != '\0') {
        ImGui::SameLine(total_w - unit_w);
        ImGui::SetNextItemWidth(unit_w);
        ImGui::TextUnformatted(unit);
        h = ImGui::GetItemRectSize().y;
        if (h > total_h) total_h = h;
    }

    m_rect.x2 = m_rect.x1 + total_w - 1;
    m_rect.y2 = m_rect.y1 + total_h - 1;

    /* Let base class implementation handle the rest.
     */
    return eComponent::draw(prm);
}


/**
****************************************************************************************************

  @brief Start editing value, toggle checkbox or show drop down list.

  The eLineEdit::activate() function is called when a value is clicked, or key (for example
  spacebar) is hit to start editing the value. Actual operation depends on metadata, the
  function can either start value edit, toggle a checkbox or show drop down list.

  @return  None.

****************************************************************************************************
*/
void eLineEdit::activate()
{
    switch (m_attr.showas())
    {
        case E_SHOWAS_CHECKBOX:
            setpropertyi(ECOMP_VALUE, m_imgui_checked);
            m_set_checked = true;
            break;

        case E_SHOWAS_DROP_DOWN_ENUM:
            drop_down_list(m_attr.get_list());
            break;

        default:
            m_prev_edit_value = false;
            m_edit_value = true;

            eVariable value;
            propertyv(ECOMP_VALUE, &value);
            enice_value_for_ui(&value, this, &m_attr);
            m_edit_buf.set(value.gets(), 256);
            break;
    }
}


/**
****************************************************************************************************

  @brief Set value for ImGui checkmark, when needed.

  The set_checked() function is called when drawing to set value to determine value for
  m_imgui_checked boolean. Pointer to this boolean is passed to the ImGui to inform wether
  to draw a check mark in to indicate true or false state of boolean.

  @return  None.

****************************************************************************************************
*/
void eLineEdit::set_checked()
{
    m_imgui_checked = propertyi(ECOMP_VALUE) ? true : false;
}
