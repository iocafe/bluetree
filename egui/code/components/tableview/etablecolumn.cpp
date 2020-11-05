/**

  @file    etablecolumn.cpp
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
eTableColumn::eTableColumn(
    eObject *parent,
    e_oid id,
    os_int flags)
    : eObject(parent, id, flags)
{
}


/**
****************************************************************************************************

  @brief Virtual destructor.

  X...

  @return  None.

****************************************************************************************************
*/
eTableColumn::~eTableColumn()
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
eObject *eTableColumn::clone(
    eObject *parent,
    e_oid id,
    os_int aflags)
{
    eTableColumn *clonedobj;
    clonedobj = new eTableColumn(parent, id == EOID_CHILD ? oid() : id, flags());
    clonegeneric(clonedobj, aflags);
    return clonedobj;
}


/**
****************************************************************************************************

  @brief Add eTableColumn to class list and class'es properties to it's property set.

  The eTableColumn::setupclass function adds eTableColumn to class list and class'es properties to
  it's property set. The class list enables creating new objects dynamically by class identifier,
  which is used for serialization reader functions. The property set stores static list of
  class'es properties and metadata for those.

****************************************************************************************************
*/
void eTableColumn::setupclass()
{
    const os_int cls = EGUICLASSID_TABLE_COLUMN;

    os_lock();
    eclasslist_add(cls, (eNewObjFunc)newobj, "eTableColumn");
    os_unlock();
}

void eTableColumn::setup_column(
    eVariable *col_conf)
{
    eName *n;

    m_text.get(col_conf, EVARP_TEXT);
    if (m_text.isempty()) {
        n = col_conf->primaryname();
        if (n) {
            m_text.setv(n);
        }
    }

    m_unit.get(col_conf, EVARP_UNIT);
    m_attr.for_variable(col_conf);
}


void eTableColumn::draw_column_header()
{
    const os_char *text;
    os_int col_nr;

    col_nr = oid();

    text = m_text.ptr();
    if (text == OS_NULL) {
        text = "?";
    }
    ImGui::TableSetupColumn(text, col_nr == 0 ? ImGuiTableColumnFlags_NoHide
        : ImGuiTableColumnFlags_None);
}

// Modifies value
void eTableColumn::draw_value(
    eVariable *value,
    eTableView *view)
{
    const os_char *text;

    enice_value_for_ui(value, this, &m_attr);
    text = value->gets();

// Right align test
ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetColumnWidth() - ImGui::CalcTextSize(text).x
     /* - ImGui::GetStyle().CellPadding.x */);

    ImGui::TextUnformatted(text);

    // ImGui::InputText(label, view->m_edit_buf.ptr(), m_edit_buf.sz(), eflags);
}



