/**

  @file    thread1.cpp
  @brief   Example code about naming objects.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

  This example demonstrates how to create a thread and sen messages to it.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "eobjects.h"
#include "threads.h"

/* Purpose of a message is specified by 32 bit command. Negative command identifiers are
   reserved for the eobject library related, but positive ones can be used freely.
 */
#define MY_COMMAND 10

/* Every class needs to have unique class identifier (classid). Class identifier is is 32 bit
   integer. Class identifiers starting from ECLASSID_APP_BASE are reserved for the application.
 */
#define MY_CLASS_ID (ECLASSID_APP_BASE + 1)

/**
****************************************************************************************************
  Example thread class.
****************************************************************************************************
*/
class eMyThread : public eThread
{
    /* Get class identifier.
     */
    virtual os_int classid() {return MY_CLASS_ID;}

    virtual void initialize(
        eContainer *params = OS_NULL)
    {
        osal_console_write("initializing worker\n");
    }

    virtual void run()
    {
        while (!exitnow())
        {
            alive();
            osal_console_write("worker running\n");
        }
    }

    virtual void onmessage(
        eEnvelope *envelope)
    {
        eVariable *content;

        /* If at final destination for the message.
         */
        if (*envelope->target()=='\0' && envelope->command() == MY_COMMAND)
        {
            content = eVariable::cast(envelope->content());
            osal_console_write(content->gets());
            osal_console_write(" ");
            osal_console_write(envelope->source());
            osal_console_write("\n");
            return;
        }

        /* Default thread message processing.
         */
        eThread::onmessage(envelope);
    }
};


/**
****************************************************************************************************
  Thread example 1 main.
****************************************************************************************************
*/
void thread_example_1()
{
    eContainer
        root;

    eVariable
        *txt;

    eThread
        *t;

    eThreadHandle
        thandle;

    /* Create and start thread named "worker".
     */
    t = new eMyThread();
    t->addname("worker", ENAME_PROCESS_NS);
    t->start(&thandle); /* After this t pointer is useless */

    for (os_int i = 0; i<1000; i++)
    {
        osal_console_write("master running\n");
        os_sleep(2000);

        txt = new eVariable(&root);
        txt->sets("Swimming Dog");
        root.message (MY_COMMAND, "//worker", OS_NULL, txt, EMSG_DEL_CONTENT);
    }

    /* Wait for thread to terminate
     */
    thandle.terminate();
    thandle.join();
}
