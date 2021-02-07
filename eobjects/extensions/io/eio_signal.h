/**

  @file    esignal.h
  @brief   Object representing an IO signal.
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
#ifndef EIO_SIGNAL_H_
#define EIO_SIGNAL_H_
#include "eobjects.h"



/**
****************************************************************************************************
  The eioSignal is a class derived from eVariable. It adds time stamp and state bits.
****************************************************************************************************
*/
class eioSignal : public eVariable
{
    friend class eioSignalSpace;

    /**
    ************************************************************************************************
      Generic eObject functionality.
    ************************************************************************************************
    */
public:
    /* Constructor.
     */
    eioSignal(
        eObject *parent = OS_NULL,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_EROOT_OPTIONAL);

    /* Virtual destructor.
     */
    virtual ~eioSignal();

    /* Clone the eioSignal.
     */
    virtual eObject *clone(
        eObject *parent,
        e_oid id = EOID_CHILD,
        os_int aflags = 0);

    /* Cast eObject pointer to eioSignal pointer.
     */
    inline static eioSignal *cast(
        eObject *o)
    {
        e_assert_type(o, ECLASSID_EIO_SIGNAL)
        return (eioSignal*)o;
    }

    /* Get class identifier.
     */
    virtual os_int classid() {return ECLASSID_EIO_SIGNAL;}

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
    static eioSignal *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new eioSignal(parent, id, flags);
    }

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
