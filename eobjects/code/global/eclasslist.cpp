/**

  @file    eclasslist.cpp
  @brief   Class list.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

  Maintain list of classes which can be created dynamically by class ID.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "eobjects.h"



/**
****************************************************************************************************

  @brief Add a class to class list.

  Class list contains pointers to static constructor for the class by class ID index.
  It is used to constructr objects so that class can be a parameter.

  @return  None.

****************************************************************************************************
*/
void eclasslist_add(
    os_int cid,
    eNewObjFunc nfunc,
    const os_char *classname,
    os_int baseclass_id)
{
    eVariable *pointer, *baseclass_ix;

    /* Syncronization neeeded for eclasslist_add() function.
     */
    os_lock();

#if OSAL_DEBUG
    /* Check for duplicated calls with same cid.
     */
    pointer = eglobal->classlist->firstv(cid);
    if (pointer)
    {
        osal_debug_error("eclasslist_add() called with same cid twice");
        goto getout;
    }
#endif
    /* Store pointer to class'es newobj() function and name it.
     */
    pointer = new eVariable(eglobal->classlist, cid);
    pointer->setp((os_pointer)nfunc);
    pointer->addname(classname);

    if (baseclass_id != ECLASSID_OBJECT) {
        baseclass_ix = new eVariable(pointer, EOID_APPENDIX, EOBJ_IS_ATTACHMENT);
        baseclass_ix->setl(baseclass_id);
    }

getout:
    /* Finished with synchronization.
     */
    os_unlock();
}


/**
****************************************************************************************************

  @brief Get static constuctor function pointer by class ID.

  The eclasslist_newobj function...

  @param   cid Class identifier to look for.
  @return  Pointer to static constructor function, or OS_NULL if none found.

****************************************************************************************************
*/
eNewObjFunc eclasslist_newobj(
    os_int cid)
{
    eVariable *pointer;
    eNewObjFunc nfunc;

    nfunc = OS_NULL;
    os_lock();

    pointer = eglobal->classlist->firstv(cid);
    if (pointer)
    {
        nfunc = (eNewObjFunc)pointer->getp();
    }
#if OSAL_DEBUG
    else
    {
        osal_debug_error_int("eclasslist_newobj: Class not found, cid=", cid);
    }
#endif

    os_unlock();
    return nfunc;
}


/**
****************************************************************************************************

  @brief Check if a class is derived from a base class, or instance of the base class.

  The eclasslist_isinstanceof function check is this_cid identifies a class derived from
  base_class_cid, or is simply instance of base_class_cid.

  @param   this_cid Class identifier to check.
  @param   base_class_cid Base class identifier.

  @return  The function returns OS_TRUE, if class idenfified by this_cid is derived from
           base class or instance of the base class. If not, the function returns OS_FALSE (0).

****************************************************************************************************
*/
os_boolean eclasslist_isinstanceof(
    os_int this_cid,
    os_int base_class_cid)
{
    eVariable *pointer, *appendix;
    os_boolean rval;

    if (this_cid == base_class_cid || base_class_cid == ECLASSID_OBJECT) {
        return OS_TRUE;
    }

    os_lock();

    rval = OS_FALSE;
    while (OS_TRUE) {
        pointer = eglobal->classlist->firstv(this_cid);
        if (pointer == OS_NULL) {
            osal_debug_error_int("eclasslist_isinstanceof: Class not found, cid=", this_cid);
            break;
        }

        appendix = pointer->firstv(EOID_APPENDIX);
        if (appendix == OS_NULL) {
            break;
        }

        this_cid = appendix->geti();
        if (this_cid == base_class_cid) {
            rval = OS_TRUE;
            break;
        }
    }

    os_unlock();
    return rval;
}


/**
****************************************************************************************************

  @brief Get class name.

  The eclasslist_classname function returns clas name matching to class id (cid).

  @param   cid Class identifier to look for.
  @return  Class name, or OS_NULL if none found.

****************************************************************************************************
*/
os_char *eclasslist_classname(
    os_int cid)
{
    eVariable *pointer;
    eName *name;
    os_char *namestr = OS_NULL;

    os_lock();
    pointer = eglobal->classlist->firstv(cid);
    if (pointer)
    {
        name = pointer->firstn(EOID_NAME);
        if (name) namestr = name->gets();
    }
#if OSAL_DEBUG
    else
    {
        osal_debug_error_int("eclasslist_classname: Class not found, cid=", cid);
    }
#endif

    os_unlock();
    return namestr;
}


/**
****************************************************************************************************

  @brief Initialize class list and property sets.

  The eclasslist_initialize function must be called before any objects are created.

****************************************************************************************************
*/
void eclasslist_initialize()
{
    eglobal->root = new eContainer();
    eglobal->classlist = new eContainer(eglobal->root);
    eglobal->classlist->ns_create();

    eglobal->propertysets = new eContainer(eglobal->root);
    eglobal->empty = new eVariable();

    /* eVariable should be first to add to class list followed by then eSet and eContainer.
       Reason is that these same classes are used to store description of classes, including
       themselves.
     */
    eVariable::setupclass();
    eSet::setupclass();
    eContainer::setupclass();
    ePersistent::setupclass();
    eName::setupclass();
    eNameSpace::setupclass();
    eValueX::setupclass();
    ePointer::setupclass();
    eEnvelope::setupclass();
    eBinding::setupclass();
    ePropertyBinding::setupclass();
    eRowSetBinding::setupclass();
    eSynchronized::setupclass();
    eSyncConnector::setupclass();
    eTimer::setupclass();
    eQueue::setupclass();
    eBuffer::setupclass();
    eTable::setupclass();
    eMatrix::setupclass();
    eRowSet::setupclass();
    eDBM::setupclass();
    eBitmap::setupclass();
    eConnection::setupclass();
    eEndPoint::setupclass();
    eThread::setupclass();
    eProcess::setupclass();
    eRoot::setupclass();
    eStream::setupclass();
    eOsStream::setupclass();
    eFileSystem::setupclass();
}


/**
****************************************************************************************************

  @brief Free memory allocated for class list, property sets, etc.

  The eclasslist_release function should be called when all thread except current one have
  been terminated.

****************************************************************************************************
*/
void eclasslist_release()
{
    delete eglobal->root;
    delete eglobal->empty;
}
