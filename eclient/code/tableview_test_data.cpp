/**

  @file    tableview_test_data.cpp
  @brief   Matrix as table for testing eTableView.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    2.11.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "eclient.h"
#include "tableview_test_data.h"

/* Every class needs to have unique class identifier (classid). Class identifier is is 32 bit
   integer. Class identifiers starting from ECLASSID_APP_BASE are reserved for the application.
 */
#define MY_CLASS_ID_1 (ECLASSID_APP_BASE + 1)
#define MY_CLASS_ID_2 (ECLASSID_APP_BASE + 2)

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
        }
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
        column->addname("tstamp", ENAME_NO_MAP);

/* char buf[128];
buf[0] = 'C';
for (int i = 0; i<150; i++) {
    osal_int_to_str(buf+1, sizeof(buf)-1, i);
    column = new eVariable(columns);
    column->addname(buf, ENAME_NO_MAP);
} */

        column = new eVariable(columns);
        column->addname("connected", ENAME_NO_MAP);
        column->setpropertyi(ECOMP_TYPE, OS_BOOLEAN);

        column = new eVariable(columns);
        column->addname("connectto", ENAME_NO_MAP);
        column->setpropertys(ECOMP_UNIT, "ms");
        column->setpropertyi(EVARP_TYPE, OS_STR);


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


static eThreadHandle *thandle1, *thandle2;


void tableview_test_start()
{
    eThread *t;

    thandle1 = new eThreadHandle;
    thandle2 = new eThreadHandle;

    /* Create and start threads
     */
    t = new ThreadExposingTheTable();
    t->addname("//mythread1");
    t->start(thandle1);
    t = new ThreadUsingTheTable();
    t->start(thandle2);
}


void tableview_test_end()
{
    /* Terminate threads and wait for them to finish.
     */
    thandle2->terminate();
    thandle2->join();
    thandle1->terminate();
    thandle1->join();
}

