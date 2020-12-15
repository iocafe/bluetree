/**

  @file    ioconnect.h
  @brief   Connects to iocom device.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    14.9.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef IOCONNECT_H_
#define IOCONNECT_H_
#include "ioconnect.h"

/**
****************************************************************************************************

  @name Defines

  X...

****************************************************************************************************
*/

/* Enumeration of object properties.
 */
#define IOCONP_VALUE EVARP_VALUE
#define IOCONP_STATE_BITS EVARP_STATE_BITS
#define IOCONP_TIMESTAMP EVARP_TIMESTAMP
#define IOCONP_DIGS EVARP_DIGS
#define IOCONP_TEXT EVARP_TEXT
#define IOCONP_UNIT EVARP_UNIT
#define IOCONP_MIN EVARP_MIN
#define IOCONP_MAX EVARP_MAX
#define IOCONP_TYPE EVARP_TYPE
#define IOCONP_ATTR EVARP_ATTR
#define IOCONP_DEFAULT EVARP_DEFAULT
#define IOCONP_GAIN EVARP_GAIN
#define IOCONP_OFFSET EVARP_OFFSET
#define IOCONP_PATH 30

/* Property names.
 */
#define ioconp_value evarp_value
#define ioconp_digs evarp_digs
#define ioconp_text evarp_text
#define ioconp_unit evarp_unit
#define ioconp_min evarp_min
#define ioconp_max evarp_max
#define ioconp_type evarp_type
#define ioconp_attr evarp_attr
#define ioconp_default evarp_default
#define ioconp_gain evarp_gain
#define ioconp_offset evarp_offset
#define ioconp_state_bits evarp_state_bits
#define ioconp_timestamp evarp_timestamp
extern const os_char ioconp_path[];



/**
****************************************************************************************************

  @brief ioConnect class.

  The ioConnect is dynamically typed variable, which can store integers, floating point values
  and strings.

****************************************************************************************************
*/
class ioConnect : public eObject
{
    /**
    ************************************************************************************************
      eObject base class function overrides.
    ************************************************************************************************
    */
public:
    /* Constructor.
     */
    ioConnect(
        eObject *parent = OS_NULL,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT);

    /* Virtual destructor.
     */
    virtual ~ioConnect();

    /* Clone object.
     */
    virtual eObject *clone(
        eObject *parent,
        e_oid id = EOID_CHILD,
        os_int aflags = 0);

    /* Casting eObject pointer to ioConnect pointer.
     */
    inline static ioConnect *cast(
        eObject *o)
    {
        e_assert_type(o, IOCONNCLASSID_CONNECT)
        return (ioConnect*)o;
    }

    /* Get class identifier
     */
    virtual os_int classid() {return IOCONNCLASSID_CONNECT; }

    /* Static function to add class to propertysets and class list.
     */
    static void setupclass();

    /* Add class'es properties to property set.
     */
    static void setupproperties(
        os_int cls);

    /* Static constructor function for generating instance by class list.
     */
    static ioConnect *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new ioConnect(parent, id, flags);
    }

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

    /* Message to or trough this object.
     */
    /* virtual void onmessage(); */


protected:
};


#endif
