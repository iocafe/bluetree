/**

  @file    egui.cpp
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
#include "egui.h"


/* GUI property names.
 */
const os_char
    eguip_text[] = "x",
    eguip_open[] = "open";


/**
****************************************************************************************************

  @brief Constructor.

  X...

  @return  None.

****************************************************************************************************
*/
eGui::eGui(
    eObject *parent,
    e_oid id,
    os_int flags)
    : eObject(parent, id, flags)
{
    m_viewport = eimgui_open_viewport();

    os_memclear(&m_draw_prm, sizeof(eDrawParams));
    m_draw_prm.gui = this;
    m_autolabel_count = 0;
    m_drag_mode = EGUI_NOT_DRAGGING;

    os_memclear(&m_mouse, sizeof(eguiMouseState));

    addname("//gui");
    ns_create("gui");

    m_delete_list = new eContainer(this, EOID_GUI_TO_BE_DELETED, EOBJ_TEMPORARY_ATTACHMENT);
}


/**
****************************************************************************************************

  @brief Virtual destructor.

  X...

  @return  None.

****************************************************************************************************
*/
eGui::~eGui()
{
    egui_close_viewport(m_viewport);
}


/**
****************************************************************************************************

  @brief Clone object

  The clone function clones the GUI and clonable attachments. Names will be left detached
  in clone if EOBJ_NO_MAP flag is given.

  @param  parent Parent for the clone.
  @param  id Object identifier for the clone.
  @param  aflags 0 for default operation. EOBJ_NO_MAP not to map names.
  @return Pointer to the clone.

****************************************************************************************************
*/
eObject *eGui::clone(
    eObject *parent,
    e_oid id,
    os_int aflags)
{
    eGui *clonedobj;
    clonedobj = new eGui(parent, id == EOID_CHILD ? oid() : id, flags());

    /** Copy variable value.
     */
    // clonedobj->setv(this);
    // clonedobj->setdigs(digs());

    /* Copy clonable attachments.
     */
    clonegeneric(clonedobj, aflags);
    return clonedobj;
}


/**
****************************************************************************************************

  @brief Add eGui to class list and class'es properties to it's property set.

  The eGui::setupclass function adds eGui to class list and class'es properties to
  it's property set. The class list enables creating new objects dynamically by class identifier,
  which is used for serialization reader functions. The property set stores static list of
  class'es properties and metadata for those.

****************************************************************************************************
*/
void eGui::setupclass()
{
    const os_int cls = EGUICLASSID_GUI;

    /* Add the class to class list.
     */
    os_lock();
    eclasslist_add(cls, (eNewObjFunc)newobj, "eGui");
    addproperty(cls, EGUIP_TEXT, eguip_text, "text");
    addproperty(cls, EGUIP_OPEN, eguip_open, "open window");
    propertysetdone(cls);
    os_unlock();
}


/**
****************************************************************************************************

  @brief Generating ImGui autolabel.

  The eGui::make_autolabel generates unique nonzero numbers for ImGui labels.

  @return Unique number (we wish).

****************************************************************************************************
*/
os_long eGui::make_autolabel()
{
    while (++m_autolabel_count == 0);
    return m_autolabel_count;
}


/**
****************************************************************************************************

  @brief Get the first child component identified by oid.

  The eComponent::firstcomponent() function returns pointer to the next EGUI component.

  @param   id Object idenfifier. Default value EOID_CHILD specifies to count a child objects,
           which are not flagged as an attachment. Value EOID_ALL specifies to get count all
           child objects, regardless wether these are attachment or not. Other values
           specify object identifier, only children with that specified object identifier
           are searched for.

  @return  Pointer to the first child component, or OS_NULL if none found.

****************************************************************************************************
*/
eComponent *eGui::firstcomponent(
    e_oid id)
{
    eObject *o;
    os_int cid;

    o = first(id);
    if (o == OS_NULL) {
        return OS_NULL;
    }

    cid = o->classid();
    if (cid >= EGUICLASSID_BEGIN_COMPONENTS && cid <= EGUICLASSID_END_COMPONENTS) {
        return (eComponent*)o;
    }

    eHandle *h = o->handle()->next(id);
    while (h)
    {
        cid = h->object()->classid();
        if (cid >= EGUICLASSID_BEGIN_COMPONENTS &&
            cid <= EGUICLASSID_END_COMPONENTS)
        {
            return (eComponent*)h->object();
        }

        h = h->next(id);
    }
    return OS_NULL;
}


/**
****************************************************************************************************

  @brief Called to inform the class about property value change (override).

  The onpropertychange() function is called when class'es property changes, unless the
  property is flagged with EPRO_NOONPRCH.
  If property is flagged as EPRO_SIMPLE, this function shuold save the property value
  in class members and and return it when simpleproperty() is called.

  Notice for change logging: Previous value is still valid when this function is called.
  You can get the old value by calling property() function inside onpropertychange()
  function.

  @param   propertynr Property number of changed property.
  @param   x Variable containing the new value.
  @param   flags
  @return  If successfull, the function returns ESTATUS_SUCCESS (0). Nonzero return values do
           indicate that there was no property with given property number.

****************************************************************************************************
*/
eStatus eGui::onpropertychange(
    os_int propertynr,
    eVariable *x,
    os_int flags)
{
    switch (propertynr)
    {
        case EGUIP_TEXT:
            eimgui_set_window_title(x->gets());
            break;

        case EGUIP_OPEN:
            // setdigs((os_int)x->getl());
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
eStatus eGui::simpleproperty(
    os_int propertynr,
    eVariable *x)
{
    return eObject::simpleproperty(propertynr, x);
}



//-----------------------------------------------------------------------------
// [SECTION] Example App: Docking, DockSpace / ShowExampleAppDockSpace()
//-----------------------------------------------------------------------------

// Demonstrate using DockSpace() to create an explicit docking node within an existing window.
// Note that you already dock windows into each others _without_ a DockSpace() by just moving windows
// from their title bar (or by holding SHIFT if io.ConfigDockingWithShift is set).
// DockSpace() is only useful to construct to a central location for your application.
void ShowExampleAppDockSpace(bool* p_open)
{
    static bool opt_fullscreen = true;
    static bool opt_padding = false;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

    // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
    // because it would be confusing to have two docking targets within each others.
#ifdef UKEK
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
#else
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
#endif
    if (opt_fullscreen)
    {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->GetWorkPos());
        ImGui::SetNextWindowSize(viewport->GetWorkSize());
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    }
    else
    {
        dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
    }

    // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
    // and handle the pass-thru hole, so we ask Begin() to not render a background.
    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        window_flags |= ImGuiWindowFlags_NoBackground;

    // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
    // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
    // all active windows docked into it will lose their parent and become undocked.
    // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
    // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
    if (!opt_padding)
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace Demo", p_open, window_flags);
    if (!opt_padding)
        ImGui::PopStyleVar();

    if (opt_fullscreen)
        ImGui::PopStyleVar(2);

    // DockSpace
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigWindowsMoveFromTitleBarOnly = true;
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    }
    else
    {
        // ShowDockingDisabledMessage();
    }

#ifdef UKEK
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("Options"))
        {
            // Disabling fullscreen would allow the window to be moved to the front of other windows,
            // which we can't undo at the moment without finer window depth/z control.
            ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen);
            ImGui::MenuItem("Padding", NULL, &opt_padding);
            ImGui::Separator();

            if (ImGui::MenuItem("Flag: NoSplit",                "", (dockspace_flags & ImGuiDockNodeFlags_NoSplit) != 0))                 { dockspace_flags ^= ImGuiDockNodeFlags_NoSplit; }
            if (ImGui::MenuItem("Flag: NoResize",               "", (dockspace_flags & ImGuiDockNodeFlags_NoResize) != 0))                { dockspace_flags ^= ImGuiDockNodeFlags_NoResize; }
            if (ImGui::MenuItem("Flag: NoDockingInCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_NoDockingInCentralNode) != 0))  { dockspace_flags ^= ImGuiDockNodeFlags_NoDockingInCentralNode; }
            if (ImGui::MenuItem("Flag: AutoHideTabBar",         "", (dockspace_flags & ImGuiDockNodeFlags_AutoHideTabBar) != 0))          { dockspace_flags ^= ImGuiDockNodeFlags_AutoHideTabBar; }
            if (ImGui::MenuItem("Flag: PassthruCentralNode",    "", (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) != 0, opt_fullscreen)) { dockspace_flags ^= ImGuiDockNodeFlags_PassthruCentralNode; }
            ImGui::Separator();

            if (ImGui::MenuItem("Close", NULL, false, p_open != NULL))
                *p_open = false;
            ImGui::EndMenu();
        }
        /* HelpMarker(
            "When docking is enabled, you can ALWAYS dock MOST window into another! Try it now!" "\n\n"
            " > if io.ConfigDockingWithShift==false (default):" "\n"
            "   drag windows from title bar to dock" "\n"
            " > if io.ConfigDockingWithShift==true:" "\n"
            "   drag windows from anywhere and hold Shift to dock" "\n\n"
            "This demo app has nothing to do with it!" "\n\n"
            "This demo app only demonstrate the use of ImGui::DockSpace() which allows you to manually create a docking node _within_ another window. This is useful so you can decorate your main application window (e.g. with a menu bar)." "\n\n"
            "ImGui::DockSpace() comes with one hard constraint: it needs to be submitted _before_ any window which may be docked into it. Therefore, if you use a dock spot as the central point of your application, you'll probably want it to be part of the very first window you are submitting to imgui every frame." "\n\n"
            "(NB: because of this constraint, the implicit \"Debug\" window can not be docked into an explicit DockSpace() node, because that window is submitted as part of the NewFrame() call. An easy workaround is that you can create your own implicit \"Debug##2\" window after calling DockSpace() and leave it in the window stack for anyone to use.)"
        ); */

        ImGui::EndMenuBar();
    }
#endif

    ImGui::End();
}


eStatus eGui::run()
{
    eComponent *c;
    eStatus s;
    os_int mouse_button_nr;

    bool  openoi = true;

    while (true)
    {
// TEMP PATCH. THIS NEEDS TO WAIT FOR USER INPUT OR OTHER EVENTS, NOT TO RUN IN FAST LOOP
        eglobal->eguiglobal->guilib_thread->alive(EALIVE_RETURN_IMMEDIATELY);

        s = eimgui_start_frame(m_viewport);
        if (s) break;

        ShowExampleAppDockSpace(&openoi);

        m_draw_prm.io = &ImGui::GetIO();

        handle_mouse();
        // m_draw_prm.right_click = ImGui::IsMouseReleased(EIMGUI_RIGHT_MOUSE_BUTTON);

        for (c = firstcomponent(EOID_GUI_WINDOW); c; c = c->nextcomponent(EOID_GUI_WINDOW))
        {
            c->draw(m_draw_prm);
        }

        for (mouse_button_nr = 0;
             mouse_button_nr < EIMGUI_NRO_MOUSE_BUTTONS;
             mouse_button_nr++)
        {
            if (m_draw_prm.mouse_dragging[mouse_button_nr]) {
                drag(mouse_button_nr);
            }

            if (m_draw_prm.mouse_drop_event[mouse_button_nr]) {
                drop_modification(mouse_button_nr);
            }
        }

        eimgui_finish_frame(m_viewport);

        delete_pending();
    }

    return s;
}


/**
****************************************************************************************************

  @brief Sort out ImGui mouse clicks, and drag and drop, etc, into m_draw_prm.

  This function separated drag and drop from mouse click, etc, and stores this information
  into eDrawParams structure "m_draw_prm" in format which is useful for egui library.

  About eDrawParams structure

    - mouse_pos Current mouse position.
    - mouse_click Pulse 1 when mouse click (no drag drop detected)
    - mouse_drag_event Pulse 1 when drag starts.
    - mouse_drop_event Pulse 1 when dropped.
    - mouse_dragging Stays 1 while dragging.
    - mouse_drag_start_pos Mouse down position for drag, set at same time with mouse_drag_event

****************************************************************************************************
*/
void eGui::handle_mouse()
{
    os_int i, dx, dy, kf;

    m_draw_prm.io = &ImGui::GetIO();

    /* Normally we give only "pulses" to components.
     */
    m_draw_prm.mouse_click[EIMGUI_LEFT_MOUSE_BUTTON] = OS_FALSE;
    m_draw_prm.mouse_click[EIMGUI_RIGHT_MOUSE_BUTTON] = OS_FALSE;

    m_draw_prm.mouse_drop_event[EIMGUI_LEFT_MOUSE_BUTTON] = OS_FALSE;
    m_draw_prm.mouse_drop_event[EIMGUI_RIGHT_MOUSE_BUTTON] = OS_FALSE;
    m_draw_prm.mouse_drag_event[EIMGUI_LEFT_MOUSE_BUTTON] = OS_FALSE;
    m_draw_prm.mouse_drag_event[EIMGUI_RIGHT_MOUSE_BUTTON] = OS_FALSE;
    m_draw_prm.mouse_dragging[EIMGUI_LEFT_MOUSE_BUTTON] = OS_FALSE;
    m_draw_prm.mouse_dragging[EIMGUI_RIGHT_MOUSE_BUTTON] = OS_FALSE;

    m_draw_prm.mouse_pos.x = m_draw_prm.io->MousePos.x;
    m_draw_prm.mouse_pos.y = m_draw_prm.io->MousePos.y;

    for (i = 0; i < EIMGUI_NRO_MOUSE_BUTTONS; i++)
    {
        m_mouse.is_down[i] = ImGui::IsMouseDown(i);

        /* When mouse is down, detect hold down and start of drag
         */
        if (m_mouse.is_down[i]) {

            if (m_mouse.is_down[i] &&
                m_mouse.was_down[i] != m_mouse.is_down[i])
            {
                m_mouse.down_pos[i] = m_draw_prm.mouse_pos;
                m_mouse.is_dragging[i] = OS_FALSE;
                m_mouse.held_still[i] = OS_FALSE;

                // m_draw_prm.io->KeyMap[] - convert key define to key number
                // m_draw_prm.io->KeysDown[]; - check if key is down by key number
                kf = 0;

                if (m_draw_prm.io->KeyCtrl) kf |= EDRAW_LEFT_CTRL_DOWN;
                m_mouse.keyboard_flags[i] = kf;
            }

            dx = m_draw_prm.mouse_pos.x - m_mouse.down_pos[i].x;
            dy = m_draw_prm.mouse_pos.y - m_mouse.down_pos[i].y;

            if (dx*dx + dy*dy > 20 && !m_mouse.is_dragging[i])
            {
                m_mouse.is_dragging[i] = OS_TRUE;
            }

            if (!m_mouse.is_dragging[i] &&
                !m_mouse.held_still[i])
            {
                if (m_draw_prm.io->MouseDownDuration[i] > 1)
                {
                    m_mouse.held_still[i] = OS_TRUE;
                }
            }
        }

        /* Mouse left_press is always is used to lock window to place.
           Do not check held_still
         */
        if (i == EIMGUI_LEFT_MOUSE_BUTTON &&
            m_draw_prm.mouse_left_press != m_mouse.is_down[i])
        {
            m_draw_prm.mouse_left_press = m_mouse.is_down[i];
            if (m_draw_prm.mouse_left_press) {
                m_draw_prm.mouse_left_press_pos = m_draw_prm.mouse_pos;
            }
        }

        /* Start dragging event with position, maintain "dragging" flag.
         */
        if (m_mouse.is_dragging[i])
        {
            if (m_mouse.is_dragging[i] != m_mouse.was_dragging[i])
            {
                if (i == EIMGUI_LEFT_MOUSE_BUTTON && !m_mouse.held_still[i]) {
                    m_draw_prm.mouse_drag_event[EIMGUI_LEFT_MOUSE_BUTTON] = OS_TRUE;
                    m_draw_prm.mouse_drag_start_pos[EIMGUI_LEFT_MOUSE_BUTTON]
                        = m_mouse.down_pos[i];
                    m_draw_prm.mouse_drag_keyboard_flags[EIMGUI_LEFT_MOUSE_BUTTON]
                        = m_mouse.keyboard_flags[i];
                }
                else {
                    m_draw_prm.mouse_drag_event[EIMGUI_RIGHT_MOUSE_BUTTON] = OS_TRUE;
                    m_draw_prm.mouse_drag_start_pos[EIMGUI_RIGHT_MOUSE_BUTTON]
                        = m_mouse.down_pos[i];
                    m_draw_prm.mouse_drag_keyboard_flags[EIMGUI_RIGHT_MOUSE_BUTTON]
                        = m_mouse.keyboard_flags[i];
                }
                save_drag_origin(OS_NULL, EGUI_NOT_DRAGGING);
            }
            if (i == EIMGUI_LEFT_MOUSE_BUTTON && !m_mouse.held_still[i]) {
                m_draw_prm.mouse_dragging[EIMGUI_LEFT_MOUSE_BUTTON] = OS_TRUE;
            }
            else {
                m_draw_prm.mouse_dragging[EIMGUI_RIGHT_MOUSE_BUTTON] = OS_TRUE;
            }
        }
        m_mouse.was_dragging[i] = m_mouse.is_dragging[i];

        /* Generate mouse click and drop on release pulses on mouse release.
         */
        if (!m_mouse.is_down[i] &&
            m_mouse.was_down[i] != m_mouse.is_down[i])
        {
            if (m_mouse.is_dragging[i])
            {
                if (i == EIMGUI_LEFT_MOUSE_BUTTON && !m_mouse.held_still[i]) {
                    m_draw_prm.mouse_drop_event[EIMGUI_LEFT_MOUSE_BUTTON] = OS_TRUE;
                }
                else {
                    m_draw_prm.mouse_drop_event[EIMGUI_RIGHT_MOUSE_BUTTON] = OS_TRUE;
               }
            }

            else {
                if (i == EIMGUI_LEFT_MOUSE_BUTTON && !m_mouse.held_still[i]) {
                    m_draw_prm.mouse_click[EIMGUI_LEFT_MOUSE_BUTTON] = OS_TRUE;
                    m_draw_prm.mouse_click_keyboard_flags[EIMGUI_LEFT_MOUSE_BUTTON]
                        = m_mouse.keyboard_flags[i];
                }
                else {
                    m_draw_prm.mouse_click[EIMGUI_RIGHT_MOUSE_BUTTON] = OS_TRUE;
                    m_draw_prm.mouse_click_keyboard_flags[EIMGUI_RIGHT_MOUSE_BUTTON]
                        = m_mouse.keyboard_flags[i];
                }
            }

            m_mouse.is_dragging[i] = OS_FALSE;
            m_mouse.held_still[i] = OS_FALSE;
        }

        m_mouse.was_down[i] = m_mouse.is_down[i];
    }
}


/**
****************************************************************************************************

  @brief Set drag origin component.

  The function stores reference (ePointer) to refer to "drag origin" component and sets drag
  mode. These are stored within eGui object.

  @param   c Pointer to component to use as "drag origin".
  @param   drag_mode Drag modes EGUI_DRAG_TO_COPY_COMPONENT and EGUI_DRAG_TO_MOVE_OR_COPY_COMPONENT
           drag GUI components to move or copy them. If component is dragged from window to
           another, it is always copied. If component is dragged within window, it is moved
           by default. But it will be copied if user presses CTRL key when starting the
           drag.
           Value EGUI_DRAG_TO_MODIFY_COMPONENT specifies that we are modifying component
           by dragging some point of it with mouse.

****************************************************************************************************
*/
void eGui::save_drag_origin(
    eComponent *c,
    eGuiDragMode drag_mode)
{
    m_drag_origin.set(c);
    m_drag_mode = drag_mode;
}


/**
****************************************************************************************************

  @brief Get drag origin component.

  Drag modes EGUI_DRAG_TO_COPY_COMPONENT and EGUI_DRAG_TO_MOVE_OR_COPY_COMPONENT:
  When dragging components, we drag all components in select list of a window. Despite this
  one of dragged components can be called "drag origin" component. The drag origin is used
  to get the window from which components are dragged from, etc.

  EGUI_DRAG_TO_MODIFY_COMPONENT drag origin is component being modified.

  @param   mouse_button_nr Either EIMGUI_LEFT_MOUSE_BUTTON (0) or
           EIMGUI_RIGHT_MOUSE_BUTTON (1).

****************************************************************************************************
*/
eComponent *eGui::get_drag_origin()
{
    return (eComponent*)m_drag_origin.get();
}


/**
****************************************************************************************************

  @brief Visualize dragging.

  Plan is to add drag visualization code to on_drag for gui component.
  NO VISUALIZATION FOR NOW.

  @param   mouse_button_nr Either EIMGUI_LEFT_MOUSE_BUTTON (0) or
           EIMGUI_RIGHT_MOUSE_BUTTON (1).

****************************************************************************************************
*/
void eGui::drag(
    os_int mouse_button_nr)
{
    eComponent *c;
    c = get_drag_origin();
    if (c) {
        c->on_drag(m_draw_prm, mouse_button_nr, get_drag_mode());
    }
}


/**
****************************************************************************************************

  @brief Finish "drag" modification to GUI component.

  This function is called to end EGUI_DRAG_TO_MODIFY_COMPONENT drag to modify a component.

  @param   mouse_button_nr Either EIMGUI_LEFT_MOUSE_BUTTON (0) or
           EIMGUI_RIGHT_MOUSE_BUTTON (1).

****************************************************************************************************
*/
void eGui::drop_modification(
    os_int mouse_button_nr)
{
    eComponent *origin;
    eGuiDragMode drag_mode;

    origin = get_drag_origin();
    if (origin == OS_NULL) {
        return;
    }

    drag_mode = get_drag_mode();
    if (drag_mode == EGUI_DRAG_TO_MODIFY_COMPONENT) {
        origin->on_drop(m_draw_prm, mouse_button_nr, OS_NULL, drag_mode);
        save_drag_origin(OS_NULL, EGUI_NOT_DRAGGING);
    }
}


/**
****************************************************************************************************

  @brief Add object to list of pending deletes.

  Add reference to object (usually GUI component) to list objects to be deleted.
  See delete_pending() function.

  @param   o Pointer to object to add to pending deletes.

****************************************************************************************************
*/
void eGui::delete_later(eObject *o)
{
    ePointer *p;

    p = new ePointer(m_delete_list);
    p->set(o);
}


/**
****************************************************************************************************

  @brief Do pending deletes.

  When GUI component is to be deleted, deleting it immediately while prosessing messages, etc,
  is risky. To avoid issues with objects deleted in middle of operation, the ePointer
  reference to object is added to list of pending deletes by calling delete_later() function.
  This function is called after all GUI frame processing has been completed. It deletes
  objects referred on pending deletes list and clears the list.

****************************************************************************************************
*/
void eGui::delete_pending()
{
    ePointer *p, *next_p;
    eObject *o;

    for (p = (ePointer*)m_delete_list->first(); p; p = next_p)
    {
        next_p = (ePointer*)p->next();
        if (p->classid() != ECLASSID_POINTER) continue;
        o = p->get();
        if (o) delete o;
        delete p;
    }
}
