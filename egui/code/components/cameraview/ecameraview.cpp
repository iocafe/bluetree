/**

  @file    ecameraview.cpp
  @brief   Display camera, etc, live bitmap based image.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

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
eCameraView::eCameraView(
    eObject *parent,
    e_oid id,
    os_int flags)
    : eComponent(parent, id, flags)
{
    m_textureID = 0;
    m_textureID_set = OS_FALSE;
    m_texture_w = m_texture_h = 0;
}


/**
****************************************************************************************************
  Virtual destructor.
****************************************************************************************************
*/
eCameraView::~eCameraView()
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
eObject *eCameraView::clone(
    eObject *parent,
    e_oid id,
    os_int aflags)
{
    eCameraView *clonedobj;
    clonedobj = new eCameraView(parent, id == EOID_CHILD ? oid() : id, flags());
    clonegeneric(clonedobj, aflags);
    return clonedobj;
}


/**
****************************************************************************************************

  @brief Add eCameraView to class list and class'es properties to it's property set.

  The eCameraView::setupclass function adds eCameraView to class list and class'es properties to
  it's property set. The class list enables creating new objects dynamically by class identifier,
  which is used for serialization reader functions. The property set stores static list of
  class'es properties and metadata for those.

****************************************************************************************************
*/
void eCameraView::setupclass()
{
    const os_int cls = EGUICLASSID_CAMERA_VIEW;

    os_lock();
    eclasslist_add(cls, (eNewObjFunc)newobj, "eCameraView", EGUICLASSID_COMPONENT);
    setupproperties(cls, ECOMP_NO_OPTIONAL_PROPERITES);
    addpropertys(cls, ECOMP_TEXT, ecomp_text, "text", EPRO_METADATA);
    addproperty (cls, ECOMP_VALUE, ecomp_value, "value");
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
eStatus eCameraView::onpropertychange(
    os_int propertynr,
    eVariable *x,
    os_int flags)
{
    eObject *bitmap;

    switch (propertynr)
    {
        case ECOMP_VALUE:
            bitmap = x->geto();
            if (bitmap) if (bitmap->isinstanceof(ECLASSID_BITMAP)) {
                upload_texture_to_grahics_card((eBitmap*)bitmap);
            }
            break;

        case ECOMP_TEXT:
            m_text.clear();
            break;

        default:
            return eComponent::onpropertychange(propertynr, x, flags);
    }

    return ESTATUS_SUCCESS;
}


/**
****************************************************************************************************

  @brief Draw the component.

  The eCameraView::draw() function calls ImGui API to render the component.

  @param   Prm Drawing parameters.
  @return  The function return ESTATUS_SUCCESS if all is fine. Other values indicate that the
           component is no longer drawable or useful. This could be for example a pop up menu
           closed implicitely by clicking elsewhere.

****************************************************************************************************
*/
eStatus eCameraView::draw(
    eDrawParams& prm)
{
    ImVec2 sz;

    add_to_zorder(prm.window, prm.layer);

    ImVec2 cpos = ImGui::GetCursorScreenPos();
    m_rect.x1 = (os_int)cpos.x;
    m_rect.y1 = (os_int)cpos.y;

    sz.x = sz.y = 0;

    // label = m_text.get(this, ECOMP_TEXT);

    if (m_textureID_set)
    {
        ImVec2 r = ImGui::GetContentRegionAvail();
        r.x -= 2;
        r.y -= 2;
//         ImGui::Text("%.0fx%.0f", (float)m_texture_w, (float)m_texture_h);
        //ImVec2 pos = ImGui::GetCursorScreenPos();
        ImVec2 uv_min = ImVec2(0.0f, 0.0f);                 // Top-left
        ImVec2 uv_max = ImVec2(1.0f, 1.0f);                 // Lower-right
        ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);   // No tint
        ImVec4 border_col = ImVec4(0.0f, 1.0f, 1.0f, 0.5f); // 50% opaque white

        ImGui::Image(m_textureID, r /* ImVec2(m_texture_w, m_texture_h) */, uv_min, uv_max, tint_col, border_col);
        /* if (ImGui::IsItemHovered())
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
        } */
    }

    sz = ImGui::GetItemRectSize();

    m_rect.x2 = m_rect.x1 + (os_int)sz.x - 1;
    m_rect.y2 = m_rect.y1 + (os_int)sz.y - 1;

    /* Let base class implementation handle the rest.
     */
    return eComponent::draw(prm);
}


/**
****************************************************************************************************

  @brief Component clicked.

  The eCameraView::on_click() function is called when a component is clicked. If the component
  processess the mouse click, it returns OS_TRUE. This indicates that the click has been
  processed. If it doesn't process the click, it call's eComponent base classess'es on_click()
  function to try if base class wants to process the click.
  When the mouse click is not processed, it is passed to parent object in z order.

  @param   prm Drawing parameters, notice especially edit_mode.
  @param   mouse_cameraview_nr Which mouse cameraview, for example EIMGUI_LEFT_MOUSE_CAMERA_VIEW.

  @return  OS_TRUE if mouse click was processed by this component, or OS_FALSE if not.

****************************************************************************************************
*/
os_boolean eCameraView::on_click(
    eDrawParams& prm,
    os_int mouse_cameraview_nr)
{
    if (!prm.edit_mode && mouse_cameraview_nr == EIMGUI_LEFT_MOUSE_BUTTON) {
        return OS_TRUE;
    }
    return eComponent::on_click(prm, mouse_cameraview_nr);
}


/**
****************************************************************************************************

  @brief Start editing value, toggle checkbox or show drop down list.

  The eCameraView::activate() function is called when a value is clicked, or key (for example
  spacebar) is hit to start editing the value. Actual operation depends on metadata, the
  function can either start value edit, toggle a checkbox or show drop down list.

  @return  None.

****************************************************************************************************
*/
void eCameraView::activate()
{
    /* eVariable target;

    propertyv(ECOMP_TARGET, &target);
    if (!target.isempty()){
        eVariable value;
        propertyv(ECOMP_SETVALUE, &value);
        setpropertyv_msg(target.gets(), &value);
    } */
}


/**
****************************************************************************************************

  @brief Load a bitmap to graphics card.

  X...

****************************************************************************************************
*/
void eCameraView::upload_texture_to_grahics_card(
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
void eCameraView::delete_texture_on_grahics_card()
{
    if (m_textureID_set) {
        eimgui_delete_texture_on_grahics_card(m_textureID);
        m_textureID_set = 0;
        m_textureID = 0;
    }
}

