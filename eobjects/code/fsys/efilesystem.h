/**

  @file    efilesystem.h
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
#pragma once
#ifndef EFILESYSTEM_H_
#define EFILESYSTEM_H_
#include "eobjects.h"


/**
****************************************************************************************************
  Defines
****************************************************************************************************
*/

/* File system property numbers.
 */
#define EFSYSP_PATH 10

/* File system property names.
 */
extern const os_char
    efsysp_path[];


/**
****************************************************************************************************
  eFileSystem class.
****************************************************************************************************
*/
class eFileSystem : public eThread
{
public:
    /* Constructor.
     */
    eFileSystem(
        eObject *parent = OS_NULL,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT);

    /* Virtual destructor.
     */
    virtual ~eFileSystem();

    /* Clone object.
     */
    virtual eObject *clone(
        eObject *parent,
        e_oid id = EOID_CHILD,
        os_int aflags = 0);

    /* Casting eObject pointer to eFileSystem pointer.
     */
    inline static eFileSystem *cast(
        eObject *o)
    {
        e_assert_type(o, ECLASSID_FILE_SYSTEM)
        return (eFileSystem*)o;
    }

    /* Get class identifier.
     */
    virtual os_int classid() {return ECLASSID_FILE_SYSTEM; }

    /* Static function to add class to propertysets and class list.
     */
    static void setupclass();

    /* Static constructor function for generating instance by class list.
     */
    static eFileSystem *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new eFileSystem(parent, id, flags);
    }

    virtual void initialize(
        eContainer *params = OS_NULL);

    /* Process an incoming message.
     */
    virtual void onmessage(
        eEnvelope *envelope);

    /* Called when property value changes.
     */
    virtual eStatus onpropertychange(
        os_int propertynr,
        eVariable *x,
        os_int flags);

    virtual void browse_list_namespace(
        eContainer *content,
        const os_char *target);

    virtual void object_info(
        eVariable *item,
        eVariable *name,
        eSet *appendix,
        const os_char *target);

protected:
    /**
    ************************************************************************************************
      Internal functions.
    ************************************************************************************************
    */



    /**
    ************************************************************************************************
      Member variables
    ************************************************************************************************
    */

    /* OS path to root directory
     */
    eVariable *m_path;
};

/* Expose OS directory as object tree.
 */
void efsys_expose_directory(
    const os_char *fsys_name,
    const os_char *os_path,
    eThreadHandle *fsys_thread_handle);

#endif
