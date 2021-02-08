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
    struct eioSignalInfo *sinfo)
{
    eioMblk *mblk;

    mblk = eioMblk::cast(grandparent());

    m_variable_ref->set(variable);

    m_signal.handle = mblk->handle_ptr();
    m_signal.addr = sinfo->addr;
    m_signal.flags = sinfo->flags;
    m_signal.n = sinfo->n;

    if (1) up();

        // sinfo->ncolumns;

}


void eioSignal::up()
{
    eioVariable *v;
    iocValue vv;
    os_short type_id;

    v = eioVariable::cast(m_variable_ref->get());
    if (v == OS_NULL) {
        return;
    }

    type_id = m_signal.flags & OSAL_TYPEID_MASK;


    if (type_id == OS_STR) {
        //vv[i].state_bits = ioc_move_str(sig, nbuf, sizeof(nbuf), OSAL_STATE_CONNECTED, flags);
    }
    else {
        ioc_move(&m_signal, &vv, 1, IOC_SIGNAL_NO_THREAD_SYNC);

        switch (type_id)
        {
            default:
                v->setl(vv.value.l);
                break;

            case OS_FLOAT:
            case OS_DOUBLE:
                v->setd(vv.value.d);
                break;
        }
    }
}
