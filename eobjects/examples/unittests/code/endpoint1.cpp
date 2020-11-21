/**

  @file    endpoint1.cpp
  @brief   Connecting two processes, end point.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    10.10.2020

  Unit test: End point listening for incoming socket connections.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "eobjects.h"
#include "endpoint.h"
#include <stdio.h>

/* Every class needs to have unique class identifier (classid). Class identifier is is 32 bit
   integer. Class identifiers starting from ECLASSID_APP_BASE are reserved for the application.
 */
#define MY_CLASS_ID_1 (ECLASSID_APP_BASE + 1)

/* Enumeration of epMyClass properties. Normally these would be in header file.
 */
#define EMYCLASS1P_A 10
#define EMYCLASS1P_B 20

static const os_char emyclass1p_a[] = "A";
static const os_char emyclass1p_b[] = "B";


/**
****************************************************************************************************
  Example thread class.
****************************************************************************************************
*/
class epMyClass : public eThread
{
public:
    /* Constructor.
     */
    epMyClass(
        eObject *parent = OS_NULL,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
        : eThread(parent, id, flags)
    {
        initproperties();
    }

    /* Add eMyClass'es properties to class'es property set.
    */
    static void setupclass()
    {
        const os_int cls = MY_CLASS_ID_1;

        os_lock();
        eclasslist_add(cls, (eNewObjFunc)newobj, "epMyClass");
        addproperty(cls, EMYCLASS1P_A, emyclass1p_a, "A");
        addproperty(cls, EMYCLASS1P_B, emyclass1p_b, "B");
        os_unlock();
    }

    /* Static constructor function for generating instance by class list.
     */
    static epMyClass *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new epMyClass(parent, id, flags);
    }

    /* Get class identifier.
     */
    virtual os_int classid()
    {
        return MY_CLASS_ID_1;
    }

    /* Process incoming messages.
     */
    virtual void onmessage(
        eEnvelope *envelope)
    {
        /* If this is message to me, not to my children?
         */
        if (*envelope->target() == '\0')
        {
            /* If timer message, then increment A?
             */
            if (envelope->command() == ECMD_TIMER)
            {
                // setpropertyl(EMYCLASS1P_A, propertyl(EMYCLASS1P_A) + 1);
                return;
            }
        }

        /* Default message procesing.
         */
        eThread::onmessage(envelope);
    }

    /* This gets called when property value changes
     */
    virtual eStatus onpropertychange(
        os_int propertynr,
        eVariable *x,
        os_int flags)
    {
        switch (propertynr)
        {
            case EMYCLASS1P_A:
                printf ("1: GOT A \'%s\'\n", x->gets());
                break;

            case EMYCLASS1P_B:
                printf ("1: GOT B \'%s\'\n", x->gets());
                break;

            default:
                return ESTATUS_FAILED;
        }

        return ESTATUS_SUCCESS;
    }
};


/**
****************************************************************************************************
  End point example 1.
****************************************************************************************************
*/
void endpoint_example_1()
{
    eThread *t;
    eThreadHandle thandle1, endpointthreadhandle;
    eContainer c;

    /* Aet up class for use.
     */
    epMyClass::setupclass();

    /* Create and start class epMyClass as thread named "myclass1".
     */
    t = new epMyClass();
    t->addname("myclass1", ENAME_PROCESS_NS);
t->setpropertys(EMYCLASS1P_A, "Nasse");
    t->timer(4500);
    t->timer(20);
    t->start(&thandle1); /* After this t pointer is useless */

    /* Create and start end point thread to listen for incoming socket connections,
       name it "myendpoint".
     */
    t = new eEndPoint();
    t->addname("//myendpoint");
    t->start(&endpointthreadhandle); /* After this t pointer is useless */
    c.setpropertys_msg(endpointthreadhandle.uniquename(),
         "socket::" IOC_DEFAULT_SOCKET_PORT_STR, eendpp_ipaddr);

    os_sleep(15000000);

    /* Wait for the threads to terminate.
     */
    thandle1.terminate();
    thandle1.join();
    endpointthreadhandle.terminate();
    endpointthreadhandle.join();
}
