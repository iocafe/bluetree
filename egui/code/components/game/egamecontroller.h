/**

  @file    egamecontroller.h
  @brief   Display camera, etc, live bitmap based image.
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
#ifndef EGAMECONTROLLER_H_
#define EGAMECONTROLLER_H_
#include "egui.h"


/**
****************************************************************************************************
  eGameController class.
****************************************************************************************************
*/
class eGameController : public eComponent
{
    /**
    ************************************************************************************************
      Generic eObject functionality.
    ************************************************************************************************
    */
public:
    /* Constructor.
     */
    eGameController(
        eObject *parent = OS_NULL,
        e_oid id = EOID_GUI_COMPONENT,
        os_int flags = EOBJ_DEFAULT);

    /* Clone object.
     */
    virtual eObject *clone(
        eObject *parent,
        e_oid id = EOID_CHILD,
        os_int aflags = 0);

    /* Casting eObject pointer to eGameController pointer.
     */
    inline static eGameController *cast(
        eObject *o)
    {
        e_assert_type(o, EGUICLASSID_GAME_CONTROLLER)
        return (eGameController*)o;
    }

    /* Get class identifier.
     */
    virtual os_int classid() {return EGUICLASSID_GAME_CONTROLLER; }

    /* Static function to add class to propertysets and class list.
     */
    static void setupclass();

    /* Called when property value changes.
     */
    virtual eStatus onpropertychange(
        os_int propertynr,
        eVariable *x,
        os_int flags);

    /* Get value of simple property (override).
     */
    eStatus simpleproperty(
        os_int propertynr,
        eVariable *x);

    /* Static constructor function for generating instance by class list.
     */
    static eGameController *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new eGameController(parent, id, flags);
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

    /* Process mouse click.
     */
    virtual os_boolean on_click(
        eDrawParams& prm,
        os_int mouse_gamecontroller_nr);


protected:

    /**
    ************************************************************************************************
      Protected functions
    ************************************************************************************************
    */

    os_boolean update_motion(
        os_timer timer_us,
        os_boolean change_it);

    /**
    ************************************************************************************************
      Member variables.
    ************************************************************************************************
    */

    eStrBuffer
        m_text;

    os_ushort
        m_alive;

    os_short
        m_speed,
        m_turn;

    os_boolean
        m_L1,
        m_L2,
        m_R1,
        m_R2,
        m_triangle,
        m_circle,
        m_cross,
        m_square;

    os_short
        m_stick_x,
        m_stick_y;

    os_timer
        m_update_timer;
};


#endif
