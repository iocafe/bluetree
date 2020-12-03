/**

  @file    edesktop.cpp
  @brief   Desktop application specific setup and functions.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "egui.h"


/**
****************************************************************************************************

  @brief Setup egui for desktop application

  The setup_desktop_application function creates GUI components for desktop application.

  @return None.

****************************************************************************************************
*/
void eGui::setup_desktop_application()
{
    eWindow *w;
    eParameterList *p;
    eLineEdit *e;
    eTreeNode *n;
    eTableView *t;
    eValueX x;

    w = new eWindow(this);
    w->setpropertys(ECOMP_VALUE, "Muuttujia");

    e = new eLineEdit(w);
    e->setpropertys(ECOMP_TEXT, "Editti");
    e->setpropertys(ECOMP_ATTR, "tstamp=\"yy,usec\"");
    e->setpropertyl(ECOMP_VALUE, etime());
    e->setpropertys(ECOMP_TTIP, "Minun helppo helppi");

    e = new eLineEdit(w);
    e->setpropertys(ECOMP_VALUE, "Tijana maalaa");
    e->setpropertys(ECOMP_TEXT, "Mut on kaksi");
    x = 1234.5;
    x.set_sbits(OSAL_STATE_ORANGE|OSAL_STATE_CONNECTED);
    x.set_tstamp(etime());
    e->setpropertyo(ECOMP_VALUE, &x);

    w = new eWindow(this);
    w->setpropertys(ECOMP_VALUE, "parameter list");
    p = new eParameterList(w);

    e = new eLineEdit(p);
    e->setpropertys(ECOMP_TEXT, "Editti");
    e->setpropertys(ECOMP_ATTR, "tstamp=\"yy,usec\"");
    e->setpropertyl(ECOMP_VALUE, etime());
    e->setpropertys(ECOMP_TTIP, "Minun helppo helppi");

    e = new eLineEdit(p);
    e->setpropertys(ECOMP_VALUE, "Tijana maalaa");
    e->setpropertys(ECOMP_TEXT, "Mut on kaksi");
    e->setpropertys(ECOMP_UNIT, "%");
    x = 1234.5;
    x.set_sbits(OSAL_STATE_ORANGE|OSAL_STATE_CONNECTED);
    x.set_tstamp(etime());
    e->setpropertyo(ECOMP_VALUE, &x);
    // e->setpropertyo(ECOMP_TYPE, OS_DOUBLE);

    e = new eLineEdit(p);
    e->setpropertyi(ECOMP_VALUE, OS_FALSE);
    e->setpropertys(ECOMP_TEXT, "ENNUUPPI");
    e->setpropertys(ECOMP_ATTR, "enum=\"1.eka,2.toka\"");

    e = new eLineEdit(p);
    e->setpropertyi(ECOMP_VALUE, OS_TRUE);
    e->setpropertys(ECOMP_TEXT, "BOxI2");
    e->setpropertyi(ECOMP_TYPE, OS_BOOLEAN);

    e = new eLineEdit(p);
    e->setpropertyi(ECOMP_VALUE, OS_TRUE);
    e->setpropertys(ECOMP_TEXT, "floppia");
    e->setpropertyi(ECOMP_TYPE, OS_DOUBLE);
    e->setpropertyi(ECOMP_DIGS, 2);

    e = new eLineEdit(p);
    e->setpropertyd(ECOMP_VALUE, 11.11);
    e->setpropertys(ECOMP_TEXT, "duudi daa");
    e->setpropertyi(ECOMP_TYPE, OS_DOUBLE);
    e->setpropertyi(ECOMP_DIGS, 5);

    e = new eLineEdit(p);
    e->setpropertyd(ECOMP_VALUE, 11.11);
    e->setpropertys(ECOMP_TEXT, "kasse ja ma");
    e->setpropertyi(ECOMP_TYPE, OS_DOUBLE);
    e->setpropertyi(ECOMP_DIGS, 5);

    e = new eLineEdit(p);
    e->setpropertyd(ECOMP_VALUE, 11.12);
    e->setpropertys(ECOMP_TEXT, "kasse ja ma");
    e->setpropertyi(ECOMP_TYPE, OS_DOUBLE);
    e->setpropertyi(ECOMP_DIGS, 5);

    w = new eWindow(this);
    w->setpropertys(ECOMP_VALUE, "test table");

    t = new eTableView(w);
    t->setpropertys(ECOMP_PATH, "//mymtx");

    w = new eWindow(this);
    w->setpropertys(ECOMP_VALUE, "kayttajat");

    t = new eTableView(w);
    t->setpropertys(ECOMP_PATH, "//service/accounts");

    w = new eWindow(this);
    w->setpropertys(ECOMP_VALUE, "Browseri");

    n = new eTreeNode(w);
    n->setpropertys(ECOMP_PATH, "//");
    n->setpropertys(ECOMP_IPATH, "//");

    w = new eWindow(this);
    w->setpropertys(ECOMP_VALUE, "Browseri 2");
    p = new eParameterList(w);

    n = new eTreeNode(p);
    n->setpropertys(ECOMP_PATH, "//");
    n->setpropertys(ECOMP_IPATH, "//");


    /* e = new eLineEdit(w);
    e->setpropertys(ECOMP_VALUE, "Jakke ja suutari");
    e->setpropertys(ECOMP_TEXT, "Editti");
    e->setpropertys(ECOMP_UNIT, "%");

    e = new eLineEdit(w);
    e->setpropertys(ECOMP_VALUE, "Tijana maalaa");
    e->setpropertys(ECOMP_TEXT, "Mut on kaksi");
    */


    setpropertys(EGUIP_TEXT, eglobal->process_name);
}
