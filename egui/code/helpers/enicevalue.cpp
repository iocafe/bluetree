/**

  @file    eautolabel.cpp
  @brief   Format value nicely for UI display.
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

/* Format value nicely for UI display.
 */
void enice_value_for_ui(
    eVariable *value,
    eComponent *component,
    eAttrBuffer *attr)
{
    eVariable *item;
    eContainer *drop_down_list;
    os_double d;
    os_int id;

    if (attr) {
        switch (attr->showas())
        {
            case E_SHOWAS_INTEGER_NUMBER:
                if (!value->isempty())
                {
                    d = value->getd();
                    value->setl(os_round_long(d));
                }
                break;

            case E_SHOWAS_DECIMAL_NUMBER:
                if (!value->isempty())
                {
                    d = value->getd();
                    value->setd(d);
                    value->setdigs(attr->digs());
                }
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
