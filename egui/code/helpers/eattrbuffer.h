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
    E_SHOWAS_DECIMAL_NUMBER,
    E_SHOWAS_STRING,

    E_SHOWAS_TIMESTAMP,
    E_SHOWAS_DROP_DOWN_LIST,
    E_SHOWAS_DROP_DOWN_ENUM
}
eShowAs;


typedef enum
{
    E_TSTAMP_HHMM,
    E_TSTAMP_HHMMSS,
    E_TSTAMP_HHMMSSMMM,

    E_TS_DATE,
    E_TS_DATE_HHMM,
    E_TS_DATE_HHMMSS,
    E_TS_DATE_HHMMSSMMM
}
eTimeStampFormat;


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

    inline void for_variable(
        eComponent *component)
    {
        if (!m_initialized) {
            initialize_for_variable(component);
        }
    }

    const os_char *get(
        eComponent *component,
        os_int propertynr);

    inline eShowAs showas()
        {return m_show_as; }

    inline os_short digs()
        {return m_digs; }

    inline os_short tstr_flags()
        {return m_tstr_flags; }

    inline os_short dstr_flags()
        {return m_dstr_flags; }

    inline eContainer *get_list()
        {return m_drop_down_list;}

protected:
    void initialize_for_variable(
        eComponent *component);

    void initialize(
        eVariable *attr,
        osalTypeId type,
        eVariable *unit,
        os_int digs,
        os_double min,
        os_double max);

    void setup_list(
        const os_char *value,
        os_memsz value_sz);

    bool m_initialized;

    eShowAs m_show_as;

    /* Digits after decimal point
     */
    os_short m_digs;

    os_short m_tstr_flags;
    os_short m_dstr_flags;

    eContainer *m_drop_down_list;
};


#endif
