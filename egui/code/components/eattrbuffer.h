/**

  @file    eattrbuffer.h
  @brief   Attribute buffer for egui components.
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
#ifndef EATTRBUFFER_H_
#define EATTRBUFFER_H_
#include "egui.h"

typedef enum
{
    E_SHOWAS_CHECKBOX,
    E_SHOWAS_INTEGER_NUMBER,
    E_SHOWAS_FLOAT_NUMBER,
    E_SHOWAS_STRING,

    E_SHOWAS_TIMESTAMP,
    E_SHOWAS_DROP_DOWN_LIST,
    E_SHOWAS_DROP_DOWN_ENUM
}
eShowAs;

/**
****************************************************************************************************

  @brief eAttrBuffer class.

  The eAttrBuffer is buffer used for line edits, etc.

****************************************************************************************************
*/
class eAttrBuffer
{
public:
    /* Constructor and destructor.
     */
    eAttrBuffer();
    ~eAttrBuffer();

    void clear();

    inline void for_variable(eObject *obj)
    {
        if (!m_initialized) {
            initialize_for_variable(obj);
        }
    }

    const os_char *get(
        eComponent *component,
        os_int propertynr);

    inline eShowAs showas()
        {return m_show_as; }


protected:
    void allocate(
        os_memsz sz);

    /* void setv(
        eVariable *value); */

    void initialize_for_variable(
        eObject *obj);

    void initialize(
        eVariable *attr,
        osalTypeId type,
        eVariable *unit,
        os_int digs,
        os_double min,
        os_double max);

    bool m_initialized;

    eShowAs m_show_as;

    os_char *m_buf;
    os_memsz m_buf_sz;
};


#endif
