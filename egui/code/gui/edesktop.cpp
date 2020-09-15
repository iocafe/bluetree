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
    new eiocCheckbox(this);
    new eLineEdit(this);
    new eLineEdit(this);
}
