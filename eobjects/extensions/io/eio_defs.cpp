/**

  @file    eio_defs.cpp
  @brief   Common defines and variables for eio.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "extensions/io/eio.h"

/* Property names.
 */
const os_char
    eiop_connected[] = "connected",
    eiop_bound[] = "bound",
    eiop_assembly_type[] = "atype",
    eiop_assembly_exp[] = "exp",
    eiop_assembly_imp[] = "imp",
    eiop_assembly_timeout[] = "timeout";
