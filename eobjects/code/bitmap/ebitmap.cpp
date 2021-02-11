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

/* Bitmap property names.
 */
const os_char
    ebitmapp_tstamp[] = "tstamp",
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
    m_format = OSAL_RGB24;
    m_bflags = 0;
    m_pixel_nbytes = 3;
    m_row_nbytes = 0;
    m_width = m_height = 0;
    m_pixel_width_um = m_pixel_height_um = 0.0;
    m_compression = EBITMAP_MEDIUM_COMPRESSION;
    m_timestamp = 0;
    m_buf = OS_NULL;
    m_buf_sz = m_buf_alloc_sz = 0;
    m_jpeg = OS_NULL;
    m_jpeg_sz = m_jpeg_alloc_sz = 0;
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
    addproperty (cls, EBITMAPP_TSTAMP, ebitmapp_tstamp, "timestamp",
        EPRO_PERSISTENT|EPRO_SIMPLE);
    v = addpropertyl(cls, EBITMAPP_FORMAT, ebitmapp_format, "format",
        EPRO_PERSISTENT|EPRO_SIMPLE);
    v->setpropertys(EVARP_ATTR,
        "enum=\"8.grayscale/8,"
        "16.grayscale/16,"
        "152.color/24,"
        "160.color/32\"");
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
    clonedobj->m_pixel_width_um = 0.0;
    clonedobj->m_pixel_height_um = 0.0;
    clonedobj->allocate(m_format, m_width, m_height, m_bflags);

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
eStatus eBitmap::onpropertychange(
    os_int propertynr,
    eVariable *x,
    os_int flags)
{
    os_int v;

    switch (propertynr)
    {
        case EBITMAPP_TSTAMP:
            m_timestamp = x->getl();
            break;

        case EBITMAPP_FORMAT:
            v = x->geti();
            resize((osalBitmapFormat)v, m_width, m_height, m_bflags|EBITMAP_KEEP_CONTENT);
            break;

        case EBITMAPP_WIDTH:
            v = x->geti();
            resize(m_format, v, m_height, m_bflags|EBITMAP_KEEP_CONTENT);
            break;

        case EBITMAPP_HEIGHT:
            v = x->geti();
            resize(m_format, m_width, v, m_bflags|EBITMAP_KEEP_CONTENT);
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
            return eObject::onpropertychange(propertynr, x, flags);
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
            return eObject::simpleproperty(propertynr, x);
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
    resize((osalBitmapFormat)format, (os_int)width, (os_int)height, m_bflags);

    /* Write the bitmap, either uncompressed or as jpeg.
     */
    if (m_width * m_height) {
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

            uncompress();
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
  @param  bflags EBITMAP_CLEAR_CONTENT or EBITMAP_KEEP_CONTENT, wether to preserve old bitmap
          content if size doesn't cahnge.

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
  @param  bflags EBITMAP_CLEAR_CONTENT or EBITMAP_KEEP_CONTENT, wether to preserve old bitmap
          content if size doesn't cahnge.

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
    os_short keep_content;

    keep_content = (bflags & EBITMAP_KEEP_CONTENT);
    bflags &= ~EBITMAP_KEEP_CONTENT;

    /* If nothing has changed.
     */
    if (m_format == format && m_bflags == bflags && m_height == height && m_width == width)
    {
        if (m_buf && !keep_content) {
            os_memclear(m_buf, m_buf_sz);
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
    if (buf_sz > 0) {
        if (buf_sz > m_buf_alloc_sz || buf_sz < m_buf_alloc_sz / 2) {
            clear();
            m_buf = (os_uchar*)os_malloc(buf_sz, &m_buf_alloc_sz);
            os_memclear(m_buf, buf_sz);
        }
        else {
            os_memclear(m_buf, buf_sz);
        }

    }
    else {
        clear();
    }

    m_format = format;
    m_bflags = (bflags & ~EBITMAP_KEEP_CONTENT);
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
    m_format = OSAL_RGB24;
    m_bflags = 0;
}


/**
****************************************************************************************************

  @brief Compress bitmap as JPEG within eBitmap object.

  The compress function creates JPEG in eBitmap's internal buffer.

****************************************************************************************************
*/
void eBitmap::compress()
{
}


/**
****************************************************************************************************

  @brief Free memory allocated for compressed JPEG bitmap, if any.

****************************************************************************************************
*/
void eBitmap::clear_compress()
{
}


/**
****************************************************************************************************

  @brief Uncompress JPEG to flat bitmap buffer.

  The uncompress function uncompressess bitmap in internal buffer.

****************************************************************************************************
*/
void eBitmap::uncompress()
{
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


/**
****************************************************************************************************

  @brief Information for opening object has been requested, send it.

  The object has received ECMD_INFO request and it needs to return back information
  for opening the object.

  @param   envelope Message envelope. Contains command, target and source paths and
           message content, etc.
  @return  None.

****************************************************************************************************
*/
void eBitmap::send_open_info(
    eEnvelope *envelope)
{
    eContainer *request, *reply;
    eVariable *v;
    os_int command = EBROWSE_OPEN;

    /* Get command
     */
    request = eContainer::cast(envelope->content());
    if (request->classid() != ECLASSID_CONTAINER) return;
    if (request) {
        v = request->firstv(EOID_PARAMETER);
        if (v) {
            command = v->geti();
        }
    }

// EBROWSE_PROPERTIES

    /* The "open" selection shows the bitmap content as table.
     */
    if (command == EBROWSE_OPEN)
    {
        /* Created container for reply content.
         */
        reply = new eContainer(this, EOID_ITEM, EOBJ_IS_ATTACHMENT);
        new eVariable(reply, ECLASSID_BITMAP);

        eVariable tmp;
        propertyv(ECONTP_TEXT, &tmp);
        if (!tmp.isempty()) {
            reply->setpropertyv(ECONTP_TEXT, &tmp);
        }

        /* Send reply to caller
         */
        message(ECMD_OPEN_REPLY, envelope->source(),
            envelope->target(), reply, EMSG_DEL_CONTENT, envelope->context());
    }

    /* Otherwise use default implementation for properties, etc.
     */
    else {
        eObject::send_open_info(envelope);
    }
}

