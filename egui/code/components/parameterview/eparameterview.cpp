/**

  @file    eparameterview.cpp
  @brief   Display table data in GUI.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    2.11.2020

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
eParameterView::eParameterView(
    eObject *parent,
    e_oid id,
    os_int flags)
    : eComponent(parent, id, flags)
{
}


/**
****************************************************************************************************
  Virtual destructor.
****************************************************************************************************
*/
eParameterView::~eParameterView()
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
eObject *eParameterView::clone(
    eObject *parent,
    e_oid id,
    os_int aflags)
{
    eParameterView *clonedobj;
    clonedobj = new eParameterView(parent, id == EOID_CHILD ? oid() : id, flags());
    clonegeneric(clonedobj, aflags);
    return clonedobj;
}


/**
****************************************************************************************************

  @brief Add eParameterView to class list and class'es properties to it's property set.

  The eParameterView::setupclass function adds eParameterView to class list and class'es properties to
  it's property set. The class list enables creating new objects dynamically by class identifier,
  which is used for serialization reader functions. The property set stores static list of
  class'es properties and metadata for those.

****************************************************************************************************
*/
void eParameterView::setupclass()
{
    const os_int cls = EGUICLASSID_PARAMETER_VIEW;

    os_lock();
    eclasslist_add(cls, (eNewObjFunc)newobj, "eParameterView");
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
eStatus eParameterView::onpropertychange(
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

  The eParameterView::draw() function calls ImGui API to render the component.

  @param   Prm Drawing parameters.
  @return  The function return ESTATUS_SUCCESS if all is fine. Other values indicate that the
           component is no longer drawable or useful. This could be for example a pop up menu
           closed implicitely by clicking elsewhere.

****************************************************************************************************
*/
eStatus eParameterView::draw(
    eDrawParams& prm)
{
#if 0
    eTableColumn *c;
    eMatrix *m, *focused_m;
    eVariable *value;
    os_int nrows, ncols, total_w, total_h;
    os_int row, column;
    ImVec2 size, rmax, origin;
    ImVec2 cpos;
    ImGuiTableFlags flags;
    os_boolean first_row;

    // const float TEXT_BASE_WIDTH = ImGui::CalcTextSize("A").x;
    const float TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();

    add_to_zorder(prm.window);

    if (m_rowset == OS_NULL || m_columns == OS_NULL) {
        goto skipit;
    }

    nrows = m_rowset->nrows();
    ncols = m_rowset->ncolumns();
    if (ncols <= 0) {
        goto skipit;
    }

    // Using those as a base value to create width/height that are factor of the size of our font

    flags = ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg |
        ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable |
        ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;

    static int freeze_cols = 1;
    static int freeze_rows = 1;

    // When using ScrollX or ScrollY we need to specify a size for our table container!
    // Otherwise by default the table will fit all available space, like a BeginChild() call.
    size = ImVec2(0, TEXT_BASE_HEIGHT * 8);
    if (ImGui::BeginTable("##table2", ncols, flags, size))
    {
        rmax = ImGui::GetContentRegionMax();
        origin = ImGui::GetCursorPos();
        total_w = rmax.x - origin.x;
        total_h = rmax.y - origin.y;

        cpos = ImGui::GetCursorScreenPos();
        m_rect.x1 = cpos.x;
        m_rect.y1 = cpos.y;

        m_rect.x2 = m_rect.x1 + total_w - 1;
        m_rect.y2 = m_rect.y1 + total_h - 1;

/* ImDrawList* draw_list;
ImVec2 top_left, bottom_right;
top_left.x = m_rect.x1;
top_left.y = m_rect.y1;
bottom_right.x = m_rect.x2;
bottom_right.y = m_rect.y2;
ImU32  col = IM_COL32(48, 48, 255, 250);

draw_list = ImGui::GetWindowDrawList();
draw_list->AddRect(top_left, bottom_right, col, 0,
    ImDrawCornerFlags_All, 2); */


        ImGui::TableSetupScrollFreeze(freeze_cols, freeze_rows);

        for (c = eTableColumn::cast(m_columns->first()), column=0;
             c && column < ncols;
             c = eTableColumn::cast(c->next()), column++)
        {
            c->draw_column_header();
        }

        ImGui::TableHeadersRow();

        first_row = OS_TRUE;
        focused_m = eMatrix::cast(m_focused_row->get());
        value = new eVariable(this);
        ImGuiListClipper clipper;
        clipper.Begin(nrows);
        while (clipper.Step())
        {
            for (row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
            {
                ImGui::TableNextRow();

                /* Avoid crashing on program errors?
                 */
                if (row < 0 || row >= m_row_to_m_len) {
                    continue;
                }
                m = m_row_to_m[row].m_row;

                for (c = eTableColumn::cast(m_columns->first()), column=0;
                     c && column < ncols;
                     c = eTableColumn::cast(c->next()), column++)
                {
                    m->getv(0, column, value);

                    // Both TableNextColumn() and TableSetColumnIndex() return false when
                    // a column is not visible, which can be used for clipping.
                    if (first_row) {
                        c->set_visible(ImGui::TableSetColumnIndex(column));
                        if (!c->visible()) continue;
                        if (value->isempty()) continue;
                    }
                    else {
                        if (!c->visible()) continue;
                        if (value->isempty()) continue;

                        if (!ImGui::TableSetColumnIndex(column)) {
                            continue;
                        }
                    }

                    if (m == focused_m &&
                        column == m_focused_column)
                    {
                        //c->draw_edit(value, m_rowset->ix_column_name(),
//                            m->getl(0, m_rowset->ix_column_nr()), this);
                    }
                    else {
  //                      c->draw_value(value, this);
                    }
                }
                first_row = OS_FALSE;
            }
        }
        delete value;

        if (prm.mouse_click[EIMGUI_LEFT_MOUSE_BUTTON])
        {
            column = ImGui::TableGetHoveredColumn();
            if (column >= 0 && column < m_row_to_m_len) {
                c = eTableColumn::cast(m_columns->first(column));
                row = (prm.mouse_pos.y - clipper.StartPosY) / TEXT_BASE_HEIGHT;
                if (c && row >= 0 && row < nrows) {
                    m = m_row_to_m[row].m_row;
                    c->activate(m, column, this);
                }
            }
        }

        ImGui::EndTable();
    }

    /* Tool tip
     */
    if (ImGui::IsItemHovered()) {
        draw_tooltip();
    }

skipit:


#endif

    /* Let base class implementation handle the rest.
     */
    return eComponent::draw(prm);
}


/**
****************************************************************************************************

  @brief Generate right click popup menu.

  Derived component class adds call the base class'es function to generate the right click
  popup menu and then adds tree node specific items.

  @return  Pointer to the new right click popup window.

****************************************************************************************************
*/
ePopup *eParameterView::right_click_popup(
    eDrawParams& prm)
{
    ePopup *p;
    //eButton *item;
    // eVariable target;
    os_char buf[E_OIXSTR_BUF_SZ];

    p = eComponent::right_click_popup(prm);
    oixstr(buf, sizeof(buf));

    /* Generic component scope items: refresh and show all.
     */
    /* item = new eButton(p);
    item->setpropertys(ECOMP_TEXT, "new row");
    item->setpropertyl(ECOMP_VALUE, 0);
    item->setpropertyl(ECOMP_SETVALUE, ECOMPO_REFRESH);
    target = buf; target += "/_p/_command";
    item->setpropertyv(ECOMP_TARGET, &target); */

    return p;
}
