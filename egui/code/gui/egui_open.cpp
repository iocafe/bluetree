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
    eObject *content,
    eObject *context)
{
    message(ECMD_OPEN_REQUEST, path,
        OS_NULL, content, EMSG_KEEP_CONTEXT, context);
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
    OSAL_UNUSED(context);

    w = OS_NULL;
    open_content_helper(path, content, &w);
}

void eGui::open_content_helper(
    const os_char *path,
    eObject *content,
    eWindow **win)
{
    eWindow *w;
    eTableView *t;
    eParameterList *p = OS_NULL;
    eLineEdit *e;
    eVariable *v, mypath, tmp;
    eContainer *appendix;
    os_int cid;
    os_boolean is_variable, is_container, is_matrix;

    for (v = content->firstv(); v; v = v->nextv())
    {
        cid = v->oid();

        is_variable = eclasslist_isinstanceof(cid, ECLASSID_VARIABLE);
        is_container = OS_FALSE;
        if (!is_variable) {
            is_container = eclasslist_isinstanceof(cid, ECLASSID_CONTAINER);
            if (!is_container) {
                is_matrix = eclasslist_isinstanceof(cid, ECLASSID_MATRIX);
            }
        }

        if (!is_variable && !is_container && !is_matrix) {
            continue;
        }

        if (*win == OS_NULL)
        {
            w = new eWindow(this);
            content->propertyv(ECONTP_TEXT, &tmp);
            if (tmp.isempty()) {
                tmp.sets("unnamed");
            }
            w->setpropertyv(ECOMP_TEXT, &tmp);
            w->setpropertyv(ECOMP_NAME, &tmp);
            *win = w;
        }

        mypath.sets(path);
        if (!v->isempty()) {
            mypath.appends("/");
            mypath.appendv(v);
        }

        if (is_variable) {
            if (p == OS_NULL) {
                p = new eParameterList(*win);
            }
            e = new eLineEdit(p);
            e->bind(ECOMP_VALUE, mypath.gets(), EBIND_METADATA);
        }
        else if (is_container) {
            appendix = eContainer::cast(v->first(EOID_APPENDIX));
            if (appendix) {
                open_content_helper(mypath.gets(), appendix, win);
            }
        }
        else if (is_matrix) {
            t = new eTableView(w);
            t->setpropertys(ECOMP_PATH, mypath.gets());
        }
    }
}

