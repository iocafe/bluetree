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

    m_intermediate_node = false;
    m_node_type = 0;
    m_received_change = 0;
    m_all = OS_FALSE;

#if ETREENODE_TOOLTIPS_FOR_DEBUG
    m_object_flags = 0;
#endif
    m_right_click_selections = 0;
    os_memclear(&m_value_rect, sizeof(eRect));

    /* Clear row count. Set -1 that we actually clear the parents.
     */
    m_row_count = -1;
    clear_row_count();
}


/**
****************************************************************************************************
  Virtual destructor.
****************************************************************************************************
*/
eTreeNode::~eTreeNode()
{
    m_label_node.release(this);
    m_label_edit.release(this);

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
    eclasslist_add(cls, (eNewObjFunc)newobj, "eTreeNode", EGUICLASSID_COMPONENT);
    setupproperties(cls, ECOMP_VALUE_PROPERITES|ECOMP_EXTRA_UI_PROPERITES);
    addpropertys(cls, ECOMP_PATH, ecomp_path, "path");
    addpropertys(cls, ECOMP_IPATH, ecomp_ipath, "ipath");
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

                while ((child = firstcomponent()))
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
    os_char *typ;
    os_int propertynr, browse_flags = 0;

    m_node_type = item->oid();
    m_show_expand_arrow = m_node_type != EBROWSE_PROPERTIES;
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
        if (appendix->getv(EBROWSE_IPATH, &ivalue)) {
            tmp = ipath;
            if (item->oid() == EBROWSE_PROPERTIES) {
                tmp += "_p/";
            }
            else {
                if (ivalue.is_oix()) tmp.clean_to_append_oix();
            }
            tmp += ivalue;
            setpropertyv(ECOMP_IPATH, &tmp);

            if (!appendix->getv(EBROWSE_PATH, &value)) {
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

        if (appendix->getv(EBROWSE_BROWSE_FLAGS, &value)) {
            browse_flags = value.geti();
        }

        /* Do not show expand arrow for files.
         */
        if (appendix->getv(EBROWSE_ITEM_TYPE, &value)) {
            typ = value.gets();
            if (typ[0] == 'f') {
                m_show_expand_arrow = OS_FALSE;
            }
        }

        /* Additional selections in right click popup menu.
         */
        if (appendix->getv(EBROWSE_RIGHT_CLICK_SELECTIONS, &value)) {
            m_right_click_selections = value.geti();
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
    os_int command;
    eTreeNode *p;

    switch (propertynr)
    {
        case ECOMP_COMMAND:
            command = x->geti();
            switch (command)
            {
                case ECOMPO_REFRESH:
                    request_object_info();
                    setpropertyi(ECOMP_COMMAND, ECOMPO_NO_COMMAND);
                    break;

                case ECOMPO_OPEN:
                    open_request(m_ipath.get(this, ECOMP_IPATH), EBROWSE_OPEN);
                    setpropertyi(ECOMP_COMMAND, ECOMPO_NO_COMMAND);
                    break;

                case ECOMPO_GRAPH:
                    open_request(m_ipath.get(this, ECOMP_IPATH), EBROWSE_GRAPH);
                    setpropertyi(ECOMP_COMMAND, ECOMPO_NO_COMMAND);
                    break;

                case ECOMPO_PROPERTIES:
                    p = eTreeNode::cast(parent());
                    if (p->classid() == EGUICLASSID_TREE_NODE) {
                        open_request(p->m_ipath.get(p, ECOMP_IPATH), EBROWSE_PROPERTIES);
                    }
                    setpropertyi(ECOMP_COMMAND, ECOMPO_NO_COMMAND);
                    break;

                default:
                    goto call_parent;
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
            m_value.clear();
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
            m_value.clear();
            break;

        case ECOMP_PATH:
            m_path.clear();
            break;

        case ECOMP_IPATH:
            m_ipath.clear();
            break;

        default:
            goto call_parent;
    }

    return ESTATUS_SUCCESS;

call_parent:
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
    const os_char *label, /* *value, */ *text, *unit, *path;
    bool isopen;

    add_to_zorder(prm.window, prm.layer);
    m_attr.for_variable(this);

    relative_x2 = (os_int)ImGui::GetContentRegionMax().x;
    total_w = relative_x2 - (os_int)ImGui::GetCursorPosX();
    ImVec2 cpos = ImGui::GetCursorScreenPos();
    m_rect.x1 = (os_int)cpos.x;
    m_rect.y1 = (os_int)cpos.y;

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
        clear_row_count();
    }

    total_h = (os_int)ImGui::GetItemRectSize().y;

    /* Decide on column widths.
     */
    text_w = 350;
    full_edit_w = 200;
    if (m_attr.showas() == E_SHOWAS_CHECKBOX) {
        edit_w = (os_int)ImGui::GetFrameHeight();
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

    ImGui::SameLine((float)(relative_x2 - edit_w - unit_spacer - unit_w - path_w - ipath_w));
    ImGui::SetNextItemWidth((float)edit_w);

    draw_value(prm, edit_w, &total_h);

    if (unit_w > 0) {
        unit = m_unit.get(this, ECOMP_UNIT);
        if (*unit != '\0') {
            ImGui::SameLine((float)(relative_x2 - unit_w - path_w - ipath_w));
            ImGui::SetNextItemWidth((float)unit_w);
            ImGui::TextUnformatted(unit);
            h = (os_int)ImGui::GetItemRectSize().y;
            if (h > total_h) total_h = h;
        }
    }
    if (path_w > 0) {
        path = m_path.get(this, ECOMP_PATH);
        if (*path != '\0') {
            ImGui::SameLine((float)(relative_x2 - path_w - ipath_w));
            ImGui::SetNextItemWidth((float)path_w);
            ImGui::TextUnformatted(path);
            h = (os_int)ImGui::GetItemRectSize().y;
            if (h > total_h) total_h = h;
        }
    }
    if (ipath_w > 0) {
        path = m_ipath.get(this, ECOMP_IPATH);
        if (*path != '\0') {
            ImGui::SameLine((float)(relative_x2 - ipath_w));
            ImGui::SetNextItemWidth((float)ipath_w);
            ImGui::TextUnformatted(path);
            h = (os_int)ImGui::GetItemRectSize().y;
            if (h > total_h) total_h = h;
        }
    }

    m_rect.x2 = m_rect.x1 + total_w - 1;
    m_rect.y2 = m_rect.y1 + total_h - 1;

    if (!prm.edit_mode) {
        draw_underline(m_rect.y1 + total_h - 1);
    }

    if (isopen)
    {
        for (child = firstcomponent();
             child;
             child = child->nextcomponent())
        {
            child->draw(prm);
        }
        ImGui::TreePop();
    }

    /* Let base class implementation handle the rest.
     */
    eComponent::draw(prm);

    return ESTATUS_SUCCESS;
}


void eTreeNode::draw_in_parameter_list(
    eDrawParams& prm)
{
    os_int total_h = 0;
    eComponent *child;
    const os_char *label, *text, *unit, *path;
    bool isopen;

    add_to_zorder(prm.window, prm.layer);
    m_attr.for_variable(this);

    if (m_autoopen)
    {
        ImGui::SetNextItemOpen(true);
        m_autoopen = false;
    }

    if (!ImGui::TableSetColumnIndex(0)) {
        return;
    }

    ImGuiTreeNodeFlags tnflags = m_show_expand_arrow
        ? ImGuiTreeNodeFlags_SpanFullWidth
        : ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_SpanFullWidth;

    label = m_label_node.get(this);
    text = m_text.get(this, ECOMP_TEXT);
    isopen = ImGui::TreeNodeEx(label, tnflags, "%s", text);


    if (prm.mouse_click[EIMGUI_LEFT_MOUSE_BUTTON] &&
        (prm.mouse_click_keyboard_flags[EIMGUI_LEFT_MOUSE_BUTTON] & EDRAW_LEFT_CTRL_DOWN))
    {
        if (!prm.edit_mode && ImGui::IsItemHovered())
        {
            if (m_right_click_selections & EBROWSE_OPEN) {
                open_request(m_ipath.get(this, ECOMP_IPATH), EBROWSE_OPEN);
            }
            else if (m_intermediate_node && m_node_type == EBROWSE_PROPERTIES) {
                eTreeNode *p = eTreeNode::cast(parent());
                if (p->classid() == EGUICLASSID_TREE_NODE) {
                    open_request(p->m_ipath.get(p, ECOMP_IPATH), EBROWSE_PROPERTIES);
                }
            }
        }
    }

    /* If we open the component, request information.
     */
    if (isopen != m_isopen) {
        if (isopen && m_show_expand_arrow && !m_child_data_received) {
            request_object_info();
        }
        m_isopen = isopen;
        clear_row_count();
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

    if (ImGui::TableSetColumnIndex(3)) {
        path = m_path.get(this, ECOMP_PATH, &m_attr, ESTRBUF_SINGLELINE);
        if (*path != '\0') {
            ImGui::TextUnformatted(path);
        }
    }

    if (isopen)
    {
        for (child = firstcomponent();
             child;
             child = child->nextcomponent())
        {
            ImGui::TableNextRow();

            child->draw_in_parameter_list(prm);
        }
        ImGui::TreePop();
    }

    /* Let base class implementation handle the rest.
     */
    eComponent::draw(prm);
}


/* value_w Set -1 if drawing in table.
 */
void eTreeNode::draw_value(
    eDrawParams& prm,
    os_int value_w,
    os_int *total_h)
{
    const os_char *label;
    const ImVec2 zero_pad(0, 0);
    ImGuiInputTextFlags eflags;
    os_int h;

    if (m_edit_value)
    {
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
        if (value_w < 0) ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::InputText(label, m_edit_buf.ptr(), (size_t)m_edit_buf.sz(), eflags);
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

        h = (os_int)ImGui::GetItemRectSize().y;
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
    }
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

    top_left.x = (float)m_rect.x1;
    top_left.y = (float)y;
    bottom_right.x = (float)m_rect.x2;
    bottom_right.y = (float)(y + 1);

    draw_list = ImGui::GetWindowDrawList();
    col = IM_COL32(128, 128, 128, 20);
    draw_list->AddRect(top_left, bottom_right, col);
}


/**
****************************************************************************************************

  @brief Draw tool tip, called when mouse is hovering over the value

****************************************************************************************************
*/
void eTreeNode::draw_tooltip()
{
    eVariable value, flagvar;
    os_char *flagstr;

    propertyv(ECOMP_VALUE, &value);
    eobjflags_to_str(&flagvar, m_object_flags);
    flagstr = flagvar.gets();

    edraw_tooltip(this, &value,
        m_text.get(this, ECOMP_TEXT, &m_attr, ESTRBUF_SINGLELINE),
        flagstr,
        m_attr, EDRAW_TTIP_PATH|EDRAW_TTIP_IPATH);
}


/**
****************************************************************************************************

  @brief Component clicked.

  The eTreeNode::on_click() function is called when a component is clicked. If the component
  processess the mouse click, it returns OS_TRUE. This indicates that the click has been
  processed. If it doesn't process the click, it call's eComponent base classess'es on_click()
  function to try if base class wants to process the click.
  When the mouse click is not processed, it is passed to parent object in z order.

  @param   prm Drawing parameters, notice especially edit_mode.
  @param   mouse_button_nr Which mouse button, for example EIMGUI_LEFT_MOUSE_BUTTON.

  @return  OS_TRUE if mouse click was processed by this component, or OS_FALSE if not.

****************************************************************************************************
*/
os_boolean eTreeNode::on_click(
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
void eTreeNode::activate()
{
    switch (m_attr.showas())
    {
        case E_SHOWAS_CHECKBOX:
            setpropertyi(ECOMP_VALUE, propertyi(ECOMP_VALUE) ? OS_FALSE : OS_TRUE);
            break;

        case E_SHOWAS_DROP_DOWN_ENUM:
        case E_SHOWAS_DROP_DOWN_LIST:
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
ePopup *eTreeNode::right_click_popup(
    eDrawParams& prm)
{
    ePopup *p;
    p = eComponent::right_click_popup(prm);

    /* Additional right click selections from object.
     */
    if (m_right_click_selections & EBROWSE_OPEN)
    {
        add_popup_item_command("open", ECOMPO_OPEN, p);
    }
    if (m_right_click_selections & EBROWSE_GRAPH) {
        add_popup_item_command("graph", ECOMPO_GRAPH, p);
    }
    if (m_intermediate_node && m_node_type == EBROWSE_PROPERTIES) {
        add_popup_item_command("open", ECOMPO_PROPERTIES, p);
    }

    /* Other component scope items: refresh and show all.
     */
    add_popup_item_command("refresh", ECOMPO_REFRESH, p);
    add_popup_item_toggle("all items", ECOMP_ALL, ecomp_all, p);

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
        content = new eContainer(this, EOID_ITEM, EOBJ_IS_ATTACHMENT);
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

  @brief Count number or text rows needed by this node and open subnodes.

  The tree browser is displayed within eParameterList, we need to know how many rows are visible
  to make Y scrolling work.

  @return  Number of text rows needed to display this node and open subnodes.

****************************************************************************************************
*/
os_int eTreeNode::count_rows()
{
    eComponent *child;
    eTreeNode *node;

    if (!m_isopen) {
        return 1;
    }
    if (m_row_count > 0) {
        return m_row_count;
    }

    m_row_count = 1;
    for (child = firstcomponent();
         child;
         child = child->nextcomponent())
    {
        if (child->classid() == EGUICLASSID_TREE_NODE) {
            node = eTreeNode::cast(child);
            m_row_count += node->count_rows();
        }
        else {
            m_row_count++;
        }
    }
    return m_row_count;
}


/**
****************************************************************************************************

  @brief Clear visible row count.

  This function clears visible row count of this node and parents. This forces recount when
  the count_rows() is called the next time.

****************************************************************************************************
*/
void eTreeNode::clear_row_count()
{
    eObject *p;
    eTreeNode *node;

    m_row_count = 0;
    p = parent();
    if (p) if (p->classid() == EGUICLASSID_TREE_NODE) {
        node = eTreeNode::cast(p);
        node->clear_row_count();
    }
}
