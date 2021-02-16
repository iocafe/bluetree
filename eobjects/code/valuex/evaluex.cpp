/**

  @file    evaluex.cpp
  @brief   Extended value class.
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
#include "eobjects.h"


/**
****************************************************************************************************

  @brief Constructor.

  X...

  @return  None.

****************************************************************************************************
*/
eValueX::eValueX(
    eObject *parent,
    e_oid id,
    os_int flags)
    : eVariable(parent, id, flags)
{
    m_state_bits = OSAL_STATE_CONNECTED;
    m_timestamp = 0;
}


/**
****************************************************************************************************

  @brief Virtual destructor.

  X...

  @return  None.

****************************************************************************************************
*/
eValueX::~eValueX()
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
eObject *eValueX::clone(
    eObject *parent,
    e_oid id,
    os_int aflags)
{
    eValueX *clonedobj;
    clonedobj = new eValueX(parent, id == EOID_CHILD ? oid() : id, flags());

    /* Copy variable value.
     */
    clonedobj->setv(this);
    clonedobj->setdigs(digs());

    /* Copy state bits and time stamp.
     */
    clonedobj->m_state_bits = m_state_bits;
    clonedobj->m_timestamp = m_timestamp;


    /* Copy clonable attachments.
     */
    clonegeneric(clonedobj, aflags);
    return clonedobj;
}


/**
****************************************************************************************************

  @brief Add eValueX to class list and class'es properties to it's property set.

  The eValueX::setupclass function adds eValueX to class list and class'es properties to
  it's property set. The class list enables creating new objects dynamically by class identifier,
  which is used for serialization reader functions. The property set stores static list of
  class'es properties and metadata for those.

****************************************************************************************************
*/
void eValueX::setupclass()
{
    const os_int cls = ECLASSID_VALUEX;

    /* Add the class to class list.
     */
    os_lock();
    eclasslist_add(cls, (eNewObjFunc)newobj, "eValueX", ECLASSID_VARIABLE);
    eVariable::setupproperties(cls);
    addproperty (cls, EVARP_SBITS, evarp_sbits, "state bits", EPRO_PERSISTENT|EPRO_SIMPLE);
    addproperty (cls, EVARP_TSTAMP, evarp_tstamp, "timestamp", EPRO_PERSISTENT|EPRO_SIMPLE);
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

  @param   propertynr Property number of changed property.
  @param   x Variable containing the new value.
  @param   flags
  @return  If successfull, the function returns ESTATUS_SUCCESS (0). Nonzero return values do
           indicate that there was no property with given property number.

****************************************************************************************************
*/
eStatus eValueX::onpropertychange(
    os_int propertynr,
    eVariable *x,
    os_int flags)
{
    switch (propertynr)
    {
        case EVARP_SBITS:
            m_state_bits = (os_int)x->getl();
            break;

        case EVARP_TSTAMP:
            m_timestamp = x->getl();
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
eStatus eValueX::simpleproperty(
    os_int propertynr,
    eVariable *x)
{
    switch (propertynr)
    {
        case EVARP_SBITS:
            x->setl(m_state_bits);
            break;

        case EVARP_TSTAMP:
            x->setl(m_timestamp);
            break;

        default:
            return eVariable::simpleproperty(propertynr, x);
    }
    return ESTATUS_SUCCESS;
}

/**
****************************************************************************************************

  @brief Write name to stream.

  The eValueX::writer() function serializes eValueX to stream. This writes only content,
  use eObject::write() to save also class information, attachements, etc.

  @param  stream The stream to write to.
  @param  flags Serialization flags.

  @return If successfull the function returns ESTATUS_SUCCESS (0). If writing object to stream
          fails, value ESTATUS_WRITING_OBJ_FAILED is returned. Assume that all nonzero values
          indicate an error.

****************************************************************************************************
*/
eStatus eValueX::writer(
    eStream *stream,
    os_int flags)
{
    /* Version number. Increment if new serialized items are to the object,
       and check for new version's items in read() function.
     */
    const os_int version = 0;

    /* Begin the object and write version number.
     */
    if (stream->write_begin_block(version)) goto failed;

    /* Use base class'es function to do the work.
     */
    eVariable::writer(stream, flags);

    /* End the object.
     */
    if (stream->write_end_block()) goto failed;

    /* Object succesfully written.
     */
    return ESTATUS_SUCCESS;

    /* Writing object failed.
     */
failed:
    return ESTATUS_WRITING_OBJ_FAILED;
}


/**
****************************************************************************************************

  @brief Read name from stream.

  The eValueX::reader() function reads serialized name from stream. This function reads only content.
  To read whole object including attachments, names, etc, use eObject::read().

  @param  stream The stream to read from.
  @param  flags Serialization flags.

  @return If successfull the function returns ESTATUS_SUCCESS (0). If writing object to stream
          fails, value ESTATUS_READING_OBJ_FAILED is returned. Assume that all nonzero values
          indicate an error.

****************************************************************************************************
*/
eStatus eValueX::reader(
    eStream *stream,
    os_int flags)
{
    /* Version number. Used to check which versions item's are in serialized data.
     */
    os_int version;

    /* Read object start mark and version number.
     */
    if (stream->read_begin_block(&version)) goto failed;

    /* Use base class'es function to do the work.
     */
    eVariable::reader(stream, flags);

    /* End the object.
     */
    if (stream->read_end_block()) goto failed;

    /* Object succesfully read.
     */
    return ESTATUS_SUCCESS;

    /* Reading object failed.
     */
failed:
    return ESTATUS_READING_OBJ_FAILED;
}


/**
****************************************************************************************************

  @brief Compare value of this variable to another variable.

  If eVariable::compare function...

  @param   x Variable to compare to. If x is OS_NULL, then the function always returns 1.

  @return  -1:this < x, 0:this == x,1:this > x.

****************************************************************************************************
*/
os_int eValueX::compare(
    eObject *x,
    os_int flags)
{
    os_int rval;
    eValueX *ex;

    rval = eVariable::compare(x, flags);
    if (rval) return rval;

    if (x->classid() != ECLASSID_VALUEX) {
        return 0;
    }

    ex = eValueX::cast(x);
    if (m_timestamp != ex->m_timestamp) {
        return m_timestamp > ex->m_timestamp ? 1 : -1;
    }

    if (m_state_bits != ex->m_state_bits) {
        return m_state_bits > ex->m_state_bits ? 1 : -1;
    }
    return 0;
}
