/**

  @file    eparameterlist.cpp
  @brief   Display table data in GUI.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "egui.h"


/**
****************************************************************************************************
  Constructor, clear member variables.
****************************************************************************************************
*/
eParameterList::eParameterList(
    eObject *parent,
    e_oid id,
    os_int flags)
    : eComponent(parent, id, flags)
{
    m_component = OS_NULL;
    m_nro_components = 0;
    m_component_array_sz = 0;
    m_row_count = 0;
    m_treebrowser_row_count = 0;
}


/**
****************************************************************************************************
  Virtual destructor.
****************************************************************************************************
*/
eParameterList::~eParameterList()
{
    os_free(m_component, m_component_array_sz);
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
eObject *eParameterList::clone(
    eObject *parent,
    e_oid id,
    os_int aflags)
{
    eParameterList *clonedobj;
    clonedobj = new eParameterList(parent, id == EOID_CHILD ? oid() : id, flags());
    clonegeneric(clonedobj, aflags);
    return clonedobj;
}


/**
****************************************************************************************************

  @brief Add eParameterList to class list and class'es properties to it's property set.

  The eParameterList::setupclass function adds eParameterList to class list and class'es properties to
  it's property set. The class list enables creating new objects dynamically by class identifier,
  which is used for serialization reader functions. The property set stores static list of
  class'es properties and metadata for those.

****************************************************************************************************
*/
void eParameterList::setupclass()
{
    const os_int cls = EGUICLASSID_PARAMETER_LIST;

    os_lock();
    eclasslist_add(cls, (eNewObjFunc)newobj, "eParameterList", EGUICLASSID_COMPONENT);
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
eStatus eParameterList::onpropertychange(
    os_int propertynr,
    eVariable *x,
    os_int flags)
{
    switch (propertynr)
    {
        case ECOMP_VALUE: /* table name (always)  */
            // m_label_value.clear();
            // m_set_checked = true;
            break;

        case ECOMP_TEXT: /* Translatable table name */
            // m_text.clear();
            break;

        default:
            return eComponent::onpropertychange(propertynr, x, flags);
    }

    return ESTATUS_SUCCESS;
}


/**
****************************************************************************************************

  @brief Draw the component.

  The eParameterList::draw() function calls ImGui API to render the component.

  @param   Prm Drawing parameters.
  @return  The function return ESTATUS_SUCCESS if all is fine. Other values indicate that the
           component is no longer drawable or useful. This could be for example a pop up menu
           closed implicitely by clicking elsewhere.

****************************************************************************************************
*/
eStatus eParameterList::draw(
    eDrawParams& prm)
{
    eComponent *c;
    os_int total_w, total_h, ys, ncols;
    os_int row; // , column;
    ImVec2 size, rmax, origin;
    ImVec2 cpos;
    ImGuiTableFlags flags;

    // const float TEXT_BASE_WIDTH = ImGui::CalcTextSize("A").x;
    const float TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();

    add_to_zorder(prm.window, prm.layer);

    generate_component_array();

    // Using those as a base value to create width/height that are factor of the size of our font

    /* flags = ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg |
        ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable |
        ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable; */

    // Using those as a base value to create width/height that are factor of the size of our font

    flags = // ImGuiTableFlags_ScrollX |
        ImGuiTableFlags_ScrollY |
        ImGuiTableFlags_BordersInner |
        // ImGuiTableFlags_NoBordersInBodyUntilResize |
        ImGuiTableFlags_NoPadOuterX |
        ImGuiTableFlags_Resizable |
        ImGuiTableFlags_Reorderable |
        // ImGuiTableFlags_SizingPolicyStretchX |
        ImGuiTableFlags_SizingStretchProp |
        ImGuiTableFlags_NoSavedSettings;


    static int freeze_cols = 1;
    static int freeze_rows = 0; // 1;
    ncols = (m_row_count == m_treebrowser_row_count) ? 4 : 3;

    // When using ScrollX or ScrollY we need to specify a size for our table container!
    // Otherwise by default the table will fit all available space, like a BeginChild() call.
    /* size = ImVec2(0, m_treebrowser_row_count
        ? TEXT_BASE_HEIGHT * m_treebrowser_row_count
        : TEXT_BASE_HEIGHT * m_nro_components); */
    size = ImVec2(0, TEXT_BASE_HEIGHT * m_row_count);

    if (ImGui::BeginTable("##table3", ncols, flags, size))
    {
        rmax = ImGui::GetContentRegionMax();
        origin = ImGui::GetCursorPos();
        ys = (os_int)ImGui::GetScrollY();
        total_w = (os_int)(rmax.x - origin.x);
        total_h = (os_int)(rmax.y - origin.y);

        cpos = ImGui::GetCursorScreenPos();
        m_rect.x1 = (os_int)cpos.x;
        m_rect.y1 = (os_int)cpos.y + ys;

        m_rect.x2 = m_rect.x1 + total_w - 1;
        m_rect.y2 = m_rect.y1 + total_h - 1;

/* {
ImDrawList* draw_list;
ImVec2 top_left, bottom_right;
top_left.x = m_rect.x1;
top_left.y = m_rect.y1;
bottom_right.x = m_rect.x2;
bottom_right.y = m_rect.y2;
ImU32  col = IM_COL32(48, 48, 255, 250);

draw_list = ImGui::GetWindowDrawList();
draw_list->AddRect(top_left, bottom_right, col, 0,
    ImDrawCornerFlags_All, 2);
} */


        ImGui::TableSetupScrollFreeze(freeze_cols, freeze_rows);

        ImGui::TableSetupColumn("name", ImGuiTableColumnFlags_NoHide,
            (float)(m_treebrowser_row_count ? 200 : 150));
        ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_NoHide, 150);
        ImGui::TableSetupColumn("unit", ImGuiTableColumnFlags_NoHide, 30);
        if (m_treebrowser_row_count) {
            ImGui::TableSetupColumn("path", ImGuiTableColumnFlags_NoHide, 150);
        }

        // ImGui::TableHeadersRow();

        ImGuiListClipper clipper;
        clipper.Begin(m_row_count);
        while (clipper.Step())
        {
            for (row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
            {
                if (row < 0 || row >= m_nro_components) {
                    break;
                }

                ImGui::TableNextRow();

                c = m_component[row].m_ptr;

                if (!ImGui::TableSetColumnIndex(0)) {
                    continue;
                }
                c->draw_in_parameter_list(prm);
            }
        }

        ImGui::EndTable();
    }

    /* Let base class implementation handle the rest.
     */
    return eComponent::draw(prm);
}


/**
****************************************************************************************************

  @brief Generate array of child component pointers.

****************************************************************************************************
*/
void eParameterList::generate_component_array()
{
    eComponent *c;
    os_memsz sz;
    os_int nro_components, max_components, count;

    nro_components = 16;

try_again:
    max_components = (os_int)(m_component_array_sz / sizeof(ePrmListComponent));
    if (m_component == OS_NULL || nro_components > max_components)
    {
        os_free(m_component, m_component_array_sz);
        sz = (nro_components + 4) * sizeof(ePrmListComponent);
        m_component = (ePrmListComponent*)os_malloc(sz, &m_component_array_sz);
        max_components = (os_int)(m_component_array_sz / sizeof(ePrmListComponent));
    }

    m_row_count = 0;
    m_treebrowser_row_count = 0;

    for (c = firstcomponent(), nro_components = 0;
         c;
         c = c->nextcomponent(), nro_components++)
    {
        if (nro_components < max_components) {
            m_component[nro_components].m_ptr = c;
        }

        if (c->classid() == EGUICLASSID_TREE_NODE) {
            count = eTreeNode::cast(c)->count_rows();
            m_row_count += count;
            m_treebrowser_row_count += count;
        }
        else {
            m_row_count++;
        }
    }

    if (nro_components > max_components) {
        goto try_again;
    }
    m_nro_components = nro_components;
}


/**
****************************************************************************************************

  @brief Generate right click popup menu.

  Derived component class adds call the base class'es function to generate the right click
  popup menu and then adds tree node specific items.

  @return  Pointer to the new right click popup window.

****************************************************************************************************
*/
ePopup *eParameterList::right_click_popup(
    eDrawParams& prm)
{
    ePopup *p;

    p = eComponent::right_click_popup(prm);
    return p;
}


