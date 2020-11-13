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
    os_memclear(&m_value_rect, sizeof(eRect));
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
    setupproperties(cls, ECOMP_VALUE_PROPERITES|ECOMP_EXTRA_UI_PROPERITES);
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
            m_value.clear();
            break;

        case ECOMP_TEXT:
            m_text.clear();
            break;

        case ECOMP_UNIT:
            m_unit.clear();
            m_attr.clear();
            break;

        case EVARP_DIGS:
        case EVARP_MIN:
        case EVARP_MAX:
        case EVARP_TYPE:
        case EVARP_ATTR:
            m_value.clear();
            m_attr.clear();
            break;

        default:
            return eComponent::onpropertychange(propertynr, x, flags);
    }

    return ESTATUS_SUCCESS;
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
    os_int edit_w, unit_w, relative_x2, unit_spacer, total_w, total_h, h;
    const os_char *unit;

    add_to_zorder(prm.window, prm.layer);
    m_attr.for_variable(this);

    relative_x2 = ImGui::GetContentRegionMax().x;
    total_w = relative_x2 - ImGui::GetCursorPosX();

    ImVec2 cpos = ImGui::GetCursorScreenPos();
    m_rect.x1 = cpos.x;
    m_rect.y1 = cpos.y;

    ImGui::TextUnformatted(m_text.get(this, ECOMP_TEXT, &m_attr, ESTRBUF_SINGLELINE));
    total_h = ImGui::GetItemRectSize().y;

    if (m_attr.showas() == E_SHOWAS_CHECKBOX) {
        edit_w = ImGui::GetFrameHeight();
    }
    else {
        edit_w = 200;
    }
    unit_w = 60;
    unit_spacer = 6;

    ImGui::SameLine(relative_x2 - edit_w - unit_spacer - unit_w);
    ImGui::SetNextItemWidth(edit_w);

    draw_value(prm, edit_w, &total_h);

    unit = m_unit.get(this, ECOMP_UNIT, &m_attr, ESTRBUF_SINGLELINE);
    if (*unit != '\0') {
        ImGui::SameLine(relative_x2 - unit_w);
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


void eLineEdit::draw_in_parameter_list(
    eDrawParams& prm)
{
    os_int total_h = 0;
    const os_char *unit;

    add_to_zorder(prm.window, prm.layer);
    m_attr.for_variable(this);

    if (ImGui::TableSetColumnIndex(0)) {
        ImGui::TextUnformatted(m_text.get(this, ECOMP_TEXT, &m_attr, ESTRBUF_SINGLELINE));
    }

    if (ImGui::TableSetColumnIndex(1)) {
        draw_value(prm, -1, &total_h);
    }

    if (ImGui::TableSetColumnIndex(2)) {
        unit = m_unit.get(this, ECOMP_UNIT, &m_attr, ESTRBUF_SINGLELINE);
        if (*unit != '\0') {
            ImGui::TextUnformatted(unit);
        }
    }

    /* Let base class implementation handle the rest.
     */
    eComponent::draw(prm);
}

/* value_w Set -1 if drawing in table.
 */
void eLineEdit::draw_value(
    eDrawParams& prm,
    os_int value_w,
    os_int *total_h)
{
    const os_char *label;
    const ImVec2 zero_pad(0, 0);
    ImGuiInputTextFlags eflags;
    os_int h;

    if (m_edit_value) {

        label = m_label_edit.get(this);

        switch (m_attr.showas())
        {
            case E_SHOWAS_INTEGER_NUMBER:
            case E_SHOWAS_DECIMAL_NUMBER:
                eflags = ImGuiInputTextFlags_CharsDecimal|ImGuiInputTextFlags_EnterReturnsTrue|ImGuiInputTextFlags_AutoSelectAll;
                break;

            default:
                eflags = ImGuiInputTextFlags_EnterReturnsTrue|ImGuiInputTextFlags_AutoSelectAll;
                break;
        }

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, zero_pad);
        ImGui::InputText(label, m_edit_buf.ptr(), m_edit_buf.sz(), eflags);
        if ((!ImGui::IsItemActive() || ImGui::IsItemDeactivatedAfterEdit()) && m_prev_edit_value)
        {
            eVariable value, nice_value;
            propertyv(ECOMP_VALUE, &value);
            nice_value.setv(&value);
            enice_value_for_ui(&nice_value, this, &m_attr);

            m_edit_value = false;
            if (os_strcmp(m_edit_buf.ptr(), nice_value.gets())) {
                eVariable new_value;
                new_value.sets(m_edit_buf.ptr());
                enice_ui_value_to_internal_type(&value, &new_value, this, &m_attr);
                setpropertyv(ECOMP_VALUE, &value);
            }
        }
        else {
            if (!m_prev_edit_value) {
                ImGui::SetKeyboardFocusHere(-1);
                m_prev_edit_value = true;
            }
        }

        h = ImGui::GetItemRectSize().y;
        if (h > *total_h) *total_h = h;

        ImGui::PopStyleVar();

        // WE SHOULD SET m_value rect here !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    }
    else {
        /* Draw value (not editing).
         */
        eVariable value;
        value = m_value.get(this, ECOMP_VALUE, &m_attr, ESTRBUF_SINGLELINE);
        edraw_value(&value, m_value.sbits(), this, m_attr, value_w, &m_value_rect);
        if (value_w < 0) {
            m_rect = m_value_rect;
        }

        /* Edit cell upon mouse click.
         */
        /* if (prm.mouse_click[EIMGUI_LEFT_MOUSE_BUTTON]) {
            if (erect_is_point_inside(r, prm.mouse_pos) && !prm.edit_mode) {
                activate();
            }
        } */
    }
}



/**
****************************************************************************************************

  @brief Draw tool tip, called when mouse is hovering over the value

****************************************************************************************************
*/
void eLineEdit::draw_tooltip()
{
    eVariable text, item;
    eValueX *ex;
    os_long utc;
    os_int state_bits;
    os_boolean worth_showing = OS_FALSE;

#define E_DEBUG_TOOLTIPS 0

#if E_DEBUG_TOOLTIPS
    text.sets(m_text.get(this, ECOMP_TEXT, &m_attr, ESTRBUF_SINGLELINE));
#endif
    propertyv(ECOMP_TTIP, &item);
    if (!item.isempty()) {
        if (!text.isempty()) text += "\n";
        text += item;
        worth_showing = OS_TRUE;
    }

    propertyv(ECOMP_VALUE, &item);
    ex = item.getx();
    if (ex) {
        state_bits = ex->sbits();

        utc = ex->tstamp();
        if (etime_timestamp_str(utc, &item) == ESTATUS_SUCCESS)
        {
            if (!text.isempty()) text += "\n";
            text += "updated ";
            text += item;
            worth_showing = OS_TRUE;
        }

        if ((state_bits & OSAL_STATE_CONNECTED) == 0) {
            if (!text.isempty()) text += "\n";
            text += "signal is disconnected";
            worth_showing = OS_TRUE;
        }
        if (state_bits & OSAL_STATE_ERROR_MASK) {
            if (state_bits & OSAL_STATE_CONNECTED) {
                if (!text.isempty()) text += "\n";
                text += "signal ";
            }
            else {
                text += ", ";
            }
            switch (state_bits & OSAL_STATE_ERROR_MASK)
            {
                case OSAL_STATE_YELLOW: text += "warning"; break;
                default:
                case OSAL_STATE_ORANGE: text += "error"; break;
                case OSAL_STATE_RED: text += "fault"; break;
            }
            worth_showing = OS_TRUE;
        }
    }

    if (worth_showing) {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(text.gets());
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}


/**
****************************************************************************************************

  @brief Component clicked.

  The eLineEdit::on_click() function is called when a component is clicked. If the component
  processess the mouse click, it returns OS_TRUE. This indicates that the click has been
  processed. If it doesn't process the click, it call's eComponent base classess'es on_click()
  function to try if base class wants to process the click.
  When the mouse click is not processed, it is passed to parent object in z order.

  @param   prm Drawing parameters, notice especially edit_mode.
  @param   mouse_button_nr Which mouse button, for example EIMGUI_LEFT_MOUSE_BUTTON.

  @return  OS_TRUE if mouse click was processed by this component, or OS_FALSE if not.

****************************************************************************************************
*/
os_boolean eLineEdit::on_click(
    eDrawParams& prm,
    os_int mouse_button_nr)
{
    if (!prm.edit_mode && mouse_button_nr == EIMGUI_LEFT_MOUSE_BUTTON) {
        if (erect_is_point_inside(m_value_rect, prm.mouse_pos)) {
            activate();
            return OS_TRUE;
        }
    }
    return eComponent::on_click(prm, mouse_button_nr);
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
            setpropertyi(ECOMP_VALUE, propertyi(ECOMP_VALUE) ? OS_FALSE : OS_TRUE);
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

  @brief Collect information about this object for tree browser.

  The eLineEdit::object_info function fills in item (eVariable) to contain information
  about this object for tree browser view.

  @param   item Pointer to eVariable to set up with object information.
  @param   name Object's name if known. OS_NULL if object is not named or name is
           unknown at this time.
  @param   appendix Pointer to eSet into which to store property flags. The stored property
           flags indicate if object has namespace, children, or properties.

****************************************************************************************************
*/
void eLineEdit::object_info(
    eVariable *item,
    eVariable *name,
    eSet *appendix)
{
    eVariable value;
    os_int propertynr, i;

    static os_int copy_property_list[] = {EVARP_VALUE, EVARP_TYPE, EVARP_UNIT, EVARP_ATTR,
        EVARP_DEFAULT, EVARP_ABBR, EVARP_GROUP, EVARP_TTIP, EVARP_DIGS, EVARP_MIN, EVARP_MAX,
        EVARP_GAIN, EVARP_OFFSET, 0};

    eObject::object_info(item, name, appendix);

    propertyv(ECOMP_TEXT, &value);
    if (!value.isempty()) {
        eVariable value2;
        value2 += "\"";
        value2 += value;
        value2 += "\" ";
        item->propertyv(EVARP_TEXT, &value);
        value2 += value;
        item->setpropertyv(EVARP_TEXT, &value2);
    }

    i = 0;
    while ((propertynr = copy_property_list[i++])) {
        propertyv(propertynr, &value);
        item->setpropertyv(propertynr, &value);
    }
}

