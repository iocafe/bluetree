/**

  @file    efilesystem.cpp
  @brief   enet service implementation.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

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
eFileSystem::eFileSystem(
    eObject *parent,
    e_oid oid,
    os_int flags)
    : eThread(parent, oid, flags)
{
    initproperties();
}


/**
****************************************************************************************************
  Virtual destructor.
****************************************************************************************************
*/
eFileSystem::~eFileSystem()
{
}


/**
****************************************************************************************************

  @brief Clone object

  The eFileSystem::clone function clones and object including object's children.
  Names will be left detached in clone.

  @param  parent Parent for the clone.
  @param  oid Object identifier for the clone.
  @param  aflags 0 for default operation. EOBJ_NO_MAP not to map names.
  @return Pointer to the clone.

****************************************************************************************************
*/
eObject *eFileSystem::clone(
    eObject *parent,
    e_oid id,
    os_int aflags)
{
    eObject *clonedobj;
    clonedobj = new eFileSystem(parent, id == EOID_CHILD ? oid() : id, flags());
    clonegeneric(clonedobj, aflags|EOBJ_CLONE_ALL_CHILDREN);
    return clonedobj;
}


/**
****************************************************************************************************

  @brief Add the class to class list and class'es properties to it's property set.

  The eVariable::setupclass function adds the class to class list and class'es properties to
  it's property set. The class list enables creating new objects dynamically by class identifier,
  which is used for serialization reader functions. The property set stores static list of
  class'es properties and metadata for those.

****************************************************************************************************
*/
void eFileSystem::setupclass()
{
    const os_int cls = ECLASSID_CONTAINER;

    /* Add the class to class list.
     */
    os_lock();
    eclasslist_add(cls, (eNewObjFunc)newobj, "eFileSystem");
    os_unlock();
}

void eFileSystem::initialize(
    eContainer *params)
{
//    ns_create();
}


/**
****************************************************************************************************

  @brief Expose OS directory as object tree.

  @param fsys_name Name for the eFileSystem object, typically "//fsys".
  @param os_path Operating system path to directory to expose.
  @param fsys_thread_handle Handle of thread running file system is set here.

****************************************************************************************************
*/
void efsys_expose_directory(
    const os_char *fsys_name,
    const os_char *os_path,
    eThreadHandle *fsys_thread_handle)
{
    eFileSystem *fsys;

    /* Create and start net service thread to listen for incoming socket connections,
       name it "//service".
     */
    fsys = new eFileSystem();
    fsys->addname(fsys_name);
    fsys->start(fsys_thread_handle);
}
