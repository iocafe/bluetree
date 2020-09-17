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
#include "egui.h"


/**
****************************************************************************************************

  @brief Constructor and destructor.

****************************************************************************************************
*/
eAttrBuffer::eAttrBuffer()
{
    m_initialized = false;
    m_buf = OS_NULL;
    m_buf_sz = 0;
}

eAttrBuffer::~eAttrBuffer()
{
    clear();
}

void eAttrBuffer::allocate(
    os_memsz sz)
{
    if (sz > m_buf_sz) {
        if (m_buf) {
            os_free(m_buf, m_buf_sz);
        }
        m_buf = os_malloc(sz, &m_buf_sz);
    }
}

void eAttrBuffer::clear()
{
    if (m_buf) {
        os_free(m_buf, m_buf_sz);
        m_buf = OS_NULL;
        m_buf_sz = 0;
    }
    m_initialized = false;
}

/* void eAttrBuffer::setv(
    eVariable *value)
{
    os_char *ptr;
    os_memsz sz;

    ptr = value->gets(&sz);
    allocate(sz);
    os_memcpy(m_buf, ptr, sz);
}
*/


/* This function is typically used only when drawing, etc to avoid buffer allocation when
   inactive in memory. component pointer is used to get context for translation redirects, etc.
 */
void eAttrBuffer::initialize_for_variable(
    eObject *obj)
{
    eVariable attr, unit;
    osalTypeId type;
    os_int digs;
    os_double min, max;

    obj->propertyv(EVARP_ATTR, &attr);
    type = (osalTypeId)obj->propertyl(EVARP_TYPE);
    obj->propertyv(EVARP_UNIT, &unit);
    digs = (os_int)obj->propertyl(EVARP_DIGS);
    min = obj->propertyd(EVARP_MIN);
    max = obj->propertyd(EVARP_MAX);

    initialize(&attr, type, &unit, digs, min, max);
}


void eAttrBuffer::initialize(
    eVariable *attr,
    osalTypeId type,
    eVariable *unit,
    os_int digs,
    os_double min,
    os_double max)
{

    /* If this is drop down list, color selector, time stamp, etc.
       which selects type.
     */

    if (OSAL_IS_UNDEFINED_TYPE(type))
    {
        if (max > min) {
            type = OS_DOUBLE;
        }

        else if (!unit->isempty()) {
            type = OS_DOUBLE;
        }
    }

    if (OSAL_IS_BOOLEAN_TYPE(type))
    {
        m_show_as = E_SHOWAS_CHECKBOX;
    }
    else if (OSAL_IS_INTEGER_TYPE(type))
    {
        m_show_as = E_SHOWAS_INTEGER_NUMBER;
    }
    else if (OSAL_IS_FLOAT_TYPE(type))
    {
        m_show_as = E_SHOWAS_FLOAT_NUMBER;
    }
    else {
        m_show_as = E_SHOWAS_STRING;
    }

    m_initialized = true;
}
