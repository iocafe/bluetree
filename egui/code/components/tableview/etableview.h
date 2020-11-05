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


/**
****************************************************************************************************
  The eTableView is GUI component to display and modify variable value.
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
      eComponent functionality
    ************************************************************************************************
    */
    /* Draw the component.
     */
    virtual eStatus draw(
        eDrawParams& prm);

    /* Select the data to display.
     */
    void select();

protected:

    /**
    ************************************************************************************************
      Member variables.
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

    /* Table columns, eTableColumn objects.
     */
    eContainer *m_columns;

    /* Row set (data selected from table)
     */
    eRowSet *m_rowset;

    /* Converting row number to eMatrix pointer
     */
    eMatrix **m_row_to_m;
    os_memsz m_row_to_m_sz;
    os_int m_row_to_m_len;
};


#endif
