/**

  @file    matrix_as_remote_table.cpp
  @brief   Unit test, access to eTable (eMatrix here) over messages.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    15.10.2020

  Matrix is used trough
  - Application sens insert, remove, update and select command to eDBM object.
  - When eDBM receives these, it calls eMatrix function (table API) to modify or get data from
    the table.
  - A eRowSet is needed to select data. It sends select message to eDBM and receives table
    data back. The eDBM memorizes  the select as long as the eRowSet exists, so it can keep
    the row informed about changes to table data which impact this selection.

  NOTES:
  - Update and remove callbacks with access to full row to inform selections ?

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
class ThreadExposingTableData : public eThread
{
public:
    /* Get class identifier.
     */
    virtual os_int classid() {return MY_CLASS_ID;}

    virtual void initialize(
        eContainer *params = OS_NULL)
    {
        osal_console_write("initializing worker\n");

        m_mtx = new eMatrix(this);
        configure_columns();
    }

    virtual void finish()
    {
        delete m_mtx;
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
        /* If at final destination for the message.
         */
        if (*envelope->target()=='\0' && envelope->command() == MY_COMMAND)
        {
            osal_console_write(envelope->source());
            osal_console_write("\n");
            return;
        }

        /* Default thread message processing.
         */
        eThread::onmessage(envelope);
    }

protected:

    void configure_columns()
    {
        eContainer *configuration, *columns;
        eVariable *column;

        configuration = new eContainer();
        columns = new eContainer(configuration, EOID_TABLE_COLUMNS);
        columns->addname("columns", ENAME_NO_MAP);

        /* For matrix as a table row number is always the first column in configuration.
         */
        column = new eVariable(columns);
        column->addname("ix", ENAME_NO_MAP);
        column->setpropertys(EVARP_TEXT, "rivi");

        column = new eVariable(columns);
        column->addname("connected", ENAME_NO_MAP);
        column->setpropertyi(EVARP_TYPE, OS_STR);

        column = new eVariable(columns);
        column->addname("connectto", ENAME_NO_MAP);

        m_mtx->configure(configuration);
    }


    eMatrix *m_mtx;
};


/**
****************************************************************************************************
  Thread example 1.
****************************************************************************************************
*/
void matrix_as_remote_table_3()
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
    t = new ThreadExposingTableData();
    t->addname("worker", ENAME_PROCESS_NS);
    t->start(&thandle); /* After this t pointer is useless */

    for (os_int i = 0; i<1000; i++)
    {
        osal_console_write("master running\n");
        os_sleep(20);

        txt = new eVariable(&root);
        txt->sets("message content");
        root.message (MY_COMMAND, "//worker", OS_NULL, txt, EMSG_DEL_CONTENT|EMSG_NO_REPLIES);
    }

    /* Wait for thread to terminate
     */
    thandle.terminate();
    thandle.join();
}
