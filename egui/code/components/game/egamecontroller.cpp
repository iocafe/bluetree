/**

  @file    egamecontroller.cpp
  @brief   Game controller, control speed, turning, etc.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    15.9.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "egui.h"


/**
****************************************************************************************************
  Constructor.
****************************************************************************************************
*/
eGameController::eGameController(
    eObject *parent,
    e_oid id,
    os_int flags)
    : eComponent(parent, id, flags)
{
    os_get_timer(&m_update_timer);
    timer(330);
}


/**
****************************************************************************************************

  @brief Clone object

  The clone function clones the variable and clonable attachments. Names will be left detached
  in clone if EOBJ_NO_MAP flag is given.

  @param  parent Parent for the clone.
  @param  id Object identifier for the clone.
  @param  aflags 0 for default operation. EOBJ_NO_MAP not to map names.
  @return Pointer to the clone.

****************************************************************************************************
*/
eObject *eGameController::clone(
    eObject *parent,
    e_oid id,
    os_int aflags)
{
    eGameController *clonedobj;
    clonedobj = new eGameController(parent, id == EOID_CHILD ? oid() : id, flags());
    clonegeneric(clonedobj, aflags);
    return clonedobj;
}


/**
****************************************************************************************************

  @brief Add eGameController to class list and class'es properties to it's property set.

  The eGameController::setupclass function adds eGameController to class list and class'es properties to
  it's property set. The class list enables creating new objects dynamically by class identifier,
  which is used for serialization reader functions. The property set stores static list of
  class'es properties and metadata for those.

****************************************************************************************************
*/
void eGameController::setupclass()
{
    const os_int cls = EGUICLASSID_GAME_CONTROLLER;
    eVariable *v;

    os_lock();
    eclasslist_add(cls, (eNewObjFunc)newobj, "eGameController", EGUICLASSID_COMPONENT);
    setupproperties(cls, ECOMP_NO_OPTIONAL_PROPERITES);
    addpropertys(cls, ECOMP_TEXT, ecomp_gc_msg, "message", EPRO_DEFAULT);
    addproperty (cls, ECOMP_GC_COLOR, ecomp_gc_color, "color", EPRO_SIMPLE);

    addpropertyl(cls, ECOMP_GC_ALIVE, ecomp_gc_alive, "alive", EPRO_SIMPLE);
    v = addpropertyl(cls, ECOMP_GC_SPEED, ecomp_gc_speed, "speed", EPRO_SIMPLE);
    v->setpropertyl(EVARP_MIN, -10000);
    v->setpropertyl(EVARP_MAX, 10000);
    v->setpropertys(EVARP_UNIT, "%");
    v->setpropertys(EVARP_TTIP, "1/100 percents of max speed, -10000 (full backwards) .. 10000 (full forward)");
    v = addpropertyl(cls, ECOMP_GC_TURN, ecomp_gc_turn, "turn", EPRO_SIMPLE);
    v->setpropertyl(EVARP_MIN, -9000);
    v->setpropertyl(EVARP_MAX, 9000);
    v->setpropertys(EVARP_UNIT, "deg");
    v->setpropertys(EVARP_TTIP, "1/100 degrees, -9000 (left) .. 9000 (right)");
    addpropertyb(cls, ECOMP_GC_L1, ecomp_gc_L1, "L1", EPRO_SIMPLE);
    addpropertyb(cls, ECOMP_GC_L2, ecomp_gc_L2, "L2", EPRO_SIMPLE);
    addpropertyb(cls, ECOMP_GC_R1, ecomp_gc_R1, "R1", EPRO_SIMPLE);
    addpropertyb(cls, ECOMP_GC_R2, ecomp_gc_R2, "R2", EPRO_SIMPLE);
    addpropertyb(cls, ECOMP_GC_TRIANG, ecomp_gc_triang, "triangle", EPRO_SIMPLE);
    addpropertyb(cls, ECOMP_GC_CIRCLE, ecomp_gc_circle, "circle", EPRO_SIMPLE);
    addpropertyb(cls, ECOMP_GC_CROSS, ecomp_gc_cross, "cross", EPRO_SIMPLE);
    addpropertyb(cls, ECOMP_GC_SQUARE, ecomp_gc_square, "square", EPRO_SIMPLE);
    addpropertyl(cls, ECOMP_GC_STICKX, ecomp_gc_stickx, "stick x", EPRO_SIMPLE);
    addpropertyl(cls, ECOMP_GC_STICKY, ecomp_gc_sticky, "strick y", EPRO_SIMPLE);

    propertysetdone(cls);
    os_unlock();
}


/**
****************************************************************************************************

  @brief Function to process incoming messages.

  The eGameController::onmessage function sends repeated "alive" signals by timer. This
  if to inform a device that controller is alive and well.

  @param   envelope Message envelope. Contains command, target and source paths and
           message content, etc.
  @return  None.

****************************************************************************************************
*/
void eGameController::onmessage(
    eEnvelope *envelope)
{
    os_ushort next_alive;

    /* If at final destination for the message.
     */
    if (*envelope->target()=='\0')
    {
        switch (envelope->command())
        {
            case ECMD_TIMER: /* No need to do anything, timer is used just to break event wait */
                next_alive = m_alive  + 1;
                if (next_alive == 0) next_alive++;
                setpropertyl(ECOMP_GC_ALIVE, next_alive);
                return;

            default:
                break;
        }
    }

    /* Call parent class'es onmessage.
     */
    eComponent::onmessage(envelope);
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
eStatus eGameController::onpropertychange(
    os_int propertynr,
    eVariable *x,
    os_int flags)
{
    switch (propertynr)
    {
        case ECOMP_TEXT:
            m_text.clear();
            break;

        case ECOMP_GC_COLOR:
            break;

        case ECOMP_GC_ALIVE:
            m_alive = x->getl();
            break;

        case ECOMP_GC_SPEED:
            m_speed = x->getl();
            break;

        case ECOMP_GC_TURN:
            m_turn = x->getl();
            break;

        case ECOMP_GC_L1:
            m_L1 = (os_boolean)x->getl();
            break;

        case ECOMP_GC_L2:
            m_L2 = (os_boolean)x->getl();
            break;

        case ECOMP_GC_R1:
            m_R1 = (os_boolean)x->getl();
            break;

        case ECOMP_GC_R2:
            m_R2 = (os_boolean)x->getl();
            break;

        case ECOMP_GC_TRIANG:
            m_triangle = (os_boolean)x->getl();
            break;

        case ECOMP_GC_CIRCLE:
            m_circle = (os_boolean)x->getl();
            break;

        case ECOMP_GC_CROSS:
            m_cross = (os_boolean)x->getl();
            break;

        case ECOMP_GC_SQUARE:
            m_square = (os_boolean)x->getl();
            break;

        case ECOMP_GC_STICKX:
            m_stick_x = x->getl();
            break;

        case ECOMP_GC_STICKY:
            m_stick_y = x->getl();
            break;

        default:
            return eComponent::onpropertychange(propertynr, x, flags);
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
eStatus eGameController::simpleproperty(
    os_int propertynr,
    eVariable *x)
{
    switch (propertynr)
    {
        case ECOMP_GC_COLOR:
            break;

        case ECOMP_GC_ALIVE:
            x->setl(m_alive);
            break;

        case ECOMP_GC_SPEED:
            x->setl(m_speed);
            break;

        case ECOMP_GC_TURN:
            x->setl(m_turn);
            break;

        case ECOMP_GC_L1:
            x->setl(m_L1);
            break;

        case ECOMP_GC_L2:
            x->setl(m_L2);
            break;

        case ECOMP_GC_R1:
            x->setl(m_R1);
            break;

        case ECOMP_GC_R2:
            x->setl(m_R2);
            break;

        case ECOMP_GC_TRIANG:
            x->setl(m_triangle);
            break;

        case ECOMP_GC_CIRCLE:
            x->setl(m_circle);
            break;

        case ECOMP_GC_CROSS:
            x->setl(m_cross);
            break;

        case ECOMP_GC_SQUARE:
            x->setl(m_square);
            break;

        case ECOMP_GC_STICKX:
            x->setl(m_stick_x);
            break;

        case ECOMP_GC_STICKY:
            x->setl(m_stick_y);
            break;

        default:
            return eComponent::simpleproperty(propertynr, x);
    }
    return ESTATUS_SUCCESS;
}

/**
****************************************************************************************************

  @brief Draw the component.

  The eGameController::draw() function calls ImGui API to render the component.

  @param   Prm Drawing parameters.
  @return  The function return ESTATUS_SUCCESS if all is fine. Other values indicate that the
           component is no longer drawable or useful. This could be for example a pop up menu
           closed implicitely by clicking elsewhere.

****************************************************************************************************
*/
eStatus eGameController::draw(
    eDrawParams& prm)
{
    ImVec2 sz;
    float speed, turn, xdelta, ydelta;
    float xcoeff, ycoeff, xorigin, yorigin, left, right, top, bottom, x, y;
    os_boolean moving = OS_FALSE, setting_motion = OS_FALSE;

    add_to_zorder(prm.window, prm.layer);

    ImVec2 cpos = ImGui::GetCursorScreenPos();
    m_rect.x1 = (os_int)cpos.x;
    m_rect.y1 = (os_int)cpos.y;

    sz.x = sz.y = 0;

    ImDrawList *draw_list = ImGui::GetWindowDrawList();
    ImVec2 r = ImGui::GetContentRegionAvail();

    xcoeff = r.x / 18000.0f;
    ycoeff = -r.y / 20000.0f;
    xorigin = cpos.x + xcoeff * 9000.0f;
    yorigin = cpos.y - ycoeff * 10000.0f;
    left = xorigin - xcoeff * 9000.0f;
    right = xorigin + xcoeff * 9000.0f;
    top = yorigin - ycoeff * 10000.0f;
    bottom = yorigin + ycoeff * 10000.0f;

    ImU32  col = IM_COL32(128, 128, 128, 128);
    draw_list->AddLine(ImVec2(left, yorigin), ImVec2(right, yorigin), col, 1.0f /* thickness */);
    draw_list->AddLine(ImVec2(xorigin, top), ImVec2(xorigin, bottom), col, 1.0f /* thickness */);

    if (ImGui::IsWindowHovered())
    {
        if (prm.mouse_left_press && xcoeff != 0.0 && ycoeff != 0.0) {

            turn = (prm.mouse_pos.x - xorigin) / xcoeff;
            xdelta = turn - m_turn;
            speed = (prm.mouse_pos.y - yorigin) / ycoeff;
            ydelta = speed - m_speed;
            if (turn >= -9000.0f && turn <= 9000.0f &&
                speed >= -10000.0f && speed <= 10000.0f)
            {
                if (xdelta*xdelta > 100)
                    setpropertyl(ECOMP_GC_TURN, os_round_short(turn));

                if (ydelta*ydelta > 100)
                    setpropertyl(ECOMP_GC_SPEED, os_round_short(speed));
            }

            setting_motion = OS_TRUE;
        }
    }

    moving = update_motion(prm.timer_us, !setting_motion);
    if (moving) {
        x = xorigin + xcoeff * m_turn;
        y = yorigin + ycoeff * m_speed;
        col = IM_COL32(255, 255, 100, 250);
        draw_list->AddLine(ImVec2(left, y), ImVec2(right, y), col, 2.0f /* thickness */);
        draw_list->AddLine(ImVec2(x, top), ImVec2(x, bottom), col, 2.0f /* thickness */);

    }

    sz.x = r.x;
    sz.x = r.y;

    m_rect.x2 = m_rect.x1 + (os_int)sz.x - 1;
    m_rect.y2 = m_rect.y1 + (os_int)sz.y - 1;

    /* Let base class implementation handle the rest.
     */
    return eComponent::draw(prm);

}


os_boolean eGameController::update_motion(
    os_timer timer_us,
    os_boolean change_it)
{
    os_long elapsed_us;
    const os_double speed_change_per_sec = 7000.0;
    const os_double turn_change_per_sec = 3500.0;
    os_boolean rval = OS_FALSE;
    os_double change_max;
    os_short speed, turn;

    /*  How long since last update call
     */
    elapsed_us = timer_us - m_update_timer;
    if (elapsed_us < 0) elapsed_us = 0;
    if (elapsed_us > 1000000) elapsed_us = 1000000;
    m_update_timer = timer_us;

    /* Speed change.
     */
    change_max = 0.000001 * elapsed_us * speed_change_per_sec;
    if (m_speed > change_max) {
        speed = m_speed - (os_short)change_max;
    }
    else if (m_speed < -change_max) {
        speed = m_speed + (os_short)change_max;
    }
    else {
        speed = 0;
    }
    if (speed != m_speed) {
        if (change_it) {
            setpropertyl(ECOMP_GC_SPEED, speed);
        }
        rval = OS_TRUE;
    }

    /* Turn change.
     */
    change_max = 0.000001 * elapsed_us * turn_change_per_sec;
    if (m_turn > change_max) {
        turn = m_turn - (os_short)change_max;
    }
    else if (m_turn < -change_max) {
        turn = m_turn + (os_short)change_max;
    }
    else {
        turn = 0;
    }
    if (turn != m_turn) {
        if (change_it) {
            setpropertyl(ECOMP_GC_TURN, turn);
        }
        rval = OS_TRUE;
    }

    return rval;
}


/**
****************************************************************************************************

  @brief Component clicked.

  The eGameController::on_click() function is called when a component is clicked. If the component
  processess the mouse click, it returns OS_TRUE. This indicates that the click has been
  processed. If it doesn't process the click, it call's eComponent base classess'es on_click()
  function to try if base class wants to process the click.
  When the mouse click is not processed, it is passed to parent object in z order.

  @param   prm Drawing parameters, notice especially edit_mode.
  @param   mouse_gamecontroller_nr Which mouse gamecontroller, for example EIMGUI_LEFT_MOUSE_GAME_CONTROLLER.

  @return  OS_TRUE if mouse click was processed by this component, or OS_FALSE if not.

****************************************************************************************************
*/
os_boolean eGameController::on_click(
    eDrawParams& prm,
    os_int mouse_gamecontroller_nr)
{
    if (!prm.edit_mode && mouse_gamecontroller_nr == EIMGUI_LEFT_MOUSE_BUTTON) {
        return OS_TRUE;
    }
    return eComponent::on_click(prm, mouse_gamecontroller_nr);
}


