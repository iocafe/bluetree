/**

  @file    ewindow.cpp
  @brief   GUI window class.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    15.9.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "egui.h"


/**
****************************************************************************************************

  @brief Constructor.

  X...

  @return  None.

****************************************************************************************************
*/
eWindow::eWindow(
    eObject *parent,
    e_oid id,
    os_int flags)
    : eComponent(parent, id, flags)
{
    addname("window", ENAME_TEMPORARY, "gui");
    m_autolabel_count = 0;
}


/**
****************************************************************************************************

  @brief Virtual destructor.

  X...

  @return  None.

****************************************************************************************************
*/
eWindow::~eWindow()
{
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
eObject *eWindow::clone(
    eObject *parent,
    e_oid id,
    os_int aflags)
{
    eWindow *clonedobj;
    clonedobj = new eWindow(parent, id == EOID_CHILD ? oid() : id, flags());
    clonegeneric(clonedobj, aflags);
    return clonedobj;
}


/**
****************************************************************************************************

  @brief Add eWindow to class list and class'es properties to it's property set.

  The eWindow::setupclass function adds eWindow to class list and class'es properties to
  it's property set. The class list enables creating new objects dynamically by class identifier,
  which is used for serialization reader functions. The property set stores static list of
  class'es properties and metadata for those.

****************************************************************************************************
*/
void eWindow::setupclass()
{
    const os_int cls = EGUICLASSID_WINDOW;

    os_lock();
    eclasslist_add(cls, (eNewObjFunc)newobj, "eWindow");
    addproperty(cls, ECOMP_VALUE, ecomp_value, EPRO_DEFAULT, "title");
    propertysetdone(cls);
    os_unlock();
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
eStatus eWindow::onpropertychange(
    os_int propertynr,
    eVariable *x,
    os_int flags)
{
    switch (propertynr)
    {
        case ECOMP_VALUE: /* clear label to display new text and proceed */
            m_label_title.clear();
            break;
    }

    return eComponent::onpropertychange(propertynr, x, flags);
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
eStatus eWindow::simpleproperty(
    os_int propertynr,
    eVariable *x)
{
    return eComponent::simpleproperty(propertynr, x);
}


/**
****************************************************************************************************

  @brief Generating ImGui autolabel.

  The eWindow::make_autolabel generates unique nonzero numbers for ImGui labels.

  @return Unique number (we wish).

****************************************************************************************************
*/
os_long eWindow::make_autolabel()
{
    while (++m_autolabel_count == 0);
    return m_autolabel_count;
}


/**
****************************************************************************************************

  @brief Draw the component.

  The eWindow::draw() function calls ImGui API to render the component.

  @param   Prm Drawing parameters.
  @return  The function return ESTATUS_SUCCESS if all is fine. Other values indicate that the
           component is no longer drawable or useful. This could be for example a pop up menu
           closed implicitely by clicking elsewhere.

****************************************************************************************************
*/
eStatus eWindow::draw(
    eDrawParams& prm)
{
    eDrawParams childprm;
    eComponent *c;
    const os_char *label;

    label = m_label_title.get(this, ECOMP_VALUE);

    ImGui::Begin(label);                          // Create a window called "Hello, world!" and append into it.

    childprm = prm;
    if (!ImGui::IsWindowHovered()) {
        childprm.right_click = false;
    }

    for (c = firstcomponent(EOID_GUI_COMPONENT); c; c = c->nextcomponent(EOID_GUI_COMPONENT))
    {
        c->draw(childprm);
    }
    ImGui::End();
    return ESTATUS_SUCCESS;
}


/**
****************************************************************************************************

  @brief Add eTreeNode to class list and class'es properties to it's property set.

  The eObject::object_info function fills in item (eVariable) to contain information
  about this object in tree browser view.

  @param   item Pointer to eVariable to set up with object information.
  @param   name Object's name if known. OS_NULL if object is not named or name is
           unknown at this time.
  @param   appendix Pointer to eSet into which to store property flags. The stored property
           flags indicate if object has namespace, children, or properties.

****************************************************************************************************
*/
void eWindow::object_info(
    eVariable *item,
    eVariable *name,
    eSet *appendix)
{
    eVariable tmp;
    eObject::object_info(item, name, appendix);

    propertyv(ECOMP_VALUE, &tmp);
    item->setv(&tmp, OS_TRUE);
}
