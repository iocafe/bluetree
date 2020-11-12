/**

  @file    estrbuffer.cpp
  @brief   String buffer for egui components.
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


#define BUF_SZ_EMPTY_STR 1

/**
****************************************************************************************************

  @brief Constructor and destructor.

****************************************************************************************************
*/
eStrBuffer::eStrBuffer()
{
    m_buf = OS_NULL;
    m_buf_sz = 0;
    m_extended_value = OS_FALSE;
    m_state_bits = OSAL_STATE_CONNECTED;
}

eStrBuffer::~eStrBuffer()
{
    clear();
}

void eStrBuffer::allocate(
    os_memsz sz)
{
    os_char *new_buf;
    os_memsz new_buf_sz;

    if (sz > m_buf_sz && sz > BUF_SZ_EMPTY_STR) {
        new_buf = os_malloc(sz, &new_buf_sz);

        if (m_buf_sz > BUF_SZ_EMPTY_STR) {
            os_memcpy(new_buf, m_buf, m_buf_sz);
            os_free(m_buf, m_buf_sz);
        }

        m_buf = new_buf;
        m_buf_sz = new_buf_sz;
    }
}

void eStrBuffer::clear()
{
    if (m_buf_sz > BUF_SZ_EMPTY_STR) {
        os_free(m_buf, m_buf_sz);
        m_buf = OS_NULL;
    }
    m_buf_sz = 0;
}

void eStrBuffer::setv(
    eVariable *value)
{
    os_char *ptr;
    os_memsz sz;
    eValueX *ex;

    ptr = value->gets(&sz);

    /* Do not allocate empty strings, these are common, expecially in units.
     */
    if (sz <= BUF_SZ_EMPTY_STR) {
        clear();
        m_buf_sz = BUF_SZ_EMPTY_STR;
    }

    else {
        allocate(sz);
        os_memcpy(m_buf, ptr, sz);
    }

    ex = value->getx();
    if (ex) {
        m_extended_value = OS_TRUE;
        m_state_bits = ex->sbits();
    }
}

void eStrBuffer::appends(
    const os_char *value)
{
    os_memsz sz, nsz;

    nsz = os_strlen(value);
    if (nsz <= 1) return;

    if (m_buf_sz > BUF_SZ_EMPTY_STR) {
        sz = os_strlen(m_buf);
        allocate(sz + nsz - 1);
        os_memcpy(m_buf + sz - 1, value, nsz);
    }
}


/* This function is typically used only when drawing, etc to avoid buffer allocation when
   inactive in memory. obj pointer is used to get property value and context for translation redirects, etc.
   @param  attr Can be os NULL if not needed.
 */
const os_char *eStrBuffer::get(
    eObject *obj,
    os_int propertynr,
    eAttrBuffer *attr)
{
    eVariable *tmp;

    if (m_buf_sz == 0) {
        tmp = new eVariable(obj, EOID_TEMPORARY, EOBJ_TEMPORARY_ATTACHMENT);
        obj->propertyv(propertynr, tmp);

        // Here we could do language translation !!!!!!!!!!!!!!!!

        setv(tmp);
        delete tmp;
    }

    return m_buf_sz == BUF_SZ_EMPTY_STR ? osal_str_empty : m_buf;
}
