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
  Constructor.
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
    m_set_checked = true;
    m_imgui_checked = false;

    m_intermediate_node = false;
    m_node_type = 0;
    m_received_change = 0;
    m_all = OS_FALSE;
}


/**
****************************************************************************************************
  Virtual destructor.
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
    eComponent::setupproperties(cls, ECOMP_VALUE_PROPERITES|ECOMP_EXTRA_UI_PROPERITES);
    addpropertys(cls, ECOMP_PATH, ecomp_path, "path");
    addpropertys(cls, ECOMP_IPATH, ecomp_ipath, "ipath");
    addpropertyb(cls, ECOMP_REFRESH, ecomp_refresh, "refresh");
    addpropertyb(cls, ECOMP_ALL, ecomp_all, "show all");

    propertysetdone(cls);
    os_unlock();
}


/**
****************************************************************************************************

  @brief Function to process incoming messages.

  The eTreeNode::onmessage function handles messages received by object. If this function
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
    eVariable *item, path, ipath;
    eObject *pathobj;
    os_int browse_flags;

    /* If at final destination for the message.
     */
    if (*envelope->target()=='\0')
    {
        switch (envelope->command())
        {
            case ECMD_INFO_REPLY:
                m_received_change++;
                m_child_data_received = true;

                content = eContainer::cast(envelope->content());

                while ((child = firstcomponent(EOID_GUI_COMPONENT)))
                {
                    delete child;
                }

                pathobj = m_intermediate_node ? parent() : this;
                pathobj->propertyv(ECOMP_PATH, &path);
                pathobj->propertyv(ECOMP_IPATH, &ipath);

                if (osal_str_ends_with(ipath.gets(), "/") == OS_NULL) {
                    ipath += "/";
                }
                if (osal_str_ends_with(path.gets(), "/") == OS_NULL) {
                    path += "/";
                }

                browse_flags = EBROWSE_THIS_OBJECT|EBROWSE_NSPACE;
                item = content->firstv(EBROWSE_THIS_OBJECT);
                if (item) {
                    browse_flags = setup_node(item, ipath, path);
                }

                for (item = content->firstv(EBROWSE_NSPACE);
                     item;
                     item = item->nextv(EBROWSE_NSPACE))
                {
                    node = new eTreeNode(this);
                    node->m_all = m_all;
                    node->setup_node(item, ipath, path);
                }

                if (m_intermediate_node)
                {
                    for (item = content->firstv(EBROWSE_CHILDREN);
                         item;
                         item = item->nextv(EBROWSE_CHILDREN))
                    {
                        node = new eTreeNode(this);
                        node->m_all = m_all;
                        node->setup_node(item, ipath, path);
                    }
                }
                else if (browse_flags & EBROWSE_CHILDREN)
                {
                    groupnode = new eTreeNode(this);
                    groupnode->m_all = m_all;
                    groupnode->m_autoopen = false;
                    groupnode->setpropertys(ECOMP_TEXT, "children");
                    groupnode->m_node_type = EBROWSE_CHILDREN;
                    groupnode->m_intermediate_node = true;
                }

                if (m_intermediate_node)
                {
                    for (item = content->firstv(EBROWSE_PROPERTIES);
                         item;
                         item = item->nextv(EBROWSE_PROPERTIES))
                    {
                        node = new eTreeNode(this);
                        node->m_all = m_all;
                        node->setup_node(item, ipath, path);
                    }
                }
                else if (browse_flags & EBROWSE_PROPERTIES)
                {
                    groupnode = new eTreeNode(this);
                    groupnode->m_all = m_all;
                    groupnode->m_autoopen = false;
                    groupnode->setpropertys(ECOMP_TEXT, "properties");
                    groupnode->m_node_type = EBROWSE_PROPERTIES;
                    groupnode->m_intermediate_node = true;
                }

                m_received_change--;
                return;

            default:
                break;
        }
    }

    /* Call parent class'es onmessage.
     */
    eObject::onmessage(envelope);
}


/**
****************************************************************************************************

  @brief Set up this tree node.

  The eTreeNode::setup_node function sets up a tree node.

  @param   item Variable containing received data for this tree node.
  @param   ipath Internal path used by parent tree node (skipping intermediate nodes).
           This specified browsed object uniquely.
  @param   path Path of parent tree node (skipping intermediate nodes). This may specify multiple
           browsed objects.
  @return  None.

****************************************************************************************************
*/
os_int eTreeNode::setup_node(
    eVariable *item,
    eVariable& ipath,
    eVariable& path)
{
    eVariable tmp, value, ivalue, *p;
    eSet *appendix;
    eObject *o;
    os_int propertynr, browse_flags = 0;

    m_show_expand_arrow = item->oid() != EBROWSE_PROPERTIES;
    m_autoopen = false;
    m_node_type = item->oid();

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
            if (item->oid() == EBROWSE_PROPERTIES) {
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
            if (item->oid() == EBROWSE_PROPERTIES) {
                tmp += "_p/";
            }
            else {
                if (value.is_oix()) tmp.clean_to_append_oix();
            }

            tmp += value;
            setpropertyv(ECOMP_PATH, &tmp);
        }

        if (appendix->get(EBROWSE_BROWSE_FLAGS, &value)) {
            browse_flags = value.geti();
        }

#if ETREENODE_TOOLTIPS_FOR_DEBUG
        m_object_flags = (os_uint)appendix->getl(EBROWSE_OBJECT_FLAGS);
#endif

    }

    return browse_flags;
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
        case ECOMP_REFRESH:
            if (x->geti()) {
                request_object_info();
                setpropertyi(ECOMP_REFRESH, OS_FALSE);
            }
            break;

        case ECOMP_ALL:
            m_all = (os_boolean) x->geti();
            request_object_info();
            break;

        case ECOMP_VALUE:
            if (m_received_change == 0) {
                set_modified_value();
            }
            m_label_value.clear();
            m_set_checked = true;
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
            m_label_value.clear();
            m_set_checked = true;
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
    os_int text_w, edit_w, full_edit_w, unit_w, relative_x2, path_w, ipath_w, unit_spacer,
        total_w, total_h, w_left, h;
    const os_char *label, *value, *text, *unit, *path;
    ImGuiInputTextFlags eflags;
    bool isopen;

    m_attr.for_variable(this);

    relative_x2 = ImGui::GetContentRegionMax().x;
    total_w = relative_x2 - ImGui::GetCursorPosX();
    ImVec2 cpos = ImGui::GetCursorScreenPos();
    m_rect.x1 = cpos.x;
    m_rect.y1 = cpos.y;

    if (m_autoopen)
    {
        ImGui::SetNextItemOpen(true);
        m_autoopen = false;
    }

    label = m_label_node.get(this);
    text = m_text.get(this, ECOMP_TEXT);
    isopen = ImGui::TreeNodeEx(label, m_show_expand_arrow
        ? ImGuiTreeNodeFlags_None : ImGuiTreeNodeFlags_Leaf, "%s", text);

    /* If we open the component, request information.
     */
    if (isopen != m_isopen) {
        if (isopen && m_show_expand_arrow && !m_child_data_received) {
            request_object_info();
        }
        m_isopen = isopen;
    }

    total_h = ImGui::GetItemRectSize().y;

    /* Decide on column widths.
     */
    text_w = 250;
    full_edit_w = 200;
    if (m_attr.showas() == E_SHOWAS_CHECKBOX) {
        edit_w = ImGui::GetFrameHeight();
    }
    else {
        edit_w = full_edit_w;
    }
    unit_spacer = 0;
    unit_w = 0;
    path_w = 0;
    ipath_w = 0;

    w_left = relative_x2 - text_w - full_edit_w;
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
        full_edit_w += w_left;
    }

    ImGui::SameLine(relative_x2 - edit_w - unit_spacer - unit_w - path_w - ipath_w);
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
                ImGui::Button(value, ImVec2(full_edit_w, 0));
                if (ImGui::IsItemActive()) {
                    activate();
                }
                break;
        }
        ImGui::PopStyleVar();
        h = ImGui::GetItemRectSize().y;
        if (h > total_h) total_h = h;

        /* Tool tip
         */
        if (ImGui::IsItemHovered()) {
            draw_tooltip();
        }
    }

    if (unit_w > 0) {
        unit = m_unit.get(this, ECOMP_UNIT);
        if (*unit != '\0') {
            ImGui::SameLine(relative_x2 - unit_w - path_w - ipath_w);
            ImGui::SetNextItemWidth(unit_w);
            ImGui::TextUnformatted(unit);
            h = ImGui::GetItemRectSize().y;
            if (h > total_h) total_h = h;
        }
    }
    if (path_w > 0) {
        path = m_path.get(this, ECOMP_PATH);
        if (*path != '\0') {
            ImGui::SameLine(relative_x2 - path_w - ipath_w);
            ImGui::SetNextItemWidth(path_w);
            ImGui::TextUnformatted(path);
            h = ImGui::GetItemRectSize().y;
            if (h > total_h) total_h = h;
        }
    }
    if (ipath_w > 0) {
        path = m_ipath.get(this, ECOMP_IPATH);
        if (*path != '\0') {
            ImGui::SameLine(relative_x2 - ipath_w);
            ImGui::SetNextItemWidth(ipath_w);
            ImGui::TextUnformatted(path);
            h = ImGui::GetItemRectSize().y;
            if (h > total_h) total_h = h;
        }
    }

    m_rect.x2 = m_rect.x1 + total_w - 1;
    m_rect.y2 = m_rect.y1 + total_h - 1;

    if (!prm.edit_mode) {
        draw_underline(m_rect.y1 + total_h - 1);
    }

    /* Draw marker for state bits if we have an extended value.
     */
    draw_state_bits(m_rect.x2 - edit_w - unit_spacer - unit_w - path_w - ipath_w);

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

  @brief Draw line rectangle around component

****************************************************************************************************
*/
void eTreeNode::draw_underline(
    os_int y)
{
    ImDrawList* draw_list;
    ImVec2 top_left, bottom_right;
    ImU32 col;

    top_left.x = m_rect.x1;
    top_left.y = y;
    bottom_right.x = m_rect.x2;
    bottom_right.y = y+1;

    draw_list = ImGui::GetWindowDrawList();
    col = IM_COL32(128, 128, 128, 20);
    draw_list->AddRect(top_left, bottom_right, col);
}


/**
****************************************************************************************************

  @brief Draw marker for state bits if we have extended value

****************************************************************************************************
*/
void eTreeNode::draw_state_bits(
    os_int x)
{
    os_int state_bits;
    float circ_x, circ_y;
    const os_int rad = 8;

    if (!m_edit_value && m_label_value.isx())
    {
        ImVec4 colf;

        state_bits = m_label_value.sbits();
        colf = ImVec4(0.5f, 0.5f, 0.5f, 0.5f);
        switch (state_bits & OSAL_STATE_ERROR_MASK)
        {
            case OSAL_STATE_YELLOW:
                if (state_bits & OSAL_STATE_CONNECTED) {
                    colf = ImVec4(0.8f, 0.8f, 0.2f, 0.5f /* alpha */);
                }
                break;

            case OSAL_STATE_ORANGE:
                if (state_bits & OSAL_STATE_CONNECTED) {
                    colf = ImVec4(1.0f, 0.7f, 0.0f, 0.5f);
                }
                break;

            case OSAL_STATE_RED:
                colf = ImVec4(1.0f, 0.0f, 0.0f, 0.5f);
                break;

            default:
                if (state_bits & OSAL_STATE_CONNECTED) {
                    return;
                }
                break;
        }

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImU32 col = ImColor(colf);
        circ_x = (float)(x + 3*rad/2);
        circ_y = m_rect.y1 + 0.5 * (m_rect.y2 - m_rect.y1 + 1);
        draw_list->AddCircleFilled(ImVec2(circ_x, circ_y), rad, col, 0);
    }
}

/**
****************************************************************************************************

  @brief Draw tool tip, called when mouse is hovering over the value

****************************************************************************************************
*/
void eTreeNode::draw_tooltip()
{
    eVariable text, item;
    eValueX *ex;
    const os_char *str;
    os_long utc;
    os_int state_bits;
    os_boolean worth_showing = OS_FALSE;

#if ETREENODE_TOOLTIPS_FOR_DEBUG
    text.sets(m_text.get(this, ECOMP_TEXT));
#endif
    propertyv(ECOMP_TTIP, &item);
    if (!item.isempty()) {
        eliststr_appenedv(&text, &item);
        worth_showing = OS_TRUE;
    }

    propertyv(ECOMP_VALUE, &item);
    ex = item.getx();
    if (ex) {
        state_bits = ex->sbits();

        utc = ex->tstamp();
        if (etime_timestamp_str(utc, &item) == ESTATUS_SUCCESS)
        {
            eliststr_appeneds(&text, "updated ");
            text += item;
            worth_showing = OS_TRUE;
        }

        if ((state_bits & OSAL_STATE_CONNECTED) == 0) {
            eliststr_appeneds(&text, "signal is disconnected");
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

    str = m_path.get(this, ECOMP_PATH);
#if ETREENODE_TOOLTIPS_FOR_DEBUG
    if (*str != '\0')
#else
    if (*str != '\0' && os_strchr((os_char*)str, '@') == OS_NULL)
#endif
    {
        eliststr_appeneds(&text, "path: ");
        text += m_path.get(this, ECOMP_PATH);
        worth_showing = OS_TRUE;
    }

#if ETREENODE_TOOLTIPS_FOR_DEBUG
    eobjflags_to_str(&item, m_object_flags);
    if (!item.isempty()) {
        eliststr_appeneds(&text, "o-flags: ");
        text += item;
    }
#endif

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

  @brief Start editing value, toggle checkbox or show drop down list.

  The eLineEdit::activate() function is called when a value is clicked, or key (for example
  spacebar) is hit to start editing the value. Actual operation depends on metadata, the
  function can either start value edit, toggle a checkbox or show drop down list.

  @return  None.

****************************************************************************************************
*/
void eTreeNode::activate()
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

  @brief Generate right click popup menu.

  Derived component class adds call the base class'es function to generate the right click
  popup menu and then adds tree node specific items.

  @return  Pointer to the new right click popup window.

****************************************************************************************************
*/
ePopup *eTreeNode::right_click_popup()
{
    ePopup *p;
    eButton *item;
    eVariable target;
    os_char buf[E_OIXSTR_BUF_SZ];

    p = eComponent::right_click_popup();

    oixstr(buf, sizeof(buf));

    /* Generic component scope items: refresh and show all.
     */
    item = new eButton(p);
    item->setpropertys(ECOMP_TEXT, "refresh");
    item->setpropertyl(ECOMP_VALUE, 0);
    item->setpropertyl(ECOMP_SETVALUE, 1);
    target = buf; target += "/_p/refresh";
    item->setpropertyv(ECOMP_TARGET, &target);

    item = new eButton(p);
    item->setpropertys(ECOMP_TEXT, "all items");
    item->setpropertyl(ECOMP_VALUE, OS_FALSE);
    item->setpropertyl(ECOMP_SETVALUE, !m_all);
    target = buf; target += "/_p/all";
    item->setpropertyv(ECOMP_TARGET, &target);

    return p;
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
    eVariable path, *item;
    eContainer *content;
    os_int browse_flags;

    if (m_intermediate_node) {
        browse_flags = m_node_type;
        if (m_all && browse_flags == EBROWSE_CHILDREN) {
            browse_flags |= EBROWSE_ALL_CHILDREN;
        }
        parent()->propertyv(ECOMP_IPATH, &path);
    }
    else {
        browse_flags = EBROWSE_THIS_OBJECT|EBROWSE_NSPACE;
        propertyv(ECOMP_IPATH, &path);
    }

    if (!path.isempty()) {
        content = new eContainer();
        item = new eVariable(content, EBROWSE_BROWSE_FLAGS);
        item->setl(browse_flags);

        message(ECMD_INFO_REQUEST, path.gets(), OS_NULL, content, EMSG_DEL_CONTENT);
    }
}


/**
****************************************************************************************************

  @brief Set property of object

  The eTreeNode::set_modified_value() function when tree node's ECOMP_VALUE property changed by
  user setting. Member variable m_received_change is used to block calls to this function
  caused by setting received data to this function.

  @return  None.

****************************************************************************************************
*/
void eTreeNode::set_modified_value()
{
    eVariable path, value;

    propertyv(ECOMP_IPATH, &path);
    if (!path.isempty()) {
        propertyv(ECOMP_VALUE, &value);
        setpropertyv_msg(path.gets(), &value);
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
void eTreeNode::set_checked()
{
    m_imgui_checked = propertyi(ECOMP_VALUE) ? true : false;
}
