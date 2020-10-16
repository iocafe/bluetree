/**

  @file    edbm.cpp
  @brief   DBM class.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    15.10.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "eobjects.h"


/**
****************************************************************************************************
  Constructor.
****************************************************************************************************
*/
eDBM::eDBM(
    eObject *parent,
    e_oid id,
    os_int flags)
    : eObject(parent, id, flags)
{
}


/**
****************************************************************************************************
  Virtual destructor.
****************************************************************************************************
*/
eDBM::~eDBM()
{
}


/**
****************************************************************************************************

  @brief Add eDBM to class list.

  The eDBM::setupclass function adds newobj function pointer to class list. This enables creating
  new objects dynamically by class identifier, which is used for serialization reader()
  functions.

  This needs to be called after eBuffer:setupclass(), there is a dependency in setup.

****************************************************************************************************
*/
void eDBM::setupclass()
{
    const os_int cls = ECLASSID_DBM;

    /* Add the class to class list.
     */
    os_lock();
    eclasslist_add(cls, (eNewObjFunc)newobj, "eDBM");
    propertysetdone(cls);
    os_unlock();
}


/**
****************************************************************************************************

  @brief Clone object

  The eDBM::clone function clones an eDBM.

  @param  parent Parent for the clone.
  @param  id Object identifier for the clone.
  @param  aflags 0 for default operation. EOBJ_NO_MAP not to map names.
  @return Pointer to the clone.

****************************************************************************************************
*/
eObject *eDBM::clone(
    eObject *parent,
    e_oid id,
    os_int aflags)
{
    eDBM *clonedobj;

    clonedobj = new eDBM(parent, id == EOID_CHILD ? oid() : id, flags());

    clonegeneric(clonedobj, aflags);
    return clonedobj;
}
