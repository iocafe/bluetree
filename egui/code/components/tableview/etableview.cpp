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

  @brief Constructor.

  X...

  @return  None.

****************************************************************************************************
*/
eTableView::eTableView(
    eObject *parent,
    e_oid id,
    os_int flags)
    : eComponent(parent, id, flags)
{
    m_rowset = OS_NULL;

select();
}


/**
****************************************************************************************************

  @brief Virtual destructor.

  X...

  @return  None.

****************************************************************************************************
*/
eTableView::~eTableView()
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
        case ECOMP_VALUE: /* clear label to display new text and proceed */
            // m_label_value.clear();
            // m_set_checked = true;
            break;

        case ECOMP_TEXT:
            // m_text.clear();
            break;

        case ECOMP_UNIT:
            // m_unit.clear();
            // m_attr.clear();
            break;

        default:
            return eComponent::onpropertychange(propertynr, x, flags);
    }

    return ESTATUS_SUCCESS;
}


// Make the UI compact because there are so many fields
static void PushStyleCompact()
{
    ImGuiStyle& style = ImGui::GetStyle();
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(style.FramePadding.x, (float)(int)(style.FramePadding.y * 0.70f)));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(style.ItemSpacing.x, (float)(int)(style.ItemSpacing.y * 0.70f)));
}

static void PopStyleCompact()
{
    ImGui::PopStyleVar(2);
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
    os_int relative_x2, total_w, total_h;

    // const float TEXT_BASE_WIDTH = ImGui::CalcTextSize("A").x;
    const float TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();

    add_to_zorder(prm.window);
    // m_attr.for_variable(this);

    relative_x2 = ImGui::GetContentRegionMax().x;
    total_w = relative_x2 - ImGui::GetCursorPosX();
    total_h = 100;

    ImVec2 cpos = ImGui::GetCursorScreenPos();
    m_rect.x1 = cpos.x;
    m_rect.y1 = cpos.y;

    m_rect.x2 = m_rect.x1 + total_w - 1;
    m_rect.y2 = m_rect.y1 + total_h - 1;

    /* Draw marker for state bits if we have an extended value.
     */
    draw_state_bits(m_rect.x2);

    // Using those as a base value to create width/height that are factor of the size of our font


        static ImGuiTableFlags flags = ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;
        static int freeze_cols = 1;
        static int freeze_rows = 1;

        PushStyleCompact();
        ImGui::CheckboxFlags("ImGuiTableFlags_ScrollX", (unsigned int*)&flags, ImGuiTableFlags_ScrollX);
        ImGui::CheckboxFlags("ImGuiTableFlags_ScrollY", (unsigned int*)&flags, ImGuiTableFlags_ScrollY);
        ImGui::SetNextItemWidth(ImGui::GetFrameHeight());
        ImGui::DragInt("freeze_cols", &freeze_cols, 0.2f, 0, 9, NULL, ImGuiSliderFlags_NoInput);
        ImGui::SetNextItemWidth(ImGui::GetFrameHeight());
        ImGui::DragInt("freeze_rows", &freeze_rows, 0.2f, 0, 9, NULL, ImGuiSliderFlags_NoInput);
        PopStyleCompact();

        // When using ScrollX or ScrollY we need to specify a size for our table container!
        // Otherwise by default the table will fit all available space, like a BeginChild() call.
        ImVec2 size = ImVec2(0, TEXT_BASE_HEIGHT * 8);
        int add_columns = 405;
        char bbb[128];
        os_strncpy(bbb, "uke", sizeof(bbb));
        if (ImGui::BeginTable("##table2", 7 + add_columns, flags, size))
        {
            ImGui::TableSetupScrollFreeze(freeze_cols, freeze_rows);
            ImGui::TableSetupColumn("Line #", ImGuiTableColumnFlags_NoHide); // Make the first column not hideable to match our use of TableSetupScrollFreeze()
            ImGui::TableSetupColumn("One", ImGuiTableColumnFlags_None);
            ImGui::TableSetupColumn("Two", ImGuiTableColumnFlags_None);
            ImGui::TableSetupColumn("Three", ImGuiTableColumnFlags_None);
            ImGui::TableSetupColumn("Four", ImGuiTableColumnFlags_None);
            ImGui::TableSetupColumn("Five", ImGuiTableColumnFlags_None);
            ImGui::TableSetupColumn("Six", ImGuiTableColumnFlags_None);


            for (int i=0; i<add_columns; i++) {
                osal_int_to_str(bbb + 3, sizeof(bbb)-3, i+6);

                ImGui::TableSetupColumn(bbb, ImGuiTableColumnFlags_None);
            }

            ImGui::TableHeadersRow();

            int nrows = 20000;
            os_boolean first_row = OS_TRUE, visible[500];
            ImGuiListClipper clipper;
            clipper.Begin(nrows);
            while (clipper.Step())
            {
                for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)

            //     for (int row = 0; row < nrows; row++)
                {
                    ImGui::TableNextRow();
                    for (int column = 0; column < 7 + add_columns; column++)
                    {
                        // Both TableNextColumn() and TableSetColumnIndex() return false when a column is not visible, which can be used for clipping.
                        if (first_row) {
                            visible[column] = ImGui::TableSetColumnIndex(column);
                        }
                        else {
                            if (!visible[column]) continue;

                            if (!ImGui::TableSetColumnIndex(column)) {
                                continue;
                            }
                        }

                        if (column == 0)
                            ImGui::Text("Line %d", row);
                        else
                            ImGui::Text("Hello world %d,%d", row, column);
                    }
                    first_row = OS_FALSE;
                }
            }
            ImGui::EndTable();
        }

        /* Tool tip
         */
        if (ImGui::IsItemHovered()) {
            draw_tooltip();
        }


    /* Let base class implementation handle the rest.
     */
    return eComponent::draw(prm);
}


/**
****************************************************************************************************

  @brief Draw marker for state bits if we have extended value

****************************************************************************************************
*/
void eTableView::draw_state_bits(
    os_int x)
{
#if 0
    os_int state_bits;
    float circ_x, circ_y;
    const os_int rad = 8;

    if (!m_edit_value && m_label_value.isx())
    {
        ImVec4 colf;

        state_bits = m_label_value.sbits();
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
        circ_x = (float)(x + 3*rad/2);
        circ_y = m_rect.y1 + 0.5 * (m_rect.y2 - m_rect.y1 + 1);
        draw_list->AddCircleFilled(ImVec2(circ_x, circ_y), rad, col, 0);
    }
#endif
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


/* Callback when table data is received, etc.
 */
void eTableView::callback(
    eRowSet *rset,
    ersetCallbackInfo *ci)
{
    switch (ci->event) {
        case ERSET_TABLE_BINDING_COMPLETE:
            osal_console_write("binding done");
            break;

        case ERSET_INITIAL_DATA_RECEIVED:
        case ERSET_MODIFICATIONS_RECEIVED:
            rset->print_json(EOBJ_SERIALIZE_ONLY_CONTENT);
            break;
    }

    osal_console_write("eRowSet callback\n");
}

/* Static callback function just firwards to callback(). This exists to have C function pointer.
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
    os_int propertynr, i;

    static os_int copy_property_list[] = {EVARP_VALUE, EVARP_TYPE, EVARP_UNIT, EVARP_ATTR,
        EVARP_DEFAULT, EVARP_ABBR, EVARP_GROUP, EVARP_TTIP, EVARP_DIGS, EVARP_MIN, EVARP_MAX,
        EVARP_GAIN, EVARP_OFFSET, 0};

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

    i = 0;
    while ((propertynr = copy_property_list[i++])) {
        propertyv(propertynr, &value);
        item->setpropertyv(propertynr, &value);
    }
}

