/**

  @file    etreenode.h
  @brief   Line edit with label and value.
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
#ifndef ETREENODE_H_
#define ETREENODE_H_
#include "egui.h"

/**
****************************************************************************************************
  Defines
****************************************************************************************************
*/

#define ETREENODE_TOOLTIPS_FOR_DEBUG 1


/**
****************************************************************************************************
  eTreeNode class is used to implement object tree browser.
****************************************************************************************************
*/
class eTreeNode : public eComponent
{
public:

    /**
    ************************************************************************************************
      Generic eObject functionality.
    ************************************************************************************************
    */
    /* Constructor.
     */
    eTreeNode(
        eObject *parent = OS_NULL,
        e_oid id = EOID_GUI_COMPONENT,
        os_int flags = EOBJ_DEFAULT);

    /* Virtual destructor.
     */
    virtual ~eTreeNode();

    /* Clone object.
     */
    virtual eObject *clone(
        eObject *parent,
        e_oid id = EOID_CHILD,
        os_int aflags = 0);

    /* Casting eObject pointer to eTreeNode pointer.
     */
    inline static eTreeNode *cast(
        eObject *o)
    {
        e_assert_type(o, EGUICLASSID_TREE_NODE)
        return (eTreeNode*)o;
    }

    /* Get class identifier.
     */
    virtual os_int classid() {return EGUICLASSID_TREE_NODE; }

    /* Static function to add class to propertysets and class list.
     */
    static void setupclass();

    /* Function to process incoming messages.
     */
    virtual void onmessage(
        eEnvelope *envelope);

    /* Called when property value changes.
     */
    virtual eStatus onpropertychange(
        os_int propertynr,
        eVariable *x,
        os_int flags);

    /* Static constructor function for generating instance by class list.
     */
    static eTreeNode *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new eTreeNode(parent, id, flags);
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

    /* Draw the component within parameter list.
     */
    virtual void draw_in_parameter_list(
        eDrawParams& prm);

    /* Component clicked (mouse).
     */
    virtual os_boolean on_click(
        eDrawParams& prm,
        os_int mouse_button_nr);

    /* Activate the component (start editing value, toggle checkbox, or show drop down list).
     */
    virtual void activate();

    /* Generate right click popup menu.
     */
    virtual ePopup *right_click_popup(
        eDrawParams& prm);

    os_int count_rows();

    void clear_row_count();

protected:

    /**
    ************************************************************************************************
      Protected functions
    ************************************************************************************************
    */
    void draw_value(
        eDrawParams& prm,
        os_int value_w,
        os_int *total_h);

    void draw_underline(
        os_int y);

    virtual void draw_tooltip();

    void request_object_info();

    void set_modified_value();

    os_int setup_node(
        eVariable *item,
        eVariable& ipath,
        eVariable& path);

    /**
    ************************************************************************************************
      Member variables.
    ************************************************************************************************
    */

    os_int m_received_change;

    bool m_autoopen;
    bool m_isopen;
    bool m_child_data_received;
    bool m_show_expand_arrow;
    bool m_edit_value;
    bool m_prev_edit_value;
    bool m_intermediate_node;

    os_boolean m_all;

    os_int m_node_type;

    /** Additional selections in right click popup menu, like EBROWSE_OPEN,
        EBROWSE_GRAPH, etc.
.    */
    os_int m_right_click_selections;

    os_int m_row_count;

    /* Buffer for editing value.
     */
    eEditBuffer m_edit_buf;

    /* Property values */
    eStrBuffer m_text;
    eStrBuffer m_unit;
    eStrBuffer m_path;
    eStrBuffer m_ipath;

    /* Parsed attrbutes, digits, etc.
     */
    eAttrBuffer m_attr;

    eAutoLabel m_label_node;
    eStrBuffer m_value;
    eAutoLabel m_label_edit;

#if ETREENODE_TOOLTIPS_FOR_DEBUG
    os_uint m_object_flags;
#endif

    eRect m_value_rect;
};


#endif
