/**

  @file    enetservice.h
  @brief   enet service implementation.
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
#ifndef ENETSERVICE_H_
#define ENETSERVICE_H_
#include "eobjects.h"

/**
****************************************************************************************************
  eNetService class.
****************************************************************************************************
*/
class eNetService : public eThread
{
public:
    /* Constructor.
     */
    eNetService(
        eObject *parent = OS_NULL,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT);

    /* Virtual destructor.
     */
    virtual ~eNetService();

    /* Clone object.
     */
    virtual eObject *clone(
        eObject *parent,
        e_oid id = EOID_CHILD,
        os_int aflags = 0);

    /* Casting eObject pointer to eNetService pointer.
     */
    inline static eNetService *cast(
        eObject *o)
    {
        e_assert_type(o, ECLASSID_NETSERVICE)
        return (eNetService*)o;
    }

    /* Get class identifier.
     */
    virtual os_int classid() {return ECLASSID_NETSERVICE; }

    /* Static function to add class to propertysets and class list.
     */
    static void setupclass();

    /* Static constructor function for generating instance by class list.
     */
    static eNetService *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new eNetService(parent, id, flags);
    }

    virtual void initialize(
        eContainer *params = OS_NULL);

protected:
    /**
    ************************************************************************************************
      Internal functions.
    ************************************************************************************************
    */

    /* Create "user accounts" table.
     */
    void create_user_accounts_table();


    /**
    ************************************************************************************************
      Member variables
    ************************************************************************************************
    */

    /** Persistent object to hold user accounts table.
     */
    ePersistent *m_persistent_accounts;

    /** User accounts table (matrix).
     */
    eMatrix *m_accounts_matrix;
};

/* Start network service.
 */
void enet_start_server(
    eThreadHandle *server_thread_handle);

#endif
