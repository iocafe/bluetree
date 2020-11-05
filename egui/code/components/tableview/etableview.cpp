/**

  @file    etableview.cpp
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
eTableView::eTableView(
    eObject *parent,
    e_oid id,
    os_int flags)
    : eComponent(parent, id, flags)
{
    m_rowset = OS_NULL;
    m_row_to_m = OS_NULL;
    m_row_to_m_sz = 0;
    m_row_to_m_len = 0;
    m_columns = OS_NULL;

    m_focused_row = new ePointer(this);
    m_focused_column = -1;
    m_keyboard_focus_ok = OS_FALSE;

select();
}


/**
****************************************************************************************************
  Virtual destructor.
****************************************************************************************************
*/
eTableView::~eTableView()
{
    os_free(m_row_to_m, m_row_to_m_sz);
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
eObject *eTableView::clone(
    eObject *parent,
    e_oid id,
    os_int aflags)
{
    eTableView *clonedobj;
    clonedobj = new eTableView(parent, id == EOID_CHILD ? oid() : id, flags());
    clonegeneric(clonedobj, aflags);
    return clonedobj;
}


/**
****************************************************************************************************

  @brief Add eTableView to class list and class'es properties to it's property set.

  The eTableView::setupclass function adds eTableView to class list and class'es properties to
  it's property set. The class list enables creating new objects dynamically by class identifier,
  which is used for serialization reader functions. The property set stores static list of
  class'es properties and metadata for those.

****************************************************************************************************
*/
void eTableView::setupclass()
{
    const os_int cls = EGUICLASSID_TABLE_VIEW;

    os_lock();
    eclasslist_add(cls, (eNewObjFunc)newobj, "eTableView");
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
eStatus eTableView::onpropertychange(
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

  The eTableView::draw() function calls ImGui API to render the component.

  @param   Prm Drawing parameters.
  @return  The function return ESTATUS_SUCCESS if all is fine. Other values indicate that the
           component is no longer drawable or useful. This could be for example a pop up menu
           closed implicitely by clicking elsewhere.

****************************************************************************************************
*/
eStatus eTableView::draw(
    eDrawParams& prm)
{
    eTableColumn *c;
    eMatrix *m, *focused_m;
    eVariable *value;
    os_int nrows, ncols, relative_x2, total_w, total_h;
    os_int row, column;
    ImVec2 size;
    ImGuiTableFlags flags;
    os_boolean first_row;

    // const float TEXT_BASE_WIDTH = ImGui::CalcTextSize("A").x;
    const float TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();

    add_to_zorder(prm.window);

    relative_x2 = ImGui::GetContentRegionMax().x;
    total_w = relative_x2 - ImGui::GetCursorPosX();
    total_h = 100;

    ImVec2 cpos = ImGui::GetCursorScreenPos();
    m_rect.x1 = cpos.x;
    m_rect.y1 = cpos.y;

    m_rect.x2 = m_rect.x1 + total_w - 1;
    m_rect.y2 = m_rect.y1 + total_h - 1;

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
                        c->draw_edit(value, this);
                    }
                    else {
                        c->draw_value(value, this);
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
    /* Let base class implementation handle the rest.
     */
    return eComponent::draw(prm);
}

/**
****************************************************************************************************

  @brief Switch a cell to edit mode and set keyboard focus to it

****************************************************************************************************
*/
void eTableView::focus_cell(
    eMatrix *focus_row,
    os_int focus_column,
    const os_char *edit_str,
    os_int edit_sz)
{
    if (m_focused_row->get() != focus_row ||
        m_focused_column != focus_column)
    {
        m_focused_row->set(focus_row);
        m_focused_column = focus_column;
        m_edit_buf.set(edit_str, edit_sz);
        set_keyboard_focus_ok(OS_FALSE);
    }
}


/**
****************************************************************************************************

  @brief Draw tool tip, called when mouse is hovering over the value

****************************************************************************************************
*/
void eTableView::draw_tooltip()
{
    eVariable text, item;
    eValueX *ex;
    os_long utc;
    os_int state_bits;
    os_boolean worth_showing = OS_FALSE;

#define E_DEBUG_TOOLTIPS 0

#if E_DEBUG_TOOLTIPS
    text.sets(m_text.get(this, ECOMP_TEXT));
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

  @brief Select the data to display.

  The eTableView::select() function..

****************************************************************************************************
*/
void eTableView::select()
{
    eContainer columns;
    eVariable *column;

    if (m_rowset == OS_NULL) {
        m_rowset = new eRowSet(this);
    }

    m_rowset->set_dbm("//mymtx");
    m_rowset->set_callback(eTableView::static_callback, this);
    column = new eVariable(&columns);
    column->addname("*", ENAME_NO_MAP);
    m_rowset->select("*", &columns);
}


/**
****************************************************************************************************

  @brief Callback when table data is received, etc.

  The eTableView::callback() is called by row set when the row set received table configuration,
  inital table data or modifications from table data eDBM.

  @param   rset Pointer to the row set.
  @param   ci A structure containing information about callback.

****************************************************************************************************
*/
void eTableView::callback(
    eRowSet *rset,
    ersetCallbackInfo *ci)
{
    switch (ci->event) {
        case ERSET_TABLE_BINDING_COMPLETE:
            osal_console_write("binding done");
            setup_columns();
            fill_row_to_m();
            break;

        case ERSET_INITIAL_DATA_RECEIVED:
        case ERSET_MODIFICATIONS_RECEIVED:
            fill_row_to_m();
            break;
    }

    osal_console_write("eRowSet callback\n");
}


/**
****************************************************************************************************

  @brief Static callback function to use underlying C code.

  This function exists to have C compatible function pointer. The static callback function just
  forwards the callback to eTableView::callback().

  @param   rset Pointer to the row set.
  @param   ci A structure containing information about callback.
  @param   context Pointer to the eTableView object.

****************************************************************************************************
*/
void eTableView::static_callback(
    eRowSet *rset,
    ersetCallbackInfo *ci,
    eObject *context)
{
    if (rset) {
        ((eTableView*)context)->callback(rset, ci);
    }
}


/**
****************************************************************************************************

  @brief Set up an array for converting row number to row data matrix pointer.

  Setup m_row_to_m array, converts row number 0... to eMatrix pointer.

  Member variable m_row_to_m_len is set to number items in m_row_to_m array (number of rows),
  mostly for catching program errors.

  Current memory allocation for m_row_to_m is kept in m_row_to_m_sz. The m_row_to_m_sz array
  is expanded as needed.

****************************************************************************************************
*/
void eTableView::fill_row_to_m()
{
    eMatrix *m;
    os_memsz sz;
    os_int nrows, row;

    if (m_rowset == OS_NULL) return;

    nrows = m_rowset->nrows();
    sz = nrows * sizeof(eTableRow);

    /* Fill in table to convert row number to eMatrix pointer.
     */
    if (m_row_to_m == OS_NULL || sz > m_row_to_m_sz)
    {
        os_free(m_row_to_m, m_row_to_m_sz);
        if (sz < (os_memsz)(10 * sizeof(eTableRow))) {
            sz = 10 * sizeof(eTableRow);
        }
        else {
            sz = 3 * sz / 2;
        }
        m_row_to_m = (eTableRow*)os_malloc(sz, &m_row_to_m_sz);
    }

    for (m = m_rowset->firstm(), row = 0; m && row < nrows; m = m->nextm(), row++)
    {
        os_memclear(&m_row_to_m[row], sizeof(eTableRow));
        m_row_to_m[row].m_row = m;
    }
    osal_debug_assert(m == OS_NULL && row == nrows);
    m_row_to_m_len = row;
}


/**
****************************************************************************************************

  @brief Create eTableColumn object for each column.

  An eTableColumn is used to maintain information for column in format ready for rendering.
  It contains translated column header, parsed column attributes, etc.

****************************************************************************************************
*/
void eTableView::setup_columns()
{
    eContainer *rscols;
    eVariable *v;
    eTableColumn *c;
    os_int col_nr;

    if (m_rowset == OS_NULL) return;
    rscols = m_rowset->columns();
    if (rscols == OS_NULL) return;

    if (m_columns == OS_NULL) {
        m_columns = new eContainer(this);
    }
    else {
        m_columns->clear();
    }

    for (v = rscols->firstv(), col_nr = 0; v; v = v->nextv(), col_nr++)
    {
        c = new eTableColumn(m_columns, col_nr);
        c->setup_column(v);
    }
}


/**
****************************************************************************************************

  @brief Collect information about this object for tree browser.

  The eTableView::object_info function fills in item (eVariable) to contain information
  about this object for tree browser view.

  @param   item Pointer to eVariable to set up with object information.
  @param   name Object's name if known. OS_NULL if object is not named or name is
           unknown at this time.
  @param   appendix Pointer to eSet into which to store property flags. The stored property
           flags indicate if object has namespace, children, or properties.

****************************************************************************************************
*/
void eTableView::object_info(
    eVariable *item,
    eVariable *name,
    eSet *appendix)
{
    eVariable value;

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
}

