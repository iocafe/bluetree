/**

  @file    eio_defs.h
  @brief   Common defines and variables for eio.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef EIO_DEFS_H_
#define EIO_DEFS_H_
#include "extensions/io/eio.h"

/* Property numbers.
 */
#define EIOP_CONNECTED 31
#define EIOP_BOUND 32
#define EIOP_ASSEMBLY_TYPE 35
#define EIOP_ASSEMBLY_EXP 36
#define EIOP_ASSEMBLY_IMP 37
#define EIOP_ASSEMBLY_PREFIX 38
#define EIOP_ASSEMBLY_TIMEOUT 39

/* Property names.
 */
extern const os_char
    eiop_connected[],
    eiop_bound[],
    eiop_assembly_type[],
    eiop_assembly_exp[],
    eiop_assembly_imp[],
    eiop_assembly_prefix[],
    eiop_assembly_timeout[];

#endif

