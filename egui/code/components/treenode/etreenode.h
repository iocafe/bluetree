/**

  @file    etreenode.h
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
#pragma once
#ifndef ETREENODE_H_
#define ETREENODE_H_
#include "egui.h"

/**
****************************************************************************************************

  @name Defines

  X...

****************************************************************************************************
*/
/*@{*/

/*@}*/


/**
****************************************************************************************************

  @brief eTreeNode class.

  The eTreeNode is dynamically typed variable, which can store integers, floating point values
  and strings.

****************************************************************************************************
*/
class eTreeNode : public eComponent
{
    /**
    ************************************************************************************************

      @name eTreeNode overrides for eObject base class functions.

      X...

    ************************************************************************************************
    */
    /*@{*/
public:
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

    /*@}*/


    /**
    ************************************************************************************************

      @name Base class functions to implement component functionality

      CreaSerialization means writing object to stream or reading it from strem.

    ************************************************************************************************
    */
    /*@{*/

    /* Draw the component.
     */
    virtual eStatus draw(
        eDrawParams& prm);

    /* Activate the component (start editing value, toggle checkbox, or show drop down list).
     */
    virtual void activate();

    /*@}*/


protected:
    void draw_state_bits(
        os_int x);

    void draw_tooltip();

    void request_object_info();

    void set_modified_value();

    void set_checked();

    os_int setup_node(
        eVariable *item,
        eVariable& ipath,
        eVariable& path);

    os_int m_received_change;

    bool m_autoopen;
    bool m_isopen;
    bool m_child_data_received;
    bool m_show_expand_arrow;
    bool m_edit_value;
    bool m_prev_edit_value;
    bool m_set_checked;
    bool m_imgui_checked;
    bool m_intermediate_node;

    os_int m_node_type;

    /* Buffer for editing value.
     */
    eEditBuffer m_edit_buf;

    /* Property values */
    eStrBuffer m_text;
    eStrBuffer m_unit;
    eStrBuffer m_path;
    eStrBuffer m_ipath;

    eVariable m_bound_ipath;

    /* Parsed attrbutes, digits, etc.
     */
    eAttrBuffer m_attr;

    eAutoLabel m_label_node;
    eAutoLabel m_label_value;
    eAutoLabel m_label_edit;
};


#endif
