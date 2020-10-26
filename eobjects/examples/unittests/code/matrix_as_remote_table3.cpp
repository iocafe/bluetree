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
#define MY_CLASS_ID_1 (ECLASSID_APP_BASE + 1)
#define MY_CLASS_ID_2 (ECLASSID_APP_BASE + 2)
#define MY_CLASS_ID_3 (ECLASSID_APP_BASE + 3)

const os_char *table_name = OS_NULL; /* Not needed for eMatrix */


/**
****************************************************************************************************
  Thread which exposes a matrix as table.
****************************************************************************************************
*/
class ThreadExposingTheTable : public eThread
{
public:
    /* Get class identifier.
     */
    virtual os_int classid() {return MY_CLASS_ID_1;}

    virtual void initialize(
        eContainer *params = OS_NULL)
    {
        osal_console_write("ThreadExposingTheTable started\n");

        m_mtx = new eMatrix(this);
        m_mtx->addname("//mymtx");
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

            // m_mtx->print_json();
            return;
        }

        /* Default thread message processing.
         */
        eThread::onmessage(envelope);
    }

protected:

    eMatrix *m_mtx;
};



/**
****************************************************************************************************
  Thread which used to matrix remotely trough messages.
****************************************************************************************************
*/
class ThreadUsingTheTable: public eThread
{
public:
    /* Get class identifier.
     */
    virtual os_int classid() {return MY_CLASS_ID_2;}

    virtual void initialize(
        eContainer *params = OS_NULL)
    {
        osal_console_write("ThreadUsingTheTable started\n");
        configure_columns();
        m_step = 0;
        timer(1000);
    }

    virtual void onmessage(
        eEnvelope *envelope)
    {
        /* If at final destination for the message.
         */
        if (*envelope->target()=='\0' && envelope->command() == ECMD_TIMER)
        {
            osal_console_write("TIMER\n");
            one_step_at_a_time();
            return;
        }

        /* Default thread message processing.
         */
        eThread::onmessage(envelope);
    }

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

        /* ETABLE_ADOPT_ARGUMENT -> configuration will be released from memory.
         */
        etable_configure(this, "//mymtx", configuration, ETABLE_ADOPT_ARGUMENT);
    }

    void one_step_at_a_time()
    {
        switch (++m_step) {
            case 1: insert_row(3, "Mechanical Tiger"); break;
            case 2: insert_row(4, "Jack the Bouncer"); break;
            case 3: insert_row(16, "Silly Creeper"); break;
            case 4: insert_row(14, "Astounding Apple"); break;
            case 5: remove_row(4); break;
            case 6: update_row("No more creeper");
            case 7: insert_row(1, "Duudleli"); break;
            case 8: insert_row(2, "Puudleli"); break;
        }
    }

    void insert_row(
        os_int rownr,
        const os_char *text)
    {
        eContainer row;
        eVariable *element;

        element = new eVariable(&row);
        element->addname("ix", ENAME_NO_MAP);
        element->setl(rownr);

        element = new eVariable(&row);
        element->addname("connected", ENAME_NO_MAP);
        element->setl(OS_TRUE);

        element = new eVariable(&row);
        element->addname("connectto", ENAME_NO_MAP);
        element->sets(text);

        etable_insert(this, "//mymtx", table_name, &row);
    }

    void remove_row(
        os_int rownr)
    {
        eVariable where;

        where = "[";
        where += rownr;
        where += "]";
        etable_remove(this, "//mymtx", table_name, where.gets());
    }

    void update_row(
        const os_char *text)
    {
        eContainer row;
        eVariable *element;

        element = new eVariable(&row);
        element->addname("connectto", ENAME_NO_MAP);
        element->sets(text);

        element = new eVariable(&row);
        element->addname("ix", ENAME_NO_MAP);
        element->setl(12);

        etable_update(this, "//mymtx", table_name, "connectto='Silly Creeper'", &row);
    }

protected:
    os_int m_step;
};


/**
****************************************************************************************************
  Thread which selects data from table and monitors changes.
****************************************************************************************************
*/
class ThreadMonitoringTheTable: public eThread
{
public:
    /* Get class identifier.
     */
    virtual os_int classid() {return MY_CLASS_ID_3;}

    virtual void initialize(
        eContainer *params = OS_NULL)
    {
        eContainer columns;
        eVariable *column;
        osal_console_write("ThreadMonitoringTheTable started\n");

        m_rowset = new eRowSet(this);
        m_rowset->set_dbm("//mymtx");
        m_rowset->set_callback(ThreadMonitoringTheTable::static_callback, this);

        column = new eVariable(&columns);
        column->addname("*", ENAME_NO_MAP);

//        m_rowset->select("*", &columns);
        // m_rowset->print_json();
        timer(3000);
    }

    virtual void onmessage(
        eEnvelope *envelope)
    {
        /* If at final destination for the message.
         */
        if (*envelope->target()=='\0' && envelope->command() == ECMD_TIMER)
        {
            osal_console_write("TIMER\n");

eContainer columns;
eVariable *column;
column = new eVariable(&columns);
column->addname("*", ENAME_NO_MAP);
m_rowset->select("*", &columns);
// m_rowset->print_json();

            return;
        }

        /* Default thread message processing.
         */
        eThread::onmessage(envelope);
    }


    void callback(
        eRowSet *rset,
        ersetCallbackInfo *ci)
    {
        switch (ci->event) {
            case ERSET_TABLE_BINDING_COMPLETE:
                osal_console_write("binding done");
                break;

            case ERSET_INITIAL_DATA_RECEIVED:
            case ERSET_INSERT:
            case ERSET_UPDATE:
            case ERSET_REMOVE:
                rset->print_json(EOBJ_SERIALIZE_ONLY_CONTENT);
                break;
        }

        osal_console_write("eRowSet callback\n");
    }

    static void static_callback(
        eRowSet *rset,
        ersetCallbackInfo *ci,
        eObject *context)
    {
        if (rset) {
            ((ThreadMonitoringTheTable*)context)->callback(rset, ci);
        }
    }

protected:
    eRowSet *m_rowset;
};


/**
****************************************************************************************************
  Thread example 1.
****************************************************************************************************
*/
void matrix_as_remote_table_3()
{
    eThread *t;
    eThreadHandle thandle1, thandle2, thandle3;
    eContainer root;
    eVariable *txt;

    /* Create and start threads
     */
    t = new ThreadExposingTheTable();
    t->addname("//mythread1");
    t->start(&thandle1);
    t = new ThreadUsingTheTable();
    t->start(&thandle2);
    t = new ThreadMonitoringTheTable();
    t->start(&thandle3); /* After this t pointer is useless */

    for (os_int i = 0; i<1000; i++)
    {
        osal_console_write("master running\n");
        os_sleep(2000);

        txt = new eVariable(&root);
        txt->sets("Do really print it");
        root.message (MY_COMMAND, "//mythread1", OS_NULL, txt, EMSG_DEL_CONTENT|EMSG_NO_REPLIES);
    }

    /* Wait for thread to terminate
     */
    thandle3.terminate();
    thandle3.join();
    thandle2.terminate();
    thandle2.join();
    thandle1.terminate();
    thandle1.join();
}


