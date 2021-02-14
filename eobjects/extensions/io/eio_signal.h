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

class eioVariable;
class eioRoot;

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
      X
    ************************************************************************************************
    */
    void setup(
        eioVariable *variable,
        struct eioMblkInfo *minfo,
        struct eioSignalInfo *sinfo);


    void up();
    void down(eVariable *x);

    inline iocSignal *iosignal() {return &m_signal; }
    inline os_short mblk_flags() {return m_mblk_flags;}

    inline os_int io_addr() { return m_signal.addr; }
    inline os_int io_n() { return m_signal.n; }
    inline os_int io_flags() { return m_signal.flags; }

protected:
    /* IO object hierarchy root (time stamps).
     */
    eioRoot *m_eio_root;

    ePointer *m_variable_ref;

    iocSignal m_signal;

    /* Memory block flags, bit fields: IOC_MBLK_DOWN, IOC_MBLK_UP. Copied here so no need to
       look up every time from eioMlk.
     */
    os_short m_mblk_flags;

    os_int m_ncolumns;
};

#endif
