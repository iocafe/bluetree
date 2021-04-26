/**

  @file    etableview.h
  @brief   Display table data in GUI.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef ETABLEVIEW_H_
#define ETABLEVIEW_H_
#include "egui.h"

/**
****************************************************************************************************
  Defines
****************************************************************************************************
*/


typedef struct
{
    /* Pointer to row data matrix.
     */
    eMatrix *m_row;
}
eTableRow;


/* FLags for edit_row_dialog().
 */
#define OETABLE_EDIT_ROW_NEW 1
#define OETABLE_EDIT_ROW_COPY 2


/**
****************************************************************************************************
  eTableView class.
****************************************************************************************************
*/
class eTableView : public eComponent
{
public:

    /**
    ************************************************************************************************
      Generic eObject functionality.
    ************************************************************************************************
    */
    /* Constructor.
     */
    eTableView(
        eObject *parent = OS_NULL,
        e_oid id = EOID_GUI_COMPONENT,
        os_int flags = EOBJ_DEFAULT);

    /* Virtual destructor.
     */
    virtual ~eTableView();

    /* Clone object.
     */
    virtual eObject *clone(
        eObject *parent,
        e_oid id = EOID_CHILD,
        os_int aflags = 0);

    /* Casting eObject pointer to eTableView pointer.
     */
    inline static eTableView *cast(
        eObject *o)
    {
        e_assert_type(o, EGUICLASSID_TABLE_VIEW)
        return (eTableView*)o;
    }

    /* Get class identifier.
     */
    virtual os_int classid() {return EGUICLASSID_TABLE_VIEW; }

    /* Static function to add class to propertysets and class list.
     */
    static void setupclass();

    /* Called when property value changes.
     */
    virtual eStatus onpropertychange(
        os_int propertynr,
        eVariable *x,
        os_int flags);

    /* Static constructor function for generating instance by class list.
     */
    static eTableView *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new eTableView(parent, id, flags);
    }


    /**
    ************************************************************************************************
      GUI component functionality (eComponent)
    ************************************************************************************************
    */
    /* Draw the component.
     */
    virtual eStatus draw(
        eDrawParams& prm);

    /* Component clicked.
     */
    virtual os_boolean on_click(
        eDrawParams& prm,
        os_int mouse_button_nr);


    /**
    ************************************************************************************************
      eTableView specific functions
    ************************************************************************************************
    */
    /* Get index column name and index value
     */
    const os_char *ix_column_name();
    os_long ix_value(eMatrix *m);

    /* Select the data to display.
     */
    void select();

    /* eTableColumn to access edit buffer.
     */
    inline os_char *edit_buf() {return m_edit_buf.ptr(); }
    inline os_memsz edit_sz() {return m_edit_buf.sz(); }
    inline const os_char *edit_label() {return m_label_edit.get(this); }

    /* Keyboard focus set flag
     */
    inline void set_keyboard_focus_ok(os_boolean ok) {m_keyboard_focus_ok = ok;}
    inline os_boolean keyboard_focus_ok() {return m_keyboard_focus_ok;}

    void focus_cell(
        eMatrix *focus_row,
        os_int focus_column = -1,
        const os_char *edit_str = "",
        os_int edit_sz = 128);

    void update_table_cell(
        const os_char *ix_column_name,
        os_long ix_value,
        const os_char *column_name,
        eVariable *column_value);

    /* Generate right click popup menu.
     */
    virtual ePopup *right_click_popup(
        eDrawParams& prm);

protected:

    /**
    ************************************************************************************************
      Protected member functions.
    ************************************************************************************************
    */

    void draw_header_row();

    /* Callback when table data is received, etc.
     */
    void callback(
        eRowSet *rset,
        ersetCallbackInfo *ci);

    /* Static callback function to use underlying C code..
     */
    static void static_callback(
        eRowSet *rset,
        ersetCallbackInfo *ci,
        eObject *context);

    /* Setup m_row_to_m array, converts row number 0... to eMatrix pointer.
     */
    void fill_row_to_m();

    /* Create eTableColumn object for each column.
     */
    void setup_columns();

    /* Count number of header lines, called by setup_columns()
     */
    void count_header_row_lines();

    /* Create dialog window to edit a row or add a new row.
     */
    void edit_row_dialog(
        os_int flags);

    /* Create dialog window to delete row (or rows). This function can also just delete the row.
     */
    void delete_row_dialog();

    /* Save new row or edit row dialog values to table.
     */
    void save_dialog_row(
        os_int command);

    /* Collect information about this object for tree browser.
     */
    virtual void object_info(
        eVariable *item,
        eVariable *name,
        eSet *appendix,
        const os_char *target);


    /**
    ************************************************************************************************
      Member variables.
    ************************************************************************************************
    */

    /* Table columns, eTableColumn objects.
     */
    eContainer *m_columns;

    /* Row set (data selected from table)
     */
    eRowSet *m_rowset;

    /* Converting row number to eMatrix pointer
     */
    eTableRow *m_row_to_m;
    os_memsz m_row_to_m_sz;
    os_int m_row_to_m_len;

    /* Input focus
     */
    ePointer *m_focused_row;
    os_int m_focused_column;
    os_boolean m_keyboard_focus_ok;

    /* Column header row can have multiple lines of text, how many?
     */
    os_int m_nro_header_row_lines;

    /* Screen Y coordinate where the table content starts (below column header)
     */
    os_int m_data_windows_start_y;

    /* Here the data starts logically, can be smaller than m_data_windows_start_y
       if table is scrolled.
     */
    os_int m_logical_data_start_y;

    /* Data row height in pixels. For now we assume all rows to have same height.
     */
    os_int m_data_row_h;

    os_int m_hovered_column;

    /* ePointer to "edit row dialog" to edit existing row or add a new row.
       This can be moved to eGui if we want only one edit row window to be open at a time.
     */
    ePointer *m_row_dialog;

    /* ePointer to row matrix referrd by row dialog.
     */
    ePointer *m_row_dialog_m;

    /* Buffer for editing value.
     */
    eEditBuffer m_edit_buf;
    eAutoLabel m_label_edit;
};


#endif
