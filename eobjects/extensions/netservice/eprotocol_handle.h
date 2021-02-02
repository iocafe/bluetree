/**

  @file    eprotocolhandle.h
  @brief   Abstract communication protocol handle.
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
#ifndef EPROTOCOL_HANDLE_H_
#define EPROTOCOL_HANDLE_H_
#include "extensions/netservice/enetservice.h"

/**
****************************************************************************************************
  Defines
****************************************************************************************************
*/

/* Property numbers.
 */
#define EPROHANDP_ISOPEN 10

/* Property names.
 */
extern const os_char
    eprohandp_isopen[];


/**
****************************************************************************************************
  eProtocolHandle class.
****************************************************************************************************
*/
class eProtocolHandle : public eObject
{
public:
    /* Constructor.
     */
    eProtocolHandle(
        eObject *parent = OS_NULL,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT);

    /* Virtual destructor.
     */
//     virtual ~eProtocolHandle();

    /**
    ************************************************************************************************
      Protocol handle functions.
    ************************************************************************************************
    */

    /* Start a connection or end point thread.
     */
    virtual void start_thread(
        eThread *t,
        const os_char *threadname) = 0;

    /* Terminate connection or end point thread.
     */
    virtual void terminate_thread() = 0;

    /* Get unique name of a connection or end point.
     */
    virtual const os_char *uniquename() = 0;

    /* Check if connection or end point is running.
     */
    virtual os_boolean isrunning() = 0;
};


#endif
