/**

  @file    ebutton.h
  @brief   Push button or menu item.
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
#ifndef EBUTTON_H_
#define EBUTTON_H_
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

  @brief eButton class.

****************************************************************************************************
*/
class eButton : public eComponent
{
    /**
    ************************************************************************************************

      @name eButton overrides for eObject base class functions.

      X...

    ************************************************************************************************
    */
    /*@{*/
public:
    /* Constructor.
     */
    eButton(
        eObject *parent = OS_NULL,
        e_oid id = EOID_GUI_COMPONENT,
        os_int flags = EOBJ_DEFAULT);

    /* Virtual destructor.
     */
    virtual ~eButton();

    /* Clone object.
     */
    virtual eObject *clone(
        eObject *parent,
        e_oid id = EOID_CHILD,
        os_int aflags = 0);

    /* Casting eObject pointer to eButton pointer.
     */
    inline static eButton *cast(
        eObject *o)
    {
        e_assert_type(o, EGUICLASSID_BUTTON)
        return (eButton*)o;
    }

    /* Get class identifier.
     */
    virtual os_int classid() {return EGUICLASSID_BUTTON; }

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
    static eButton *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new eButton(parent, id, flags);
    }

    /*@}*/


    /**
    ************************************************************************************************

      @name Base class functions to implement component functionality

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
    void set_toggled();

    bool m_set_toggled;
    bool m_imgui_toggl;

    eStrBuffer m_text;
};


#endif
