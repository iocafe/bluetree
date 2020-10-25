/**

  @file    esyncconnector.cpp
  @brief   Synchronized transfer helper object in eProcess memory tree.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    21.10.2020

  Process mutext must be locked when sync connector objects are accessed.

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
eSyncConnector::eSyncConnector(
    eObject *parent,
    e_oid id,
    os_int flags)
    : eObject(parent, id, flags)
{
    /* Cannot be cloned or serialized.
     */
    setflags(EOBJ_NOT_CLONABLE|EOBJ_NOT_SERIALIZABLE);
    m_in_air_count = 0;
    m_queue = new eContainer(this);
    m_failed = OS_FALSE;

    m_context = new eVariable(this);
    m_context->sets("sc");
    m_context->appendl(osal_rand(1, 100000));
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
void eSyncConnector::setupclass()
{
    const os_int cls = ECLASSID_SYNC_CONNECTOR;

    /* Add the class to class list.
     */
    os_lock();
    eclasslist_add(cls, (eNewObjFunc)newobj, "eSyncConnector");
    //propertysetdone(cls);
    os_unlock();
}


/**
****************************************************************************************************

  @brief Function to process incoming messages.

  The eSyncConnector::onmessage function handles messages received by object. If this function
  doesn't process message, it calls parent class'es onmessage function.

  @param   envelope Message envelope. Contains command, target and source paths and
           message content, etc.
  @return  None.

****************************************************************************************************
*/
void eSyncConnector::onmessage(
    eEnvelope *envelope)
{
    eObject *context;

    /* If at final destination for the message.
     */
    context = envelope->context();
    if (*envelope->target()=='\0' && context)
    {
        /* Make sure that this is reply to request sent out by this object.
         */
        if (context->classid() == ECLASSID_VARIABLE) if (!m_context->compare((eVariable*)context))
        {
          switch (envelope->command())
          {
            case ECMD_NO_TARGET:
            case ECMD_INTERRUPT:
            case ECMD_ERROR:
                m_failed = OS_TRUE;
                osal_event_set(m_event);
                return;

            default:
                envelope->clone(m_queue, EOID_ITEM);
                /* continues as ECMD_ACK...
                 */

            case ECMD_ACK:
                m_in_air_count--;
                osal_event_set(m_event);
#if OSAL_DEBUG
                if (m_in_air_count == -1) {
                    osal_debug_error("sync connector received more replies than it sent messages");
                }
#endif
                return;
          }
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
eStatus eSyncConnector::simpleproperty(
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

  @brief Send a message, synchronization.

  The eSyncConnector::send_message function sends an envelope as message. The message envelope
  (or clone of it) will be recieved as onmessage call by remote object.

  The envelope object given as argument is adopted/deleted by this function.

  @param   envelope Message envelope to send. Contains command, target and source paths and
           message content, etc. Envelope pointer will not be valid after this function call.
  @return  The function returns ESTATUS_SUCCESS if all is fine. Other return values indicate
           that there is no connection to the receiving object.

****************************************************************************************************
*/
eStatus eSyncConnector::send_message(
    eEnvelope *envelope)
{
    if (m_failed) {
        return ESTATUS_FAILED;
    }

    /* Set context to make sure that replies are messages sent by this object.
     */
    envelope->setcontext(m_context);
    message(envelope);

    /* Increment in air count.
     */
    increment_in_air_count();

    return ESTATUS_SUCCESS;
}


/**
****************************************************************************************************

  @brief Check for received reply messages.

  The function gets oldest received message from queue.

  @param  parent Received message (if any) is adopted from queue as child of this object.
  @return Pointer to received message envelope, or OS_NULL if no received messages in queue.

****************************************************************************************************
*/
eEnvelope *eSyncConnector::get_received_message(
    eObject *parent)
{
    eEnvelope *envelope;

    if (m_failed) {
        return OS_NULL;
    }

    envelope = eEnvelope::cast(m_queue->first());
    if (envelope) {
        envelope->adopt(parent, EOID_ITEM, EOBJ_TEMPORARY_ATTACHMENT | EOBJ_NO_MAP);
    }
    return envelope;
}
