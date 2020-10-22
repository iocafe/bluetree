/**

  @file    ecroot.h
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
#include "econnect.h"

/**
****************************************************************************************************

  @name Defines

  X...

****************************************************************************************************
*/

/* Enumeration of GUI object properties.
 */
#define ECROOTP_VALUE 1
#define ECROOTP_STATE_BITS 2
#define ECROOTP_TIMESTAMP 3
#define ECROOTP_DIGS 4
#define ECROOTP_TEXT 6
#define ECROOTP_UNIT 8
#define ECROOTP_MIN 10
#define ECROOTP_MAX 12
#define ECROOTP_TYPE 14
#define ECROOTP_ATTR 16
#define ECROOTP_DEFAULT 18
#define ECROOTP_GAIN 20
#define ECROOTP_OFFSET 22
#define ECROOTP_CONF 24

/* GUI property names.
 */
extern const os_char
    ecrootp_value[],
    ecrootp_digs[],
    ecrootp_text[],
    ecrootp_unit[],
    ecrootp_min[],
    ecrootp_max[],
    ecrootp_type[],
    ecrootp_attr[],
    ecrootp_default[],
    ecrootp_gain[],
    ecrootp_offset[],
    ecrootp_state_bits[],
    ecrootp_timestamp[],
    ecrootp_conf[];


/**
****************************************************************************************************

  @brief ecRoot class.

  The ecRoot is root of graphical user interface display.

****************************************************************************************************
*/
class ecRoot : public eObject
{
public:
    /**
    ************************************************************************************************
      ecRoot overrides for eObject base class functions.
    ************************************************************************************************
    */
    /* Constructor.
     */
    ecRoot(
        eObject *parent = OS_NULL,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT);

    /* Virtual destructor.
     */
    virtual ~ecRoot();

    /* Clone object.
     */
    virtual eObject *clone(
        eObject *parent,
        e_oid id = EOID_CHILD,
        os_int aflags = 0);

    /* Casting eObject pointer to ecRoot pointer.
     */
    inline static ecRoot *cast(
        eObject *o)
    {
        e_assert_type(o, ECONNCLASSID_ROOT)
        return (ecRoot*)o;
    }

    /* Get class identifier.
     */
    virtual os_int classid()
    {
        return ECONNCLASSID_ROOT;
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
    static ecRoot *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new ecRoot(parent, id, flags);
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
