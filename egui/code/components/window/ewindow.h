/**

  @file    ewindow.h
  @brief   GUI window class.
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
#ifndef EWINDOW_H_
#define EWINDOW_H_
#include "egui.h"


/**
****************************************************************************************************

  @brief eWindow class.

  The eWindow is dynamically typed variable, which can store integers, floating point values
  and strings.

****************************************************************************************************
*/
class eWindow : public eComponent
{
    /**
    ************************************************************************************************

      @name eWindow overrides for eObject base class functions.

      X...

    ************************************************************************************************
    */
    /*@{*/
public:
    /* Constructor.
     */
    eWindow(
        eObject *parent = OS_NULL,
        e_oid id = EOID_GUI_WINDOW,
        os_int flags = EOBJ_DEFAULT);

    /* Virtual destructor.
     */
    virtual ~eWindow();

    /* Clone object.
     */
    virtual eObject *clone(
        eObject *parent,
        e_oid id = EOID_CHILD,
        os_int aflags = 0);

    /* Casting eObject pointer to eWindow pointer.
     */
    inline static eWindow *cast(
        eObject *o)
    {
        e_assert_type(o, EGUICLASSID_WINDOW)
        return (eWindow*)o;
    }

    /* Get class identifier.
     */
    virtual os_int classid() {return EGUICLASSID_WINDOW; }

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
    static eWindow *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new eWindow(parent, id, flags);
    }

    /* Generating ImGui autolabel.
     */
    virtual os_long make_autolabel();

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

    /*@}*/


    /* Get/set edit mode.
     */
    inline os_boolean editmode() {return m_edit_mode; }
    inline void set_editmode(os_int x) {m_edit_mode = (os_boolean)x; }

protected:
    void draw_edit_mode_decorations(
        eDrawParams& prm);

    virtual void object_info(
        eVariable *item,
        eVariable *name,
        eSet *appendix);

    void open_popup(
        eDrawParams& prm);

    void click(
        eDrawParams& prm,
        os_int mouse_button_nr);

    void start_drag(
        eDrawParams& prm,
        os_int mouse_button_nr);

    void drag(
        eDrawParams& prm,
        os_int mouse_button_nr);

    void drop(
        eDrawParams& prm,
        os_int mouse_button_nr);

    eAutoLabel m_label_title;

    /* ImGui autolabel count for generating labels for components within the eWindow.
     */
    os_long m_autolabel_count;

    /* This component is in edit mode.
     */
    os_boolean m_edit_mode;

};


#endif
