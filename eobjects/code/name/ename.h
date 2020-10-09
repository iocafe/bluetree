/**

  @file    ename.h
  @brief   Name class.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  The names map to name space. These are used to represent tree structure of named objects.
  The eName class is derived from eVariable class.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef ENAME_H_
#define ENAME_H_
#include "eobjects.h"

/* Flags for addname()
 */
#define ENAME_PERSISTENT 0
#define ENAME_TEMPORARY 1
#define ENAME_PARENT_NS 0
#define ENAME_PROCESS_NS 2
#define ENAME_THREAD_NS 4
#define ENAME_SPECIFIED_NS 8
#define ENAME_NO_NS 16
#define ENAME_THIS_NS 32
#define ENAME_NO_MAP 64


/**
****************************************************************************************************

  @brief eName class.

  The eName is a class derived from eVariable. It adds capability to map to eNameSpace.

****************************************************************************************************
*/
class eName : public eVariable
{
    friend class eNameSpace;

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
    eName(
        eObject *parent = OS_NULL,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT);

    /* Virtual destructor.
     */
    virtual ~eName();

    /* Clone the eName.
     */
    virtual eObject *clone(
        eObject *parent,
        e_oid id = EOID_CHILD,
        os_int aflags = 0);

    /* Cast eObject pointer to eName pointer.
     */
    inline static eName *cast(
        eObject *o)
    {
        e_assert_type(o, ECLASSID_NAME)
        return (eName*)o;
    }

    /* Get class identifier.
     */
    virtual os_int classid() {return ECLASSID_NAME;}

    /* Static function to add class to propertysets and class list.
     */
    static void setupclass();

    /* Static constructor function for generating instance by class list.
     */
    static eName *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new eName(parent, id, flags);
    }

    /* Get next child name identified by oid.
     */
    eName *nextn(
        e_oid id = EOID_NAME);

    /* Get next name in namespace.
     */
    eName *ns_next(
        os_boolean name_match = OS_TRUE);

    eName *ns_prev(
        os_boolean name_match = OS_TRUE);

    /** Get name space to which this name is mapped to.
     */
    inline eNameSpace *nspace()
    {
        return m_namespace;
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

      @name Name specific functions

      X...

    ************************************************************************************************
    */

    /* Get name space identifier, if any, for the name.
     */
    const os_char *namespaceid();

    /* Set name space identifier, if any, for the name.
     */
    void setnamespaceid(
        const os_char *namespace_id);

    /* Map the name to a name space.
     */
    eStatus mapname();

    /* Map the name to a name space given as argument.
     */
    eStatus mapname2(
        eNameSpace *ns,
        os_int info);

    /* Detach name from name space.
     */
    void detach();

    /* Is name is mapped to name space?
     */
    inline bool is_mapped() {return m_namespace != OS_NULL; }

protected:
    /* Collect information about this object for tree browser.
     */
    virtual void object_info(
        eVariable *item,
        eVariable *name,
        eSet *appendix);

    void clear_members();

    /** Tag this object as "red".
     */
    inline void ixsetred()
    {
        m_vflags |= EVAR_IS_RED;
    }

    /** Tag this object as "black".
     */
    inline void ixsetblack()
    {
        m_vflags &= ~EVAR_IS_RED;
    }

    /** Namespace type (max one bit set). Bits: ENAME_PARENT_NS, ENAME_PROCESS_NS,
        ENAME_THREAD_NS, ENAME_SPECIFIED_NS, ENAME_NO_NS, ENAME_THIS_NS.
     */
    os_int m_ns_type;

    /** True if the name is mapped to the process name space.
     */
    os_boolean m_is_process_ns;

    /** Name space identifier string when m_ns_type is ENAME_SPECIFIED_NS.
     */
    eVariable *m_namespace_id;

    /** Pointer to left child in index'es red/black tree.
     */
    eName *m_ileft;

    /** Pointer to right child in index'es red/black tree.
     */
    eName *m_iright;

    /** Pointer to parent in index'es red/black tree.
     */
    eName *m_iup;

    /** Pointer to index.
     */
    eNameSpace *m_namespace;
};

#endif
