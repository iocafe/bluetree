/**

  @file    eio_variable.h
  @brief   IO variable class.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    2.10.2020

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


/**
****************************************************************************************************
  The eioVariable is a class derived from eVariable, used to connect to IOCOM signal(s).
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

    /**
    ************************************************************************************************
      IO variable specific functions
    ************************************************************************************************
    */
    void setup(
        struct eioSignal *signal,
        struct eioMblkInfo *minfo,
        struct eioSignalInfo *sinfo);

    void up(eValueX *x);

    void down();

    /* Process a callback from a child object.
     */
    eStatus oncallback(
        eCallbackEvent event,
        eObject *obj,
        eObject *appendix);

    /* Decide value for "bound" flag.
     */
    void set_bound();

protected:

    /**
    ************************************************************************************************
      Member variables
    ************************************************************************************************
    */
    ePointer *m_down_ref;

    os_boolean m_bound;
    os_boolean m_value_set_by_user;
    os_short m_my_own_change;
};

#endif
