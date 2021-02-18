/**

  @file    eguix.h
  @brief   GUI root object.
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
#ifndef EGUIX_H_
#define EGUIX_H_
#include "egui.h"

/**
****************************************************************************************************
  Defines
****************************************************************************************************
*/

/* Enumeration of GUI object properties.
 */
#define EGUIP_TEXT 1
#define EGUIP_OPEN 20

/* GUI property names.
 */
extern const os_char
    eguip_text[],
    eguip_open[];


typedef struct eguiMouseState
{
    ePos down_pos[EIMGUI_NRO_MOUSE_BUTTONS];
    os_boolean is_down[EIMGUI_NRO_MOUSE_BUTTONS];
    os_boolean was_down[EIMGUI_NRO_MOUSE_BUTTONS];
    os_boolean is_dragging[EIMGUI_NRO_MOUSE_BUTTONS];
    os_boolean was_dragging[EIMGUI_NRO_MOUSE_BUTTONS];
    os_boolean held_still[EIMGUI_NRO_MOUSE_BUTTONS];
    os_boolean keyboard_flags[EIMGUI_NRO_MOUSE_BUTTONS];
}
eguiMouseState;



/**
****************************************************************************************************
  The eGui is root of graphical user interface.
****************************************************************************************************
*/
class eGui : public eObject
{
    friend class eComponent;

public:

    /**
    ************************************************************************************************
      Generic eObject functionality.
    ************************************************************************************************
    */
    /* Constructor.
     */
    eGui(
        eObject *parent = OS_NULL,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT);

    /* Virtual destructor.
     */
    virtual ~eGui();

    /* Clone object.
     */
    virtual eObject *clone(
        eObject *parent,
        e_oid id = EOID_CHILD,
        os_int aflags = 0);

    /* Casting eObject pointer to eGui pointer.
     */
    inline static eGui *cast(
        eObject *o)
    {
        e_assert_type(o, EGUICLASSID_GUI)
        return (eGui*)o;
    }

    /* Get class identifier.
     */
    virtual os_int classid() {return EGUICLASSID_GUI; }

    /* Static function to add class to propertysets and class list.
     */
    static void setupclass();

    /* Function to process incoming messages.
     */
    virtual void onmessage(
        eEnvelope *envelope);

    /* Generating ImGui autolabel.
     */
    void make_autolabel(
        eComponent *c,
        eVariable *name,
        eVariable *imgui_name);

    void release_autolabel(
        const os_char *imgui_name);

    /* Static constructor function for generating instance by class list.
     */
    static eGui *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new eGui(parent, id, flags);
    }

    /* Get the first child component identified by oid.
     */
    eComponent *firstcomponent(
        e_oid id = EOID_CHILD);

    /* Called when property value changes.
     */
    virtual eStatus onpropertychange(
        os_int propertynr,
        eVariable *x,
        os_int flags);

    /* Get value of simple property.
     */
    virtual eStatus simpleproperty(
        os_int propertynr,
        eVariable *x);


    /* Message to or trough this object.
     */
    /* virtual void onmessage(); */


    /**
    ************************************************************************************************
      Top level GUI functions
    ************************************************************************************************
    */
    /* Run GUI.
     */
    eStatus run();

    /* Determine size, position of component and it's subcomponents. Setup Z order for draing.
     */
    /* eStatus layout(
        eRect& r,
        eLayoutParams& prm); */

    /* Draw the GUI.
     */
    eStatus draw(
        eDrawParams& prm,
        os_int flags);


    /**
    ************************************************************************************************
      eGui functions called from components
    ************************************************************************************************
    */

    /* Set redo layout flag.
     */
    // void redo_layout();

    /* Invalidate specified rectangle.
     */
    void invalidate(eRect& r);

    /* Set keyboard input focus to this component.
     */
    void focus();

    /* Set GUI component to use as drag origin. We copy, move or modify a component.
     * We may also copy/mode/modify selected components.
     */
    void save_drag_origin(eComponent *c,
        eGuiDragMode drag_mode);

    /* Get drag origin component.
     */
    eComponent *get_drag_origin();

    /* Get drag mode, are wee copying, moveigt or modifying component(s).
     */
    eGuiDragMode get_drag_mode() {return m_drag_mode;}

    /* Add object to list of pending deletes.
     */
    void delete_later(eObject *o);

    /* Do pending deletes.
     */
    void delete_pending();


    /**
    ************************************************************************************************
      Opening windows.
    ************************************************************************************************
    */

    /* Send message to object to request open content.
     */
    void open_request(
        const os_char *path,
        eObject *content,
        eObject *context);

    /* Create a window to display object content.
     */
    void open_content(
        const os_char *path,
        eObject *content,
        eObject *context);

    /**
    ************************************************************************************************
      Desktop application specific setup and functions.
    ************************************************************************************************
    */
    void setup_desktop_application();


protected:
    void handle_mouse();

    void drag(os_int mouse_button_nr);
    void drop_modification(os_int mouse_button_nr);

    void open_parameter_tree(
        const os_char *path,
        eObject *content,
        eWindow **win,
        eComponent *p);

    void open_brick_buffer(
        const os_char *path,
        eObject *content,
        const os_char *open_as = OS_NULL);

    void open_signal_assembly(
        const os_char *path,
        eObject *content,
        const os_char *open_as = OS_NULL);

    /* Viewport, corresponds to operating system window, etc.
     */
    eViewPort *m_viewport;

    /* Parameters for drawing components
     */
    eDrawParams m_draw_prm;

    /* ImGui autolabel count for generating labels for windows eWindow and ePopup.
     */
    // os_long m_autolabel_count;
    eContainer *m_autolabel;

    /* Component we use as an origin for drag or component beging modifified.
     */
    ePointer m_drag_origin;

    /* Are we copying or moving component? Are we modifying component?
       EGUI_DRAG_TO_COPY_COMPONENT, EGUI_DRAG_TO_MOVE_OR_COPY_COMPONENT or
       EGUI_DRAG_TO_MODIFY_COMPONENT.
     */
    eGuiDragMode m_drag_mode;

    /* Mouse state data.
     */
    eguiMouseState m_mouse;

    /* Pointers to object to be deleted by gui.
     */
    eContainer *m_delete_list;

    bool m_show_app_metrics;
    bool m_show_app_about;
    bool m_show_app_style_editor;
};


#endif
