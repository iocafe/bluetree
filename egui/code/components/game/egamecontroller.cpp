/**

  @file    egamecontroller.cpp
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
    m_textureID = 0;
    m_textureID_set = OS_FALSE;
    m_texture_w = m_texture_h = 0;

generate_bitmap();
}


/**
****************************************************************************************************
  Virtual destructor.
****************************************************************************************************
*/
eGameController::~eGameController()
{
    delete_texture_on_grahics_card();
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
    v->setpropertys(EVARP_TTIP, "1/100 degrees, -9000 (right) .. 9000 (left)");
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

    add_to_zorder(prm.window, prm.layer);

    ImVec2 cpos = ImGui::GetCursorScreenPos();
    m_rect.x1 = (os_int)cpos.x;
    m_rect.y1 = (os_int)cpos.y;

    sz.x = sz.y = 0;

//ImVec4 backgr_col = ImVec4(0.0f, 1.0f, 1.0f, 0.5f); // 50% opaque white

//ImGui::PushStyleColor(ImGuiCol_WindowBg, backgr_col); //<---Here


    // label = m_text.get(this, ECOMP_TEXT);

    if (m_textureID_set)
    {
        ImVec2 r = ImGui::GetContentRegionAvail();
        r.x -= 2;
        r.y -= 2;
//         ImGui::Text("%.0fx%.0f", (float)m_texture_w, (float)m_texture_h);
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImVec2 uv_min = ImVec2(0.0f, 0.0f);                 // Top-left
        ImVec2 uv_max = ImVec2(1.0f, 1.0f);                 // Lower-right
        ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);   // No tint
        ImVec4 border_col = ImVec4(0.0f, 1.0f, 1.0f, 0.5f); // 50% opaque white

        ImGui::Image(m_textureID, r /* ImVec2(m_texture_w, m_texture_h) */, uv_min, uv_max, tint_col, border_col);
        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            float region_sz = 32.0f;
            float region_x = prm.io->MousePos.x - pos.x - region_sz * 0.5f;
            float region_y = prm.io->MousePos.y - pos.y - region_sz * 0.5f;
            float zoom = 4.0f;
            if (region_x < 0.0f) { region_x = 0.0f; }
            else if (region_x > m_texture_w - region_sz) { region_x = m_texture_w - region_sz; }
            if (region_y < 0.0f) { region_y = 0.0f; }
            else if (region_y > m_texture_h - region_sz) { region_y = m_texture_h - region_sz; }
            ImGui::Text("Min: (%.2f, %.2f)", region_x, region_y);
            ImGui::Text("Max: (%.2f, %.2f)", region_x + region_sz, region_y + region_sz);
            ImVec2 uv0 = ImVec2((region_x) / m_texture_w, (region_y) / m_texture_h);
            ImVec2 uv1 = ImVec2((region_x + region_sz) / m_texture_w, (region_y + region_sz) / m_texture_h);
            ImGui::Image(m_textureID, ImVec2(region_sz * zoom, region_sz * zoom), uv0, uv1); // , tint_col, border_col);
            ImGui::EndTooltip();
        }
    }

    sz = ImGui::GetItemRectSize();

    m_rect.x2 = m_rect.x1 + (os_int)sz.x - 1;
    m_rect.y2 = m_rect.y1 + (os_int)sz.y - 1;

//ImGui::PopStyleColor();

    /* Let base class implementation handle the rest.
     */
    return eComponent::draw(prm);

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


/**
****************************************************************************************************

  @brief Start editing value, toggle checkbox or show drop down list.

  The eGameController::activate() function is called when a value is clicked, or key (for example
  spacebar) is hit to start editing the value. Actual operation depends on metadata, the
  function can either start value edit, toggle a checkbox or show drop down list.

  @return  None.

****************************************************************************************************
*/
void eGameController::activate()
{
    /* eVariable target;

    propertyv(ECOMP_TARGET, &target);
    if (!target.isempty()){
        eVariable value;
        propertyv(ECOMP_SETVALUE, &value);
        setpropertyv_msg(target.gets(), &value);
    } */
}


void eGameController::generate_bitmap()
{
    eBitmap *bitmap;
    os_uint *data, *p;

    union {
        os_uint u;
        os_uchar component[4];
    }
    value;

    os_int w = 640, h = 480, line_d = 1;
    os_int center_x, center_y, x, y, d, q;
    os_double max_d, v;

    bitmap = new eBitmap(ETEMPORARY);
    bitmap->allocate(OSAL_RGBA32, w, h);
    data = (os_uint*)bitmap->ptr();

    value.component[0] = 16;
    value.component[1] = 192;
    value.component[2] = 16;

    center_x = w/2;
    center_y = h/2;

    for (y = -line_d; y <= line_d; y++)
    {
        d = y;
        if (d < 0) d = -d;

        p = data + (center_y + y) * w;
        for (x = 0; x < w; x++) {
            q = x - center_x;
            if (q < 0) q = -q;

            max_d = line_d * (os_double) (center_x - q + 1) / (os_double) (center_x + 3);
            if (max_d <= 0.0) continue;
            v = 250.0 * (max_d - d + 1) / (max_d + 1.0);
            if (v > 2.0) {
                value.component[3] = (os_uchar)v;
                p[x] = value.u;
            }
        }
    }

    for (x = -line_d; x <= line_d; x++)
    {
        d = x;
        if (d < 0) d = -d;

        p = data + center_x + x;
        for (y = 0; y < h; y++) {
            q = y - center_y;
            if (q < 0) q = -q;

            max_d = line_d * (os_double) (center_y - q + 1) / (os_double) (center_y + 3);
            if (max_d <= 0.0) continue;
            v = 250.0 * (max_d - d + 1) / (max_d + 1.0);
            if (v > 2.0) {
                value.component[3] = (os_uchar)v;
                if (p[y * w] < value.u) p[y * w] = value.u;
            }
        }
    }

    upload_texture_to_grahics_card(bitmap);


    delete bitmap;
}


/**
****************************************************************************************************

  @brief Load a bitmap to graphics card.

  X...

****************************************************************************************************
*/
void eGameController::upload_texture_to_grahics_card(
    eBitmap *bitmap)
{
    const os_uchar *data;
    osalBitmapFormat format;
    os_int byte_width;
    eStatus s;

    delete_texture_on_grahics_card();

    data = bitmap->ptr();
    m_texture_w = bitmap->width();
    m_texture_h = bitmap->height();
    format = bitmap->format();
    byte_width = bitmap->row_nbytes();

    s = eimgui_upload_texture_to_grahics_card(data, m_texture_w, m_texture_h, format, byte_width, &m_textureID);
    m_textureID_set = (os_boolean)(s == ESTATUS_SUCCESS);
}


/**
****************************************************************************************************

  @brief Delete a texture (bitmap) from graphics card.

  X...

****************************************************************************************************
*/
void eGameController::delete_texture_on_grahics_card()
{
    if (m_textureID_set) {
        eimgui_delete_texture_on_grahics_card(m_textureID);
        m_textureID_set = 0;
        m_textureID = 0;
    }
}

