/**

  @file    estrbuffer.h
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
#pragma once
#ifndef ESTRBUFFER_H_
#define ESTRBUFFER_H_
#include "egui.h"

/**
****************************************************************************************************

  @brief eStrBuffer class.

  The eStrBuffer is buffer used for line edits, etc.

****************************************************************************************************
*/
class eStrBuffer
{
public:
    /* Constructor and destructor.
     */
    eStrBuffer();
    ~eStrBuffer();

    void allocate(
        os_memsz sz);

    void clear();

    void set(
        const os_char *text,
        os_memsz sz);

    void setv(
        eVariable *value);

    inline os_char *ptr()
        {return m_buf; }

    inline os_memsz sz()
        {return m_buf_sz; }

protected:
    os_char *m_buf;
    os_memsz m_buf_sz;
    // bool m_translated;
};


#endif
