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


// Modifies value argument
void edraw_value(
    eVariable *value,
    eComponent *compo, // for redirs, tool tip, etc
    eAttrBuffer& attr,
    eRect *r)
{
    const os_char *text;
    ImVec2 pos, pos_max;
    ImU32 box_col, check_col;
    ImDrawList *draw_list;
    int extra_w, x_pos;
    bool checked;

    if (r) {
        pos = ImGui::GetCursorScreenPos();
        r->x1 = pos.x;
        r->y1 = pos.y;
        r->x2 = pos.x + ImGui::GetColumnWidth();
        r->y2 = pos.y + ImGui::GetFrameHeight();
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
                    extra_w = ImGui::GetColumnWidth() - (square_sz + 2 * pad);
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
                extra_w = ImGui::GetColumnWidth() - ImGui::CalcTextSize(text).x;
                if (extra_w > 0) {
                    x_pos = ImGui::GetCursorPosX();
                    x_pos += (attr.alignment() == E_ALIGN_RIGHT) ? extra_w : extra_w/2;
                    ImGui::SetCursorPosX(x_pos);
                }
            }

            ImGui::TextUnformatted(text);
            break;
    }

   /* Tool tip
     */
    if (ImGui::IsItemHovered()) {
        compo->draw_tooltip();
    }
}
