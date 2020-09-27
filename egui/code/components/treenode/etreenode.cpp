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
    m_isopen = false;
    m_autoopen = true;
    m_child_data_received = false;

    m_edit_value = false;
    m_prev_edit_value = false;
    m_show_expand_arrow = true;
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
        ECOMP_EXTRA_UI_PROPERITES|ECOMP_CONF_PATH|ECOMP_CONF_IPATH);
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
    eContainer *content;
    eTreeNode *node, *groupnode;
    eComponent *child;
    eVariable *item, *first_item, path, ipath;

    /* If at final destination for the message.
     */
    if (*envelope->target()=='\0')
    {
        switch (envelope->command())
        {
            case ECMD_INFO_REPLY:
                m_child_data_received = true;

                content = eContainer::cast(envelope->content());

                while ((child = firstcomponent(EOID_GUI_COMPONENT)))
                {
                    delete child;
                }

                propertyv(ECOMP_IPATH, &ipath);
                if (osal_str_ends_with(ipath.gets(), "/") == OS_NULL) {
                    ipath += "/";
                }
                propertyv(ECOMP_PATH, &path);
                if (osal_str_ends_with(path.gets(), "/") == OS_NULL) {
                    path += "/";
                }

                item = content->firstv(EBROWSE_THIS_OBJECT);
                if (item) {
                    setup_node(item, ipath, path);
                }

                first_item = content->firstv(EBROWSE_IN_NSPACE);
                for (item = first_item;
                     item;
                     item = item->nextv(EBROWSE_IN_NSPACE))
                {
                    node = new eTreeNode(this);
                    node->setup_node(item, ipath, path);
                }

                first_item = content->firstv(EBROWSE_CHILD);
                if (first_item)
                {
                    groupnode = new eTreeNode(this);
                    groupnode->m_autoopen = false;
                    groupnode->setpropertys(ECOMP_TEXT, "children");

                    for (item = first_item;
                         item;
                         item = item->nextv(EBROWSE_CHILD))
                    {
                        node = new eTreeNode(groupnode);
                        node->setup_node(item, ipath, path);
                    }
                }

                first_item = content->firstv(EBROWSE_PROPERTY);
                if (first_item)
                {
                    groupnode = new eTreeNode(this);
                    groupnode->m_autoopen = false;
                    groupnode->setpropertys(ECOMP_TEXT, "properties");

                    for (item = first_item;
                         item;
                         item = item->nextv(EBROWSE_PROPERTY))
                    {
                        node = new eTreeNode(groupnode);
                        node->setup_node(item, ipath, path);
                    }
                }

     /* for (eObject *o = content->first(EBROWSE_PROPERTY);
         o;
         o = o->next(EBROWSE_PROPERTY))
    {
        eObject *e = o->first(EOID_APPENDIX);
        if (e == OS_NULL) continue;
        eSet *appendix = eSet::cast(e);
        eVariable name;
        appendix->get(EBROWSE_IPATH, &name);

        osal_debug_error_str("HERE ", name.gets());
        osal_debug_error_int("HERE ", o->oid());
    } */

                return;

        }
    }

    /* Call parent class'es onmessage.
     */
    eObject::onmessage(envelope);
}

void eTreeNode::setup_node(
    eVariable *item,
    eVariable& ipath,
    eVariable& path)
{
    eVariable tmp, value, ivalue, *p;
    eSet *appendix;
    eObject *o;
    os_int propertynr;

    m_show_expand_arrow = item->oid() != EBROWSE_PROPERTY;
    m_autoopen = false;

    for (p = item->firstp(); p; p = p->nextp())
    {
        propertynr = p->oid();
        if (firstp(propertynr)) {
            item->propertyv(propertynr, &value);
            setpropertyv(propertynr, &value);
        }
    }

    o = item->first(EOID_APPENDIX);
    if (o) {
        appendix = eSet::cast(o);
        if (appendix->get(EBROWSE_IPATH, &ivalue)) {
            tmp = ipath;
            if (item->oid() == EBROWSE_PROPERTY) {
                tmp += "_p/";
            }
            else {
                if (ivalue.is_oix()) tmp.clean_to_append_oix();
            }
            tmp += ivalue;
            setpropertyv(ECOMP_IPATH, &tmp);

            if (!appendix->get(EBROWSE_PATH, &value)) {
                value = ivalue;
            }
            tmp = path;
            if (item->oid() == EBROWSE_PROPERTY) {
                tmp += "_p/";
            }
            else {
                if (value.is_oix()) tmp.clean_to_append_oix();
            }

// osal_debug_error_str("HERE ", value.gets());

            tmp += value;
            setpropertyv(ECOMP_PATH, &tmp);
        }
    }
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

        case ECOMP_DIGS:
        case ECOMP_MIN:
        case ECOMP_MAX:
        case ECOMP_TYPE:
        case ECOMP_ATTR:
            m_attr.clear();
            break;

        case ECOMP_PATH:
            m_path.clear();
            break;

        case ECOMP_IPATH:
            m_ipath.clear();
            break;

        default:
            break;
    }

    return eComponent::onpropertychange(propertynr, x, flags);
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
    eComponent *child;
    os_int text_w, edit_w, unit_w, total_w, path_w, ipath_w, unit_spacer, total_h, w_left, h;
    const os_char *label, *text, *unit, *path;
    ImGuiInputTextFlags eflags;
    bool isopen;

    m_attr.for_variable(this);


    total_w = ImGui::GetContentRegionMax().x;
    ImVec2 cpos = ImGui::GetCursorScreenPos();      // ImDrawList API uses screen coordinates!
    m_rect.x1 = cpos.x;
    m_rect.y1 = cpos.y;

    // const float spacing = ImGui::GetStyle().ItemInnerSpacing.x;

    if (m_autoopen)
    {
        ImGui::SetNextItemOpen(true);
        m_autoopen = false;
    }

    label = m_label_node.get(this);
    text = m_text.get(this, ECOMP_TEXT);
    isopen = ImGui::TreeNodeEx(label, m_show_expand_arrow ? ImGuiTreeNodeFlags_None : ImGuiTreeNodeFlags_Leaf, "%s", text);

    /* If we open the component, request information.
     */
    if (isopen != m_isopen) {
        if (isopen && m_show_expand_arrow && !m_child_data_received) {
            request_object_info();
        }
        m_isopen = isopen;
    }

    total_h = ImGui::GetItemRectSize().y;

    //    ImGui::GetForegroundDrawList()->AddLine(prm.io->MouseClickedPos[0], prm.io->MousePos, ImGui::GetColorU32(ImGuiCol_Button), 4.0f); // Draw a line between the button and the mouse cursor

    // int edit_w = ImGui::CalcItemWidth();
    // ImGui::SameLine(total_w - edit_w);
    // edit_w = total_w - 200;

    /* Decide on column widths.
     */
    text_w = 250;
    edit_w = 200;
    unit_spacer = 0;
    unit_w = 0;
    path_w = 0;
    ipath_w = 0;

    w_left = total_w - text_w - edit_w;
    if (w_left > 0) {
        unit_spacer = w_left;
        if (unit_spacer > 6) unit_spacer = 6;
        w_left -= unit_spacer;

        if (w_left > 0) {
            unit_w = w_left;
            if (unit_w > 60) unit_w = 60;
            w_left -= unit_w;

            if (w_left > 0) {
                path_w = w_left;
                if (path_w > 300) path_w = 300;
                w_left -= path_w;
                if (w_left > 0) {
                    ipath_w = w_left;
                    if (ipath_w > 300) ipath_w = 300;
                    w_left -= ipath_w;
                }
            }
        }
    }
    else if (w_left < 0)
    {
        w_left /= 2;
        text_w += w_left;
        edit_w += w_left;
    }

    ImGui::SameLine(total_w - edit_w - unit_spacer - unit_w - path_w - ipath_w);
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
            eVariable value;
            propertyv(ECOMP_VALUE, &value);
            m_edit_value = false;
            if (os_strcmp(m_edit_buf.ptr(), value.gets())) {
                setpropertys(ECOMP_VALUE, m_edit_buf.ptr());
                set_modified_value();
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
            m_prev_edit_value = false;
            m_edit_value = true;

            eVariable value;
            propertyv(ECOMP_VALUE, &value);
            m_edit_buf.set(value.gets(), 256);
        }
        ImGui::PopStyleVar();
        h = ImGui::GetItemRectSize().y;
        if (h > total_h) total_h = h;
    }

    if (unit_w > 0) {
        unit = m_unit.get(this, ECOMP_UNIT);
        if (*unit != '\0') {
            ImGui::SameLine(total_w - unit_w - path_w - ipath_w);
            ImGui::SetNextItemWidth(unit_w);
            ImGui::TextUnformatted(unit);
            h = ImGui::GetItemRectSize().y;
            if (h > total_h) total_h = h;
        }
    }
    if (path_w > 0) {
        path = m_path.get(this, ECOMP_PATH);
        if (*path != '\0') {
            ImGui::SameLine(total_w - path_w - ipath_w);
            ImGui::SetNextItemWidth(path_w);
            ImGui::TextUnformatted(path);
            h = ImGui::GetItemRectSize().y;
            if (h > total_h) total_h = h;
        }
    }
    if (ipath_w > 0) {
        path = m_ipath.get(this, ECOMP_IPATH);
        if (*path != '\0') {
            ImGui::SameLine(total_w - ipath_w);
            ImGui::SetNextItemWidth(ipath_w);
            ImGui::TextUnformatted(path);
            h = ImGui::GetItemRectSize().y;
            if (h > total_h) total_h = h;
        }
    }

    m_rect.x2 = m_rect.x1 + total_w - 1;
    m_rect.y2 = m_rect.y1 + total_h - 1;

    /* Let base class implementation handle the rest.
     */
    eComponent::draw(prm);

    if (isopen)
    {
        for (child = firstcomponent(EOID_GUI_COMPONENT);
             child;
             child = child->nextcomponent(EOID_GUI_COMPONENT))
        {
            child->draw(prm);
        }
        ImGui::TreePop();
    }

    return ESTATUS_SUCCESS;
}


/**
****************************************************************************************************

  @brief Request information about an object

  The eTreeNode::request_object_info() function...

  @return  None.

****************************************************************************************************
*/
void eTreeNode::request_object_info()
{
    eVariable path;

    propertyv(ECOMP_IPATH, &path);
    if (!path.isempty()) {
        message(ECMD_INFO_REQUEST, path.gets());
    }
}

/**
****************************************************************************************************

  @brief Set property of object

  The eTreeNode::set_modified_value() function...

  @return  None.

****************************************************************************************************
*/
void eTreeNode::set_modified_value()
{
    eVariable path, value;

    propertyv(ECOMP_IPATH, &path);
    if (!path.isempty()) {
        propertyv(ECOMP_VALUE, &value);
        setproperty_msg(path.gets(), &value);
    }
}
