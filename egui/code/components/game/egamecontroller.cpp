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
    os_int i;

    m_alive = 0;
    for (i = 0; i< m_nro_thumbsticks; i++) {
        m_TX[i] = m_TY[i] = 0;
        m_T[i] = m_T1[i] = m_T2[i] = OS_FALSE;
    }
    m_X = m_Y = m_A = m_B = OS_FALSE;
    m_DU = m_DL = m_DR = m_DD = OS_FALSE;
    m_back = m_start = m_guide = OS_FALSE;

    os_get_timer(&m_update_timer);
    m_left_timer_press = m_update_timer;
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
    addpropertys(cls, ECOMP_GC_MSG, ecomp_gc_msg, ecomp_gc_msg, EPRO_DEFAULT);
    addproperty (cls, ECOMP_GC_COLOR, ecomp_gc_color, ecomp_gc_color, EPRO_SIMPLE);
    addpropertyl(cls, ECOMP_GC_ALIVE, ecomp_gc_alive, ecomp_gc_alive, EPRO_SIMPLE);

    v = addpropertyl(cls, ECOMP_GC_LX, ecomp_gc_LX, ecomp_gc_LX, EPRO_SIMPLE);
    v->setpropertyl(EVARP_MIN, -m_range_max);
    v->setpropertyl(EVARP_MAX, m_range_max);
    v->setpropertys(EVARP_UNIT, "deg");
    v->setpropertys(EVARP_TTIP, "Left thumbstick X: -10000 (left) .. 10000 (right)");
    v = addpropertyl(cls, ECOMP_GC_LY, ecomp_gc_LY, ecomp_gc_LY, EPRO_SIMPLE);
    v->setpropertyl(EVARP_MIN, -m_range_max);
    v->setpropertyl(EVARP_MAX, m_range_max);
    v->setpropertys(EVARP_UNIT, "%");
    v->setpropertys(EVARP_TTIP, "Ledt thumbstick Y: -10000 (back) .. 10000 (forward)");
    addpropertyb(cls, ECOMP_GC_L, ecomp_gc_L, ecomp_gc_L, EPRO_SIMPLE);
    addpropertyb(cls, ECOMP_GC_L1, ecomp_gc_L1, ecomp_gc_L1, EPRO_SIMPLE);
    addpropertyb(cls, ECOMP_GC_L2, ecomp_gc_L2, ecomp_gc_L2, EPRO_SIMPLE);

    v = addpropertyl(cls, ECOMP_GC_RX, ecomp_gc_RX, ecomp_gc_RX, EPRO_SIMPLE);
    v->setpropertyl(EVARP_MIN, -m_range_max);
    v->setpropertyl(EVARP_MAX, m_range_max);
    v->setpropertys(EVARP_UNIT, "deg");
    v->setpropertys(EVARP_TTIP, "1/100 percents of max, -10000 .. 10000");
    v = addpropertyl(cls, ECOMP_GC_RY, ecomp_gc_RY, ecomp_gc_RY, EPRO_SIMPLE);
    v->setpropertyl(EVARP_MIN, -m_range_max);
    v->setpropertyl(EVARP_MAX, m_range_max);
    v->setpropertys(EVARP_UNIT, "%");
    v->setpropertys(EVARP_TTIP, "1/100 percents of max, -10000 .. 10000");
    addpropertyb(cls, ECOMP_GC_R, ecomp_gc_R, ecomp_gc_R, EPRO_SIMPLE);
    addpropertyb(cls, ECOMP_GC_R1, ecomp_gc_R1, ecomp_gc_R1, EPRO_SIMPLE);
    addpropertyb(cls, ECOMP_GC_R2, ecomp_gc_R2, ecomp_gc_R2, EPRO_SIMPLE);

    addpropertyb(cls, ECOMP_GC_X, ecomp_gc_X, ecomp_gc_X, EPRO_SIMPLE);
    addpropertyb(cls, ECOMP_GC_Y, ecomp_gc_Y, ecomp_gc_Y, EPRO_SIMPLE);
    addpropertyb(cls, ECOMP_GC_A, ecomp_gc_A, ecomp_gc_A, EPRO_SIMPLE);
    addpropertyb(cls, ECOMP_GC_B, ecomp_gc_B, ecomp_gc_B, EPRO_SIMPLE);

    addpropertyl(cls, ECOMP_GC_DU, ecomp_gc_DU, ecomp_gc_DU, EPRO_SIMPLE);
    addpropertyl(cls, ECOMP_GC_DU, ecomp_gc_DL, ecomp_gc_DL, EPRO_SIMPLE);
    addpropertyl(cls, ECOMP_GC_DU, ecomp_gc_DR, ecomp_gc_DR, EPRO_SIMPLE);
    addpropertyl(cls, ECOMP_GC_DU, ecomp_gc_DD, ecomp_gc_DD, EPRO_SIMPLE);

    addpropertyl(cls, ECOMP_GC_BACK, ecomp_gc_back, ecomp_gc_back, EPRO_SIMPLE);
    addpropertyl(cls, ECOMP_GC_START, ecomp_gc_start, ecomp_gc_start, EPRO_SIMPLE);
    addpropertyl(cls, ECOMP_GC_GUIDE, ecomp_gc_guide, ecomp_gc_guide, EPRO_SIMPLE);

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
        case ECOMP_GC_MSG:
            m_text.clear();
            break;

        case ECOMP_GC_COLOR:
            break;

        case ECOMP_GC_ALIVE:
            m_alive = x->getl();
            break;

        case ECOMP_GC_LX:
            m_TX[0] = x->getl();
            break;

        case ECOMP_GC_LY:
            m_TY[0] = x->getl();
            break;

        case ECOMP_GC_L:
            m_T[0] = (os_boolean)x->getl();
            break;

        case ECOMP_GC_L1:
            m_T1[0] = (os_boolean)x->getl();
            break;

        case ECOMP_GC_L2:
            m_T2[0] = (os_boolean)x->getl();
            break;

        case ECOMP_GC_RX:
            m_TX[1] = x->getl();
            break;

        case ECOMP_GC_RY:
            m_TY[1] = x->getl();
            break;

        case ECOMP_GC_R:
            m_T[1] = (os_boolean)x->getl();
            break;

        case ECOMP_GC_R1:
            m_T1[1] = (os_boolean)x->getl();
            break;

        case ECOMP_GC_R2:
            m_T2[1] = (os_boolean)x->getl();
            break;

        case ECOMP_GC_X:
            m_X = (os_boolean)x->getl();
            break;

        case ECOMP_GC_Y:
            m_Y = (os_boolean)x->getl();
            break;

        case ECOMP_GC_A:
            m_A = (os_boolean)x->getl();
            break;

        case ECOMP_GC_B:
            m_B = (os_boolean)x->getl();
            break;

        case ECOMP_GC_DU:
            m_DU = (os_boolean)x->getl();
            break;

        case ECOMP_GC_DL:
            m_DL = (os_boolean)x->getl();
            break;

        case ECOMP_GC_DR:
            m_DR = (os_boolean)x->getl();
            break;

        case ECOMP_GC_DD:
            m_DD = (os_boolean)x->getl();
            break;

        case ECOMP_GC_BACK:
            m_back = (os_boolean)x->getl();
            break;

        case ECOMP_GC_START:
            m_start = (os_boolean)x->getl();
            break;

        case ECOMP_GC_GUIDE:
            m_guide = (os_boolean)x->getl();
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

        case ECOMP_GC_LX:
            x->setl(m_TX[0]);
            break;

        case ECOMP_GC_LY:
            x->setl(m_TY[0]);
            break;

        case ECOMP_GC_L:
            x->setl(m_T[0]);
            break;

        case ECOMP_GC_L1:
            x->setl(m_T1[0]);
            break;

        case ECOMP_GC_L2:
            x->setl(m_T2[0]);
            break;

        case ECOMP_GC_RX:
            x->setl(m_TX[1]);
            break;

        case ECOMP_GC_RY:
            x->setl(m_TY[1]);
            break;

        case ECOMP_GC_R:
            x->setl(m_T[1]);
            break;

        case ECOMP_GC_R1:
            x->setl(m_T1[1]);
            break;

        case ECOMP_GC_R2:
            x->setl(m_T2[1]);
            break;

        case ECOMP_GC_X:
            x->setl(m_X);
            break;

        case ECOMP_GC_Y:
            x->setl(m_Y);
            break;

        case ECOMP_GC_A:
            x->setl(m_A);
            break;

        case ECOMP_GC_B:
            x->setl(m_B);
            break;

        case ECOMP_GC_DU:
            x->setl(m_DU);
            break;

        case ECOMP_GC_DL:
            x->setl(m_DL);
            break;

        case ECOMP_GC_DR:
            x->setl(m_DR);
            break;

        case ECOMP_GC_DD:
            x->setl(m_DD);
            break;

        case ECOMP_GC_BACK:
            x->setl(m_back);
            break;

        case ECOMP_GC_START:
            x->setl(m_start);
            break;

        case ECOMP_GC_GUIDE:
            x->setl(m_guide);
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
    float TY, TX, xdelta, ydelta;
    float xcoeff, ycoeff, xorigin, yorigin, left, right, top, bottom, x, y;
    os_boolean setting_motion;
    os_int i;
    os_long elapsed_us;

    add_to_zorder(prm.window, prm.layer);

    ImVec2 cpos = ImGui::GetCursorScreenPos();
    m_rect.x1 = (os_int)cpos.x;
    m_rect.y1 = (os_int)cpos.y;

    sz.x = sz.y = 0;

    ImDrawList *draw_list = ImGui::GetWindowDrawList();
    ImVec2 r = ImGui::GetContentRegionAvail();


    xcoeff = r.x / (float)(2 * m_range_max);
    ycoeff = -r.y / (float)(2 * m_range_max);
    if (xcoeff == 0.0 && ycoeff == 0.0) return ESTATUS_SUCCESS;
    xorigin = cpos.x + xcoeff * m_range_max;
    yorigin = cpos.y - ycoeff * m_range_max;
    left = xorigin - xcoeff * m_range_max;
    right = xorigin + xcoeff * m_range_max;
    top = yorigin - ycoeff * m_range_max;
    bottom = yorigin + ycoeff * m_range_max;

    ImU32  col = IM_COL32(128, 128, 128, 128);
    draw_list->AddLine(ImVec2(left, yorigin), ImVec2(right, yorigin), col, 1.0f /* thickness */);
    draw_list->AddLine(ImVec2(xorigin, top), ImVec2(xorigin, bottom), col, 1.0f /* thickness */);

    /* Time to update to slide home?
     */
    elapsed_us = prm.timer_us - m_update_timer;
    if (elapsed_us < 10000) elapsed_us = 0;
    if (elapsed_us > 1000000) elapsed_us = 1000000;
    if (elapsed_us) {
        m_update_timer = prm.timer_us;
    }

    setting_motion = -1;
    if (ImGui::IsWindowHovered())
    {
        if (prm.mouse_left_press) {
            setting_motion = 0;
            m_left_timer_press = prm.timer_us;
        }
        else if (prm.mouse_click[EIMGUI_RIGHT_MOUSE_BUTTON] &&
            prm.timer_us > m_left_timer_press + 100000)
        {
            setting_motion = 1;
            prm.mouse_click[EIMGUI_RIGHT_MOUSE_BUTTON] = OS_FALSE;
        }
    }

    for (i = 0; i < m_nro_thumbsticks; i++)
    {
        if (setting_motion == i)
        {
            TX = (prm.mouse_pos.x - xorigin) / xcoeff;
            xdelta = TX - m_TX[i];
            TY = (prm.mouse_pos.y - yorigin) / ycoeff;
            ydelta = TY - m_TY[i];
            if (TX >= -m_range_max && TX <= m_range_max &&
                TY >= -m_range_max && TY <= m_range_max)
            {
                if (xdelta*xdelta > 100)
                    setpropertyl(i ? ECOMP_GC_RX : ECOMP_GC_LX, os_round_short(TX));

                if (ydelta*ydelta > 100)
                    setpropertyl(i ? ECOMP_GC_RY : ECOMP_GC_LY, os_round_short(TY));
            }
        }

        if (i == 0 && setting_motion != 0 && elapsed_us) {
            autocenter_thumbstick(i, elapsed_us);
        }

        if (m_TX[i] != 0.0f || m_TY[i] != 0.0f) {
            x = xorigin + xcoeff * m_TX[i];
            y = yorigin + ycoeff * m_TY[i];
            col = i ? IM_COL32(255, 255, 90, 250) : IM_COL32(90, 255, 90, 250);
            draw_list->AddLine(ImVec2(left, y), ImVec2(right, y), col, 2.0f /* thickness */);
            draw_list->AddLine(ImVec2(x, top), ImVec2(x, bottom), col, 2.0f /* thickness */);
        }
    }

    sz.x = r.x;
    sz.x = r.y;

    m_rect.x2 = m_rect.x1 + (os_int)sz.x - 1;
    m_rect.y2 = m_rect.y1 + (os_int)sz.y - 1;

    /* Let base class implementation handle the rest.
     */
    return eComponent::draw(prm);
}


/**
****************************************************************************************************

  @brief Auto center left thumbstick.

  The GameController::autocenter_thumbstick() function moves left thumbstick (simulation)
  back to center 0,0 once mouse is received.

  @param   thumbstick_nr 0 for left thumbstick, 1 for right.
  @param   elapsed_us How long has elaapsed since last update, microseconds.

****************************************************************************************************
*/
void eGameController::autocenter_thumbstick(
    os_int thumbstick_nr0,
    os_long elapsed_us)
{
    const os_double TY_change_per_sec = 0.7 * m_range_max;
    const os_double TX_change_per_sec = 0.35  * m_range_max;
    os_double change_max;
    os_short TY, TX;

    /* Speed change (0.000001: us->s).
     */
    change_max = 0.000001 * elapsed_us * TY_change_per_sec;
    if (m_TY[thumbstick_nr0] > change_max) {
        TY = m_TY[thumbstick_nr0] - (os_short)change_max;
    }
    else if (m_TY[thumbstick_nr0] < -change_max) {
        TY = m_TY[thumbstick_nr0] + (os_short)change_max;
    }
    else {
        TY = 0;
    }
    if (TY != m_TY[thumbstick_nr0]) {
        setpropertyl(thumbstick_nr0 ? ECOMP_GC_RY : ECOMP_GC_LY, TY);
    }

    /* Turn change (0.000001: us->s).
     */
    change_max = 0.000001 * elapsed_us * TX_change_per_sec;
    if (m_TX[thumbstick_nr0] > change_max) {
        TX = m_TX[thumbstick_nr0] - (os_short)change_max;
    }
    else if (m_TX[thumbstick_nr0] < -change_max) {
        TX = m_TX[thumbstick_nr0] + (os_short)change_max;
    }
    else {
        TX = 0;
    }
    if (TX != m_TX[thumbstick_nr0]) {
        setpropertyl(thumbstick_nr0 ? ECOMP_GC_RX : ECOMP_GC_LX, TX);
    }
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
  @param   mouse_button_nr Which mouse button, for example EIMGUI_LEFT_MOUSE_BUTTON.

  @return  OS_TRUE if mouse click was processed by this component, or OS_FALSE if not.

****************************************************************************************************
*/
os_boolean eGameController::on_click(
    eDrawParams& prm,
    os_int mouse_button_nr)
{
    if (!prm.edit_mode && mouse_button_nr == EIMGUI_LEFT_MOUSE_BUTTON) {
        return OS_TRUE;
    }
    return eComponent::on_click(prm, mouse_button_nr);
}


