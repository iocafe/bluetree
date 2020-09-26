/**

  @file    eobject_properties.cpp
  @brief   Object base class.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  The eObject base class functionality related to objet properties.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "eobjects.h"

void eObject::setproperty_msg(
    const os_char *remotepath,
    eObject *x,
    const os_char *propertyname,
    os_int flags)
{
    eVariable
        path;

    if (propertyname)
    {
        path.sets(remotepath);
        path.appends("/_p/");
        path.appends(propertyname);
        remotepath = path.gets();
    }
    else
    {
        if (os_strstr(remotepath, "/_p/", OSAL_STRING_DEFAULT) == OS_NULL)
        {
            path.sets(remotepath);
            path.appends("/_p/x");
            remotepath = path.gets();
        }
    }

    message (ECMD_SETPROPERTY, remotepath, OS_NULL, x, EMSG_KEEP_CONTENT|EMSG_NO_REPLIES);
}

void eObject::setpropertyl_msg(
    const os_char *remotepath,
    os_long x,
    const os_char *propertyname)
{
    eVariable v;
    v.setl(x);
    setproperty_msg(remotepath,  &v, propertyname);
}

void eObject::setpropertyd_msg(
    const os_char *remotepath,
    os_double x,
    const os_char *propertyname)
{
    eVariable v;
    v.setd(x);
    setproperty_msg(remotepath,  &v, propertyname);
}

void eObject::setpropertys_msg(
    const os_char *remotepath,
    const os_char *x,
    const os_char *propertyname)
{
    eVariable v;
    v.sets(x);
    setproperty_msg(remotepath,  &v, propertyname);
}


/**
****************************************************************************************************

  @brief Add property to property set (any type).

  The addproperty function adds a property to class'es global property set.

  @param  classid Specifies to which classes property set the property is being added.
  @param  propertynr Property number, class specific.
  @param  propertyname Property name, class specific.
  @param  pflags Bit fields, combination of:
          - EPRO_DEFAULT (0): No options
          - EPRO_PERSISTENT: Property value is persistant is when saving.
          - EPRO_METADATA: Much like EPRO_PERSISTENT, but property value is saved if
            metadata is to be saved.
          - EPRO_SIMPLE: Do not keep copy of non default property in variable. Class implementation
            takes care about this.
          - EPRO_NOONPRCH: Do not call onpropertychange when value changes.
          - EPRO_NOPACK: Do not pack this property value within property set.
  @param  text Name of the property displayed to user.

  @return Pointer to eVariable in property set defining the property. Additional attributes for
          property can be added trough this returned pointer.

****************************************************************************************************
*/
eVariable *eObject::addproperty(
    os_int cid,
    os_int propertynr,
    const os_char *propertyname,
    os_int pflags,
    const os_char *text)
{
    eContainer *pset;
    eVariable *p;

    /* Get pointer to class'es property set. If not found, create one. Property set always
       has name space
     */
    pset = eglobal->propertysets->firstc(cid);
    if (pset == OS_NULL)
    {
        pset = new eContainer(eglobal->propertysets, cid, EOBJ_IS_ATTACHMENT);
        pset->ns_create();
    }

    /* Add variable for this property in property set and name it.
     */
    p = new eVariable(pset, propertynr, pflags);
    p->addname(propertyname);

    /* Set name of the property to display to user.
     */
    if (text) {
        p->setpropertys(EVARP_TEXT, text);
    }

    return p;
}


/**
****************************************************************************************************

  @brief Property set for class done, complete it.

  The propertysetdone function lists attributes (subproperties) for each base property.

  @param  classid Specifies to which classes property set the property is being added.
  @return None.

****************************************************************************************************
*/
void eObject::propertysetdone(
    os_int cid)
{
    eContainer *pset;
    eVariable *p, *mp;
    eName *name;
    os_char *propertyname, *e;

    /* Get pointer to class'es property set. If not found, create one. Property set always
       has name space
     */
    pset = eglobal->propertysets->firstc(cid);
    if (pset == OS_NULL) return;

    for (p = pset->firstv(); p; p = p->nextv())
    {
        name = p->firstn();
        if (name == OS_NULL) continue;
        propertyname = name->gets();

        /* If this is subproperty, like "x.min", add to main propertie's list of subproperties.
         */
        e = os_strchr((os_char*)propertyname, '.');
        if (e)
        {
            eVariable v;
            v.sets(propertyname, e - propertyname);
            mp = eVariable::cast(pset->byname(v.gets()));
            if (mp)
            {
                p->propertyv(EVARP_CONF, &v);
                v.appends(e);
                p->setpropertyv(EVARP_CONF, &v);
            }
        }
    }
}


/**
****************************************************************************************************

  @brief Add integer property to property set.

  The addpropertyl function adds property typed as integer to property set, and optionally
  sets default value for it. See addproperty() for more information.

  @param  x Default value.
  @return Pointer to eVariable in property set.

****************************************************************************************************
*/
eVariable *eObject::addpropertyl(
    os_int cid,
    os_int propertynr,
    const os_char *propertyname,
    os_int pflags,
    const os_char *text,
    os_long x)
{
    eVariable *p;
    p = addproperty(cid, propertynr, propertyname, pflags, text);
    p->setl(x);
    p->setpropertyl(EVARP_TYPE, OS_LONG);
    p->setl(x);
    return p;
}


/**
****************************************************************************************************

  @brief Add double property to property set.

  The addpropertyd function adds property typed as double precision float to property set, and
  optionally sets default value for it. See addproperty() for more information.

  @param  x Default value.
  @return Pointer to eVariable in property set.

****************************************************************************************************
*/
eVariable *eObject::addpropertyd(
    os_int cid,
    os_int propertynr,
    const os_char *propertyname,
    os_int pflags,
    const os_char *text,
    os_double x,
    os_int digs)
{
    eVariable *p;
    p = addproperty(cid, propertynr, propertyname, pflags, text);
    p->setd(x);
    p->setpropertyl(EVARP_TYPE, OS_DOUBLE);
    p->setpropertyl(EVARP_DIGS, digs);
    p->setd(x);
    return p;
}


/**
****************************************************************************************************

  @brief Add string property to property set.

  The addpropertys function adds property typed as string to property set, and optionally sets
  default value for it. See addproperty() for more information.

  @param  x Default value.
  @return Pointer to eVariable in property set.

****************************************************************************************************
*/
eVariable *eObject::addpropertys(
    os_int cid,
    os_int propertynr,
    const os_char *propertyname,
    os_int pflags,
    const os_char *text,
    const os_char *x)
{
    eVariable *p;
    p = addproperty(cid, propertynr, propertyname, pflags, text);
    p->setpropertyl(EVARP_TYPE, OS_STR);
    if (x)
    {
        p->sets(x);
    }
    return OS_NULL;
}


/**
****************************************************************************************************

  @brief Get pointer to class'es property set.

  The Object::propertyset function gets pointer to class'es property set. Mutex lock is used
  in case new classes are added.

  @param   flags EPRO_NO_ERRORS to disable error reporting if class has no property set.
           EPRO_DEFAULT for normal operation.
  @return  Pointer to class'es property set, or OS_NULL if class has no property set.


****************************************************************************************************
*/
eContainer *eObject::propertyset(
    os_int flags)
{
    eContainer *pset;

    os_lock();
    pset = eglobal->propertysets->firstc(classid());
    os_unlock();

#ifdef OSAL_DEBUG
    if (pset == OS_NULL && (flags|EMSG_NO_ERRORS) == 0) {
        osal_debug_error("setproperty: Class has no property support "
            "(did you call setupclass for it?)");
    }
#endif

    return pset;
}


/**
****************************************************************************************************

  @brief Get pointer to class'es first static property.

  The Object::firstp function gets pointer to class'first static property in property set.
  Static properties are global and can be read only.

  @param   id Object idenfifier. Default value EOID_CHILD specifies to count a child objects,
           which are not flagged as an attachment. Value EOID_ALL specifies to get count all
           child objects, regardless wether these are attachment or not. Other values
           specify object identifier, only children with that specified object identifier
           are searched for.
  @param   flags EPRO_NO_ERRORS to disable error reporting if class has no property set.
           EPRO_DEFAULT for normal operation.

  @return  Pointer to first property, eVariable class. OS_NULL if none found.

****************************************************************************************************
*/
eVariable *eObject::firstp(
    e_oid id,
    os_int flags)
{
    eContainer *pset;

    pset = propertyset(flags);

    if (pset) {
        return pset->firstv(id);
    }

    return OS_NULL;
}


/**
****************************************************************************************************

  @brief Initialize properties to default values.

  The initproperties function can be called from class'es constructor, if classes properties need
  to be initialized to default values. Properties with EPRO_SIMPLE or EPRO_NOONPRCH flag will not
  be initialized.


****************************************************************************************************
*/
void eObject::initproperties()
{
    eVariable *p;

    for (p = firstp();
         p;
         p = p->nextp())
    {
        if ((p->flags() & (EPRO_SIMPLE|EPRO_NOONPRCH)) == 0)
        {
            onpropertychange(p->oid(), p, 0);
        }
    }
}


/**
****************************************************************************************************

  @brief Get property number by property name.

  The propertynr() function gets property number for this class by property name.

  @param  propertyname Name of the property numnr
  @return Property number, -1 if failed.

****************************************************************************************************
*/
os_int eObject::propertynr(
    const os_char *propertyname)
{
    eContainer *pset;
    eNameSpace *ns;
    eName *name;
    os_int pnr;
    eVariable v;

    /* Get pointer to class'es property set. If not found, create one. Property set always
       has name space
     */
    pset = propertyset();
    if (pset == OS_NULL) goto notfound;

    /* Get property nr from global variable describing the property by name.
     */
    ns = eNameSpace::cast(pset->first(EOID_NAMESPACE));
    v.sets(propertyname);
    name = ns->findname(&v);
    if (name == OS_NULL) goto notfound;
    pnr = name->parent()->oid();

    /* End sync and return.
     */
    return pnr;

notfound:
    return -1;
}


/**
****************************************************************************************************

  @brief Get property name by property number.

  The propertynr() function gets property number for this class by property name.

  @param  propertyname Name of the property numnr
  @return Property number, OS_NULL if failed.

****************************************************************************************************
*/
os_char *eObject::propertyname(
    os_int propertynr)
{
    eName *name;
    eVariable *p;
    os_char *namestr;

    /* Get global variable for this propery.
     */
    p = firstp(propertynr);
    if (p == OS_NULL) goto notfound;

    /* get first name.
     */
    name = p->firstn(EOID_NAME);
    if (name == OS_NULL) goto notfound;
    namestr = name->gets();

    return namestr;

notfound:
    return OS_NULL;
}

/**
****************************************************************************************************

  @brief Set property value.

  The setproperty function sets property value from eVariable.

  @param  propertynr
  @param  x
  @param  source
  @param  flags

  @return None.

****************************************************************************************************
*/
void eObject::setpropertyv(
    os_int propertynr,
    eVariable *x,
    eObject *source,
    os_int flags)
{
    eSet *properties;
    eVariable *p;
    eVariable v;
    os_int pflags;

    /* Get global eVariable describing this property.
     */
    p = firstp(propertynr);
    if (p == OS_NULL)
    {
        osal_debug_error("setproperty: Property number is not valid for the class");
        return;
    }
    pflags = p->flags();

    /* Empty x and x as null pointer are thes ame thing, handle these in
       the same way.
     */
    if (x == OS_NULL)
    {
        x = eglobal->empty;
    }

    /* If this is simple property without marking.
     */
    if (pflags & EPRO_SIMPLE)
    {
        /* If new value variable is same as current one, do nothing.
         */
        if (x->type() != OS_OBJECT)
        {
            propertyv(propertynr, &v);
            if (!v.compare(x)) return;
        }

        /* Call class'es onpropertychange function.
         */
        if ((pflags & EPRO_NOONPRCH) == 0)
        {
            onpropertychange(propertynr, x,  0); // CHECK FLAGS
        }
    }
    else
    {
        /* Get eSet holding stored property values. If it doesn't exist, create it.
         */
        properties = eSet::cast(first(EOID_PROPERTIES));
        if (properties == OS_NULL)
        {
            properties = new eSet(this, EOID_PROPERTIES);
            properties->setflags(EOBJ_IS_ATTACHMENT);
        }

        /* Find stored property value. If matches value to set, do nothing.
         */
        properties->get(propertynr, &v);
        if (!v.compare(x)) return;

        /* Call class'es onpropertychange function.
         */
        if ((pflags & EPRO_NOONPRCH) == 0)
        {
            onpropertychange(propertynr, x,  0); // CHECK FLAGS
        }

        /* If x matches to default value, then remove the
           value from eSet.
         */
        if (!p->compare(x))
        {
            properties->set(propertynr, OS_NULL);
        }

        /* No match. Store x in eSet.
         */
        else
        {
            properties->set(propertynr, x);
        }
    }

    /* Forward property value to bindings, if any.
     */
    forwardproperty(propertynr, x, source, flags);
}


/* Set property value as integer.
 */
void eObject::setpropertyl(
    os_int propertynr,
    os_long x)
{
    eVariable v;
    v.setl(x);
    setpropertyv(propertynr, &v);
}

/* Set property value as double.
    */
void eObject::setpropertyd(
    os_int propertynr,
    os_double x)
{
    eVariable v;
    v.setd(x);
    setpropertyv(propertynr, &v);
}

/* Set property value as string.
    */
void eObject::setpropertys(
    os_int propertynr,
    const os_char *x)
{
    eVariable v;
    v.sets(x);
    setpropertyv(propertynr, &v);
}

/* Get property value.
 */
void eObject::propertyv(
    os_int propertynr,
    eVariable *x,
    os_int flags)
{
    eSet *properties;
    eVariable *p;

    /* Look for eSet holding stored property values. If found, check for
       property number.
     */
    properties = eSet::cast(first(EOID_PROPERTIES));
    if (properties)
    {
        /* Find stored property value. If matches value to set, do nothing.
         */
        if (properties->get(propertynr, x)) return;
    }

    /* Check for simple property
     */
    if (simpleproperty(propertynr, x) == ESTATUS_SUCCESS) return;

    /* Get global eVariable describing this property.
     */
    p = firstp(propertynr);
    if (p == OS_NULL)
    {
        osal_debug_error("setproperty: Property number is not valid for the class");
        x->clear();
        return;
    }

    /* Return default value for the property.
     */
    x->setv(p);
    return;
}


os_long eObject::propertyl(
    os_int propertynr)
{
    eVariable v;
    propertyv(propertynr, &v);
    return v.geti();
}

os_double eObject::propertyd(
    os_int propertynr)
{
    eVariable v;
    propertyv(propertynr, &v);
    return v.getd();
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
eStatus eObject::simpleproperty(
    os_int propertynr,
    eVariable *x)
{
    x->clear();
    return ESTATUS_NO_SIMPLE_PROPERTY_NR;
}
