/**

  @file    etypeenum_helpers.cpp
  @brief   Create string to enumerate data types in "attr" property.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  The enumeration allows equi, etc, to show drop down list, etc, to select data type.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "eobjects.h"

typedef struct
{
    osalTypeId type_id;
    const os_char *label;
}
eenumRegularType;

static eenumRegularType regular_types[] = {
    {OS_UNDEFINED_TYPE, "undefined"},
    {OS_BOOLEAN, "boolean"},
    {OS_LONG, "integer number"},
    {OS_DOUBLE, "decimal number"},
    {OS_STR, "string"},
    {OS_OBJECT, "object"}};

#define EENUM_NRO_REGULAR_TYPES (os_int)((sizeof(regular_types)/sizeof(eenumRegularType)))

/* Get user interface string label for type number
 */
const os_char *etype_to_label(
    osalTypeId type_id)
{
    os_int i;

    /* Try regular types.
     */
    for (i = 0; i<EENUM_NRO_REGULAR_TYPES; i++) {
        if (type_id == regular_types[i].type_id) {
            return regular_types[i].label;
        }
    }

    /* No match, return EOSAL library's type name.
     */
    return osal_typeid_to_name(type_id);
}

/* Generate enumeration of types for "attr" property.
 */
void emake_type_enum_str(
    eVariable *str,
    os_boolean append_it)
{
    os_int i;
    os_char nbuf[OSAL_NBUF_SZ];

    if (append_it) {
        if (str->isempty()) {
            *str += ",";
        }
    }
    else {
        str->clear();
    }

    *str += "enum=\"";

    /* Append regular types.
     */
    for (i = 0; i<EENUM_NRO_REGULAR_TYPES; i++) {
        if (i) *str += ",";
        osal_int_to_str(nbuf, sizeof(nbuf), (os_long)regular_types[i].type_id);
        *str += nbuf;
        *str += ". ";
        *str += regular_types[i].label;
    }

    *str += "\"";
}
