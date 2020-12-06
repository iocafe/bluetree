/**

  @file    econtainer.cpp
  @brief   Simple object container.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  The container object is like a box holding a set of child objects.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "eobjects.h"


/* Persistent object property names.
 */
const os_char
    eperp_root_dir[] = "rootdir",
    eperp_root_path[] = "rootpath",
    eperp_path[] = "path";

/**
****************************************************************************************************
  Constructor.
****************************************************************************************************
*/
ePersistent::ePersistent(
    eObject *parent,
    e_oid oid,
    os_int flags)
    : eContainer(parent, oid, flags)
{
    m_latest_touch = 0;
    m_oldest_touch = 0;

    initproperties();
}


/**
****************************************************************************************************
  Virtual destructor.
****************************************************************************************************
*/
ePersistent::~ePersistent()
{
}


/**
****************************************************************************************************

  @brief Clone object

  The ePersistent::clone function clones and object including object's children.
  Names will be left detached in clone.

  @param  parent Parent for the clone.
  @param  oid Object identifier for the clone.
  @param  aflags 0 for default operation. EOBJ_NO_MAP not to map names.
  @return Pointer to the clone.

****************************************************************************************************
*/
eObject *ePersistent::clone(
    eObject *parent,
    e_oid id,
    os_int aflags)
{
    eObject *clonedobj;
    clonedobj = new ePersistent(parent, id == EOID_CHILD ? oid() : id, flags());
    clonegeneric(clonedobj, aflags|EOBJ_CLONE_ALL_CHILDREN);
    return clonedobj;
}


/**
****************************************************************************************************

  @brief Add the class to class list and class'es properties to it's property set.

  The ePersistent::setupclass function adds the class to class list and class'es properties to
  it's property set. The class list enables creating new objects dynamically by class identifier,
  which is used for serialization reader functions. The property set stores static list of
  class'es properties and metadata for those.

****************************************************************************************************
*/
void ePersistent::setupclass()
{
    const os_int cls = ECLASSID_PERSISTENT;

    /* Add the class to class list.
     */
    os_lock();
    eclasslist_add(cls, (eNewObjFunc)newobj, "ePersistent");
    addpropertys(cls, EPERP_ROOT_DIR, eperp_root_dir, "/coderoot/fsys", "root dir", EPRO_PERSISTENT);
    addpropertys(cls, EPERP_ROOT_PATH, eperp_root_path, "//fsys", "root path", EPRO_PERSISTENT);
    addpropertys(cls, EPERP_PATH, eperp_path, "path", "unknown.eo", EPRO_PERSISTENT);
    propertysetdone(cls);
    os_unlock();
}


/**
****************************************************************************************************

  @brief Mark the peristent object changed.

  The ePersistent::touch function adds the class to class list and class'es properties to
  it's property set. The class list enables creating new objects dynamically by class identifier,
  which is used for serialization reader functions. The property set stores static list of
  class'es properties and metadata for those.

****************************************************************************************************
*/
void ePersistent::touch()
{
    osal_get_timer(&m_latest_touch);
    if (m_oldest_touch == 0) {
        m_oldest_touch = m_latest_touch;
    }
}
