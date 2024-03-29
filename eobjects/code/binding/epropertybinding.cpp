/**

  @file    epropertybinding.cpp
  @brief   Bind a property to an another object.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

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
ePropertyBinding::ePropertyBinding(
    eObject *parent,
    e_oid id,
    os_int flags)
    : eBinding(parent, id, flags)
{
    /* Clear member variables.
     */
    m_propertyname = OS_NULL;
    m_propertynamesz = 0;
}


/**
****************************************************************************************************

  @brief Virtual destructor.

  If connected, disconnect binding. Release all resources allocated for the binging.
  @return  None.

****************************************************************************************************
*/
ePropertyBinding::~ePropertyBinding()
{
    set_propertyname(OS_NULL);
}


/**
****************************************************************************************************

  @brief Clone object

  The ePropertyBinding::clone function clones and object including object's children.
  Names will be left detached in clone.

  @param  parent Parent for the clone.
  @param  id Object identifier for the clone.
  @param  aflags 0 for default operation. EOBJ_NO_MAP not to map names.
  @return Pointer to the clone.

****************************************************************************************************
*/
eObject *ePropertyBinding::clone(
    eObject *parent,
    e_oid id,
    os_int aflags)
{
    eObject
        *clonedobj,
        *child;

    clonedobj = new ePropertyBinding(parent, id == EOID_CHILD ? oid() : id, flags());

    for (child = first(EOID_ALL); child; child = child->next(EOID_ALL))
    {
        if (child->isclonable())
        {
            child->clone(clonedobj, child->oid(), aflags);
        }
    }

    return clonedobj;
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
void ePropertyBinding::setupclass()
{
    const os_int cls = ECLASSID_PROPERTY_BINDING;

    /* Add the class to class list.
     */
    os_lock();
    eclasslist_add(cls, (eNewObjFunc)newobj, "ePropertyBinding", ECLASSID_BINDING);
    os_unlock();
}


/**
****************************************************************************************************

  @brief Write property binding content to stream.

  The ePropertyBinding::writer() function serializes the property binding to stream. This writes
  only the content, use eObject::write() to save also class information, attachements, etc.

  @param  stream The stream to write to.
  @param  flags Serialization flags.

  @return If successfull the function returns ESTATUS_SUCCESS (0). If writing object to stream
          fails, value ESTATUS_WRITING_OBJ_FAILED is returned. Assume that all nonzero values
          indicate an error.

****************************************************************************************************
*/
eStatus ePropertyBinding::writer(
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

  @brief Read property binding content from stream.

  The ePropertyBinding::reader() function reads serialized propertybinding from stream.
  This function reads only the object content. To read whole object including attachments,
  names, etc, use eObject::read().

  @param  stream The stream to read from.
  @param  flags Serialization flags.

  @return If successfull the function returns ESTATUS_SUCCESS (0). If writing object to stream
          fails, value ESTATUS_READING_OBJ_FAILED is returned. Assume that all nonzero values
          indicate an error.

****************************************************************************************************
*/
eStatus ePropertyBinding::reader(
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

    /* Read children.
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

  @brief Function to process incoming messages.

  The ePropertyBinding::onmessage function handles messages received by object. If this function
  doesn't process message, it calls parent class'es onmessage function.

  @param   envelope Message envelope. Contains command, target and source paths and
           message content, etc.
  @return  None.

****************************************************************************************************
*/
void ePropertyBinding::onmessage(
    eEnvelope *envelope)
{
    os_int cmd;

    /* If at final destination for the message.
     */
    if (*envelope->target()=='\0')
    {
        cmd = envelope->command();
        switch (cmd)
        {
            case ECMD_BIND_REPLY:
                cbindok(this, envelope);
                return;

            case ECMD_UNBIND:
            case ECMD_SRV_UNBIND:
            case ECMD_NO_TARGET:
                if (m_bflags & EBIND_CLIENT)
                {
                    disconnect(OS_FALSE);
                }
                else
                {
                    delete this;
                }
                return;

            case ECMD_FWRD:
                update(envelope);
                return;

            case ECMD_ACK:
                ack(envelope);
                return;

            case ECMD_REBIND:
                bind2(OS_NULL);
                return;
        }
    }

    /* Call parent class'es onmessage.
     */
    eBinding::onmessage(envelope);
}


/**
****************************************************************************************************

  @brief Bind this object's property to remote property.

  The eObject::bind() function creates binding to remote property. When two variables are bound
  together, they have the same value. When the other changes, so does the another. Bindings
  work over messaging, thus binding work as well between objects in same thread or objects in
  different computers.

  @param  localpropertyno This object's propery number to bind.
  @param  remotepath Path to remote object to bind to.
  @param  remoteproperty Name of remote property to bind. If OS_NULL variable value
          is assumed.
  @param  bflags Combination of EBIND_DEFAULT (0), EBIND_CLIENTINIT, EBIND_NOFLOWCLT
          and EBIND_METADATA.
          - EBIND_DEFAULT:  bind without special options.
          - EBIND_CLIENTINIT: Local property value is used as initial value. Normally
            remote end's value is used as initial value.
          - EBIND_NOFLOWCLT: Disable flow control. Normally if property value changes
            faster than it can be transferred, some values are skipped. If EBIND_NOFLOWCLT
            flag is given, it disables flow control and every value is transferred without
            any limit to buffered memory use.
          - EBIND_METADATA: If meta data, like text, unit, attributes, etc exists, it is
            also transferred from remote object to local object.
          - EBIND_METADATA: Bind also attributes (subproperties like "x.min").
  @return None.

****************************************************************************************************
*/
void ePropertyBinding::bind(
    os_int localpropertynr,
    const os_char *remotepath,
    const os_char *remoteproperty,
    os_int bflags)
{
    /* Save bind parameters and flags.
     */
    set_propertyname(remoteproperty);
    m_localpropertynr = localpropertynr;
    m_bflags = bflags | EBIND_CLIENT;

    bind2(remotepath);
}

/* If remotepath is OS_NULL last used name will be preserved/
*/
void ePropertyBinding::bind2(
    const os_char *remotepath)
{
    eSet *parameters;
    eVariable x;

    /* Get parameters from derived class and add flags to parameters.
     */
    parameters = new eSet(this);
    parameters->setl(EPR_BINDING_FLAGS, m_bflags & EBIND_SER_MASK);
    parameters->sets(EPR_BINDING_PROPERTYNAME, m_propertyname);

    /* If this client is master, get property value.
     */
    if (m_bflags & EBIND_CLIENTINIT)
    {
        if (!binding_getproperty(&x))
        {
#if OSAL_DEBUG
            osal_debug_error("bind(): Unknown property number");
#endif
            return;
        }
        parameters->setv(EPR_BINDING_VALUE, &x);
    }

    /* If we are binding attributes like "x.min", get these.
     */
    if (m_bflags & EBIND_METADATA)
    {
        if (list_meta_pr_names(m_localpropertynr, &x))
        {
            parameters->setv(EPR_BINDING_META_PR_NAMES, &x);
        }
    }

    /* Call base class to do binding.
     */
    eBinding::bind_base(remotepath, parameters, OS_TRUE);
}


/**
****************************************************************************************************

  @brief Create server end property binding.

  The ePropertyBinding::srvbind() function...

  @param  envelope Received ECMD_BIND command envelope.
  @return None.

****************************************************************************************************
*/
void ePropertyBinding::srvbind(
    eObject *obj,
    eEnvelope *envelope)
{
    eSet *parameters, *reply;
    eVariable v, propertyname;
    const os_char *propertyname_str;

    parameters = eSet::cast(envelope->content());
    if (parameters == OS_NULL)
    {
#if OSAL_DEBUG
        osal_debug_error("srvbind() failed: no content");
#endif
        goto notarget;
    }

    /* Get property name.
     */
    if (!parameters->getv(EPR_BINDING_PROPERTYNAME, &propertyname))
    {
#if OSAL_DEBUG
        osal_debug_error("srvbind() failed: Property name missing");
#endif
        goto notarget;
    }
    propertyname_str = propertyname.gets();

    /* Convert property name to property number (-1 = unknown property).
     */
    m_localpropertynr = obj->propertynr(propertyname_str);
    if (m_localpropertynr < 0)
    {
#if OSAL_DEBUG
        osal_debug_error("srvbind() failed: Property name unknwon");
        osal_debug_error(propertyname_str);
#endif
        goto notarget;
    }

    /* Set flags. Set EBIND_INTERTHREAD if envelope has not been moved from thread to another.
     */
    m_bflags = (os_short)parameters->getl(EPR_BINDING_FLAGS);
    if (envelope->mflags() & EMSG_INTERTHREAD)
    {
        m_bflags |= EBIND_INTERTHREAD;
    }

    /* If subproperties are requested, list ones matching in both ends.
       Store initial property value, unless clientmaster.
     */
    reply = new eSet(this);

    /* If this client is nor master at initialization, get property value.
     */
    if ((m_bflags & EBIND_CLIENTINIT) == 0)
    {
        binding_getproperty(&v);
        reply->setv(EPR_BINDING_VALUE, &v);
    }
    else
    {
        parameters->getv(EPR_BINDING_VALUE, &v);
        binding_setproperty(&v);
    }

    /* If we need to send meta data with binding.
     */
    if (m_bflags & EBIND_METADATA)
    {
        parameters->getv(EPR_BINDING_META_PR_NAMES, &v);
        get_meta_pr_values(obj, propertyname_str, v.gets(), reply);
    }

    /* Complete the server end of binding and return.
     */
    srvbind_base(envelope, reply);
    return;

notarget:
    /* Send "no target" reply message to indicate that recipient was not found.
     */
    notarget(envelope);
}


/**
****************************************************************************************************

  @brief Complete property binding at client end.

  The ePropertyBinding::cbindok() function...

  @param  obj Pointer to object being bound.
  @param  envelope The enveloped returned from server end as ECMD_BIND_REPLY.
  @return None.

****************************************************************************************************
*/
void ePropertyBinding::cbindok(
    eObject *obj,
    eEnvelope *envelope)
{
    eSet *reply;
    eVariable v;

    reply = eSet::cast(envelope->content());
    if (reply == OS_NULL)
    {
#if OSAL_DEBUG
        osal_debug_error("cbindok() failed: no content");
#endif
        goto notarget;
    }

    /* If we need to send meta data with binding.
     */
    if (m_bflags & EBIND_METADATA)
    {
        set_meta_pr_values(reply);
    }

    /* If this server side is master at initialization, get property value.
     */
    if ((m_bflags & EBIND_CLIENTINIT) == 0)
    {
        reply->getv(EPR_BINDING_VALUE, &v);
        binding_setproperty(&v);
    }

notarget:

    /* Call base class to complete the binding.
     */
    cbindok_base(envelope);
}


/**
****************************************************************************************************

  @brief Mark property value changed.

  The ePropertyBinding::changed function marks a property value changed, so that it needs
  to forwarded. The function forwards the property value immediately, if flow control allows.
  Otherwise the property just remain marked to be forwarded.
  If property number given as argument is not for this binding, do nothing.

  @param propertynr Property number of the changed property.
  @param x Optional property value, used to save requerying it in common case.

  @return None.

****************************************************************************************************
*/
void ePropertyBinding::changed(
    os_int propertynr,
    eVariable *x,
    os_boolean delete_x)
{
    /* If not for this property, do nothing.
     */
    if (propertynr != m_localpropertynr) return;

    /* Mark property value, etc changed. Forward immediately, if binding if flow
       control does not block it.
     */
    setchanged();
    forward(x, delete_x);
}


/**
****************************************************************************************************

  @brief Forward property value trough binding.

  The forward function sends value of a property if flow control allows.

  @param  x Variable containing value, if available.
  @param  delete_x Flag weather value should be deleted.
  @return None.

****************************************************************************************************
*/
void ePropertyBinding::forward(
    eVariable *x,
    os_boolean delete_x)
{
    eVariable *tmp;

    if (forwardnow())
    {

        if (x == OS_NULL)
        {
            tmp = new eVariable;
            binding_getproperty(tmp);

            message(ECMD_FWRD, m_bindpath, OS_NULL, tmp,
                EMSG_DEL_CONTENT /* EMSG_NO_ERROR_MSGS */);
        }
        else
        {
            /* Send data as ECMD_FWRD message.
             */
            message(ECMD_FWRD, m_bindpath, OS_NULL, x,
                delete_x ? EMSG_DEL_CONTENT : EMSG_DEFAULT  /* EMSG_NO_ERROR_MSGS */);
            x = OS_NULL;
        }

        /* Clear changed bit and increment acknowledge count.
         */
        forwarddone();
    }

    if (delete_x && x)
    {
        delete x;
    }
}


/**
****************************************************************************************************

  @brief Property value has been received from binding.

  The ePropertyBinding::update function...
  @return None.

****************************************************************************************************
*/
void ePropertyBinding::update(
    eEnvelope *envelope)
{
    eVariable *x;

    x = eVariable::cast(envelope->content());
    binding_setproperty(x);
    sendack(envelope);
}


/**
****************************************************************************************************

  @brief Send acknowledge.

  The sendack function.

  @param  envelope Message envelope from server binding.
  @return None.

****************************************************************************************************
*/
void ePropertyBinding::sendack(
    eEnvelope *envelope)
{
    sendack_base(envelope);

    /* If this is server, if m_ackcount is not zero, mark changed.
     */
    if ((m_bflags & EBIND_CLIENT) == 0 && m_ackcount)
    {
        setchanged();
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
void ePropertyBinding::ack(
    eEnvelope *envelope)
{
    ack_base(envelope);
}


/**
****************************************************************************************************

  @brief Save remote property name.

  The ePropertyBinding::set_propertyname() releases current m_propertyname and stores
  propertyname given as argument. If propertyname is OS_NULL, memory is just freeed.

  @param  propertyname Pointer to object path.
  @return None.

****************************************************************************************************
*/
void ePropertyBinding::set_propertyname(
    const os_char *propertyname)
{
    if (m_propertyname)
    {
        os_free(m_propertyname, m_propertynamesz);
        m_propertyname = OS_NULL;
        m_propertynamesz = 0;
    }

    if (propertyname)
    {
        m_propertynamesz = (os_short)os_strlen(propertyname);
        m_propertyname = os_malloc(m_propertynamesz, OS_NULL);
        os_memcpy(m_propertyname, propertyname, m_propertynamesz);
    }
}


/**
****************************************************************************************************

  @brief Save property value.

  The binding_setproperty() is used to set a property of bound object.

  @param  x Variable holding property value to set.
  @return OS_TRUE if successfull.

****************************************************************************************************
*/
os_boolean ePropertyBinding::binding_setproperty(
    eVariable *x)
{
    eObject *obj;

    obj = grandparent();
    if (obj == OS_NULL) return OS_FALSE;

    /* Set property value.
     */
    obj->setpropertyv(m_localpropertynr, x, this);

    return OS_TRUE;
}


/**
****************************************************************************************************

  @brief Get property value.

  The binding_getproperty() function gets property value of bound object.

  @param  x Variable where to store property value.
  @return OS_TRUE if successfull.

****************************************************************************************************
*/
os_boolean ePropertyBinding::binding_getproperty(
    eVariable *x)
{
    eObject *obj;

    obj = grandparent();
    if (obj == OS_NULL) return OS_FALSE;

    /* Set property value.
     */
    obj->propertyv(m_localpropertynr, x);

    return OS_TRUE;
}


/**
****************************************************************************************************

  @brief List attributes (subproperties like "x.min") for the property.

  The ePropertyBinding::list_meta_pr_names() function...

  @param  x Variable where to store attribute list.
  @return OS_TRUE if successfull.

****************************************************************************************************
*/
os_boolean ePropertyBinding::list_meta_pr_names(
    os_int propertynr,
    eVariable *x)
{
    eContainer *propertyset;
    eVariable *propertyvar;
    eObject *obj;

    /* Get property set for the class.
     */
    obj = grandparent();
    if (obj == OS_NULL) return OS_FALSE;
    os_lock();
    propertyset = eglobal->propertysets->firstc(obj->classid());
    os_unlock();

    /* Get property var
     */
    propertyvar = propertyset->firstv(propertynr);
    if (propertyvar == OS_NULL) return OS_FALSE;

    /* Get subproperty list
     */
    propertyvar->propertyv(EVARP_CONF, x);
    return !x->isempty();
}


/**
****************************************************************************************************

  @brief Get values for metadata properties (like "x.min") for related to the property.

  The ePropertyBinding::get_meta_pr_values() function...

  @param  x Variable where to store attribute list.
  @return OS_TRUE if successfull.

****************************************************************************************************
*/
void ePropertyBinding::get_meta_pr_values(
    eObject *obj,
    const os_char *propertyname,
    const os_char *metadata_pr_list,
    eSet *reply)
{
    eVariable *v = OS_NULL, *mprname = OS_NULL, *pr = OS_NULL;
    eContainer *propertyset = OS_NULL;
    os_int meta_prnr, set_ix;
    os_char *meta_prname, *meta_prext, *q;
    const os_char *p, *e;

    /* Loop trough all requested metadata property name.
     */
    p = metadata_pr_list;
    set_ix = EPR_BINDING_META_PR_VALUES;
    while (*p != '\0')
    {
        if (v == OS_NULL) {
            v = new eVariable(ETEMPORARY);
            mprname = new eVariable(ETEMPORARY);
        }
        e = os_strchr(p, ',');
        if (e == OS_NULL) {
            e = os_strchr(p, '\0');
        }

        /* Generate metadata property name to try.
         */
        mprname->sets(propertyname);
        mprname->appends_nbytes(p, e - p);
        meta_prname = mprname->gets();
        meta_prext = os_strchr(meta_prname, '.');

        /* If there is object specific value, use it.
         */
        meta_prnr = obj->propertynr(meta_prname);
        if (meta_prnr >= 0 && meta_prext)
        {
            reply->sets(set_ix++, meta_prext);
            obj->propertyv(meta_prnr, v);
            reply->setv(set_ix++, v);
            goto goon;
        }

        /* Otherwise get class specific value.
         */
        if (propertyset == OS_NULL) {
            os_lock();
            propertyset = eglobal->propertysets->firstc(obj->classid());
            if (propertyset == OS_NULL) {
                os_unlock();
                goto goon;
            }
            pr = eVariable::cast(propertyset->byname(propertyname));
        }

        if (meta_prext[0] == '.') {
            q = os_strchr(meta_prext + 1, '.');
            if (q) meta_prext = q;
        }

        if (pr) {
            v->sets(evarp_value);
            v->appends_nbytes(p, e - p);

            meta_prnr = pr->propertynr(v->gets());
            if (meta_prnr >= 0) {
                pr->propertyv(meta_prnr, v);
                reply->sets(set_ix++, meta_prext);
                reply->setv(set_ix++, v);
            }
        }

goon:
        if (*e == '\0') break;
        p = e + 1;
    }

    if (propertyset) {
        os_unlock();
    }

    delete v;
    delete mprname;
}


/**
****************************************************************************************************

  @brief Set metadata to properties at client end of binding.

  The ePropertyBinding::set_meta_pr_values() function...

****************************************************************************************************
*/
void ePropertyBinding::set_meta_pr_values(
    eSet *reply)
{
    eObject *obj;
    eName *propertyname;
    eContainer *propertyset;
    eVariable meta_prext, meta_prname, *v;
    eVariable *meta_pvar, *propertyvar;
    os_int get_ix;

    /* Get local property name.
     */
    obj = grandparent();
    if (obj == OS_NULL) return;

    v = new eVariable(ETEMPORARY);
    os_lock();

    /* Get property var
     */
    propertyset = eglobal->propertysets->firstc(obj->classid());
    propertyvar = propertyset->firstv(m_localpropertynr);
    if (propertyvar == OS_NULL) goto getout;
    propertyname = propertyvar->primaryname();
    if (propertyname == OS_NULL) goto getout;
    // propertyname_str = propertyname.gets();

    get_ix = EPR_BINDING_META_PR_VALUES;
    while (reply->getv(get_ix++, &meta_prext)) {
        meta_prname.setv(propertyname);
        meta_prname.appendv(&meta_prext);
        reply->getv(get_ix++, v);

        meta_pvar = eVariable::cast(propertyset->byname(meta_prname.gets()));
        if (meta_pvar == OS_NULL) {
            osal_debug_error_str("set_meta_pr_values, property not found: ", meta_prname.gets());
            continue;
        }

        obj->setpropertyv(meta_pvar->oid(), v);
    }

getout:
    os_unlock();
    delete v;
}


/**
****************************************************************************************************

  @brief Get the next property binding identified by oid.

  The ePropertyBinding::nextpb() function returns pointer to the next property binding.

  @param   id Object idenfifier. Default value EOID_CHILD specifies to count a child objects,
           which are not flagged as an attachment. Value EOID_ALL specifies to get count all
           child objects, regardless wether these are attachment or not. Other values
           specify object identifier, only children with that specified object identifier
           are searched for.

  @return  Pointer to the ePropertyBinding, or OS_NULL if none found.

****************************************************************************************************
*/
ePropertyBinding *ePropertyBinding::nextpb(
    e_oid id)
{
    if (mm_handle == OS_NULL) return OS_NULL;
    eHandle *h = mm_handle->next(id);
    while (h)
    {
        if (h->object()->classid() == ECLASSID_PROPERTY_BINDING)
            return ePropertyBinding::cast(h->object());

        h = h->next(id);
    }
    return OS_NULL;
}
