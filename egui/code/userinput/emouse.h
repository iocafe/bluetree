/**

  @file    emouse.h
  @brief   Mouse input.
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
#ifndef EMOUSE_H_
#define EMOUSE_H_
#include "eguilib.h"


/**
****************************************************************************************************

  @name Defines

****************************************************************************************************
*/
/*@{*/

/* Enumeration of mouse buttons.
 */
typedef enum eMouseButton
{
    EMOUSE_LEFT_BUTTON,
    EMOUSE_RIGHT_BUTTON,
    EMOUSE_SCROLL_WHEEL
}
eMouseButton;

/* Enumeration of events which trigger a mouse message.
 */
typedef enum eMouseEvent
{
    EMOUSE_DOWN,
    EMOUSE_UP,
    EMOUSE_MOVE,
}
eMouseEvent;

/*@}*/


/**
****************************************************************************************************

  @brief eMouseMessage class.

  The eMouseMessage is defined as class to allow queueing mouse messages and passing these
  from thread to another.

****************************************************************************************************
*/
class eMouseMessage : public eObject
{
    /**
    ************************************************************************************************

      @name eMouseMessage overrides for eObject base class functions.

      X...

    ************************************************************************************************
    */
    /*@{*/
public:
    /* Constructor.
     */
    eMouseMessage(
        eObject *parent = OS_NULL,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT);

    /* Virtual destructor.
     */
    virtual ~eMouseMessage();

    /* Clone object.
     */
    virtual eObject *clone(
        eObject *parent,
        e_oid id = EOID_CHILD,
        os_int aflags = 0);

    /* Casting eObject pointer to eMouseMessage pointer.
     */
    inline static eMouseMessage *cast(
        eObject *o)
    {
        e_assert_type(o, EGUICLASSID_MOUSE_MESSAGE)
        return (eMouseMessage*)o;
    }

    /* Get class identifier.
     */
    virtual os_int classid()
    {
        return EGUICLASSID_MOUSE_MESSAGE;
    }

    /* Static function to add class to propertysets and class list.
     */
    static void setupclass();

    /* Add class'es properties to property set.
     */
    static void setupproperties(
        os_int cls);

    /* Static constructor function for generating instance by class list.
     */
    static eMouseMessage *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new eMouseMessage(parent, id, flags);
    }

    /* Get next object identified by oid.
     */
    eMouseMessage *nextv(
        e_oid id = EOID_CHILD);

    /*@}*/


    /**
    ************************************************************************************************

      @name eObject virtual function implementations

      Serialization means writing object to stream or reading it from strem.

    ************************************************************************************************
    */
    /*@{*/

    /* Write variable to stream.
     */
    virtual eStatus writer(
        eStream *stream,
        os_int flags);

    /* Read variable from stream.
     */
    virtual eStatus reader(
        eStream *stream,
        os_int flags);


    /**
    ************************************************************************************************

      @name Mouse message member variables

      These are simply public, function wrappers could be implemented.

    ************************************************************************************************
    */

    /* Mouse down, mouse up or mouse move.
     */
    eMouseEvent m_mouse_event;

    /* Which mouse button, or scroll wheel.
     */
    eMouseButton m_button;

    /* Mouse position.
     */
    ePos m_pos;

    /* Timer when mouse message was generated.
     */
    os_timer m_timer;

    /* Mouse has been moved after mouse button was pressed down significant number of pixels.
     */
    bool m_moved_down;

    // Keyboard shift codes ???

    // eMouseSwipe eMouseSwipe;
};


#endif
