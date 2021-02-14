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
  Constructor
****************************************************************************************************
*/
eAttrBuffer::eAttrBuffer()
{
    m_initialized = false;
    m_digs = 2;
    m_show_as = E_SHOWAS_STRING;
    m_align = E_ALIGN_LEFT;
    m_drop_down_list = OS_NULL;
    m_tstr_flags = ETIMESTR_DISABLED;
    m_dstr_flags = EDATESTR_DISABLED;
    m_rdonly = OS_FALSE;
    m_nosave = OS_FALSE;
}


/**
****************************************************************************************************
  The destructor releases allocated memory.
****************************************************************************************************
*/
eAttrBuffer::~eAttrBuffer()
{
    clear();
}


/**
****************************************************************************************************
  @brief Release allocated memory and mark that attributes are not up to date.
  @return  None.
****************************************************************************************************
*/
void eAttrBuffer::clear()
{
    delete m_drop_down_list;
    m_drop_down_list = OS_NULL;
    m_initialized = false;
}


/**
****************************************************************************************************

  @brief Set attributes for LineEdit, eTreeNode.

  The eAttrBuffer::initialize_for_variable function function sets UI value formating related
  attributes assuming that properties are same as for eLineEdit, eTreeNode, etc.
  The function calls eAttrBuffer::initialize().

  @param   obj Pointer to eLineEdit, eTreeNode, etc. GUI component whose properties are
           similar to eVariable, or to eVariable holding column configuration.
  @return  None.

****************************************************************************************************
*/
void eAttrBuffer::initialize_for_variable(
    eObject *obj)
{
    eVariable attr, unit;
    osalTypeId type;
    os_int digs;
    os_double min, max;

    obj->propertyv(EVARP_ATTR, &attr);
    type = (osalTypeId)obj->propertyi(EVARP_TYPE);
    obj->propertyv(EVARP_UNIT, &unit);
    digs = obj->propertyi(EVARP_DIGS);
    min = obj->propertyd(EVARP_MIN);
    max = obj->propertyd(EVARP_MAX);

    initialize(&attr, type, &unit, digs, min, max, obj->classid());
}


/**
****************************************************************************************************

  @brief Set attributes as values of member variables.

  The eAttrBuffer::initialize() function function sets UI value formating related attributes
  within members of the eAttrBuffer class, making them quick to to access.
  The function takes property values of the GUI component as arguments, and sets up display
  related data accorringly

  @return  None.

****************************************************************************************************
*/
void eAttrBuffer::initialize(
    eVariable *attr,
    osalTypeId type,
    eVariable *unit,
    os_int digs,
    os_double min,
    os_double max,
    os_int cid)
{
    const os_char *list_str, *value;
    os_char *p, *e;
    os_memsz value_sz;
    eAlignment default_align;
    os_boolean is_table_column;

    is_table_column = (os_boolean)(cid == ECLASSID_VARIABLE);
    default_align = is_table_column ? E_ALIGN_LEFT : E_ALIGN_RIGHT;
    m_digs = digs;
    m_align = E_ALIGN_NONE;

    /* If this is drop down list, color selector, time stamp, etc.
       which selects type.
     */
    list_str = attr->gets();
    if (list_str) {
        value = osal_str_get_item_value(list_str, "align", &value_sz, OSAL_STRING_DEFAULT);
        if (value) {
            eVariable tmp;
            tmp.sets(value, value_sz);
            p = tmp.gets();

            if (is_table_column) {
                if (!os_strcmp(p, "cleft")) {
                    m_align = E_ALIGN_LEFT;
                }
                if (!os_strcmp(p, "ccenter")) {
                    m_align = E_ALIGN_CENTER;
                }
                if (!os_strcmp(p, "cright")) {
                    m_align = E_ALIGN_RIGHT;
                }
            }
            else {
                if (!os_strcmp(p, "left")) {
                    m_align = E_ALIGN_LEFT;
                }
                if (!os_strcmp(p, "center")) {
                    m_align = E_ALIGN_CENTER;
                }
                if (!os_strcmp(p, "right")) {
                    m_align = E_ALIGN_RIGHT;
                }
            }
        }

        m_rdonly = os_strstr(list_str, "rdonly", OSAL_STRING_SEARCH_ITEM_NAME) ? OS_TRUE : OS_FALSE;
        m_nosave = os_strstr(list_str, "nosave", OSAL_STRING_SEARCH_ITEM_NAME) ? OS_TRUE : OS_FALSE;

        value = osal_str_get_item_value(list_str, "enum", &value_sz, OSAL_STRING_DEFAULT);
        if (value) {
            m_show_as = E_SHOWAS_DROP_DOWN_ENUM;
            setup_list(value, value_sz);
            goto goon;
        }

        value = osal_str_get_item_value(list_str, "list", &value_sz, OSAL_STRING_DEFAULT);
        if (value) {
            m_show_as = E_SHOWAS_DROP_DOWN_LIST;
            setup_list(value, value_sz);
            goto goon;
        }

        value = osal_str_get_item_value(list_str, "tstamp", &value_sz, OSAL_STRING_DEFAULT);
        if (value) {
            eVariable tmp;
            tmp.sets(value, value_sz);
            p = tmp.gets();

            m_tstr_flags = ETIMESTR_DISABLED;
            m_dstr_flags = EDATESTR_DISABLED;
            while (OS_TRUE) {
                e = os_strchr(p, ','); /* in case new flags are added in future */
                if (e) *e = '\0';

                if (!os_strcmp(p, "min"))  {
                    m_tstr_flags = ETIMESTR_MINUTES;
                }
                else if (!os_strcmp(p, "sec")) {
                    m_tstr_flags = ETIMESTR_MINUTES|ETIMESTR_SECONDS;
                }
                else if (!os_strcmp(p, "msec")) {
                    m_tstr_flags = ETIMESTR_MINUTES|ETIMESTR_SECONDS|
                        ETIMESTR_MILLISECONDS;
                }
                else if (!os_strcmp(p, "usec")) {
                    m_tstr_flags = ETIMESTR_MINUTES|ETIMESTR_SECONDS|
                        ETIMESTR_MILLISECONDS|ETIMESTR_MICROSECONDS;
                }
                else if (!os_strcmp(p, "usec")) {
                    m_tstr_flags = ETIMESTR_MINUTES|ETIMESTR_SECONDS|
                        ETIMESTR_MILLISECONDS|ETIMESTR_MICROSECONDS;
                }
                else if (!os_strcmp(p, "yyyy")) {
                    m_dstr_flags = EDATESTR_FOUR_DIGIT_YEAR;
                }
                else if (!os_strcmp(p, "yy")) {
                    m_dstr_flags = EDATESTR_TWO_DIGIT_YEAR;
                }
                else if (!os_strcmp(p, "year")) {
                    m_dstr_flags = EDATESTR_YEAR;
                }
                else if (!os_strcmp(p, "month")) {
                    m_dstr_flags = EDATESTR_MONTH;
                }
                else if (!os_strcmp(p, "weekday")) {
                    m_dstr_flags = EDATESTR_WEEKDAY;
                }

                if (e == OS_NULL) break;
                p = e + 1;
            }

            /* If no sensible time stamp format, set something.
             */
            if (m_tstr_flags == ETIMESTR_DISABLED &&
                m_dstr_flags == ETIMESTR_DISABLED)
            {
                m_dstr_flags = EDATESTR_TWO_DIGIT_YEAR;
                m_tstr_flags = ETIMESTR_MINUTES|ETIMESTR_SECONDS;
            }

            m_show_as = E_SHOWAS_TIMESTAMP;
            goto goon;
        }
    }
    else {
        m_show_as = E_SHOWAS_STRING;
        m_align = E_ALIGN_LEFT;
        m_tstr_flags = ETIMESTR_DISABLED;
        m_dstr_flags = EDATESTR_DISABLED;
        m_rdonly = OS_FALSE;
        m_nosave = OS_FALSE;
    }

    if (OSAL_IS_UNDEFINED_TYPE(type))
    {
        if (max > min) {
            type = OS_DOUBLE;
            default_align = E_ALIGN_RIGHT;
        }

        else if (!unit->isempty()) {
            type = OS_DOUBLE;
            default_align = E_ALIGN_RIGHT;
        }
    }

    if (OSAL_IS_BOOLEAN_TYPE(type))
    {
        m_show_as = E_SHOWAS_CHECKBOX;
        if (is_table_column) default_align = E_ALIGN_CENTER;
    }
    else if (OSAL_IS_INTEGER_TYPE(type))
    {
        m_show_as = E_SHOWAS_INTEGER_NUMBER;
        default_align = E_ALIGN_RIGHT;
    }
    else if (OSAL_IS_FLOAT_TYPE(type))
    {
        m_show_as = E_SHOWAS_DECIMAL_NUMBER;
        default_align = E_ALIGN_RIGHT;
    }
    else {
        m_show_as = E_SHOWAS_STRING;
    }

goon:
    if (m_align == E_ALIGN_NONE) {
        m_align = default_align;
    }

    m_initialized = true;
}


/**
****************************************************************************************************

  @brief Make a drop-down list for the component.

  The eAttrBuffer::setup_list() function function parses a string like enum="1.candy,2.gina" and
  creates m_drop_down_list to to hold drop down list items. The m_drop_down_list is eContainer
  and each item is eVariable. Object identifier oid() of the eVariable is the enum value.

  @param   value Pointer to string to parse, this doesn't need to be '\0' terminated.
  @param   value_sz Number of characters in value to parse.
  @return  None.

****************************************************************************************************
*/
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

    delete m_drop_down_list;
    m_drop_down_list = new eContainer();

    tmp.sets(value, value_sz);
    p = tmp.gets();

    while (OS_TRUE) {
        s = osal_str_list_iter(buf, sizeof(buf), &p, OSAL_STRING_DEFAULT);
        if (s) break;
        switch (m_show_as)
        {
            case E_SHOWAS_DROP_DOWN_ENUM:
                id = (os_int)osal_str_to_int(buf, &bytes);
                if (id < 0) id = EOID_CHILD;
                while (buf[bytes] == '.' || osal_char_isspace(buf[bytes])) bytes++;

                item = new eVariable(m_drop_down_list, id);
                item->sets(buf + bytes);
                break;

            case E_SHOWAS_DROP_DOWN_LIST:
                item = new eVariable(m_drop_down_list, EOID_CHILD);
                item->sets(buf);
                break;

            default:
                break;
        }
    }
}
