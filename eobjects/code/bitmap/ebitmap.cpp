/**

  @file    ebitmap.cpp
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
#include "eobjects.h"
#include "eosal_jpeg.h"

/* Bitmap property names.
 */
const os_char
    ebitmapp_format[] = "type",
    ebitmapp_width[] = "width",
    ebitmapp_height[] = "height",
    ebitmapp_pixel_width_um[] = "pixw",
    ebitmapp_pixel_height_um[] = "pixw",
    ebitmapp_compression[] = "compression",
    ebitmapp_metadata[] = "metagata";


/**
****************************************************************************************************
  Constructor.
****************************************************************************************************
*/
eBitmap::eBitmap(
    eObject *parent,
    e_oid id,
    os_int flags)
    : eContainer(parent, id, flags)
{
    m_format = OSAL_BITMAP_FORMAT_NOT_SET;
    m_bflags = 0;
    m_pixel_nbytes = 0;
    m_row_nbytes = 0;
    m_width = m_height = 0;
    m_pixel_width_um = m_pixel_height_um = 0.0;
    m_compression = EBITMAP_MEDIUM_COMPRESSION;
    m_timestamp = 0;
    m_buf = OS_NULL;
    m_buf_sz = m_buf_alloc_sz = 0;
    m_jpeg = OS_NULL;
    m_jpeg_sz = m_jpeg_alloc_sz = 0;
    m_alpha = OS_NULL;
    m_alpha_sz = m_alpha_alloc_sz = 0;
    m_state_bits = OSAL_STATE_CONNECTED;
}


/**
****************************************************************************************************
  Virtual destructor.
****************************************************************************************************
*/
eBitmap::~eBitmap()
{
    clear();
}


/**
****************************************************************************************************

  @brief Add eBitmap to class list.

  The eBitmap::setupclass function adds newobj function pointer to class list. This enables creating
  new objects dynamically by class identifier, which is used for serialization reader()
  functions.

  This needs to be called after eBuffer:setupclass(), there is a dependency in setup.

****************************************************************************************************
*/
void eBitmap::setupclass()
{
    eVariable *v;
    const os_int cls = ECLASSID_BITMAP;

    /* Add the class to class list.
     */
    os_lock();
    eclasslist_add(cls, (eNewObjFunc)newobj, "eBitmap", ECLASSID_TABLE);
    addpropertys(cls, ECONTP_TEXT, econtp_text, "text",
        EPRO_PERSISTENT|EPRO_NOONPRCH);
    addpropertyl(cls, EBITMAPP_SBITS, ebitmapp_sbits, "state bits",
        EPRO_SIMPLE);
    addproperty(cls, EBITMAPP_TSTAMP, ebitmapp_tstamp, "timestamp",
        EPRO_PERSISTENT|EPRO_SIMPLE);
    v = addpropertyl(cls, EBITMAPP_FORMAT, ebitmapp_format, "format",
        EPRO_PERSISTENT|EPRO_SIMPLE);
    v->setpropertys(EVARP_ATTR,
        "enum=\"8.grayscale/8,"
        "16.grayscale/16,"
        "152.color/24,"
        "160.color/32,"
        "224.color/32+alpha\"");
    addpropertyl(cls, EBITMAPP_WIDTH, ebitmapp_width, "nro columns",
        EPRO_PERSISTENT|EPRO_SIMPLE);
    addpropertyl(cls, EBITMAPP_HEIGHT, ebitmapp_height, "nro rows",
        EPRO_PERSISTENT|EPRO_SIMPLE);
    v = addpropertyd(cls, EBITMAPP_PIXEL_WIDTH_UM, ebitmapp_pixel_width_um, "pixel width",
        EPRO_PERSISTENT|EPRO_SIMPLE);
    v->setpropertys(EVARP_UNIT, "um");
    v = addpropertyd(cls, EBITMAPP_PIXEL_HEIGHT_UM, ebitmapp_pixel_height_um, "pixel height",
        EPRO_PERSISTENT|EPRO_SIMPLE);
    v->setpropertys(EVARP_UNIT, "um");
    v = addpropertyl(cls, EBITMAPP_COMPRESSION, ebitmapp_compression, "compression",
        EPRO_PERSISTENT|EPRO_SIMPLE);
    v->setpropertys(EVARP_ATTR,
        "enum=\"0.uncompressed,"
        "10.light,"
        "20.medium,"
        "30.heavy\"");
    addproperty (cls, EBITMAPP_METADATA, ebitmapp_metadata, "metadata",
        EPRO_PERSISTENT|EPRO_NOONPRCH);
    propertysetdone(cls);
    os_unlock();
}


/**
****************************************************************************************************

  @brief Clone object

  The eBitmap::clone function clones an eBitmap.

  @param  parent Parent for the clone.
  @param  id Object identifier for the clone.
  @param  aflags 0 for default operation. EOBJ_NO_MAP not to map names.
  @return Pointer to the clone.

****************************************************************************************************
*/
eObject *eBitmap::clone(
    eObject *parent,
    e_oid id,
    os_int aflags)
{
    eBitmap *clonedobj;

    clonedobj = new eBitmap(parent, id == EOID_CHILD ? oid() : id, flags());
    clonedobj->m_compression = m_compression;
    clonedobj->m_timestamp = m_timestamp;
    clonedobj->m_state_bits = m_state_bits;
    clonedobj->m_pixel_width_um = m_pixel_width_um;
    clonedobj->m_pixel_height_um = m_pixel_height_um;
    clonedobj->m_width = m_width;
    clonedobj->m_height = m_height;
    clonedobj->m_pixel_nbytes = m_pixel_nbytes;
    clonedobj->m_row_nbytes = m_row_nbytes;
    clonedobj->m_bflags = m_bflags;

    if (m_buf) {
        clonedobj->m_buf = (os_uchar*)os_malloc(m_buf_sz, &(clonedobj->m_buf_alloc_sz));
        if (clonedobj->m_buf) {
            os_memcpy(clonedobj->m_buf, m_buf, m_buf_sz);
            clonedobj->m_buf_sz = m_buf_sz;
        }
    }

    if (m_jpeg) {
        clonedobj->m_jpeg = (os_uchar*)os_malloc(m_jpeg_sz, &(clonedobj->m_jpeg_alloc_sz));
        if (clonedobj->m_jpeg) {
            os_memcpy(clonedobj->m_jpeg, m_jpeg, m_jpeg_sz);
            clonedobj->m_jpeg_sz = m_jpeg_sz;
        }
    }

    if (m_alpha) {
        clonedobj->m_alpha = (os_uchar*)os_malloc(m_alpha_sz, &(clonedobj->m_alpha_alloc_sz));
        if (clonedobj->m_alpha) {
            os_memcpy(clonedobj->m_alpha, m_alpha, m_alpha_sz);
            clonedobj->m_alpha_sz = m_alpha_sz;
        }
    }

    clonegeneric(clonedobj, aflags);
    return clonedobj;
}


/**
****************************************************************************************************

  @brief Called to inform the class about property value change (override).

  The onpropertychange() function is called when class'es property changes, unless the
  property is flagged with EPRO_NOONPRCH.
  If property is flagged as EPRO_SIMPLE, this function shuold save the property value
  in class members and and return it when simpleproperty() is called.

  @param   propertynr Property number of changed property.
  @param   x Variable containing the new value.
  @param   flags
  @return  If successfull, the function returns ESTATUS_SUCCESS (0). Nonzero return values do
           indicate that there was no property with given property number.

****************************************************************************************************
*/
eStatus eBitmap::onpropertychange(
    os_int propertynr,
    eVariable *x,
    os_int flags)
{
    os_int v;

    switch (propertynr)
    {
        case EBITMAPP_SBITS:
            m_state_bits = (os_uchar)x->getl();
            break;

        case EBITMAPP_TSTAMP:
            m_timestamp = x->getl();
            break;

        case EBITMAPP_FORMAT:
            v = x->geti();
            if (v != m_format) {
                resize((osalBitmapFormat)v, m_width, m_height, m_bflags|
                    EBITMAP_KEEP_CONTENT|EBITMAP_NO_NEW_MEMORY_ALLOCATION);
            }
            break;

        case EBITMAPP_WIDTH:
            v = x->geti();
            if (v != m_width) {
                resize(m_format, v, m_height, m_bflags|
                    EBITMAP_KEEP_CONTENT|EBITMAP_NO_NEW_MEMORY_ALLOCATION);
            }
            break;

        case EBITMAPP_HEIGHT:
            v = x->geti();
            if (v != m_height) {
                resize(m_format, m_width, v, m_bflags|
                    EBITMAP_KEEP_CONTENT|EBITMAP_NO_NEW_MEMORY_ALLOCATION);
            }
            break;

        case EBITMAPP_PIXEL_WIDTH_UM:
            m_pixel_width_um = x->getd();
            break;

        case EBITMAPP_PIXEL_HEIGHT_UM:
            m_pixel_height_um = x->getd();
            break;

        case EBITMAPP_COMPRESSION:
            m_compression = (eBitmapCompression)x->geti();
            break;

        default:
            return eContainer::onpropertychange(propertynr, x, flags);
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
eStatus eBitmap::simpleproperty(
    os_int propertynr,
    eVariable *x)
{
    switch (propertynr)
    {
        case EBITMAPP_SBITS:
            x->setl(m_state_bits);
            break;

        case EBITMAPP_TSTAMP:
            x->setl(m_timestamp);
            break;

        case EBITMAPP_FORMAT:
            x->setl(m_format);
            break;

        case EBITMAPP_WIDTH:
            x->setl(m_width);
            break;

        case EBITMAPP_HEIGHT:
            x->setl(m_height);
            break;

        case EBITMAPP_PIXEL_WIDTH_UM:
            x->setd(m_pixel_width_um);
            break;

        case EBITMAPP_PIXEL_HEIGHT_UM:
            x->setd(m_pixel_height_um);
            break;

        case EBITMAPP_COMPRESSION:
            x->setl(m_compression);
            break;

        default:
            return eContainer::simpleproperty(propertynr, x);
    }
    return ESTATUS_SUCCESS;
}


/**
****************************************************************************************************

  @brief Write bitmap to stream.

  The eBitmap::writer() function serializes the eBitmap to stream. This writes only the
  content, use eObject::write() to save also class information, attachements, etc.

  @param  stream The stream to write to.
  @param  sflags Serialization flags.

  @return If successfull the function returns ESTATUS_SUCCESS (0). If writing object to stream
          fails, value ESTATUS_WRITING_OBJ_FAILED is returned. Assume that all nonzero values
          indicate an error.

****************************************************************************************************
*/
eStatus eBitmap::writer(
    eStream *stream,
    os_int sflags)
{
    const os_char *p;
    os_memsz n;
    os_int y;
    const os_int version = 0;
    OSAL_UNUSED(sflags);

    /* Begin the object and write version number. Increment if new serialized items are
       added to the object, and check for new version's items in read() function.
     */
    if (stream->write_begin_block(version)) goto failed;

    /* Write bitmap data type and size.
     */
    if (stream->putl(m_format)) goto failed;
    if (stream->putl(m_width)) goto failed;
    if (stream->putl(m_height)) goto failed;
    if (stream->putd(m_pixel_width_um)) goto failed;
    if (stream->putd(m_pixel_height_um)) goto failed;
    if (stream->putl(m_compression)) goto failed;
    if (stream->putl(m_timestamp)) goto failed;
    if (stream->putl(m_state_bits)) goto failed;

    /* Write the bitmap, either uncompressed or as jpeg.
     */
    if (m_buf_sz) {
        if (m_compression == EBITMAP_UNCOMPRESSED)
        {
            if (m_buf == OS_NULL) {
                goto failed;
            }

            n = m_pixel_nbytes * m_width;
            if (n == m_row_nbytes) {
                if (stream->write((const os_char*)m_buf,
                    (os_memsz)m_row_nbytes * (os_memsz)m_height))
                {
                    goto failed;
                }
            }

            else {
                p = (const os_char*)m_buf;
                for (y = 0; y < m_height; y++) {
                    if (stream->write(p, n)) goto failed;
                    p += m_row_nbytes;
                }
            }
        }
        else
        {
            if (m_jpeg == OS_NULL) {
                compress();
                if (m_jpeg == OS_NULL) goto failed;
            }

            if (stream->putl(m_jpeg_sz)) goto failed;
            if (stream->write((const os_char*)m_jpeg, m_jpeg_sz)) {
                goto failed;
            }

            if (m_format & OSAL_BITMAP_ALPHA_CHANNEL_FLAG) {
                if (m_alpha == OS_NULL) {
                    goto failed;
                }

                if (stream->putl(m_alpha_sz)) goto failed;
                if (stream->write((const os_char*)m_alpha, m_alpha_sz)) {
                    goto failed;
                }
            }
        }
    }

    /* End the object.
     */
    if (stream->write_end_block()) goto failed;

    /* Object succesfully written.
     */
    return ESTATUS_SUCCESS;

    /* Writing object failed.
     */
failed:
    return ESTATUS_WRITING_OBJ_FAILED;
}


/**
****************************************************************************************************

  @brief Read a bitmap from stream.

  The eBitmap::reader() function reads serialized eBitmap from stream. This function
  reads only the object content. To read whole object including attachments, names, etc,
  use eObject::read().

  @param  stream The stream to read from.
  @param  sflags Serialization flags.

  @return If successfull the function returns ESTATUS_SUCCESS (0). If writing object to stream
          fails, value ESTATUS_READING_OBJ_FAILED is returned. Assume that all nonzero values
          indicate an error.

****************************************************************************************************
*/
eStatus eBitmap::reader(
    eStream *stream,
    os_int sflags)
{
    os_char *p;
    os_long format, height, width, tmp;
    os_int version, n, y;
    OSAL_UNUSED(sflags);

    /* If we have old data, delete it.
     */
    clear();

    /* Read object start mark and version number. Used to check which versions item's
       are in serialized data.
     */
    if (stream->read_begin_block(&version)) goto failed;

    /* Read bitmap data type and size, allocate bitmap.
     */
    if (stream->getl(&format)) goto failed;
    if (stream->getl(&width)) goto failed;
    if (stream->getl(&height)) goto failed;
    if (stream->getd(&m_pixel_width_um)) goto failed;
    if (stream->getd(&m_pixel_height_um)) goto failed;
    if (stream->getl(&tmp)) goto failed;
    m_compression = (eBitmapCompression)tmp;
    if (stream->getl(&tmp)) goto failed;
    m_timestamp = tmp;
    if (stream->getl(&tmp)) goto failed;
    m_state_bits = (os_uchar)tmp;

    resize((osalBitmapFormat)format, (os_int)width, (os_int)height,
        m_compression == EBITMAP_UNCOMPRESSED
        ? m_bflags
        : m_bflags|EBITMAP_NO_NEW_MEMORY_ALLOCATION);

    /* Write the bitmap, either uncompressed or as jpeg.
     */
    if (m_width * m_height != 0) {
        if (m_buf == OS_NULL) {
            goto failed;
        }

        if (m_compression == EBITMAP_UNCOMPRESSED) {
            n = m_pixel_nbytes * m_width;
            if (n == m_row_nbytes) {
                if (stream->read((os_char*)m_buf,
                    (os_memsz)m_row_nbytes * (os_memsz)m_height))
                {
                    goto failed;
                }
            }
            else {
                p = (os_char*)m_buf;
                for (y = 0; y < m_height; y++) {
                    if (stream->read(p, n)) goto failed;
                    p += m_row_nbytes;
                }
            }
        }
        else {
            if (stream->getl(&m_jpeg_sz)) goto failed;
            if (m_jpeg_sz <= 0) goto failed;

            m_jpeg = (os_uchar*)os_malloc(m_jpeg_sz, &m_jpeg_alloc_sz);
            if (m_jpeg <= 0) goto failed;

            if (stream->read((os_char*)m_jpeg, m_jpeg_sz)) {
                goto failed;
            }

            if (m_format & OSAL_BITMAP_ALPHA_CHANNEL_FLAG) {
                if (stream->getl(&m_alpha_sz)) goto failed;
                if (m_alpha_sz <= 0) goto failed;

                m_alpha = (os_uchar*)os_malloc(m_alpha_sz, &m_alpha_alloc_sz);
                if (m_alpha <= 0) goto failed;

                if (stream->read((os_char*)m_alpha, m_alpha_sz)) {
                    goto failed;
                }
            }
        }
    }

    /* End the object.
     */
    if (stream->read_end_block()) goto failed;

    /* Object succesfully read.
     */
    return ESTATUS_SUCCESS;

    /* Reading object failed.
     */
failed:
    return ESTATUS_READING_OBJ_FAILED;
}


#if E_SUPPROT_JSON
/**
****************************************************************************************************

  @brief Write bitmap specific content to stream as JSON.

  The eBitmap::json_writer() function writes class specific object content to stream as JSON.

  @param  stream The stream to write to.
  @param  sflags Serialization flags. Typically EOBJ_SERIALIZE_DEFAULT.
  @param  indent Indentation depth, 0, 1... Writes 2x this spaces at beginning of a line.

  @return If successfull the function returns ESTATUS_SUCCESS (0). If writing object to stream
          fails, value ESTATUS_WRITING_OBJ_FAILED is returned. Assume that all nonzero values
          indicate an error.

****************************************************************************************************
*/
eStatus eBitmap::json_writer(
    eStream *stream,
    os_int sflags,
    os_int indent)
{
    return eContainer::json_writer(stream, sflags, indent);
}
#endif


/**
****************************************************************************************************

  @brief Allocate a bitmap.

  The eBitmap::allocate function allocates bitmap buffer or resizes the buffer, and sets bitmap
  size properties.

  @param  format New image format, color image or grayscale, bits per pixel:
          OSAL_GRAYSCALE8 (8), OSAL_GRAYSCALE16 (16), OSAL_RGB24 (152) or OSAL_RGBA32 (160).
  @param  width New image width in pixels.
  @param  height New image height in pixels.
  @param  bflags Bit fields:
          - EBITMAP_CLEAR_CONTENT Clear bitmap content, if any.
          - EBITMAP_KEEP_CONTENT Preserve old bitmap content if size doesn't cahnge.
          - EBITMAP_NO_NEW_MEMORY_ALLOCATION Do not allocate new bitmap.

****************************************************************************************************
*/
void eBitmap::allocate(
    osalBitmapFormat format,
    os_int width,
    os_int height,
    os_short bflags)
{
    resize(format, width, height, bflags);
    setpropertyl(EBITMAPP_FORMAT, m_format);
    setpropertyl(EBITMAPP_WIDTH,  m_width);
    setpropertyl(EBITMAPP_HEIGHT, m_height);
}


/**
****************************************************************************************************

  @brief Resize the bitmap.

  The eBitmap::resize function allocates bitmap buffer or resizes the buffer.

  @param  format New image format, color image or grayscale, bits per pixel:
          OSAL_GRAYSCALE8 (8), OSAL_GRAYSCALE16 (16), OSAL_RGB24 (152) or OSAL_RGBA32 (160).
  @param  width New image width in pixels.
  @param  height New image height in pixels.
  @param  bflags Bit fields:
          - EBITMAP_CLEAR_CONTENT Clear bitmap content, if any.
          - EBITMAP_KEEP_CONTENT Preserve old bitmap content if size doesn't cahnge.
          - EBITMAP_NO_NEW_MEMORY_ALLOCATION Do not allocate new bitmap.

****************************************************************************************************
*/
void eBitmap::resize(
    osalBitmapFormat format,
    os_int width,
    os_int height,
    os_short bflags)
{
    os_short pixel_nbytes;
    os_int row_nbytes;
    os_memsz buf_sz;
    os_short tmp_flags;

    tmp_flags = (bflags & EBITMAP_TMP_FLAGS_MASK);
    bflags &= ~EBITMAP_TMP_FLAGS_MASK;

    /* If nothing has changed.
     */
    if (m_format == format && m_bflags == bflags && m_height == height && m_width == width)
    {
        if (m_buf && (tmp_flags & EBITMAP_KEEP_CONTENT) == 0) {
            os_memclear(m_buf, m_buf_sz);
            clear_compress();
        }
        return;
    }

    pixel_nbytes = OSAL_BITMAP_BYTES_PER_PIX(format);
    row_nbytes = pixel_nbytes * width;

    /* RGB24: Align rows to 4 byte boundary.
     */
    if (pixel_nbytes == 3) {
        row_nbytes = (row_nbytes + 3) / 4;
        row_nbytes *= 4;
    }

    buf_sz = (os_memsz)row_nbytes * (os_memsz)height;
    if (m_buf || (tmp_flags & EBITMAP_NO_NEW_MEMORY_ALLOCATION) == 0)
    {
        if (buf_sz > 0) {
            if (buf_sz > m_buf_alloc_sz || buf_sz < m_buf_alloc_sz / 2) {
                clear();
                m_buf = (os_uchar*)os_malloc(buf_sz, &m_buf_alloc_sz);
                if (m_buf) os_memclear(m_buf, buf_sz);
            }
            else {
                clear_compress();
                os_memclear(m_buf, buf_sz);
            }
        }
        else {
            clear();
        }
    }
    else {
        clear();
    }

    m_format = format;
    m_bflags = bflags;
    m_width = width;
    m_height = height;
    m_pixel_nbytes = pixel_nbytes;
    m_row_nbytes = row_nbytes;
    m_buf_sz = buf_sz;
}


/**
****************************************************************************************************

  @brief Clear bitmap.

  The eBitmap::clear releases all data allocated for bitmap and sets bitmap size to 0, 0.

****************************************************************************************************
*/
void eBitmap::clear()
{
    clear_compress();
    if (m_buf) {
        os_free(m_buf, m_buf_alloc_sz);
        m_buf = OS_NULL;
        m_buf_alloc_sz = 0;
    }
    m_width = m_height = 0;
    m_pixel_nbytes = 0;
    m_row_nbytes = 0;
    m_format = OSAL_BITMAP_FORMAT_NOT_SET;
    m_bflags = 0;
}


/**
****************************************************************************************************

  @brief Get pointer to uncompressed bitmap.

  If there is compressed bitmap, but not uncompressed one, it is decompressed.

****************************************************************************************************
*/
os_uchar *eBitmap::ptr()
{
    if (m_buf) return m_buf;
    uncompress();
    return m_buf;
}


/**
****************************************************************************************************

  @brief Get bitmap format.

  If format is not known, but we have uncompressed bitmap, uncompress it and get format from there.

****************************************************************************************************
*/
osalBitmapFormat eBitmap::format()
{
    if (m_format != OSAL_BITMAP_FORMAT_NOT_SET) return m_format;
    uncompress();
    return m_format;
}


/**
****************************************************************************************************

  @brief Get bitmap width.

  If width is not known, but we have uncompressed bitmap, uncompress it and get width from there.

****************************************************************************************************
*/
os_int eBitmap::width()
{
    if (m_width) return m_width;
    uncompress();
    return m_width;
}


/**
****************************************************************************************************

  @brief Get bitmap height.

  If height is not known, but we have uncompressed bitmap, uncompress it and get height from there.

****************************************************************************************************
*/
os_int eBitmap::height()
{
    if (m_height) return m_height;
    uncompress();
    return m_height;
}


/**
****************************************************************************************************

  @brief Get pixel size in bytes.

  If pixel size is not known, but we have uncompressed bitmap, uncompress it and get pixel
  size from there.

****************************************************************************************************
*/
os_int eBitmap::pixel_nbytes()
{
    if (m_pixel_nbytes) return m_pixel_nbytes;
    uncompress();
    return m_pixel_nbytes;
}


/**
****************************************************************************************************

  @brief Get bitmap width in bytes.

  Bitmap width in bytes (this may not be same as with() * pixel_sz(), since rows
  can be aligned to 4 byte boundary, etc.
  If byte width is not known, but we have uncompressed bitmap, uncompress it and get byte width
  from there.

****************************************************************************************************
*/
os_int eBitmap::row_nbytes()
{
    if (m_row_nbytes) return m_row_nbytes;
    uncompress();
    return m_row_nbytes;
}


/**
****************************************************************************************************

  @brief Store JPEG data within bitmap.

  If you want to use image size and other parameters from JPEG, do not call eBitmap::alloc,
  but just call this function on new eBitmap.

  @param   data Pointer to JPEG data.
  @param   data_sz JPED data size in bytes.
  @param   adopt_data
           - OS_TRUE: If data buffer is allocated with os_malloc, the "adopt_data" option can
             be used and the eBitmap simply takes ownership of the bufffer.
           - OS_FALSE: New buffer is allocated and data is copied into it.

****************************************************************************************************
*/
void eBitmap::set_jpeg_data(
    os_uchar *data,
    os_memsz data_sz,
    os_boolean adopt_data)
{
    if (adopt_data) {
        if (m_jpeg) {
            clear_compress();
        }
        m_jpeg = data;
        m_jpeg_sz = m_jpeg_alloc_sz = data_sz;
        return;
    }

    if (m_jpeg) {
        if (data_sz > m_jpeg_alloc_sz || data_sz < m_jpeg_sz/2) {
            clear_compress();
        }
    }

    if (m_jpeg == OS_NULL) {
        m_jpeg = (os_uchar*)os_malloc(data_sz, &m_jpeg_alloc_sz);
        if (m_jpeg == OS_NULL) return;
    }

    os_memcpy(m_jpeg, data, data_sz);
    m_jpeg_sz = data_sz;

    if (m_buf) {
        os_free(m_buf, m_buf_alloc_sz);
        m_buf = OS_NULL;
        m_buf_alloc_sz = 0;
    }
}


/**
****************************************************************************************************

  @brief Compress bitmap as JPEG within eBitmap object.

  The compress function creates JPEG in eBitmap's internal buffer.

****************************************************************************************************
*/
void eBitmap::compress()
{
    osalStream dst_stream;
    osalStatus s;
    os_memsz nbytes;
    os_int quality;

    /* If we got the jpeg, do not recompress.
     */
    if (m_jpeg) return;

    if (m_buf == OS_NULL) {
        osal_debug_error("eBitmap: nothing to compress");
        return;
    }

    switch (m_compression)
    {
        case EBITMAP_LIGHT_COMPRESSION:  quality = 80; break;
        default:
        case EBITMAP_MEDIUM_COMPRESSION: quality = 60; break;
        case EBITMAP_HEAVY_COMPRESSION:  quality = 40; break;
    }

    dst_stream = osal_stream_buffer_open(OS_NULL,
        OS_NULL, OS_NULL, OSAL_STREAM_WRITE);
    if (dst_stream == OS_NULL) {
        osal_debug_error("eBitmap: osal_stream_buffer_open failed");
        return;
    }

    s = os_compress_JPEG(
        m_buf, m_width, m_height, m_row_nbytes, m_format, quality, dst_stream,
            OS_NULL, 0, &nbytes, OSAL_JPEG_DEFAULT);
    if (s) {
        osal_debug_error("eBitmap: compression failed 1");
        goto getout;
    }

    m_jpeg = (os_uchar*)osal_stream_buffer_adopt_content(dst_stream,
        &m_jpeg_sz, &m_jpeg_alloc_sz);

    if (m_format & OSAL_BITMAP_ALPHA_CHANNEL_FLAG) {
        s = os_compress_JPEG(
            m_buf, m_width, m_height, m_row_nbytes, m_format, quality, dst_stream,
                OS_NULL, 0, &nbytes, OSAL_JPEG_SELECT_ALPHA_CHANNEL);
        if (s) {
            osal_debug_error("eBitmap: compression failed 2");
            goto getout;
        }

        m_alpha = (os_uchar*)osal_stream_buffer_adopt_content(dst_stream,
            &m_alpha_sz, &m_alpha_alloc_sz);
    }

getout:
    osal_stream_close(dst_stream, OSAL_STREAM_DEFAULT);
}


/**
****************************************************************************************************

  @brief Free memory allocated for compressed JPEG bitmap, if any.

****************************************************************************************************
*/
void eBitmap::clear_compress()
{
    if (m_jpeg) {
        os_free(m_jpeg, m_jpeg_alloc_sz);
    }
    m_jpeg = OS_NULL;
    m_jpeg_sz = m_jpeg_alloc_sz = 0;
    if (m_alpha) {
        os_free(m_alpha, m_alpha_alloc_sz);
    }
    m_alpha = OS_NULL;
    m_alpha_sz = m_alpha_alloc_sz = 0;
}


/**
****************************************************************************************************

  @brief Uncompress JPEG to flat bitmap buffer.

  The uncompress function uncompressess bitmap in internal buffer.

****************************************************************************************************
*/
eStatus eBitmap::uncompress()
{
    osalJpegMallocContext alloc_context;
    osalStatus s;

    if (m_jpeg == OS_NULL) {
        return ESTATUS_FAILED;
    }

    /* If buffer size if configured, but not allocated.
     */
    if (m_buf == OS_NULL && m_buf_sz)
    {
        m_buf = (os_uchar*)os_malloc(m_buf_sz, &m_buf_alloc_sz);
        if (m_buf == OS_NULL) return ESTATUS_FAILED;
        os_memclear(m_buf, m_buf_sz);
    }

    /* Uncompress to existing buffer, if we have got one. Otherwise if m_buf is OS_NULL,
       the os_uncompress_JPEG allocates new buffer.
     */
    os_memclear(&alloc_context, sizeof(alloc_context));
    if (m_buf) {
        alloc_context.buf = m_buf;
        alloc_context.buf_sz = m_buf_alloc_sz;
        alloc_context.row_nbytes = m_row_nbytes;
        alloc_context.format = m_format;
    }

    s = os_uncompress_JPEG(m_jpeg, m_jpeg_sz, &alloc_context, OSAL_JPEG_DEFAULT);
    if (s) {
        return ESTATUS_FROM_OSAL_STATUS(s);
    }

    if (m_buf == OS_NULL) {
        m_buf = alloc_context.buf;
        m_buf_sz = alloc_context.buf_sz;
        m_buf_alloc_sz = alloc_context.nbytes;
        m_row_nbytes = (os_int)alloc_context.row_nbytes;
        m_width = alloc_context.w;
        m_height = alloc_context.h;
        m_format = alloc_context.format;
        m_pixel_nbytes = OSAL_BITMAP_BYTES_PER_PIX(m_format);
        setpropertyl(EBITMAPP_FORMAT, m_format);
        setpropertyl(EBITMAPP_WIDTH,  m_width);
        setpropertyl(EBITMAPP_HEIGHT, m_height);
    }
    else {
        osal_debug_assert(alloc_context.w == m_width && alloc_context.h == m_height);
    }

    if (m_format & OSAL_BITMAP_ALPHA_CHANNEL_FLAG) {
        s = os_uncompress_JPEG(m_alpha, m_alpha_sz, &alloc_context,
            OSAL_JPEG_SELECT_ALPHA_CHANNEL);
        if (s) {
            return ESTATUS_FROM_OSAL_STATUS(s);
        }
    }

    return ESTATUS_SUCCESS;
}


/**
****************************************************************************************************

  @brief Collect information about this bitmap for tree browser, etc.

  The eBitmap::object_info function fills in item (eVariable) to contain information
  about this object for tree browser view.

  @param   item Pointer to eVariable to set up with object information.
  @param   name Object's name if known. OS_NULL if object is not named or name is
           unknown at this time.
  @param   appendix Pointer to eSet into which to store property flags. The stored property
           flags indicate if object has namespace, children, or properties.
  @param   target Path "within object" when browsing a tree which is not made out
           of actual eObjects. For example OS file system directory.

****************************************************************************************************
*/
void eBitmap::object_info(
    eVariable *item,
    eVariable *name,
    eSet *appendix,
    const os_char *target)
{
    eObject::object_info(item, name, appendix, target);
    appendix->setl(EBROWSE_RIGHT_CLICK_SELECTIONS, EBROWSE_OPEN);
}
