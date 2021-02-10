/**

  @file    ebitmap.h
  @brief   Bitmap class.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    15.10.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef EBITMAP_H_
#define EBITMAP_H_
#include "eobjects.h"

class eBuffer;
class eDBM;

/**
****************************************************************************************************
  Defines
****************************************************************************************************
*/


/* Bitmap property numbers.
 */
#define EBITMAPP_TEXT 10
#define EBITMAPP_PIXEL_TYPE 20
#define EBITMAPP_WIDTH 21
#define EBITMAPP_HEIGHT 22
#define EBITMAPP_METADATA 30

/* Bitmap property names.
 */
extern const os_char
    ebitmapp_text[],
    ebitmapp_pixel_type[],
    ebitmapp_width[],
    ebitmapp_height[],
    ebitmapp_metadata[];


/**
****************************************************************************************************
  Bitmap class.
****************************************************************************************************
*/
class eBitmap : public eTable
{
public:

    /**
    ************************************************************************************************
      Generic eObject functionality.
    ************************************************************************************************
    */
    /* Constructor.
     */
    eBitmap(
        eObject *parent = OS_NULL,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT);

    /* Virtual destructor.
     */
    virtual ~eBitmap();

    /* Clone object.
     */
    virtual eObject *clone(
        eObject *parent,
        e_oid id = EOID_CHILD,
        os_int aflags = 0);

    /* Casting eObject pointer to eBitmap pointer.
     */
    inline static eBitmap *cast(
        eObject *o)
    {
        e_assert_type(o, ECLASSID_BITMAP)
        return (eBitmap*)o;
    }

    /* Get class identifier.
     */
    virtual os_int classid() {return ECLASSID_BITMAP; }

    /* Static function to add class to propertysets and class list.
     */
    static void setupclass();

    /* Static constructor function for generating instance by class list.
     */
    static eBitmap *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new eBitmap(parent, id, flags);
    }

    /* Function to process incoming messages.
     */
    virtual void onmessage(
        eEnvelope *envelope);

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

    /* Write bitmap content to stream.
     */
    virtual eStatus writer(
        eStream *stream,
        os_int flags);

    /* Read bitmap content from stream.
     */
    virtual eStatus reader(
        eStream *stream,
        os_int flags);

#if E_SUPPROT_JSON
    /* Called to check if object has class specific content. If there is no class
       specific JSON content, json_writer or json_reader should not be called.
     */
    virtual os_boolean has_json_content()
    {
        return OS_TRUE;
    }

    /* Write bitmap specific content to stream as JSON.
     */
    virtual eStatus json_writer(
        eStream *stream,
        os_int sflags,
        os_int indent);
#endif


    /**
    ************************************************************************************************
      Bitmap functions.
    ************************************************************************************************
    */
    /* Allocate bitmap.
     */
    void allocate(
        osalTypeId type,
        os_int nrows = 0,
        os_int ncolumns = 0);

    /* Release all allocated data, empty the bitmap.
     */
    void clear();

    /* Get bitmap data type.
     */
    inline osalTypeId pixel_type() {return m_pixel_type; }

    /* Get bitmap width.
     */
    inline os_int width() {return m_width; }

    /* Get bitmap height.
     */
    inline os_int height() {return m_height; }

protected:
    /**
    ************************************************************************************************
      Protected functions.
    ************************************************************************************************
    */

    /* Resize the bitmap (change width, height or pixel type).
     */
    void resize(
        osalTypeId pixel_type,
        os_int width,
        os_int height);

    /* Collect information about this bitmap for tree browser, etc.
     */
    virtual void object_info(
        eVariable *item,
        eVariable *name,
        eSet *appendix,
        const os_char *target);

    /* Information for opening object has been requested, send it.
     */
    virtual void send_open_info(
        eEnvelope *envelope);


    /**
    ************************************************************************************************
      Member variables
    ************************************************************************************************
    */
    /** Pixel data type.
     */
    osalTypeId m_pixel_type;

    /** Pixel data size in bytes.
     */
    os_short m_pixel_nbytes;

    /** Bitmap width in pixels.
     */
    os_int m_row_nbytes;

    /** Bitmap width in pixels.
     */
    os_int m_width;

    /** Bitmap height in pixels.
     */
    os_int m_height;

    /** To prevent recursive resizing.
     */
    os_short m_own_change;
};

#endif
