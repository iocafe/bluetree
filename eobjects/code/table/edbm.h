/**

  @file    edbm.h
  @brief   Rowset class.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    15.10.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef EDBM_H_
#define EDBM_H_
#include "eobjects.h"

/**
****************************************************************************************************
  Defines
****************************************************************************************************
*/



/**
****************************************************************************************************
  DBM class.
****************************************************************************************************
*/
class eDBM : public eObject
{
    /**
    ************************************************************************************************

      @name Generic object functionality.

      These functions enable using objects of this class as generic eObjects.

    ************************************************************************************************
    */
    /*@{*/
public:
    /* Constructor.
     */
    eDBM(
        eObject *parent = OS_NULL,
        e_oid id = EOID_RITEM,
        os_int flags = EOBJ_DEFAULT);

    /* Virtual destructor.
     */
    virtual ~eDBM();

    /* Clone object.
     */
    virtual eObject *clone(
        eObject *parent,
        e_oid id = EOID_CHILD,
        os_int aflags = 0);

    /* Casting eObject pointer to eDBM pointer.
     */
    inline static eDBM *cast(
        eObject *o)
    {
        e_assert_type(o, ECLASSID_DBM)
        return (eDBM*)o;
    }

    /* Get class identifier.
     */
    virtual os_int classid() {return ECLASSID_DBM; }

    /* Static function to add class to propertysets and class list.
     */
    static void setupclass();

    /* Static constructor function for generating instance by class list.
     */
    static eDBM *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new eDBM(parent, id, flags);
    }

    /*@}*/


    /**
    ************************************************************************************************

      @name DBM functions.

      These function treat dbm as organized to rows and columns.

    ************************************************************************************************
    */
    /*@{*/

    /*@}*/


protected:
    /**
    ************************************************************************************************

      @name Internal to DBM.

      Protected functions and member variables.

    ************************************************************************************************
    */
    /*@{*/


    /*@}*/
};

#endif
