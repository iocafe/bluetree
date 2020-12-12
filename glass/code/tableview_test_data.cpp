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
#include "glass.h"
#include "tableview_test_data.h"

/* Every class needs to have unique class identifier (classid). Class identifier is is 32 bit
   integer. Class identifiers starting from ECLASSID_APP_BASE are reserved for the application.
 */
#define MY_CLASS_ID_1 (ECLASSID_APP_BASE + 1)
#define MY_CLASS_ID_2 (ECLASSID_APP_BASE + 2)

const os_char *table_name = OS_NULL; /* Not needed for eMatrix */

static eThreadHandle *thandle1, *thandle2;

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
        eVariable *v;

        osal_console_write("ThreadExposingTheTable started\n");

        m_mtx = new eMatrix(this);
        m_mtx->addname("//mymtx");

        m_pers = new ePersistent(this);
        m_pers->addname("//mypersistent");
        m_pers->ns_create();
        v = new eVariable(m_pers);
        v->addname("../abba");
        v->setpropertys(EVARP_TEXT, "ABBA");
        v = new eVariable(m_pers);
        v->addname("../bansku");
        v->setpropertys(EVARP_TEXT, "BANSKU");
        v->setpropertys(EVARP_UNIT, "kN");
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
    ePersistent *m_pers;
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
        timer(5);
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

        configuration = new eContainer(this);
        columns = new eContainer(configuration, EOID_TABLE_COLUMNS);
        columns->addname("columns", ENAME_NO_MAP);

        /* For matrix as a table row number is always the first column in configuration.
         */
        column = new eVariable(columns);
        column->addname("ix", ENAME_NO_MAP);
        column->setpropertys(EVARP_TEXT, "rivi");
        column->setpropertyi(EVARP_TYPE, OS_INT);


/* char buf[128];
buf[0] = 'C';
for (int i = 0; i<150; i++) {
    osal_int_to_str(buf+1, sizeof(buf)-1, i);
    column = new eVariable(columns);
    column->addname(buf, ENAME_NO_MAP);
} */

        column = new eVariable(columns);
        column->addname("connected", ENAME_NO_MAP);
        column->setpropertyi(EVARP_TYPE, OS_BOOLEAN);

        column = new eVariable(columns);
        column->addname("connectto", ENAME_NO_MAP);
        column->setpropertys(EVARP_UNIT, "ms");
        column->setpropertyi(EVARP_TYPE, OS_STR);
        column->setpropertys(EVARP_ATTR, "align=cright");

        column = new eVariable(columns);
        column->addname("tstamp", ENAME_NO_MAP);
        column->setpropertys(EVARP_TEXT, "aika-\nleiska");
        column->setpropertys(EVARP_ATTR, "tstamp=\"yy,sec\"");
        column->setpropertys(EVARP_TTIP, "Daa dillan dillan daa");


        column = new eVariable(columns);
        column->addname("selectit", ENAME_NO_MAP);
        column->setpropertyi(EVARP_TYPE, OS_CHAR);
        column->setpropertys(EVARP_ATTR, "enum=\"1.eka,2.toka,3.koka\"");

        column = new eVariable(columns);
        column->addname("x", ENAME_NO_MAP);

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
            case 9: insert_row(6, "KEPPO"); break;
            case 11: insert_row(7, "kappo"); break;
            case 14: insert_row(8, "sussu"); break;
            case 17: insert_row(9, "deeku"); break;
            default: if (m_step < 1000) insert_row(m_step+100, "deeku"); break;
        }

        // if (m_step > 16) m_step = 16;
    }

    void insert_row(
        os_int rownr,
        const os_char *text)
    {
        eContainer row;
        eVariable *element;
        eValueX x;

        element = new eVariable(&row);
        element->addname("ix", ENAME_NO_MAP);
        element->setl(rownr);

        element = new eVariable(&row);
        element->addname("connected", ENAME_NO_MAP);
        element->setl(osal_rand(0, 1));

        element = new eVariable(&row);
        element->addname("connectto", ENAME_NO_MAP);
        element->sets(text);

        element = new eVariable(&row);
        element->addname("tstamp", ENAME_NO_MAP);
        element->setl(etime());

        element = new eVariable(&row);
        element->addname("selectit", ENAME_NO_MAP);
        element->setl(osal_rand(0, 3));

        element = new eVariable(&row);
        element->addname("x", ENAME_NO_MAP);
        x = 1234.5;
        x.set_sbits(OSAL_STATE_ORANGE|OSAL_STATE_CONNECTED);
        x.set_tstamp(etime());
        element->setpropertyo(EVARP_VALUE, &x);

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

