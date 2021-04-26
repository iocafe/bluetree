/**

  @file    etableview.cpp
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
#include "imgui_internal.h"

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
    m_logical_data_start_y = 0;
    m_data_windows_start_y = 0;
    m_data_row_h = 24;
    m_hovered_column = -1;

    m_focused_row = new ePointer(this);
    m_focused_column = -1;
    m_keyboard_focus_ok = OS_FALSE;
    m_nro_header_row_lines = 1;

    m_row_dialog = new ePointer(this);
    m_row_dialog_m = new ePointer(this);
}


/**
****************************************************************************************************
  Virtual destructor.
****************************************************************************************************
*/
eTableView::~eTableView()
{
    m_label_edit.release(this);
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
    eclasslist_add(cls, (eNewObjFunc)newobj, "eTableView", EGUICLASSID_COMPONENT);
    // setupproperties(cls, ECOMP_VALUE_PROPERITES|ECOMP_EXTRA_UI_PROPERITES);
    addpropertys(cls, ECOMP_PATH, ecomp_path, "path", EPRO_PERSISTENT);
    addpropertyl(cls, ECOMP_DROP_DOWN_LIST_SELECT, ecomp_drop_down_list_select,
        "drop down select", EPRO_SIMPLE);
    addpropertyl(cls, ECOMP_COMMAND, ecomp_command, "command");
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
    eMatrix *focused_row;
    eTableColumn *c;

    switch (propertynr)
    {
        case ECOMP_PATH:
            delete m_rowset;
            m_rowset = OS_NULL;
            break;

        case ECOMP_DROP_DOWN_LIST_SELECT:
            focused_row = eMatrix::cast(m_focused_row->get());
            c = eTableColumn::cast(m_columns->first(m_focused_column));
            if (focused_row && c) {
                update_table_cell(ix_column_name(), ix_value(focused_row), c->name(), x);
            }
            break;

        case ECOMP_COMMAND:
            switch (x->geti()) {
                case ECOMPO_NEW_ROW:
                    edit_row_dialog(OETABLE_EDIT_ROW_NEW);
                    break;

                case ECOMPO_EDIT_ROW:
                    edit_row_dialog(OETABLE_EDIT_ROW_COPY);
                    break;

                case ECOMPO_DELETE_ROW:
                    delete_row_dialog();
                    break;

                case ECOMPO_INSERT_DLG_ROW:
                case ECOMPO_UPDATE_DLG_ROW:
                    save_dialog_row(x->geti());
                    gui()->delete_later(m_row_dialog->get());
                    break;

                default:
                    goto call_parent;
            }
            setpropertyi(ECOMP_COMMAND, ECOMPO_NO_COMMAND);
            break;

        default:
            return eComponent::onpropertychange(propertynr, x, flags);
    }

    return ESTATUS_SUCCESS;

call_parent:
    return eComponent::onpropertychange(propertynr, x, flags);
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
    eVariable *value, *col_conf;
    eContainer *rscols;
    os_int nrows, ncols, total_w, total_h;
    os_int row, column, ys;
    ImVec2 size, rmax, origin;
    ImVec2 cpos;
    ImGuiTableFlags flags;
    ImGuiListClipper clipper;
    os_boolean first_row;

    // const float TEXT_BASE_WIDTH = ImGui::CalcTextSize("A").x;
    const float TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();

    add_to_zorder(prm.window, prm.layer);

    if (m_rowset == OS_NULL || m_columns == OS_NULL) {
        if (m_rowset == OS_NULL) {
            select();
        }

        goto skipit;
    }

    nrows = m_rowset->nrows();
    ncols = m_rowset->ncolumns();
    if (ncols <= 0) {
        goto skipit;
    }

    // Using those as a base value to create width/height that are factor of the size of our font

    flags = ImGuiTableFlags_ScrollX |
        ImGuiTableFlags_ScrollY |
        ImGuiTableFlags_BordersInner |
        ImGuiTableFlags_NoPadOuterX |
        // ImGuiTableFlags_NoPadInnerX |
        ImGuiTableFlags_Resizable |
        ImGuiTableFlags_Reorderable |
        ImGuiTableFlags_Hideable;

    static int freeze_cols = 1;
    static int freeze_rows = 1;

    // When using ScrollX or ScrollY we need to specify a size for our table container!
    // Otherwise by default the table will fit all available space, like a BeginChild() call.
    size = ImVec2(0, TEXT_BASE_HEIGHT * 8);
    if (ImGui::BeginTable("##table2", ncols, flags /*, size */))
    {
        rmax = ImGui::GetContentRegionMax();
        origin = ImGui::GetCursorPos();
        ys = (os_int)ImGui::GetScrollY();

        total_w = (os_int)(rmax.x - origin.x);
        total_h = (os_int)(rmax.y - origin.y) + ys;

        cpos = ImGui::GetCursorScreenPos();
        m_rect.x1 = (os_int)cpos.x;
        m_rect.y1 = (os_int)cpos.y + ys;

        m_rect.x2 = m_rect.x1 + total_w - 1;
        m_rect.y2 = m_rect.y1 + total_h - 1;


        ImGui::TableSetupScrollFreeze(freeze_cols, freeze_rows);

        for (c = eTableColumn::cast(m_columns->first()), column=0;
             c && column < ncols;
             c = eTableColumn::cast(c->next()), column++)
        {
            c->prepare_column_header_for_drawing();
        }

        draw_header_row();

        m_data_windows_start_y = (os_int)ImGui::GetCursorScreenPos().y + ys;

        first_row = OS_TRUE;
        focused_m = eMatrix::cast(m_focused_row->get());
        value = new eVariable(this);
        clipper.Begin(nrows);
        m_logical_data_start_y = (os_int)clipper.StartPosY;
        m_data_row_h = (os_int)TEXT_BASE_HEIGHT;
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
                        m_data_windows_start_y = (os_int)ImGui::GetCursorScreenPos().y + ys;
                    }

                    if (!c->visible()) {
                        continue;
                    }

                    if (m == focused_m &&
                        column == m_focused_column)
                    {
                        if (!first_row) {
                            if (!ImGui::TableSetColumnIndex(column)) {
                                continue;
                            }
                        }

                        c->draw_edit(value, m, this);
                    }
                    else if (!value->isempty() || c->showas() == E_SHOWAS_CHECKBOX) {
                        if (!first_row) {
                            if (!ImGui::TableSetColumnIndex(column)) {
                                continue;
                            }
                        }
                        c->draw_value(value, this);
                    }
                }
                first_row = OS_FALSE;
            }
        }

        m_hovered_column = ImGui::TableGetHoveredColumn();
        if (m_hovered_column >= 0) {
            c = eTableColumn::cast(m_columns->first(m_hovered_column));
            rscols = m_rowset->columns();

            if (c && rscols) {
                col_conf = eVariable::cast(rscols->first(m_hovered_column));
                if (col_conf) {
                    value->clear();
                    row = (prm.mouse_pos.y - m_logical_data_start_y);
                    if (row >= 0) {
                        row /= m_data_row_h;
                        if (row < m_row_to_m_len) {
                            m = m_row_to_m[row].m_row;
                            if (m) m->getv(0, m_hovered_column, value);
                        }
                    }
                    c->draw_tooltip(value, col_conf,
                        row >= 0 ? EDRAW_TTIP_CELL_VALUE : EDRAW_TTIP_DEFAULT);
                }
            }
        }

        delete value;
        ImGui::EndTable();
    }

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

skipit:

    /* Let base class implementation handle the rest.
     */
    return eComponent::draw(prm);
}

/**
****************************************************************************************************

  @brief Component clicked.

  The eTableView::on_click() function is called when a component is clicked. If the component
  processess the mouse click, it returns OS_TRUE. This indicates that the click has been
  processed. If it doesn't process the click, it call's eComponent base classess'es on_click()
  function to try if base class wants to process the click.
  When the mouse click is not processed, it is passed to parent object in z order.

  @param   prm Drawing parameters, notice especially edit_mode.
  @param   mouse_button_nr Which mouse button, for example EIMGUI_LEFT_MOUSE_BUTTON.

  @return  OS_TRUE if mouse click was processed by this component, or OS_FALSE if not.

****************************************************************************************************
*/
os_boolean eTableView::on_click(
    eDrawParams& prm,
    os_int mouse_button_nr)
{
    eTableColumn *c;
    eMatrix *m;
    os_int row, column;

    if (!prm.edit_mode && mouse_button_nr == EIMGUI_LEFT_MOUSE_BUTTON) {
        column = m_hovered_column;
        if (column >= 0 && prm.mouse_pos.y >= m_data_windows_start_y) {
            c = eTableColumn::cast(m_columns->first(column));
            row = (prm.mouse_pos.y - m_logical_data_start_y);
            if (row >= 0) {
                row /= m_data_row_h;
                if (c && row < m_row_to_m_len) {
                    m = m_row_to_m[row].m_row;
                    c->activate(m, column, this);
                }
            }
        }

        return OS_TRUE;
    }

    /* Allow ImGui to handle right clicks to table header
     */
    if (mouse_button_nr == EIMGUI_RIGHT_MOUSE_BUTTON) {
       row = (prm.mouse_pos.y - m_logical_data_start_y);
       if (row <= 0) {
            return OS_TRUE;
       }
    }

    return eComponent::on_click(prm, mouse_button_nr);
}



// Instead of calling TableHeadersRow() we'll submit custom headers ourselves
void eTableView::draw_header_row()
{
    eTableColumn *c;
    os_int column, ncols;

    ncols = m_rowset->ncolumns();

    ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
    for (c = eTableColumn::cast(m_columns->first()), column=0;
         c && column < ncols;
         c = eTableColumn::cast(c->next()), column++)
    {
        c->draw_column_header(column, m_nro_header_row_lines);
    }
}


const os_char *eTableView::ix_column_name()
{
    if (m_rowset == OS_NULL) return OS_NULL;
    return m_rowset->ix_column_name();
}

os_long eTableView::ix_value(eMatrix *m)
{
    if (m_rowset == OS_NULL) return -1;
    return m->getl(0, m_rowset->ix_column_nr());
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

/** Update table cell value
    @param   ix_column_name Index column name, often "ix".
    @param   ix_value Index value
*/
void eTableView::update_table_cell(
    const os_char *ix_column_name,
    os_long ix_value,
    const os_char *column_name,
    eVariable *column_value)
{
    eVariable where_clause, *element;
    eContainer *row;
    if (m_rowset == OS_NULL) return;

    where_clause = "[";
    where_clause += ix_value;
    where_clause += "]";

    row = new eContainer(this);
    element = new eVariable(row);
    element->addname(column_name, ENAME_NO_MAP);
    element->setv(column_value);

    m_rowset->update(where_clause.gets(), row, ETABLE_ADOPT_ARGUMENT);
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
    eVariable *column, path;

    propertyv(ECOMP_PATH, &path);
    if (path.isempty()) {
        return;
    }

    if (m_rowset == OS_NULL) {
        m_rowset = new eRowSet(this);
    }

    m_rowset->set_dbm(path.gets());
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

    count_header_row_lines();

    /* THIS MODIFIES WINDOW TITLE, SHOW NOT ALWAYS HAPPEN.
     */
    /* eVariable tmp;
    eComponent *w;
    m_rowset->propertyv(ETABLEP_TEXT, &tmp);
    if (!tmp.isempty()) {
        w = window();
        if (w) w->setpropertyv(ECOMP_TEXT, &tmp);
    } */
}


void eTableView::count_header_row_lines()
{
    eTableColumn *c;
    os_int nro_lines;

    m_nro_header_row_lines = 1;
    for (c = eTableColumn::cast(m_columns->first());
         c;
         c = eTableColumn::cast(c->next()))
    {
        nro_lines = c->count_header_row_lines();
        if (nro_lines > m_nro_header_row_lines) {
            m_nro_header_row_lines = nro_lines;
        }
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
ePopup *eTableView::right_click_popup(
    eDrawParams& prm)
{
    ePopup *p;
    os_int row;

    if (prm.mouse_pos.y < m_data_windows_start_y) return OS_NULL;
    row = (prm.mouse_pos.y - m_logical_data_start_y);
    if (row < 0) return OS_NULL;
    row /= m_data_row_h;

    /* Close old dialog if any.
     */
    delete m_row_dialog->get();

    p = eComponent::right_click_popup(prm);

    add_popup_item_command("new row", ECOMPO_NEW_ROW, p);

    if (row < m_row_to_m_len) {
        add_popup_item_command("edit row", ECOMPO_EDIT_ROW, p);
        add_popup_item_command("delete row", ECOMPO_DELETE_ROW, p);

        /* Set ePointer to row matrix.
         */
        m_row_dialog_m->set(m_row_to_m[row].m_row);
    }
    else {
        m_row_dialog_m->set(OS_NULL);
    }

    return p;
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
    eSet *appendix,
    const os_char *target)
{
    eVariable value;

    eComponent::object_info(item, name, appendix, target);

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

