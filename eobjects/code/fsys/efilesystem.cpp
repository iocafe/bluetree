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
    const os_int cls = ECLASSID_FILE_SYSTEM;

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

  @brief Process an incoming message.

  The eFileSystem::onmessage function handles messages received by object. This function
  checks for commands related to file system.

  @param   envelope Message envelope. Contains command, target and source paths and
           message content, etc.
  @return  None.

****************************************************************************************************
*/
void eFileSystem::onmessage(
    eEnvelope *envelope)
{
    eThread::onmessage(envelope);
}




/**
****************************************************************************************************

  @brief List names in this object's namespace. Here we list files and folders.

  The eFileSystem::browse_list_namespace function lists named children, grandchildren, etc,
  when name is mapped to name space of this object. Each list item is a variable.

  @param   content Pointer to container into which to place list items.
  @param   target When browsing structure which is not made out of eObjects,
           this can be path within the object (like file system, etc).
  @param   None.

****************************************************************************************************
*/
void eFileSystem::browse_list_namespace(
    eContainer *content,
    const os_char *target)
{
    eVariable *item;
    eSet *appendix;
    os_char buf[E_OIXSTR_BUF_SZ];
    osalDirListItem *list, *listitem;
    osalStatus s;

    s = osal_dir("/coderoot", "*", &list, OSAL_DIR_FILESTAT);
    if (s) {
        osal_debug_error("osal_dir failed");
        return;
    }

    oixstr(buf, sizeof(buf));

    for (listitem = list; listitem; listitem = listitem->next)
    {
        item = new eVariable(content, EBROWSE_NSPACE);
        appendix = new eSet(item, EOID_APPENDIX, EOBJ_IS_ATTACHMENT);
        appendix->sets(EBROWSE_PATH, listitem->name);

        /** Get oix and ucnt as string.
         */
        appendix->sets(EBROWSE_IPATH, listitem->name);

        item->setpropertys(EVARP_TEXT, listitem->name);

    }

    /* Release directory list from memory.
     */
    osal_free_dirlist(list);
}


/**
****************************************************************************************************

  @brief Collect information about a file or folder.

  The eFileSystem::object_info function fills in item (eVariable) to contain information
  about this object for tree browser view.

  @param   item Pointer to eVariable to set up with object information.
  @param   name Object's name if known. OS_NULL if object is not named or name is
           unknown at this time.
  @param   appendix Pointer to eSet into which to store property flags. The stored property
           flags indicate if object has namespace, children, or properties.

****************************************************************************************************
*/
void eFileSystem::object_info(
    eVariable *item,
    eVariable *name,
    eSet *appendix)
{
    // eVariable value;

    eObject::object_info(item, name, appendix);

    /* propertyv(ECOMP_TEXT, &value);
    if (!value.isempty()) {
        eVariable value2;
        value2 += "\"";
        value2 += value;
        value2 += "\" ";
        item->propertyv(EVARP_TEXT, &value);
        value2 += value;
        item->setpropertyv(EVARP_TEXT, &value2);
    } */
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
