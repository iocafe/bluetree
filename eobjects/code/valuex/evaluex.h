/**

  @file    evaluex.h
  @brief   Extended value class.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    2.10.2020

  Value with timestamp and state bits to value.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef EVALUEX_H_
#define EVALUEX_H_
#include "eobjects.h"

/* Extended value property numbers. Extended value has all variable properties as well.
   SBITS = State bits
   TSTAMP = Time stamp
 */
#define EVALXP_SBITS 20
#define EVALXP_TSTAMP 21


/* Extended value property names.
 */
extern const os_char
    evalxp_sbits[],
    evalxp_tstamp[];


/**
****************************************************************************************************

  @brief eValueX class.

  The eValueX is a class derived from eVariable. It adds time stamp and state bits.

****************************************************************************************************
*/
class eValueX : public eVariable
{
    friend class eValueXSpace;

    /**
    ************************************************************************************************

      @name Generic object functionality.

      These functions enable using objects of this class as a generic eObject.

    ************************************************************************************************
    */
    /*@{*/
public:
    /* Constructor.
     */
    eValueX(
        eObject *parent = OS_NULL,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT);

    /* Virtual destructor.
     */
    virtual ~eValueX();

    /* Clone the eValueX.
     */
    virtual eObject *clone(
        eObject *parent,
        e_oid id = EOID_CHILD,
        os_int aflags = 0);

    /* Cast eObject pointer to eValueX pointer.
     */
    inline static eValueX *cast(
        eObject *o)
    {
        e_assert_type(o, ECLASSID_VALUEX)
        return (eValueX*)o;
    }

    /* Get class identifier.
     */
    virtual os_int classid() {return ECLASSID_VALUEX;}

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
    static eValueX *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new eValueX(parent, id, flags);
    }

    /* Write name to stream.
     */
    virtual eStatus writer(
        eStream *stream,
        os_int flags);

    /* Read name from stream.
     */
    virtual eStatus reader(
        eStream *stream,
        os_int flags);

    /*@}*/


    /**
    ************************************************************************************************
      Extended value functions
    ************************************************************************************************
    */
    inline os_int sbits() {return m_state_bits;}
    inline os_long tstamp() {return m_timestamp;}
    inline void set_sbits(os_int x) {m_state_bits = x;}
    inline void set_tstamp(os_long x) {m_timestamp = x;}

protected:
    os_int m_state_bits;
    os_long m_timestamp;
};

#endif
