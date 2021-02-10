/**

  @file    eio_variable.cpp
  @brief   IO variable class.
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
eioVariable::eioVariable(
    eObject *parent,
    e_oid id,
    os_int flags)
    : eVariable(parent, id, flags)
{
    m_down_ref = OS_NULL;
    m_my_own_change = 0;
    m_value_set_by_user = OS_FALSE;
}


/**
****************************************************************************************************

  @brief Virtual destructor.

  X...

  @return  None.

****************************************************************************************************
*/
eioVariable::~eioVariable()
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
eObject *eioVariable::clone(
    eObject *parent,
    e_oid id,
    os_int aflags)
{
    eioVariable *clonedobj;
    clonedobj = new eioVariable(parent, id == EOID_CHILD ? oid() : id, flags());

    /* Copy variable value.
     */
    clonedobj->setv(this);
    clonedobj->setdigs(digs());

    /* Copy clonable attachments.
     */
    clonegeneric(clonedobj, aflags);
    return clonedobj;
}


/**
****************************************************************************************************

  @brief Add eioVariable to class list and class'es properties to it's property set.

  The eioVariable::setupclass function adds eioVariable to class list and class'es properties to
  it's property set. The class list enables creating new objects dynamically by class identifier,
  which is used for serialization reader functions. The property set stores static list of
  class'es properties and metadata for those.

****************************************************************************************************
*/
void eioVariable::setupclass()
{
    const os_int cls = ECLASSID_EIO_VARIABLE;

    /* Add the class to class list.
     */
    os_lock();
    eclasslist_add(cls, (eNewObjFunc)newobj, "eioVariable", ECLASSID_VARIABLE);
    eVariable::setupproperties(cls);
//    addproperty (cls, EIOP_SBITS, eiop_sbits, "state bits", EPRO_PERSISTENT|EPRO_SIMPLE);
//    addproperty (cls, EIOP_TSTAMP, eiop_tstamp, "timestamp", EPRO_PERSISTENT|EPRO_SIMPLE);
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
eStatus eioVariable::onpropertychange(
    os_int propertynr,
    eVariable *x,
    os_int flags)
{
    switch (propertynr)
    {
        /* case EIOP_SBITS:
            m_state_bits = (os_int)x->getl();
            break;

        case EIOP_TSTAMP:
            m_timestamp = x->getl();
            break; */

        case EVARP_VALUE:
            m_value_set_by_user = !m_my_own_change;
            if (compare(x)) {
                eVariable::onpropertychange(propertynr, x, flags);
                if (m_value_set_by_user) {
                    down();
                }
            }
            break;


        default:
            return eVariable::onpropertychange(propertynr, x, flags);
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
eStatus eioVariable::simpleproperty(
    os_int propertynr,
    eVariable *x)
{
    switch (propertynr)
    {
        /* case EIOP_SBITS:
            x->setl(m_state_bits);
            break;

        case EIOP_TSTAMP:
            x->setl(m_timestamp);
            break; */

        default:
            return eVariable::simpleproperty(propertynr, x);
    }
    return ESTATUS_SUCCESS;
}


void eioVariable::setup(
    struct eioSignal *signal,
    struct eioMblkInfo *minfo,
    struct eioSignalInfo *sinfo)
{
    os_short mblk_flags;
    os_int type_id;
    os_long min_value, max_value;
    os_boolean is_rdonly;

    mblk_flags = signal->mblk_flags();

    type_id = (sinfo->flags & OSAL_TYPEID_MASK);
    if (OSAL_IS_INTEGER_TYPE(type_id))
    {
        osal_type_range((osalTypeId)type_id, &min_value, &max_value);
        if (max_value > min_value) {
            setpropertyl(EVARP_MIN, min_value);
            setpropertyl(EVARP_MAX, max_value);
        }
        type_id = OS_LONG;
    }
    setpropertyl(EVARP_TYPE, type_id);

    if (mblk_flags & IOC_MBLK_DOWN) {
        if (m_down_ref == OS_NULL) {
            m_down_ref = new ePointer(this);
        }

        m_down_ref->set(signal);

        if (m_value_set_by_user && (mblk_flags & IOC_MBLK_UP) == 0) {
            down();
        }
    }

    is_rdonly = OS_TRUE;
    if (m_down_ref) if (m_down_ref->get()) {
        is_rdonly = OS_FALSE;
    }
    setpropertys(EVARP_ATTR, is_rdonly ? "rdonly" : "");
}

/* Adopts x.
 * os_lock() must be on when thi when this function is called.
 */
void eioVariable::up(eValueX *x)
{
    eObject *ox;

    /* Do nothing if value has not chaned.
     */
    if (type() == OS_OBJECT) {
        ox = m_value.valbuf.v.o;
        if (ox) {
            if (!ox->compare(eVariable::cast(x))) {
                delete x;
                return;
            }
        }
    }

    m_my_own_change++;
    setpropertyo(EVARP_VALUE, x, EMSG_DEL_CONTENT);
    m_my_own_change--;
}


void eioVariable::down()
{
    eioSignal *sig;

    if (m_down_ref) {
        sig = eioSignal::cast(m_down_ref->get());
        if (sig) {
            sig->down(this);
        }
    }
}
