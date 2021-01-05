/**

  @file    eautolabel.h
  @brief   Generate Dear ImGui labels by enumeration.
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
#ifndef EAUTOLABEL_H_
#define EAUTOLABEL_H_
#include "egui.h"

class eComponent;
class eAttrBuffer;

/**
****************************************************************************************************

  @brief eAutoLabel class.

  The eAutoLabel is automatically generated hidden label for IMGui.

****************************************************************************************************
*/
class eAutoLabel
{
public:
    /* Constructor and destructor.
     */
    eAutoLabel();
    ~eAutoLabel();

    /* Release must be called before destructor gets called.
     */
    void release(
        eComponent *component);

    /* Get pointer to label string.
     */
    const os_char *get(
        eComponent *component,
        os_int propertynr = -1,
        os_int name_propertynr = -1,
        eAttrBuffer *attr = OS_NULL);

    void clear(
        bool clear_count = false);

    inline os_boolean isx() {return m_extended_value;}
    inline os_int sbits() {return m_state_bits;}

protected:
    /* Set text to display as label before "##".
     */
    void setstr(
        eComponent *component,
        const os_char *text,
        eVariable *name);

    void set(
        eComponent *component,
        os_int propertynr,
        os_int name_propertynr,
        eAttrBuffer *attr);

    void allocate(
        os_memsz sz);

    os_char *m_label;
    os_memsz m_label_sz;
    os_long m_count;
    os_boolean m_extended_value;
    os_char m_state_bits;
    os_char *m_autolabel;
    os_memsz m_autolabel_sz;
};


#endif
