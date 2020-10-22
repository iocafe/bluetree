/**

  @file    eset.h
  @brief   Set of enumerated values.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  A set stores key, value pairs. Key is always integer and value can be an primitive type
  or object of any type. The eSet is efficient when smallish set of mostly primitive values
  needs to be kept. Typical use is storing object's properties.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef ESET_H_
#define ESET_H_
#include "eobjects.h"

/* Set flags
 */
#define ESET_PERSISTENT 0
#define ESET_TEMPORARY 1
#define ESET_STORE_AS_VARIABLE 2
#define ESET_ADOPT_X_CONTENT 64
#define ESET_DELETE_X 128

/**
****************************************************************************************************
  eSet stored set of values with integer keys.
****************************************************************************************************
*/
class eSet : public eObject
{
public:

    /**
    ************************************************************************************************
      Generic eObject functionality.
    ************************************************************************************************
    */
    /* Constructor.
     */
    eSet(
        eObject *parent = OS_NULL,
        e_oid id = EOID_RITEM,
        os_int flags = EOBJ_DEFAULT);

    /* Virtual destructor.
     */
    virtual ~eSet();

    /* Clone object.
     */
    virtual eObject *clone(
        eObject *parent,
        e_oid id = EOID_CHILD,
        os_int aflags = 0);

    /* Casting eObject pointer to eSet pointer.
     */
    inline static eSet *cast(
        eObject *o)
    {
        e_assert_type(o, ECLASSID_SET)
        return (eSet*)o;
    }

    /* Get class identifier.
     */
    virtual os_int classid() {return ECLASSID_SET; }

    /* Static function to add class to propertysets and class list.
     */
    static void setupclass();

    /* Static constructor function for generating instance by class list.
     */
    static eSet *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new eSet(parent, id, flags);
    }

    /* Get next set identified by oid.
     */
    eSet *nexts(
        e_oid id = EOID_CHILD);

    /* Write set content to stream.
     */
    virtual eStatus writer(
        eStream *stream,
        os_int flags);

    /* Read set content from stream.
     */
    virtual eStatus reader(
        eStream *stream,
        os_int flags);

#if E_SUPPROT_JSON
    /* Called to check if object has class specific content. If there is no class
       specific JSON content, json_writer or json_reader should not be called.
     */
    virtual os_boolean has_json_content()
    {
        return OS_TRUE;
    }

    /* Write set to stream as JSON.
     */
    virtual eStatus json_writer(
        eStream *stream,
        os_int sflags,
        os_int indent);
#endif

    /**
    ************************************************************************************************

      @name Setting and getting item values.

      A value can be stored into set using set() function. If the value with this id exists,
      it is overwritten. Values are retrieved by get() function.

    ************************************************************************************************
    */
    /* Store variable value into set.
     */
    void setv(
        os_int id,
        eVariable *x,
        os_int sflags = 0);

    /* Store object into set.
     */
    void seto(
        os_int id,
        eObject *x,
        os_int sflags = 0);

    /* Store value into set.
     */
    inline void setl(
        os_int id,
        os_long x)
    {
        eVariable v;
        v.setl(x);
        setv(id, &v);
    }

    /* Store value into set.
     */
    inline void setd(
        os_int id,
        os_double x)
    {
        eVariable v;
        v.setd(x);
        setv(id, &v);
    }

    /* Store value into set (sflags may have ESET_STORE_AS_VARIABLE).
     */
    inline void sets(
        os_int id,
        const os_char *x,
        os_int sflags = 0)
    {
        eVariable v;
        v.sets(x);
        setv(id, &v, sflags|ESET_ADOPT_X_CONTENT);
    }

    /* Get value from set.
     */
    os_boolean getv(
        os_int id,
        eVariable *x,
        os_int *sflags = OS_NULL);

    /* Get value as integer.
     */
    inline os_int geti(
        os_int id)
    {
        eVariable v;
        getv(id, &v);
        return (os_int)v.getl();
    }

    /* Get value as integer.
     */
    inline os_long getl(
        os_int id)
    {
        eVariable v;
        getv(id, &v);
        return v.getl();
    }

    /* Get value as double.
     */
    inline os_double getd(
        os_int id)
    {
        eVariable v;
        getv(id, &v);
        return v.getd();
    }

    /* Get pointer to variable within the set, which is used to store a value.
     */
    eVariable *getv_ptr(
        os_int id,
        os_int *sflags = OS_NULL);

    /* Get pointer to value object.
     */
    eObject *geto_ptr(
        os_int id,
        os_int *sflags = OS_NULL);

    /* Get pointer to string value stored within a variable in set.
     */
    const os_char *gets_ptr(
        os_int id,
        os_int *sflags = OS_NULL);

    /* Clear the set.
     */
    void clear();


protected:

    /**
    ************************************************************************************************
      Member variables.
    ************************************************************************************************
    */
    /* Buffer containing items
     */
    os_uchar *m_items;

    /* Buffer used, bytes.
     */
    os_int m_used;

    /* Buffer allocated, bytes.
     */
    os_int m_alloc;
};

#endif
