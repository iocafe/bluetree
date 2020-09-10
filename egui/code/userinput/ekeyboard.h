/**

  @file    eKeyboard.h
  @brief   Keyboard input.
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
#ifndef EKEYBOARD_H_
#define EKEYBOARD_H_
#include "eguilib.h"


/**
****************************************************************************************************

  @name Defines

****************************************************************************************************
*/
/*@{*/

/* Enumeration of specical keys.
 */
typedef enum eKeyCode
{
    EKEY_ESC,
    EKEY_RIGHT
}
eKeyCode;

/* Enumeration of events which trigger a keyboard message.
 */
typedef enum eKeyboardEvent
{
    EKEY_DOWN,
    EKEY_UP,
    EKEY_MOVE,
}
eKeyboardEvent;

/*@}*/


/**
****************************************************************************************************

  @brief eKeyboardMessage class.

  The eKeyboardMessage is defined as class to allow queueing keyboard messages and passing these
  from thread to another.

****************************************************************************************************
*/
class eKeyboardMessage : public eObject
{
    /**
    ************************************************************************************************

      @name eKeyboardMessage overrides for eObject base class functions.

      X...

    ************************************************************************************************
    */
    /*@{*/
public:
    /* Constructor.
     */
    eKeyboardMessage(
        eObject *parent = OS_NULL,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT);

    /* Virtual destructor.
     */
    virtual ~eKeyboardMessage();

    /* Clone object.
     */
    virtual eObject *clone(
        eObject *parent,
        e_oid id = EOID_CHILD,
        os_int aflags = 0);

    /* Casting eObject pointer to eKeyboardMessage pointer.
     */
    inline static eKeyboardMessage *cast(
        eObject *o)
    {
        e_assert_type(o, EGUICLASSID_KEYBOARD_MESSAGE)
        return (eKeyboardMessage*)o;
    }

    /* Get class identifier.
     */
    virtual os_int classid()
    {
        return EGUICLASSID_KEYBOARD_MESSAGE;
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
    static eKeyboardMessage *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new eKeyboardMessage(parent, id, flags);
    }

    /* Get next object identified by oid.
     */
    eKeyboardMessage *nextv(
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

      @name Keyboard message member variables

      These are simply public, function wrappers could be implemented.

    ************************************************************************************************
    */

    /* UTF32 character code.
     */
    os_int m_character;

    // os_int m_scancode;
    eKeyCode m_key_code;
};


#endif
