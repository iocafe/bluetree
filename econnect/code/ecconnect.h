/**

  @file    ecconnect.h
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
#ifndef ECCONNECT_H_
#define ECCONNECT_H_
#include "econnect.h"

/**
****************************************************************************************************

  @name Defines

  X...

****************************************************************************************************
*/

/* Enumeration of object properties.
 */
#define ECCONP_VALUE EVARP_VALUE
#define ECCONP_STATE_BITS EVARP_STATE_BITS
#define ECCONP_TIMESTAMP EVARP_TIMESTAMP
#define ECCONP_DIGS EVARP_DIGS
#define ECCONP_TEXT EVARP_TEXT
#define ECCONP_UNIT EVARP_UNIT
#define ECCONP_MIN EVARP_MIN
#define ECCONP_MAX EVARP_MAX
#define ECCONP_TYPE EVARP_TYPE
#define ECCONP_ATTR EVARP_ATTR
#define ECCONP_DEFAULT EVARP_DEFAULT
#define ECCONP_GAIN EVARP_GAIN
#define ECCONP_OFFSET EVARP_OFFSET
#define ECCONP_PATH 30

/* Property names.
 */
#define ecconp_value evarp_value
#define ecconp_digs evarp_digs
#define ecconp_text evarp_text
#define ecconp_unit evarp_unit
#define ecconp_min evarp_min
#define ecconp_max evarp_max
#define ecconp_type evarp_type
#define ecconp_attr evarp_attr
#define ecconp_default evarp_default
#define ecconp_gain evarp_gain
#define ecconp_offset evarp_offset
#define ecconp_state_bits evarp_state_bits
#define ecconp_timestamp evarp_timestamp
extern const os_char ecconp_path[];



/**
****************************************************************************************************

  @brief ecConnect class.

  The ecConnect is dynamically typed variable, which can store integers, floating point values
  and strings.

****************************************************************************************************
*/
class ecConnect : public eObject
{
    /**
    ************************************************************************************************

      @name ecConnect overrides for eObject base class functions.

      X...

    ************************************************************************************************
    */
public:
    /* Constructor.
     */
    ecConnect(
        eObject *parent = OS_NULL,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT);

    /* Virtual destructor.
     */
    virtual ~ecConnect();

    /* Clone object.
     */
    virtual eObject *clone(
        eObject *parent,
        e_oid id = EOID_CHILD,
        os_int aflags = 0);

    /* Casting eObject pointer to ecConnect pointer.
     */
    inline static ecConnect *cast(
        eObject *o)
    {
        e_assert_type(o, ECONNCLASSID_CONNECT)
        return (ecConnect*)o;
    }

    /* Get class identifier
     */
    virtual os_int classid() {return ECONNCLASSID_CONNECT; }

    /* Static function to add class to propertysets and class list.
     */
    static void setupclass();

    /* Add class'es properties to property set.
     */
    static void setupproperties(
        os_int cls);

    /* Static constructor function for generating instance by class list.
     */
    static ecConnect *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new ecConnect(parent, id, flags);
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
