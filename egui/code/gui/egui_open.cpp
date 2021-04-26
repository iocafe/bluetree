/**

  @file    egui_open.cpp
  @brief   Open object as new window.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

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
    const os_char *open_as = OS_NULL;
    OSAL_UNUSED(context);

    v = content->firstv(EOID_PARAMETER);
    if (v) open_as = v->gets();

    if (!os_strcmp(open_as, "camera")) {
        open_brick_buffer(path, content, open_as);
    }
    else if (!os_strcmp(open_as, "gamecontrol")) {
        open_signal_assembly(path, content, open_as);
    }
    else if (!os_strcmp(open_as, "graph")) {
    }
    else {
        w = OS_NULL;
        open_parameter_tree(path, content, &w, OS_NULL);
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


void eGui::open_brick_buffer(
    const os_char *path,
    eObject *content,
    const os_char *open_as)
{
    eWindow *w;
    eCameraView *camview;
    eVariable *v, mypath, tmp;
    os_int cid;

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

        mypath.sets(path);
        if (!v->isempty()) {
            mypath.appends("/");
            mypath.appendv(v);
        }

        if (!os_strcmp(open_as, "camera")) {
            camview = new eCameraView(w);
            camview->bind(ECOMP_VALUE, mypath.gets(), EBIND_METADATA);
            break;
        }
    }
}


void eGui::open_signal_assembly(
    const os_char *path,
    eObject *content,
    const os_char *open_as)
{
    eWindow *w;
    eObject *assembly;
    eVariable *v, mypath, tmp;
    os_int cid, property_nr;
    os_char *item_path, *property_name;

    w = new eWindow(this);
    content->propertyv(ECONTP_TEXT, &tmp);
    if (tmp.isempty()) {
        tmp.sets("unnamed");
    }
    w->setpropertyv(ECOMP_TEXT, &tmp);
    w->setpropertyv(ECOMP_NAME, &tmp);

    if (!os_strcmp(open_as, "gamecontrol")) {
        assembly = new eGameController(w);
    }
    else {
        osal_debug_error_str("eGui: unknown signal assembly: ", open_as);
        return;
    }

    for (v = content->firstv(); v; v = v->nextv())
    {
        cid = v->oid();
        if (cid == EOID_PARAMETER) continue;

        mypath.sets(path);
        if (!v->isempty()) {
            mypath.appends("/");
            mypath.appendv(v);
            item_path = mypath.gets();
            property_name = os_strchr(item_path, ',');
            if (property_name == OS_NULL) {
                osal_debug_error_str("eGui: invalid assembly item string: ", item_path);
                return;
            }
            *(property_name++) = '\0';
            tmp.sets("x.");
            tmp.appends(property_name);
            property_name = tmp.gets();

            property_nr = assembly->propertynr(property_name);
            if (property_nr < 0) {
                osal_debug_error_str("eGui: unknown assembly property: ", property_name);
                continue;
            }

            assembly->bind(property_nr, item_path, EBIND_DEFAULT);
        }
    }
}
