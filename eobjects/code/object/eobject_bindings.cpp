/**

  @file    eobject_bindings.cpp
  @brief   Object base class, bindings.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  eObject base class binding related functions.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "eobjects.h"

/**
****************************************************************************************************

  @brief Forward property change trough bindings.

  The forwardproperty function...

  @param  propertynr
  @param  x
  @param  source
  @param  flags

  @return None.

****************************************************************************************************
*/
void eObject::forwardproperty(
    os_int propertynr,
    eVariable *x,
    eObject *source,
    os_int flags)
{
    ePropertyBinding *b, *nextb;

    for (b = firstpb(); b; b = nextb)
    {
        nextb = b->nextpb();
        if (b != source) {
            b->changed(propertynr, x, OS_FALSE);
        }
    }
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
          EBIND_METADATA and EBIND_TEMPORARY.
          - EBIND_DEFAULT:  bind without special options.
          - EBIND_CLIENTINIT: Local property value is used as initial value. Normally
            remote end's value is used as initial value.
          - EBIND_NOFLOWCLT: Disable flow control. Normally if property value changes
            faster than it can be transferred, some values are skipped. If EBIND_NOFLOWCLT
            flag is given, it disables flow control and every value is transferred without
            any limit to buffered memory use.
          - EBIND_METADATA: If meta data, like text, unit, attributes, etc exists, it is
            also transferred from remote object to local object.
          - EBIND_TEMPORARY: Binding is temporary and will not be cloned nor serialized.
  @param  envelope Used for server binding only. OS_NULL for clint binding.
  @return None.

****************************************************************************************************
*/
void eObject::bind(
    os_int localpropertynr,
    const os_char *remotepath,
    const os_char *remoteproperty,
    os_int bflags)
{
    eContainer *bindings;
    ePropertyBinding *binding;

    /* Get or create bindings container.
     */
    bindings = bindings_container();

    /* Verify that same binding dows not already exist ?? How to modify bindings ????
     */

    /* Create binding
     */
    binding = new ePropertyBinding(bindings, EOID_ITEM, (bflags & EBIND_TEMPORARY)
         ? EOBJ_NOT_CLONABLE | EOBJ_NOT_SERIALIZABLE : EOBJ_DEFAULT);

    /* Bind properties. This function will send message to remote object to bind.
     */
    binding->bind(localpropertynr, remotepath, remoteproperty, bflags);
}


/**
****************************************************************************************************

  @brief Bind this object's property to remote property.

  See bind() function above, this almost the same but remote path may contain also property
  name, separated from path by "/_p/". If "/_p/" is not found, the "x" is used as default.

****************************************************************************************************
*/
void eObject::bind(
    os_int localpropertynr,
    const os_char *remotepath,
    os_int bflags)
{
    eVariable v;
    os_char *p, *e;
    const os_char *ee;

    v.sets(remotepath);
    p = v.gets();
    e = os_strstr(p, "/_p/", OSAL_STRING_DEFAULT);
    if (e)
    {
        *e = '\0';
        ee = e + 4;
    }
    else
    {
        ee = evarp_value;
    }

    bind(localpropertynr, p, ee, bflags);
}


/**
****************************************************************************************************

  @brief Create server end of property binding.

  The eObject::srvbind() function

  @param  envelope Message envelope, COMMAND ECMD_BIND or ECMD_BIND_RS.
  @return None.

****************************************************************************************************
*/
void eObject::srvbind(
    eEnvelope *envelope)
{
    eContainer *bindings;
    eBinding *binding;
    eVariable *tmp;
    eSet *set;
    os_char *source;
    os_int cid, bf, pnr;

    /* Get or create bindings container.
     */
    bindings = bindings_container();

    /* Decide on binding class.
     */
    cid = ECLASSID_PROPERTY_BINDING;
    set = eSet::cast(envelope->content());
    if (set == OS_NULL) {
        osal_debug_error("srvbind without parameters");
        return;
    }
    bf = set->geti(ERSET_BINDING_FLAGS);
    if (bf & EBIND_BIND_ROWSET) {
        cid = ECLASSID_ROW_SET_BINDING;
        pnr = EOID_TABLE_SERVER_BINDING;
    }
    else {
        tmp = new eVariable(this, EOID_ITEM, EOBJ_TEMPORARY_ATTACHMENT);
        if (!set->getv(EPR_BINDING_PROPERTYNAME, tmp)) {
            osal_debug_error_str("srcbind: Unknown property: ", tmp->gets());
        }
        pnr = propertynr(tmp->gets());
        delete tmp;
    }

    /* Verify that same binding dows not already exist. If we have old binding, delete
     * it before creating new so we never have a duplicate.
     */
    source = envelope->source();
    bf &= (/* EBIND_METADATA|EBIND_CLIENT| */EBIND_BIND_ROWSET);
    for (binding = eBinding::cast(bindings->first(pnr));
         binding;
         binding = eBinding::cast(binding->next(pnr)))
    {
        if ((binding->bflags() & (/* EBIND_METADATA|EBIND_CLIENT| */EBIND_BIND_ROWSET)) != bf) continue;
        if (os_strcmp(binding->bindpath(), source)) continue;
    }
    if (binding) {
        delete binding;
    }

    /* Create binding
     */
    binding = eBinding::cast(newobject(bindings, cid, pnr,
         EOBJ_NOT_CLONABLE | EOBJ_NOT_SERIALIZABLE));

    binding->srvbind(this, envelope);
}


/**
****************************************************************************************************

  @brief Create bindings container for the object or get pointer to it.

  The eObject::bindings_container() function checks if bindings container identified as
  EOID_BINDINGS already exists. If it doesn't, the container is created as attachment of
  this object.

  @return  Pointer to the bindings container.

****************************************************************************************************
*/
eContainer *eObject::bindings_container()
{
    eContainer *bindings;

    /* Get or create bindings container.
     */
    bindings = firstc(EOID_BINDINGS);
    if (bindings == OS_NULL) {
        bindings = new eContainer(this, EOID_BINDINGS, EOBJ_IS_ATTACHMENT);
    }
    return bindings;
}


/**
****************************************************************************************************

  @brief Get object's the first property binding.

  The eObject::firstpb() function returns pointer to the first eRowSetBinding matching to oid.
  The binding is searched from object's "bindings" container.

  @param   id Object idenfifier. Default value EOID_CHILD specifies to count a child objects,
           which are not flagged as an attachment. Value EOID_ALL specifies to get count all
           child objects, regardless wether these are attachment or not. Other values
           specify object identifier, only children with that specified object identifier
           are searched for.

  @return  Pointer to the first property binding, or OS_NULL if none found.

****************************************************************************************************
*/
ePropertyBinding *eObject::firstpb(
    e_oid id)
{
    eContainer *bindings;
    eHandle *h;

    /* Get or create bindings container.
     */
    bindings = firstc(EOID_BINDINGS);
    if (bindings == OS_NULL) {
        return OS_NULL;
    }

    h = bindings->mm_handle->first(id);
    while (h)
    {
        if (h->m_object->classid() == ECLASSID_PROPERTY_BINDING)
            return ePropertyBinding::cast(h->m_object);

        h = h->next(id);
    }
    return OS_NULL;
}


/**
****************************************************************************************************

  @brief Get object's first row set binding.

  The eObject::firstrb() function returns pointer to the first eRowSetBinding matching to oid.
  The binding is searched from object's "bindings" container.

  @param   id EOID_TABLE_CLIENT_BINDING to loop trough client bindinds or EOID_TABLE_SERVER_BINDING
           to loop trough server bindings.

  @return  Pointer to the first row set binding, or OS_NULL if none found.

****************************************************************************************************
*/
eRowSetBinding *eObject::firstrb(
    e_oid id)
{
    eContainer *bindings;
    eHandle *h;

    /* Get or create bindings container.
     */
    bindings = firstc(EOID_BINDINGS);
    if (bindings == OS_NULL) {
        return OS_NULL;
    }

    h = bindings->mm_handle->first(id);
    while (h)
    {
        if (h->m_object->classid() == ECLASSID_ROW_SET_BINDING)
            return eRowSetBinding::cast(h->m_object);

        h = h->next(id);
    }
    return OS_NULL;
}


/**
****************************************************************************************************

  @brief Check if this object has server side bindings.

  It is often useful to know if anyone is "looking" at this object. For example it doesn't make
  sense to transfer camera data, if noone is looking at it. Also some objects should not be
  deleted until noone uses them.

  @return  OS_TRUE if this object has serve side bindings (is "needed" or "looked at").
           OS_FALSE if not.

****************************************************************************************************
*/
os_boolean eObject::is_bound()
{
    eContainer *bindings;
    ePropertyBinding *binding;
    eHandle *h;

    /* Get or create bindings container.
     */
    bindings = firstc(EOID_BINDINGS);
    if (bindings == OS_NULL) {
        return OS_NULL;
    }

    h = bindings->mm_handle->first(EOID_CHILD);
    while (h)
    {
        if (h->m_object->classid() == ECLASSID_PROPERTY_BINDING) {
            binding = ePropertyBinding::cast(h->m_object);
            if ((binding->bflags() & EBIND_CLIENT) == 0) {
                return OS_TRUE;
            }
        }
        h = h->next(EOID_CHILD);
    }
    return OS_FALSE;
}
