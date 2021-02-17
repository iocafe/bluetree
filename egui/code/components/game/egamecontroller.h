/**

  @file    egamecontroller.h
  @brief   Display camera, etc, live bitmap based image.
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
#ifndef EGAMECONTROLLER_H_
#define EGAMECONTROLLER_H_
#include "egui.h"

/**
****************************************************************************************************
  eGameController class.
****************************************************************************************************
*/
class eGameController : public eComponent
{
    /**
    ************************************************************************************************
      Generic eObject functionality.
    ************************************************************************************************
    */
public:
    /* Constructor.
     */
    eGameController(
        eObject *parent = OS_NULL,
        e_oid id = EOID_GUI_COMPONENT,
        os_int flags = EOBJ_DEFAULT);

    /* Virtual destructor.
     */
    virtual ~eGameController();

    /* Clone object.
     */
    virtual eObject *clone(
        eObject *parent,
        e_oid id = EOID_CHILD,
        os_int aflags = 0);

    /* Casting eObject pointer to eGameController pointer.
     */
    inline static eGameController *cast(
        eObject *o)
    {
        e_assert_type(o, EGUICLASSID_GAME_CONTROLLER)
        return (eGameController*)o;
    }

    /* Get class identifier.
     */
    virtual os_int classid() {return EGUICLASSID_GAME_CONTROLLER; }

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
    static eGameController *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new eGameController(parent, id, flags);
    }


    /**
    ************************************************************************************************
      GUI component functionality (eComponent)
    ************************************************************************************************
    */
    /* Draw the component.
     */
    virtual eStatus draw(
        eDrawParams& prm);

    /* Process mouse click.
     */
    virtual os_boolean on_click(
        eDrawParams& prm,
        os_int mouse_gamecontroller_nr);

    /* Activate the component (start editing value, toggle checkbox, or show drop down list).
     */
    virtual void activate();


protected:

    /**
    ************************************************************************************************
      Protected functions
    ************************************************************************************************
    */

    /* Load a bitmap to the graphics card.
     */
    void upload_texture_to_grahics_card(
        eBitmap *bitmap);

    /* Delete a texture (bitmap) from graphics card.
     */
    void delete_texture_on_grahics_card();

    /**
    ************************************************************************************************
      Member variables.
    ************************************************************************************************
    */

    /** OpenGL, etc. texture ID.
     */
    ImTextureID m_textureID;

    /** Flag indicating that m_textureID idenfidies meaningfull texture.
     */
    os_boolean m_textureID_set;

    /** Texture size.
     */
    os_int m_texture_w, m_texture_h;

    eStrBuffer m_text;
};


#endif
