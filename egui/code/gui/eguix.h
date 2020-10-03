/**

  @file    eguix.h
  @brief   GUI root object.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef EGUIX_H_
#define EGUIX_H_
#include "egui.h"

/**
****************************************************************************************************

  @name Defines

  X...

****************************************************************************************************
*/
/*@{*/

/* Enumeration of GUI object properties.
 */
#define EGUIP_TEXT 1
#define EGUIP_OPEN 10

/* GUI property names.
 */
extern const os_char
    eguip_text[],
    eguip_open[];

/*@}*/


/**
****************************************************************************************************

  @brief eGui class.

  The eGui is root of graphical user interface display.

****************************************************************************************************
*/
class eGui : public eObject
{
    friend class eComponent;

    /**
    ************************************************************************************************

      @name eGui overrides for eObject base class functions.

      X...

    ************************************************************************************************
    */
    /*@{*/
public:
    /* Constructor.
     */
    eGui(
        eObject *parent = OS_NULL,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT);

    /* Virtual destructor.
     */
    virtual ~eGui();

    /* Clone object.
     */
    virtual eObject *clone(
        eObject *parent,
        e_oid id = EOID_CHILD,
        os_int aflags = 0);

    /* Casting eObject pointer to eGui pointer.
     */
    inline static eGui *cast(
        eObject *o)
    {
        e_assert_type(o, EGUICLASSID_GUI)
        return (eGui*)o;
    }

    /* Get class identifier.
     */
    virtual os_int classid() {return EGUICLASSID_GUI; }

    /* Static function to add class to propertysets and class list.
     */
    static void setupclass();

    /* Add class'es properties to property set.
     */
    static void setupproperties(
        os_int cls);

    /* Generating ImGui autolabel.
     */
    os_long make_autolabel();

    /* Static constructor function for generating instance by class list.
     */
    static eGui *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new eGui(parent, id, flags);
    }

    /* Get the first child component identified by oid.
     */
    eComponent *firstcomponent(
        e_oid id = EOID_CHILD);

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

    /*@}*/



    /* Message to or trough this object.
     */
    /* virtual void onmessage(); */


    /**
    ************************************************************************************************

      @name Base class functions to implement component functionality

      CreaSerialization means writing object to stream or reading it from strem.

    ************************************************************************************************
    */
    /*@{*/

    /* Run equi.
     */
    eStatus run();

    /* Determine size, position of component and it's subcomponents. Setup Z order for draing.
     */
    eStatus layout(
        eRect& r,
        eLayoutParams& prm);

    /* Draw the component.
     */
    eStatus draw(
        eDrawParams& prm,
        os_int flags);

    /* Pass mouse event to component, returns true if mouse event was processed.
     */
    bool onmouse(
        eMouseMessage& mevent);

    /* Pass keyboard event to component, returns true if keyboard event was processed.
     */
    bool onkeyboard(
        eKeyboardMessage& mevent);

    /*@}*/

    /**
    ************************************************************************************************

      @name Base class functions to implement component functionality

      CreaSerialization means writing object to stream or reading it from strem.

    ************************************************************************************************
    */
    /*@{*/

    /* Set redo layout flag.
     */
    void redo_layout();

    /* Invalidate specified rectangle.
     */
    void invalidate(eRect& r);

    /* Set keyboard input focus to this component.
     */
    void focus();

    /* Capture mouse events to this component.
     */
    void capture_mouse();

    /*@}*/


    /**
    ************************************************************************************************

      @name Desktop application specific setup and functions.

      These function are for desktop application.

    ************************************************************************************************
    */
    /*@{*/
    void setup_desktop_application();


protected:
    /* Viewport, corresponds to operating system window, etc.
     */
    eViewPort *m_viewport;

    /* Parameters for drawing components
     */
    eDrawParams m_draw_prm;

    /* Display size.
     */
    eSize m_sz;

    /* ImGui autolabel count for generating labels for windows eWindow and ePopup.
     */
    os_long m_autolabel_count;
};


#endif
