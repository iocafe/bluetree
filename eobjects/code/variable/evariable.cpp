/**

  @file    evariable.cpp
  @brief   Dynamically typed variables.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  Dynamically typed variables...

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "eobjects.h"

/* Variable property names.
 */
const os_char
    evarp_value[] = "x",
    evarp_digs[] = "x.digs",
    evarp_text[] = "x.text",
    evarp_abbr[] = "x.abbr",
    evarp_ttip[] = "x.ttip",
    evarp_unit[] = "x.unit",
    evarp_min[] = "x.min",
    evarp_max[] = "x.max",
    evarp_type[] = "x.type",
    evarp_attr[] = "x.attr",
    evarp_default[] = "x.default",
    evarp_group[] = "x.group",
    evarp_gain[] = "x.gain",
    evarp_offset[] = "x.offset",
    evarp_sbits[] = "x.sbits",
    evarp_tstamp[] = "x.tstamp",
    evarp_conf[] = "_conf"; /* This MUST not start with "x." */


/**
****************************************************************************************************

  @brief Constructor.

  X...

  @return  None.

****************************************************************************************************
*/
eVariable::eVariable(
    eObject *parent,
    e_oid id,
    os_int flags)
    : eObject(parent, id, flags)
{
    /* No type, number 2 digits after decimal point for doubles.
     */
    m_vflags = OS_UNDEFINED_TYPE|(EVARP_DIGS_UNDEFINED << EVAR_DDIGS_SHIFT);
    m_value.valbuf.tmpstr = OS_NULL;
}


/**
****************************************************************************************************

  @brief Virtual destructor.

  X...

  @return  None.

****************************************************************************************************
*/
eVariable::~eVariable()
{
    /* Release any allocated memory.
     */
    clear();
}


/**
****************************************************************************************************

  @brief Clone object

  The clone function clones the variable and clonable attachments. Names will be left detached
  in clone if EOBJ_NO_MAP flag is given.

  @param  parent Parent for the clone.
  @param  id Object identifier for the clone. Set EOID_CHILD to use the same indentifier as the
          original object.
  @param  aflags EOBJ_NO_AFLAGS (0) for default operation.
          - EOBJ_NO_MAP not to map names.
          - EOBJ_CLONE_ALL_CHILDREN to clone all children, not just attachments.
          - EOBJ_NO_CLONED_NAMES Do not clone object names.

  @return Pointer to the new clone.

****************************************************************************************************
*/
eObject *eVariable::clone(
    eObject *parent,
    e_oid id,
    os_int aflags)
{
    eVariable *clonedobj;

    /* This must be eVariable, not class derived from it.
     */
    osal_debug_assert(classid() == ECLASSID_VARIABLE);

    clonedobj = new eVariable(parent, id == EOID_CHILD ? oid() : id, flags());

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

  @brief Add eVariable to class list and class'es properties to it's property set.

  The eVariable::setupclass function adds eVariable to class list and class'es properties to
  it's property set. The class list enables creating new objects dynamically by class identifier,
  which is used for serialization reader functions. The property set stores static list of
  class'es properties and metadata for those.

****************************************************************************************************
*/
void eVariable::setupclass()
{
    const os_int cls = ECLASSID_VARIABLE;

    /* Add the class to class list.
     */
    os_lock();
    eclasslist_add(cls, (eNewObjFunc)newobj, "eVariable");
    setupproperties(cls);
    propertysetdone(cls);
    os_unlock();
}


/**
****************************************************************************************************

  @brief Add class'es properties to property set.

  The eVariable::setupproperties is helper function for setupclass, it is called from both
  eVariable class and derived classes like eName.

  Process mutex must be locked when calling this function.

****************************************************************************************************
*/
void eVariable::setupproperties(
    os_int cls)
{
    eVariable *text, *vtype, *conf = OS_NULL;

    /* Order of these addproperty() calls is important, since eVariable itself is used to
       describe the properties in property set. The property to set must be added to
       property set before setting value for it. There is trick with p to set text type
       after adding property type. This effects only eVariable class.
     */
    if (cls == ECLASSID_VARIABLE) {
        conf = addproperty (cls, EVARP_CONF, evarp_conf, OS_NULL, EPRO_NOONPRCH); /* Property stuff */
    }
    text = addproperty(cls, EVARP_TEXT, evarp_text, "text", EPRO_METADATA|EPRO_NOONPRCH);
    vtype = addpropertyl (cls, EVARP_TYPE, evarp_type, "type", EPRO_METADATA|EPRO_NOONPRCH);
    addproperty (cls, EVARP_ABBR, evarp_abbr, "abbreviation", EPRO_METADATA|EPRO_NOONPRCH);
    addproperty (cls, EVARP_TTIP, evarp_ttip, "tooltip", EPRO_METADATA|EPRO_NOONPRCH);
    addproperty (cls, EVARP_VALUE, evarp_value, "value", EPRO_PERSISTENT|EPRO_SIMPLE);
    addproperty (cls, EVARP_DEFAULT, evarp_default, "default", EPRO_METADATA|EPRO_NOONPRCH);
    addpropertys(cls, EVARP_GROUP, evarp_group, "group", EPRO_METADATA|EPRO_NOONPRCH);
    addpropertyl(cls, EVARP_DIGS, evarp_digs, "digs", EPRO_METADATA|EPRO_SIMPLE);
    addpropertys(cls, EVARP_UNIT, evarp_unit, "unit", EPRO_METADATA|EPRO_NOONPRCH);
    addpropertyd(cls, EVARP_MIN, evarp_min, "min", EPRO_METADATA|EPRO_NOONPRCH);
    addpropertyd(cls, EVARP_MAX, evarp_max, "max", EPRO_METADATA|EPRO_NOONPRCH);
    addpropertys(cls, EVARP_ATTR, evarp_attr, "attr", EPRO_METADATA);
    addpropertyd(cls, EVARP_GAIN, evarp_gain, "gain", EPRO_METADATA|EPRO_NOONPRCH);
    addpropertyd(cls, EVARP_OFFSET, evarp_offset, "offset", EPRO_METADATA|EPRO_NOONPRCH);

    {
        eVariable tmp;
        text->setpropertyl(EVARP_TYPE, OS_STR);
        emake_type_enum_str(&tmp);
        vtype->setpropertyv(EVARP_ATTR, &tmp);
    }

    if (cls == ECLASSID_VARIABLE) {
        conf->setpropertys(EVARP_TEXT, "conf");
    }
}


/**
****************************************************************************************************

  @brief Collect information about this object for tree browser.

  The eVariable::object_info function fills in item (eVariable) to contain information
  about this object for tree browser view.

  @param   item Pointer to eVariable to set up with object information.
  @param   name Object's name if known. OS_NULL if object is not named or name is
           unknown at this time.
  @param   appendix Pointer to eSet into which to store property flags. The stored property
           flags indicate if object has namespace, children, or properties.
  @param   target Path "within object" when browsing a tree which is not made out
           of actual eObjects. For example OS file system directory.

****************************************************************************************************
*/
void eVariable::object_info(
    eVariable *item,
    eVariable *name,
    eSet *appendix,
    const os_char *target)
{
    eVariable value;
    os_int propertynr, i;

    static os_int copy_property_list[] = {EVARP_VALUE, EVARP_TYPE, EVARP_UNIT, EVARP_ATTR,
        EVARP_DEFAULT, EVARP_GROUP, EVARP_ABBR, EVARP_TTIP, EVARP_DIGS, EVARP_MIN, EVARP_MAX,
        EVARP_GAIN, EVARP_OFFSET, 0};

    eObject::object_info(item, name, appendix, target);
    appendix->setl(EBROWSE_RIGHT_CLICK_SELECTIONS, EBROWSE_OPEN);

    propertyv(EVARP_TEXT, &value);
    if (!value.isempty()) {
        eVariable modif_text;
        item->propertyv(EVARP_TEXT, &modif_text);
        if (!modif_text.starts_with(value.gets()))
        {
            modif_text += ", \"";
            modif_text += value;
            modif_text += "\"";
            item->setpropertyv(EVARP_TEXT, &modif_text);
        }
    }

    i = 0;
    while ((propertynr = copy_property_list[i++])) {
        propertyv(propertynr, &value);
        item->setpropertyv(propertynr, &value);
    }
}


/**
****************************************************************************************************

  @brief Get next child variable identified by oid.

  The eVariable::nextv() function returns pointer to the next object of the same class.

  @param   id Object idenfifier. Default value EOID_CHILD specifies to count a child objects,
           which are not flagged as an attachment. Value EOID_ALL specifies to get count all
           child objects, regardless wether these are attachment or not. Other values
           specify object identifier, only children with that specified object identifier
           are searched for.

  @return  Pointer to the next eVariable, or OS_NULL if none found.

****************************************************************************************************
*/
eVariable *eVariable::nextv(
    e_oid id)
{
    if (mm_handle == OS_NULL) return OS_NULL;
    eHandle *h = mm_handle->next(id);
    while (h)
    {
        if (h->object()->classid() == ECLASSID_VARIABLE)
            return eVariable::cast(h->object());

        h = h->next(id);
    }
    return OS_NULL;
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
eStatus eVariable::onpropertychange(
    os_int propertynr,
    eVariable *x,
    os_int flags)
{
    os_int di;

    switch (propertynr)
    {
        case EVARP_VALUE:
            setv(x);
            break;

        case EVARP_DIGS:
            if (x->isempty()) {
                di = EVARP_DIGS_UNDEFINED;
            }
            else {
                di = (os_int)x->getl();
            }
            setdigs(di);
            break;

        case EVARP_ATTR:
            m_vflags &= ~EVAR_NOSAVE;
            if (x->type() == OS_STR) {
                if (os_strstr(x->gets(), "nosave", OSAL_STRING_SEARCH_ITEM_NAME)) {
                    m_vflags |= EVAR_NOSAVE;
                }
            }
            break;

        default:
            return eObject::onpropertychange(propertynr, x, flags);
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
eStatus eVariable::simpleproperty(
    os_int propertynr,
    eVariable *x)
{
    os_int di;

    switch (propertynr)
    {
        case EVARP_VALUE:
            x->setv(this);
            break;

        case EVARP_DIGS:
            di = digs();
            if (di == EVARP_DIGS_UNDEFINED) {
                x->clear();
            }
            else {
                x->setl(di);
            }
            break;

        default:
            return eObject::simpleproperty(propertynr, x);
    }
    return ESTATUS_SUCCESS;
}


/**
****************************************************************************************************

  @brief Set number of digits after decimal point. .

  The setdigs() function sets number of digits after decimal point and clears buffered string,
  if any.

  @return  None.

****************************************************************************************************
*/
void eVariable::setdigs(os_int ddigs)
{
    m_vflags &= ~EVAR_DDIGS_MASK;
    m_vflags |= ((ddigs << EVAR_DDIGS_SHIFT) & EVAR_DDIGS_MASK);
    cleartmpstr();
}


/**
****************************************************************************************************

  @brief Empty the variable value.

  The clear() function clears variable value and frees ay memory allocated for the variable.

  @return  None.

****************************************************************************************************
*/
void eVariable::clear()
{
    switch (type())
    {
        /* If this is string, check if long string has been allocated in separate
           buffer.
         */
        case OS_STR:
            /* If separate string buffer has been allocated.
             */
            if (m_vflags & EVAR_STRBUF_ALLOCATED)
            {
                os_free(m_value.strptr.ptr, m_value.strptr.allocated);
            }
            break;

        /* If variable contains object, delete it.
         */
        case OS_OBJECT:
            delete m_value.valbuf.v.o;

            /* Continues...
             */

        /* Other data types: Delete temporary string, if any.
         */
        default:
            if (m_value.valbuf.tmpstr)
            {
                os_free(m_value.valbuf.tmpstr,
                    m_value.valbuf.tmpstr_sz);
            }
            break;
    }

    /* Mark that variable contains no data. Mark that string buffer has not
       been allocated.
     */
    settype(OS_UNDEFINED_TYPE);
    m_value.valbuf.tmpstr = OS_NULL;
    m_vflags &= ~EVAR_STRBUF_ALLOCATED;
}


/**
****************************************************************************************************

  @brief Set integer value to variable.

  The setl() function sets an integer value to variable. Integer values are stored as os_long type.

  @param   x Integer value to set.
  @return  None.

****************************************************************************************************
*/
void eVariable::setl(
    const os_long x)
{
    os_boolean change;

    /* Check if the new value is different from the old one. (We must still call clear() even if
       value is different, we may have a string buffer which needs to be released).
     */
    change = (os_boolean)(type() != OS_LONG || m_value.valbuf.v.l != x);

    /* Release any allocated memory.
     */
    clear();

    /* Save value.
     */
    m_value.valbuf.v.l = x;

    /* Set data type.
     */
    settype(OS_LONG);

    /* Inform parent object that a variable value was changed.
     */
    if (change && (m_vflags & EVAR_NOSAVE) == 0) {
        docallback(ECALLBACK_VARIABLE_VALUE_CHANGED);
    }
}


/**
****************************************************************************************************

  @brief Set double value to variable.

  The setd() function sets a double precision floating point value to variable.

  @param   x Value to set.
  @return  None.

****************************************************************************************************
*/
void eVariable::setd(
    const os_double x)
{
    os_boolean change;

    /* Check if the new value is different from the old one. (We must still call clear() even if
       value is different, we may have a string buffer which needs to be released).
     */
    change = (os_boolean)(type() != OS_DOUBLE || m_value.valbuf.v.d != x);

    /* Release any allocated memory.
     */
    clear();

    /* Save value.
     */
    m_value.valbuf.v.d = x;

    /* Set data type.
     */
    settype(OS_DOUBLE);

    /* Inform parent object that a variable value was changed.
     */
    if (change && (m_vflags & EVAR_NOSAVE) == 0) {
        docallback(ECALLBACK_VARIABLE_VALUE_CHANGED);
    }
}


/**
****************************************************************************************************

  @brief Set string value to variable.

  The sets() function sets a string value to variable.

  @param   x Value to set. OS_NULL is same as empty string.
  @param   max_chars Maximum number of characters to store excluding terminating NULL character.
           -1 for unlimited.
  @return  None.

****************************************************************************************************
*/
void eVariable::sets(
    const os_char *x,
    os_memsz max_chars)
{
    os_memsz n, tmpstr_sz;
    os_char *tmpstr, *str;
    os_int change;

    /* Minimum string is:
     */
    if (x == OS_NULL) x = "";

    /* Save temporary buffer if any, in case it is used as argument.
     */
    tmpstr = OS_NULL;
    if (type() != OS_STR)
    {
        tmpstr = m_value.valbuf.tmpstr;
        tmpstr_sz = m_value.valbuf.tmpstr_sz;
        m_value.valbuf.tmpstr = OS_NULL;
        change = OS_TRUE;
    }

    else {
        /* Check if the new value is different from the old one. (We must still call clear() even if
           value is different, we may have a string buffer which needs to be released).
         */
        str = (m_vflags & EVAR_STRBUF_ALLOCATED) ? m_value.strptr.ptr : m_value.strbuf.buf;
        change = os_strcmp(x, str);
    }

    /* Release any allocated memory.
     */
    clear();

    /* Get string length. Check if string length is limited.
     */
    n = os_strlen(x);
    if (max_chars >= 0)
    {
        if (n > max_chars+1) n = max_chars+1;
        else max_chars = -1;
    }

    /* If string fits into small buffer, copy it and save used size.
     */
    if (n <= EVARIABLE_STRBUF_SZ)
    {
        os_memcpy(m_value.strbuf.buf, x, n);
        if (max_chars >= 0) m_value.strbuf.buf[max_chars] = '\0';
        m_value.strbuf.used = (os_uchar)n;
    }

    /* Otherwise we need to allocate buffer for long string. Allocate buffer, copy data in,
       save allocated size and used size. Set EVAR_STRBUF_ALLOCATED flag to indicate that
       buffer was allocated.
     */
    else
    {
        m_value.strptr.ptr = os_malloc(n, &m_value.strptr.allocated);
        os_memcpy(m_value.strptr.ptr, x, n);
        if (max_chars >= 0) m_value.strptr.ptr[max_chars] = '\0';
        m_value.strptr.used = n;
        m_vflags |= EVAR_STRBUF_ALLOCATED;
    }

    /* Set data type.
     */
    settype(OS_STR);

    /* Release temporary string buffer.
     */
    if (tmpstr)
    {
        os_free(tmpstr, tmpstr_sz);
    }

    /* Inform parent object that a variable value was changed.
     */
    if (change && (m_vflags & EVAR_NOSAVE) == 0) {
        docallback(ECALLBACK_VARIABLE_VALUE_CHANGED);
    }
}


/**
****************************************************************************************************

  @brief Copy or move variable value from another variable.

  The setv() function sets a double precision floating point value to variable.

  @param  x Value to set.
  @param  move_value If OS_FALSE, the value of x is copied to this variable. If OS_TRUE and
          x contains allocated string or object, the value is moved from x to this variable
          and X will be empty after this call.
  @return None.

****************************************************************************************************
*/
void eVariable::setv(
    eVariable *x,
    os_boolean move_value)
{
    osalTypeId srctype, oldtype;
    os_memsz n;
    os_char *newstr, *oldstr;
    os_boolean change;

    change = OS_TRUE;
    oldtype = type();

    if (x) {
        srctype = x->type();
    }
    else {
        srctype = OS_UNDEFINED_TYPE;
    }

    if (srctype == OS_STR && oldtype == OS_STR)
    {
        /* Check if the new string is same as old one. .
         */
        oldstr = (m_vflags & EVAR_STRBUF_ALLOCATED) ? m_value.strptr.ptr : m_value.strbuf.buf;
        newstr = (x->m_vflags & EVAR_STRBUF_ALLOCATED) ? x->m_value.strptr.ptr : x->m_value.strbuf.buf;
        if (!os_strcmp(newstr, oldstr)) return;
    }

    switch (srctype)
    {
        /* If empty, clear() did the job.
         */
        case OS_UNDEFINED_TYPE:
            if (oldtype != OS_UNDEFINED_TYPE) {
                clear();
                if ((m_vflags & EVAR_NOSAVE) == 0) {
                    docallback(ECALLBACK_VARIABLE_VALUE_CHANGED);
                }
            }
            return;

        /* If this is string, check if long string has been allocated in separate
           buffer.
         */
        case OS_STR:
            clear();

            /* If separate string buffer has been allocated.
             */
            if (x->m_vflags & EVAR_STRBUF_ALLOCATED)
            {
                if (move_value)
                {
                    /* Move string value.
                     */
                    m_value.strptr.ptr = x->m_value.strptr.ptr;
                    m_value.strptr.allocated = x->m_value.strptr.allocated;
                    m_value.strptr.used = x->m_value.strptr.used;

                    /* Source variable has no value.
                     */
                    x->settype(OS_UNDEFINED_TYPE);
                    x->m_vflags &= ~EVAR_STRBUF_ALLOCATED;
                }
                else
                {
                    n = x->m_value.strptr.used;
                    m_value.strptr.ptr = os_malloc(n,
                        &m_value.strptr.allocated);
                    os_memcpy(m_value.strptr.ptr, x->m_value.strptr.ptr, n);
                    m_value.strptr.used = n;
                    m_vflags |= EVAR_STRBUF_ALLOCATED;
                }
            }
            else
            {
                n = x->m_value.strbuf.used;
                os_memcpy(m_value.strbuf.buf, x->m_value.strbuf.buf, n);
                m_value.strbuf.used = (os_uchar)n;
            }
            break;

        /* If variable contains object.
         */
        case OS_OBJECT:
            clear();

            if (move_value)
            {
                m_value.valbuf.v.o = x->m_value.valbuf.v.o;
                if (m_value.valbuf.v.o) {
                    m_value.valbuf.v.o->adopt(this, EOID_ITEM);
                }
                x->settype(OS_UNDEFINED_TYPE);
            }
            else
            {
                m_value.valbuf.v.o = x->m_value.valbuf.v.o->clone(this, EOID_ITEM);
            }

            /* Variable value object cannot be attachment. It could be processed twice.
               It also has to be clonable and serializable.
             */
            if (m_value.valbuf.v.o) {
                m_value.valbuf.v.o->clearflags(EOBJ_TEMPORARY_ATTACHMENT);
            }
            break;

        /* Other data types, just copy the value.
         */
        default:
            if (oldtype == srctype) switch (srctype)
            {
                case OS_LONG:
                    change = (m_value.valbuf.v.l != x->m_value.valbuf.v.l);
                    break;

                case OS_DOUBLE:
                    change = (m_value.valbuf.v.d != x->m_value.valbuf.v.d);
                    break;

                default:
                    break;
            }

            clear();
            m_value.valbuf.v = x->m_value.valbuf.v;
            break;
    }

    /* Set data type.
     */
    settype(srctype);

    /* Inform parent object that a variable value was changed.
     */
    if (change && (m_vflags & EVAR_NOSAVE) == 0) {
        docallback(ECALLBACK_VARIABLE_VALUE_CHANGED);
    }
}


/**
****************************************************************************************************

  @brief Set object as variable value.

  The seto() function sets an object as variable value.

  @param  x Value to set.
  @param  adopt_x If OS_FALSE, the object x is copied to be value of this variable. If OS_TRUE,
          the object x is adopted as value of the variable.

  @return None.

****************************************************************************************************
*/
void eVariable::seto(
    eObject *x,
    os_boolean adopt_x)
{
    /* Release any allocated memory.
     */
    clear();

    if (x == OS_NULL) return;

    if (adopt_x)
    {
        m_value.valbuf.v.o = x;
        x->adopt(this, EOID_ITEM /* OID value */);
    }
    else
    {
        m_value.valbuf.v.o = x->clone(this, EOID_ITEM /* OID value */);
    }

    /* Variable value object cannot be attachment. It could be processed twice.
       It also has to be clonable and serializable.
     */
    m_value.valbuf.v.o->clearflags(EOBJ_TEMPORARY_ATTACHMENT);

    /* Set data type.
     */
    settype(OS_OBJECT);

    /* Inform parent object that a variable value was changed.
     */
    if ((m_vflags & EVAR_NOSAVE) == 0) {
        docallback(ECALLBACK_VARIABLE_VALUE_CHANGED);
    }
}


/**
****************************************************************************************************

  @brief Set pointer as a value to variable.

  The setd() function sets apointer value to variable.

  @param   x Value to set.
  @return  None.

****************************************************************************************************
*/
void eVariable::setp(
    os_pointer x)
{
    /* Release any allocated memory.
     */
    clear();

    /* Save value.
     */
    m_value.valbuf.v.p = x;

    /* Set data type.
     */
    settype(OS_POINTER);
}


/**
****************************************************************************************************

  @brief Check if variable is empty.

  The isempty() function checks if variable is empty. Empty string is considered empty.
  If variable contains extended value object eValueX, it returns if extended value object
  is empty.

  @return OS_TRUE if variable is empty, OS_FALSE if not.

****************************************************************************************************
*/
os_boolean eVariable::isempty()
{
    eValueX *ex;
    os_char c;

    switch (type())
    {
        case OS_UNDEFINED_TYPE:
            return OS_TRUE;

        case OS_STR:
            if (m_vflags & EVAR_STRBUF_ALLOCATED)
            {
                c = *m_value.strptr.ptr;
            }
            else
            {
                c = m_value.strbuf.buf[0];
            }
            return (c == '\0') ? OS_TRUE : OS_FALSE;

        case OS_OBJECT:
            ex = getx();
            if (ex) {
                return ex->isempty();
            }
            break;

        default:
            break;
    }
    return OS_FALSE;
}


/**
****************************************************************************************************

  @brief Get variable value as long integer.

  The getl() function returns variable value as long integer os_long.

  @return Variable value as long integer. If variable has no value, or value cannot be converted
          to long integer, the function returns 0.

****************************************************************************************************
*/
os_long eVariable::getl()
{
    eValueX *ex;
    os_long x;

    /* Convert value to long integer.
     */
    switch (type())
    {
        case OS_LONG:
            x = m_value.valbuf.v.l;
            break;

        case OS_DOUBLE:
            if (m_value.valbuf.v.d >= 0) {
                x = (os_long)(m_value.valbuf.v.d + 0.5);
            }
            else {
                x = -(os_long)(-m_value.valbuf.v.d + 0.5);
            }
            break;

        case OS_STR:
            x = osal_str_to_int(
                (m_vflags & EVAR_STRBUF_ALLOCATED)
                ? m_value.strptr.ptr
                : m_value.strbuf.buf,
                OS_NULL);
            break;

        case OS_OBJECT:
            ex = getx();
            if (ex) {
                return ex->getl();
            }
            x = 0;
            break;

        default:
            x = 0;
            break;
    }

    /* Return the value.
     */
    return x;
}

/**
****************************************************************************************************

  @brief Get variable value as double.

  The getd() function returns variable value as double precision floating point number.

  @return  Variable value. If variable has no value, or value cannot be converted to double, the
           function returns 0.0.

****************************************************************************************************
*/
os_double eVariable::getd()
{
    eValueX *ex;
    os_double x;

    /* Convert value to double.
     */
    switch (type())
    {
        case OS_LONG:
            x = (os_double)m_value.valbuf.v.l;
            break;

        case OS_DOUBLE:
            x = m_value.valbuf.v.d;
            break;

        case OS_STR:
            if (m_vflags & EVAR_STRBUF_ALLOCATED) {
                x = osal_str_to_double(m_value.strptr.ptr, OS_NULL);
            }
            else {
                x = osal_str_to_double(m_value.strbuf.buf, OS_NULL);
            }
            break;

        case OS_OBJECT:
            ex = getx();
            if (ex) {
                return ex->getd();
            }
            x = 0;
            break;

        default:
            x = 0;
            break;
    }

    /* Return the value.
     */
    return x;
}


/**
****************************************************************************************************

  @brief Get variable value as string.

  The gets() function gets variable value as string. Integers, and floating point numbers
  are converted to string stored within temporary buffer allocated within variable.
  Temporary buffer will exists as long as variale object exit, and variable value is
  not modified.

  @param   sz Pointer where to store number of bytes in string (including terminating null
           character), OS_NULL if not needed.

  @return  Pointer to value as string. This function always returns some string, returned
           pointer is never OS_NULL.

****************************************************************************************************
*/
os_char *eVariable::gets(
    os_memsz *sz)
{
    os_char *str, *vstr, buf[64];
    eValueX *ex;
    os_memsz vsz;
    os_int di;

    switch (type())
    {
        /* If empty or object.
         */
        case OS_UNDEFINED_TYPE:
            str = (os_char*)"";
            vsz = 1;
            goto getout;

        /* If this is string, just return pointer to the string.
         */
        case OS_STR:
            /* If separate string buffer has been allocated.
             */
            if (m_vflags & EVAR_STRBUF_ALLOCATED)
            {
                str = m_value.strptr.ptr;
                vsz = m_value.strptr.used;
            }
            else
            {
                str = m_value.strbuf.buf;
                vsz = m_value.strbuf.used;
            }
            goto getout;

        /* We need to convert to string.
         */
        default:
            break;
    }

    /* If we already have converted value to string, no need to do anything.
     */
    if (m_value.valbuf.tmpstr)
    {
        str = m_value.valbuf.tmpstr;
        vsz = m_value.valbuf.tmpstr_sz;
        goto getout;
    }

    /* Convert to string.
     */
    switch (type())
    {
        case OS_LONG:
            vsz = osal_int_to_str(buf, sizeof(buf), m_value.valbuf.v.l);
            break;

        case OS_DOUBLE:
            di = digs();
            if (di == EVARP_DIGS_UNDEFINED) {
                di = EVARP_DEFAULT_DIGS;
            }
            vsz = osal_double_to_str(buf, sizeof(buf),
                m_value.valbuf.v.d, di, OSAL_FLOAT_DEFAULT);
            break;

        case OS_OBJECT:
            ex = getx();
            if (ex) {
                vstr = ex->gets(&vsz);
                str = os_malloc(vsz, OS_NULL);
                os_memcpy(str, vstr, vsz);
                m_value.valbuf.tmpstr = str;
                m_value.valbuf.tmpstr_sz = vsz;
                ex->gets_free();
                goto getout;
            }
            else {
                os_strncpy(buf, "<obj>", sizeof(buf));
                vsz = os_strlen(buf);
            }
            break;

        case OS_POINTER:
            os_strncpy(buf, "<ptr>", sizeof(buf));
            vsz = os_strlen(buf);
            break;

        default:
            str = (os_char*)"";
            vsz = 1;
            goto getout;
    }

    /* Allocate buffer for temporary string and save it.
     */
    str = os_malloc(vsz, OS_NULL);
    os_memcpy(str, buf, vsz);
    m_value.valbuf.tmpstr = str;
    m_value.valbuf.tmpstr_sz = vsz;

getout:
    if (sz) *sz = vsz;
    return str;
}


/**
****************************************************************************************************

  @brief Release memory allocated for temporary buffer by gets() function.

  The gets_free() function frees temporary string conversion buffer, if one is allocated.
  If gets() function needs to convert for example integer or double value to string, it
  allocates temprary buffer for the returned string. This buffer will be deleted when the
  variable value is modified or variable is deleted. This function may be called to release
  the temporary buffer immediately, altough this should not normally be needed.

  @return  None.

****************************************************************************************************
*/
void eVariable::gets_free()
{
    if (type() != OS_STR && m_value.valbuf.tmpstr)
    {
        os_free(m_value.valbuf.tmpstr,
            m_value.valbuf.tmpstr_sz);

        m_value.valbuf.tmpstr = OS_NULL;
    }
}



/**
****************************************************************************************************

  @brief Get pointer to object contained by variable.

  The geto() function returns pointer to object, if variable value is object.

  @return  Pointer to object, or OS_NULL if variable value is not object.

****************************************************************************************************
*/
eObject *eVariable::geto()
{
    if (type() == OS_OBJECT)
    {
        return m_value.valbuf.v.o;
    }

    return OS_NULL;
}


/**
****************************************************************************************************

  @brief Get extended value.

  The getx() function returns pointer to extended value object.

  @return Pointer to extended value object, or OS_NULL if variable value if not extended.

****************************************************************************************************
*/
eValueX *eVariable::getx()
{
    eObject *x;

    if (type() == OS_OBJECT) {
        x = m_value.valbuf.v.o;
        if (x) if (x->classid() == ECLASSID_VALUEX) {
            return (eValueX*)x;
        }
    }

    return OS_NULL;
}


/**
****************************************************************************************************

  @brief Get pointer value of variable.

  The getp() function returns pointer which was stored as variable value, if value type
  is OS_POINTER.

  @return  Pointer value, or OS_NULL if variable value is pointer or is NULL pointer.

****************************************************************************************************
*/
os_pointer eVariable::getp()
{
    if (type() == OS_POINTER)
    {
        return m_value.valbuf.v.p;
    }

    return OS_NULL;
}


/**
****************************************************************************************************

  @brief Append string value to variable.

  The appends() function appends a string to variable. After this
  call the variable always contains a string.

  @param   x Value to append. OS_NULL value is treated as empty string.
  @return  None.

****************************************************************************************************
*/
void eVariable::appends(
    const os_char *x)
{
    if (x == OS_NULL) x = "";

    appends_nbytes(x, os_strlen(x)-1);
}


/**
****************************************************************************************************

  @brief Convert integer ro string and append it to variable.

  The appendl() function appends an integer number to variable. After this
  call the variable always contains a string.

  @param   x Value to append.
  @return  None.

****************************************************************************************************
*/
void eVariable::appendl(
    os_long x)
{
    os_char nbuf[OSAL_NBUF_SZ];

    osal_int_to_str(nbuf, sizeof(nbuf), x);
    appends_nbytes(nbuf, os_strlen(nbuf)-1);
}


/**
****************************************************************************************************

  @brief Append string value to variable.

  The appendv() function appends value of another variable to variable as string. After this
  call the variable always contains a string.

  @param   x Value to append. OS_NULL value is treated as empty string.
  @return  None.

****************************************************************************************************
*/
void eVariable::appendv(
    eVariable *x)
{
    os_char
        *str;

    os_memsz
        sz;

    os_boolean
        del_tmpstr;

    if (x == OS_NULL)
    {
        appends(OS_NULL);
        return;
    }

    del_tmpstr = !x->tmpstrallocated(); // ????????????????????????????????????????????

    str = x->gets(&sz);
    appends_nbytes(str, sz-1);

    if (del_tmpstr) x->gets_free();
}


/**
****************************************************************************************************

  @brief Compare value of this variable to another variable.

  If eVariable::compare function...

  @param   x Variable to compare to. If x is OS_NULL, then the function always returns 1.

  @return  -1:this < x, 0:this == x,1:this > x.

****************************************************************************************************
*/
os_int eVariable::compare(
    eVariable *x,
    os_int flags)
{
    eVariable
        *y,
        *tmp;

    eObject
        *ox,
        *oy;

    os_int
        rval = 0,
        reverse;

    os_long
        lx,
        ly;

    os_double
        dx,
        dy;

    os_char
        nbuf[32];

    OSAL_UNUSED(flags);

    if (x == OS_NULL) {
        return 1;
    }

    /* Arrange by type id enum, so that type number of x is smaller than y's.
     */
    y = this;
    reverse = 1;
    if (x->type() > y->type())
    {
        tmp = x;
        x = y;
        y = tmp;
        reverse = -1;
    }

    switch (x->type())
    {
        case OS_LONG:
            switch (y->type())
            {
                case OS_LONG:
                    lx = x->m_value.valbuf.v.l;
                    ly = y->m_value.valbuf.v.l;
                    if (ly > lx) rval = 1;
                    if (ly < lx) rval = -1;
                    break;

                case OS_DOUBLE:
                    dx = (os_double)x->m_value.valbuf.v.l;
                    dy = y->m_value.valbuf.v.d;
                    if (dy > dx) rval = 1;
                    if (dy < dx) rval = -1;
                    break;

                case OS_STR:
                    /* If string can be converted to number, compare as numbers.
                     */
                    if (y->autotype(OS_FALSE))
                    {
                        eVariable tmpv;
                        tmpv.setv(y);
                        tmpv.autotype(OS_TRUE);
                        dx = (os_double)x->m_value.valbuf.v.l;
                        dy = tmpv.getd();
                        if (dy > dx) rval = 1;
                        if (dy < dx) rval = -1;
                    }
                    else
                    {
                        osal_int_to_str(nbuf, sizeof(nbuf), x->m_value.valbuf.v.l);
                        rval = os_strcmp(y->gets(), nbuf);
                    }
                    break;

                case OS_OBJECT:
                    rval = -1;
                    break;

                default:
                    osal_debug_error("eVariable::compare error 1");
                    break;
            }
            break;

        case OS_DOUBLE:
            switch (y->type())
            {
                case OS_DOUBLE:
                    dx = x->m_value.valbuf.v.d;
                    dy = y->m_value.valbuf.v.d;
                    if (dy > dx) rval = 1;
                    if (dy < dx) rval = -1;
                    break;

                case OS_STR:
                    /* If string can be converted to number, compare as numbers.
                     */
                    if (y->autotype(OS_FALSE))
                    {
                        eVariable tmpv;
                        tmpv.setv(y);
                        tmpv.autotype(OS_TRUE);
                        dx = x->m_value.valbuf.v.d;
                        dy = tmpv.getd();
                        if (dy > dx) rval = 1;
                        if (dy < dx) rval = -1;
                    }
                    else
                    {
                        rval = os_strcmp(y->gets(), nbuf);
                    }
                    break;

                case OS_OBJECT:
                    rval = -1;
                    break;

                default:
                    osal_debug_error("eVariable::compare error 2");
                    break;
            }
            break;

        case OS_STR:
            switch (y->type())
            {
                case OS_STR:
                    rval = os_strcmp(y->gets(), x->gets());
                    break;

                case OS_OBJECT:
                    rval = -1;
                    break;

                case OS_UNDEFINED_TYPE:
                    rval = -!x->isempty();
                    break;

                default:
                    osal_debug_error("eVariable::compare error 3");
                    break;
            }
            break;

        case OS_OBJECT:
            rval = -1;
            if (y->type() != OS_OBJECT) {
                break;
            }
            ox = x->m_value.valbuf.v.o;
            oy = y->m_value.valbuf.v.o;
            if (ox == OS_NULL || oy == OS_NULL) {
                break;
            }
            rval = oy->compare(eVariable::cast(ox));
            break;

        case OS_POINTER:
            if (y->type() == OS_POINTER)
            {
                if (y->m_value.valbuf.v.p > x->m_value.valbuf.v.p) rval = 1;
                if (y->m_value.valbuf.v.p < x->m_value.valbuf.v.p) rval = -1;
            }
            break;

        default:
        case OS_UNDEFINED_TYPE:
            rval = !y->isempty();
            break;
    }
    return reverse * rval;
}


/**
****************************************************************************************************

  @brief Automatically type variable value.

  If variable contains a string, the function checks if string is well defined integer or floating
  point number. If so, the autotype function converts variable to one.

  @param   modify_value value is set by default. If modify_value is set to OS_TRUE, this function
           will only check if variable can be automatically typed, but doesn't modify it's value.

  @return  The function returns OS_TRUE is variable value was or can be automatically typed.

****************************************************************************************************
*/
os_boolean eVariable::autotype(
    os_boolean modify_value)
{
    os_char
        *p,
        *q,
        c;

    os_double
        d;

    os_long
        l;

    os_memsz
        count;

    os_boolean
        digit_found,
        dot_found;

    /* If this variable isn't a string, do nothing.
     */
    if (type() != OS_STR) return OS_FALSE;

    /* Pointer to string content.
     */
    p = (m_vflags & EVAR_STRBUF_ALLOCATED) ? m_value.strptr.ptr : m_value.strbuf.buf;

    /* Skip spaces in beginning
     */
    while (osal_char_isspace(*p)) p++;

    /* If optional + or minus sign, save value start pointer, skip sign and following empty space.
     */
    q = p;
    if (*p == '+' || *p == '-')
    {
        p++;
        while (osal_char_isspace(*p)) p++;
    }

    /* Check the number part.
     */
    digit_found = dot_found = OS_FALSE;
    while (OS_TRUE)
    {
        c = *p;

        /* If this is decimal digit ?
         */
        if (osal_char_isdigit(c))
        {
            digit_found = OS_TRUE;
        }

        /* If this is dot ?
         */
        else if (c == '.')
        {
            /* If second dot, like in date, then do not convert.
             */
            if (dot_found) return OS_FALSE;
            dot_found = OS_TRUE;
        }

        /* If this 'e' or 'E' for exponent ?
         */
        /* else if (c == 'e' || c == 'E')
        {
        } */

        /* Otherwise end of data. This may contain only blanks
         */
        else
        {
            while (*p)
            {
                if (!osal_char_isspace(*p)) return OS_FALSE;
                p++;
            }
            break;
        }
        p++;
    }

    if (!digit_found) return OS_FALSE;
    if (dot_found)
    {
        d = osal_str_to_double(q, &count);
        if (count)
        {
            if (modify_value) setd(d);
            return OS_TRUE;
        }
    }
    else
    {
        l = osal_str_to_int(q, &count);
        if (count)
        {
            if (modify_value) setl(l);
            return OS_TRUE;
        }
    }
    return OS_FALSE;
}


/**
****************************************************************************************************

  @brief Convert variable value to string and get string pointer.

  The eVariable::tostring() function converts variable value to string (modifies variable value,
  if not already string) and returns pointer to string value.

  @return  Pointer to string variable value. Never OS_NULL.

****************************************************************************************************
*/
os_char *eVariable::tostring()
{
    /* If this variable isn't a string, convert to one.
     */
    if (type() != OS_STR)
    {
        sets(gets());
    }

    /* Return pointer to string content.
     */
    return (m_vflags & EVAR_STRBUF_ALLOCATED) ? m_value.strptr.ptr : m_value.strbuf.buf;
}


/**
****************************************************************************************************

  @brief Write variable to stream.

  The eVariable::writer() function serialized variable to stream. This writes only variable
  specific content, use eObject::write() to save also class information, attachements, etc.

  @param  stream The stream to write to.
  @param  flags Serialization flags.

  @return If successfull the function returns ESTATUS_SUCCESS (0). If writing object to stream
          fails, value ESTATUS_WRITING_OBJ_FAILED is returned. Assume that all nonzero values
          indicate an error.

****************************************************************************************************
*/
eStatus eVariable::writer(
    eStream *stream,
    os_int flags)
{
    os_memsz sz;

    /* Version number. Increment if new serialized items are to the object,
       and check for new version's items in read() function.
     */
    const os_int version = 0;

    /* Begin the object and write version number.
     */
    if (stream->write_begin_block(version)) goto failed;

    /* Write type and number of decimal digits in flags.
     */
    if (*stream << (m_vflags & EVAR_SERIALIZATION_MASK)) goto failed;

    /* Write the value, if any.
     */
    switch (type())
    {
        case OS_LONG:
            if (*stream << m_value.valbuf.v.l) goto failed;
            break;

        case OS_DOUBLE:
            if (*stream << m_value.valbuf.v.d) goto failed;
            break;

        case OS_STR:
            if (m_vflags & EVAR_STRBUF_ALLOCATED)
            {
                sz = m_value.strptr.used - 1;
                if (*stream << sz) goto failed;
                if (sz > 0) if (stream->write(m_value.strptr.ptr, sz)) goto failed;
            }
            else
            {
                sz = m_value.strbuf.used - 1;
                if (*stream << sz) goto failed;
                if (sz > 0) if (stream->write(m_value.strbuf.buf, sz)) goto failed;
            }
            break;

        case OS_OBJECT:
            if (m_value.valbuf.v.o->write(stream, flags)) goto failed;
            break;

        default:
            break;
    }

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

  @brief Read variable from stream.

  The eVariable::reader() function reads serialized variable from stream.
  This function reads only object content. To read whole object including attachments, names,
  etc, use eObject::read().

  @param  stream The stream to read from.
  @param  flags Serialization flags.

  @return If successfull the function returns ESTATUS_SUCCESS (0). If writing object to stream
          fails, value ESTATUS_READING_OBJ_FAILED is returned. Assume that all nonzero values
          indicate an error.

****************************************************************************************************
*/
eStatus eVariable::reader(
    eStream *stream,
    os_int flags)
{
    os_short vflags;
    os_int sz;

    /* Version number. Used to check which versions item's are in serialized data.
     */
    os_int version;

    /* Release any allocated memory.
     */
    clear();

    /* Read object start mark and version number.
     */
    if (stream->read_begin_block(&version)) goto failed;

    /* Read type and number of decimal digits in flags.
     */
    if (*stream >> vflags) goto failed;

    /* Read the value, if any.
     */
    switch (vflags & EVAR_TYPE_MASK)
    {
        case OS_LONG:
            if (*stream >> m_value.valbuf.v.l) goto failed;
            break;

        case OS_DOUBLE:
            if (*stream >> m_value.valbuf.v.d) goto failed;
            break;

        case OS_STR:
            if (*stream >> sz) goto failed;

            /* If string fits into small buffer, copy it and save used size.
               Leave space for '\0' character (sz doesn not include '\0').
             */
            if (sz < EVARIABLE_STRBUF_SZ)
            {
                if (sz > 0) if (stream->read(m_value.strbuf.buf, sz)) goto failed;
                m_value.strbuf.buf[sz] = '\0';
                m_value.strbuf.used = (os_uchar)sz + 1;
            }

            /* Otherwise we need to allocate buffer for long string. Allocate buffer, copy data in,
               save allocated size and used size. Set EVAR_STRBUF_ALLOCATED flag to indicate that
               buffer was allocated.
             */
            else
            {
                m_value.strptr.ptr = os_malloc(sz+1, &m_value.strptr.allocated);
                if (sz > 0) if (stream->read(m_value.strptr.ptr, sz)) goto failed;
                m_value.strptr.ptr[sz] = '\0';
                m_value.strptr.used = sz + 1;
                m_vflags |= EVAR_STRBUF_ALLOCATED;
            }
            break;

        case OS_OBJECT:
            m_value.valbuf.v.o = read(stream, flags);
            if (m_value.valbuf.v.o == OS_NULL) goto failed;
            break;

        default:
            break;
    }

    /* Store data type and decimal digits.
     */
    m_vflags &= ~EVAR_SERIALIZATION_MASK;
    m_vflags |= (vflags & EVAR_SERIALIZATION_MASK);

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

  @brief Append string to variable value, internal.

  The appends_nbytes() function appends characters from string to variable. After this
  call the variable always contains a string.

  @param   str Pointer to the string to append. If str is NULL, the function just allocates
           nchars more space for string.
  @param   nchars Number of characters to append. This should not include terminating null
           character.
  @return  None.

****************************************************************************************************
*/
void eVariable::appends_nbytes(
    const os_char *str,
    os_memsz nchars)
{
    os_char
        *val,
        *newval;

    os_memsz
        used,
        n,
        allocated;

    /* If this variable isn't a string, convert to one.
     */
    if (type() != OS_STR)
    {
        sets(gets());
    }

    /* If no separate string buffer has allocated.
     */
    if ((m_vflags & EVAR_STRBUF_ALLOCATED) == 0)
    {
        val = m_value.strbuf.buf;
        used = m_value.strbuf.used;
        n = used + nchars;

        /* If all can fit into contained buffer, append and return.
         */
        if (n <= EVARIABLE_STRBUF_SZ)
        {
            if (str) os_memcpy(val + (used - 1), str, nchars);
            val[n-1] = '\0';
            m_value.strbuf.used = (os_uchar)n;
            return;
        }
    }
    else
    {
        val = m_value.strptr.ptr;
        used = m_value.strptr.used;
        n = used + nchars;

        /* If all can fit into already allocated buffer, append and return.
         */
        if (n <= m_value.strptr.allocated)
        {
            if (str) os_memcpy(val + (used - 1), str, nchars);
            val[n-1] = '\0';
            m_value.strptr.used = n;
            return;
        }
    }

    /* Allocate and combine within new buffer.
     */
    newval = os_malloc(n, &allocated);
    os_memcpy(newval, val, used - 1);
    if (str) os_memcpy(newval + (used - 1), str, nchars);
    newval[n-1] = '\0';

    /* If we need to delete old buffer.
     */
    if (m_vflags & EVAR_STRBUF_ALLOCATED)
    {
        os_free(m_value.strptr.ptr, m_value.strptr.allocated);
    }

    /* Set pointer to buffer, buffer use and allocated bytes.
     */
    m_value.strptr.ptr = newval;
    m_value.strptr.used = n;
    m_value.strptr.allocated = allocated;

    /* Flag that we have allocated buffer.
     */
    m_vflags |= EVAR_STRBUF_ALLOCATED;
}


/**
****************************************************************************************************

  @brief Allocate buffer to hold a string.

  The allocate() function clears the variable and sets variable type to string.
  Returned pointer is pointer to string with quaranteed space for nchars characters followed
  by NULL character placed in advance.

  Retuenrd string is uninitialized and may contain garbage.

  @param   nchars Number of characters to allocate space for. This should not include terminating
           NULL character.
  @return  Pointer to buffer which can be modified, size nchars + 1 (NULL char).

****************************************************************************************************
*/
os_char *eVariable::allocate(
    os_memsz nchars)
{
    sets(OS_NULL);
    appends_nbytes(OS_NULL, nchars);
    return gets();
}


/**
****************************************************************************************************

  @brief Check if object name is oix.

  Check the object name is oix like "@401_3".
  @return OS_TRUE if object name is oix.

****************************************************************************************************
*/
os_boolean eVariable::is_oix()
{
    os_char *objname;

    objname = gets();
    return (os_boolean)(objname[0] == '@');
}


/**
****************************************************************************************************

  @brief Remove new lines from string.

  If variable contains a string, convert new line characters to spaces and remove "-\n".

****************************************************************************************************
*/
void eVariable::singleline()
{
    os_char *p, *q;
    os_memsz used;

    if (type() != OS_STR) return;

    if (m_vflags & EVAR_STRBUF_ALLOCATED) {
        p = m_value.strptr.ptr;
        used = m_value.strptr.used;
    }
    else {
        p = m_value.strbuf.buf;
        used = m_value.strbuf.used;
    }
    if (os_strchr(p, '\n') == OS_NULL) return;

    while ((q = os_strstr(p, "-\n", OSAL_STRING_DEFAULT))) {
        os_memmove(q, q + 2, used - (q - p));
        used -= 2;
    }

    while ((q = os_strchr(p, '\n'))) {
        *q = ' ';
        p = q + 1;
    }

    if (m_vflags & EVAR_STRBUF_ALLOCATED) {
        m_value.strptr.used = used;
    }
    else {
        m_value.strbuf.used = (os_uchar)used;
    }
}


/**
****************************************************************************************************

  @brief Make string lower case.

  If variable contains a string, convert all characters to lower case.
  Note: For now this works only for ASCII characters, plan is to expand the function
  for common languages.

****************************************************************************************************
*/
void eVariable::tolower()
{
    os_char *p;
    os_uchar c;

    if (type() != OS_STR) return;

    if (m_vflags & EVAR_STRBUF_ALLOCATED) {
        p = m_value.strptr.ptr;
        if (p == OS_NULL) return;
    }
    else {
        p = m_value.strbuf.buf;
    }

    while (*p != '\0')
    {
        c = (os_uchar)*p;
        if (c >= 'A' && c <= 'Z') {
            c = c - 'A' + 'a';
            *p = c;
        }
        p++;
    }
}

/**
****************************************************************************************************

  @brief Check if variable value as string starts with text given as argument.

  Check if name variable value starts with specified string, but doesn't continue with text.

****************************************************************************************************
*/
os_boolean eVariable::starts_with(
    os_char *text)
{
    os_memsz text_len;
    const os_char *p;
    os_boolean starts_with_it;

    p = gets();
    text_len = os_strlen(text) - 1;
    starts_with_it = OS_FALSE;
    if (!os_strncmp(p, text, text_len)) {
        starts_with_it = !osal_char_isalpha(p[text_len]);
    }
    return starts_with_it;
}


/**
****************************************************************************************************

  @brief Remove stuff from path what becomes unnecessary when oix is appended to it.

  If a path contains oix object name like "@401_3", the last object name is removed.
  Also other unnecessary stuff, like lone "//" may be removed.
  Is input path is terminated with '/', so is the path after function call.

  Once we implement connections, this must not remove stuff on other side of the connection.

  @return OS_TRUE if oix, etc was removed, or OS_FALSE if not.

****************************************************************************************************
*/
os_boolean eVariable::clean_to_append_oix()
{
    os_int trailing_slash;
    os_memsz vsz, e;
    os_char *path;

    if (type() != OS_STR) return OS_FALSE;

    /* If separate string buffer has been allocated.
     */
    if (m_vflags & EVAR_STRBUF_ALLOCATED)
    {
        path = m_value.strptr.ptr;
        vsz = m_value.strptr.used;
    }
    else
    {
        path = m_value.strbuf.buf;
        vsz = m_value.strbuf.used;
    }

    if (vsz <= 1) return OS_FALSE;
    e = vsz - 1;

    if (vsz == 3) if (!os_strcmp(path, "//")) {
        sets(OS_NULL);
        return OS_TRUE;
    }

    trailing_slash = 0;
    if (e >= 2 && path[e-1] == '/') {trailing_slash = 1; e--; }

    while (e >= 0) {
        if (e >= 1) {
            if (path[e-1] != '/') {
                e--;
                continue;
            }
        }

        if (path[e] == '@')
        {
            vsz = e - 1 + trailing_slash;
            if (vsz > 0) path[vsz] = '\0';
            vsz++;
            if (vsz == 3) if (!os_strcmp(path, "//")) {
                vsz = 0;
                path[vsz++] = '\0';
            }
            if (m_vflags & EVAR_STRBUF_ALLOCATED) {
                m_value.strptr.used = vsz;
            }
            else {
                m_value.strbuf.used = (os_uchar)vsz;
            }
            return OS_TRUE;
        }
        else if (path[e] == '_') {
            if (path[e+1] == 'r') {
                if (path[e+2] == '/' || path[e+2] == '\0')  {
                    return OS_FALSE;
                }
            }
        }
        e--;
    }

    return OS_FALSE;
}


/**
****************************************************************************************************

  @brief Get extended value state bits.

  If the variable holds extended value, this function returns state bits set for value. If time
  stamp is not available, the function returns 0.

  The state bits are used with IO signals, etc, to indicate if signal is connected, errornous,
  etc.

  @return Time stamp, microseconds since Epoc.

****************************************************************************************************
*/
os_int eVariable::sbits_internal()
{
    eObject *o;
    o = geto();
    if (o) if (o->classid() == ECLASSID_VALUEX) {
        return eValueX::cast(o)->sbits();
    }

    return OSAL_STATE_CONNECTED;
}


/**
****************************************************************************************************

  @brief Get extended value time stamp.

  If the variable holds extended value, this function returns time stamp of the value. If time
  stamp is not available, the function returns 0.

  @return Time stamp, microseconds since Epoc.

****************************************************************************************************
*/
os_long eVariable::tstamp_internal()
{
    eObject *o;
    o = geto();
    if (o) if (o->classid() == ECLASSID_VALUEX) {
        return eValueX::cast(o)->tstamp();
    }

    return 0;
}
