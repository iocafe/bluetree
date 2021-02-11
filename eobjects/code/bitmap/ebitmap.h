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

/**
****************************************************************************************************
  Defines
****************************************************************************************************
*/

/* Bitmap property numbers.
 */
#define EBITMAPP_TSTAMP 15
#define EBITMAPP_FORMAT 20
#define EBITMAPP_WIDTH 21
#define EBITMAPP_HEIGHT 22
#define EBITMAPP_PIXEL_WIDTH_UM 23
#define EBITMAPP_PIXEL_HEIGHT_UM 24
#define EBITMAPP_COMPRESSION 25
#define EBITMAPP_METADATA 30

/* Bitmap property names.
 */
extern const os_char
    ebitmapp_tstamp[],
    ebitmapp_format[],
    ebitmapp_width[],
    ebitmapp_height[],
    ebitmapp_pixel_width_um[],
    ebitmapp_pixel_height_um[],
    ebitmapp_compression[],
    ebitmapp_metadata[];

/* Bitmap compression (when serializing).
 */
typedef enum eBitmapCompression {
  EBITMAP_UNCOMPRESSED = 0,
  EBITMAP_LIGHT_COMPRESSION = 10,
  EBITMAP_MEDIUM_COMPRESSION = 20,
  EBITMAP_HEAVY_COMPRESSION = 30,
}
eBitmapCompression;

/* eBitmap::allocate argument bflags
 */
#define EBITMAP_CLEAR_CONTENT 0
#define EBITMAP_KEEP_CONTENT 1


/**
****************************************************************************************************
  Bitmap class.
****************************************************************************************************
*/
class eBitmap : public eContainer
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
        osalBitmapFormat format,
        os_int width,
        os_int height,
        os_short bflags = 0);

    /* Release all allocated data, empty the bitmap.
     */
    void clear();

    /* Get bitmap data type.
     */
    inline osalBitmapFormat format() {return m_format; }

    /* Get bitmap width.
     */
    inline os_int width() {return m_width; }

    /* Get bitmap height.
     */
    inline os_int height() {return m_height; }

    /* Get width of single pixel, micrometers. 0.0 if not set.
     */
    inline os_double pixel_width_um() {return m_pixel_width_um; }

    /* Get height of single pixel, micrometers. 0.0 if not set.
     */
    inline os_double pixel_height_um() {return m_pixel_height_um; }

    /* Get pixel size in bytes.
     */
    inline os_int pixel_nbytes() {return m_pixel_nbytes; }

    /* Get bitmap width in bytes (this may not be same as with() * pixel_sz(), since rows
       can be aligned to 4 byte boundary, etc.
     */
    inline os_int row_nbytes() {return m_row_nbytes; }

    /* Get flags given to eBitmap::allocate().
     */
    inline os_short bflags() {return m_bflags; }

    /* Compress bitmap as JPEG within eBitmap object.
     */
    void compress();

    /* Free memory allocated for compressed JPEG bitmap, if any.
     */
    void clear_compress();

    /* Uncompress JPEG to flat bitmap buffer.
     */
    void uncompress();


protected:
    /**
    ************************************************************************************************
      Protected functions.
    ************************************************************************************************
    */

    /* Resize the bitmap (change width, height or pixel type).
     */
    void resize(
        osalBitmapFormat format,
        os_int width,
        os_int height,
        os_short bflags);

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
    osalBitmapFormat m_format;

    /** Flags given to eBitmap::allocate().
     */
    os_short m_bflags;

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

    /** Bitmap compression, when serializing the bitmap. One of: EBITMAP_UNCOMPRESSED (0),
        EBITMAP_LIGHT_COMPRESSION, EBITMAP_MEDIUM_COMPRESSION, or EBITMAP_HEAVY_COMPRESSION.
     */
    eBitmapCompression m_compression;

    /** Width of single pixel, micrometers. 0.0 if not set.
     */
    os_double m_pixel_width_um;

    /** Height of single pixel, micrometers. 0.0 if not set.
     */
    os_double m_pixel_height_um;

    /** Bitmap time stamp, 0 if not set.
     */
    os_long m_timestamp;

    /** Pointer to bitmap buffer, OS_NULL if none.
     */
    os_uchar *m_buf;

    /** Buffer allocation size in bytes, 0 if not allocated.
     */
    os_memsz m_buf_alloc_sz;

    /** Used buffer size in bytes, 0 if not allocated.
     */
    os_memsz m_buf_sz;

    /** JPEG compressed image, OS_NULL if none.
     */
    os_uchar *m_jpeg;

    /** Used JPEG buffer size in bytes, 0 if not allocated.
     */
    os_memsz m_jpeg_sz;

    /** Allocated JPEG buffer size in bytes, 0 if not allocated.
     */
    os_memsz m_jpeg_alloc_sz;
};

#endif
