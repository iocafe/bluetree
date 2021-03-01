/**

  @file    enetparameters.h
  @brief   Service parameters.
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
#ifndef ENETSERV_PRM_H_
#define ENETSERV_PRM_H_
#include "extensions/netservice/enetservice.h"

typedef struct
{
    eVariable *nickname;
    eVariable *enable_lighthouse_server;
    eVariable *serv_cert_path;
    eVariable *serv_priv_key;
}
eNetServPrm;

#endif
