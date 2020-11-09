/**

  @file    etableview.h
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

    /* Row pixel coordinates
     */
    // os_int m_y1;
    // os_int m_y2;
}
eTableRow;


/**
****************************************************************************************************
  The eTableView is GUI component to display and modify variable value.
****************************************************************************************************
*/
class eTableView : public eComponent
{
//    friend class eTableColumn;

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
      eComponent functionality
    ************************************************************************************************
    */
    /* Draw the component.
     */
    virtual eStatus draw(
        eDrawParams& prm);

    /* Index column info
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
      Protected functions.
    ************************************************************************************************
    */

    void draw_tooltip();

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

    /* Collect information about this object for tree browser.
     */
    virtual void object_info(
        eVariable *item,
        eVariable *name,
        eSet *appendix);


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

    /* Buffer for editing value.
     */
    eEditBuffer m_edit_buf;
    eAutoLabel m_label_edit;
};


#endif
