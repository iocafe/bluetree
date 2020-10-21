/**

  @file    erowsetbinding.h
  @brief   Binding row set to table.
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
#ifndef EROWSETBINDING_H_
#define EROWSETBINDING_H_
#include "eobjects.h"

struct eSelectParameters;


/* Enumeration of binding parameters. Flags must be parameter 1 and match with EPR_BINDING_FLAGS.
 */
typedef enum eRsetBindingParamEnum {
    ERSET_BINDING_FLAGS = EPR_BINDING_FLAGS,
    ERSET_BINDING_WHERE_CLAUSE,
    ERSET_BINDING_REQUESTED_COLUMNS,
    ERSET_BINDING_FINAL_COLUMNS,
    ERSET_BINDING_TABLE_NAME,
    ERSET_BINDING_LIMIT,
    ERSET_BINDING_PAGE_MODE,
    ERSET_BINDING_ROW_MODE,
    ERSET_BINDING_TZONE,
}
eRsetBindingParamEnum;


/**
****************************************************************************************************

  @brief Table binding class.

  The eRowSetBinding is class derived from eBinding. It implements property binding specific
  functionality.


****************************************************************************************************
*/
class eRowSetBinding : public eBinding
{
    /**
    ************************************************************************************************

      @name Generic object functionality.

      These functions enable using objects of this class as generic eObjects.

    ************************************************************************************************
    */
    /*@{*/
public:
    /* Constructor.
     */
    eRowSetBinding(
        eObject *parent = OS_NULL,
        e_oid id = EOID_RITEM,
        os_int flags = EOBJ_DEFAULT);

    /* Virtual destructor.
     */
    virtual ~eRowSetBinding();

    /* Clone object.
     */
/*     virtual eObject *clone(
        eObject *parent,
        e_oid id = EOID_CHILD,
        os_int aflags = 0); */

    /* Casting eObject pointer to eRowSetBinding pointer.
     */
    inline static eRowSetBinding *cast(
        eObject *o)
    {
        e_assert_type(o, ECLASSID_ROW_SET_BINDING)
        return (eRowSetBinding*)o;
    }

    /* Get class identifier.
     */
    virtual os_int classid() {return ECLASSID_ROW_SET_BINDING; }

    /* Static function to add class to propertysets and class list.
     */
    static void setupclass();

    /* Static constructor function for generating instance by class list.
     */
    static eRowSetBinding *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new eRowSetBinding(parent, id, flags);
    }

    /* Get value of simple property.
     */
    virtual eStatus simpleproperty(
        os_int propertynr,
        eVariable *x);

    /* Process received messages
     */
    virtual void onmessage(
        eEnvelope *envelope);

    /*@}*/


    /**
    ************************************************************************************************

      @name Property binding functions

      These functions implement property finding functionality.

    ************************************************************************************************
    */
    /*@{*/

    /* Bind row set to table (select).
     */
    void bind(
        eVariable *dbm_path,
        const os_char *whereclause,
        eContainer *columns,
        eSelectParameters *prm,
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

    /* Store select parameters as eSet.
     */
    void prm_struct_to_set(
        const os_char *whereclause,
        eContainer *columns,
        eSelectParameters *prm,
        os_int bflags);

     void prm_set_to_struct();

    /*@}*/

    /**
    ************************************************************************************************

      @name Member variables.

      The member variables hold information where to bind (for client binding) and current
      binding state.

    ************************************************************************************************
    */
    /*@{*/

    /* Select parameters as set.
     */
    eSet *m_pset;

    /* Select parameters as structure, pointers to values within m_pset. Set by prm_set_to_struct().
     */
    eSelectParameters m_pstruct;

    /* Requested columns, pointer to eContainer within m_pset. Set by prm_set_to_struct().
     */
    eContainer *m_requested_columns;

    /* Where clause, pointer to within m_pset. Set by prm_set_to_struct().
     */
    const os_char *m_where_clause;

    /*@}*/

};

#endif
