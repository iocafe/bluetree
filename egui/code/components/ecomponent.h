/**

  @file    ecomponent.h
  @brief   GUI component base class.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  Base class for all GUI components (widgets).

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef ECOMPONENT_H_
#define ECOMPONENT_H_
#include "egui.h"

class eGui;
class eWindow;
class ePopup;

/**
****************************************************************************************************
  Defines
****************************************************************************************************
*/

/* Enumeration of GUI component properties. These often match eobject library eVariable
   property numbers and names, but a specific eComponent may not need to implement all.
   eComponent has also properties which eVariable doesn't have.
 */
#define ECOMP_VALUE EVARP_VALUE
#define ECOMP_DIGS EVARP_DIGS
#define ECOMP_TEXT EVARP_TEXT
#define ECOMP_UNIT EVARP_UNIT
#define ECOMP_MIN EVARP_MIN
#define ECOMP_MAX EVARP_MAX
#define ECOMP_TYPE EVARP_TYPE
#define ECOMP_ATTR EVARP_ATTR
#define ECOMP_DEFAULT EVARP_DEFAULT
#define ECOMP_GROUP EVARP_GROUP
#define ECOMP_GAIN EVARP_GAIN
#define ECOMP_OFFSET EVARP_OFFSET
#define ECOMP_ABBR EVARP_ABBR
#define ECOMP_TTIP EVARP_TTIP

#define ECOMP_TARGET 30
#define ECOMP_SETVALUE 31
#define ECOMP_NAME 32
#define ECOMP_PATH 33
#define ECOMP_IPATH 34
#define ECOMP_EDIT 35
#define ECOMP_ALL 36
#define ECOMP_DROP_DOWN_LIST_SELECT 37

#define ECOMP_SELECT 60
#define ECOMP_COMMAND 61

#define ECOMP_GC_ALIVE 70
#define ECOMP_GC_SPEED 71
#define ECOMP_GC_TURN 72
#define ECOMP_GC_L1 73
#define ECOMP_GC_L2 74
#define ECOMP_GC_R1 75
#define ECOMP_GC_R2 76
#define ECOMP_GC_TRIANG 77
#define ECOMP_GC_CIRCLE 78
#define ECOMP_GC_CROSS 79
#define ECOMP_GC_SQUARE 80
#define ECOMP_GC_STICKX 81
#define ECOMP_GC_STICKY 82
#define ECOMP_GC_COLOR 87

/* Flags for eComponent::setupproperties() to specify which optional properties for
   the component class.
 */
#define ECOMP_NO_OPTIONAL_PROPERITES 0
#define ECOMP_VALUE_PROPERITES 1
#define ECOMP_EXTRA_UI_PROPERITES 4

/* GUI component property names. Many of these map directly eVariable's property names
 */
#define ecomp_value evarp_value
#define ecomp_digs evarp_digs
#define ecomp_text evarp_text
#define ecomp_abbr evarp_abbr
#define ecomp_ttip evarp_ttip
#define ecomp_unit evarp_unit
#define ecomp_min evarp_min
#define ecomp_max evarp_max
#define ecomp_type evarp_type
#define ecomp_attr evarp_attr
#define ecomp_default evarp_default
#define ecomp_group evarp_group
#define ecomp_gain evarp_gain
#define ecomp_offset evarp_offset
extern const os_char ecomp_target[];
extern const os_char ecomp_setvalue[];
extern const os_char ecomp_name[];
extern const os_char ecomp_path[];
extern const os_char ecomp_ipath[];
extern const os_char ecomp_edit[];
extern const os_char ecomp_all[];
extern const os_char ecomp_drop_down_list_select[];

extern const os_char ecomp_select[];
extern const os_char ecomp_command[];

extern const os_char ecomp_gc_alive[];
extern const os_char ecomp_gc_speed[];
extern const os_char ecomp_gc_turn[];
extern const os_char ecomp_gc_L1[];
extern const os_char ecomp_gc_L2[];
extern const os_char ecomp_gc_R1[];
extern const os_char ecomp_gc_R2[];
extern const os_char ecomp_gc_triang[];
extern const os_char ecomp_gc_circle[];
extern const os_char ecomp_gc_cross[];
extern const os_char ecomp_gc_square[];
extern const os_char ecomp_gc_stickx[];
extern const os_char ecomp_gc_sticky[];
extern const os_char ecomp_gc_msg[];
extern const os_char ecomp_gc_color[];


/* More specific info about drop position, returned by check_pos() function.
 */
typedef enum ecompoPosCheckRval
{
    ECOMPO_IGNORE_MOUSE = 0,
    ECOMPO_POS_OK,
}
ecompoPosCheckRval;


/* Parameters for layout(). These determine size, position of component and it's subcomponents.
   Setup Z order for draing.
 */
 #if 0
typedef struct eLayoutParams
{
    /* Enable component. Disabled components have size 0 and are not dron nor appear in Z order.
     */
    os_boolean enable;

    /* Component is visible. Invisible components are not drawn. Component still
       has size, but it cannot react to mouse or keyboard. Neither it is drawn.
     */
    os_boolean visible;

    /* This component can set input focus.
     */
    os_boolean can_focus;

    /* This component can react to mouse and get mouse capture.
     */
    os_boolean enable_mouse;

    /* This component is in edit mode.
     */
    os_boolean edit_mode;
}
eLayoutParams;
#endif

/* Keyboard flags with mouse.
 */
#define EDRAW_LEFT_CTRL_DOWN 1

typedef struct eDrawParams
{
    /* Pointer to eGui
     */
    eGui *gui;

    /* Pointer to eWindow. OS_NULL if this is popup, etc.
     */
    eWindow *window;

    /* Layer, 0 for base layer, 1 for popup...
     */
    os_int layer;

    /* This component is in edit mode.
     */
    os_boolean edit_mode;

    /* Mouse is over this window.
     */
    os_boolean mouse_over_window;

    /* Component is beging dragged with mouse to this window.
     */
    os_boolean mouse_dragged_over_window;

    /* Components should use these.
     - mouse_pos Current mouse position.
     - mouse_left_click Pulse 1 when mouse click (no drag drop detected)
     - mouse_left_press Left mouse button pressed, used to lock window to place. This is always real mouse button.
     - mouse_left_drag_event Pulse 1 when drag starts.
     - mouse_left_drop_event Pulse 1 when dropped.
     - mouse_left_dragging Stays 1 while dragging.
     - mouse_left_drag_start_pos Mouse down position for drag, set at same time with mouse_left_drag_event.
     */
    ePos mouse_pos;

    os_boolean mouse_left_press;
    ePos mouse_left_press_pos;

    os_boolean mouse_click[EIMGUI_NRO_MOUSE_BUTTONS];
    os_boolean mouse_click_keyboard_flags[EIMGUI_NRO_MOUSE_BUTTONS];

    os_boolean mouse_drag_event[EIMGUI_NRO_MOUSE_BUTTONS];
    os_boolean mouse_drop_event[EIMGUI_NRO_MOUSE_BUTTONS];
    os_boolean mouse_dragging[EIMGUI_NRO_MOUSE_BUTTONS];
    ePos mouse_drag_start_pos[EIMGUI_NRO_MOUSE_BUTTONS];
    os_boolean mouse_drag_keyboard_flags[EIMGUI_NRO_MOUSE_BUTTONS];

    /* Internal for mouse processing. Normally components should not use these.
     */
    ImGuiIO *io;
}
eDrawParams;

/* Draw mode: Are we copying or moving component? Are we modifying component?
 */
typedef enum {
    EGUI_NOT_DRAGGING,
    EGUI_DRAG_TO_COPY_COMPONENT,
    EGUI_DRAG_TO_MOVE_OR_COPY_COMPONENT,
    EGUI_DRAG_TO_MODIFY_COMPONENT
}
eGuiDragMode;

/* Commands, typically from right click popup to component.
 */
typedef enum {
    ECOMPO_NO_COMMAND = 0,
    ECOMPO_REFRESH = 1,
    ECOMPO_CUT,
    ECOMPO_COPY,
    ECOMPO_PASTE,

    ECOMPO_OPEN,
    ECOMPO_GRAPH,
    ECOMPO_PROPERTIES,

    ECOMPO_NEW_ROW,
    ECOMPO_EDIT_ROW,
    ECOMPO_DELETE_ROW,
    ECOMPO_INSERT_DLG_ROW,
    ECOMPO_UPDATE_DLG_ROW
}
eCompoCommand;


/**
****************************************************************************************************
  GUI component base class.
****************************************************************************************************
*/
class eComponent : public eObject
{
    friend class eWindow;

public:

    /**
    ************************************************************************************************
      Generic eObject functionality.
    ************************************************************************************************
    */
    /* Constructor.
     */
    eComponent(
        eObject *parent = OS_NULL,
        e_oid id = EOID_GUI_COMPONENT,
        os_int flags = EOBJ_DEFAULT);

    /* Virtual destructor.
     */
    virtual ~eComponent();

    /* Casting eObject pointer to eComponent pointer.
     */
    inline static eComponent *cast(
        eObject *o)
    {
        return (eComponent*)o;
    }

    /* Get class identifier.
     */
    virtual os_int classid() {return EGUICLASSID_COMPONENT; }

    /* Static function to add class to propertysets and class list.
     */
    static void setupclass();

    /* Add class'es properties to property set.
     */
    static void setupproperties(
        os_int cls,
        os_int flags);

    /* Static constructor function for generating instance by class list.
     */
    static eComponent *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new eComponent(parent, id, flags);
    }

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

    /* Write variable to stream.
     */
    virtual eStatus writer(
        eStream *stream,
        os_int flags);

    /* Read variable from stream.
     */
    virtual eStatus reader(
        eStream *stream,
        os_int flags);


    /**
    ************************************************************************************************
      eComponent base class functions (can be overloaded by specific GUI component)
    ************************************************************************************************
    */

    /* Generating ImGui autolabel.
     */
    virtual os_long make_autolabel() {return 0;}

    virtual ImVec2 window_padding() {return ImVec2(0.0f, 0.0f); }

    /* Draw the component.
     */
    virtual eStatus draw(
        eDrawParams& prm);

    virtual void draw_in_parameter_list(
        eDrawParams& prm) {draw(prm);}

    virtual void draw_tooltip() {}

    /* Draw edit mode decorations, like component frames, etc.
     */
    void draw_edit_mode_decorations(
        eDrawParams& prm,
        os_boolean mouse_over);

    /* Check if mouse position applies to this component.
     */
    virtual ecompoPosCheckRval check_pos(
        ePos& pos,
        eDrawParams *prm,
        eComponent *drag_origin);

    /* Component clicked.
     */
    virtual os_boolean on_click(
        eDrawParams& prm,
        os_int mouse_button_nr);

    /* Drag desture detected, start to drag this component.
     */
    virtual void on_start_drag(
        eDrawParams& prm,
        os_int mouse_button_nr);

    /* Mouse dragging, we are copying/moving/modifying component(s).
     */
    virtual void on_drag(
        eDrawParams& prm,
        os_int mouse_button_nr,
        eGuiDragMode drag_mode);

    /* Mouse released to end drag, actually copy/move object or and modification.
     */
    virtual void on_drop(
        eDrawParams& prm,
        os_int mouse_button_nr,
        eComponent *origin,
        eGuiDragMode drag_mode);

    /* Start editing value, toggle checkbox or show drop down list.
     */
    virtual void activate() {}

    /* Generate right click popup menu.
     */
    virtual ePopup *right_click_popup(
        eDrawParams& prm);

    /* Get visible component rectangle, for checking mouse clicks, ect.
     * For now whole component rectangle is returned.
     */
    eRect& visible_rect() {return m_rect; }


    /**
    ************************************************************************************************
      eComponent class specific functions (base class only)
    ************************************************************************************************
    */

    /* Get first child component identified by oid.
     */
    eComponent *firstcomponent(
        e_oid id = EOID_CHILD);

    /* Get next component identified by oid.
     */
    eComponent *nextcomponent(
        e_oid id = EOID_CHILD);

    /* Get topmost component in Z order which encloses (x, y) position.
     */
    eComponent *findcomponent(
        ePos pos,
        eDrawParams *prm,
        eComponent *drag_origin = OS_NULL);

    /* Add component to window's Z order
     */
    void add_to_zorder(
        eWindow *w,
        os_int layer);

    /* Remove component from window's Z order
     */
    void remove_from_zorder();

    /* Wipe out whole Z order
     */
    void clear_zorder();

    /* Get parent window (eWindow or ePopup).
     */
    eComponent *window(
        os_int cid = 0);

    /* Get parent gui.
     */
    inline eGui *gui()
    {
        return (eGui*)parent(EGUICLASSID_GUI, EOID_ALL, true);
    }

    /* Create popup window, often "select value" or "right click" popup menu.
     */
    ePopup *popup();

    /* Add command item to right click popup menu
     */
    void add_popup_item_command(
        const os_char *text,
        os_int command,
        ePopup *p);

    /* Add toggle item to right click popup menu
     */
    void add_popup_item_toggle(
        const os_char *text,
        os_int propertynr,
        const os_char *propertyname,
        ePopup *p);

    /* Send message to object to request open content
     */
    void open_request(
        const os_char *path,
        os_int command = EBROWSE_OPEN);

    /* Create "select value from drop down list" popup.
     */
    ePopup *drop_down_list(
        eContainer *list,
        const os_char *propertyname = OS_NULL,
        eVariable *value = OS_NULL);

    /* Add edit mode items into "right click" popup menu.
     */
    void add_popup_edit_mode_items(
        eDrawParams& prm,
        ePopup *p);

    /* If this GUI component has popup open, close it.
     */
    void close_popup();

    /* Delete has been selected from pop up menu, etc.
     */
    void on_delete();

    /* Set redo layout flag.
     */
    // void redo_layout();

    /* Invalidate current component rectangle.
     */
    inline void invalidate()
    {
        invalidate(m_rect);
    }

    /* Invalidate specified rectangle.
     */
    void invalidate(eRect& r);

    /* Set keyboard input focus to this component.
     */
    // void focus();

    /* Capture mouse events to this component.
     */
    // void capture_mouse();


    eRect& rect() {return m_rect; }


protected:

    /**
    ************************************************************************************************
      Member variables.
    ************************************************************************************************
    */

    /* Current component rectangle (screen coordinates).
     */
    eRect m_rect;

    os_boolean m_select;        /* Component is selected, property value */
    os_boolean m_popup_open;

    /* Z order */
    eComponent *m_next_z, *m_prev_z;
    os_int m_zlayer;
};


#endif
