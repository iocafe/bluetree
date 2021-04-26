/**

  @file    eprocess.h
  @brief   eProcess class implementation.
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
#ifndef EPROCESS_H_
#define EPROCESS_H_
#include "eobjects.h"

/**
****************************************************************************************************
  eProcess is special thread which can be used to share data within process (mutex lock required)
****************************************************************************************************
*/
class eProcess : public eThread
{
public:

    /**
    ************************************************************************************************
      Generic eObject functionality.
    ************************************************************************************************
    */
    /* Constructor.
     */
    eProcess(
        eObject *parent = OS_NULL,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT);

    /* Virtual destructor.
     */
    virtual ~eProcess();

    /* Casting eObject pointer to eProcess pointer.
     */
    inline static eProcess *cast(
        eObject *o)
    {
        e_assert_type(o, ECLASSID_PROCESS)
        return (eProcess*)o;
    }

    /* Get class identifier.
     */
    virtual os_int classid() {return ECLASSID_PROCESS;}

    /* Static function to add class to propertysets and class list.
     */
    static void setupclass();

    virtual void onmessage(
        eEnvelope *envelope);


    /**
    ************************************************************************************************
      eThread functionality
    ************************************************************************************************
    */
    virtual void initialize(
        eContainer *params = OS_NULL);

    virtual void run();


    /**
    ************************************************************************************************
      eProcess specific functions
    ************************************************************************************************
    */
    /* Get pointer to eSyncConnector objects for synchronized data transfers.
     */
    static eContainer *sync_connectors();


protected:
    /**
    ************************************************************************************************
      Member variables
    ************************************************************************************************
    */
    eContainer *m_sync_connectors;
};


/* Create eProcess object and start a thread to run it.
 */
void eprocess_create();

/* Terminate eProcess thread and clean up.
 */
void eprocess_close();


#endif
