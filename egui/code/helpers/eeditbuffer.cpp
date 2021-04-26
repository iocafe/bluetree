/**

  @file    eeditbuffer.cpp
  @brief   Edit buffer for ImGui line edits, etc.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

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
eEditBuffer::eEditBuffer()
{
    m_buf = OS_NULL;
    m_buf_sz = 0;
}

eEditBuffer::~eEditBuffer()
{
    clear();
}

void eEditBuffer::allocate(
    os_memsz sz)
{
    if (sz > m_buf_sz) {
        if (m_buf) {
            os_free(m_buf, m_buf_sz);
        }
        m_buf = os_malloc(sz, &m_buf_sz);
    }
}

void eEditBuffer::clear()
{
    if (m_buf) {
        os_free(m_buf, m_buf_sz);
        m_buf = OS_NULL;
        m_buf_sz = 0;
    }
}


void eEditBuffer::set(
    const os_char *text,
    os_memsz sz)
{
    allocate(sz);
    if (text == OS_NULL) {
        text = osal_str_empty;
    }
    os_strncpy(m_buf, text, m_buf_sz);
}
