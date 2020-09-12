/**

  @file    ecomponent.cpp
  @brief   Abstract GUI component.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  Base call for all GUI components (widgets)...

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "eguilib.h"


/**
****************************************************************************************************

  @brief Constructor.

  X...

  @return  None.

****************************************************************************************************
*/
eiocCheckbox::eiocCheckbox(
    eObject *parent,
    e_oid id,
    os_int flags)
    : eObject(parent, id, flags)
{
    /* No type, number 2 digits after decimal point for doubles.
     */
    // m_vflags = OS_UNDEFINED_TYPE|(2 << EVAR_DDIGS_SHIFT);
    // m_value.valbuf.tmpstr = OS_NULL;
}


/**
****************************************************************************************************

  @brief Virtual destructor.

  X...

  @return  None.

****************************************************************************************************
*/
eiocCheckbox::~eiocCheckbox()
{
    /* Release any allocated memory.
     */
    // clear();
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
eObject *eiocCheckbox::clone(
    eObject *parent,
    e_oid id,
    os_int aflags)
{
    eiocCheckbox *clonedobj;
    clonedobj = new eiocCheckbox(parent, id == EOID_CHILD ? oid() : id, flags());

    /* Copy clonable attachments.
     */
    clonegeneric(clonedobj, aflags);
    return clonedobj;
}


/**
****************************************************************************************************

  @brief Add eiocCheckbox to class list and class'es properties to it's property set.

  The eiocCheckbox::setupclass function adds eiocCheckbox to class list and class'es properties to
  it's property set. The class list enables creating new objects dynamically by class identifier,
  which is used for serialization reader functions. The property set stores static list of
  class'es properties and metadata for those.

****************************************************************************************************
*/
void eiocCheckbox::setupclass()
{
    const os_int cls = EGUICLASSID_IOC_CHECKBOX;

    /* Add the class to class list.
     */
    os_lock();
    eclasslist_add(cls, (eNewObjFunc)newobj, "eiocCheckbox");
    eComponent::setupproperties(cls, ECOMP_NO_OPTIONAL_PROPERITES);
    propertysetdone(cls);
    os_unlock();
}


/**
****************************************************************************************************

  @brief Get next child variable identified by oid.

  The eiocCheckbox::nextv() function returns pointer to the next child object of this object.

  @param   id Object idenfifier. Default value EOID_CHILD specifies to count a child objects,
           which are not flagged as an attachment. Value EOID_ALL specifies to get count all
           child objects, regardless wether these are attachment or not. Other values
           specify object identifier, only children with that specified object identifier
           are searched for.

  @return  Pointer to the first child variable, or OS_NULL if none found.

****************************************************************************************************
*/
eiocCheckbox *eiocCheckbox::nextv(
    e_oid id)
{
    if (mm_handle == OS_NULL) return OS_NULL;
    eHandle *h = mm_handle->next(id);
    while (h)
    {
        if (h->object()->classid() == EGUICLASSID_IOC_CHECKBOX)
            return eiocCheckbox::cast(h->object());

        h = h->next(id);
    }
    return OS_NULL;
}


/* Draw the component.
 */
void eiocCheckbox::draw(
    eDrawParams& prm)
{
    ImGui::Text("HERREE.");
}

