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
        e_oid id = EOID_ITEM,
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

    /**
    ************************************************************************************************
      eDBM function
    ************************************************************************************************
    */
    /* Get pointer to to list of columns needed for trigger. Holds named eVariable
       for each column.
     */
    inline eContainer *trigger_columns() {return m_trigger_columns; }

    /** Get index range minimum and maximum. Functions return OS_LONG_MIN and OS_LONG_MAX
        respectively if index range is not set.
     */
    inline os_long minix() {return m_minix;}
    inline os_long maxix() {return m_maxix;}

    /* Trigger remove row (append to trig data to send to row set)
     */
    void trigdata_append_remove(
        os_long ix_value);

    /* Trigger insert or update row (append to trig data to send to row set)
     */
    void trigdata_append_insert_or_update(
        os_long ix_value);

protected:
    /**
    ************************************************************************************************
      Internal functions and functions shared called from eRowSetBinding
    ************************************************************************************************
    */
    /* Forward select sent early to binding.
     */
    void forward_select_message_to_binding(
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

    /* Generate merged trigged data based on all server side row set bindings.
     */
    void generate_trigger_data();

    /* Clear trigged "remove row" and "insert/update row" data in bindings.
     */
    void trigdata_clear();

    /* Send trigged "remove row" and "insert/update row" data to bindings.
     */
    void trigdata_send();

    /**
    ************************************************************************************************
      Member variables
    ************************************************************************************************
    */

    /** Trigger data: List of all columns used in any server side eRowSetBinding objects.
       Includes both columns from where clause and columns list;
     */
    eContainer *m_trigger_columns;

    /** Trigger data: Maximum index range from all selections, OS_LONG_MIN, OS_LONG_MAX if
        not limited.
     */
    os_long m_minix, m_maxix;

};

#endif
