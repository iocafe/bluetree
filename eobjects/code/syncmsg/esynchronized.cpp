/**

  @file    esynchronized.cpp
  @brief   Synchronized data exchange.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    21.10.2020

  When transferring large amount of data, it is sometimes necessary to divide the data
  into pieces and transfer these as received. Typically thread sending the data is
  in loop to collect the data, and should not process messages.

  To make this work, an intermediate eSynchronized object created under eProcess.
  Data will be sent and received data acknowledged by this object. This provides
  flow controlled data transfer.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "eobjects.h"


/**
****************************************************************************************************

  @brief Property binding class constructor.

  Clear member variables.
  @return  None.

****************************************************************************************************
*/
eSynchronized::eSynchronized(
    eObject *parent,
    e_oid id,
    os_int flags)
    : eObject(parent, id, flags)
{
    /* Row set bindings cannot be cloned or serialized.
     */
    setflags(EOBJ_NOT_CLONABLE|EOBJ_NOT_SERIALIZABLE);
}


/**
****************************************************************************************************

  @brief Virtual destructor.

  If connected, disconnect binding. Release all resources allocated for the binging.
  @return  None.

****************************************************************************************************
*/
eSynchronized::~eSynchronized()
{
}


/**
****************************************************************************************************

  @brief Add the class to class list and class'es properties to it's property set.

  The eVariable::setupclass function adds the class to class list and class'es properties to
  it's property set. The class list enables creating new objects dynamically by class identifier,
  which is used for serialization reader functions. The property set stores static list of
  class'es properties and metadata for those.

****************************************************************************************************
*/
void eSynchronized::setupclass()
{
    const os_int cls = ECLASSID_SYNCHRONIZED;

    /* Add the class to class list.
     */
    os_lock();
    eclasslist_add(cls, (eNewObjFunc)newobj, "eSynchronized");
    //propertysetdone(cls);
    os_unlock();
}


/**
****************************************************************************************************

  @brief Function to process incoming messages.

  The eSynchronized::onmessage function handles messages received by object. If this function
  doesn't process message, it calls parent class'es onmessage function.

  @param   envelope Message envelope. Contains command, target and source paths and
           message content, etc.
  @return  None.

****************************************************************************************************
*/
void eSynchronized::onmessage(
    eEnvelope *envelope)
{
    /* If at final destination for the message.
     */
    if (*envelope->target()=='\0')
    {
        switch (envelope->command())
        {
            case ECMD_BIND_REPLY:
                return;

            case ECMD_UNBIND:
            case ECMD_SRV_UNBIND:
            case ECMD_NO_TARGET:
                return;

            case ECMD_FWRD:
                return;

            case ECMD_ACK:
                return;

            case ECMD_REBIND:
                return;
        }
    }

    /* Call parent class'es onmessage.
     */
    eObject::onmessage(envelope);
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
eStatus eSynchronized::simpleproperty(
    os_int propertynr,
    eVariable *x)
{
    switch (propertynr)
    {
        /* case ERSETP_LIMIT:
            if (m_pstruct.limit == 0) goto clear_x;
            x->setl(m_pstruct.limit);
            break; */

        default:
            return eObject::simpleproperty(propertynr, x);
    }
    return ESTATUS_SUCCESS;

// clear_x:
    x->clear();
    return ESTATUS_SUCCESS;
}


/**
****************************************************************************************************

  @brief Property value has been received from binding.

  The eSynchronized::update function...
  @return None.

****************************************************************************************************
*/
/* void eSynchronized::update(
    eEnvelope *envelope)
{
    // eVariable *x;

    // x = eVariable::cast(envelope->content());
    // binding_setproperty(x);
    sendack(envelope);
}
*/


/**
****************************************************************************************************

  @brief Send acknowledge.

  The sendack function.

  @param  envelope Message envelope from server binding.
  @return None.

****************************************************************************************************
*/
/* void eSynchronized::sendack(
    eEnvelope *envelope)
{
    sendack_base(envelope);

    if ((m_bflags & EBIND_CLIENT) == 0 && m_ackcount)
    {
        setchanged();
    }
}
*/

/**
****************************************************************************************************

  @brief Synchronized received.

  The ack function decrements acknowledge wait count and tries to send again.

  @param  envelope Message envelope from server binding.
  @return None.

****************************************************************************************************
*/
/* void eSynchronized::ack(
    eEnvelope *envelope)
{
    ack_base(envelope);
}


*/
