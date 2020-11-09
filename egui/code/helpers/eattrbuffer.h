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

typedef enum
{
    E_ALIGN_NONE,
    E_ALIGN_LEFT,
    E_ALIGN_CENTER,
    E_ALIGN_RIGHT
}
eAlignment;


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

    /* Set attributes for LineEdit, eTreeNode, eVariable, etc.
     */
    inline void for_variable(
        eObject *obj)
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

    /* Get value alignment, left, center or right.
     */
    inline eAlignment alignment()
        {return m_align; }

    inline os_short digs()
        {return m_digs; }

    inline os_short tstr_flags()
        {return m_tstr_flags; }

    inline os_short dstr_flags()
        {return m_dstr_flags; }

    inline eContainer *get_list()
        {return m_drop_down_list;}

protected:

    /**
    ************************************************************************************************
      Protected member functions.
    ************************************************************************************************
    */

    /* Set attributes for LineEdit, eTreeNode.
     */
    void initialize_for_variable(
        eObject *obj);

    /* Set attributes as values of member variables.
     */
    void initialize(
        eVariable *attr,
        osalTypeId type,
        eVariable *unit,
        os_int digs,
        os_double min,
        os_double max,
        os_int cid);

    /* Make a drop-down list for the component.
     */
    void setup_list(
        const os_char *value,
        os_memsz value_sz);

    /**
    ************************************************************************************************
      Member variables.
    ************************************************************************************************
    */

    /* Flag indicating that the vValues stored in this class have been
       initailized and are up to date.
     */
    bool m_initialized;

    /* What type of data we have, editable number/string, check box, drop down list, etc.
     */
    eShowAs m_show_as;

    /* Align value to E_ALIGN_LEFT, E_ALIGN_CENTER or E_ALIGN_RIGHT.
     */
    eAlignment m_align;

    /* Number of digits after decimal point, significant only for decimal numbers.
     */
    os_short m_digs;

    os_short m_tstr_flags;
    os_short m_dstr_flags;

    eContainer *m_drop_down_list;
};


#endif
