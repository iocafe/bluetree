/**

  @file    epropertybinding.h
  @brief   Bind a property to an another object.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef EPROPERTYBINDING_H_
#define EPROPERTYBINDING_H_
#include "eobjects.h"

/* Enumeration of binding parameters. Flags must be parameter 1 and match with ERSET_BINDING_FLAGS.
 */
typedef enum ePrBindingParamEnum {
    EPR_BINDING_FLAGS = 1,
    EPR_BINDING_PROPERTYNAME,
    EPR_BINDING_VALUE,
    EPR_BINDING_META_PR_NAMES,

    EPR_BINDING_META_PR_VALUES /* must be last */
}
ePrBindingParamEnum;

/**
****************************************************************************************************

  @brief Property binding class.

  The ePropertyBinding is class derived from eBinding. It implements property binding specific
  functionality.


****************************************************************************************************
*/
class ePropertyBinding : public eBinding
{
public:
    /**
    ************************************************************************************************
      Generic eObject functionality.
    ************************************************************************************************
    */
    /* Constructor.
     */
    ePropertyBinding(
        eObject *parent = OS_NULL,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT);

    /* Virtual destructor.
     */
    virtual ~ePropertyBinding();

    /* Clone object.
     */
    virtual eObject *clone(
        eObject *parent,
        e_oid id = EOID_CHILD,
        os_int aflags = 0);

    /* Casting eObject pointer to ePropertyBinding pointer.
     */
    inline static ePropertyBinding *cast(
        eObject *o)
    {
        e_assert_type(o, ECLASSID_PROPERTY_BINDING)
        return (ePropertyBinding*)o;
    }

    /* Get class identifier.
     */
    virtual os_int classid() {return ECLASSID_PROPERTY_BINDING; }

    /* Static function to add class to propertysets and class list.
     */
    static void setupclass();

    /* Static constructor function for generating instance by class list.
     */
    static ePropertyBinding *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new ePropertyBinding(parent, id, flags);
    }

    /* Write propertybinding content to stream.
     */
    virtual eStatus writer(
        eStream *stream,
        os_int flags);

    /* Read propertybinding content from stream.
     */
    virtual eStatus reader(
        eStream *stream,
        os_int flags);

    /* Process received messages
     */
    virtual void onmessage(
        eEnvelope *envelope);

    /* Get property nr.
     */
    /* inline os_int bound_propertynr() {return m_localpropertynr;} */



    /**
    ************************************************************************************************
      Property binding functions
    ************************************************************************************************
    */

    /* Get the next property binding identified by oid.
     */
    ePropertyBinding *nextpb(
        e_oid id = EOID_CHILD);

    /* Bind property.
     */
    void bind(
        os_int localpropertynr,
        const os_char *remotepath,
        const os_char *remoteproperty,
        os_int bflags);

    void bind2(
        const os_char *remotepath);

    /* Bind the server end.
     */
    virtual void srvbind(
        eObject *obj,
        eEnvelope *envelope);

    /* Mark property value changed (may forward it immediately).
     */
    void changed(
        os_int propertynr,
        eVariable *x,
        os_boolean delete_x);

protected:

    /* Finish the client end of binding.
     */
    void cbindok(
        eObject *obj,
        eEnvelope *envelope);

    /* Virtual function to forward property value trough binding.
     */
    virtual void forward(
        eVariable *x = OS_NULL,
        os_boolean delete_x = OS_FALSE);

    /* Update to property value has been received.
     */
    void update(
        eEnvelope *envelope);

    void sendack(
        eEnvelope *envelope);

    void ack(
        eEnvelope *envelope);

    /* Set object's property from binding.
     */
    os_boolean binding_setproperty(
        eVariable *x);

    /* Get object's property value from binding.
     */
    os_boolean binding_getproperty(
        eVariable *x);

    /* List attributes (subproperties like "x.min") for the property.
     */
    os_boolean list_meta_pr_names(
        os_int propertynr,
        eVariable *x);

    void get_meta_pr_values(
        eObject *obj,
        const os_char *propertyname,
        const os_char *metadata_pr_list,
        eSet *reply);

    /* Set metadata to properties at client end of binding.
     */
    void set_meta_pr_values(
        eSet *reply);

    /* Save property name.
     */
    void set_propertyname(
        const os_char *propertyname);


    /**
    ************************************************************************************************

      @name Member variables.

      The member variables hold information where to bind (for client binding) and current
      binding state.

    ************************************************************************************************
    */

    /** Client: property name on remote object to bind to.
        Server: Always OS_NULL.
     */
    os_char *m_propertyname;

    /** Size of property name allocation in bytes.
     */
    os_short m_propertynamesz;

    /** Which property of local object is bound.
     */
    os_int m_localpropertynr;
};

#endif
