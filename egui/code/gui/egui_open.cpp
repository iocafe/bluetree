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
    eVariable *v;
    os_int open_as = EBROWSE_OPEN;
    OSAL_UNUSED(context);

    v = content->firstv(EOID_PARAMETER);
    if (v) open_as = v->geti();

    switch (open_as)
    {
        default:
        case EBROWSE_OPEN:
            w = OS_NULL;
            open_parameter_tree(path, content, &w, OS_NULL);
            break;

        case EBROWSE_GRAPH:
            break;

        case EBROWSE_CAMERA:
            open_camera_view(path, content);
            break;
    }

}

void eGui::open_parameter_tree(
    const os_char *path,
    eObject *content,
    eWindow **win,
    eComponent *p)
{
    eWindow *w;
    eTableView *t;
    eLineEdit *e;
    eVariable *v, mypath, tmp;
    eContainer *appendix;
    eTreeNode *node;
    os_int cid;
    os_boolean is_variable, is_container, is_matrix;

    for (v = content->firstv(); v; v = v->nextv())
    {
        cid = v->oid();
        if (cid == EOID_PARAMETER) continue;

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
                if (p == OS_NULL) {
                    p = new eParameterList(*win);
                }
                node = new eTreeNode(p);
                node->setpropertyv(ECOMP_TEXT, v);
                open_parameter_tree(mypath.gets(), appendix, win, node);
            }
        }
        else if (is_matrix) {
            t = new eTableView(*win);
            t->setpropertys(ECOMP_PATH, mypath.gets());
        }
    }
}


void eGui::open_camera_view(
    const os_char *path,
    eObject *content)
{
    eWindow *w;
    eCameraView *camview;
    eVariable *v, mypath, tmp;
    os_int cid;
    os_boolean is_brick_buffer;

    w = new eWindow(this);
    content->propertyv(ECONTP_TEXT, &tmp);
    if (tmp.isempty()) {
        tmp.sets("unnamed");
    }
    w->setpropertyv(ECOMP_TEXT, &tmp);
    w->setpropertyv(ECOMP_NAME, &tmp);

    for (v = content->firstv(); v; v = v->nextv())
    {
        cid = v->oid();
        if (cid == EOID_PARAMETER) continue;

        is_brick_buffer = eclasslist_isinstanceof(cid, ECLASSID_EIO_BRICK_BUFFER);

        mypath.sets(path);
        if (!v->isempty()) {
            mypath.appends("/");
            mypath.appendv(v);
        }

        if (is_brick_buffer) {
            camview = new eCameraView(w);
            camview->bind(ECOMP_VALUE, mypath.gets(), EBIND_METADATA);
        }
    }
}
