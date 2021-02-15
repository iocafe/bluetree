/**

  @file    eimgui_texture_opengl3.cpp
  @brief   Bitmap from memory to graphics card, OpenGL.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    15.9.2020

  Load a bitmap as texture to graphics card, OpenGL implementation.

  See https://github.com/ocornut/imgui/wiki/Image-Loading-and-Displaying-Examples, it has
  instructions for this and for implementing bitmap loading for other graphic rendering
  pipelines, like DirectX.

  See https://www.khronos.org/opengl/wiki/GLAPI/glPixelStore about how to tell OpenGL how
  source bitmap is packed.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "egui.h"

/*
****************************************************************************************************
 * The #include/#define section is copied from eimgui_iface_glfw_opengl3.cpp.
 */
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

/*
 * End of #include/#define section.
****************************************************************************************************
 */


/**
****************************************************************************************************

  @brief Load bitmap to graphics card.

  The function loads a bitmap to graphics card as "texture".
  When no longer needed, texture needs to be deleted from graphics card by calling
  eimgui_delete_texture_on_grahics_card() function.

  @param   bitmap_data Pointer to bitmap data.
  @param   bitmap_width Bitmap width in pixels.
  @param   bitmap_height Bitmap height in pixels.
           bitmap_format OSAL_GRAYSCALE8, OSAL_GRAYSCALE16, OSAL_RGB24, or OSAL_RGBA32.
  @param   byte_width bitmap row width in bytes. Used to set data alignemt. Can be zero,
           for "best guess".
  @param   texttureID Pointer where to store the texture ID. The texture ID is given as
           argument to ImGui's "Image" function to specify which image to display.

  @return  If the bitmap was successfully loaded, this function return ESTATUS_SUCCESS.
           Other return values indicate error, and that the bitmap was not loaded to
           graphics card.

****************************************************************************************************
*/
eStatus eimgui_upload_texture_to_grahics_card(
    const os_uchar *bitmap_data,
    os_int bitmap_width,
    os_int bitmap_height,
    osalBitmapFormat bitmap_format,
    os_int byte_width,
    os_uint *textureID)
{
    GLuint image_texture;
    GLint glformat, internalformat;
    GLenum gltype, err;

    switch (bitmap_format)
    {
        case OSAL_GRAYSCALE8:
            internalformat = 1;
            glformat = GL_LUMINANCE;
            gltype = GL_UNSIGNED_BYTE;
            break;

        case OSAL_GRAYSCALE16:
            internalformat = 1; /* number of color components, not bytes */
            glformat = GL_LUMINANCE;
            gltype = GL_UNSIGNED_SHORT;
            break;

        case OSAL_RGB24:
            internalformat = 3;
            glformat = GL_RGB;
            gltype = GL_UNSIGNED_BYTE;
            break;

        case OSAL_RGBA32:
            internalformat = 4;
            glformat = GL_RGBA;
            gltype = GL_UNSIGNED_BYTE;
            break;

        default:
            osal_debug_error("eimgui_upload_texture_to_grahics_card: Illegal format");
            return ESTATUS_FAILED;
    }

    /* Create a OpenGL texture identifier.
     */
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    /* Setup filtering parameters for display. The GL_TEXTURE_WRAP_S and GL_TEXTURE_WRAP_T
     * settings are required on WebGL for non power-of-two textures
     */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    /* Upload pixels into texture
     */
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif

    /* glPixelStorei with argument GL_UNPACK_ALIGNMENT, and it can be set to one, two, four, or eight bytes.
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    See https://www.khronos.org/opengl/wiki/GLAPI/glPixelStore
    */

    glTexImage2D(GL_TEXTURE_2D, 0, internalformat,
        bitmap_width, bitmap_height, 0, glformat, gltype, bitmap_data);
    err = glGetError();
    if (err != GL_NO_ERROR) {
        osal_debug_error((err == GL_INVALID_OPERATION)
            ? "eimgui_upload_texture_to_grahics_card: called between glBegin and glEnd."
            : "eimgui_upload_texture_to_grahics_card: invalid arguments.");

        glDeleteTextures(1, &image_texture);
        goto failed;
    }

    *textureID = (os_uint)image_texture;
    return ESTATUS_SUCCESS;

failed:
    *textureID = 0;
    return ESTATUS_FAILED;
}


/**
****************************************************************************************************

  @brief Delete a texture (bitmap) from graphics card.

  When a texture is no longer needed, call this function to delete it from graphics card.
  @param   texttureID Specifies which texture to delete.

****************************************************************************************************
*/
void eimgui_delete_texture_on_grahics_card(
    os_uint textureID)
{
    GLuint image_texture = (GLuint)textureID;
    glDeleteTextures(1, &image_texture);
}

