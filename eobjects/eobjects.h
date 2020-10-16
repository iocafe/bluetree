/**

  @file    eobjects.h
  @brief   Main eobject library header file.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  This eobject library base main header file. If further includes rest of eobjects base
  headers.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef EOBJECTS_H_
#define EOBJECTS_H_
#include "eobjects.h"

/* Include extended eosal headers.
 */
#include "eosalx.h"

/* Initialize eobject library for use.
 */
void eobjects_initialize(
    void *reserved);

/* Shut down eobjects library.
*/
void eobjects_shutdown();

/* Include eobject headers.
 */
#include "code/defs/estatus.h"
#include "code/defs/edefs.h"
#include "code/defs/eoid.h"
#include "code/defs/eclassid.h"
#include "code/defs/emacros.h"
#include "code/defs/etypes.h"
#include "code/object/ehandle.h"
#include "code/object/eobject.h"
#include "code/object/ehandletable.h"
#include "code/object/ehandleroot.h"
#include "code/global/eclasslist.h"
#include "code/root/eroot.h"
#include "code/variable/evariable.h"
#include "code/set/eset.h"
#include "code/container/econtainer.h"
#include "code/pointer/epointer.h"
#include "code/valuex/evaluex.h"
#include "code/name/ename.h"
#include "code/name/enamespace.h"
#include "code/binding/ebinding.h"
#include "code/binding/epropertybinding.h"
#include "code/envelope/eenvelope.h"
#include "code/table/ewhere.h"
#include "code/table/erange.h"
#include "code/table/etable.h"
#include "code/table/erowset.h"
#include "code/table/edbm.h"
#include "code/table/etablemessages.h"
#include "code/matrix/ematrix.h"
#include "code/thread/ethreadhandle.h"
#include "code/thread/ethread.h"
#include "code/timer/etimer.h"
#include "code/time/etime.h"
#include "code/global/eprocess.h"
#include "code/global/eglobal.h"
#include "code/stream/estream.h"
#include "code/stream/equeue.h"
#include "code/osstream/eosstream.h"
#include "code/buffer/ebuffer.h"
#include "code/connection/econnection.h"
#include "code/connection/eendpoint.h"
#include "code/helpers/etypeenum_helpers.h"
#include "code/helpers/eliststr_helpers.h"
#include "code/helpers/eobjflags_helpers.h"
#include "code/string/eint2str.h"
#include "code/main/emain.h"

#endif
