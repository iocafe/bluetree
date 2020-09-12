/**

  @file    ecomponent.h
  @brief   Abstract GUI component.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  Base call for all GUI components (widgets)...

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef ECOMPONENT_H_
#define ECOMPONENT_H_
#include "eguilib.h"

/**
****************************************************************************************************

  @name Defines

  X...

****************************************************************************************************
*/
/*@{*/

/* Enumeration of common GUI component properties. These often match eobject library eVariable
   property numbers and names, but a specific eComponent may not need to implement all.
   eComponent has also properties which eVariable doesn't have.
 */
#define ECOMP_VALUE EVARP_VALUE
#define ECOMP_STATE_BITS EVARP_STATE_BITS
#define ECOMP_TIMESTAMP EVARP_TIMESTAMP
#define ECOMP_DIGS EVARP_DIGS
#define ECOMP_TEXT EVARP_TEXT
#define ECOMP_UNIT EVARP_UNIT
#define ECOMP_MIN EVARP_MIN
#define ECOMP_MAX EVARP_MAX
#define ECOMP_TYPE EVARP_TYPE
#define ECOMP_ATTR EVARP_ATTR
#define ECOMP_DEFAULT EVARP_DEFAULT
#define ECOMP_GAIN EVARP_GAIN
#define ECOMP_OFFSET EVARP_OFFSET
#define ECOMP_CONF EVARP_CONF
#define ECOMP_PATH 30

/* Flags for eComponent::setupproperties() to specify which optional properties for
   the component class.
 */
#define ECOMP_NO_OPTIONAL_PROPERITES 0
#define ECOMP_VALUE_PROPERITES 1
#define ECOMP_CONF_PROPERITES 2 /* Is this really needed? */

/* GUI component property names. Many of these map directly eVariable's property names
 */
#define ecomp_value evarp_value
#define ecomp_digs evarp_digs
#define ecomp_text evarp_text
#define ecomp_unit evarp_unit
#define ecomp_min evarp_min
#define ecomp_max evarp_max
#define ecomp_type evarp_type
#define ecomp_attr evarp_attr
#define ecomp_default evarp_default
#define ecomp_gain evarp_gain
#define ecomp_offset evarp_offset
#define ecomp_state_bits evarp_state_bits
#define ecomp_timestamp evarp_timestamp
#define ecomp_conf evarp_conf
extern const os_char ecomp_path[];

/*@}*/


/* Parameters for layout(). These determine size, position of component and it's subcomponents.
   Setup Z order for draing.
 */
typedef struct eLayoutParams
{
    /* Enable component. Disabled components have size 0 and are not dron nor appear in Z order.
     */
    os_boolean enable;

    /* Component is visible. Invisible components are not drawn. Component still
       has size, but it cannot react to mouse or keyboard. Neither it is drawn.
     */
    os_boolean visible;

    /* This component can set input focus.
     */
    bool can_focus;

    /* This component can react to mouse and get mouse capture.
     */
    bool enable_mouse;

    /* This component is in edit mode.
     */
    bool edit_mode;
}
eLayoutParams;


typedef struct eDrawParams
{

}
eDrawParams;


/**
****************************************************************************************************

  @brief eComponent class.

  The eComponent is dynamically typed variable, which can store integers, floating point values
  and strings.

****************************************************************************************************
*/
class eComponent : public eObject
{
    /**
    ************************************************************************************************

      @name eComponent overrides for eObject base class functions.

      X...

    ************************************************************************************************
    */
    /*@{*/
public:
    /* Constructor.
     */
    eComponent(
        eObject *parent = OS_NULL,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT);

    /* Virtual destructor.
     */
    virtual ~eComponent();

    /* Clone object.
     */
    virtual eObject *clone(
        eObject *parent,
        e_oid id = EOID_CHILD,
        os_int aflags = 0);

    /* Casting eObject pointer to eComponent pointer.
     */
    inline static eComponent *cast(
        eObject *o)
    {
        e_assert_type(o, EGUICLASSID_COMPONENT)
        return (eComponent*)o;
    }

    /* Get class identifier.
     */
    virtual os_int classid()
    {
        return EGUICLASSID_COMPONENT;
    }

    /* Static function to add class to propertysets and class list.
     */
    static void setupclass();

    /* Add class'es properties to property set.
     */
    static void setupproperties(
        os_int cls,
        os_int flags);

    /* Static constructor function for generating instance by class list.
     */
    static eComponent *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new eComponent(parent, id, flags);
    }

    /* Get next object identified by oid.
     */
    eComponent *nextv(
        e_oid id = EOID_CHILD);

    /* Called when property value changes.
     */
    virtual void onpropertychange(
        os_int propertynr,
        eVariable *x,
        os_int flags);

    /* Get value of simple property.
     */
    virtual eStatus simpleproperty(
        os_int propertynr,
        eVariable *x);

    /*@}*/


    /**
    ************************************************************************************************

      @name Operator overloads

      The operator overloads are implemented for convinience, and map to the member functions.
      Using operator overloads may lead to more readable code, but may also confuse the
      reader.

    ************************************************************************************************
    */
    /*@{*/

    /** Operator "=", setting variable value.
     */
    /* inline const os_char operator=(const os_char x) { setl(x); return x; }
    inline const os_uchar operator=(const os_uchar x) { setl(x); return x; }
    inline const os_short operator=(const os_short x) { setl(x); return x; }
    inline const os_ushort operator=(const os_ushort x) { setl(x); return x; }
    inline const os_int operator=(const os_int x) { setl(x); return x; }
    inline const os_uint operator=(const os_uint x) { setl(x); return x; }
    inline const os_long operator=(const os_long x) { setl(x); return x; }
    inline const os_float operator=(const os_float x) { setd(x); return x; }
    inline const os_double operator=(const os_double x) { setd(x); return x; }
    inline const os_char *operator=(const os_char *x) { sets(x); return x; }
    inline void operator=(eComponent& x) { setv(&x); } */

    /** Operator "+=", appending variable value.
     */
    /* inline const os_char *operator+=(const os_char *x) { appends(x); return x; }
    inline void operator+=(eComponent& x) { appendv(&x); } */

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

#if 0

    /* Determine size, position of component and it's subcomponents. Setup Z order for draing.
     */
    virtual eStatus layout(
        eRect& r,
        eLayoutParams& prm);
#endif

    /* Draw the component.
     */
    virtual void draw(
        eDrawParams& prm) {}

#if 0
    /* Pass mouse event to component, returns true if mouse event was processed.
     */
    virtual bool onmouse(
        eMouseMessage& mevent);

    /* Pass keyboard event to component, returns true if keyboard event was processed.
     */
    virtual bool onkeyboard(
        eKeyboardMessage& mevent);

#endif
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

    /* Invalidate current component rectangle.
     */
    inline void invalidate()
    {
        invalidate(m_rect);
    }

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


protected:
    /* Current component rectangle.
     */
    eRect m_rect;

    /* Saved layout parameters.
     */
    eLayoutParams m_layout_prm;

    /* Minimum and maximum sizes in pixels what component can be drawn in and still looks acceptable.
     */
    eSize m_min_sz;
    eSize m_max_sz;

    /* Natural size for the component.
     */
    eSize m_natural_sz;
};


#endif
