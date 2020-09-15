/**

  @file    eAutoLabel.cpp
  @brief   Abstract GUI component.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  Base call for all GUI components (widgets)...

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "egui.h"


/**
****************************************************************************************************

  @brief Constructor.

****************************************************************************************************
*/
eAutoLabel::eAutoLabel()
{
    m_label[0] = '\0';
}


/**
****************************************************************************************************

  @brief Get hidden ImGui label reserved for this components.

  The eAutoLabel::get function reserved a label if none reserved yet.
  @return Pointer to the label string. Empty string if label cannot be generated.

****************************************************************************************************
*/
os_char *eAutoLabel::get(
    eComponent *component)
{
    os_char nbuf[OSAL_NBUF_SZ];
    os_long count = -1;

    if (m_label[0] != '\0') {
        return m_label;
    }

    /* while (component) {
        if (component->classid() == EGUICLASSID_WINDOW) {
            count = window->inc_autolabel_count();
        }
        component = component->parent();
    } */

    if (count == -1) {
        count = eglobal->eguiglobal->autolabel_count++;
    }

    osal_int_to_str(nbuf, sizeof(nbuf), count);

    os_strncpy(m_label, "##l", sizeof(m_label));
    os_strncat(m_label, nbuf, sizeof(m_label));
    return m_label;
}
