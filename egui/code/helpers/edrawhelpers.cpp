/**

  @file    edrawhelpers.h
  @brief   Helper functions for rendering components.
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
#include "imgui_internal.h"

/* Forward referred static functions.
 */
static ImU32 edraw_get_state_color(
    os_int state_bits);


/* value_w Set -1 if drawing in table.
 * Modifies value argument
 */

void edraw_value(
    eVariable *value,
    os_int state_bits,
    eComponent *compo, // for redirs, tool tip, etc
    eAttrBuffer& attr,
    os_int value_w,
    eRect *r,
    os_int dflags)
{
    const os_char *text;
    ImVec2 pos, pos_max;
    ImU32 check_col;
    ImDrawList *draw_list;
    int extra_w, x_pos;
    const os_int pad = 2;
    os_int square_sz;
    bool checked;

    if (value_w < 0) {
        value_w = ImGui::GetColumnWidth();
    }

    if (r) {
        pos = ImGui::GetCursorScreenPos();
        r->x1 = pos.x;
        r->y1 = pos.y;
        r->x2 = pos.x + value_w;
        r->y2 = pos.y + ImGui::GetFrameHeight();
    }

    switch (attr.showas())
    {
        case E_SHOWAS_CHECKBOX:
            checked = (bool)value->getl();

            square_sz = ImGui::GetFrameHeight();
            square_sz -= 3 * pad;

            pos = ImGui::GetCursorScreenPos();
            pos.x += pad;

            if (attr.alignment() != E_ALIGN_LEFT) {
                extra_w = value_w - (square_sz + 2 * pad);
                if (extra_w > 0) {
                    if (attr.alignment() == E_ALIGN_CENTER) extra_w /= 2;
                    pos.x += extra_w;
                }
            }

            pos_max = pos;
            pos_max.x += square_sz;
            pos_max.y += square_sz;

            if (state_bits == OSAL_STATE_CONNECTED) {
                if (checked) {
                    check_col = ImGui::GetColorU32(attr.rdonly() ? ImGuiCol_Border : ImGuiCol_CheckMark);
                }
                else {
                    check_col = ImGui::GetColorU32(ImGuiCol_Button);
                }
            }
            else {
                check_col = edraw_get_state_color(state_bits);
            }

            draw_list = ImGui::GetWindowDrawList();
            // draw_list->AddRect(pos, pos_max, color, 0);
            if (checked) {
                pos.x++;
                ImGui::RenderCheckMark(draw_list, pos, check_col, square_sz - pad);
            }
            else if ((dflags & EDRAW_VALUE_TABLE) == 0 || state_bits != OSAL_STATE_CONNECTED)
            {
                pos.x += square_sz/2;
                pos.y += square_sz/2;
                ImGui::RenderBullet(draw_list, pos, check_col);
            }
            break;

        default:
            enice_value_for_ui(value, compo, &attr);
            text = value->gets();

            /* Align left, center, or right.
             */
            if (attr.alignment() != E_ALIGN_LEFT) {
                extra_w = value_w - ImGui::CalcTextSize(text).x;
                if (extra_w > 0) {
                    x_pos = ImGui::GetCursorPosX();
                    x_pos += (attr.alignment() == E_ALIGN_RIGHT) ? extra_w : extra_w/2;
                    ImGui::SetCursorPosX(x_pos);
                }
            }

            if (state_bits == OSAL_STATE_CONNECTED) {
                ImGui::TextUnformatted(text);
            }
            else {
                ImGui::PushStyleColor(ImGuiCol_Text, edraw_get_state_color(state_bits));
                // fcolor = ImGui::ColorConvertU32ToFloat4(edraw_get_state_color(state_bits));
                // ImGui::PushStyleColor(ImGuiCol_Text, fcolor);
                ImGui::TextUnformatted(text);
                ImGui::PopStyleColor();
            }

            break;
    }


    /* Tool tip
     */
    if (value_w >= 0) if (ImGui::IsItemHovered()) {
        compo->draw_tooltip();
    }
}


/**
****************************************************************************************************

  @brief Convert state bits to Dear ImGUi color.

  The state bits are used to indicate if an IO signal, etc is connected.

  @param  state_bits OSAL_STATE_CONNECTED, OSAL_STATE_YELLOW, OSAL_STATE_ORANGE, OSAL_STATE_RED
  @return Imgui color.

****************************************************************************************************
*/
static ImU32 edraw_get_state_color(
    os_int state_bits)
{
    switch (state_bits & OSAL_STATE_ERROR_MASK)
    {
        case OSAL_STATE_YELLOW:
            if (state_bits & OSAL_STATE_CONNECTED) {
                return ImGui::ColorConvertFloat4ToU32(ImVec4(0.6f, 0.6f, 0.1f, 1.0f /* alpha */));
            }
            break;

        case OSAL_STATE_ORANGE:
            if (state_bits & OSAL_STATE_CONNECTED) {
                return ImGui::ColorConvertFloat4ToU32(ImVec4(7.0f, 0.5f, 0.0f, 1.0f));
            }
            break;

        case OSAL_STATE_RED:
            return ImGui::ColorConvertFloat4ToU32(ImVec4(0.8f, 0.0f, 0.0f, 1.0f));

        default:
            if (state_bits & OSAL_STATE_CONNECTED) {
                return ImGui::GetColorU32(ImGuiCol_Text);
            }
            break;
    }

    return ImGui::GetColorU32(ImGuiCol_TextDisabled);
}


/**
****************************************************************************************************

  @brief Draw tool tip, called when mouse is hovering over the value

****************************************************************************************************
*/
void edraw_tooltip(
    eObject *obj,
    eVariable *value,
    const os_char *otext,
    const os_char *flagstr,
    eAttrBuffer& oattr,
    os_int flags)
{
    eVariable text, item;
    eValueX *ex;
    os_long utc;
    os_int state_bits;
    os_boolean worth_showing = OS_FALSE;
    const os_char *str;

#define E_DEBUG_TOOLTIPS 0

    if ((flags & EDRAW_TTIP_CELL_VALUE) == 0)
    {
#if E_DEBUG_TOOLTIPS
        text.sets(otext);
        text.singleline();
#endif
        obj->propertyv(ECOMP_TTIP, &item);
        if (!item.isempty()) {
            if (!text.isempty()) text += "\n";
            text += item;
            worth_showing = OS_TRUE;
        }
    }

    if (value) if (!value->isempty()) {
        ex = value->getx();
        if (ex) {
            state_bits = ex->sbits();

            utc = ex->tstamp();
            if (etime_timestamp_str(utc, &item) == ESTATUS_SUCCESS)
            {
                if (!text.isempty()) text += "\n";
                text += "time stamp: ";
                text += item;
                worth_showing = OS_TRUE;
            }

            if ((state_bits & OSAL_STATE_CONNECTED) == 0) {
                if (!text.isempty()) text += "\n";
                text += "signal is not connected";
                worth_showing = OS_TRUE;
            }
            if (state_bits & OSAL_STATE_ERROR_MASK) {
                if (state_bits & OSAL_STATE_CONNECTED) {
                    if (!text.isempty()) text += "\n";
                    text += "signal state: ";
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
    }


    if (flags & EDRAW_TTIP_PATH)
    {
        obj->propertyv(ECOMP_PATH, &item);
        str = item.gets();

#if ETREENODE_TOOLTIPS_FOR_DEBUG
        if (str) if (*str != '\0')
#else
        if (str) if (*str != '\0' && os_strchr((os_char*)str, '@') == OS_NULL)
#endif
        {
            eliststr_appeneds(&text, "path: ");
            text += str;
            worth_showing = OS_TRUE;
        }
    }

    if (flags & EDRAW_TTIP_IPATH)
    {
        obj->propertyv(ECOMP_IPATH, &item);
        str = item.gets();
#if ETREENODE_TOOLTIPS_FOR_DEBUG
        if (str) if (*str != '\0')
#else
        if (str) if (*str != '\0' && os_strchr((os_char*)str, '@') == OS_NULL)
#endif
        {
            eliststr_appeneds(&text, "ipath: ");
            text += str;
            worth_showing = OS_TRUE;
        }
    }

#if ETREENODE_TOOLTIPS_FOR_DEBUG
    if (flagstr) if (*flagstr) {
        eliststr_appeneds(&text, "o-flags: ");
        text += flagstr;
        worth_showing = OS_TRUE;
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
