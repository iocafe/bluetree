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

#define ESTRBUF_DEFAULT 0
#define ESTRBUF_SINGLELINE 1

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

    void clear();

    const os_char *get(
        eObject *component,
        os_int propertynr,
        eAttrBuffer *attr = OS_NULL,
        os_int flags = ESTRBUF_DEFAULT);

    void setv(
        eVariable *value);

    void appends(
        const os_char *value);

    inline const os_char *ptr() {return m_buf; }
    inline os_boolean isempty() {return (os_boolean)(m_buf == OS_NULL);}

    inline os_boolean isx() {return m_extended_value;}
    inline os_int sbits() {return m_state_bits;}

protected:
    void allocate(
        os_memsz sz);

    os_char *m_buf;
    os_memsz m_buf_sz;

    os_boolean m_extended_value;
    os_char m_state_bits;
};

#endif
