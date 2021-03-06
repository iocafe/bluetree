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
#define EBITMAPP_SBITS EVARP_SBITS /* 20 */
#define EBITMAPP_TSTAMP EVARP_TSTAMP /* 21 */
#define EBITMAPP_FORMAT 30
#define EBITMAPP_WIDTH 31
#define EBITMAPP_HEIGHT 32
#define EBITMAPP_PIXEL_WIDTH_UM 33
#define EBITMAPP_PIXEL_HEIGHT_UM 34
#define EBITMAPP_METADATA 35
#define EBITMAPP_COMPRESSION 36

/* Bitmap property names.
 */
#define ebitmapp_sbits evarp_sbits
#define ebitmapp_tstamp evarp_tstamp
extern const os_char
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
#define EBITMAP_NO_NEW_MEMORY_ALLOCATION 2
#define EBITMAP_TMP_FLAGS_MASK (EBITMAP_KEEP_CONTENT|EBITMAP_NO_NEW_MEMORY_ALLOCATION)

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

    /* Get pointer to uncompressed bitmap.
     */
    os_uchar *ptr();

    /* Get bitmap data type.
     */
    osalBitmapFormat format();

    /* Get bitmap width.
     */
    os_int width();

    /* Get bitmap height.
     */
    os_int height();

    /* Get width of single pixel, micrometers. 0.0 if not set.
     */
    inline os_double pixel_width_um() {return m_pixel_width_um; }

    /* Get height of single pixel, micrometers. 0.0 if not set.
     */
    inline os_double pixel_height_um() {return m_pixel_height_um; }

    /* Get pixel size in bytes.
     */
    os_int pixel_nbytes();

    /* Get bitmap width in bytes (this may not be same as with() * pixel_sz(), since rows
       can be aligned to 4 byte boundary, etc.
     */
    os_int row_nbytes();

    /* Get flags given to eBitmap::allocate().
     */
    inline os_short bflags() {return m_bflags; }

    /* Store JPEG data within bitmap.
     */
    void set_jpeg_data(
        os_uchar *data,
        os_memsz data_sz,
        os_boolean adopt_data);

    /* Compress bitmap as JPEG within eBitmap object.
     */
    void compress();

    /* Free memory allocated for compressed JPEG bitmap, if any.
     */
    void clear_compress();

    /* Uncompress JPEG to flat bitmap buffer.
     */
    eStatus uncompress();


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

    /** State bits, like OSAL_STATE_CONNECTED (2), OSAL_STATE_YELLOW,
        OSAL_STATE_ORANGE...
     */
    os_uchar m_state_bits;

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

    /** JPEG compressed alpha channel, OS_NULL if none.
     */
    os_uchar *m_alpha;

    /** Used alpha buffer size in bytes, 0 if not allocated.
     */
    os_memsz m_alpha_sz;

    /** Allocated alpha buffer size in bytes, 0 if not allocated.
     */
    os_memsz m_alpha_alloc_sz;
};

#endif
