/**

  @file    efilesystem.cpp
  @brief   File system as object tree.
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

/* File system property names.
 */
const os_char
    efsysp_path[] = "path";

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
    m_path = new eVariable(this);
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
    eclasslist_add(cls, (eNewObjFunc)newobj, "eFileSystem", ECLASSID_THREAD);
    addpropertys(cls, EFSYSP_PATH, efsysp_path, "/coderoot/fsys", "path", EPRO_SIMPLE);
    propertysetdone(cls);
    os_unlock();
}

void eFileSystem::initialize(
    eContainer *params)
{
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
    os_char *target, c;
    os_int command;

    target = envelope->target();
    c = *target;

    /* Process commands to files and foldes */
    if (c != '\0' && c != '_' && c != '@')
    {
        command = envelope->command();
        switch (command)
        {
          case ECMD_INFO_REQUEST:
            send_browse_info(envelope);
            return;

          case ECMD_SAVE_FILE:
            save_file(envelope);
            return;
        }
    }

    eThread::onmessage(envelope);
}

/**

****************************************************************************************************

  @brief Called to inform the class about property value change (override).

  The onpropertychange() function is called when class'es property changes, unless the
  property is flagged with EPRO_NOONPRCH.

  @param   propertynr Property number of changed property.
  @param   x Variable containing the new value.
  @param   flags
  @return  If successfull, the function returns ESTATUS_SUCCESS (0). Nonzero return values do
           indicate that there was no property with given property number.

****************************************************************************************************
*/
eStatus eFileSystem::onpropertychange(
    os_int propertynr,
    eVariable *x,
    os_int flags)
{
    switch (propertynr)
    {
        case EFSYSP_PATH: /* Read only for sandbox security */
            break;

        default:
            return eObject::onpropertychange(propertynr, x, flags);
    }

    return ESTATUS_SUCCESS;
}


/**
****************************************************************************************************

  @brief Get value of simple property (override).

  The simpleproperty() function stores current value of simple property into variable x.

  @param   propertynr Property number to get.
  @param   x Variable into which to store the property value.
  @return  If property with property number was stored in x, the function returns
           ESTATUS_SUCCESS (0). Nonzero return values indicate that property with
           given number was not among simple properties.

****************************************************************************************************
*/
eStatus eFileSystem::simpleproperty(
    os_int propertynr,
    eVariable *x)
{
    switch (propertynr)
    {
        case EFSYSP_PATH:
            m_path->setv(m_path);
            break;

        default:
            return eObject::simpleproperty(propertynr, x);
    }
    return ESTATUS_SUCCESS;
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
    eVariable tmp, *item;
    eSet *appendix;
    eContainer *tmp_content;
    eName *name;
    eObject *obj;
    os_char buf[E_OIXSTR_BUF_SZ], nbuf[OSAL_NBUF_SZ], c;
    osalDirListItem *list, *listitem;
    osalStatus s;

    tmp.setv(m_path);
    tmp += "/";
    tmp += target;

    s = osal_dir(tmp.gets(), "*", &list, OSAL_DIR_FILESTAT);
    if (s) {
        osal_debug_error("osal_dir failed");
        return;
    }

    oixstr(buf, sizeof(buf));

    tmp_content = new eContainer(ETEMPORARY);
    tmp_content->ns_create();

    for (listitem = list; listitem; listitem = listitem->next)
    {
        item = new eVariable(tmp_content, EBROWSE_NSPACE);
        appendix = new eSet(item, EOID_APPENDIX, EOBJ_IS_ATTACHMENT);
        appendix->sets(EBROWSE_PATH, listitem->name);
        appendix->sets(EBROWSE_ITEM_TYPE, listitem->isdir ? "d" : "f");
        appendix->sets(EBROWSE_IPATH, listitem->name);
        item->setpropertys(EVARP_TEXT, listitem->name);

        if (!listitem->isdir) {
            etime_timestamp_str(listitem->tstamp, &tmp, ETIMESTR_SECONDS);
            if (!tmp.isempty()) {
                tmp += ", ";
                item->setpropertyl(EVARP_TYPE, OS_STR);
            }
            osal_int_to_str(nbuf, sizeof(nbuf), listitem->sz);
            tmp += nbuf;
            item->setpropertyv(EVARP_VALUE, &tmp);
            item->setpropertys(EVARP_UNIT, "bytes");
        }

        /* Add name for sorting
         */
        tmp.sets(listitem->isdir ? "a" : "b");
        c = listitem->name[0];
        tmp.appends(listitem->name);
        tmp.appends(osal_char_isupper(c) ? "a" : "b");
        tmp.tolower();
        item->addname(tmp.gets());
    }

    /* Copy in sort order.
     */
    while ((name = tmp_content->ns_first())) {
        obj = name->parent();
        delete name;
        obj->adopt(content);
    }

    delete tmp_content;

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
  @param   target Path "within object" when browsing a tree which is not made out
           of actual eObjects. For example OS file system directory.

****************************************************************************************************
*/
void eFileSystem::object_info(
    eVariable *item,
    eVariable *name,
    eSet *appendix,
    const os_char *target)
{
    const os_char *fname_only;

    if (*target == '\0') {
        eThread::object_info(item, name, appendix, target);
    }
    else
    {
        /* Get file or directory name without path.
         */
        fname_only = os_strechr((os_char*)target, '/');
        if (fname_only) {
            fname_only++;
        }
        else {
            fname_only = target;
        }

        item->setpropertys(EVARP_TEXT, fname_only);

        os_int browse_flags = EBROWSE_NSPACE;
        appendix->setl(EBROWSE_BROWSE_FLAGS, browse_flags);
    }
}


/**
****************************************************************************************************

  @brief Save envelope content as a file (binary serialization).

  @param envelope Message envelope received by the eFileSystem object. This is eContainer
         which holds eVariable for path and eObject for content.

****************************************************************************************************
*/
void eFileSystem::save_file(
    eEnvelope *envelope)
{
    eObject *c, *file_content;
    eVariable file_path, *relative_path;
    const os_char *p;
    eStatus s = ESTATUS_FAILED;

    /* Get path to eFileSystem root directory. If path doesn't end with '/', append one.
     */
    file_path.setv(m_path);
    p = os_strechr(file_path.gets(), '/');
    if (p) if (p[1] != '\0') {
        file_path.appends("/");
    }

    /* Get file content and relative path from message content. Save to file.
     */
    c = envelope->content();
    if (c) {
        relative_path = eVariable::cast(c->first(EOID_PATH));

        if (relative_path) {
            file_path.appendv(relative_path);
            file_content = c->first(EOID_CONTENT);
            if (file_content) {
                s = file_content->save(file_path.gets());
            }
        }
    }

    /* Report error or success
     */
    if (s) {
        eVariable message;
        message.sets("Saving \'");
        message.appendv(&file_path);
        message.appends("\' failed.");
        reply(ECMD_ERROR, envelope, message.gets());
    }
    else {
        reply(ECMD_ERROR, envelope);
    }
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
    // NOW REfsys->setpropertys(EFSYSP_PATH, os_path);
    fsys->set_os_path(os_path);
    fsys->start(fsys_thread_handle);
}

