/**

  @file    connection1.cpp
  @brief   Example code for connecting two processes.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    10.10.2020

  This example demonstrates connecting two processes.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "eobjects.h"
#include "connection.h"
#include <stdio.h>

/* Every class needs to have unique class identifier (classid). Class identifier is is 32 bit
   integer. Class identifiers starting from ECLASSID_APP_BASE are reserved for the application.
 */
#define MY_CLASS_ID_2 (ECLASSID_APP_BASE + 2)

/* Enumeration of c1MyClass properties.
 */
#define EMYCLASS2P_X 10
#define EMYCLASS2P_Y 20

static const os_char emyclass2p_x[] = "X";
static const os_char emyclass2p_y[] = "Y";


/**
****************************************************************************************************
  Example thread class.
****************************************************************************************************
*/
class c1MyClass : public eThread
{
public:
    /* Constructor. Initialize properties to default values.
     */
    c1MyClass(
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
        const os_int cls = MY_CLASS_ID_2;

        os_lock();
        addproperty(cls, EMYCLASS2P_X, emyclass2p_x, "X", EPRO_PERSISTENT);
        addproperty(cls, EMYCLASS2P_Y, emyclass2p_y, "Y", EPRO_PERSISTENT);
        os_unlock();
    }

    /* Get class identifier.
     */
    virtual os_int classid()
    {
        return MY_CLASS_ID_2;
    }

    virtual void initialize(
        eContainer *params = OS_NULL)
    {
        bind(EMYCLASS2P_X, "//myconnection/myclass1/_p/A", EBIND_DEFAULT|EBIND_ATTR);
// setpropertyd(EMYCLASS2P_Y, 3.3);
        bind(EMYCLASS2P_Y, "//myconnection/myclass1/_p/B", EBIND_CLIENTINIT);
// setpropertyd(EMYCLASS2P_Y, 4.3);
    }

    /* Process incoming messages.
     */
    virtual void onmessage(
        eEnvelope *envelope)
    {
        eVariable v;

        /* If this is message to me, not to my children?
         */
        if (*envelope->target() == '\0')
        {
            /* If timer message, then increment A?
             */
            if (envelope->command() == ECMD_TIMER)
            {
//                setpropertyl(EMYCLASS2P_X, -2 * propertyl(EMYCLASS2P_X));
//                setpropertyl(EMYCLASS2P_Y, propertyl(EMYCLASS2P_Y)+1);
                propertyv(EMYCLASS2P_X, &v);
printf ("ULLE \'%s\'\n", v.gets());
                setpropertyv(EMYCLASS2P_Y, &v);

/*    setpropertys_msg("//myconnection/myclass1",
         "message from connection_example1", "A"); */

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
            case EMYCLASS2P_X:
                printf ("GOT X \'%s\'\n", x->gets());
                break;

            case EMYCLASS2P_Y:
                printf ("GOT Y \'%s\'\n", x->gets());
                break;

            default:
                return ESTATUS_FAILED;
        }

        return ESTATUS_SUCCESS;
    }
};


/**
****************************************************************************************************
  Connection example 1.
****************************************************************************************************
*/
void connection_example_1()
{
    eThread *t;
    eThreadHandle thandle1, thandle2, conthreadhandle;
    eContainer c;

    /* Set up clas for use.
     */
    c1MyClass::setupclass();

    /* Create and start thread to listen for incoming socket connections,
       name it "//myconnection".
     */
    t = new eConnection();
    t->addname("//myconnection");
    t->start(&conthreadhandle); /* After this t pointer is useless */
    c.setpropertys_msg(conthreadhandle.uniquename(), // "//myconnection",
         "socket:localhost", econnp_ipaddr);

//    c.setpropertys_msg("//myconnection/myclass1",
//         "message from connection_example1", "A");

//    os_sleep(2000);

    /* Create and start thread named "thread2".
     */
    t = new c1MyClass();
    t->addname("thread2", ENAME_PROCESS_NS);
//  t->timer(120);
    t->start(&thandle2); /* After this t pointer is useless */

//    c.setpropertyd_msg("//thread2/_p/Y", 11.5);

    os_sleep(1500000);

    /* Wait for the threads to terminate.
     */
    thandle2.terminate();
    thandle2.join();
    conthreadhandle.terminate();
    conthreadhandle.join();
}
