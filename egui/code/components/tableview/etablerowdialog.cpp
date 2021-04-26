/**

  @file    etablerowdialog.cpp
  @brief   Dialogs related to adding new, editing or deleting rows.
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
    os_int flags)
{
    eWindow *w;
    eParameterList *p;
    eLineEdit *e;
    eContainer *rscols;
    eVariable *v;
    eName *n;
    eTableColumn *c;
    eMatrix *m;
    eVariable value;
    eButton *button;
    os_int col_nr, i, propertynr;
    os_char buf[E_OIXSTR_BUF_SZ + 32];

    static os_int copy_property_list[] = {EVARP_TYPE, EVARP_UNIT, EVARP_ATTR,
        EVARP_DEFAULT, EVARP_ABBR, EVARP_GROUP, EVARP_TTIP, EVARP_DIGS, EVARP_MIN, EVARP_MAX,
        EVARP_GAIN, EVARP_OFFSET, 0};

    if (m_rowset == OS_NULL) return;
    rscols = m_rowset->columns();
    if (rscols == OS_NULL) return;

    delete m_row_dialog->get();

    m = eMatrix::cast(m_row_dialog_m->get());
    if ((m == OS_NULL) && (flags & OETABLE_EDIT_ROW_COPY)) {
        return;
    }

    w = new eWindow(gui());
    w->setpropertys(ECOMP_TEXT, (flags & OETABLE_EDIT_ROW_NEW) ? "new row" : "edit row");

    p = new eParameterList(w);
    for (v = rscols->firstv(), col_nr = 0; v; v = v->nextv(), col_nr++)
    {
        n = v->primaryname();
        if (n == OS_NULL) continue;

        if (m_columns) {
            c = eTableColumn::cast(m_columns->first(col_nr));
            if (c) {
                if (c->rdonly() || c->nosave()) {
                    continue;
                }
            }
        }

        e = new eLineEdit(p);
        e->addname(n->gets(), ENAME_NO_MAP);
        v->propertyv(EVARP_TEXT, &value);
        value.singleline();
        e->setpropertyv(EVARP_TEXT, value.isempty() ? n : &value);

        i = 0;
        while ((propertynr = copy_property_list[i++])) {
            v->propertyv(propertynr, &value);
            e->setpropertyv(propertynr, &value);
        }

        if (flags & OETABLE_EDIT_ROW_COPY) {
            m->getv(0, v->oid(), &value);
        }
        else {
            v->propertyv(ECOMP_DEFAULT, &value);
        }
        e->setpropertyv(ECOMP_VALUE, &value);
    }

    oixstr(buf, sizeof(buf));
    os_strncat(buf, "/_p/_command", sizeof(buf));

    button = new eButton(w);
    button->setpropertyl(ECOMP_VALUE, ECOMPO_NO_COMMAND);
    button->setpropertyl(ECOMP_SETVALUE,
        (flags & OETABLE_EDIT_ROW_NEW) ? ECOMPO_INSERT_DLG_ROW : ECOMPO_UPDATE_DLG_ROW);
    button->setpropertys(ECOMP_TARGET, buf);
    button->setpropertys(ECOMP_TEXT, "ok");

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


/**
****************************************************************************************************

  @brief Create dialog window confirm delete row or rows.

  The eTableView::delete_row_dialog functio...
  For now, no window is created, row is deleted without warning!

****************************************************************************************************
*/
void eTableView::save_dialog_row(
    os_int command)
{
    eComponent *dlg, *plist, *c;
    eMatrix *m;
    eContainer *row;
    eVariable *v;
    eName *n;
    os_int ix_col;
    eVariable where_clause;

    if (m_rowset == OS_NULL) return;
    dlg = eComponent::cast(m_row_dialog->get());
    if (dlg == OS_NULL) return;
    plist = dlg->firstcomponent(EOID_GUI_PARAMETER_LIST);
    if (plist == OS_NULL) return;

    row = new eContainer(ETEMPORARY);

    for (c = plist->firstcomponent(); c; c = c->nextcomponent())
    {
        n = c->primaryname();
        if (c->classid() != EGUICLASSID_LINE_EDIT || n == OS_NULL) continue;

        v = new eVariable(row);
        c->propertyv(ECOMP_VALUE, v);
        v->addname(n->gets(), ENAME_NO_MAP);
    }

    ix_col = m_rowset->ix_column_nr();

    if (command == ECOMPO_UPDATE_DLG_ROW) // Update
    {
        m = eMatrix::cast(m_row_dialog_m->get());
        if (m == OS_NULL) return;
        where_clause = "[";
        where_clause += m->getl(0, ix_col);
        where_clause += "]";
        m_rowset->update(where_clause.gets(), row, ETABLE_ADOPT_ARGUMENT);
    }

    else {
        m_rowset->insert(row, ETABLE_ADOPT_ARGUMENT);
    }

}
