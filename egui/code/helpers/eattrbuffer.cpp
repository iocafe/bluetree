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
    m_digs = 2;
    m_show_as = E_SHOWAS_STRING;
    m_drop_down_list = OS_NULL;
}

eAttrBuffer::~eAttrBuffer()
{
    clear();
}


void eAttrBuffer::clear()
{
    if (m_drop_down_list) {
        delete m_drop_down_list;
        m_drop_down_list = OS_NULL;
    }
    m_initialized = false;
}


/* This function is typically used only when drawing, etc to avoid buffer allocation when
   inactive in memory. component pointer is used to get context for translation redirects, etc.
 */
void eAttrBuffer::initialize_for_variable(
    eComponent *component)
{
    eVariable attr, unit;
    osalTypeId type;
    os_int digs;
    os_double min, max;

    component->propertyv(EVARP_ATTR, &attr);
    type = (osalTypeId)component->propertyi(EVARP_TYPE);
    component->propertyv(EVARP_UNIT, &unit);
    digs = component->propertyi(EVARP_DIGS);
    min = component->propertyd(EVARP_MIN);
    max = component->propertyd(EVARP_MAX);

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
    const os_char *list_str, *value;
    os_memsz value_sz;

    m_digs = digs;

    /* If this is drop down list, color selector, time stamp, etc.
       which selects type.
     */
    list_str = attr->gets();
    if (list_str) {
        value = osal_str_get_item_value(list_str, "enum", &value_sz, OSAL_STRING_DEFAULT);
        if (value) {
            m_show_as = E_SHOWAS_DROP_DOWN_ENUM;
            setup_list(value, value_sz);
            goto goon;
        }
    }

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
        m_show_as = E_SHOWAS_DECIMAL_NUMBER;
    }
    else {
        m_show_as = E_SHOWAS_STRING;
    }

goon:
    m_initialized = true;
}

void eAttrBuffer::setup_list(
    const os_char *value,
    os_memsz value_sz)
{
    eVariable tmp, *item;
    const os_char *p;
    os_memsz bytes;
    os_char buf[128];
    osalStatus s;
    os_int id;

    if (m_drop_down_list) {
        delete m_drop_down_list;
    }
    m_drop_down_list = new eContainer();

    tmp.sets(value, value_sz);
    p = tmp.gets();

    while (OS_TRUE) {
        s = osal_str_list_iter(buf, sizeof(buf), &p, OSAL_STRING_DEFAULT);
        if (s) break;
        if (m_show_as == E_SHOWAS_DROP_DOWN_ENUM)
        {
            id = osal_str_to_int(buf, &bytes);
            if (id < 0) id = EOID_CHILD;
            while (buf[bytes] == '.' || osal_char_isspace(buf[bytes])) bytes++;

            item = new eVariable(m_drop_down_list, id);
            item->sets(buf + bytes);
        }
    }
}
