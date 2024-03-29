/**

  @file    etablehelpers.h
  @brief   Helper functions for using tables.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef ETABLEHELPERS_H_
#define ETABLEHELPERS_H_
#include "eobjects.h"

/* Get column index by name.
*/
os_int etable_column_ix(
    const os_char *column_name,
    eContainer *columns);

#endif
