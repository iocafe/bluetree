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

void eObject::setpropertyv_msg(
    const os_char *remotepath,
    eVariable *x,
    const os_char *propertyname,
    os_int mflags)
{
    eVariable path;

    if (propertyname) {
        path.sets(remotepath);
        path.appends("/_p/");
        path.appends(propertyname);
        remotepath = path.gets();
    }
    else {
        if (os_strstr(remotepath, "/_p/", OSAL_STRING_DEFAULT) == OS_NULL) {
            path.sets(remotepath);
            path.appends("/_p/x");
            remotepath = path.gets();
        }
    }

    message(ECMD_SETPROPERTY, remotepath, OS_NULL, x, mflags);
}

void eObject::setpropertyo_msg(
    const os_char *remotepath,
    eObject *x,
    const os_char *propertyname,
    os_int mflags)
{
    eVariable *v = new eVariable();
    v->seto(x, (mflags & EMSG_DEL_CONTENT) ? OS_TRUE : OS_FALSE);
    setpropertyv_msg(remotepath, v, propertyname, mflags|EMSG_DEL_CONTENT);
}


void eObject::setpropertyl_msg(
    const os_char *remotepath,
    os_long x,
    const os_char *propertyname)
{
    eVariable *v = new eVariable();
    v->setl(x);
    setpropertyv_msg(remotepath, v, propertyname, EMSG_DEL_CONTENT|EMSG_NO_REPLIES);
}


void eObject::setpropertyd_msg(
    const os_char *remotepath,
    os_double x,
    const os_char *propertyname)
{
    eVariable *v = new eVariable();
    v->setd(x);
    setpropertyv_msg(remotepath, v, propertyname, EMSG_DEL_CONTENT|EMSG_NO_REPLIES);
}


void eObject::setpropertys_msg(
    const os_char *remotepath,
    const os_char *x,
    const os_char *propertyname)
{
    eVariable *v = new eVariable();
    v->sets(x);
    setpropertyv_msg(remotepath, v, propertyname, EMSG_DEL_CONTENT|EMSG_NO_REPLIES);
}


/**
****************************************************************************************************

  @brief Add property to property set (any type).

  The function adds a property to class'es global property set.

  @param  cid Specifies to which classes property set the property is being added.
  @param  propertynr Property number, class specific.
  @param  propertyname Property name, class specific.
  @param  text Name of the property displayed to user. osal_str_empty to not to set text.
  @param  pflags Bit fields, combination of:
          - EPRO_DEFAULT (0): No options
          - EPRO_PERSISTENT: Property value is persistant is when saving.
          - EPRO_METADATA: The same as EPRO_PERSISTENT.
          - EPRO_SIMPLE: Do not keep copy of non default property in variable. Class implementation
            takes care about this.
          - EPRO_NOONPRCH: Do not call onpropertychange when value changes.
          - EPRO_NOPACK: Do not pack this property value but keep it as variable within property set.
          - EPRO_EARLYPRCH: Call onpropertychange() before setting stored property value.

  @return Pointer to the new eVariable representing the property in class'es property set.
          Additional attributes for the property can be added trough the returned pointer.

****************************************************************************************************
*/
eVariable *eObject::addproperty(
    os_int cid,
    os_int propertynr,
    const os_char *propertyname,
    const os_char *text,
    os_int pflags)
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
    if (text) if (*text != '\0') {
        p->setpropertys(EVARP_TEXT, text);
    }

    return p;
}


/**
****************************************************************************************************

  @brief Add bolean property to property set.

  The addpropertyb function adds property typed as boolean to property set.
  See addproperty() for more information.

  @param  x Default value.
  @return Pointer to eVariable in property set.

****************************************************************************************************
*/
eVariable *eObject::addpropertyb(
    os_int cid,
    os_int propertynr,
    const os_char *propertyname,
    const os_char *text,
    os_int pflags)
{
    eVariable *p;
    p = addproperty(cid, propertynr, propertyname, text, pflags);
    p->setpropertyl(EVARP_TYPE, OS_BOOLEAN);
    return p;
}


/**
****************************************************************************************************

  @brief Add boolean property to property set.

  The addpropertyb function adds property typed as boolean to class'es property set and
  sets initial/default value for it. See addproperty() for more information.

  @param  x Default value.
  @return Pointer to eVariable in property set.

****************************************************************************************************
*/
eVariable *eObject::addpropertyb(
    os_int cid,
    os_int propertynr,
    const os_char *propertyname,
    os_boolean x,
    const os_char *text,
    os_int pflags)
{
    eVariable *p;
    p = addpropertyb(cid, propertynr, propertyname, text, pflags);
    p->setpropertyl(EVARP_DEFAULT, x);
    p->setl(x);
    return p;
}


/**
****************************************************************************************************

  @brief Add integer property to property set.

  The addpropertyl function adds property typed as integer to property set.
  See addproperty() for more information.

  @param  x Default value.
  @return Pointer to eVariable in property set.

****************************************************************************************************
*/
eVariable *eObject::addpropertyl(
    os_int cid,
    os_int propertynr,
    const os_char *propertyname,
    const os_char *text,
    os_int pflags)
{
    eVariable *p;
    p = addproperty(cid, propertynr, propertyname, text, pflags);
    p->setpropertyl(EVARP_TYPE, OS_LONG);
    return p;
}


/**
****************************************************************************************************

  @brief Add integer property to property set.

  The addpropertyl function adds property typed as long integer to property set, and
  sets default value for it. See addproperty() for more information.

  @param  x Default value.
  @return Pointer to eVariable in property set.

****************************************************************************************************
*/
eVariable *eObject::addpropertyl(
    os_int cid,
    os_int propertynr,
    const os_char *propertyname,
    os_long x,
    const os_char *text,
    os_int pflags)
{
    eVariable *p;
    p = addpropertyl(cid, propertynr, propertyname, text, pflags);
    p->setpropertyl(EVARP_DEFAULT, x);
    p->setl(x);
    return p;
}


/**
****************************************************************************************************

  @brief Add double property to property set.

  The addpropertyd function adds property typed as double precision float to property set.
  See addproperty() for more information.

  @param  x Default value.
  @return Pointer to eVariable in property set.

****************************************************************************************************
*/
eVariable *eObject::addpropertyd(
    os_int cid,
    os_int propertynr,
    const os_char *propertyname,
    const os_char *text,
    os_int digs,
    os_int pflags)
{
    eVariable *p;
    p = addproperty(cid, propertynr, propertyname, text, pflags);
    p->setpropertyl(EVARP_TYPE, OS_DOUBLE);
    p->setpropertyl(EVARP_DIGS, digs);
    return p;
}


/**
****************************************************************************************************

  @brief Add double property to property set.

  The addpropertyd function adds property typed as double precision float to property set, and
  sets default value for it. See addproperty() for more information.

  @param  x Default value.
  @return Pointer to eVariable in property set.

****************************************************************************************************
*/
eVariable *eObject::addpropertyd(
    os_int cid,
    os_int propertynr,
    const os_char *propertyname,
    os_double x,
    const os_char *text,
    os_int digs,
    os_int pflags)
{
    eVariable *p;
    p = addpropertyd(cid, propertynr, propertyname, text, digs, pflags);
    p->setpropertyd(EVARP_DEFAULT, x);
    p->setd(x);
    return p;
}


/**
****************************************************************************************************

  @brief Add string property to property set.

  The addpropertys function adds property typed as string to property set.
  See addproperty() for more information.

  @param  x Default value.
  @return Pointer to eVariable in property set.

****************************************************************************************************
*/
eVariable *eObject::addpropertys(
    os_int cid,
    os_int propertynr,
    const os_char *propertyname,
    const os_char *text,
    os_int pflags)
{
    eVariable *p;
    p = addproperty(cid, propertynr, propertyname, text, pflags);
    p->setpropertyl(EVARP_TYPE, OS_STR);
    return p;
}


/**
****************************************************************************************************

  @brief Add string property to property set.

  The addpropertys function adds property typed as string to property set, and sets
  default value for it. See addproperty() for more information.

  @param  x Default value.
  @return Pointer to eVariable in property set.

****************************************************************************************************
*/
eVariable *eObject::addpropertys(
    os_int cid,
    os_int propertynr,
    const os_char *propertyname,
    const os_char *x,
    const os_char *text,
    os_int pflags)
{
    eVariable *p;
    p = addpropertys(cid, propertynr, propertyname, text, pflags);
    if (x) {
        p->sets(x);
        p->setpropertys(EVARP_DEFAULT, x);
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
    eVariable *p, *next_p, *mp;
    eName *name;
    os_char *propertyname, *e;

    /* Get pointer to class'es property set. If not found, create one. Property set always
       has name space
     */
    pset = eglobal->propertysets->firstc(cid);
    if (pset == OS_NULL) return;

    for (p = pset->firstv(); p; p = next_p)
    {
        next_p = p->nextv();
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
                mp->propertyv(EVARP_CONF, &v);
                if (!v.isempty()) {
                    v.appends(",");
                }
                v.appends(e);
                mp->setpropertyv(EVARP_CONF, &v);
            }
        }
    }
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
    if (pset == OS_NULL && (flags & EMSG_NO_ERRORS) == 0) {
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
    if (ns == OS_NULL) goto notfound;
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
    eVariable *p, *v;
    os_int pflags, sflags;

    /* Get global eVariable describing this property.
     */
    p = firstp(propertynr);
    if (p == OS_NULL)
    {
        osal_debug_error("setproperty: Property number is not valid for the class");
        return;
    }
    pflags = p->flags();

    v = new eVariable(this, EOID_ITEM, EOBJ_IS_ATTACHMENT);

    /* Empty x and x as null pointer are the same thing, handled the same way.
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
            propertyv(propertynr, v);
            if (!v->compare(x)) goto getout;
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
        properties->getv(propertynr, v);
        if (!v->compare(x)) goto getout;

        /* Early call class'es onpropertychange function.
         */
        if ((pflags & (EPRO_NOONPRCH|EPRO_EARLYPRCH)) == EPRO_EARLYPRCH)
        {
            onpropertychange(propertynr, x,  0); // CHECK FLAGS
        }

        /* If x matches to default value, then remove the
           value from eSet.
         */
        if (!p->compare(x))
        {
            properties->setv(propertynr, OS_NULL);
        }

        /* No match. Store x in eSet.
         */
        else
        {
            sflags = (pflags & (EPRO_PERSISTENT|EPRO_METADATA))
                ? ESET_PERSISTENT : ESET_TEMPORARY;

            if (pflags & EPRO_NOPACK) {
                sflags |= ESET_STORE_AS_VARIABLE;
            }

            properties->setv(propertynr, x, sflags);
        }

        /* Late (normal) call class'es onpropertychange function.
         */
        if ((pflags & (EPRO_NOONPRCH|EPRO_EARLYPRCH)) == 0)
        {
            onpropertychange(propertynr, x,  0); // CHECK FLAGS
        }
    }

    /* Forward property value to bindings, if any.
     */
    forwardproperty(propertynr, x, source, flags);

getout:
    delete v;
    return;
}


void eObject::setpropertyo(
    os_int propertynr,
    eObject *x,
    os_int mflags)
{
    eVariable *v;
    v = new eVariable(ETEMPORARY);
    v->seto(x, (mflags & EMSG_DEL_CONTENT) ? OS_TRUE : OS_FALSE);
    setpropertyv(propertynr, v);
    delete v;
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
       stored propery with property number.
     */
    properties = eSet::cast(first(EOID_PROPERTIES));
    if (properties) {
        if (properties->getv(propertynr, x)) return;
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

  @brief Forward simple property change to bindings.

  The propertychanged() function is called when value of simple property has been changed
  from within class. It forward changed value to bindings.

  @param   propertynr Property number to forward.

****************************************************************************************************
*/
void eObject::propertychanged(
    os_int propertynr)
{
    ePropertyBinding *b, *nextb;
    eVariable *v = OS_NULL;

    for (b = firstpb(); b; b = nextb)
    {
        nextb = b->nextpb();

        if (v == OS_NULL) {
            v = new eVariable(this, EOID_ITEM, EOBJ_TEMPORARY_ATTACHMENT);
            propertyv(propertynr, v);
        }

        /* Notice that this deleted v at last loop */
        b->changed(propertynr, v, nextb ? OS_FALSE : OS_TRUE);
    }
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
