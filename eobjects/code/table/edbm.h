/**

  @file    edbm.h
  @brief   Rowset class.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    15.10.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef EDBM_H_
#define EDBM_H_
#include "eobjects.h"

/**
****************************************************************************************************
  Defines
****************************************************************************************************
*/



/**
****************************************************************************************************
  DBM class.
****************************************************************************************************
*/
class eDBM : public eObject
{
    friend class eRowSetBinding;

    /**
    ************************************************************************************************
      Generic eObject functionality.
    ************************************************************************************************
    */
public:
    /* Constructor.
     */
    eDBM(
        eObject *parent = OS_NULL,
        e_oid id = EOID_RITEM,
        os_int flags = EOBJ_DEFAULT);

    /* Virtual destructor.
     */
    virtual ~eDBM();

    /* Clone object.
     */
    virtual eObject *clone(
        eObject *parent,
        e_oid id = EOID_CHILD,
        os_int aflags = 0);

    /* Casting eObject pointer to eDBM pointer.
     */
    inline static eDBM *cast(
        eObject *o)
    {
        e_assert_type(o, ECLASSID_DBM)
        return (eDBM*)o;
    }

    /* Get class identifier.
     */
    virtual os_int classid() {return ECLASSID_DBM; }

    /* Static function to add class to propertysets and class list.
     */
    static void setupclass();

    /* Static constructor function for generating instance by class list.
     */
    static eDBM *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new eDBM(parent, id, flags);
    }

    /* Function to process incoming messages.
     */
    virtual void onmessage(
        eEnvelope *envelope);


protected:
    /**
    ************************************************************************************************
      Internal to DBM.
    ************************************************************************************************
    */
    /* Forward select sent early to binding.
     */
    void rset_select(
        eEnvelope *envelope);

    /* Get tflags from message content.
     */
    os_int get_tflags(
        eObject *content);

    /* Configure a table.
     */
    void configure(
        eContainer *configuration,
        os_int tflags);

    /* Insert rows into table.
     */
    void insert(
        eVariable *table_name,
        eContainer *rows,
        os_int tflags);

    /* Update a row or rows of a table.
     */
    void update(
        eVariable *table_name,
        eVariable *where_clause,
        eContainer *row,
        os_int tflags);

    /* Remove rows from the table.
     */
    void remove(
        eVariable *table_name,
        eVariable *where_clause,
        os_int tflags);

    /* Solve column wildcards, requested_columns -> resolved_columns.
     */
    void solve_table_configuration(
        eContainer *resolved_configuration,
        eContainer *requested_columns,
        eVariable *table_name);

    /* Select data from table.
     */
    eStatus select(
        const os_char *where_clause,
        eContainer *columns,
        eSelectParameters *prm,
        os_int tflags);

    /* Get pointer to table object derived from eTable.
     */
    eTable *get_table(
        eVariable *table_name);
};

#endif
