/**

  @file    etablecolumn.h
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
#ifndef ETABLCOLUMN_H_
#define ETABLCOLUMN_H_
#include "egui.h"

/**
****************************************************************************************************
  Defines
****************************************************************************************************
*/


/**
****************************************************************************************************
  The eTableColumn is stores table column state.
****************************************************************************************************
*/
class eTableColumn : public eObject
{
public:

    /**
    ************************************************************************************************
      Generic eObject functionality.
    ************************************************************************************************
    */
    /* Constructor.
     */
    eTableColumn(
        eObject *parent = OS_NULL,
        e_oid id = EOID_GUI_COMPONENT,
        os_int flags = EOBJ_DEFAULT);

    /* Virtual destructor.
     */
    virtual ~eTableColumn();

    /* Clone object.
     */
    virtual eObject *clone(
        eObject *parent,
        e_oid id = EOID_CHILD,
        os_int aflags = 0);

    /* Casting eObject pointer to eTableColumn pointer.
     */
    inline static eTableColumn *cast(
        eObject *o)
    {
        e_assert_type(o, EGUICLASSID_TABLE_COLUMN)
        return (eTableColumn*)o;
    }

    /* Get class identifier.
     */
    virtual os_int classid() {return EGUICLASSID_TABLE_COLUMN; }

    /* Static function to add class to propertysets and class list.
     */
    static void setupclass();

    /* Static constructor function for generating instance by class list.
     */
    static eTableColumn *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new eTableColumn(parent, id, flags);
    }

    void setup_column(
        eVariable *col_conf);

    os_int count_header_row_lines();


    inline const os_char *name() {return m_name.ptr();}

    inline void set_visible(bool visible) {m_visible = visible;}
    inline bool visible() {return m_visible;}

    void prepare_column_header_for_drawing();

    void draw_column_header(
        os_int column_nr,
        os_int nro_header_row_lines);

    void draw_value(
        eVariable *value,
        eTableView *view);

    void draw_edit(
        eVariable *value,
        eMatrix *m,
        eTableView *view);

    void activate(
        eMatrix *focus_row,
        os_int focus_column,
        eTableView *view);

    inline eShowAs showas() {return m_attr.showas(); }

protected:
    eStrBuffer m_name;
    eStrBuffer m_text;
    eStrBuffer m_unit;
    eAttrBuffer m_attr;

    os_short m_nro_header_row_lines;

    /* Column is visible flag.
     */
    bool m_visible;

};


#endif
