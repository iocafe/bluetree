/**

  @file    eio_variable.h
  @brief   IO variable class.
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
#ifndef EIO_VARIABLE_H_
#define EIO_VARIABLE_H_
#include "eobjects.h"

typedef struct eioSignalMaping
{
    /** Memory block name, max 15 characters.
     */
    os_char mblk_name[IOC_NAME_SZ];

    /** Memory block handle.
     */
    iocHandle handle;

}
eioSignalMaping;


/**
****************************************************************************************************
  The eioVariable is a class derived from eVariable. It adds time stamp and state bits.
****************************************************************************************************
*/
class eioVariable : public eVariable
{
    friend class eioVariableSpace;

    /**
    ************************************************************************************************
      Generic eObject functionality.
    ************************************************************************************************
    */
public:
    /* Constructor.
     */
    eioVariable(
        eObject *parent = OS_NULL,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_EROOT_OPTIONAL);

    /* Virtual destructor.
     */
    virtual ~eioVariable();

    /* Clone the eioVariable.
     */
    virtual eObject *clone(
        eObject *parent,
        e_oid id = EOID_CHILD,
        os_int aflags = 0);

    /* Cast eObject pointer to eioVariable pointer.
     */
    inline static eioVariable *cast(
        eObject *o)
    {
        e_assert_type(o, ECLASSID_EIO_VARIABLE)
        return (eioVariable*)o;
    }

    /* Get class identifier.
     */
    virtual os_int classid() {return ECLASSID_EIO_VARIABLE;}

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
    static eioVariable *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new eioVariable(parent, id, flags);
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

    /**
    ************************************************************************************************
      X
    ************************************************************************************************
    */
    void setup(
        struct eioMblkInfo *minfo,
        struct eioSignalInfo *sinfo);

    void up(eValueX *x);

protected:
    eioSignalMaping m_in;
    eioSignalMaping m_out;

    os_int m_state_bits;
    os_long m_timestamp;
};

#endif
