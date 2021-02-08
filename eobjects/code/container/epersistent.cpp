/**

  @file    epersistent.cpp
  @brief   Persistent container (saved to disc).
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  The persistent object is container, which typically holds named eVariable and eMatrix items.
  - When a persisten object is initialised, the content is loaded from local file.
  - When a variable value, or marix content is changed, the persistent object's oncallback
    function gets called.
  - The oncallback function sets up timer to save the changes to disc drive.
  - When some time has elapsed since last change, the persistent object send's a clone
    of itself to eFileSystem object for saving (EPERP_ROOT_PATH property, typically "//fsys").

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "eobjects.h"


/* Persistent object property names.
 */
const os_char
    eperp_root_path[] = "root_path",
    eperp_relative_path[] = "rel_path",
    eperp_file[] = "file_name",
    eperp_save_time_ms[] = "time_ms",
    eperp_save_latest_time_ms[] = "latest_ms";

/**
****************************************************************************************************
  Constructor.
****************************************************************************************************
*/
ePersistent::ePersistent(
    eObject *parent,
    e_oid oid,
    os_int flags)
    : eContainer(parent, oid, flags)
{
    m_latest_touch = 0;
    m_oldest_touch = 0;
    m_save_time = 201;
    m_save_latest_time = 2001;
    m_timer_ms = 0;

    initproperties();
}


/**
****************************************************************************************************
  Virtual destructor.
****************************************************************************************************
*/
ePersistent::~ePersistent()
{
}


/**
****************************************************************************************************

  @brief Clone object

  The ePersistent::clone function clones and object including object's children.
  Names will be left detached in clone.

  @param  parent Parent for the clone.
  @param  oid Object identifier for the clone.
  @param  aflags 0 for default operation. EOBJ_NO_MAP not to map names.
  @return Pointer to the clone.

****************************************************************************************************
*/
eObject *ePersistent::clone(
    eObject *parent,
    e_oid id,
    os_int aflags)
{
    eObject *clonedobj;
    clonedobj = new ePersistent(parent, id == EOID_CHILD ? oid() : id, flags());
    clonegeneric(clonedobj, aflags|EOBJ_CLONE_ALL_CHILDREN);
    return clonedobj;
}


/**
****************************************************************************************************

  @brief Add the class to class list and class'es properties to it's property set.

  The ePersistent::setupclass function adds the class to class list and class'es properties to
  it's property set. The class list enables creating new objects dynamically by class identifier,
  which is used for serialization reader functions. The property set stores static list of
  class'es properties and metadata for those.

****************************************************************************************************
*/
void ePersistent::setupclass()
{
    const os_int cls = ECLASSID_PERSISTENT;
    eVariable *p;

    /* Add the class to class list.
     */
    os_lock();
    eclasslist_add(cls, (eNewObjFunc)newobj, "ePersistent", ECLASSID_CONTAINER);
    addpropertys(cls, EPERP_TEXT, eperp_text, "text", EPRO_PERSISTENT);
    addpropertys(cls, EPERP_ROOT_PATH, eperp_root_path, "//fsys", "root path", EPRO_DEFAULT);
    addproperty (cls, EPERP_RELATIVE_PATH, eperp_relative_path, "relative path", EPRO_DEFAULT);
    addpropertys(cls, EPERP_FILE, eperp_file, "unknown.eo", "file name", EPRO_PERSISTENT);
    p = addpropertyl(cls, EPERP_SAVE_TIME_MS, eperp_save_time_ms, 200, "save time", EPRO_DEFAULT);
    p->setpropertys(EVARP_UNIT, "ms");
    p = addpropertyl(cls, EPERP_SAVE_LATEST_TIME_MS, eperp_save_latest_time_ms, 2000,
        "save latest", EPRO_DEFAULT);
    p->setpropertys(EVARP_UNIT, "ms");
    propertysetdone(cls);
    os_unlock();
}


/**
****************************************************************************************************

  @brief Function to process incoming messages.

  The eTreeNode::onmessage function handles messages received by object. If this function
  doesn't process message, it calls parent class'es onmessage function.

  @param   envelope Message envelope. Contains command, target and source paths and
           message content, etc.
  @return  None.

****************************************************************************************************
*/
void ePersistent::onmessage(
    eEnvelope *envelope)
{
    /* If at final destination for the message.
     */
    if (*envelope->target()=='\0' && envelope->command() == ECMD_TIMER)
    {
        check_save_timer();
        return;
    }

    /* Default thread message processing.
     */
    eContainer::onmessage(envelope);
}


/**
****************************************************************************************************

  @brief Called to inform the class about property value change (override).

  The onpropertychange() function is called when class'es property changes, unless the
  property is flagged with EPRO_NOONPRCH.
  If property is flagged as EPRO_SIMPLE, this function shuold save the property value
  in class members and and return it when simpleproperty() is called.

  Notice for change logging: Previous value is still valid when this function is called.
  You can get the old value by calling property() function inside onpropertychange()
  function.

  @param   propertynr Property number of changed property.
  @param   x Variable containing the new value.
  @param   flags
  @return  If successfull, the function returns ESTATUS_SUCCESS (0). Nonzero return values do
           indicate that there was no property with given property number.

****************************************************************************************************
*/
eStatus ePersistent::onpropertychange(
    os_int propertynr,
    eVariable *x,
    os_int flags)
{
    switch (propertynr)
    {
        case EPERP_ROOT_PATH:
        case EPERP_RELATIVE_PATH:
        case EPERP_FILE:
            break;

        case EPERP_SAVE_TIME_MS:
            m_save_time = x->geti();
            break;

        case EPERP_SAVE_LATEST_TIME_MS:
            m_save_latest_time = x->geti();
            break;

        default:
            goto call_parent;
    }

    return ESTATUS_SUCCESS;

call_parent:
    return eContainer::onpropertychange(propertynr, x, flags);
}


/**
****************************************************************************************************

  @brief Process a callback from a child object.

  The ePersistent::oncallback function

****************************************************************************************************
*/
eStatus ePersistent::oncallback(
    eCallbackEvent event,
    eObject *obj,
    eObject *appendix)
{
    switch (event)
    {
        case ECALLBACK_VARIABLE_VALUE_CHANGED:
        case ECALLBACK_TABLE_CONTENT_CHANGED:
            touch();
            break;

        default:
            break;
    }

    /* If we need to pass callback to parent class.
     */
    if (flags() & (EOBJ_PERSISTENT_CALLBACK|EOBJ_TEMPORARY_CALLBACK)) {
        eContainer::oncallback(event, obj, appendix);
    }

    return ESTATUS_SUCCESS;
}


/**
****************************************************************************************************

  @brief Flags the peristent object changed (needs to be saved).

  The ePersistent::touch function

****************************************************************************************************
*/
void ePersistent::touch()
{
    os_get_timer(&m_latest_touch);
    if (m_oldest_touch == 0) {
        m_oldest_touch = m_latest_touch;
    }

    set_timer(m_save_time);
}


/**
****************************************************************************************************

  @brief Check if enugh time has passed since last change to save the peristent data.

  The ePersistent::check_save_timer function...

****************************************************************************************************
*/
void ePersistent::check_save_timer()
{
    os_timer now_t;

    if (m_timer_ms)
    {
        os_get_timer(&now_t);
        if (os_has_elapsed_since(&m_latest_touch, &now_t, m_save_time) ||
            os_has_elapsed_since(&m_oldest_touch, &now_t, m_save_latest_time))
        {
            save_as_message();
            m_latest_touch = 0;
            m_oldest_touch = 0;
            set_timer(0);
        }
    }
}


/**
****************************************************************************************************

  @brief Save persistent object by sending it as message to file system.

  The ePersistent::save_as_message function...

****************************************************************************************************
*/
void ePersistent::save_as_message()
{
    eVariable target, *relative_path, tmp;
    eContainer *content;
    const os_char *p;

    content = new eContainer(ETEMPORARY);
    relative_path = new eVariable(content, EOID_PATH);
    clone(content, EOID_CONTENT);

// content->print_json();

    propertyv(EPERP_ROOT_PATH, &target);
    get_relative_path(relative_path);
    propertyv(EPERP_FILE, &tmp);
    relative_path->appends("/");
    relative_path->appendv(&tmp);

    target.appends("/");
    target.appendv(relative_path);

    p = target.gets();
    message(ECMD_SAVE_FILE, p, OS_NULL, content, EMSG_DEL_CONTENT);

    docallback(ECALLBACK_PERSISTENT_CHANGED);
}


/**
****************************************************************************************************

  @brief Load persistent object from local file system.

  The ePersistent::load_file function...

****************************************************************************************************
*/
void ePersistent::load_file(
    const os_char *file_name)
{
    eVariable path, tmp;
    eObject *content;

    if (file_name) {
        setpropertys(EPERP_FILE, file_name);
    }

    path.sets(eglobal->root_path);
    path.appends("/");
    get_relative_path(&tmp);
    path.appendv(&tmp);
    path.appends("/");
    propertyv(EPERP_FILE, &tmp);
    path.appendv(&tmp);

    content = load(path.gets());
    if (content) {
        content->adopt(this, EOID_TEMPORARY, EOBJ_NO_MAP|EOBJ_IS_ATTACHMENT);
        use_loded_content(ePersistent::cast(content));
        delete content;
        set_timer(0);
    }
}


/* Get relative path, like "data/grumpy10"
 */
void ePersistent::get_relative_path(
    eVariable *relative_path)
{
    propertyv(EPERP_RELATIVE_PATH, relative_path);
    if (relative_path->isempty()) {
        relative_path->sets(eglobal->data_dir);
        setpropertyv(EPERP_RELATIVE_PATH, relative_path);
    }
}



/**
****************************************************************************************************

  @brief Copy loaded data to use.

  The ePersistent::use_loded_content function...

****************************************************************************************************
*/
void ePersistent::use_loded_content(
    ePersistent *content)
{
    eObject *srcobj, *dstobj;
    eName *srcname;

// content->print_json();

    for (srcobj = content->first();
         srcobj;
         srcobj = srcobj->next())
    {
        switch (srcobj->classid())
        {
            case ECLASSID_VARIABLE:
            case ECLASSID_MATRIX:
                break;

            default:
                continue;
        }

        srcname = srcobj->primaryname(ENAME_PARENT_NS);
        if (srcname) {
            dstobj = byname(srcname->gets());
            if (dstobj == OS_NULL) {
                dstobj = ns_get(srcname->gets(), srcname->namespaceid(), srcobj->classid());
            }
        }
        else {
            srcname = srcobj->primaryname();
            if (srcname == OS_NULL) continue;

            dstobj = ns_get(srcname->gets(), srcname->namespaceid(), srcobj->classid());
        }

        if (dstobj == OS_NULL) continue;
        switch (dstobj->classid())
        {
            case ECLASSID_VARIABLE:
                if (!((eVariable*)dstobj)->is_nosave()) {
                    ((eVariable*)dstobj)->setv((eVariable*)srcobj);
                }
                break;

            case ECLASSID_MATRIX:
                copy_loaded_matrix((eMatrix*)dstobj, (eMatrix*)srcobj);
                break;
        }
    }
}


/**
****************************************************************************************************

  @brief Copy loaded matrix data into used matrix.

  The ePersistent::copy_loaded_matrix function...

****************************************************************************************************
*/
void ePersistent::copy_loaded_matrix(
    eMatrix *dstm,
    eMatrix *srcm)
{
    eContainer *sc, *dc, *src_cols, *dst_cols;
    eName *n;
    eVariable *v, *tmp, *dcol;
    os_int max_src_cols, i, dst_i, *column_ix_tab;
    os_int nro_src_rows, row;

    /* Get column list of both source and destination matrices.
     */
    sc = srcm->configuration();
    dc = dstm->configuration();
    if (sc == OS_NULL || dc == OS_NULL) {
        osal_debug_error("copy_loaded_matrix: Unconfigured matrix");
        return;
    }
    src_cols = sc->firstc(EOID_TABLE_COLUMNS);
    dst_cols = dc->firstc(EOID_TABLE_COLUMNS);
    if (src_cols == OS_NULL || dst_cols == OS_NULL) {
        osal_debug_error("copy_loaded_matrix: No column information");
        return;
    }

    /* Generate column_ix_tab to convert source column index to destination column index.
     */
    max_src_cols = src_cols->childcount();
    column_ix_tab = (os_int*)os_malloc(max_src_cols * sizeof(os_int), OS_NULL);
    for (i = 0; i < max_src_cols; i++) {
        column_ix_tab[i] = -1;
    }
    for (v = src_cols->firstv(); v; v = v->nextv())
    {
        n = v->primaryname();
        if (n == OS_NULL) continue;
        dcol = eVariable::cast(dst_cols->byname(n->gets()));
        if (dcol == OS_NULL) continue;
        if (dcol->is_nosave()) continue;

        i = v->oid();
        if (i < 0 || i >= max_src_cols) {
            osal_debug_error("copy_loaded_matrix: column index out of bounds?");
            continue;
        }

        column_ix_tab[i] = dcol->oid();
    }
    column_ix_tab[EMTX_FLAGS_COLUMN_NR] = EMTX_FLAGS_COLUMN_NR;
    tmp = new eVariable(ETEMPORARY);

    nro_src_rows = srcm->nrows();
    for (row = 0; row < nro_src_rows; row++)
    {
        for (i = 0; i<max_src_cols; i++) {
            dst_i = column_ix_tab[i];
            if (dst_i < 0) continue;
            if (i == EMTX_FLAGS_COLUMN_NR) {
                if ((srcm->getl(row, i) & EMTX_FLAGS_ROW_OK) == 0) {
                    continue;
                }
            }
            srcm->getv(row, i, tmp);
            dstm->setv(row, dst_i, tmp);
        }
    }

    /* Cleanup
     */
    delete tmp;
    os_free(column_ix_tab, max_src_cols * sizeof(os_int));
}


/**
****************************************************************************************************

  @brief Set timer period, how often to recive timer messages.

  This function sets how oftern onmessage() is called with ECMD_TIMER argument by timer.
  Call this function instead of calling timer() directlt to avois repeated set or clear
  of the timer period.

  @param timer_ms Timer hit period in milliseconds, repeated. Set 0 to disable timer.

****************************************************************************************************
*/
void ePersistent::set_timer(
    os_int timer_ms)
{
    if (timer_ms != m_timer_ms) {
        m_timer_ms = timer_ms;
        timer(timer_ms);
    }
}
