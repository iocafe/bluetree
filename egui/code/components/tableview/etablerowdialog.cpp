/**

  @file    etablerowdialog.cpp
  @brief   Dialogs related to adding new, editing or deleting rows.
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

  @brief Create dialog window to edit a row or add a new row.

  The eTableView::edit_row_dialog function creates a dialog for adding a new row to a table
  or modifying an existing row. In desctop environment with keyboard, etc, tables rows are
  edited "in place" within table display, and this function is necessary only for adding
  a new row. On PDA, cellphone, or generically with touchscreen, this dialog is used also
  to edit row.

  @param   create_new_row OS_TRUE to crate new row.

****************************************************************************************************
*/
void eTableView::edit_row_dialog(
    os_boolean create_new_row)
{
    eWindow *w;
    eParameterList *p;
    eLineEdit *e;
    eContainer *rscols;
    eVariable *v;
    eName *n;
    eMatrix *m;
    eVariable value;
    os_int col_nr, i, propertynr;

    static os_int copy_property_list[] = {EVARP_TYPE, EVARP_UNIT, EVARP_ATTR,
        EVARP_DEFAULT, EVARP_ABBR, EVARP_GROUP, EVARP_TTIP, EVARP_DIGS, EVARP_MIN, EVARP_MAX,
        EVARP_GAIN, EVARP_OFFSET, 0};

    if (m_rowset == OS_NULL) return;
    rscols = m_rowset->columns();
    if (rscols == OS_NULL) return;

    m = eMatrix::cast(m_row_dialog_m->get());
    if (m == OS_NULL) return;

    delete m_row_dialog->get();

    w = new eWindow(gui());
    w->setpropertys(ECOMP_VALUE, !create_new_row ? "edit row" : "new row");

    p = new eParameterList(w);

    for (v = rscols->firstv(), col_nr = 0; v; v = v->nextv(), col_nr++)
    {
        n = v->primaryname();
        if (n == OS_NULL) continue;

        e = new eLineEdit(p);
        v->propertyv(EVARP_TEXT, &value);
        value.singleline();
        e->setpropertyv(EVARP_TEXT, value.isempty() ? n : &value);

        i = 0;
        while ((propertynr = copy_property_list[i++])) {
            v->propertyv(propertynr, &value);
            e->setpropertyv(propertynr, &value);
        }

        m->getv(0, v->oid(), &value);
        e->setpropertyv(ECOMP_VALUE, &value);
    }

    m_row_dialog->set(w);
}


/**
****************************************************************************************************

  @brief Create dialog window confirm delete row or rows.

  The eTableView::delete_row_dialog functio...
  For now, no window is created, row is deleted without warning!

****************************************************************************************************
*/
void eTableView::delete_row_dialog()
{
    eMatrix *m;
    os_int ix_col;
    eVariable where_clause;

    delete m_row_dialog->get();
    if (m_rowset == OS_NULL) return;
    ix_col = m_rowset->ix_column_nr();
    m = eMatrix::cast(m_row_dialog_m->get());
    if (m == OS_NULL) return;

    where_clause = "[";
    where_clause += m->getl(0, ix_col);
    where_clause += "]";

    m_rowset->remove(where_clause.gets());
}

