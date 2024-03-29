/**

  @file    ebinding.cpp
  @brief   Binding base class for properties, DB tables and files.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    12.1.2016

  This base class serves derived classes for property, selection to table and file to handle
  bindings.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "eobjects.h"

/* Some shared property names for ePropertyBinding and eRowSetBinding.
 */
const os_char ebindp_bflags[] = "bflags";
const os_char ebindp_state[] = "state";


/**
****************************************************************************************************
  Binding base class constructor.  Clear member variables.
****************************************************************************************************
*/
eBinding::eBinding(
    eObject *parent,
    e_oid id,
    os_int flags)
    : eObject(parent, id, flags)
{
    /* Clear member variables.
     */
    m_state = E_BINDING_UNUSED;
    m_bflags = EBIND_DEFAULT;
    m_ackcount = 0;
    m_objpathsz = m_bindpathsz = 0;
    m_objpath = m_bindpath = OS_NULL;
}


/**
****************************************************************************************************

  @brief Virtual destructor.

  If connected, disconnect binding. Release all resources allocated for the binging.
  @return  None.

****************************************************************************************************
*/
eBinding::~eBinding()
{
    /* Disconnect and clear all allocated memory
     */
    disconnect(OS_TRUE);
    set_objpath(OS_NULL);
}


/**
****************************************************************************************************

  @brief Clone object

  The eBinding::clone function clones and object including object's children.
  Names will be left detached in clone.

  @param  parent Parent for the clone.
  @param  id Object identifier for the clone.
  @param  aflags 0 for default operation. EOBJ_NO_MAP not to map names.
  @return Pointer to the clone.

****************************************************************************************************
*/
eObject *eBinding::clone(
    eObject *parent,
    e_oid id,
    os_int aflags)
{
    return OS_NULL;
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
void eBinding::setupclass()
{
    const os_int cls = ECLASSID_BINDING;

    /* Add the class to class list.
     */
    os_lock();
    eclasslist_add(cls, (eNewObjFunc)OS_NULL, "eBinding");
    os_unlock();
}


/**
****************************************************************************************************

  @brief Write binding content to stream.

  The eBinding::writer() function serializes the binding to stream. This writes only the
  content, use eObject::write() to save also class information, attachements, etc.

  @param  stream The stream to write to.
  @param  flags Serialization flags.

  @return If successfull the function returns ESTATUS_SUCCESS (0). If writing object to stream
          fails, value ESTATUS_WRITING_OBJ_FAILED is returned. Assume that all nonzero values
          indicate an error.

****************************************************************************************************
*/
eStatus eBinding::writer(
    eStream *stream,
    os_int flags)
{
    /* Version number. Increment if new serialized items are added to the object,
       and check for new version's items in read() function.
     */
    const os_int version = 0;
    eObject *child;

    /* Begin the object and write version number.
     */
    if (stream->write_begin_block(version)) goto failed;

    /* Write child count to stream (no attachments).
     */
    if (*stream << childcount())  goto failed;

    /* Write childern (no attachments).
     */
    for (child = first(); child; child = child->next())
    {
        child->write(stream, flags);
    }

    /* End the object.
     */
    if (stream->write_end_block()) goto failed;

    /* Object successfully written.
     */
    return ESTATUS_SUCCESS;

    /* Writing object failed.
     */
failed:
    return ESTATUS_WRITING_OBJ_FAILED;
}


/**
****************************************************************************************************

  @brief Read binding content from stream.

  The eBinding::reader() function reads serialized binding from stream. This function
  reads only the object content. To read whole object including attachments, names, etc,
  use eObject::read().

  @param  stream The stream to read from.
  @param  flags Serialization flags.

  @return If successfull the function returns ESTATUS_SUCCESS (0). If writing object to stream
          fails, value ESTATUS_READING_OBJ_FAILED is returned. Assume that all nonzero values
          indicate an error.

****************************************************************************************************
*/
eStatus eBinding::reader(
    eStream *stream,
    os_int flags)
{
    /* Version number. Used to check which versions item's are in serialized data.
     */
    os_int version;
    os_long count;

    /* Read object start mark and version number.
     */
    if (stream->read_begin_block(&version)) goto failed;

    /* Read child count from (no attachments).
     */
    if (*stream >> count)  goto failed;

    /* Read children
     */
    while (count-- > 0)
    {
        read(stream, flags);
    }

    /* End the object.
     */
    if (stream->read_end_block()) goto failed;

    /* Object successfully read.
     */
    return ESTATUS_SUCCESS;

    /* Reading object failed.
     */
failed:
    return ESTATUS_READING_OBJ_FAILED;
}


/**
****************************************************************************************************

  @brief Connect client eBinding to server eBinding.

  The bind function initiates the binding in client end.

  @param  objpath Path to object to bind to. If objpath is NULL, it or m_bflags is not changed.
          This is used for reactivating binding.
  @param  parameters Parameters for binding, depends on binding use. If adopt parameters is set,
          this function adopts parameters eSet, so it cannot be used after this call.
  @param  adopt_parameters Set to true to adopt parameters argument, oe OS_FALSE to make a
          copy.

  @return None.

****************************************************************************************************
*/
void eBinding::bind_base(
    const os_char *objpath,
    eSet *parameters,
    os_boolean adopt_parameters)
{
    os_int cmd;

    /* Clear binding state.
     */
    disconnect(OS_FALSE);

    /* Save objpath. If objpath is NULL, this is skipped for reactivating binding.
     */
    if (objpath) {
        set_objpath(objpath);
    }
    set_bindpath(OS_NULL);

    /* Send ECMD_BIND or ECMD_BIND_RS (row set) message to object to bind to.
     */
    cmd = (classid() == ECLASSID_ROW_SET_BINDING) ? ECMD_BIND_RS : ECMD_BIND;
    message(cmd, m_objpath, OS_NULL, parameters,
        adopt_parameters ? EMSG_DEL_CONTENT : EMSG_KEEP_CONTENT /* EMSG_NO_ERROR_MSGS */);

    /* Set that we are binding now
     */
    m_state = E_BINDING_NOW;
}


/**
****************************************************************************************************

  @brief Complete server end of binding.

  The srvbind_base function...

  @return None.

****************************************************************************************************
*/
void eBinding::srvbind_base(
    eEnvelope *envelope,
    eObject *reply)
{
    /* Save path from which the message was received.
     */
    set_bindpath(envelope->source());

    /* Send ECMD_BIND_REPLY message to back to client binding.
     */
    message(ECMD_BIND_REPLY, m_bindpath, OS_NULL, reply,
        EMSG_DEL_CONTENT /* EMSG_NO_ERROR_MSGS */);

    /* Set binding state ok.
     */
    m_state = E_BINDING_OK;

    /* Do "binding disconnected" callback regardless. Here we do not care a
     */
    if ((m_bflags & EBIND_CLIENT) == 0) {
        eObject *obj = grandparent();
        if (obj) {
            obj->oncallback(ECALLBACK_SERVER_BINDING_CONNECTED, this, OS_NULL);
        }
    }
}


/**
****************************************************************************************************

  @brief Complete client end of binding.

  The cbindok_base function is called when client end of binding receives ECMD_BIND_REPLY message
  from server end of binding.

  @param  envelope Message envelope from server binding.
  @return None.

****************************************************************************************************
*/
void eBinding::cbindok_base(
    eEnvelope *envelope)
{
    /* Save path from which the message was received.
     */
    set_bindpath(envelope->source());

    /* If envelope has not been moved from thread to another.
     */
    if (envelope->mflags() & EMSG_INTERTHREAD)
    {
        m_bflags |= EBIND_INTERTHREAD;
    }

    /* Set binding state ok.
     */
    m_state = E_BINDING_OK;

    /* If server is master, then do not send changes before this moment.
     */
    if ((m_bflags & EBIND_CLIENTINIT) == 0)
    {
        m_bflags &= ~EBIND_CHANGED;
    }
    else
    {
        forward();
    }

}


/**
****************************************************************************************************

  @brief Send acknowledge.

  The sendack function.

  @param  envelope Message envelope from server binding.
  @return None.

****************************************************************************************************
*/
void eBinding::sendack_base(
    eEnvelope *envelope)
{
    /* Send ECMD_BIND_REPLY message to back to client binding.
     */
    if (m_bflags & EBIND_INTERTHREAD)
    {
        message(ECMD_ACK, m_bindpath);
    }
}


/**
****************************************************************************************************

  @brief Acknowledge received.

  The ack function decrements acknowledge wait count and tries to send again.

  @param  envelope Message envelope from server binding.
  @return None.

****************************************************************************************************
*/
void eBinding::ack_base(
    eEnvelope *envelope)
{
    m_ackcount--;
    forward();
}


/**
****************************************************************************************************

  @brief Save object path.

  The eBinding::set_objpath() releases current m_objpath and stores objpath given as argument.
  If objpath is OS_NULL, memory is just freeed.
  @param  objpath Pointer to object path.

  @return None.

****************************************************************************************************
*/
void eBinding::set_objpath(
    const os_char *objpath)
{
    if (m_objpath)
    {
        os_free(m_objpath, m_objpathsz);
        m_objpath = OS_NULL;
        m_objpathsz = 0;
    }

    if (objpath)
    {
        m_objpathsz = (os_short)os_strlen(objpath);
        m_objpath = os_malloc(m_objpathsz, OS_NULL);
        os_memcpy(m_objpath, objpath, m_objpathsz);
    }
}


/**
****************************************************************************************************

  @brief Save object path.

  The eBinding::set_objpath() releases current m_objpath and stores objpath given as argument.
  If objpath is OS_NULL, memory is just freeed.
  @param  objpath Pointer to object path.

  @return None.

****************************************************************************************************
*/
void eBinding::set_bindpath(
    const os_char *bindpath)
{
    if (m_bindpath)
    {
        os_free(m_bindpath, m_bindpathsz);
        m_bindpath = OS_NULL;
        m_bindpathsz = 0;
    }

    if (bindpath)
    {
        m_bindpathsz = (os_short)os_strlen(bindpath);
        m_bindpath = os_malloc(m_bindpathsz, OS_NULL);
        os_memcpy(m_bindpath, bindpath, m_bindpathsz);
    }
}


/**
****************************************************************************************************

  @brief Disconnect the binding and release allocated memory.

  The eBinding::disconnects() disconnects and clears allocated memory.

  @param   cmd ECMD_UNBIND, ECMD_SRV_UNBIND, or ECMD_NO_TARGET.
           ECMD_UNBIND Really forget the binding from eConnection's list.

****************************************************************************************************
*/
void eBinding::disconnect(
    os_boolean send_unbind_message)
{
    /* Send disconnect message
     */
    if (send_unbind_message) switch (m_state)
    {
        case E_BINDING_UNUSED:
            break;

        case E_BINDING_NOW:
            message(ECMD_UNBIND,
                 m_objpath, OS_NULL, OS_NULL, EMSG_NO_ERRORS);
            break;

        case E_BINDING_OK:
            message((m_bflags & EBIND_CLIENT) ? ECMD_UNBIND : ECMD_SRV_UNBIND,
                 m_bindpath, OS_NULL, OS_NULL, EMSG_NO_ERRORS|EMSG_NO_RESOLVE);
            break;
    }

    if (m_bindpath) {
        set_bindpath(OS_NULL);
    }

    /* Set unused state and clear changed bit and ack counter.
     */
    m_bflags &= ~(EBIND_CHANGED|EBIND_INTERTHREAD);
    m_ackcount = 0;

    /* Set unused state and oncallback of the bound object. Do "binding disconnected"
       callback regardless of object flags.
     */
    if (m_state != E_BINDING_UNUSED)
    {
        m_state = E_BINDING_UNUSED;

        /* Do "binding disconnected" callback regardless. Here we do not care a
         */
        if ((m_bflags & EBIND_CLIENT) == 0) {
            eObject *obj = grandparent();
            if (obj) {
                obj->oncallback(ECALLBACK_SERVER_BINDING_DISCONNECTED, this, OS_NULL);
            }
        }
    }
}
