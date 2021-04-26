/**

  @file    eeditbuffer.h
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
#pragma once
#ifndef EEDITBUFFER_H_
#define EEDITBUFFER_H_
#include "egui.h"

/**
****************************************************************************************************

  @brief eEditBuffer class.

  The eEditBuffer is buffer used for line edits, etc.

****************************************************************************************************
*/
class eEditBuffer
{
public:
    /* Constructor and destructor.
     */
    eEditBuffer();
    ~eEditBuffer();

    void allocate(
        os_memsz sz);

    void clear();

    void set(
        const os_char *text,
        os_memsz sz);

    inline os_char *ptr()
        {return m_buf; }

    inline os_memsz sz()
        {return m_buf_sz; }

protected:
    os_char *m_buf;
    os_memsz m_buf_sz;
};


#endif
