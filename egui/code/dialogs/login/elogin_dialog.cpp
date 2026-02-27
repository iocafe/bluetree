/**

  @file    elogin_dialog.cpp
  @brief   User login dialog.
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
  Constructor.
****************************************************************************************************
*/
eLoginDialog::eLoginDialog(
    eObject *parent,
    e_oid id,
    os_int flags)
    : eWindow(parent, id, flags)
{
    os_int row;
    elogin_load(&m_data);

    m_show_popup = -1;
    m_popup_row = 0;

    row = m_data.selected_row;
    if (row >= 0 && row < ELOGIN_MAX_ROWS) {
        if (m_data.rows[row].display_row &&
            m_data.rows[row].user_name[0] &&
            m_data.rows[row].password[0] == '\0')
        {
            m_show_popup = row;
        }
    }

    os_memclear(m_password_buf, sizeof(m_password_buf));
    initproperties();
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
    addpropertys(cls, ECOMP_NAME, ecomp_name, "login", "name", EPRO_PERSISTENT);
    addpropertys(cls, ECOMP_TEXT, ecomp_text, "user login", "title text", EPRO_PERSISTENT);
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
        case ECOMP_NAME:
            m_label_title.clear();
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
    os_int total_w, total_h, ys, nrows, row, i, j;
    os_int data_row[ELOGIN_MAX_ROWS];
    ImVec2 size, rmax, origin;
    ImVec2 cpos;
    ImGuiTableFlags flags;
    bool show_window = true, ok;
    os_char tmplabel[OSAL_NBUF_SZ+3];
    int select_state = 0;
    bool check, rval, change_bg_color, change_text_color;
    float text_height;

    const int header_row = 0;
    const int freeze_cols = 1;
    const int freeze_rows = header_row;
    const int ncols = 4;
    const float TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();

    ImGui::SetNextWindowSize(ImVec2(500, 150), ImGuiCond_FirstUseEver);

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
        elogin_defaults(&m_data);
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

        ImGui::TableSetupColumn("select", ImGuiTableColumnFlags_NoHide, 7.0f);
        ImGui::TableSetupColumn("user name", ImGuiTableColumnFlags_NoHide, 46.0f);
        ImGui::TableSetupColumn("password", ImGuiTableColumnFlags_NoHide, 40.0f);
        ImGui::TableSetupColumn("save password", ImGuiTableColumnFlags_NoHide, 7.0f);

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
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("select row");
                }
                if (rval) {
                    set_select(j, OS_TRUE, OS_FALSE);
                }

                ImGui::TableNextColumn();

                tmplabel[2] = 'U';
                ImGui::SetNextItemWidth(-FLT_MIN);
                change_bg_color = (j == m_data.selected_row && m_data.rows[j].password[0]);
                if (change_bg_color) {
                    ImVec4 color = style.Colors[ImGuiCol_CheckMark];
                    color.w /= 4;
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, color);
                }
                rval = ImGui::InputText(tmplabel, m_data.rows[j].user_name,
                    OSAL_LONG_USER_NAME_SZ, ImGuiInputTextFlags_EnterReturnsTrue);
                if (change_bg_color) {
                    ImGui::PopStyleColor();
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("user name");
                }
                if (rval) {
                    set_select(j, OS_TRUE, OS_TRUE);
                }

                ImGui::TableNextColumn();

                tmplabel[2] = 'P';
                ImGui::SetNextItemWidth(-FLT_MIN);

                change_text_color = (j == m_data.selected_row && !m_data.rows[j].password[0]);
                if (change_text_color) {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(ImColor(192, 0, 0)));
                    ImGui::PushStyleColor(ImGuiCol_TextDisabled, ImVec4(ImColor(192, 0, 0)));
                }
                rval = ImGui::InputTextWithHint(tmplabel, m_data.rows[j].password[0]
                    ? "<password ok>" : "<password not set>",
                    m_data.rows[j].password, OSAL_SECRET_STR_SZ, ImGuiInputTextFlags_Password|
                    ImGuiInputTextFlags_EnterReturnsTrue|ImGuiInputTextFlags_AutoSelectAll);
                if (change_text_color) {
                    ImGui::PopStyleColor(2);
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("password");
                }
                if (rval) {
                    set_select(j, OS_FALSE, OS_TRUE);
                }

                ImGui::TableNextColumn();

                tmplabel[2] = 'C';
                ImGui::SetNextItemWidth(-FLT_MIN);
                check = m_data.rows[j].save_password;
                ImGui::Checkbox(tmplabel, &check);
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("save password");
                }
                if ((os_boolean)check != m_data.rows[j].save_password) {
                    m_data.rows[j].save_password = check;
                    save_login();
                }
            }
        }

        ImGui::EndTable();

    }
    ImGui::PopStyleColor();

    /* Draw password popup window as needed.
     */
    draw_popup();

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

  @brief Draw password popup window as needed.

****************************************************************************************************
*/
void eLoginDialog::draw_popup()
{
    bool rval, rval2;
    os_char tmp[90];

    /* Handle password popup.
     */
    if (m_show_popup >= 0) {
        ImGui::OpenPopup("my_passwd_popup");
        m_popup_row = m_show_popup;
        m_show_popup  = -1;
        os_memclear(m_password_buf, sizeof(m_password_buf));
    }

    ImVec2 win_pos = ImGui::GetWindowPos();
    ImVec2 win_sz = ImGui::GetWindowSize();
    ImVec2 win_o = ImGui::GetWindowContentRegionMin();

    if (ImGui::BeginPopup("my_passwd_popup"))
    {
        if (!ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0)) {
            ImGui::SetKeyboardFocusHere(0);
        }

        ImVec2 pop_pos = ImGui::GetWindowPos();
        if (pop_pos.x < win_pos.x || pop_pos.x > win_pos.x + win_sz.x ||
            pop_pos.y < win_pos.y || pop_pos.y > win_pos.y + win_sz.y)
        {
            ImGui::SetWindowPos(ImVec2(win_pos.x + win_o.x, win_pos.y + win_o.y));
        }

        os_strncpy(tmp, "type password for ", sizeof(tmp));
        os_strncat(tmp, m_data.rows[m_popup_row].user_name, sizeof(tmp));
        ImGui::Text("%s", tmp);
        rval = ImGui::InputTextWithHint("##passw", "<password>", m_password_buf,
            OSAL_SECRET_STR_SZ, ImGuiInputTextFlags_Password|
        ImGuiInputTextFlags_EnterReturnsTrue|ImGuiInputTextFlags_AutoSelectAll);

        if (ImGui::Button("cancel")) {
            ImGui::CloseCurrentPopup();
        }
        else {
            ImGui::SameLine(0.0f, 10.0f);
            rval2 = ImGui::Button("ok");

            if (rval || rval2) {
                os_memcpy(m_data.rows[m_popup_row].password, m_password_buf, OSAL_SECRET_STR_SZ);
                ImGui::CloseCurrentPopup();
                save_login();
            }
        }
        ImGui::EndPopup();
    }
}


/**
****************************************************************************************************

  @brief Set selected row.

  The eLoginDialog::set_select is called when user changes selected row.

****************************************************************************************************
*/
void eLoginDialog::set_select(
    os_int select_row,
    os_boolean can_open_password_dialog,
    os_boolean save_unconditionally)
{
    os_int i;
    os_boolean modified = OS_FALSE;

    /* If password dialog is open, close it.
     */
    m_show_popup = -1;

    /* Clear passwords which are not marked "saved" on other rows, but selected one.
     */
    for (i = 0; i<ELOGIN_MAX_ROWS; i++) {
        if (i != select_row) {
            if (!m_data.rows[i].save_password) {
                if (m_data.rows[i].password[0]) {
                    os_memclear(m_data.rows[i].password, OSAL_SECRET_STR_SZ);
                    modified = OS_TRUE;
                }
            }
        }
    }

    /* If selected row has changed, and we may open password dialog.
     */
    if (m_data.selected_row != select_row && can_open_password_dialog && select_row >= 0)
    {
        if (!m_data.rows[select_row].password[0]) {
            m_show_popup = select_row;
        }
    }

    if (m_data.selected_row != select_row) {
        m_data.selected_row = select_row;
        modified = OS_TRUE;
    }

    /* Save if modified.
     */
    if (modified || save_unconditionally) {
        save_login();
    }
}


/**
****************************************************************************************************

  @brief Activate user login and save login data.

****************************************************************************************************
*/
void eLoginDialog::save_login()
{
    eLoginData data_no_passwd;
    os_int i;

    elogin_set_data(&m_data);

    /* Make temp copy, wipe passwords without "save password" checkmark before saving.
     */
    data_no_passwd = m_data;
    for (i = 0; i<ELOGIN_MAX_ROWS; i++) {
        if (!data_no_passwd.rows[i].save_password) {
            if (data_no_passwd.rows[i].password[0]) {
                os_memclear(data_no_passwd.rows[i].password, OSAL_SECRET_STR_SZ);
            }
        }
    }
    elogint_save(&data_no_passwd);
}
