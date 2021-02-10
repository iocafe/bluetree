/**

  @file    esignal.cpp
  @brief   Object representing an IO signal.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    2.10.2020

  Value with timestamp and state bits to value.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "extensions/io/eio.h"


/**
****************************************************************************************************

  @brief Constructor.

  X...

  @return  None.

****************************************************************************************************
*/
eioSignal::eioSignal(
    eObject *parent,
    e_oid id,
    os_int flags)
    : eObject(parent, id, flags)
{
    os_memclear(&m_signal, sizeof(m_signal));
    m_mblk_flags = 0;
    m_ncolumns = 1;
    m_eio_root = OS_NULL;

    // m_state_bits = OSAL_STATE_CONNECTED;
    // m_timestamp = 0;
    m_variable_ref = new ePointer(this);
}


/**
****************************************************************************************************

  @brief Virtual destructor.

  X...

  @return  None.

****************************************************************************************************
*/
eioSignal::~eioSignal()
{
}


/**
****************************************************************************************************

  @brief Clone object

  The clone function clones the variable and clonable attachments. Names will be left detached
  in clone if EOBJ_NO_MAP flag is given.

  @param  parent Parent for the clone.
  @param  id Object identifier for the clone.
  @param  aflags 0 for default operation. EOBJ_NO_MAP not to map names.
  @return Pointer to the clone.

****************************************************************************************************
*/
eObject *eioSignal::clone(
    eObject *parent,
    e_oid id,
    os_int aflags)
{
    eioSignal *clonedobj;
    clonedobj = new eioSignal(parent, id == EOID_CHILD ? oid() : id, flags());

    os_memcpy(&clonedobj->m_signal, &m_signal, sizeof(iocSignal));

    /* Copy state bits and time stamp.
     */
    // clonedobj->m_state_bits = m_state_bits;
    // clonedobj->m_timestamp = m_timestamp;

    /* Copy clonable attachments.
     */
    clonegeneric(clonedobj, aflags);
    return clonedobj;
}


/**
****************************************************************************************************

  @brief Add eioSignal to class list and class'es properties to it's property set.

  The eioSignal::setupclass function adds eioSignal to class list and class'es properties to
  it's property set. The class list enables creating new objects dynamically by class identifier,
  which is used for serialization reader functions. The property set stores static list of
  class'es properties and metadata for those.

****************************************************************************************************
*/
void eioSignal::setupclass()
{
    const os_int cls = ECLASSID_EIO_SIGNAL;
    eVariable *vtype;
    eVariable tmp;

    /* Add the class to class list.
     */
    os_lock();
    eclasslist_add(cls, (eNewObjFunc)newobj, "eioSignal");
    addpropertyl(cls, EIOP_SIG_ADDR, eiop_sig_addr, "address", EPRO_PERSISTENT|EPRO_SIMPLE);
    addpropertyl(cls, EIOP_SIG_N, eiop_sig_n, "n", EPRO_PERSISTENT|EPRO_SIMPLE);
    vtype = addpropertyl(cls, EIOP_SIG_TYPE, eiop_sig_type, "type", EPRO_PERSISTENT|EPRO_SIMPLE);
    emake_type_enum_str(&tmp, OS_FALSE, OS_TRUE);
    vtype->setpropertyv(EVARP_ATTR, &tmp);

    propertysetdone(cls);
    os_unlock();
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
eStatus eioSignal::onpropertychange(
    os_int propertynr,
    eVariable *x,
    os_int flags)
{
    switch (propertynr)
    {
        case EIOP_SIG_ADDR: /* read only */
        case EIOP_SIG_N:
        case EIOP_SIG_TYPE:
            break;


        default:
            return eObject::onpropertychange(propertynr, x, flags);
    }

    return ESTATUS_SUCCESS;
}


/**
****************************************************************************************************

  @brief Get value of simple property (override).

  The simpleproperty() function stores current value of simple property into variable x.

  @param   propertynr Property number to get.
  @param   x Variable into which to store the property value.
  @return  If property with property number was stored in x, the function returns
           ESTATUS_SUCCESS (0). Nonzero return values indicate that property with
           given number was not among simple properties.

****************************************************************************************************
*/
eStatus eioSignal::simpleproperty(
    os_int propertynr,
    eVariable *x)
{
    switch (propertynr)
    {
        case EIOP_SIG_ADDR:
            x->setl(m_signal.addr);
            break;

        case EIOP_SIG_N:
            x->setl(m_signal.n);
            break;

        case EIOP_SIG_TYPE:
            x->setl(m_signal.flags & OSAL_TYPEID_MASK);
            break;

        default:
            return eObject::simpleproperty(propertynr, x);
    }
    return ESTATUS_SUCCESS;
}


void eioSignal::setup(
    eioVariable *variable,
    struct eioMblkInfo *minfo,
    struct eioSignalInfo *sinfo)
{
    eioMblk *mblk;

    mblk = eioMblk::cast(grandparent());

    m_variable_ref->set(variable);

    m_signal.handle = mblk->handle_ptr();
    m_signal.addr = sinfo->addr;
    m_signal.flags = sinfo->flags;
    m_signal.n = sinfo->n;
    m_ncolumns = sinfo->ncolumns;
    if (m_ncolumns < 1) m_ncolumns = 1;
    m_eio_root = minfo->eio_root;

    /* Copy memory block flags (like IOC_MBLK_DOWN, IOC_MBLK_UP) for fast access.
     */
    m_mblk_flags = mblk->mblk_flags();

#if IOC_BIDIRECTIONAL_MBLK_CODE
    if (m_mblk_flags & (IOC_MBLK_UP | IOC_BIDIRECTIONAL)) {
        up();
    }
#else
    if (m_mblk_flags & IOC_MBLK_UP) {
        up();
    }
#endif
}

/* os_lock() must be on when this function is called.
 */
void eioSignal::up()
{
    eioVariable *v;
    eMatrix *m;
    eValueX *x;
    iocValue vv;
    os_memsz p_sz, type_sz;
    os_int nrows, row, col;
    osalTypeId type_id, mtx_type_id;
    os_char  buf[64], *p, state_bits;
    os_long *l_ptr;

    v = eioVariable::cast(m_variable_ref->get());
    if (v == OS_NULL) {
        return;
    }

    x = new eValueX(ETEMPORARY);
    x->set_tstamp(m_eio_root->time_now());
    type_id = (osalTypeId)(m_signal.flags & OSAL_TYPEID_MASK);

    /* If string.
     */
    if (type_id == OS_STR) {
        p_sz = m_signal.n;
        if (p_sz < 1) p_sz = 1;
        if (p_sz <= (os_memsz)sizeof(buf)) {
            p = buf;
        }
        else {
            p = os_malloc(p_sz, OS_NULL);
        }
        state_bits = ioc_move_str(&m_signal, p, p_sz,
            OSAL_STATE_CONNECTED, OS_STR|IOC_SIGNAL_NO_THREAD_SYNC);
        x->set_sbits(state_bits & ~OSAL_STATE_BOOLEAN_VALUE);
        *x = p;

        if (p != buf) {
            os_free(p, p_sz);
        }
    }

    /* If array.
     */
    else if (m_signal.n > 1) {
        type_sz = osal_type_size(type_id);
        p_sz = m_signal.n * type_sz;
        p = os_malloc(p_sz, OS_NULL);

        state_bits = ioc_move_array(&m_signal, 0, p, m_signal.n,
            OSAL_STATE_CONNECTED, IOC_SIGNAL_NO_THREAD_SYNC|type_id);
        x->set_sbits(state_bits);

        nrows = (m_signal.n + m_ncolumns - 1) / m_ncolumns;
        m = new eMatrix(ETEMPORARY);
        if (OSAL_IS_FLOAT_TYPE(type_id)) {
            mtx_type_id = type_id;
        }
        else if (type_sz >= 4) {
            mtx_type_id = OS_LONG;
        }
        else if (type_sz >= 2) {
            mtx_type_id = OS_INT;
        }
        else {
            mtx_type_id = OS_SHORT;
        }
        m->allocate(mtx_type_id, nrows, m_ncolumns);

        l_ptr = (os_long*)p;
        for (row = 0; row < nrows; row++) {
            for (col = 0; col < m_ncolumns; col++) {
                switch (type_id) {
                    case OS_LONG: m->setl(row, col, *(l_ptr++)); break;
                    default: break;
                }
            }
        }

        os_free(p, p_sz);

        x->seto(m, OS_TRUE);
    }

    /* Otherwise plain signal.
     */
    else {
        ioc_move(&m_signal, &vv, 1, IOC_SIGNAL_NO_THREAD_SYNC);

        switch (type_id)
        {
            default:
                *x = vv.value.l;
                break;

            case OS_FLOAT:
            case OS_DOUBLE:
                *x = vv.value.d;
                break;
        }
        x->set_sbits(vv.state_bits & ~OSAL_STATE_BOOLEAN_VALUE);
    }

    v->up(x);
}

/* Does not modify x */
void eioSignal::down(eVariable *x)
{
//     eMatrix *m;
    iocValue vv;
//     os_memsz p_sz, type_sz;
//     os_int nrows, row, col;
    osalTypeId type_id;
    os_char  *p, state_bits;

    state_bits = (os_char)x->sbits();
    type_id = (osalTypeId)(m_signal.flags & OSAL_TYPEID_MASK);

    /* If string.
     */
    if (type_id == OS_STR) {
        p = x->gets();
        state_bits = ioc_move_str(&m_signal, p, os_strlen(p),
            state_bits, OS_STR|IOC_SIGNAL_NO_THREAD_SYNC|IOC_SIGNAL_WRITE);
    }

    /* If array.
     */
    else if (m_signal.n > 1) {
#if 0
        type_sz = osal_type_size(type_id);
        p_sz = m_signal.n * type_sz;
        p = os_malloc(p_sz, OS_NULL);

        state_bits = ioc_move_array(&m_signal, 0, p, m_signal.n,
            OSAL_STATE_CONNECTED, IOC_SIGNAL_NO_THREAD_SYNC);
        x->set_sbits(state_bits);

        nrows = (m_signal.n + m_ncolumns - 1) / m_ncolumns;
        m = new eMatrix(ETEMPORARY);

        l_ptr = (os_long*)p;
        for (row = 0; row < nrows; row++) {
            for (col = 0; col < m_ncolumns; col++) {
                switch (type_id) {
                    case OS_LONG: m->setl(row, col, *(l_ptr++)); break;
                    default: break;
                }
            }
        }

        os_free(p, p_sz);
#endif
    }

    /* Otherwise plain signal.
     */
    else {

        switch (type_id)
        {
            default:
                vv.value.l = x->getl();
                break;

            case OS_FLOAT:
            case OS_DOUBLE:
                vv.value.d = x->getd();
                break;
        }
        vv.state_bits = state_bits;
        ioc_move(&m_signal, &vv, 1, IOC_SIGNAL_NO_THREAD_SYNC|IOC_SIGNAL_WRITE);
    }

    m_eio_root->trig_io();
}
