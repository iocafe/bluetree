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
    eContainer *bindings;
    eObject *b, *nextb;

    /* Get bindings container.
     */
    bindings = firstc(EOID_BINDINGS);
    if (bindings == OS_NULL) return;

    for (b = bindings->first(); b; b = nextb)
    {
        nextb = b->next();
        if (b->classid() == ECLASSID_PROPERTY_BINDING && b != source)
        {
            ePropertyBinding::cast(b)->changed(propertynr, x, OS_FALSE);
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
    bindings = firstc(EOID_BINDINGS);
    if (bindings == OS_NULL)
    {
        bindings = new eContainer(this, EOID_BINDINGS, EOBJ_IS_ATTACHMENT);
    }

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

  @param  envelope Message envelope, COMMAND ECMD_BIND.
  @return None.

****************************************************************************************************
*/
void eObject::srvbind(
    eEnvelope *envelope)
{
    eContainer *bindings;
    ePropertyBinding *binding;

    /* Get or create bindings container.
     */
    bindings = firstc(EOID_BINDINGS);
    if (bindings == OS_NULL)
    {
        bindings = new eContainer(this, EOID_BINDINGS, EOBJ_IS_ATTACHMENT);
    }

    /* Verify that same binding dows not already exist ?? How to modify bindings ????
     */

    /* Create binding
     */
    binding = new ePropertyBinding(bindings, EOID_ITEM,
         EOBJ_NOT_CLONABLE | EOBJ_NOT_SERIALIZABLE);

    /* Bind properties.
     */
    if (binding)
    {
        binding->srvbind(this,  envelope);
    }
}