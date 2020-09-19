/**

  @file    etreenode.cpp
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
eTreeNode::eTreeNode(
    eObject *parent,
    e_oid id,
    os_int flags)
    : eComponent(parent, id, flags)
{
    /* m_value = new eVariable(this, EOID_ITEM,
        EOBJ_IS_ATTACHMENT|EOBJ_NOT_CLONABLE|EOBJ_NOT_SERIALIZABLE); */
    m_value = new eVariable(this);
    m_edit_value = false;
    m_prev_edit_value = false;
}


/**
****************************************************************************************************

  @brief Virtual destructor.

  X...

  @return  None.

****************************************************************************************************
*/
eTreeNode::~eTreeNode()
{
    delete m_value;
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
eObject *eTreeNode::clone(
    eObject *parent,
    e_oid id,
    os_int aflags)
{
    eTreeNode *clonedobj;
    clonedobj = new eTreeNode(parent, id == EOID_CHILD ? oid() : id, flags());
    clonegeneric(clonedobj, aflags);
    return clonedobj;
}


/**
****************************************************************************************************

  @brief Add eTreeNode to class list and class'es properties to it's property set.

  The eTreeNode::setupclass function adds eTreeNode to class list and class'es properties to
  it's property set. The class list enables creating new objects dynamically by class identifier,
  which is used for serialization reader functions. The property set stores static list of
  class'es properties and metadata for those.

****************************************************************************************************
*/
void eTreeNode::setupclass()
{
    const os_int cls = EGUICLASSID_TREE_NODE;

    os_lock();
    eclasslist_add(cls, (eNewObjFunc)newobj, "eTreeNode");
    eComponent::setupproperties(cls, ECOMP_VALUE_PROPERITES|ECOMP_VALUE_STATE_PROPERITES|
        ECOMP_EXTRA_UI_PROPERITES|ECOMP_CONF_PATH);
    propertysetdone(cls);
    os_unlock();
}


/**
****************************************************************************************************

  @brief Function to process incoming messages.

  The eeTreeNode::onmessage function handles messages received by object. If this function
  doesn't process message, it calls parent class'es onmessage function.

  @param   envelope Message envelope. Contains command, target and source paths and
           message content, etc.
  @return  None.

****************************************************************************************************
*/
void eTreeNode::onmessage(
    eEnvelope *envelope)
{
    /* If at final destination for the message.
     */
    if (*envelope->target()=='\0')
    {
        switch (envelope->command())
        {
            case ECMD_INFO_REPLY:
                int ulle;
                ulle = 2;
                return;
        }
    }

    /* Call parent class'es onmessage.
     */
    eObject::onmessage(envelope);
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
eStatus eTreeNode::onpropertychange(
    os_int propertynr,
    eVariable *x,
    os_int flags)
{
    switch (propertynr)
    {
        case ECOMP_VALUE: /* clear label to display new text and proceed */
            m_label_value.clear();
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
            m_attr.clear();
            break;

        default:
            break;
    }

    if (m_value->onpropertychange(propertynr, x, flags) == ESTATUS_SUCCESS) {
        // invalidate
        return ESTATUS_SUCCESS;
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
eStatus eTreeNode::simpleproperty(
    os_int propertynr,
    eVariable *x)
{
    /*
    eObject *obj;
    switch (propertynr)
    {
        case ?:
            x->setl(m_command);
            break;


        default:
            break;
    } */

    if (m_value->simpleproperty(propertynr, x) == ESTATUS_SUCCESS)
    {
        return ESTATUS_SUCCESS;
    }

    return eComponent::simpleproperty(propertynr, x);
}


/**
****************************************************************************************************

  @brief Draw the component.

  The eTreeNode::draw() function calls ImGui API to render the component.

  @param   Prm Drawing parameters.
  @return  The function return ESTATUS_SUCCESS if all is fine. Other values indicate that the
           component is no longer drawable or useful. This could be for example a pop up menu
           closed implicitely by clicking elsewhere.

****************************************************************************************************
*/
eStatus eTreeNode::draw(
    eDrawParams& prm)
{
    os_int edit_w, unit_w, total_w, unit_spacer, total_h, h;
    const os_char *label, *unit;
    ImGuiInputTextFlags eflags;

    m_attr.for_variable(this);

    ImVec2 c = ImGui::GetContentRegionAvail();
    total_w = c.x;


    ImVec2 cpos = ImGui::GetCursorScreenPos();      // ImDrawList API uses screen coordinates!
    m_rect.x1 = cpos.x;
    m_rect.y1 = cpos.y;

    ImGui::TextUnformatted(m_text.get(this, ECOMP_TEXT));
    total_h = ImGui::GetItemRectSize().y;

    //    ImGui::GetForegroundDrawList()->AddLine(prm.io->MouseClickedPos[0], prm.io->MousePos, ImGui::GetColorU32(ImGuiCol_Button), 4.0f); // Draw a line between the button and the mouse cursor


    // int edit_w = ImGui::CalcItemWidth();
    // ImGui::SameLine(total_w - edit_w);
    // edit_w = total_w - 200;

    edit_w = 200;
    unit_w = 60;
    unit_spacer = 6;

    ImGui::SameLine(total_w - edit_w - unit_spacer - unit_w);


    ImGui::SetNextItemWidth(edit_w);

    if (m_edit_value) {
        label = m_label_edit.get(this);

        switch (m_attr.showas())
        {
            case E_SHOWAS_INTEGER_NUMBER:
            case E_SHOWAS_FLOAT_NUMBER:
                eflags = ImGuiInputTextFlags_CharsDecimal|ImGuiInputTextFlags_EnterReturnsTrue;
                break;

            default:
                eflags = ImGuiInputTextFlags_EnterReturnsTrue;
                break;
        }

        ImGui::InputText(label, m_edit_buf.ptr(), m_edit_buf.sz(), eflags);
        if ((!ImGui::IsItemActive() || ImGui::IsItemDeactivatedAfterEdit()) && m_prev_edit_value)
        {
            m_edit_value = false;
            if (os_strcmp(m_edit_buf.ptr(), m_value->gets())) {
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
        label = m_label_value.get(this, ECOMP_VALUE);
        ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(1.0f, 0.5f));
        ImGui::Button(label, ImVec2(edit_w, 0));
        if (ImGui::IsItemActive()) {
request_object_info();
            m_prev_edit_value = false;
            m_edit_value = true;
            m_edit_buf.set(m_value->gets(), 256);
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

  @brief Draw the component.

  The eTreeNode::draw() function calls ImGui API to render the component.

  @param   Prm Drawing parameters.
  @return  The function return ESTATUS_SUCCESS if all is fine. Other values indicate that the
           component is no longer drawable or useful. This could be for example a pop up menu
           closed implicitely by clicking elsewhere.

****************************************************************************************************
*/
void eTreeNode::request_object_info()
{
    eVariable path;

    propertyv(ECOMP_PATH, &path);
    message(ECMD_INFO_REQUEST, path.gets());
}
