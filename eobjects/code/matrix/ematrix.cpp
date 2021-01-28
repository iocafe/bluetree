/**

  @file    ematrix.cpp
  @brief   Matrix class.
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

/* Matrix property names.
 */
const os_char
    emtxp_datatype[] = "type",
    emtxp_nrows[] = "nrows",
    emtxp_ncolumns[] = "ncolumns",
    emtxp_configuration[] = "configuration";

/* Approximate size for one eBuffer, adjusted to memory allocation block.
 */
#define OEMATRIX_APPROX_BUF_SZ 128

/* Flags for getptrs() function.
 */
#define EMATRIX_ALLOCATE_IF_NEEDED 1
#define EMATRIX_CLEAR_ELEMENT 2

typedef union
{
    os_long l;
    os_double d;
    os_char *s;
    eObject *o;
}
eMatrixDataItem;

static const os_short emtx_no_short_value = OS_SHORT_MIN;
static const os_int emtx_no_int_value = OS_INT_MIN;
static const os_long emtx_no_long_value = OS_LONG_MIN;


/**
****************************************************************************************************
  Constructor.
****************************************************************************************************
*/
eMatrix::eMatrix(
    eObject *parent,
    e_oid id,
    os_int flags)
    : eTable(parent, id, flags)
{
    /** Initial data type is OS_OBJECT.
     */
    m_datatype = OS_OBJECT;
    m_typesz = typesz(m_datatype);
    m_elemsz = m_typesz + sizeof(os_char);

    /** Initial size is 0, 0
     */
    m_nrows = m_ncolumns = 0;

    m_own_change = 0;
    m_columns = OS_NULL;
}


/**
****************************************************************************************************
  Virtual destructor.
****************************************************************************************************
*/
eMatrix::~eMatrix()
{
    clear();
}


/**
****************************************************************************************************

  @brief Add eMatrix to class list.

  The eMatrix::setupclass function adds newobj function pointer to class list. This enables creating
  new objects dynamically by class identifier, which is used for serialization reader()
  functions.

  This needs to be called after eBuffer:setupclass(), there is a dependency in setup.

****************************************************************************************************
*/
void eMatrix::setupclass()
{
    const os_int cls = ECLASSID_MATRIX;

    /* Add the class to class list.
     */
    os_lock();
    eclasslist_add(cls, (eNewObjFunc)newobj, "eMatrix");
    addpropertyl(cls, EMTXP_DATATYPE, emtxp_datatype, "data type", EPRO_PERSISTENT|EPRO_SIMPLE);
    addpropertyl(cls, EMTXP_NROWS, emtxp_nrows, "nro rows", EPRO_PERSISTENT|EPRO_SIMPLE);
    addpropertyl(cls, EMTXP_NCOLUMNS, emtxp_ncolumns, "nro columns", EPRO_PERSISTENT|EPRO_SIMPLE);
    addproperty (cls, EMTXP_CONFIGURATION, emtxp_configuration, "configuration",
        EPRO_PERSISTENT|EPRO_SIMPLE);

    add_generic_table_properties(cls, ETABLE_BASIC_ATTR_GROUP);
    propertysetdone(cls);
    os_unlock();

    /* Test memory manager and eBuffer implementation, how much memory we
     * actually get when we request OEMATRIX_APPROX_BUF_SZ bytes. It can be significantly
     * more and we do not want to leave it unused.
     */
    eBuffer buf;
    buf.allocate(OEMATRIX_APPROX_BUF_SZ);
    eglobal->matrix_buffer_allocation_sz = buf.allocated();
}


/**
****************************************************************************************************

  @brief Clone object

  The eMatrix::clone function clones an eMatrix.

  @param  parent Parent for the clone.
  @param  id Object identifier for the clone.
  @param  aflags 0 for default operation. EOBJ_NO_MAP not to map names.
  @return Pointer to the clone.

****************************************************************************************************
*/
eObject *eMatrix::clone(
    eObject *parent,
    e_oid id,
    os_int aflags)
{
    eMatrix *clonedobj;
    eVariable *tmp;
    os_int row, column;

    clonedobj = new eMatrix(parent, id == EOID_CHILD ? oid() : id, flags());
    tmp = new eVariable(this);

    /* Slightly slow but simple clone. Optimize later if time.
     */
    clonedobj->allocate(m_datatype, m_nrows, m_ncolumns);
    for (row = 0; row < m_nrows; row++)
    {
        for (column = 0; column < m_ncolumns; column++)
        {
            if (getv(row, column, tmp))
            {
                clonedobj->setv(row, column, tmp);
            }
        }
    }

    clonegeneric(clonedobj, aflags);

    delete tmp;
    return clonedobj;
}


/**
****************************************************************************************************

  @brief Get next child matrix identified by oid.

  The eMatrix::nextm() function returns pointer to the next object of the same class.

  @param   id Object idenfifier. Default value EOID_CHILD specifies to count a child objects,
           which are not flagged as an attachment. Value EOID_ALL specifies to get count all
           child objects, regardless wether these are attachment or not. Other values
           specify object identifier, only children with that specified object identifier
           are searched for.

  @return  Pointer to the next eMatrix, or OS_NULL if none found.

****************************************************************************************************
*/
eMatrix *eMatrix::nextm(
    e_oid id)
{
    if (mm_handle == OS_NULL) return OS_NULL;
    eHandle *h = mm_handle->next(id);
    while (h)
    {
        if (h->object()->classid() == ECLASSID_MATRIX)
            return eMatrix::cast(h->object());

        h = h->next(id);
    }
    return OS_NULL;
}


/**
****************************************************************************************************

  @brief Function to process incoming messages.

  The eMatrix::onmessage function handles messages received by object. If this function
  doesn't process message, it calls parent class'es onmessage function.

  @param   envelope Message envelope. Contains command, target and source paths and
           message content, etc.

****************************************************************************************************
*/
void eMatrix::onmessage(
    eEnvelope *envelope)
{
    /* If at final destination for the message.
     */
    if (*envelope->target()=='\0')
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
    }

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
eStatus eMatrix::onpropertychange(
    os_int propertynr,
    eVariable *x,
    os_int flags)
{
    eObject *configuration;
    os_long v;

    switch (propertynr)
    {
        case EMTXP_DATATYPE:
            if (m_own_change == 0) {
                v = x->getl();
                m_own_change++;
                resize((osalTypeId)v, m_nrows, m_ncolumns);
                m_own_change--;
            }
            break;

        case EMTXP_NROWS:
            if (m_own_change == 0) {
                v = x->getl();
                m_own_change++;
                resize(m_datatype, (os_int)v, m_ncolumns);
                m_own_change--;
            }
            break;

        case EMTXP_NCOLUMNS:
            if (m_own_change == 0) {
                v = x->getl();
                m_own_change++;
                resize(m_datatype, m_nrows, (os_int)v);
                m_own_change--;
            }
            break;

        case EMTXP_CONFIGURATION:
            if (m_own_change == 0) {
                configuration = x->geto();
                if (configuration) if (configuration->classid() == ECLASSID_CONTAINER) {
                    m_own_change++;
                    configure((eContainer*)configuration);
                    m_own_change--;
                }
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
eStatus eMatrix::simpleproperty(
    os_int propertynr,
    eVariable *x)
{
    eContainer *c;

    switch (propertynr)
    {
        case EMTXP_DATATYPE:
            x->setl(m_datatype);
            break;

        case EMTXP_NROWS:
            x->setl(m_nrows);
            break;

        case EMTXP_NCOLUMNS:
            x->setl(m_ncolumns);
            break;

        case EMTXP_CONFIGURATION:
            c = firstc(EOID_TABLE_CONFIGURATION);
            if (c) {
                c = eContainer::cast(c->clone(ETEMPORARY));
                add_attribs_to_configuration(c, ETABLE_BASIC_ATTR_GROUP);
                x->seto(c, OS_TRUE);
            }
            else {
                x->clear();
            }
            break;

        default:
            return eObject::simpleproperty(propertynr, x);
    }
    return ESTATUS_SUCCESS;
}


/**
****************************************************************************************************

  @brief Write matrix to stream.

  The eMatrix::writer() function serializes the eBuffer to stream. This writes only the
  content, use eObject::write() to save also class information, attachements, etc.

  @param  stream The stream to write to.
  @param  sflags Serialization flags.

  @return If successfull the function returns ESTATUS_SUCCESS (0). If writing object to stream
          fails, value ESTATUS_WRITING_OBJ_FAILED is returned. Assume that all nonzero values
          indicate an error.

****************************************************************************************************
*/
eStatus eMatrix::writer(
    eStream *stream,
    os_int sflags)
{
    eBuffer *buffer;
    os_char *dataptr, *typeptr;
    e_oid id;
    os_int first_elem_ix, elem_ix, first_full_ix, full_count, i, per_block;
    os_boolean prev_isempty, isempty;

    /* Version number. Increment if new serialized items are added to the object,
       and check for new version's items in read() function.
     */
    const os_int version = 0;

    /* Begin the object and write version number.
     */
    if (stream->write_begin_block(version)) goto failed;

    /* Write matrix data type and size.
     */
    if (stream->putl(m_datatype)) goto failed;
    if (stream->putl(m_nrows)) goto failed;
    if (stream->putl(m_ncolumns)) goto failed;

    /* Write data as "full groups".
     */
    prev_isempty = OS_TRUE;
    first_full_ix = full_count = 0;

    per_block = elems_per_block();

    for (buffer = eBuffer::cast(first());
         buffer;
         buffer = eBuffer::cast(buffer->next()))
    {
        id = buffer->oid();
        if (id <= 0) continue;

        first_elem_ix = (id - 1) * per_block;

        dataptr = buffer->ptr();
        typeptr = dataptr + per_block * m_typesz;

        for (i = 0; i < per_block; i++)
        {
            elem_ix = first_elem_ix + i;

            /* If element is empty
             */
            isempty = OS_TRUE;
            switch (m_datatype)
            {
                case OS_OBJECT:
                    switch (typeptr[i])
                    {
                        case OS_LONG:
                        case OS_DOUBLE:
                        case OS_STR:
                        case OS_OBJECT:
                            isempty = OS_FALSE;
                        break;

                        default:
                            break;
                    }
                    break;

                case OS_CHAR:
                    isempty = (os_boolean)(*(os_char*)dataptr == OS_CHAR_MIN);
                    break;

                case OS_SHORT:
                case OS_DEC01:
                case OS_DEC001:
                    isempty = !os_memcmp(dataptr, &emtx_no_short_value, sizeof(os_short));
                    break;

                case OS_INT:
                    isempty = !os_memcmp(dataptr, &emtx_no_int_value, sizeof(os_int));
                    break;

                case OS_LONG:
                    isempty = !os_memcmp(dataptr, &emtx_no_long_value, sizeof(os_long));
                    break;

                case OS_FLOAT:
                case OS_DOUBLE:
                    isempty = (*typeptr == OS_UNDEFINED_TYPE);
                    break;

                default:
                    break;
            }

            if (isempty)
            {
                if (!prev_isempty)
                {
                    if (elementwrite(stream, first_full_ix,
                        full_count, sflags)) goto failed;
                    prev_isempty = OS_TRUE;
                }
            }
            else
            {
                if (prev_isempty)
                {
                    first_full_ix = elem_ix;
                    full_count = 1;
                    prev_isempty = OS_FALSE;
                }
                else
                {
                    full_count++;
                }
            }
        }
    }

    /* Last group.
     */
    if (!prev_isempty)
    {
        if (elementwrite(stream, first_full_ix, full_count, sflags))
            goto failed;
    }

    /* Write -1 to indicate end of data.
     */
    if (stream->putl(-1)) goto failed;

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

/* Write consequent non-empty matrix elements to stream.
 */
eStatus eMatrix::elementwrite(
    eStream *stream,
    os_int first_full_ix,
    os_int full_count,
    os_int sflags)
{
    eBuffer *buffer = OS_NULL;
    eMatrixDataItem mo;
    eObject *o = OS_NULL;
    os_char *s = OS_NULL, *dataptr, *typeptr;
    os_long l;
    os_double d;
    os_float f;
    osalTypeId datatype;
    os_int i, prev_buffer_nr, buffer_nr, elem_ix, ix_in_block, per_block, ii;
    os_short ss;

    if (stream->putl(first_full_ix)) goto failed;
    if (stream->putl(full_count)) goto failed;

    prev_buffer_nr = -1;

    per_block = elems_per_block();

    for (i = 0; i<full_count; i++)
    {
        elem_ix = first_full_ix + i;
        buffer_nr =  elem_ix / per_block + 1;
        if (buffer_nr != prev_buffer_nr)
        {
            buffer = eBuffer::cast(first(buffer_nr));
            if (buffer == OS_NULL)
            {
                osal_debug_error("ematrix.cpp: progerr 1.");
                return ESTATUS_FAILED;
            }

            prev_buffer_nr = buffer_nr;
        }

        dataptr = buffer->ptr();
        typeptr = dataptr + per_block * m_typesz;

        ix_in_block = elem_ix - (buffer_nr - 1) * per_block;
        dataptr += sizeof(eMatrixDataItem) * ix_in_block;
        typeptr += ix_in_block;

        datatype = OS_UNDEFINED_TYPE;
        switch (m_datatype)
        {
            case OS_OBJECT:
                os_memcpy(&mo, dataptr, sizeof(eMatrixDataItem));
                switch (*typeptr)
                {
                    case OS_LONG:
                        l = mo.l;
                        datatype = OS_LONG;
                        break;

                    case OS_DOUBLE:
                        d = (os_double)mo.l;
                        datatype = OS_DOUBLE;
                        break;

                    case OS_STR:
                        s = mo.s;
                        datatype = OS_STR;
                        break;

                    case OS_OBJECT:
                        o = mo.o;
                        datatype = OS_OBJECT;
                        break;

                    default:
                        break;
                }
                break;

            case OS_CHAR:
                l = *((os_char*)dataptr);
                datatype = OS_LONG;
                break;

            case OS_SHORT:
            case OS_DEC01:
            case OS_DEC001:
                os_memcpy(&ss, dataptr, sizeof(os_short));
                l = ss;
                datatype = OS_LONG;
                break;

            case OS_INT:
                os_memcpy(&ii, dataptr, sizeof(os_int));
                l = ii;
                datatype = OS_LONG;
                break;

            case OS_LONG:
                os_memcpy(&l, dataptr, sizeof(os_long));
                datatype = OS_LONG;
                break;

            case OS_FLOAT:
                os_memcpy(&f, dataptr, sizeof(os_float));
                datatype = OS_FLOAT;
                break;

            case OS_DOUBLE:
                os_memcpy(&d, dataptr, sizeof(os_double));
                datatype = OS_DOUBLE;
                break;

            default:
                break;
        }

        if (m_datatype == OS_OBJECT)
        {
            if (stream->putl(datatype)) goto failed;
        }

        switch (datatype)
        {
            case OS_LONG:
                if (stream->putl(l)) goto failed;
                break;

            case OS_FLOAT:
                if (stream->putf(f)) goto failed;
                break;

            case OS_DOUBLE:
                if (stream->putd(d)) goto failed;
                break;

            case OS_STR:
                osal_debug_assert(s);
                if (stream->puts(s)) goto failed;
                break;

            case OS_OBJECT:
                osal_debug_assert(o);
                if (o->write(stream, sflags)) goto failed;
                break;

            default:
                osal_debug_error("ematrix.cpp: progerr 2.");
                break;
        }
    }

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

  @brief Read a matrix from stream.

  The eMatrix::reader() function reads serialized eBuffer from stream. This function
  reads only the object content. To read whole object including attachments, names, etc,
  use eObject::read().

  @param  stream The stream to read from.
  @param  sflags Serialization flags.

  @return If successfull the function returns ESTATUS_SUCCESS (0). If writing object to stream
          fails, value ESTATUS_READING_OBJ_FAILED is returned. Assume that all nonzero values
          indicate an error.

****************************************************************************************************
*/
eStatus eMatrix::reader(
    eStream *stream,
    os_int sflags)
{
    /* Version number. Used to check which versions item's are in serialized data.
     */
    eObject *o;
    eVariable tmp;
    os_long datatype, nrows, ncolumns, first_full_ix, full_count, l;
    os_double d;
    os_float f;
    os_int version, elem_ix, i, row, column;

    /* If we have old data, delete it.
     */
    clear();

    /* Read object start mark and version number.
     */
    if (stream->read_begin_block(&version)) goto failed;

    /* Read matrix data type and size, allocate matrix.
     */
    if (stream->getl(&datatype)) goto failed;
    if (stream->getl(&nrows)) goto failed;
    if (stream->getl(&ncolumns)) goto failed;
    allocate((osalTypeId)datatype, (os_int)nrows, (os_int)ncolumns);

    /* Read data
     */
    while (OS_TRUE)
    {
        /* Read first full index. -1 indicates end of data.
         */
        if (stream->getl(&first_full_ix)) goto failed;
        if (first_full_ix == -1) break;

        /* Read number of full elements.
         */
        if (stream->getl(&full_count)) goto failed;

        /* Read elements
         */
        for (i = 0; i<full_count; i++)
        {
            elem_ix = (os_int)first_full_ix + i;
            row = elem_ix / m_ncolumns;
            column = elem_ix % m_ncolumns;

            /* If we have datatype, read it.
             */
            if (m_datatype == OS_OBJECT)
            {
                if (stream->getl(&datatype)) goto failed;
            }

            switch (datatype)
            {
                case OS_LONG:
                    if (stream->getl(&l)) goto failed;
                    setl(row, column, l);
                    break;

                case OS_FLOAT:
                    if (stream->getf(&f)) goto failed;
                    /* TO DO: SHOULD SETF BE IMPLEMETED TO MINIMIZE DOUBLE-FLOAT CONV RUNDING ERRORS? */
                    setd(row, column, f);
                    break;

                case OS_DOUBLE:
                    if (stream->getd(&d)) goto failed;
                    setd(row, column, d);
                    break;

                case OS_STR:
                    if (stream->gets(&tmp)) goto failed;
                    setv(row, column, &tmp);
                    break;

                case OS_OBJECT:
                    o = read(stream, sflags);
                    /* TO DO: MINOR SPEED OPTIMIZATION: HERE WE COULD ADOPT THE CONTENT, NOT COPY */
                    seto(row, column, o);
                    delete o;
                    break;
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

  @brief Write matrix specific content to stream as JSON.

  The eMatrix::json_writer() function writes class specific object content to stream as JSON.

  @param  stream The stream to write to.
  @param  sflags Serialization flags. Typically EOBJ_SERIALIZE_DEFAULT.
  @param  indent Indentation depth, 0, 1... Writes 2x this spaces at beginning of a line.

  @return If successfull the function returns ESTATUS_SUCCESS (0). If writing object to stream
          fails, value ESTATUS_WRITING_OBJ_FAILED is returned. Assume that all nonzero values
          indicate an error.

****************************************************************************************************
*/
eStatus eMatrix::json_writer(
    eStream *stream,
    os_int sflags,
    os_int indent)
{
    os_boolean comma1, comma2;
    eVariable tmp;
    eObject *o;
    os_int row, column, type_id;
    os_boolean has_value;

    indent++;
    if (json_puts(stream, "[")) goto failed;
    comma1 = OS_FALSE;
    for (row = 0; row < m_nrows; row++)
    {
        /* If this is a table, skip rows without "row ok" flag.
         */
        if (m_columns) {
            if ((getl(row, EMTX_FLAGS_COLUMN_NR) & EMTX_FLAGS_ROW_OK) == 0) {
                continue;
            }
        }

        if (comma1) {
            if (json_puts(stream, ",")) goto failed;
        }
        comma1 = OS_TRUE;

        if (json_indent(stream, indent, EJSON_NEW_LINE_BEFORE /* , &comma */)) goto failed;
        if (json_puts(stream, "[")) goto failed;
        comma2 = OS_FALSE;

        for (column = 0; column < m_ncolumns; column++)
        {
            if (comma2) {
                if (json_puts(stream, ",")) goto failed;
            }
            comma2 = OS_TRUE;

            /* If this is a table, we want to show row number instead of flags column.
             */
            if (m_columns && column == EMTX_FLAGS_COLUMN_NR) {
                // setl(row_nr, EMTX_FLAGS_COLUMN_NR, EMTX_FLAGS_ROW_OK);
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

        if (json_puts(stream, "]")) goto failed;
    }

    if (json_indent(stream, --indent, EJSON_NEW_LINE_BEFORE /* , &comma */)) goto failed;
    if (json_puts(stream, "]")) goto failed;

    return ESTATUS_SUCCESS;

failed:
    return ESTATUS_FAILED;
}
#endif


/* Allocate matrix.
 */
void eMatrix::allocate(
    osalTypeId datatype,
    os_int nrows,
    os_int ncolumns)
{
    /* Make sure that data type is known.
     */
    switch (datatype)
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

        case OS_BOOLEAN: datatype = OS_CHAR; break;
        case OS_UCHAR: datatype = OS_SHORT; break;
        case OS_USHORT: datatype = OS_INT; break;
        case OS_UINT: datatype = OS_LONG; break;
        case OS_INT64: datatype = OS_LONG; break;

        default:
            datatype = OS_OBJECT;
            break;
    }

    /* Set data type, element size and matrix dimensions.
       If we have existing matrix, resize the matrix.
     */
    resize(datatype, nrows, ncolumns);
}


/**
****************************************************************************************************

  @brief Clear matrix.

  The eMatrix::clear releases all data allocated for matrix and sets matrix size to 0, 0.

****************************************************************************************************
*/
void eMatrix::clear()
{
    eBuffer *buffer, *nextbuffer;

    for (buffer = eBuffer::cast(first());
         buffer;
         buffer = nextbuffer)
    {
        nextbuffer = eBuffer::cast(buffer->next());
        if (buffer->oid() > 0)
        {
            releasebuffer(buffer);
        }
    }

    m_nrows = m_ncolumns = 0;
}


/**
****************************************************************************************************

  @brief Store variable value into matrix.

  The eMatrix::set stores variable value to matrix. Row and column specify position in matrix.
  The first row is 0 and first columns is 0. The matrix is expanded if row/column specify position
  outside current matrix size.

  @param  row Row number, 0...
  @param  column Column number, 0...
  @param  x Value to store.
  @param  mflags Reserved for future, set 0 for now.
  @return None.

****************************************************************************************************
*/
void eMatrix::setv(
    os_int row,
    os_int column,
    eVariable *x,
    os_int mflags)
{
    /* If variable is empty, clear the value
     */
    if (x->isempty())
    {
        clear(row, column);
        return;
    }

    /* Switch by data type in variable.
     */
    switch (x->type())
    {
        case OS_BOOLEAN:
        case OS_CHAR:
        case OS_UCHAR:
        case OS_SHORT:
        case OS_USHORT:
        case OS_INT:
        case OS_UINT:
        case OS_LONG:
        case OS_INT64:
            setl(row, column, x->getl());
            break;

        case OS_FLOAT:
        case OS_DOUBLE:
        case OS_DEC01:
        case OS_DEC001:
            setd(row, column, x->getd());
            break;

        case OS_STR:
            sets(row, column, x->gets());
            break;

        case OS_OBJECT:
            seto(row, column, x->geto());
            break;

        default:
            osal_debug_error("ematrix.cpp: unknown variable data type.");
            clear(row, column);
            break;
    }
}


/**
****************************************************************************************************

  @brief Store integer value into matrix.

  The eMatrix::setl stores integer value to matrix. Row and column specify position in matrix.
  The first row is 0 and first columns is 0. The matrix is expanded if row/column specify position
  outside current matrix size.

  @param  row Row number, 0...
  @param  column Column number, 0...
  @param  x Value to store.
  @return None.

****************************************************************************************************
*/
void eMatrix::setl(
    os_int row,
    os_int column,
    os_long x)
{
    os_char *dataptr, *typeptr;
    os_short ss;
    os_int ii;
    os_float ff;
    os_double dd;
    eMatrixDataItem mo;

    /* Make sure that row and column are not negative.
     */
    if (checknegative(row, column)) return;

    /* Get pointer to data and if matrix data type is OS_OBJECT, also to
       data type of item.
     */
    dataptr = getptrs(row, column, &typeptr, EMATRIX_ALLOCATE_IF_NEEDED|EMATRIX_CLEAR_ELEMENT);
    if (dataptr == OS_NULL) return;

    switch (m_datatype)
    {
        case OS_CHAR:
            *((os_char*)dataptr) = (os_char)x;
            break;

        case OS_SHORT:
            ss = (os_short)x;
            os_memcpy(dataptr, &ss, sizeof(os_short));
            break;

        case OS_INT:
            ii = (os_int)x;
            os_memcpy(dataptr, &ii, sizeof(os_int));
            break;

        case OS_LONG:
            os_memcpy(dataptr, &x, sizeof(os_long));
            break;

        case OS_DEC01:
            ss = (os_short)(10 * x);
            os_memcpy(dataptr, &ss, sizeof(os_short));
            break;

        case OS_DEC001:
            ss = (os_short)(100 * x);
            os_memcpy(dataptr, &ss, sizeof(os_short));
            break;

        case OS_FLOAT:
            ff = (os_float)x;
            os_memcpy(dataptr, &ff, sizeof(os_float));
            *typeptr = OS_FLOAT;
            break;

        case OS_DOUBLE:
            dd = (os_double)x;
            os_memcpy(dataptr, &dd, sizeof(os_double));
            *typeptr = OS_DOUBLE;
            break;

        case OS_OBJECT:
            mo.l = x;
            os_memcpy(dataptr, &mo, sizeof(eMatrixDataItem));
            *typeptr = OS_LONG;
            break;

        default:
            break;
    }
}


/**
****************************************************************************************************

  @brief Store double value into matrix.

  The eMatrix::setd stores double precision floating point value to matrix. Row and column specify
  position in matrix. The first row is 0 and first columns is 0. The matrix is expanded if
  row/column specify position outside current matrix size.

  @param  row Row number, 0...
  @param  column Column number, 0...
  @param  x Value to store.
  @return None.

****************************************************************************************************
*/
void eMatrix::setd(
    os_int row,
    os_int column,
    os_double x)
{
    os_char *dataptr, *typeptr;
    os_short ss;
    os_int ii;
    os_long ll;
    os_float ff;
    eMatrixDataItem mo;

    /* Make sure that row and column are not negative.
     */
    if (checknegative(row, column)) return;

    /* Get pointer to data and if matrix data type is OS_OBJECT, also to
       data type of item.
     */
    dataptr = getptrs(row, column, &typeptr, EMATRIX_ALLOCATE_IF_NEEDED|EMATRIX_CLEAR_ELEMENT);
    if (dataptr == OS_NULL) return;

    switch (m_datatype)
    {
        case OS_CHAR:
            *((os_char*)dataptr) = eround_double_to_char(x);
            break;

        case OS_SHORT:
            ss = eround_double_to_short(x);
            os_memcpy(dataptr, &ss, sizeof(os_short));
            break;

        case OS_INT:
            ii = eround_double_to_int(x);
            os_memcpy(dataptr, &ii, sizeof(os_int));
            break;

        case OS_LONG:
            ll = eround_double_to_long(x);
            os_memcpy(dataptr, &ll, sizeof(os_long));
            break;

        case OS_DEC01:
            ss = eround_double_to_short(10.0 * x);
            os_memcpy(dataptr, &ss, sizeof(os_short));
            break;

        case OS_DEC001:
            ss = eround_double_to_short(100.0 * x);
            os_memcpy(dataptr, &ss, sizeof(os_short));
            break;

        case OS_FLOAT:
            ff = (os_float)x;
            os_memcpy(dataptr, &ff, sizeof(os_float));
            *typeptr = OS_FLOAT;
            break;

        case OS_DOUBLE:
            os_memcpy(dataptr, &x, sizeof(os_double));
            *typeptr = OS_DOUBLE;
            break;

        case OS_OBJECT:
            mo.d = x;
            os_memcpy(dataptr, &mo, sizeof(eMatrixDataItem));
            *typeptr = OS_DOUBLE;
            break;

        default:
            break;
    }
}


/**
****************************************************************************************************

  @brief Store string value into matrix.

  The eMatrix::sets stores string value to matrix. Row and column specify
  position in matrix. The first row is 0 and first columns is 0. The matrix is expanded if
  row/column specify position outside current matrix size.

  @param  row Row number, 0...
  @param  column Column number, 0...
  @param  x Value to store.
  @return None.

****************************************************************************************************
*/
void eMatrix::sets(
    os_int row,
    os_int column,
    const os_char *x)
{
    os_char *dataptr, *typeptr;
    os_long l;
    os_memsz sz;
    os_memsz count;
    os_double d;
    eMatrixDataItem mo;

    if (x == 0) x = "";
    if (*x == '\0')
    {
        clear(row, column);
        return;
    }

    /* Make sure that row and column are not negative.
     */
    if (checknegative(row, column)) return;

    /* Get pointer to data and if matrix data type is OS_OBJECT, also to
       data type of item.
     */
    dataptr = getptrs(row, column, &typeptr, EMATRIX_ALLOCATE_IF_NEEDED|EMATRIX_CLEAR_ELEMENT);
    if (dataptr == OS_NULL) return;

    switch (m_datatype)
    {
        case OS_CHAR:
        case OS_SHORT:
        case OS_INT:
        case OS_LONG:
            /* Convert string to integer.
             */
            l = osal_str_to_int(x, &count);
            if (count >= 1) {
                setl(row, column, l);
            }
            else {
                clear(row, column);
            }
            break;

        case OS_FLOAT:
        case OS_DOUBLE:
            /* Convert string to double.
             */
            d = osal_str_to_double(x, &count);
            if (count >= 1) {
                setd(row, column, d);
            }
            else {
                clear(row, column);
            }
            break;

        case OS_OBJECT:
            sz = os_strlen(x);
            mo.s = os_malloc(sz, OS_NULL);
            os_memcpy(mo.s, x, sz);
            os_memcpy(dataptr, &mo, sizeof(eMatrixDataItem));
            *typeptr = OS_STR;
            break;

        default:
            break;
    }
}


/**
****************************************************************************************************

  @brief Store object into matrix.

  The eMatrix::seto stores object to matrix. Row and column specify position in matrix.
  The first row is 0 and first columns is 0. The matrix is expanded if row/column specify
  position outside current matrix size.

  @param  row Row number, 0...
  @param  column Column number, 0...
  @param  x Value to store.
  @param  mflags Reserved for future, set 0 for now.
  @return None.

****************************************************************************************************
*/
void eMatrix::seto(
    os_int row,
    os_int column,
    eObject *x,
    os_int mflags)
{
    os_char *dataptr, *typeptr;
    eBuffer *buffer;
    eObject *o;
    eMatrixDataItem mo;

    /* Make sure that row and column are not negative.
     */
    if (checknegative(row, column)) return;

    /* Objects can be stored only if matrix data type is OS_OBJECT.
     */
    if (m_datatype != OS_OBJECT)
    {
        osal_debug_error("ematrix.cpp: cannot store object to matrix with fixed type.");
        return;
    }

    dataptr = getptrs(row, column, &typeptr, EMATRIX_ALLOCATE_IF_NEEDED|EMATRIX_CLEAR_ELEMENT, &buffer);
    if (dataptr == OS_NULL) return;

    o = x->clone(buffer, EOID_INTERNAL);
    o->setflags(EOBJ_TEMPORARY_ATTACHMENT);
    mo.o = o;
    os_memcpy(dataptr, &mo, sizeof(eMatrixDataItem));
    *typeptr = OS_OBJECT;
}


/**
****************************************************************************************************

  @brief Clear matrix element.

  The eMatrix::clear sets matrix element to empty. Row and column specify position in matrix.
  The first row is 0 and first columns is 0. The matrix is expanded if row/column specify
  position outside current matrix size.

  @param  row Row number, 0...
  @param  column Column number, 0...
  @return None.

****************************************************************************************************
*/
void eMatrix::clear(
    os_int row,
    os_int column)
{
    os_char *typeptr;

    /* Make sure that row and column are not negative.
     */
    if (checknegative(row, column)) return;

    /* Getptrs function is used to clear the elemenet.
     */
    getptrs(row, column, &typeptr, EMATRIX_CLEAR_ELEMENT);
}


/**
****************************************************************************************************

  @brief Clear matrix row.

  Calls clear(row, col) for all elements of matrix row.

  @param  row Row number, 0...
  @return None.

****************************************************************************************************
*/
void eMatrix::clear_row(
    os_int row)
{
    os_int col;

    if (row < 0 || row >= m_nrows) {
        return;
    }

    for (col = 0; col < m_ncolumns; col++)
    {
        clear(row, col);
    }
}


/**
****************************************************************************************************

  @brief Copy matrix row.

  @param  dstrow Destination row number, 0...
  @param  srcrow Destination row number, 0...
  @return None.

****************************************************************************************************
*/
void eMatrix::copy_row(
    os_int dstrow,
    os_int srcrow)
{
    eVariable *v;
    os_int col;

    if (srcrow == dstrow) return;

    if (srcrow < 0 || srcrow >= m_nrows) {
        clear_row(dstrow);
        return;
    }

    v = new eVariable(this, EOID_ITEM, EOBJ_TEMPORARY_ATTACHMENT);
    for (col = 0; col < m_ncolumns; col++)
    {
        getv(srcrow, col, v);
        setv(dstrow, col, v);
    }
    delete v;
}


/**
****************************************************************************************************

  @brief Get value from matrix.

  The eMatrix::get retrieves value from matrix. Row and column specify position in
  matrix. The first row is 0 and first columns is 0. If row/column specify position outside
  current matrix size, hasvalue is set to OS_FALSE and function returns zero.

  @param  row Row number, 0...
  @param  column Column number, 0...
  @param  x Variable into which to store the value.
  @return OS_TRUE if value was retrieved, os OS_FALSE if value is empty.

****************************************************************************************************
*/
os_boolean eMatrix::getv(
    os_int row,
    os_int column,
    eVariable *x)
{
    os_char *dataptr, *typeptr;
    eMatrixDataItem *mo;
    os_short s;
    os_int i;
    os_long l;
    os_double d;
    os_float f;

    /* Make sure that row and column are not negative.
     */
    if (checknegative(row, column)) goto return_empty;

    /* Get pointer to data and if matrix data type is OS_OBJECT, also to
       data type of item.
     */
    dataptr = getptrs(row, column, &typeptr, OS_FALSE);
    if (dataptr == OS_NULL) goto return_empty;

    switch (m_datatype)
    {
        case OS_OBJECT:
            mo = (eMatrixDataItem*)dataptr;
            switch (*typeptr)
            {
                case OS_LONG:
                    x->setl(mo->l);
                    break;

                case OS_DOUBLE:
                    x->setd(mo->d);
                    break;

                case OS_STR:
                    x->sets(mo->s);
                    break;

                case OS_OBJECT:
                    x->seto(mo->o);
                    break;

                default:
                    goto return_empty;
            }
            break;

        case OS_CHAR:
            l = *((os_char*)dataptr);
            if (l == OS_CHAR_MIN) goto return_empty;
            x->setl(l);
            break;

        case OS_SHORT:
            os_memcpy(&s, dataptr, sizeof(os_short));
            if (s == OS_SHORT_MIN) goto return_empty;
            x->setl(s);
            break;

        case OS_INT:
            os_memcpy(&i, dataptr, sizeof(os_int));
            if (i == OS_INT_MIN) goto return_empty;
            x->setl(i);
            break;

        case OS_LONG:
            os_memcpy(&l, dataptr, sizeof(os_long));
            if (l == OS_LONG_MIN) goto return_empty;
            x->setl(l);
            break;

        case OS_DEC01:
            os_memcpy(&s, dataptr, sizeof(os_short));
            if (s == OS_SHORT_MIN) goto return_empty;
            x->setd(0.1 * s);
            break;

        case OS_DEC001:
            os_memcpy(&s, dataptr, sizeof(os_short));
            if (s == OS_SHORT_MIN) goto return_empty;
            x->setd(0.01 * s);
            break;

        case OS_FLOAT:
            if (*typeptr == OS_UNDEFINED_TYPE) goto return_empty;
            os_memcpy(&f, dataptr, sizeof(os_float));
            x->setd(f);
            break;

        case OS_DOUBLE:
            if (*typeptr == OS_UNDEFINED_TYPE) goto return_empty;
            os_memcpy(&d, dataptr, sizeof(os_double));
            x->setd(d);
            break;

        default:
            goto return_empty;
    }
    return OS_TRUE;

return_empty:
    x->clear();
    return OS_FALSE;
}


/**
****************************************************************************************************

  @brief Get integer value from matrix.

  The eMatrix::getl retrieves an integer value from matrix. Row and column specify position in
  matrix. The first row is 0 and first columns is 0. If row/column specify position outside
  current matrix size, hasvalue is set to OS_FALSE and function returns zero.

  @param  row Row number, 0...
  @param  column Column number, 0...
  @param  hasvalue Set to OS_TRUE if matrix element has value, ot to oe_FALSE if element is empty.
  @return Value.

****************************************************************************************************
*/
os_long eMatrix::getl(
    os_int row,
    os_int column,
    os_boolean *hasvalue)
{
    os_char *dataptr, *typeptr;
    eMatrixDataItem mo;
    os_short s;
    os_int i;
    os_long l;
    os_double d;
    os_float f;

    /* Make sure that row and column are not negative.
     */
    if (checknegative(row, column)) goto return_empty;

    /* Get pointer to data and if matrix data type is OS_OBJECT, also to
       data type of item.
     */
    dataptr = getptrs(row, column, &typeptr, OS_FALSE);
    if (dataptr == OS_NULL) goto return_empty;

    switch (m_datatype)
    {
        case OS_OBJECT:
            os_memcpy(&mo, dataptr, sizeof(eMatrixDataItem));
            switch (*typeptr)
            {
                case OS_LONG:
                    l = mo.l;
                    break;

                case OS_DOUBLE:
                    l = eround_double_to_long(mo.d);
                    break;

                case OS_STR:
                    l = osal_str_to_int(mo.s, OS_NULL);
                    break;

                default:
                    goto return_empty;
            }
            break;

        case OS_CHAR:
            l = *((os_char*)dataptr);
            if (l == OS_CHAR_MIN) goto return_empty;
            break;

        case OS_SHORT:
            os_memcpy(&s, dataptr, sizeof(os_short));
            if (s == OS_SHORT_MIN) goto return_empty;
            l = s;
            break;

        case OS_INT:
            os_memcpy(&i, dataptr, sizeof(os_int));
            if (i == OS_INT_MIN) goto return_empty;
            l = i;
            break;

        case OS_LONG:
            os_memcpy(&l, dataptr, sizeof(os_long));
            if (l == OS_LONG_MIN) goto return_empty;
            break;

        case OS_DEC01:
            os_memcpy(&s, dataptr, sizeof(os_short));
            if (s == OS_SHORT_MIN) goto return_empty;
            l = eround_double_to_long(0.1 * s);
            break;

        case OS_DEC001:
            os_memcpy(&s, dataptr, sizeof(os_short));
            if (s == OS_SHORT_MIN) goto return_empty;
            l = eround_double_to_long(0.01 * s);
            break;

        case OS_FLOAT:
            if (*typeptr == OS_UNDEFINED_TYPE) goto return_empty;
            os_memcpy(&f, dataptr, sizeof(os_float));
            l = eround_float_to_long(f);
            break;

        case OS_DOUBLE:
            if (*typeptr == OS_UNDEFINED_TYPE) goto return_empty;
            os_memcpy(&d, dataptr, sizeof(os_double));
            l = eround_double_to_long(d);
            break;

        default:
            goto return_empty;
    }

    if (hasvalue) *hasvalue = OS_TRUE;
    return l;

return_empty:
    if (hasvalue) *hasvalue = OS_FALSE;
    return 0;
}


/**
****************************************************************************************************

  @brief Get double value from matrix.

  The eMatrix::getd retrieves a double precifion floating point number from matrix. Row and
  column specify position in matrix. The first row is 0 and first columns is 0. If row/column
  specify position outside current matrix size, hasvalue is set to OS_FALSE and function returns
  zero.

  @param  row Row number, 0...
  @param  column Column number, 0...
  @param  hasvalue Set to OS_TRUE if matrix element has value, ot to oe_FALSE if element is empty.
  @return Value.

****************************************************************************************************
*/
os_double eMatrix::getd(
    os_int row,
    os_int column,
    os_boolean *hasvalue)
{
    os_char *dataptr, *typeptr;
    eMatrixDataItem mo;
    os_short s;
    os_int i;
    os_long l;
    os_double d;
    os_float f;

    /* Make sure that row and column are not negative.
     */
    if (checknegative(row, column)) goto return_empty;

    /* Get pointer to data and if matrix data type is OS_OBJECT, also to
       data type of item.
     */
    dataptr = getptrs(row, column, &typeptr, OS_FALSE);
    if (dataptr == OS_NULL) goto return_empty;

    switch (m_datatype)
    {
        case OS_OBJECT:
            os_memcpy(&mo, dataptr, sizeof(eMatrixDataItem));
            switch (*typeptr)
            {
                case OS_LONG:
                    d = (os_double)mo.l;
                    break;

                case OS_DOUBLE:
                    d = mo.d;
                    break;

                case OS_STR:
                    d = osal_str_to_double(mo.s, OS_NULL);
                    break;

                default:
                    goto return_empty;
            }
            break;

        case OS_CHAR:
            l = *((os_char*)dataptr);
            if (l == OS_CHAR_MIN) goto return_empty;
            d = (os_double)l;
            break;

        case OS_SHORT:
            os_memcpy(&s, dataptr, sizeof(os_short));
            if (s == OS_SHORT_MIN) goto return_empty;
            d = (os_double)s;
            break;

        case OS_INT:
            os_memcpy(&i, dataptr, sizeof(os_int));
            if (i == OS_INT_MIN) goto return_empty;
            d = (os_double)i;
            break;

        case OS_LONG:
            os_memcpy(&l, dataptr, sizeof(os_long));
            if (l == OS_LONG_MIN) goto return_empty;
            d = (os_double)l;
            break;

        case OS_DEC01:
            os_memcpy(&s, dataptr, sizeof(os_short));
            if (s == OS_SHORT_MIN) goto return_empty;
            d = 0.1 * s;
            break;

        case OS_DEC001:
            os_memcpy(&s, dataptr, sizeof(os_short));
            if (s == OS_SHORT_MIN) goto return_empty;
            d = 0.01 * s;
            break;

        case OS_FLOAT:
            if (*typeptr == OS_UNDEFINED_TYPE) goto return_empty;
            os_memcpy(&f, dataptr, sizeof(os_float));
            d = f;
            break;

        case OS_DOUBLE:
            if (*typeptr == OS_UNDEFINED_TYPE) goto return_empty;
            os_memcpy(&d, dataptr, sizeof(os_double));
            break;

        default:
            goto return_empty;
    }

    if (hasvalue) *hasvalue = OS_TRUE;
    return d;

return_empty:
    if (hasvalue) *hasvalue = OS_FALSE;
    return 0;
}


/**
****************************************************************************************************

  @brief Resize the matrix.

  The eMatrix::resize function changes matrix size. Data in matrix is preserved.

  @param  nrows New number of rows.
  @param  ncolumns New number of columns.
  @return None.

****************************************************************************************************
*/
void eMatrix::resize(
    osalTypeId datatype,
    os_int nrows,
    os_int ncolumns)
{
    eBuffer *buffer, *nextbuffer;
    eVariable *tmp;
    eMatrix *m;
    os_int elem_ix, buffer_nr, minrows, mincolumns, row, column, per_block;

    /* If we need to reorganize, do it the hard way. This is slow, application
       should be written in such way that this is not needed repeatedly.
       We need to reorganize if:
       - We number of columns has changed and we have more than 1 row of data.
       - Datatype has changed and we have data.
     */
    if ((ncolumns != m_ncolumns || datatype != m_datatype)
        && (m_nrows > 1 || (datatype != m_datatype && m_nrows > 0))
        && m_ncolumns > 0)
    {
        tmp = new eVariable(this, EOID_ITEM, EOBJ_TEMPORARY_ATTACHMENT);
        m = new eMatrix(this, EOID_ITEM, EOBJ_TEMPORARY_ATTACHMENT);
        m->allocate(datatype, nrows, ncolumns);

        minrows = nrows < m_nrows ? nrows : m_nrows;
        mincolumns = ncolumns < m_ncolumns ? ncolumns : m_ncolumns;

        for (row = 0; row < minrows; row++)
        {
            for (column = 0; column < mincolumns; column++)
            {
                if (getv(row, column, tmp))
                {
                    m->setv(row, column, tmp);
                }
            }
        }

        clear();

        /* Adopt data buffers.
         */
        for (buffer = eBuffer::cast(m->first());
             buffer;
             buffer = nextbuffer)
        {
            nextbuffer = eBuffer::cast(buffer->next());
            if (buffer->oid() > 0) {
                buffer->adopt(this);
            }
        }

        delete m;
        delete tmp;
    }

    /* No reorganization.
     */
    else
    {
        /* Otherwise if we need to delete rows
         */
        if (nrows < m_nrows && m_nrows > 0 && m_ncolumns > 0)
        {
            /* Element index of last element to keep.
             */
            elem_ix = ((nrows-1) * m_ncolumns-1) + (ncolumns-1);

            /* Buffer number of last buffer to keep
             */
            per_block = elems_per_block();
            buffer_nr =  elem_ix / per_block + 1;

            /* Delete buffers with bigger number than buffer_nr
             */
            for (buffer = eBuffer::cast(first());
                 buffer;
                 buffer = nextbuffer)
            {
                nextbuffer = eBuffer::cast(buffer->next());
                if (buffer->oid() > buffer_nr)
                {
                    releasebuffer(buffer);
                }
            }
        }
    }

    m_datatype = datatype;
    m_typesz = typesz(m_datatype);
    m_elemsz = m_typesz;
    if (m_datatype == OS_OBJECT ||
        m_datatype == OS_DOUBLE ||
        m_datatype == OS_FLOAT)
    {
        m_elemsz += sizeof(os_char);
    }
    m_nrows = nrows;
    m_ncolumns =ncolumns;

    if (m_own_change == 0) {
        m_own_change++;
        setpropertyl(EMTXP_DATATYPE, m_datatype);
        setpropertyl(EMTXP_NROWS, m_nrows);
        setpropertyl(EMTXP_NCOLUMNS, m_ncolumns);
        m_own_change--;
    }
}


/**
****************************************************************************************************

  @brief Get pointer to data.

  The eMatrix::getptrs function retrieves pointer to data for element and if m_type is
  OS_OBJECT also element type pointer.

  If isset is OS_TRUE: New eBuffer objects are allocated as needed. The matrix is expanded
  if row/column is outside the matrix and matrix element is emptied.

  If isset is OS_FALSE: No new eBuffer objects are allocated, matrix is not expanded and
  the element value is not emptied.

  @param  row Row number, 0...
  @param  column Column number, 0...
  @param  typeptr Where to store pointer to data type for matrix containing objects.
  @param  flags EMATRIX_ALLOCATE_IF_NEEDED: Allocate memory block if it doesn't exist.
          If the flag is not given, function returns OS_NULL if the buffer doesn't exist.
          EMATRIX_CLEAR_ELEMENT If the matrix element currently contains object, it is
          deleted.
  @param  pbuffer Pointer to eBuffer containing the matrix element is stored here.
          Set OS_NULL if not needed.
  @return Pointer to element data.

****************************************************************************************************
*/
os_char *eMatrix::getptrs(
    os_int row,
    os_int column,
    os_char **typeptr,
    os_int flags,
    eBuffer **pbuffer)
{
    eBuffer *buffer;
    os_char *dataptr;
    os_int elem_ix, buffer_nr, per_block;

    /* If this is outside current matrix size.
     */
    if (row >= m_nrows ||
        column >= m_ncolumns)
    {
        /* If reading from matrix, it cannot be expanded.
         */
        if ((flags & EMATRIX_ALLOCATE_IF_NEEDED) == 0) return OS_NULL;

        /* Make matrix bigger to fit this point
         */
        resize(m_datatype, row >= m_nrows ? row + 1 : m_nrows,
            column >= m_ncolumns ? column + 1 : m_ncolumns);
    }

    /* Element index is
     */
    elem_ix = (row * m_ncolumns + column);

    per_block = elems_per_block();

    /* Buffer index from 1... and element index within buffer 0...
     */
    buffer_nr = elem_ix / per_block + 1;
    elem_ix %= per_block;

    /* Get eBuffer where this value belongs to.
     */
    buffer = getbuffer(buffer_nr, flags);
    if (buffer == OS_NULL) return OS_NULL;

    dataptr = buffer->ptr();
    if (m_datatype == OS_OBJECT || m_datatype == OS_DOUBLE || m_datatype == OS_FLOAT) {
        *typeptr = dataptr + per_block * m_typesz + elem_ix;
    }
    else {
        *typeptr = OS_NULL;
    }
    dataptr += elem_ix * m_typesz;

    /* Item found, dataptr and typeptr are set now. If this is
       set and m_datatype is OS_OBJECT, we check if we need to
       release object or string from memory.
     */
    if (flags & EMATRIX_CLEAR_ELEMENT)
    {
        emptyobject(dataptr, *typeptr);
    }
    if (pbuffer) *pbuffer = buffer;
    return dataptr;
}


/**
****************************************************************************************************

  @brief Get or allocate eBuffer by buffer number (oid).

  The eMatrix::getbuffer function...

  @param  buffer_nr Object ID
  @param  flags EMATRIX_ALLOCATE_IF_NEEDED: Allocate memory block if it doesn't exist.
          If the flag is not given, function returns OS_NULL if the buffer doesn't exist.
  @return Pointer to eBuffer.

****************************************************************************************************
*/
eBuffer *eMatrix::getbuffer(
    os_int buffer_nr,
    os_int flags)
{
    eBuffer *buffer;
    os_int bytes_per_elem, count;
    os_char *ptr;

    buffer = eBuffer::cast(first(buffer_nr));
    if (buffer || (flags & EMATRIX_ALLOCATE_IF_NEEDED) == 0) return buffer;

    buffer = new eBuffer(this, buffer_nr);

    bytes_per_elem = m_typesz;
    if (m_datatype == OS_OBJECT) bytes_per_elem += sizeof(os_char);

    /* eBuffer:allocate() fills buffer with zeroes.
     */
    buffer->allocate(eglobal->matrix_buffer_allocation_sz);

    /* If matrix data type has no type for element which is cleared by buffer->allocate(),
       but empry values are rather indicated by minimum integer, set empty values.
     */
    if (m_datatype != OS_OBJECT &&
        m_datatype != OS_FLOAT &&
        m_datatype != OS_DOUBLE)
    {
        ptr = buffer->ptr();
        count = elems_per_block();
        while (count--)
        {
            emptyobject(ptr, OS_NULL);
            ptr += m_typesz;
        }
    }

    return buffer;
}


/**
****************************************************************************************************

  @brief Free buffer and any objects/strings allocated for it.

  The eMatrix::releasebuffer function...
  @param  buffer_Pointer to buffer to free.

****************************************************************************************************
*/
void eMatrix::releasebuffer(
    eBuffer *buffer)
{
    eMatrixDataItem *mo;
    os_char *typeptr;
    os_int i, per_block;

    /* If this object data type.
     */
    if (m_datatype == OS_OBJECT)
    {
        mo = (eMatrixDataItem*)buffer->ptr();
        per_block = elems_per_block();
        typeptr = (os_char*)(mo + per_block);

        for (i = 0; i < per_block; i++)
        {
            switch (typeptr[i])
            {
                case OS_STR:
                    os_free(mo[i].s, os_strlen(mo[i].s));
                    break;

                case OS_OBJECT:
                    delete mo[i].o;
                    break;

                default:
                    break;
            }
        }
    }

    delete buffer;
}


/**
****************************************************************************************************

  @brief Make matrix element empty and release memory associated with it, if any.

  Release element memory allocated for matrix element (if any) and mark
  matrix value empty.

  @param  dataptr
  @param  typeptr

****************************************************************************************************
*/
void eMatrix::emptyobject(
    os_char *dataptr,
    os_char *typeptr)
{
    eMatrixDataItem mo;

    switch (m_datatype)
    {
        case OS_OBJECT:
            os_memcpy(&mo, dataptr, sizeof(eMatrixDataItem));
            switch (*typeptr)
            {
                case OS_STR:
                    os_free(mo.s, os_strlen(mo.s));
                    break;

                case OS_OBJECT:
                    delete mo.o;
                    break;

                default:
                    break;
            }

            os_memclear(dataptr, sizeof(eMatrixDataItem));
            *typeptr = OS_UNDEFINED_TYPE;
            break;

        case OS_CHAR:
            *(os_char*)dataptr = OS_CHAR_MIN;
            break;

        case OS_SHORT:
        case OS_DEC01:
        case OS_DEC001:
            os_memcpy(dataptr, &emtx_no_short_value, sizeof(os_short));
            break;

        case OS_INT:
            os_memcpy(dataptr, &emtx_no_int_value, sizeof(os_int));
            break;

        case OS_LONG:
            os_memcpy(dataptr, &emtx_no_long_value, sizeof(os_long));
            break;

        case OS_FLOAT:
            os_memclear(dataptr, sizeof(os_float));
            *typeptr = OS_UNDEFINED_TYPE;
            break;

        case OS_DOUBLE:
            os_memclear(dataptr, sizeof(os_double));
            *typeptr = OS_UNDEFINED_TYPE;
            break;

        default:
            break;
    }
}


/**
****************************************************************************************************

  @brief Get matrix element size in byte.

  How many bytes are needed in buffer to store this data type.

  @param  datatype Data type like OS_LONG, OS_DOUBLE, OS_OBJECT, etc.
  @param  Number of bytes needed in buffer.

****************************************************************************************************
*/
os_short eMatrix::typesz(
    osalTypeId datatype)
{
    if (datatype == OS_OBJECT) return sizeof(eMatrixDataItem);
    return (os_short)osal_type_size(datatype);
}


/**
****************************************************************************************************

  @brief How many element we can fit in memory buffer?

  How many matrix element are stored in one buffer. This depends on buffer allocation size
  and number of bytes needed to store the data and is there element specific type flag.

  @param  Number of elements what can ne stored in one buffer.

****************************************************************************************************
*/
os_int eMatrix::elems_per_block()
{
    return eglobal->matrix_buffer_allocation_sz/m_elemsz;
}


/**
****************************************************************************************************

  @brief Collect information about this matrix for tree browser, etc.

  The eMatrix::object_info function fills in item (eVariable) to contain information
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
void eMatrix::object_info(
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
void eMatrix::send_open_info(
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

    /* The "open" selection shows the matrix content as table.
     */
    if (command == EBROWSE_OPEN)
    {
        /* Created container for reply content.
         */
        reply = new eContainer(this, EOID_ITEM, EOBJ_IS_ATTACHMENT);
        new eVariable(reply, ECLASSID_MATRIX);

        eVariable tmp;
        propertyv(ETABLEP_TEXT, &tmp);
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
