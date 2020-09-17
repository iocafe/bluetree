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


/**
****************************************************************************************************

  @brief Constructor and destructor.

****************************************************************************************************
*/
eStrBuffer::eStrBuffer()
{
    m_buf = OS_NULL;
    m_buf_sz = 0;
    // m_translated = false;
}

eStrBuffer::~eStrBuffer()
{
    clear();
}

void eStrBuffer::allocate(
    os_memsz sz)
{
    if (sz > m_buf_sz) {
        if (m_buf) {
            os_free(m_buf, m_buf_sz);
        }
        m_buf = os_malloc(sz, &m_buf_sz);
    }
}

void eStrBuffer::clear()
{
    if (m_buf) {
        os_free(m_buf, m_buf_sz);
        m_buf = OS_NULL;
        m_buf_sz = 0;
    }
}


void eStrBuffer::set(
    const os_char *text,
    os_memsz sz)
{
    allocate(sz);
    if (text == OS_NULL) {
        text = osal_str_empty;
    }
    os_strncpy(m_buf, text, m_buf_sz);
}


void eStrBuffer::setv(
    eVariable *value)
{
    os_char *ptr;
    os_memsz sz;

    ptr = value->gets(&sz);
    allocate(sz);
    os_memcpy(m_buf, ptr, sz);
}
