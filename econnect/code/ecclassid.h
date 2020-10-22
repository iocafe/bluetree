/**

  @file    ecclassid.h
  @brief   Enumeration of econnect class identifiers.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    14.9.2020

  This header file defines class identifiers used by the econnect library.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef ECCLASSID_H_
#define ECCLASSID_H_
#include "econnect.h"

/**
****************************************************************************************************

  @name Class identifiers used by econnect library.

****************************************************************************************************
*/
#define ECONNCLASSID_BASE 196

#define ECONNCLASSID_ROOT        (ECONNCLASSID_BASE + 1)
#define ECONNCLASSID_CONNECT     (ECONNCLASSID_BASE + 2)


#endif
