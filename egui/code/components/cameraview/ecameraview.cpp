/**

  @file    ecameraview.cpp
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

os_uint textureID;
int image_width = 512;
int image_height = 512;
static bool LoadTextureXXX();

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
    m_imgui_toggl = false;
    m_set_toggled = true;
}


/**
****************************************************************************************************
  Virtual destructor.
****************************************************************************************************
*/
eCameraView::~eCameraView()
{
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
    addpropertyl(cls, ECOMP_SETVALUE, ecomp_setvalue, OS_TRUE, "set value", EPRO_METADATA);
    addpropertys(cls, ECOMP_TARGET, ecomp_target, "target", EPRO_METADATA);

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
    switch (propertynr)
    {
        case ECOMP_VALUE:
            m_set_toggled = true;
            break;

        case ECOMP_TEXT:
            m_text.clear();
            break;

        case ECOMP_SETVALUE:
            m_set_toggled = true;
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
    eComponent *c;
    const os_char *label;
    ImVec2 sz;
    os_int cid;

    add_to_zorder(prm.window, prm.layer);

    if (m_set_toggled) {
        set_toggled();
        m_set_toggled = false;
    }

    ImVec2 cpos = ImGui::GetCursorScreenPos();
    m_rect.x1 = (os_int)cpos.x;
    m_rect.y1 = (os_int)cpos.y;

    sz.x = sz.y = 0;

    label = m_text.get(this, ECOMP_TEXT);

    LoadTextureXXX();
    ImTextureID my_tex_id = (ImTextureID)textureID;
    float my_tex_w = (float)image_width;
    float my_tex_h = (float)image_height;

/*         ImTextureID my_tex_id = prm.io->Fonts->TexID;
        float my_tex_w = (float)prm.io->Fonts->TexWidth;
        float my_tex_h = (float)prm.io->Fonts->TexHeight; */
        {
            ImGui::Text("%.0fx%.0f", my_tex_w, my_tex_h);
            ImVec2 pos = ImGui::GetCursorScreenPos();
            ImVec2 uv_min = ImVec2(0.0f, 0.0f);                 // Top-left
            ImVec2 uv_max = ImVec2(1.0f, 1.0f);                 // Lower-right
            ImVec4 tint_col = ImVec4(1.0f, 0.0f, 1.0f, 1.0f);   // No tint
            ImVec4 border_col = ImVec4(0.0f, 1.0f, 1.0f, 0.5f); // 50% opaque white
            ImGui::Image(my_tex_id, ImVec2(my_tex_w, my_tex_h), uv_min, uv_max, tint_col, border_col);
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                float region_sz = 32.0f;
                float region_x = prm.io->MousePos.x - pos.x - region_sz * 0.5f;
                float region_y = prm.io->MousePos.y - pos.y - region_sz * 0.5f;
                float zoom = 4.0f;
                if (region_x < 0.0f) { region_x = 0.0f; }
                else if (region_x > my_tex_w - region_sz) { region_x = my_tex_w - region_sz; }
                if (region_y < 0.0f) { region_y = 0.0f; }
                else if (region_y > my_tex_h - region_sz) { region_y = my_tex_h - region_sz; }
                ImGui::Text("Min: (%.2f, %.2f)", region_x, region_y);
                ImGui::Text("Max: (%.2f, %.2f)", region_x + region_sz, region_y + region_sz);
                ImVec2 uv0 = ImVec2((region_x) / my_tex_w, (region_y) / my_tex_h);
                ImVec2 uv1 = ImVec2((region_x + region_sz) / my_tex_w, (region_y + region_sz) / my_tex_h);
                ImGui::Image(my_tex_id, ImVec2(region_sz * zoom, region_sz * zoom), uv0, uv1, tint_col, border_col);
                ImGui::EndTooltip();
            }
        }

#if 0
    /* If this component has children, it contains a sub menu.
     */
    c = firstcomponent();
    if (c) {
        if (ImGui::BeginMenu(label))
        {
            while (c) {
                c->draw(prm);
                c = c->nextcomponent();
            }

            ImGui::EndMenu();
        }
    }
    else {
        cid = parent()->classid();
        if (cid == EGUICLASSID_POPUP || cid == EGUICLASSID_CAMERA_VIEW)
        {
            if (ImGui::MenuItem(label, "", &m_imgui_toggl)) {
                activate();
            }
        }
        else {
            os_int www = (os_int)ImGui::CalcTextSize(label).x;
            ImVec2 inner_spacing = ImGui::GetStyle().ItemInnerSpacing;
            ImVec2 cameraview_padding(50, inner_spacing.y);
            cameraview_padding.x -= www/2;
            if (cameraview_padding.x < inner_spacing.x) {
                cameraview_padding.x = inner_spacing.x;
            }
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, cameraview_padding);

            if (m_imgui_toggl)
            {
                ImGui::PushID(label);
                ImGui::PushStyleColor(ImGuiCol_Button, ImGui::ColorConvertFloat4ToU32(ImVec4(0.3f, 0.7f, 0.5f, 1.0f)));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::ColorConvertFloat4ToU32(ImVec4(0.5f, 0.9f, 0.7f, 1.0f)));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::ColorConvertFloat4ToU32(ImVec4(0.3f, 0.7f, 0.5f, 1.0f)));
                ImGui::Button(label);
                if (ImGui::IsItemClicked(0))
                {
                    setpropertyl(ECOMP_VALUE, !propertyi(ECOMP_SETVALUE));
                }
                ImGui::PopStyleColor(3);
                ImGui::PopID();
            }
            else
            {

                ImGui::Button(label);
                if (ImGui::IsItemClicked(0)) {
                    eVariable tmp;
                    propertyv(ECOMP_SETVALUE, &tmp);
                    setpropertyv(ECOMP_VALUE, &tmp);
                    activate();
                }
            }
            ImGui::PopStyleVar();
        }
    }
#endif

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
    eVariable target;

    propertyv(ECOMP_TARGET, &target);
    if (!target.isempty()){
        eVariable value;
        propertyv(ECOMP_SETVALUE, &value);
        setpropertyv_msg(target.gets(), &value);
    }
}


/**
****************************************************************************************************

  @brief Set value for ImGui toggle mark, when needed.

  The set_toggled() function is called when drawing to set value to determine value for
  m_imgui_toggl boolean. Pointer to this boolean is passed to the ImGui to inform wether
  to draw a check mark in menu cameraview.
  @return  None.

****************************************************************************************************
*/
void eCameraView::set_toggled()
{
    eVariable tmp1, tmp2;

    propertyv(ECOMP_VALUE, &tmp1);
    propertyv(ECOMP_SETVALUE, &tmp2);

    m_imgui_toggl = tmp1.compare(&tmp2) ? false : true;
}



#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#if defined(_MSC_VER) && _MSC_VER <= 1500 // MSVC 2008 or earlier
#include <stddef.h>     // intptr_t
#else
#include <stdint.h>     // intptr_t
#endif

// GL includes
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#elif defined(IMGUI_IMPL_OPENGL_ES3)
#if (defined(__APPLE__) && (TARGET_OS_IOS || TARGET_OS_TV))
#include <OpenGLES/ES3/gl.h>    // Use GL ES 3
#else
#include <GLES3/gl3.h>          // Use GL ES 3
#endif
#else
// About Desktop OpenGL function loaders:
//  Modern desktop OpenGL doesn't have a standard portable header file to load OpenGL function pointers.
//  Helper libraries are often used for this purpose! Here we are supporting a few common ones (gl3w, glew, glad).
//  You may use another loader/header of your choice (glext, glLoadGen, etc.), or chose to manually implement your own.
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#include <GL/gl3w.h>            // Needs to be initialized with gl3wInit() in user's code
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h>            // Needs to be initialized with glewInit() in user's code.
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h>          // Needs to be initialized with gladLoadGL() in user's code.
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD2)
#include <glad/gl.h>            // Needs to be initialized with gladLoadGL(...) or gladLoaderLoadGL() in user's code.
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE       // GLFW including OpenGL headers causes ambiguity or multiple definition errors.
#endif
#include <glbinding/Binding.h>  // Needs to be initialized with glbinding::Binding::initialize() in user's code.
#include <glbinding/gl/gl.h>
using namespace gl;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE       // GLFW including OpenGL headers causes ambiguity or multiple definition errors.
#endif
#include <glbinding/glbinding.h>// Needs to be initialized with glbinding::initialize() in user's code.
#include <glbinding/gl/gl.h>
using namespace gl;
#else
#include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#endif
#endif

// Desktop GL 3.2+ has glDrawElementsBaseVertex() which GL ES and WebGL don't have.
#if !defined(IMGUI_IMPL_OPENGL_ES2) && !defined(IMGUI_IMPL_OPENGL_ES3) && defined(GL_VERSION_3_2)
#define IMGUI_IMPL_OPENGL_MAY_HAVE_VTX_OFFSET
#endif

// Desktop GL 3.3+ has glBindSampler()
#if !defined(IMGUI_IMPL_OPENGL_ES2) && !defined(IMGUI_IMPL_OPENGL_ES3) && defined(GL_VERSION_3_3)
#define IMGUI_IMPL_OPENGL_MAY_HAVE_BIND_SAMPLER
#endif

// Desktop GL 3.1+ has GL_PRIMITIVE_RESTART state
#if !defined(IMGUI_IMPL_OPENGL_ES2) && !defined(IMGUI_IMPL_OPENGL_ES3) && defined(GL_VERSION_3_1)
#define IMGUI_IMPL_OPENGL_MAY_HAVE_PRIMITIVE_RESTART
#endif


// Simple helper function to load an image into a OpenGL texture with common settings
static bool LoadTextureXXX()
{
    static int done = 0;

// SEE https://github.com/ocornut/imgui/wiki/Image-Loading-and-Displaying-Examples

    GLuint image_texture;
    if (done) return false;
    done = 1;

    os_memsz image_sz = image_width * (os_memsz)image_height * 4;
    os_uchar *p, *image_data = (os_uchar*)os_malloc(image_sz, OS_NULL);
    os_memsz count;

    p = image_data;
    count = image_sz;
    while (count--) {*(p++) = osal_rand(0, 255);}
    if (image_data == NULL) return false;

    // Create a OpenGL texture identifier
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

    // Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);

    os_free(image_data, image_sz);

    textureID = image_texture;

    return true;
}

/*
void WINAPI glDeleteTextures(
         GLsizei n,
   const GLuint  *textures
);

glDeleteTextures(1, &image_texture);
*/
