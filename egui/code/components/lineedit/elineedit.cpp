/**

  @file    elineedit.cpp
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

  @brief Constructor.

  X...

  @return  None.

****************************************************************************************************
*/
eLineEdit::eLineEdit(
    eObject *parent,
    e_oid id,
    os_int flags)
    : eObject(parent, id, flags)
{
    m_edit_value = false;
    m_prev_edit_value = false;
}


/**
****************************************************************************************************

  @brief Virtual destructor.

  X...

  @return  None.

****************************************************************************************************
*/
eLineEdit::~eLineEdit()
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
eObject *eLineEdit::clone(
    eObject *parent,
    e_oid id,
    os_int aflags)
{
    eLineEdit *clonedobj;
    clonedobj = new eLineEdit(parent, id == EOID_CHILD ? oid() : id, flags());
    clonegeneric(clonedobj, aflags);
    return clonedobj;
}


/**
****************************************************************************************************

  @brief Add eLineEdit to class list and class'es properties to it's property set.

  The eLineEdit::setupclass function adds eLineEdit to class list and class'es properties to
  it's property set. The class list enables creating new objects dynamically by class identifier,
  which is used for serialization reader functions. The property set stores static list of
  class'es properties and metadata for those.

****************************************************************************************************
*/
void eLineEdit::setupclass()
{
    const os_int cls = EGUICLASSID_LINE_EDIT;

    os_lock();
    eclasslist_add(cls, (eNewObjFunc)newobj, "eLineEdit");
    eComponent::setupproperties(cls, ECOMP_NO_OPTIONAL_PROPERITES);
    propertysetdone(cls);
    os_unlock();
}


/**
****************************************************************************************************

  @brief Draw the component.

  The eLineEdit::draw() function calls ImGui API to render the component.

  @param   Prm Drawing parameters.
  @return  None.

****************************************************************************************************
*/
void eLineEdit::draw(
    eDrawParams& prm)
{
    int total_w;
    os_char *text_input_label, *text_display_label;
    static char buf[256];

    ImVec2 c = ImGui::GetContentRegionAvail();
    total_w = c.x;
    ImGui::Text("Duudeling");
    // int w = ImGui::CalcItemWidth();
    // ImGui::SameLine(total_w - w);
    // int w = total_w - 200;
    int w = 200;
    ImGui::SameLine(total_w - w);

    ImGui::SetNextItemWidth(w);

    if (m_edit_value) {
        text_input_label = m_text_input_label.get((eComponent*)this);

        ImGui::InputText(text_input_label, buf, sizeof(buf), ImGuiInputTextFlags_CharsDecimal|ImGuiInputTextFlags_EnterReturnsTrue);
        if ((!ImGui::IsItemActive() || ImGui::IsItemDeactivatedAfterEdit()) && m_prev_edit_value)
        {
            m_edit_value = false;
        }
        else
        {
            if (!m_prev_edit_value) {
                ImGui::SetKeyboardFocusHere(-1);
                m_prev_edit_value = true;
            }
        }
    }
    else {
        text_display_label = m_text_display_label.get((eComponent*)this);
        ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(1.0f, 0.5f));

        os_char label[256];
        os_strncpy(label, "Tesuuppa!", sizeof(label));
        os_strncat(label, text_display_label, sizeof(label));

        ImGui::Button(label, ImVec2(w, 0));
        if (ImGui::IsItemActive())
        {
            m_prev_edit_value = false;
            m_edit_value = true;
        }
        ImGui::PopStyleVar();
    }

#if 0
    ImGuiInputTextFlags_None                = 0,
    ImGuiInputTextFlags_CharsDecimal        = 1 << 0,   // Allow 0123456789.+-*/
    ImGuiInputTextFlags_CharsHexadecimal    = 1 << 1,   // Allow 0123456789ABCDEFabcdef
    ImGuiInputTextFlags_CharsUppercase      = 1 << 2,   // Turn a..z into A..Z
    ImGuiInputTextFlags_CharsNoBlank        = 1 << 3,   // Filter out spaces, tabs
    ImGuiInputTextFlags_AutoSelectAll       = 1 << 4,   // Select entire text when first taking mouse focus
    ImGuiInputTextFlags_EnterReturnsTrue    = 1 << 5,   // Return 'true' when Enter is pressed (as opposed to every time the value was modified). Consider looking at the IsItemDeactivatedAfterEdit() function.
    ImGuiInputTextFlags_CallbackCompletion  = 1 << 6,   // Callback on pressing TAB (for completion handling)
    ImGuiInputTextFlags_CallbackHistory     = 1 << 7,   // Callback on pressing Up/Down arrows (for history handling)
    ImGuiInputTextFlags_CallbackAlways      = 1 << 8,   // Callback on each iteration. User code may query cursor position, modify text buffer.
    ImGuiInputTextFlags_CallbackCharFilter  = 1 << 9,   // Callback on character inputs to replace or discard them. Modify 'EventChar' to replace or discard, or return 1 in callback to discard.
    ImGuiInputTextFlags_AllowTabInput       = 1 << 10,  // Pressing TAB input a '\t' character into the text field
    ImGuiInputTextFlags_CtrlEnterForNewLine = 1 << 11,  // In multi-line mode, unfocus with Enter, add new line with Ctrl+Enter (default is opposite: unfocus with Ctrl+Enter, add line with Enter).
    ImGuiInputTextFlags_NoHorizontalScroll  = 1 << 12,  // Disable following the cursor horizontally
    ImGuiInputTextFlags_AlwaysInsertMode    = 1 << 13,  // Insert mode
    ImGuiInputTextFlags_ReadOnly            = 1 << 14,  // Read-only mode
    ImGuiInputTextFlags_Password            = 1 << 15,  // Password mode, display all characters as '*'
    ImGuiInputTextFlags_NoUndoRedo          = 1 << 16,  // Disable undo/redo. Note that input text owns the text data while active, if you want to provide your own undo/redo stack you need e.g. to call ClearActiveID().
    ImGuiInputTextFlags_CharsScientific     = 1 << 17,  // Allow 0123456789.+-*/eE (Scientific notation input)
    ImGuiInputTextFlags_CallbackResize      = 1 << 18,  // Callback on buffer capacity changes request (beyond 'buf_size' parameter value), allowing the string to grow. Notify when the string wants to be resized (for string types which hold a cache of their Size). You will be provided a new BufSize in the callback and NEED to honor it. (see misc/cpp/imgui_stdlib.h for an example of using this)
    ImGuiInputTextFlags_CallbackEdit        = 1 << 19,  // Callback on any edit (note that InputText() already returns true on edit, the callback is useful mainly to manipulate the underlying buffer while focus is active)
#endif
}

