/**

  @file    econtainer.h
  @brief   Simple object container.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

  The container object is like a box holding a set of child objects.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef ECONTAINER_H_
#define ECONTAINER_H_
#include "eobjects.h"

/* eContainer property numbers.
 */
#define ECONTP_TEXT 5

/* eContainer property names.
 */
extern const os_char
    econtp_text[];

/**
****************************************************************************************************
  eContainer is like a box of objects.
****************************************************************************************************
*/
class eContainer : public eObject
{
public:
    /* Constructor.
     */
    eContainer(
        eObject *parent = OS_NULL,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT);

    /* Virtual destructor.
     */
    virtual ~eContainer();

    /* Clone object.
     */
    virtual eObject *clone(
        eObject *parent,
        e_oid id = EOID_CHILD,
        os_int aflags = 0);

    /* Casting eObject pointer to eContainer pointer.
     */
    inline static eContainer *cast(
        eObject *o)
    {
        e_assert_type(o, ECLASSID_CONTAINER)
        return (eContainer*)o;
    }

    /* Get class identifier.
     */
    virtual os_int classid() {return ECLASSID_CONTAINER; }

    /* Static function to add class to propertysets and class list.
     */
    static void setupclass();

    /* Static constructor function for generating instance by class list.
     */
    static eContainer *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new eContainer(parent, id, flags);
    }

    /* Get next child container identified by oid.
     */
    eContainer *nextc(
        e_oid id = EOID_CHILD);

    /* Write container content to stream.
     */
    virtual eStatus writer(
        eStream *stream,
        os_int flags);

    /* Read container content from stream.
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

    /* Write container specific content to stream as JSON.
     */
    virtual eStatus json_writer(
        eStream *stream,
        os_int sflags,
        os_int indent);
#endif

    /* Empty the container.
     */
    void clear();


protected:
    /**
    ************************************************************************************************
      Protected functions.
    ************************************************************************************************
    */

    /* Collect information about this object for tree browser, etc.
     */
    virtual void object_info(
        eVariable *item,
        eVariable *name,
        eSet *appendix,
        const os_char *target);

    /* Information for opening object has been requested, send it.
     */
    virtual void send_open_info(
        eEnvelope *envelope);

    /* Helper function for send_open_info().
     */
    void send_open_info_helper(
        eContainer *reply);


};

#endif
