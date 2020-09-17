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
    eLineEdit *e;

    new eiocCheckbox(this);
    e = new eLineEdit(this);
    e->setpropertys(ECOMP_VALUE, "Jakke ja suutari");
    e->setpropertys(ECOMP_TEXT, "Editti");

    e = new eLineEdit(this);
    e->setpropertys(ECOMP_VALUE, "Tijana maalaa");
    e->setpropertys(ECOMP_TEXT, "Mut on kaksi");

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
