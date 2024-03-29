/**

  @file    epopup.h
  @brief   GUI popup like drop down menu.
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
#ifndef EPOPUP_H_
#define EPOPUP_H_
#include "egui.h"


/**
****************************************************************************************************
  GUI popup window ("right click" menu, "select value" from drop down list).
****************************************************************************************************
*/
class ePopup : public eComponent
{
public:
    /**
    ************************************************************************************************
      Generic eObject functionality.
    ************************************************************************************************
    */
    /* Constructor.
     */
    ePopup(
        eObject *parent = OS_NULL,
        e_oid id = EOID_GUI_POPUP,
        os_int flags = EOBJ_DEFAULT);

    /* Virtual destructor.
     */
    virtual ~ePopup();

    /* Clone object.
     */
    virtual eObject *clone(
        eObject *parent,
        e_oid id = EOID_CHILD,
        os_int aflags = 0);

    /* Casting eObject pointer to ePopup pointer.
     */
    inline static ePopup *cast(
        eObject *o)
    {
        e_assert_type(o, EGUICLASSID_POPUP)
        return (ePopup*)o;
    }

    /* Get class identifier.
     */
    virtual os_int classid() {return EGUICLASSID_POPUP; }

    /* Static function to add class to propertysets and class list.
     */
    static void setupclass();

    /* Called when property value changes.
     */
    virtual eStatus onpropertychange(
        os_int propertynr,
        eVariable *x,
        os_int flags);

    /* Get value of simple property.
     */
    virtual eStatus simpleproperty(
        os_int propertynr,
        eVariable *x);

    /* Static constructor function for generating instance by class list.
     */
    static ePopup *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new ePopup(parent, id, flags);
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

    /* Generating ImGui autolabel.
     */
    virtual os_long make_autolabel();

protected:

    /**
    ************************************************************************************************
      Member variables.
    ************************************************************************************************
    */
    os_boolean m_open_popup_called;

    eAutoLabel m_label_title;

    /* ImGui autolabel count for generating labels for components within the ePopup.
     */
    os_long m_autolabel_count;
};


#endif
