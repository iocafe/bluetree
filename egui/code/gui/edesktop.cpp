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
    eLineEdit *e;
    eTreeNode *n;
    eValueX x;

    w = new eWindow(this);
    w->setpropertys(ECOMP_VALUE, "Muuttujia");

    e = new eLineEdit(w);
    e->setpropertys(ECOMP_TEXT, "Editti");
    e->setpropertys(ECOMP_ATTR, "tstamp=\"yy,usec\"");
    e->setpropertyl(ECOMP_VALUE, etime());

    e = new eLineEdit(w);
    e->setpropertys(ECOMP_VALUE, "Tijana maalaa");
    e->setpropertys(ECOMP_TEXT, "Mut on kaksi");
    x = 1234.5;
    x.set_sbits(OSAL_STATE_YELLOW);
    x.set_tstamp(etime());
    e->setpropertyo(ECOMP_VALUE, &x);

    e = new eLineEdit(w);
    e->setpropertyi(ECOMP_VALUE, OS_FALSE);
    e->setpropertys(ECOMP_TEXT, "ENNUUPPI");
    e->setpropertys(ECOMP_ATTR, "enum=\"1.eka,2.toka\"");

    e = new eLineEdit(w);
    e->setpropertyi(ECOMP_VALUE, OS_TRUE);
    e->setpropertys(ECOMP_TEXT, "BOxI2");
    e->setpropertyi(ECOMP_TYPE, OS_BOOLEAN);

    e = new eLineEdit(w);
    e->setpropertyi(ECOMP_VALUE, OS_TRUE);
    e->setpropertys(ECOMP_TEXT, "floppia");
    e->setpropertyi(ECOMP_TYPE, OS_DOUBLE);
    e->setpropertyi(ECOMP_DIGS, 2);

    w = new eWindow(this);
    w->setpropertys(ECOMP_VALUE, "Browseri");

    n = new eTreeNode(w);
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

/*     ECOMP_TYPE

    #define ECOMP_STATE_BITS EVARP_STATE_BITS
#define ECOMP_TIMESTAMP EVARP_TIMESTAMP
#define ECOMP_DIGS EVARP_DIGS
#define ECOMP_TEXT EVARP_TEXT
#define ECOMP_UNIT EVARP_UNIT
#define ECOMP_MIN EVARP_MIN
#define ECOMP_MAX EVARP_MAX
#define ECOMP_TYPE EVARP_TYPE
#define ECOMP_ATTR EVARP_ATTR
#define ECOMP_DEFAULT EVARP_DEFAULT
#define ECOMP_GAIN EVARP_GAIN
#define ECOMP_OFFSET EVARP_OFFSET
#define ECOMP_CONF EVARP_CONF
#define ECOMP_PATH 30
*/

}
