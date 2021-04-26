/**

  @file    egamecontroller.h
  @brief   Game controller, control speed, turning, etc.
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

    /* Function to process incoming messages.
     */
    void onmessage(
        eEnvelope *envelope);

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

    void autocenter_thumbstick(
        os_int thumbstick_nr0,
        os_long elapsed_us);

    /**
    ************************************************************************************************
      Member variables.
    ************************************************************************************************
    */

    static const os_int
        m_range_max = 10000,
        m_nro_thumbsticks = 2;

    eStrBuffer
        m_text;

    os_ushort
        m_alive;

    os_short
        m_TX[m_nro_thumbsticks],
        m_TY[m_nro_thumbsticks];

    os_boolean
        m_T[m_nro_thumbsticks],
        m_T1[m_nro_thumbsticks],
        m_T2[m_nro_thumbsticks];

    os_boolean
        m_X,
        m_Y,
        m_A,
        m_B;

    os_boolean
        m_DU,
        m_DL,
        m_DR,
        m_DD;

    os_boolean
        m_back,
        m_start,
        m_guide;

    os_timer
        m_update_timer,
        m_left_timer_press;
};


#endif
