/**

  @file    eset.cpp
  @brief   Simple set.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  The set stores enumerated collection of values.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "eobjects.h"

/* Bit combined with vale type to mark that this item is not cloned or serialized.
 */
#define ESET_TYPEID_TEMPORARY OSAL_TYPEID_EXTRA_BIT_A


/**
****************************************************************************************************

  @brief Constructor.

  X...

  @return  None.

****************************************************************************************************
*/
eSet::eSet(
    eObject *parent,
    e_oid id,
    os_int flags)
    : eObject(parent, id, flags)
{
    m_items = OS_NULL;
    m_used = m_alloc = 0;
}


/**
****************************************************************************************************

  @brief Virtual destructor.

  X...

  @return  None.

****************************************************************************************************
*/
eSet::~eSet()
{
    /* Clear the set to release all allocated memory.
     */
    clear();

    /* Release items buffer
     */
    os_free(m_items, m_alloc);
}


/**
****************************************************************************************************

  @brief Add eSet to class list.

  The eSet::setupclass function adds newobj function pointer to class list. This enables creating
  new objects dynamically by class identifier, which is used for serialization reader()
  functions.

****************************************************************************************************
*/
void eSet::setupclass()
{
    const os_int cls = ECLASSID_SET;

    /* Add the class to class list.
     */
    os_lock();
    eclasslist_add(cls, (eNewObjFunc)newobj, "eSet");
    os_unlock();
}


/**
****************************************************************************************************

  @brief Clone object

  The eSet::clone function clones and object including object's children.

  @param  parent Parent for the clone.
  @param  id Object identifier for the clone.
  @param  aflags 0 for default operation. EOBJ_NO_MAP not to map names.
  @return Pointer to the clone.

****************************************************************************************************
*/
eObject *eSet::clone(
    eObject *parent,
    e_oid id,
    os_int aflags)
{
    eSet *clonedobj;
    os_uchar *src, *dst;
    os_uchar ibytes, itype;
    os_int spos, dpos;
    os_memsz sz;

    clonedobj = new eSet(parent, id == EOID_CHILD ? oid() : id, flags());

    if (m_items)
    {
        clonedobj->m_items = (os_uchar*)os_malloc(m_used, &sz);
        clonedobj->m_used = m_used;
        clonedobj->m_alloc = (os_int)sz;

        /* Prepare to go trough items.
         */
        src = m_items;
        dst = clonedobj->m_items;

        spos = 0;
        dpos = 0;

        /* Clone item at a time.
         */
        while (spos < m_used)
        {
            ibytes = src[spos + 1] + 3;
            itype = src[spos + 2];
            if ((itype & ESET_TYPEID_TEMPORARY) == 0)
            {
                os_memcpy(dst + dpos, src + spos, ibytes);
                dpos += ibytes;
            }

            spos += ibytes;
        }
        clonedobj->m_used = dpos;
    }

    clonegeneric(clonedobj, aflags|EOBJ_CLONE_ALL_CHILDREN);
    return clonedobj;
}


/**
****************************************************************************************************

  @brief Get next child set identified by oid.

  The eSet::nexts() function returns pointer to the next object of the same class.

  @param   id Object idenfifier. Default value EOID_CHILD specifies to count a child objects,
           which are not flagged as an attachment. Value EOID_ALL specifies to get count all
           child objects, regardless wether these are attachment or not. Other values
           specify object identifier, only children with that specified object identifier
           are searched for.

  @return  Pointer to the next eSet, or OS_NULL if none found.

****************************************************************************************************
*/
eSet *eSet::nexts(
    e_oid id)
{
    if (mm_handle == OS_NULL) return OS_NULL;
    eHandle *h = mm_handle->next(id);
    while (h)
    {
        if (h->object()->classid() == ECLASSID_SET)
            return eSet::cast(h->object());

        h = h->next(id);
    }
    return OS_NULL;
}


/**
****************************************************************************************************

  @brief Write set content to stream.

  The eSet::writer() function serializes the eSet to stream. This writes only the
  content, use eObject::write() to save also class information, attachements, etc.

  @param  stream The stream to write to.
  @param  flags Serialization flags.

  @return If successfull the function returns ESTATUS_SUCCESS (0). If writing object to stream
          fails, value ESTATUS_WRITING_OBJ_FAILED is returned. Assume that all nonzero values
          indicate an error.

****************************************************************************************************
*/
eStatus eSet::writer(
    eStream *stream,
    os_int flags)
{
    /* Version number. Increment if new serialized items are added to the object,
       and check for new version's items in read() function.
     */
    const os_int version = 0;
    os_uchar *p, *e;
    eHandle *handle;
    os_int count;
    os_memsz nwritten;
    os_uchar iid, ibytes, itype;
    os_double d;
    os_long l;
    os_int i;
    os_short s;

    /* Begin the object and write version number.
     */
    if (stream->write_begin_block(version)) goto failed;

    /* Save items stored as variable.
     */
    if (mm_handle)
    {
        /* Calculate number of items to be saves and store write this number.
         */
        count = 0;
        for (handle = mm_handle->first();
             handle;
             handle = handle->next())
        {
            if (handle->oid() >= 0 && (handle->flags() & EOBJ_NOT_SERIALIZABLE) == 0)
            {
                count++;
            }
        }
        if (stream->putl(count)) goto failed;

        /* Write serializable items.
         */
        for (handle = mm_handle->first();
             handle;
             handle = handle->next())
        {
            if (handle->oid() >= 0 && (handle->flags() & EOBJ_NOT_SERIALIZABLE) == 0)
            {
                if (stream->putl(handle->oid())) goto failed;
                if (handle->object()->writer(stream, flags)) goto failed;
            }
        }
    }

    /* Buffer used, bytes.
     */
    if (stream->putl(m_used)) goto failed;

    /* Prepare to go trough items.
     */
    p = m_items;
    e = p + m_used;

    /* Search id from items until match found.
     */
    while (p < e)
    {
        iid = *(p++);
        ibytes = *(p++);
        itype = *(p++);

        if ((itype & ESET_TYPEID_TEMPORARY) == 0)
        {
            if (stream->putl(iid)) goto failed;
            if (stream->putl(itype)) goto failed;

            switch (itype & OSAL_TYPEID_MASK)
            {
                case OS_CHAR:
                    if (stream->putl(*(os_schar*)p)) goto failed;
                    break;

                case OS_SHORT:
                    os_memcpy(&s, p, sizeof(s));
                    if (stream->putl(s)) goto failed;
                    break;

                case OS_INT:
                    os_memcpy(&i, p, sizeof(i));
                    if (stream->putl(i)) goto failed;
                    break;

                case OS_LONG:
                    os_memcpy(&l, p, sizeof(l));
                    if (stream->putl(l)) goto failed;
                    break;

                case OS_DOUBLE:
                    os_memcpy(&d, p, sizeof(d));
                    if (stream->putd(d)) goto failed;
                    break;

                case OS_STR:
                    if (stream->putl(ibytes)) goto failed;
                    if (stream->write((os_char*)p, ibytes, &nwritten)) goto failed;
                    if (nwritten != ibytes) goto failed;
                    break;

                default:
                    osal_debug_error("eset: Unknown itype");
                    goto failed;
            }
        }

        p += ibytes;
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

  @brief Read set content from stream.

  The eSet::reader() function reads serialized set from stream. This function
  reads only the object content. To read whole object including attachments, names, etc,
  use eObject::read().

  @param  stream The stream to read from.
  @param  flags Serialization flags.

  @return If successfull the function returns ESTATUS_SUCCESS (0). If writing object to stream
          fails, value ESTATUS_READING_OBJ_FAILED is returned. Assume that all nonzero values
          indicate an error.

****************************************************************************************************
*/
eStatus eSet::reader(
    eStream *stream,
    os_int flags)
{
    /* Version number. Used to check which versions item's are in serialized data.
     */
    os_int version;
    os_long lval, count;
    os_memsz nread;
    eVariable *v;
    os_uchar *p, *e, *ibytes_pos;
    os_uchar iid, ibytes, itype;
    os_double d;
    os_int i;
    os_short s;

    /* Read object start mark and version number.
     */
    if (stream->read_begin_block(&version)) goto failed;

    /* Read item stored as variable.
     */
    if (mm_handle)
    {
        /* Read number if items.
         */
        if (stream->getl(&count)) goto failed;

        /* Read the items.
         */
        while (count-- > 0)
        {
            /* Read object identifier, allocate variable and
               read variable from stream.
             */
            if (stream->getl(&lval)) goto failed;
            v = new eVariable(this, (e_oid)lval);
            if (v->reader(stream, flags)) goto failed;
        }
    }

    /* Buffer used number of bytes for m_items buffer.
     */
    if (stream->getl(&lval)) goto failed;
    m_used = (os_int)lval;
    if (m_used == 0) goto skipit;

    /* Allocate buffer containing items
     */
    m_items = (os_uchar*)os_malloc(m_used, &nread);
    m_alloc = (os_int)nread;

    /* Prepare to go trough items.
     */
    p = m_items;
    e = p + m_used;

    /* Search id from items until match found.
     */
    while (p < e)
    {
        if (stream->getl(&lval)) goto failed;
        iid = (os_uchar)lval;
        if (stream->getl(&lval)) goto failed;
        itype = (os_uchar)lval;

        *(os_uchar*)(p++) = iid;
        ibytes_pos = p++;
        *(os_schar*)(p++) = itype;
        ibytes = (os_uchar)osal_type_size((osalTypeId)itype);
        *ibytes_pos = ibytes;

        switch (itype & OSAL_TYPEID_MASK)
        {
            case OS_CHAR:
                if (stream->getl(&lval)) goto failed;
                *(os_schar*)p = (os_schar)lval;
                break;

            case OS_SHORT:
                if (stream->getl(&lval)) goto failed;
                s = (os_short)lval;
                os_memcpy(p, &s, sizeof(s));
                break;

            case OS_INT:
                if (stream->getl(&lval)) goto failed;
                i = (os_int)lval;
                os_memcpy(p, &i, sizeof(i));
                break;

            case OS_LONG:
                if (stream->getl(&lval)) goto failed;
                os_memcpy(p, &lval, sizeof(lval));
                break;

            case OS_DOUBLE:
                if (stream->getd(&d)) goto failed;
                os_memcpy(p, &d, sizeof(d));
                break;

            case OS_STR:
                if (stream->getl(&lval)) goto failed;
                ibytes = (os_uchar)lval;
                if (stream->read((os_char*)p, ibytes, &nread)) goto failed;
                if (nread != lval) goto failed;
                *ibytes_pos = ibytes;
                break;

            default:
                osal_debug_error("eset: Unknown itype");
                goto failed;
        }

        p += ibytes;
    }

skipit:
    /* End the object.
     */
    if (stream->read_end_block()) goto failed;

    /* Object succesfully read.
     */
    return ESTATUS_SUCCESS;

    /* Reading object failed.
     */
failed:
    os_free(m_items, m_alloc);
    m_items = OS_NULL;
    m_alloc = m_used = 0;

    return ESTATUS_READING_OBJ_FAILED;
}


#if E_SUPPROT_JSON
/**
****************************************************************************************************

  @brief Write set to stream as JSON.

  The eSet::json_writer() function writes class specific object content to stream as JSON.

  @param  stream The stream to write to.
  @param  sflags Serialization flags. Typically EOBJ_SERIALIZE_DEFAULT.
  @param  indent Indentation depth, 0, 1... Writes 2x this spaces at beginning of a line.

  @return If successfull the function returns ESTATUS_SUCCESS (0). If writing object to stream
          fails, value ESTATUS_WRITING_OBJ_FAILED is returned. Assume that all nonzero values
          indicate an error.

****************************************************************************************************
*/
eStatus eSet::json_writer(
    eStream *stream,
    os_int sflags,
    os_int indent)
{
    eVariable x, *v;
    os_uchar *p, *e;
    os_char nbuf[OSAL_NBUF_SZ];
    os_uchar iid, ibytes, itype;
    os_boolean comma = OS_TRUE;

#if 0
    /* Try first if this value is stored in separate variable.
     */
     v = firstv(id);
    if (v)
    {
        x->setv(v);
        return OS_TRUE;
    }


    /* If this ID cannot be presented as unsigned char.
     */
    if (id < 0 || id > 255) goto getout;
#endif

    /* Prepare to go trough items.
     */
    p = m_items;
    if (p == OS_NULL) return ESTATUS_SUCCESS;
    e = p + m_used;

    /* Search id from items until match found.
     */
    while (p < e)
    {
        iid = *(os_uchar*)(p++);
        ibytes = *(os_uchar*)(p++);

        if (ibytes == 0)
        {
            x.clear();
        }
        else
        {
            itype = *(p++);

            switch (itype & OSAL_TYPEID_MASK)
            {
                case OS_CHAR:
                    x.setl(*(os_schar*)p);
                    break;

                case OS_SHORT:
                    x.setl(*(os_short*)p);
                    break;

                case OS_INT:
                    x.setl(*(os_int*)p);
                    break;

                case OS_LONG:
                    x.setl(*(os_long*)p);
                    break;

                case OS_DOUBLE:
                    if (ibytes == 1)
                        x.setd(*(os_schar*)p);
                    else
                        x.setd(*(os_double*)p);
                    break;

                case OS_STR:
                    x.sets((os_char*)p, ibytes);
                    break;

                default:
                    x.clear();
                    break;
            }
        }

        // if (json_puts(stream, ",\n")) goto failed;
        if (json_indent(stream, indent, EJSON_NEW_LINE_BEFORE, &comma)) goto failed;

        if (json_puts(stream, "\"i")) goto failed;
        osal_int_to_str(nbuf, sizeof(nbuf), iid);
        if (json_puts(stream, nbuf)) goto failed;
        if (json_puts(stream, "\": ")) goto failed;

        if (json_putv(stream, OS_NULL, &x, sflags, indent + 1)) goto failed;

        p += ibytes;
    }

    for (v = firstv(); v; v = v->nextv())
    {
        if (v->oid() < 0) continue;

        if (json_indent(stream, indent, EJSON_NEW_LINE_BEFORE, &comma)) goto failed;

        if (json_puts(stream, "\"v")) goto failed;
        osal_int_to_str(nbuf, sizeof(nbuf), v->oid());
        if (json_puts(stream, nbuf)) goto failed;
        if (json_puts(stream, "\": ")) goto failed;
        if (json_putv(stream, OS_NULL, v, sflags, indent + 1)) goto failed;
    }

    return ESTATUS_SUCCESS;

failed:
    return ESTATUS_FAILED;
}
#endif


/**
****************************************************************************************************

  @brief Store value into set.

  The eSet::setv function

  @param  id Identification number (for example property number) for value to store.
  @param  x Variable containing value to store.
          - x = OS_NULL -> delete value
  @param  sflags Least signigican bit is either ESET_PERSISTENT (0) or ESET_TEMPORARY (1).
          Temporary values are not cloned or serialized.
          Flag ESET_STORE_AS_VARIABLE (2) specifies that value is always stored as variable
          and never packed in m_items buffer. If value changes often, and expecially
          if value byte size varies, storing as bariable is faster, but takes more memory.
          if this eSet is used to store object's properities, property flag
          EPRO_NOPACK will select this option.

  @return None.

****************************************************************************************************
*/
void eSet::setv(
    os_int id,
    eVariable *x,
    os_int sflags)
{
    eVariable *v;
    os_uchar itype, ibytes, jbytes, jid;
    os_uchar *p, *e, *q, *start;
    os_char *bufp;
    os_memsz sz;
    const os_int slack = 10;
    void *iptr;
    os_double d;
    os_long l;
    os_int i;
    os_short s;
    os_schar c;

    osal_debug_assert(id >= 0);

    /* If we have variable with this id, use it.
     */
    v = firstv(id);
    if (v) {
        if (x) if (!x->isempty())
        {
            v->setv(x);
            return;
        }
        delete v;
        return;
    }

    /* If this id cannot be presented as uchar, use variable.
     */
    if (id < 0 || id > 255 || (sflags & ESET_STORE_AS_VARIABLE))
    {
        goto store_as_var;
    }

    /* Determnine size and type,
     */
    if (x == OS_NULL)
    {
        itype = OS_UNDEFINED_TYPE;
        ibytes = 0;
        iptr = OS_NULL;
    }
    else switch (x->type())
    {
        default:
        case OS_LONG:
            l = x->getl();
            if (l >= -0x80 && l <= 0x7F)
            {
                itype = OS_CHAR;
                ibytes = sizeof(os_schar);
                c = (os_schar)l;
                iptr = &c;
            }
            else if (l >= -0x8000 && l <= 0x7FFF)
            {
                itype = OS_SHORT;
                ibytes = sizeof(os_short);
                s = (os_short)l;
                iptr = &s;
            }
            else if (l >= -2147483647 && l <= 0x7FFFFFFF)
            {
                itype = OS_INT;
                ibytes = sizeof(os_int);
                i = (os_int)l;
                iptr = &i;
            }
            else
            {
                itype = OS_LONG;
                ibytes = sizeof(os_long);
                iptr = &l;
            }
            break;

        case OS_FLOAT:
        case OS_DOUBLE:
        case OS_DEC01:
        case OS_DEC001:
            d = x->getd();
            itype = OS_DOUBLE;
            ibytes = sizeof(os_double);
            iptr = &d;
            if (d == (os_double)x->getl())
            {
                if (d >= -128.0 && d <= 127.0)
                {
                    ibytes = sizeof(os_char);
                    c = os_round_char(d);
                    iptr = &c;
                    itype = OS_CHAR;
                }
                else if (d >= -32768.0 && d <= 32767.0)
                {
                    ibytes = sizeof(os_short);
                    s = os_round_short(d);
                    iptr = &s;
                    itype = OS_SHORT;
                }
            }
            break;

        case OS_OBJECT:
        case OS_POINTER:
            goto store_as_var;

        case OS_STR:
            q = (os_uchar*)x->gets(&sz);
            if (*q == '\0')
            {
                itype = OS_UNDEFINED_TYPE;
                ibytes = 0;
                iptr = OS_NULL;
            }
            else if (sz > 255)
            {
                goto store_as_var;
            }
            else
            {
                itype = OS_STR;
                ibytes = (os_uchar)sz;
                iptr = q;
            }
            break;

        case OS_UNDEFINED_TYPE: /* empty */
            itype = OS_UNDEFINED_TYPE;
            ibytes = 0;
            iptr = OS_NULL;
            break;
    }

    /* If this is temporary value, set temporary bit
     */
    if (sflags & ESET_TEMPORARY) {
        itype |= ESET_TYPEID_TEMPORARY;
    }

    /* Prepare to go trough items.
     */
    p = m_items;
    e = p + m_used;

    /* Search id from items until match found.
     */
    while (p < e)
    {
        start = p;
        jid = *(p++);
        jbytes = *(p++);
        p++;
        if (jid == (os_uchar)id)
        {
            /* If it is same length
             */
            if (ibytes == jbytes)
            {
                p = start + 2;
                *(p++) = itype;
                if (ibytes) {
                    os_memcpy(p, iptr, ibytes);
                }
                return;
            }

            /* Different length, remove this entry form m_items buffer.
             */
            p += jbytes;
            if (e != p)
            {
                os_memmove(start, p, e - p);
            }
            m_used -= (os_int)(p - start);
            break;
        }
        p += jbytes;
    }

    /* If no value.
     */
    if (ibytes == 0) return;

    /* If we need to allocate more memory?
     */
    if ((os_memsz)(m_used + ibytes + 3 * sizeof(os_char)) > m_alloc)
    {
        bufp = os_malloc(3 * sizeof(os_char)
            + ibytes + m_used + m_used/4 + slack, &sz);
        if (m_items)
        {
            os_memcpy(bufp, m_items, m_used);
            os_free(m_items, m_alloc);
        }
        m_alloc = (os_int)sz;
        m_items = (os_uchar*)bufp;
    }

    /* Append new value.
     */
    p = m_items + m_used;
    *(p++) = (os_uchar)id;
    *(p++) = ibytes;
    *(p++) = itype;
    if (ibytes)
    {
        os_memcpy(p, iptr, ibytes);
        p += ibytes;
    }
    m_used = (os_int)(p - m_items);
    return;

store_as_var:
    v = new eVariable(this, id, sflags & ESET_TEMPORARY
        ? EOBJ_NOT_CLONABLE|EOBJ_NOT_SERIALIZABLE : EOBJ_DEFAULT);
    v->setv(x);
}


/**
****************************************************************************************************

  @brief Store object into set.

  The eSet::seto function

  @param  id Identification number (for example property number) for value to store.
  @param  x Pointer to object to store
          - x = OS_NULL -> delete value
  @param  sflags sflags Least signigican bit is either ESET_PERSISTENT (0) or ESET_TEMPORARY (1).
          Temporary values are not cloned or serialized.
          Objects are are always stored as variable.

  @return None.

****************************************************************************************************
*/
void eSet::seto(
    os_int id,
    eObject *x,
    os_int sflags)
{
    eVariable *v;
    osal_debug_assert(id >= 0);

    /* If we have variable with this id, use it.
     */
    v = firstv(id);
    if (v) {
        if (x)
        {
            v->seto(x);
            return;
        }
        delete v;
        return;
    }

    if (x) {
        v = new eVariable(this, id, sflags & ESET_TEMPORARY
            ? EOBJ_NOT_CLONABLE|EOBJ_NOT_SERIALIZABLE : EOBJ_DEFAULT);
        v->seto(x);
    }
}


/**
****************************************************************************************************

  @brief Get value from set.

  The eSet::getv function

  @param  id Identification number (for example property number) for value to store.
  @param  x Variable containing value to store.
          - x = OS_NULL -> delete value
          - x = empty var -> store empty mark;
  @param  sflags Pointer to integer where to store flags. If not needed, set OS_NULL.
          Either ESET_PERSISTENT (0) or ESET_TEMPORARY (1). Temporary values are not
          cloned or serialized.

  @return Return value can be used between empty value and unset value. This is needed for
          properties. OS_TRUE if value was found, even empty one. OS_FALSE if no value for
          the ID was found.

****************************************************************************************************
*/
os_boolean eSet::getv(
    os_int id,
    eVariable *x,
    os_int *sflags)
{
    eVariable *v;
    os_uchar *p, *e;
    os_uchar iid, ibytes, itype;
    os_double d;
    os_long l;
    os_int i;
    os_short s;

    if (sflags) {
        *sflags = ESET_PERSISTENT;
    }

    /* Try first if this value is stored in separate variable.
     */
    v = firstv(id);
    if (v) {
        x->setv(v);
        if (sflags && (v->flags() & EOBJ_NOT_CLONABLE)) {
            *sflags = ESET_TEMPORARY;
        }
        return OS_TRUE;
    }

    /* If this ID cannot be presented as unsigned char.
     */
    if (id < 0 || id > 255) goto getout;

    /* Prepare to go trough items.
     */
    p = m_items;
    if (p == OS_NULL) goto getout;
    e = p + m_used;

    /* Search id from items until match found.
     */
    while (p < e)
    {
        iid = *(p++);
        ibytes = *(p++);
        itype = *(p++);
        if (iid == (os_uchar)id)
        {
            if (ibytes == 0)
            {
                x->clear();
                return OS_TRUE;
            }

            switch (itype & OSAL_TYPEID_MASK)
            {
                case OS_CHAR:
                    x->setl(*(os_schar*)p);
                    break;

                case OS_SHORT:
                    os_memcpy(&s, p, sizeof(s));
                    x->setl(s);
                    break;

                case OS_INT:
                    os_memcpy(&i, p, sizeof(i));
                    x->setl(i);
                    break;

                case OS_LONG:
                    os_memcpy(&l, p, sizeof(l));
                    x->setl(l);
                    break;

                case OS_DOUBLE:
                    os_memcpy(&d, p, sizeof(d));
                    x->setd(d);
                    break;

                case OS_STR:
                    x->sets((os_char*)p, ibytes);
                    break;

                default:
                    x->clear();
                    break;
            }

            return OS_TRUE;
        }

        p += ibytes;
    }

    /* continues...
     */
getout:
    x->clear();
    return OS_FALSE;
}


/**
****************************************************************************************************

  @brief Clear the set.

  The eSet::clear function clears all data in set, but doesn't release internal buffer. This
  is left for possiblt reuse.

****************************************************************************************************
*/
void eSet::clear()
{
    eVariable *v;

    while ((v = firstv())) {
        delete v;
    }

    m_used = 0;
}
