/**

  @file    eprotocolhandle.h
  @brief   Abstract communication protocol handle.
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

    /* Virtual destructor
     */
    virtual ~eProtocolHandle() {}

    /* Get class identifier.
     */
    virtual os_int classid() {return ECLASSID_PROTOCOL_HANDLE;}

    /* Static function to add class to propertysets and class list.
     */
    static void setupclass();

    /* Called when property value changes.
     */
    virtual eStatus onpropertychange(
        os_int propertynr,
        eVariable *x,
        os_int flags);

    /* Get value of simple property (override).
     */
    virtual eStatus simpleproperty(
        os_int propertynr,
        eVariable *x);

    /**
    ************************************************************************************************
      Protocol handle functions.
    ************************************************************************************************
    */

    /* Check if connection or end point is running.
     */
    virtual os_boolean started() = 0;

    /**
    ************************************************************************************************
      Member variables
    ************************************************************************************************
    */

    /** Communication channel open flag.
     */
    os_boolean m_is_open;
};


#endif
