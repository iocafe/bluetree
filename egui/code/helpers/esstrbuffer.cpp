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
}

eStrBuffer::~eStrBuffer()
{
    clear();
}

void eStrBuffer::allocate(
    os_memsz sz)
{
    if (sz > m_buf_sz) {
        if (m_buf_sz > BUF_SZ_EMPTY_STR) {
            os_free(m_buf, m_buf_sz);
        }
        m_buf = os_malloc(sz, &m_buf_sz);
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
}

/* This function is typically used only when drawing, etc to avoid buffer allocation when
   inactive in memory. obj pointer is used to get property value and context for translation redirects, etc.
 */
const os_char *eStrBuffer::get(
    eObject *obj,
    os_int propertynr)
{
    if (m_buf_sz == 0) {
        eVariable tmp;
        obj->propertyv(propertynr, &tmp);

        // Here we could do language translation !!!!!!!!!!!!!!!!

        setv(&tmp);
    }

    return m_buf_sz == BUF_SZ_EMPTY_STR ? osal_str_empty : m_buf;
}
