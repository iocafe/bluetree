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

    /* Get pointer to label string.
     */
    const os_char *get(
        eComponent *component);

    /* Set text to display as label before "##".
     */
    void set_text(
        eComponent *component,
        const os_char *text = OS_NULL);

    void clear(
        bool clear_count = false);

    inline bool is_set() {return m_label != OS_NULL; }

protected:
    void allocate(
        os_memsz sz);

    os_char *m_label;
    os_memsz m_label_sz;
    os_long m_count;
};


#endif
