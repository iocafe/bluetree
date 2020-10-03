/**

  @file    eobjflags_helpers.h
  @brief   Convert various flags to strings.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    2.1.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "eobjects.h"

/* Convert eObject flags to string.
 */
void eobjflags_to_str(
    eVariable *strvar,
    os_uint flags)
{
    const os_char comma_sep[] = ",";

    strvar->clear();
    if (flags & EOBJ_IS_ATTACHMENT) { eliststr_appeneds(strvar, "attachment", comma_sep); }
    if (flags & EOBJ_HAS_NAMESPACE) { eliststr_appeneds(strvar, "namespace", comma_sep); }
    if (flags & EOBJ_NOT_CLONABLE) { eliststr_appeneds(strvar, "no-clo", comma_sep); }
    if (flags & EOBJ_NOT_SERIALIZABLE) { eliststr_appeneds(strvar, "no-ser", comma_sep); }

    if (flags & EOBJ_CUST_FLAG1) { eliststr_appeneds(strvar, "c1", comma_sep); }
    if (flags & EOBJ_CUST_FLAG2) { eliststr_appeneds(strvar, "c2", comma_sep); }
    if (flags & EOBJ_CUST_FLAG3) { eliststr_appeneds(strvar, "c3", comma_sep); }
    if (flags & EOBJ_CUST_FLAG4) { eliststr_appeneds(strvar, "c4", comma_sep); }
    if (flags & EOBJ_CUST_FLAG5) { eliststr_appeneds(strvar, "c5", comma_sep); }
    if (flags & EOBJ_CUST_FLAG6) { eliststr_appeneds(strvar, "c6", comma_sep); }
}

