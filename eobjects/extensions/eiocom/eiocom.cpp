/**

  @file    eiocom.cpp
  @brief   iocom within eobjects.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "extensions/eiocom/eiocom.h"

#if 0


/**
****************************************************************************************************

  @brief Initialize iocom data structures, etc..

  Setup iocom root class and creates global iocom root object.

****************************************************************************************************
*/
void eio_initialize(
    struct eNetService *net_service)
{
    eioRoot::setupclass();

    os_lock();
    eioRoot *root = new eioRoot(eglobal->process);
    root->addname("//netservice");
    root->set_netservice(net_service);

    os_unlock();
}


/* Release resources allocated for iocom data structures.
 */
void eio_shutdown()
{
}

#endif
