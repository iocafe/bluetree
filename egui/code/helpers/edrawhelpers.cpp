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
static void edraw_state_bits(
    os_int state_bits,
    eRect *r);


/* value_w Set -1 if drawing in table.
 * Modifies value argument
 */

void edraw_value(
    eVariable *value,
    os_int state_bits,
    eComponent *compo, // for redirs, tool tip, etc
    eAttrBuffer& attr,
    os_int value_w,
    eRect *r)
{
    const os_char *text;
    ImVec2 pos, pos_max;
    ImU32 box_col, check_col;
    ImDrawList *draw_list;
    eRect tmpr;
    int extra_w, x_pos;
    bool checked;

    if (value_w < 0) {
        value_w = ImGui::GetColumnWidth();
    }

    if (r || state_bits != OSAL_STATE_CONNECTED) {
        if (r == OS_NULL) {
            r = &tmpr;
        }
        pos = ImGui::GetCursorScreenPos();
        r->x1 = pos.x;
        r->y1 = pos.y;
        r->x2 = pos.x + value_w;
        r->y2 = pos.y + ImGui::GetFrameHeight();

        if (state_bits != OSAL_STATE_CONNECTED) {
            edraw_state_bits(state_bits, r);
        }
    }

    switch (attr.showas())
    {
        case E_SHOWAS_CHECKBOX:
            {
                checked = (bool)value->getl();

                const os_int pad = 2;
                os_int square_sz = ImGui::GetFrameHeight();
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

                box_col = ImGui::GetColorU32(ImGuiCol_Border);
                draw_list = ImGui::GetWindowDrawList();
                draw_list->AddRect(pos, pos_max, box_col, 0);
                if (checked) {
                    check_col = ImGui::GetColorU32(ImGuiCol_CheckMark);
                    pos.x++;
                    ImGui::RenderCheckMark(draw_list, pos, check_col, square_sz - pad);
                }
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

            //ImGui::TextUnformatted(text);
            ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%s", text);

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

  @brief Draw marker for state bits if we have extended value

****************************************************************************************************
*/
static void edraw_state_bits(
    os_int state_bits,
    eRect *r)
{
    float circ_x, circ_y;
    const os_int rad = 8;
    ImVec4 colf;

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
    circ_x = (float)(r->x1 + 3*rad/2);
    circ_y = r->y1 + 0.5F * (r->y2 - r->y1);
    draw_list->AddCircleFilled(ImVec2(circ_x, circ_y), rad, col, 0);
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
    eAttrBuffer& oattr)
{
    eVariable text, item;
    eValueX *ex;
    os_long utc;
    os_int state_bits;
    os_boolean worth_showing = OS_FALSE;

#define E_DEBUG_TOOLTIPS 1

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

    if (value) if (!value->isempty()) {
        ex = value->getx();
        if (ex) {
            state_bits = ex->sbits();

            utc = ex->tstamp();
            if (etime_timestamp_str(utc, &item) == ESTATUS_SUCCESS)
            {
                if (!text.isempty()) text += "\n";
                text += "timestamp ";
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
    }

    if (worth_showing) {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(text.gets());
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}
