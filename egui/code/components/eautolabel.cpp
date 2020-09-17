/**

  @file    eautolabel.cppl
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
#include "egui.h"


/**
****************************************************************************************************

  @brief Constructor and destructor.

****************************************************************************************************
*/
eAutoLabel::eAutoLabel()
{
    m_label = OS_NULL;
    m_label_sz = 0;
    m_count = 0;
}

eAutoLabel::~eAutoLabel()
{
    clear();
}

void eAutoLabel::allocate(
    os_memsz sz)
{
    if (sz > m_label_sz) {
        if (m_label) {
            os_free(m_label, m_label_sz);
        }
        m_label = os_malloc(sz, &m_label_sz);
    }
}

void eAutoLabel::clear(
    bool clear_count)
{
    if (m_label) {
        os_free(m_label, m_label_sz);
        m_label = OS_NULL;
        m_label_sz = 0;
    }

    if (clear_count) {
        m_count = 0;
    }
}


/**
****************************************************************************************************

  @brief Get hidden ImGui label reserved for this components.

  The eAutoLabel::get function reserved a label if none reserved yet.
  @return Pointer to the label string. Empty string if label cannot be generated.

****************************************************************************************************
*/
const os_char *eAutoLabel::get(
    eComponent *component)
{
    if (m_label) {
        return m_label;
    }

    set_text(component);
    return m_label;
}


/**
****************************************************************************************************

  @brief Store text Get hidden ImGui label reserved for this components.

  The eAutoLabel::set_text function...
  @return None.

****************************************************************************************************
*/
void eAutoLabel::set_text(
    eComponent *component,
    const os_char *text)
{
    const os_char hide_label_mark[] = "##l";
    os_char nbuf[OSAL_NBUF_SZ];
    os_memsz sz;
    eWindow *window;

    /* Make sure that we have count.
     */
    if (m_count == 0) {
        window = component->window(true);
        if (window) {
            // count = window->inc_autolabel_count();
        }

        if (m_count == 0) {
            m_count = ++(eglobal->eguiglobal->autolabel_count);
        }
    }

    sz = osal_int_to_str(nbuf, sizeof(nbuf), m_count)
       + os_strlen(hide_label_mark) - 1;

    if (text) {
        sz += os_strlen(text) - 1;
    }
    else {
        text = osal_str_empty;
    }

    allocate(sz);
    os_strncpy(m_label, text, m_label_sz);
    os_strncat(m_label, "##l", m_label_sz);
    os_strncat(m_label, nbuf, m_label_sz);
}


