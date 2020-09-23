/**

  @file    ecomponent.h
  @brief   Abstract GUI component.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  Base call for all GUI components (widgets)...

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef EIOC_CHECKBOX_H_
#define EIOC_CHECKBOX_H_
#include "egui.h"

/**
****************************************************************************************************

  @name Defines

  X...

****************************************************************************************************
*/
/*@{*/

/*@}*/


/**
****************************************************************************************************

  @brief eiocCheckbox class.

  The eiocCheckbox is dynamically typed variable, which can store integers, floating point values
  and strings.

****************************************************************************************************
*/
class eiocCheckbox : public eComponent
{
    /**
    ************************************************************************************************

      @name eiocCheckbox overrides for eObject base class functions.

      X...

    ************************************************************************************************
    */
    /*@{*/
public:
    /* Constructor.
     */
    eiocCheckbox(
        eObject *parent = OS_NULL,
        e_oid id = EOID_GUI_COMPONENT,
        os_int flags = EOBJ_DEFAULT);

    /* Virtual destructor.
     */
    virtual ~eiocCheckbox();

    /* Clone object.
     */
    virtual eObject *clone(
        eObject *parent,
        e_oid id = EOID_CHILD,
        os_int aflags = 0);

    /* Casting eObject pointer to eiocCheckbox pointer.
     */
    inline static eiocCheckbox *cast(
        eObject *o)
    {
        e_assert_type(o, EGUICLASSID_IOC_CHECKBOX)
        return (eiocCheckbox*)o;
    }

    /* Get class identifier and name.
     */
    virtual os_int classid() {return EGUICLASSID_IOC_CHECKBOX; }
    virtual const os_char *classname() {return "ioc_checkbox";}

    /* Static function to add class to propertysets and class list.
     */
    static void setupclass();

    /* Static constructor function for generating instance by class list.
     */
    static eiocCheckbox *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new eiocCheckbox(parent, id, flags);
    }

    /*@}*/


    /**
    ************************************************************************************************

      @name Base class functions to implement component functionality

      CreaSerialization means writing object to stream or reading it from strem.

    ************************************************************************************************
    */
    /*@{*/

    /* Draw the component.
     */
    virtual eStatus draw(
        eDrawParams& prm);

    /*@}*/


protected:
};


#endif
