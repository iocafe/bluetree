/**

  @file    elogin_dialog.cpp
  @brief   User login dialog.
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
eLoginDialog::eLoginDialog(
    eObject *parent,
    e_oid id,
    os_int flags)
    : eWindow(parent, id, flags)
{
    setup_default_data();
}


/**
****************************************************************************************************
  Virtual destructor.
****************************************************************************************************
*/
eLoginDialog::~eLoginDialog()
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
eObject *eLoginDialog::clone(
    eObject *parent,
    e_oid id,
    os_int aflags)
{
    eLoginDialog *clonedobj;
    clonedobj = new eLoginDialog(parent, id == EOID_CHILD ? oid() : id, flags());
    clonegeneric(clonedobj, aflags);
    return clonedobj;
}


/**
****************************************************************************************************

  @brief Add eLoginDialog to class list and class'es properties to it's property set.

  The eLoginDialog::setupclass function adds eLoginDialog to class list and class'es properties to
  it's property set. The class list enables creating new objects dynamically by class identifier,
  which is used for serialization reader functions. The property set stores static list of
  class'es properties and metadata for those.

****************************************************************************************************
*/
void eLoginDialog::setupclass()
{
    const os_int cls = EGUICLASSID_LOGIN_DIALOG;

    os_lock();
    eclasslist_add(cls, (eNewObjFunc)newobj, "eLoginDialog", EGUICLASSID_WINDOW);
    setupproperties(cls, ECOMP_NO_OPTIONAL_PROPERITES);
    addpropertys(cls, ECOMP_TEXT, ecomp_text, "user login", "window title", EPRO_METADATA);
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
eStatus eLoginDialog::onpropertychange(
    os_int propertynr,
    eVariable *x,
    os_int flags)
{
    switch (propertynr)
    {
        case ECOMP_TEXT:
            m_text.clear();
            break;

        default:
            return eWindow::onpropertychange(propertynr, x, flags);
    }

    return ESTATUS_SUCCESS;
}


/**
****************************************************************************************************

  @brief Draw the component.

  The eLoginDialog::draw() function calls ImGui API to render the component.

  @param   Prm Drawing parameters.
  @return  The function return ESTATUS_SUCCESS if all is fine. Other values indicate that the
           component is no longer drawable or useful. This could be for example a pop up menu
           closed implicitely by clicking elsewhere.

****************************************************************************************************
*/
eStatus eLoginDialog::draw(
    eDrawParams& prm)
{
    const os_char *label;
    ImVec2 sz;
    os_int total_w, total_h, ys, nrows, row, i, j, text_height;
    os_int data_row[ELOGIN_MAX_ROWS];
    ImVec2 size, rmax, origin;
    ImVec2 cpos;
    ImGuiTableFlags flags;
    bool show_window = true, ok;
    os_char tmplabel[OSAL_NBUF_SZ+3];
    int select_state = 0;
    bool check, rval;

    const int header_row = 0;
    const int freeze_cols = 1;
    const int freeze_rows = header_row;
    const int ncols = 4;
    const int TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();

    label = m_label_title.get(this, ECOMP_TEXT, ECOMP_NAME);
    ok = ImGui::Begin(label, &show_window);

    /* Early out if the window is collapsed, as an optimization.
     */
    if (!ok) {
        ImGui::End();
        return ESTATUS_SUCCESS;
    }

    add_to_zorder(prm.window, prm.layer);

    flags =
        ImGuiTableFlags_ScrollY |
        ImGuiTableFlags_BordersInner |
        ImGuiTableFlags_NoPadOuterX |
        ImGuiTableFlags_Resizable |
        ImGuiTableFlags_Reorderable |
        ImGuiTableFlags_SizingStretchProp |
        ImGuiTableFlags_NoSavedSettings;

    /* Count the rows to display. If zero restore initial data.
     */
    nrows = 0;
    while (OS_TRUE) {
        for (i = 0 ; i < ELOGIN_MAX_ROWS; i++) {
            if (m_data.rows[i].display_row) {
                data_row[nrows++] = i;
            }
        }
        if (nrows) break;
        setup_default_data();
    }

    os_strncpy(tmplabel, "##?", sizeof(tmplabel));
    select_state = m_data.selected_row;

    ImGuiStyle &style = ImGui::GetStyle();
    ImGui::PushStyleColor(ImGuiCol_FrameBg, style.Colors[ImGuiCol_WindowBg]);
    text_height = TEXT_BASE_HEIGHT + 2.0f * style.FramePadding.y;
    size = ImVec2(0, text_height * (nrows + header_row));

    if (ImGui::BeginTable("##tableU", ncols, flags, size))
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


        ImGui::TableSetupScrollFreeze(freeze_cols, freeze_rows);

        ImGui::TableSetupColumn("select", ImGuiTableColumnFlags_NoHide, 20.0f);
        ImGui::TableSetupColumn("user name", ImGuiTableColumnFlags_NoHide, 100.0f);
        ImGui::TableSetupColumn("password", ImGuiTableColumnFlags_NoHide, 80.0f);
        ImGui::TableSetupColumn("save password", ImGuiTableColumnFlags_NoHide, 30.0f);

        if (header_row) {
            ImGui::TableHeadersRow();
        }

        ImGuiListClipper clipper;
        clipper.Begin(nrows);
        while (clipper.Step())
        {
            for (row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
            {
                if (row < 0 || row > nrows) {
                    break;
                }
                j = data_row[row];

                osal_int_to_str(tmplabel+3, OSAL_NBUF_SZ, row);

                ImGui::TableNextRow();
                if (!ImGui::TableSetColumnIndex(0)) {
                    continue;
                }

                tmplabel[2] = 'R';
                ImGui::SetNextItemWidth(-FLT_MIN);
                rval = ImGui::RadioButton(tmplabel, &select_state, j);
                if (rval) {
                    set_select(j, OS_TRUE);
                }

                ImGui::TableNextColumn();

                tmplabel[2] = 'U';
                ImGui::SetNextItemWidth(-FLT_MIN);
                if (j == m_data.selected_row) {
                    ImVec4 color = m_data.rows[j].password_set
                        ? style.Colors[ImGuiCol_CheckMark] : ImVec4(ImColor(255, 64, 64));
                    color.w /= 4;

                    ImGui::PushStyleColor(ImGuiCol_FrameBg, color);
                }
                rval = ImGui::InputText(tmplabel, m_data.rows[j].user_name, OSAL_LONG_USER_NAME_SZ);
                if (j == m_data.selected_row) {
                    ImGui::PopStyleColor();
                }
                if (rval) {
                    set_select(j, OS_TRUE);
                }

                ImGui::TableNextColumn();

                tmplabel[2] = 'P';
                ImGui::SetNextItemWidth(-FLT_MIN);

                rval = ImGui::InputTextWithHint(tmplabel, m_data.rows[j].password_set ? "<password ok>" : "<password not set>",
                    m_data.rows[j].password, OSAL_SECRET_STR_SZ, ImGuiInputTextFlags_Password);
                if (rval) {
                    set_select(j, OS_FALSE);
                }

                ImGui::TableNextColumn();

                tmplabel[2] = 'C';
                ImGui::SetNextItemWidth(-FLT_MIN);
                check = m_data.rows[j].save_password;
                ImGui::Checkbox(tmplabel, &check);
                m_data.rows[j].save_password = check;
            }
        }

        ImGui::EndTable();
    }
    ImGui::PopStyleColor();

    /* Finished with the window.
     */
    ImGui::End();

    if (!show_window) {
        gui()->delete_later(this);
    }
    return ESTATUS_SUCCESS;
}


/**
****************************************************************************************************

  @brief Set selected row.

  The eLoginDialog::set_select is called when user changes selected row.

****************************************************************************************************
*/
void eLoginDialog::set_select(
    os_int select_row,
    os_boolean can_open_password_dialog)
{
    os_int i;

    /* If password dialog is open, close it
     */

    /* Clear passwords which are not marked "saved" on other rows, but selected one.
     */
    for (i = 0; i<ELOGIN_MAX_ROWS; i++) {
        if (i != select_row) {
            if (!m_data.rows[i].save_password) {
                os_memclear(m_data.rows[i].password, OSAL_SECRET_STR_SZ);
                m_data.rows[i].password_set = OS_FALSE;
            }
        }
    }

    /* If selected row has changed, and we may open password dialog
     */
    if (m_data.selected_row != select_row && can_open_password_dialog)
    {
    }

    m_data.selected_row = select_row;
}


/**
****************************************************************************************************

  @brief Initial configuration.

  The eLoginDialog::setup_default_data() sets clear m_data structure containing initial user
  names to propose, etc. This is used when UI is started for the first time, and restored
  if UI configuration decryption/checksum fails.

****************************************************************************************************
*/
void eLoginDialog::setup_default_data()
{
    os_memclear(&m_data, sizeof(m_data));
    m_data.selected_row = 0;
    os_strncpy(m_data.rows[0].user_name, "quest", OSAL_LONG_USER_NAME_SZ);
    os_strncpy(m_data.rows[0].password, "pass", OSAL_SECRET_STR_SZ);
    m_data.rows[0].display_row = OS_TRUE;
    m_data.rows[0].save_password = OS_TRUE;
    m_data.rows[0].password_set = OS_TRUE;
    os_strncpy(m_data.rows[1].user_name, "user", OSAL_LONG_USER_NAME_SZ);
    m_data.rows[1].display_row = OS_TRUE;
    m_data.rows[1].save_password = OS_TRUE;
    os_strncpy(m_data.rows[2].user_name, "ispy", OSAL_LONG_USER_NAME_SZ);
    m_data.rows[2].display_row = OS_TRUE;
}

