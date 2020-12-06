/**

  @file    eparameterlist.h
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
#ifndef EPARAMETERLIST_H_
#define EPARAMETERLIST_H_
#include "egui.h"

/**
****************************************************************************************************
  Defines
****************************************************************************************************
*/

typedef struct ePrmListComponent
{
    eComponent *m_ptr;
}
ePrmListComponent;


/**
****************************************************************************************************
  eParameterList class
****************************************************************************************************
*/
class eParameterList : public eComponent
{
public:

    /**
    ************************************************************************************************
      Generic eObject functionality.
    ************************************************************************************************
    */
    /* Constructor.
     */
    eParameterList(
        eObject *parent = OS_NULL,
        e_oid id = EOID_GUI_PARAMETER_LIST,
        os_int flags = EOBJ_DEFAULT);

    /* Virtual destructor.
     */
    virtual ~eParameterList();

    /* Clone object.
     */
    virtual eObject *clone(
        eObject *parent,
        e_oid id = EOID_CHILD,
        os_int aflags = 0);

    /* Casting eObject pointer to eParameterList pointer.
     */
    inline static eParameterList *cast(
        eObject *o)
    {
        e_assert_type(o, EGUICLASSID_PARAMETER_LIST)
        return (eParameterList*)o;
    }

    /* Get class identifier.
     */
    virtual os_int classid() {return EGUICLASSID_PARAMETER_LIST; }

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
    static eParameterList *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new eParameterList(parent, id, flags);
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

    void generate_component_array();

    os_int count_subcomponents(
        eComponent *compo);


    /**
    ************************************************************************************************
      Member variables.
    ************************************************************************************************
    */

    ePrmListComponent *m_component;
    os_int m_nro_components;
    os_memsz m_component_array_sz;
    os_int m_treebrowser_row_count;
};


#endif
