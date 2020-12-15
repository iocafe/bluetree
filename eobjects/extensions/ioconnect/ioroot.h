/**

  @file    ioroot.h
  @brief   Root object.
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
#ifndef ECROOT_H_
#define ECROOT_H_
#include "ioconnect.h"

/**
****************************************************************************************************
  Defines
****************************************************************************************************
*/

/* Enumeration of GUI object properties.
 */
#define IOROOTP_VALUE 1
#define IOROOTP_STATE_BITS 2
#define IOROOTP_TIMESTAMP 3
#define IOROOTP_DIGS 4
#define IOROOTP_TEXT 6
#define IOROOTP_UNIT 8
#define IOROOTP_MIN 10
#define IOROOTP_MAX 12
#define IOROOTP_TYPE 14
#define IOROOTP_ATTR 16
#define IOROOTP_DEFAULT 18
#define IOROOTP_GAIN 20
#define IOROOTP_OFFSET 22
#define IOROOTP_CONF 24

/* GUI property names.
 */
extern const os_char
    iorootp_value[],
    iorootp_digs[],
    iorootp_text[],
    iorootp_unit[],
    iorootp_min[],
    iorootp_max[],
    iorootp_type[],
    iorootp_attr[],
    iorootp_default[],
    iorootp_gain[],
    iorootp_offset[],
    iorootp_state_bits[],
    iorootp_timestamp[],
    iorootp_conf[];


/**
****************************************************************************************************

  @brief ioRoot class.

  The ioRoot is root of graphical user interface display.

****************************************************************************************************
*/
class ioRoot : public eObject
{
public:
    /**
    ************************************************************************************************
      ioRoot overrides for eObject base class functions.
    ************************************************************************************************
    */
    /* Constructor.
     */
    ioRoot(
        eObject *parent = OS_NULL,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT);

    /* Virtual destructor.
     */
    virtual ~ioRoot();

    /* Clone object.
     */
    virtual eObject *clone(
        eObject *parent,
        e_oid id = EOID_CHILD,
        os_int aflags = 0);

    /* Casting eObject pointer to ioRoot pointer.
     */
    inline static ioRoot *cast(
        eObject *o)
    {
        e_assert_type(o, IOCONNCLASSID_ROOT)
        return (ioRoot*)o;
    }

    /* Get class identifier.
     */
    virtual os_int classid()
    {
        return IOCONNCLASSID_ROOT;
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
    static ioRoot *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new ioRoot(parent, id, flags);
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

    eStatus initialize(const os_char *device_name);
    void shutdown();



protected:
    iocRoot m_root;

};


#endif
