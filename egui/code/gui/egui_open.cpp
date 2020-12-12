/**

  @file    egui_open.cpp
  @brief   Open object as new window.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  The oqui_

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "egui.h"


/**
****************************************************************************************************

  @brief Send message to object to request open content

  @param  path Path to the object to "open".
  @param  context Context to be passed to open_content() function.

****************************************************************************************************
*/
void eGui::open_request(
    const os_char *path,
    eObject *context)
{
    message(ECMD_OPEN_REQUEST, path,
        OS_NULL, OS_NULL, EMSG_KEEP_CONTEXT, context);
}


/**
****************************************************************************************************

  @brief Create a window to display object content.

  The eGui has received content to open as message from object to "open" in GUI. The function
  creates a window to display the content.


****************************************************************************************************
*/
void eGui::open_content(
    const os_char *path,
    eObject *content,
    eObject *context)
{
    eWindow *w;
    eTableView *t;

    w = new eWindow(this);
    w->setpropertys(ECOMP_TEXT, "test table");

    t = new eTableView(w);
    t->setpropertys(ECOMP_PATH, path);

}


