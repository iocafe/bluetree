/**

  @file    elineedit.h
  @brief   Line edit with label and value.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    15.9.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef ELINEEDIT_H_
#define ELINEEDIT_H_
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

  @brief eLineEdit class.

  The eLineEdit is dynamically typed variable, which can store integers, floating point values
  and strings.

****************************************************************************************************
*/
class eLineEdit : public eComponent
{
    /**
    ************************************************************************************************

      @name eLineEdit overrides for eObject base class functions.

      X...

    ************************************************************************************************
    */
    /*@{*/
public:
    /* Constructor.
     */
    eLineEdit(
        eObject *parent = OS_NULL,
        e_oid id = EOID_GUI_COMPONENT,
        os_int flags = EOBJ_DEFAULT);

    /* Virtual destructor.
     */
    virtual ~eLineEdit();

    /* Clone object.
     */
    virtual eObject *clone(
        eObject *parent,
        e_oid id = EOID_CHILD,
        os_int aflags = 0);

    /* Casting eObject pointer to eLineEdit pointer.
     */
    inline static eLineEdit *cast(
        eObject *o)
    {
        e_assert_type(o, EGUICLASSID_LINE_EDIT)
        return (eLineEdit*)o;
    }

    /* Get class identifier.
     */
    virtual os_int classid() {return EGUICLASSID_LINE_EDIT; }

    /* Static function to add class to propertysets and class list.
     */
    static void setupclass();

    /* Called when property value changes.
     */
    virtual eStatus onpropertychange(
        os_int propertynr,
        eVariable *x,
        os_int flags);

    /* Static constructor function for generating instance by class list.
     */
    static eLineEdit *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new eLineEdit(parent, id, flags);
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
    bool m_edit_value;
    bool m_prev_edit_value;

    /* Buffer for editing value.
     */
    eEditBuffer m_edit_buf;

    /* Property values */
    eStrBuffer m_text;
    eStrBuffer m_unit;

    /* Parsed attrbutes, digits, etc.
     */
    eAttrBuffer m_attr;

    eAutoLabel m_label_edit;
    eAutoLabel m_label_value;
};


#endif
