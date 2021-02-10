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
    ebitmapp_text[] = "text",
    ebitmapp_datatype[] = "type",
    ebitmapp_height[] = "height",
    ebitmapp_width[] = "width",
    ebitmapp_configuration[] = "configuration";

/* Approximate size for one eBuffer, adjusted to memory allocation block.
 */
#define OEBITMAP_APPROX_BUF_SZ 128

/* Flags for getptrs() function.
 */
#define EBITMAP_ALLOCATE_IF_NEEDED 1
#define EBITMAP_CLEAR_ELEMENT 2

typedef union
{
    os_long l;
    os_double d;
    os_char *s;
    eObject *o;
}
eBitmapDataItem;

static const os_short ebitmap_no_short_value = OS_SHORT_MIN;
static const os_int ebitmap_no_int_value = OS_INT_MIN;
static const os_long ebitmap_no_long_value = OS_LONG_MIN;

/**
****************************************************************************************************
  Constructor.
****************************************************************************************************
*/
eBitmap::eBitmap(
    eObject *parent,
    e_oid id,
    os_int flags)
    : eTable(parent, id, flags)
{
    /** Initial data type is OS_OBJECT.
     */
    m_pixel_type = OS_UINT;
    m_pixel_nbytes = 0;
    m_row_nbytes = 0;

    /** Initial size is 0, 0
     */
    m_width = m_height = 0;

    m_own_change = 0;
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
    const os_int cls = ECLASSID_BITMAP;

    /* Add the class to class list.
     */
    os_lock();
    eclasslist_add(cls, (eNewObjFunc)newobj, "eBitmap", ECLASSID_TABLE);
    addpropertyl(cls, EBITMAPP_TEXT, ebitmapp_text, "text", EPRO_PERSISTENT|EPRO_NOONPRCH);
    addpropertyl(cls, EBITMAPP_PIXEL_TYPE, ebitmapp_datatype, "data type", EPRO_PERSISTENT|EPRO_SIMPLE);
    addpropertyl(cls, EBITMAPP_HEIGHT, ebitmapp_height, "nro rows", EPRO_PERSISTENT|EPRO_SIMPLE);
    addpropertyl(cls, EBITMAPP_WIDTH, ebitmapp_width, "nro columns", EPRO_PERSISTENT|EPRO_SIMPLE);
    addproperty (cls, EBITMAPP_METADATA, ebitmapp_configuration, "configuration",
        EPRO_PERSISTENT|EPRO_NOONPRCH);

    add_generic_table_properties(cls, ETABLE_BASIC_ATTR_GROUP);
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
    eVariable *tmp;

    clonedobj = new eBitmap(parent, id == EOID_CHILD ? oid() : id, flags());
    tmp = new eVariable(this);

    /* Slightly slow but simple clone. Optimize later if time.
     */
//    clonedobj->allocate(m_pixel_type, m_width, m_height);

    clonegeneric(clonedobj, aflags);

    delete tmp;
    return clonedobj;
}


/**
****************************************************************************************************

  @brief Function to process incoming messages.

  The eBitmap::onmessage function handles messages received by object. If this function
  doesn't process message, it calls parent class'es onmessage function.

  @param   envelope Message envelope. Contains command, target and source paths and
           message content, etc.

****************************************************************************************************
*/
void eBitmap::onmessage(
    eEnvelope *envelope)
{
    /* If at final destination for the message.
     */
    /* if (*envelope->target()=='\0')
    {
        switch (envelope->command())
        {
            case ECMD_CONFIGURE_TABLE:
            case ECMD_INSERT_ROWS_TO_TABLE:
            case ECMD_REMOVE_ROWS_FROM_TABLE:
            case ECMD_UPDATE_TABLE_ROWS:
            case ECMD_BIND_RS:
            case ECMD_RSET_SELECT:
                dbm_message(envelope);
                return;

            default:
                break;
        }
    } */

    /* Call parent class'es onmessage.
     */
    eTable::onmessage(envelope);
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
    os_long v;

    switch (propertynr)
    {
        case EBITMAPP_PIXEL_TYPE:
            if (m_own_change == 0) {
                v = x->getl();
                m_own_change++;
                resize((osalTypeId)v, m_width, m_height);
                m_own_change--;
            }
            break;

        case EBITMAPP_WIDTH:
            if (m_own_change == 0) {
                v = x->getl();
                m_own_change++;
                resize(m_pixel_type, (os_int)v, m_height);
                m_own_change--;
            }
            break;

        case EBITMAPP_HEIGHT:
            if (m_own_change == 0) {
                v = x->getl();
                m_own_change++;
                resize(m_pixel_type, m_width, (os_int)v);
                m_own_change--;
            }
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
        case EBITMAPP_PIXEL_TYPE:
            x->setl(m_pixel_type);
            break;

        case EBITMAPP_WIDTH:
            x->setl(m_width);
            break;

        case EBITMAPP_HEIGHT:
            x->setl(m_height);
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
    /* Version number. Increment if new serialized items are added to the object,
       and check for new version's items in read() function.
     */
    const os_int version = 0;

    /* Begin the object and write version number.
     */
    if (stream->write_begin_block(version)) goto failed;

    /* Write bitmap data type and size.
     */
    if (stream->putl(m_pixel_type)) goto failed;
    if (stream->putl(m_width)) goto failed;
    if (stream->putl(m_height)) goto failed;


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
    /* Version number. Used to check which versions item's are in serialized data.
     */
    os_long pixel_type, height, width;
    os_int version;

    /* If we have old data, delete it.
     */
    clear();

    /* Read object start mark and version number.
     */
    if (stream->read_begin_block(&version)) goto failed;

    /* Read bitmap data type and size, allocate bitmap.
     */
    if (stream->getl(&pixel_type)) goto failed;
    if (stream->getl(&height)) goto failed;
    if (stream->getl(&width)) goto failed;
    allocate((osalTypeId)pixel_type, (os_int)height, (os_int)width);

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
    os_boolean comma1;
    os_int row;

    indent++;
    if (json_puts(stream, "[")) goto failed;
    comma1 = OS_FALSE;
    for (row = 0; row < m_width; row++)
    {
        if (comma1) {
            if (json_puts(stream, ",")) goto failed;
        }
        comma1 = OS_TRUE;

        if (json_indent(stream, indent, EJSON_NEW_LINE_BEFORE /* , &comma */)) goto failed;
        if (json_puts(stream, "[")) goto failed;

#if 0
        comma2 = OS_FALSE;
        for (column = 0; column < m_height; column++)
        {
            if (comma2) {
                if (json_puts(stream, ",")) goto failed;
            }
            comma2 = OS_TRUE;

            /* If this is a table, we want to show row number instead of flags column.
             */
            if (m_columns && column == EBITMAP_FLAGS_COLUMN_NR) {
                // setl(row_nr, EBITMAP_FLAGS_COLUMN_NR, EBITMAP_FLAGS_ROW_OK);
                tmp.setl(row + 1);
                has_value = OS_TRUE;
            }
            else {
                has_value = getv(row, column, &tmp);
            }

            if (has_value)
            {
                type_id = tmp.type();
                if (OSAL_IS_BOOLEAN_TYPE(type_id) ||
                    OSAL_IS_INTEGER_TYPE(type_id) ||
                    OSAL_IS_FLOAT_TYPE(type_id))
                {
                    if (json_puts(stream, tmp.gets())) goto failed;
                }
                else if (type_id == OS_OBJECT)
                {
                    o = tmp.geto();
                    if (o) {
                        if (o->json_write(stream, sflags, indent)) goto failed;
                    }
                    else {
                        if (json_putqs(stream, "")) goto failed;
                    }
                }
                else
                {
                    if (json_putqs(stream, tmp.gets())) goto failed;
                }
            }
            else {
                if (json_putqs(stream, "")) goto failed;
            }
        }
#endif

        if (json_puts(stream, "]")) goto failed;
    }

    if (json_indent(stream, --indent, EJSON_NEW_LINE_BEFORE /* , &comma */)) goto failed;
    if (json_puts(stream, "]")) goto failed;

    return ESTATUS_SUCCESS;

failed:
    return ESTATUS_FAILED;
}
#endif


/* Allocate bitmap.
 */
void eBitmap::allocate(
    osalTypeId pixel_type,
    os_int height,
    os_int width)
{
    /* Make sure that data type is known.
     */
    switch (pixel_type)
    {
        case OS_OBJECT:
        case OS_CHAR:
        case OS_SHORT:
        case OS_INT:
        case OS_LONG:
        case OS_DEC01:
        case OS_DEC001:
        case OS_FLOAT:
        case OS_DOUBLE:
            break;

        case OS_BOOLEAN: pixel_type = OS_CHAR; break;
        case OS_UCHAR: pixel_type = OS_SHORT; break;
        case OS_USHORT: pixel_type = OS_INT; break;
        case OS_UINT: pixel_type = OS_LONG; break;
        case OS_INT64: pixel_type = OS_LONG; break;

        default:
            pixel_type = OS_OBJECT;
            break;
    }

    /* Set data type, element size and bitmap dimensions.
       If we have existing bitmap, resize the bitmap.
     */
    resize(pixel_type, height, width);
}


/**
****************************************************************************************************

  @brief Clear bitmap.

  The eBitmap::clear releases all data allocated for bitmap and sets bitmap size to 0, 0.

****************************************************************************************************
*/
void eBitmap::clear()
{
}



/**
****************************************************************************************************

  @brief Resize the bitmap.

  The eBitmap::resize function changes bitmap size.

  @param  pixel_type Is this RGB, grayscale, et, bitmap?
  @param  width New number of columns.
  @param  height New number of rows.
  @return None.

****************************************************************************************************
*/
void eBitmap::resize(
    osalTypeId pixel_type,
    os_int width,
    os_int height)
{
    m_pixel_type = pixel_type;
m_pixel_nbytes = 3;
    m_height = width;
    m_width = height;

    if (m_own_change == 0) {
        m_own_change++;
        setpropertyl(EBITMAPP_PIXEL_TYPE, m_pixel_type);
        setpropertyl(EBITMAPP_WIDTH, m_width);
        setpropertyl(EBITMAPP_HEIGHT, m_height);
        m_own_change--;
    }
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
        propertyv(EBITMAPP_TEXT, &tmp);
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
