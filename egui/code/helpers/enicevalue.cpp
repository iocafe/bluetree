/**

  @file    enicevalue.cpp
  @brief   Format value nicely for UI display.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "egui.h"

/* Format value nicely for UI display.
 */
void enice_value_for_ui(
    eVariable *value,
    eObject *obj, // for redirs, etc
    eAttrBuffer *attr)
{
    eVariable *item;
    eContainer *drop_down_list;
    os_double d;
    os_int id;
    osalTypeId type_id;

    if (attr) {
        switch (attr->showas())
        {
            case E_SHOWAS_INTEGER_NUMBER:
                type_id = value->type();
                if (!value->isempty() && !OSAL_IS_INTEGER_TYPE(type_id))
                {
                    d = value->getd();
                    value->setl(os_round_long(d));
                }
                break;

            case E_SHOWAS_DECIMAL_NUMBER:
                type_id = value->type();
                if (!value->isempty() && !OSAL_IS_FLOAT_TYPE(type_id))
                {
                    d = value->getd();
                    value->setd(d);
                }
                value->setdigs(attr->digs());
                break;

            case E_SHOWAS_TIMESTAMP:
                if (!value->isempty())
                {
                    os_long utc;
                    eLocalTime localt;
                    os_char buf[EDATETIME_STR_BUF_SZ];

                    utc = value->getl();
                    if (elocaltime(&localt, utc)) {
                        value->sets("bad tstamp");
                    }
                    else {
                        if (attr->dstr_flags() != EDATESTR_DISABLED) {
                            edate_make_str(&localt, buf, sizeof(buf), OS_NULL, attr->dstr_flags());
                            *value = buf;

                            if (attr->tstr_flags() != ETIMESTR_DISABLED) {
                                *value += " ";
                            }
                        }
                        else {
                            *value = "";
                        }

                        if (attr->tstr_flags() != ETIMESTR_DISABLED) {
                            etime_make_str(&localt, buf, sizeof(buf), OS_NULL, attr->tstr_flags());
                            *value += buf;
                        }
                    }
                }
                break;

            case E_SHOWAS_DROP_DOWN_ENUM:
                id = value->geti();
                drop_down_list = attr->get_list();
                if (drop_down_list) {
                    item = drop_down_list->firstv(id);
                    if (item) value->setv(item);
                }
                break;

            // CHECK FOR LANGUAGE TRANSLATION FOR STRINGS HERE

            default:
                break;
        }
    }
}


/* Format value nicely for UI display.
 * Current value of property to be set or updated.
 * new_value Edited string
 */
void enice_ui_value_to_internal_type(
    eVariable *value,
    eVariable *new_value,
    eObject *obj, // for redirs, etc
    eAttrBuffer *attr)
{
    os_double d;
    const os_char *str;

    if (attr) {
        switch (attr->showas())
        {
            case E_SHOWAS_INTEGER_NUMBER:
                if (!new_value->isempty()) {
                    d = new_value->getd();
                    value->setl(os_round_long(d));
                }
                else {
                    value->clear();
                }
                return;

            case E_SHOWAS_DECIMAL_NUMBER:
                if (!new_value->isempty()) {
                    d = new_value->getd();
                    value->setd(d);
                }
                else {
                    value->clear();
                }
                return;

            case E_SHOWAS_TIMESTAMP:
                str = new_value->gets();
                if (!new_value->isempty() && str)
                {
                    eLocalTime localt;
                    os_long utc;
                    os_memsz n_chars_parsed;
                    eStatus s = ESTATUS_FAILED, s2 = ESTATUS_FAILED;

                    utc = value->getl();
                    if (elocaltime(&localt, utc)) {
                        utc = etime();
                        elocaltime(&localt, utc);
                    }

                    if (attr->dstr_flags() != EDATESTR_DISABLED) {
                        s = edate_parse_str(&localt, str, OS_NULL, &n_chars_parsed);
                        str += n_chars_parsed;
                    }

                    if (attr->tstr_flags() != ETIMESTR_DISABLED) {
                        s2 = etime_parse_str(&localt, str, OS_NULL);
                    }

                    if (s == ESTATUS_SUCCESS || s2 == ESTATUS_SUCCESS) {
                        utc = emktime(&localt);
                        if (utc != -1) {
                            value->setl(utc);
                        }
                    }
                }
                else {
                    value->clear(); /* Here only if we allow empty input */
                }
                return;

            default:
                break;
        }
    }

    value->setv(new_value);
}
