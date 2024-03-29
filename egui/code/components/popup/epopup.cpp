/**

  @file    ewindow.cpp
  @brief   GUI window class.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "egui.h"


/**
****************************************************************************************************
  Constructor.
****************************************************************************************************
*/
ePopup::ePopup(
    eObject *parent,
    e_oid id,
    os_int flags)
    : eComponent(parent, id, flags)
{
    m_autolabel_count = 0;
    m_open_popup_called = OS_FALSE;
}


/**
****************************************************************************************************
  Virtual destructor.
****************************************************************************************************
*/
ePopup::~ePopup()
{
    m_label_title.release(this);
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
eObject *ePopup::clone(
    eObject *parent,
    e_oid id,
    os_int aflags)
{
    ePopup *clonedobj;
    clonedobj = new ePopup(parent, id == EOID_CHILD ? oid() : id, flags());
    clonegeneric(clonedobj, aflags);
    return clonedobj;
}


/**
****************************************************************************************************

  @brief Add ePopup to class list and class'es properties to it's property set.

  The ePopup::setupclass function adds ePopup to class list and class'es properties to
  it's property set. The class list enables creating new objects dynamically by class identifier,
  which is used for serialization reader functions. The property set stores static list of
  class'es properties and metadata for those.

****************************************************************************************************
*/
void ePopup::setupclass()
{
    const os_int cls = EGUICLASSID_POPUP;

    os_lock();
    eclasslist_add(cls, (eNewObjFunc)newobj, "ePopup", EGUICLASSID_COMPONENT);
    setupproperties(cls, ECOMP_NO_OPTIONAL_PROPERITES);
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
eStatus ePopup::onpropertychange(
    os_int propertynr,
    eVariable *x,
    os_int flags)
{
    switch (propertynr)
    {
        case ECOMP_TEXT:
            m_label_title.clear();
            break;

        default:
            return eComponent::onpropertychange(propertynr, x, flags);
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
eStatus ePopup::simpleproperty(
    os_int propertynr,
    eVariable *x)
{
    return eComponent::simpleproperty(propertynr, x);
}


/**
****************************************************************************************************

  @brief Generating ImGui autolabel.

  The ePopup::make_autolabel generates unique nonzero numbers for ImGui labels.

  @return Unique number (we wish).

****************************************************************************************************
*/
os_long ePopup::make_autolabel()
{
    while (++m_autolabel_count == 0);
    return m_autolabel_count;
}


/**
****************************************************************************************************

  @brief Draw the component.

  The ePopup::draw() function calls ImGui API to render the component.

  @param   Prm Drawing parameters.
  @return  The function return ESTATUS_SUCCESS if all is fine. Other values indicate that the
           component is no longer drawable or useful. This could be for example a pop up menu
           closed implicitely by clicking elsewhere.

****************************************************************************************************
*/
eStatus ePopup::draw(
    eDrawParams& prm)
{
    eComponent *c;
    const os_char *label;
    eDrawParams wprm;

    wprm = prm;
    wprm.layer++;

    add_to_zorder(wprm.window, wprm.layer);

    label = m_label_title.get(this);

    if (!m_open_popup_called)
    {
        ImGui::OpenPopup(label);
        m_open_popup_called = OS_TRUE;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 8.0f));

    if (ImGui::BeginPopup(label))
    {
        for (c = firstcomponent();
             c;
             c = c->nextcomponent())
        {
            c->draw(wprm);
        }

        ImGui::EndPopup();
        ImGui::PopStyleVar();

        return ESTATUS_SUCCESS;
    }

   ImGui::PopStyleVar();

    return ESTATUS_FAILED;
}

