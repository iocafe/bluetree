/**

  @file    etable.h
  @brief   Table interface.
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
#ifndef ETABLE_H_
#define ETABLE_H_
#include "eobjects.h"

class eTable;
class eMatrix;

/**
****************************************************************************************************
  Defines
****************************************************************************************************
*/
/*@{*/

/* Table property numbers.
 */
#define ETABLEP_CONFIGURATION 30

/* Table property names.
 */
extern const os_char
    etablep_configuration[];

typedef void etable_select_callback(eTable *t, eMatrix *data);


/**
****************************************************************************************************

  @brief Table interface.

  The eTable is base class for tables. Underlying table can be eMatrix, eSelection or database
  table class.


****************************************************************************************************
*/
class eTable : public eObject
{
public:
    /**
    ************************************************************************************************

      @name Generic object functionality.

      These functions enable using objects of this class as generic eObjects.

    ************************************************************************************************
    */
    /*@{*/

    /* Constructor.
     */
    eTable(
        eObject *parent = OS_NULL,
        e_oid id = EOID_RITEM,
        os_int flags = EOBJ_DEFAULT);

    /* Virtual destructor.
     */
    virtual ~eTable();

    /* Casting eObject pointer to eTable pointer.
     */
    inline static eTable *cast(
        eObject *o)
    {
        /* e_assert_type(o, ECLASSID_TABLE) */
        return (eTable*)o;
    }

    /* Get class identifier.
     */
    virtual os_int classid() {return ECLASSID_TABLE; }

    /* Static function to add class to propertysets and class list.
     */
    static void setupclass();

    /* Static constructor function for generating instance by class list.
     */
    static eTable *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new eTable(parent, id, flags);
    }

    /*@}*/

    /**
    ************************************************************************************************
      Table functions.
    ************************************************************************************************
    */
    /*@{*/

    /* Configure the table.
     */
    virtual void configure(
        eContainer *configuration,
        os_int tflags = 0)
    {
        osal_debug_error("eTable::configure is not overloaded");
    }

    /* Insert rows into table.
     * Row can be one row or container with multiple rows.
     */
    virtual void insert(
        eContainer *rows,
        os_int tflags = 0)
    {
        osal_debug_error("eTable::insert is not overloaded");
    }

    /* Update a row or rows of a table or insert a row to the table.
     */
    virtual eStatus update(
        const os_char *whereclause,
        eContainer *row,
        os_int tflags = 0)
    {
        osal_debug_error("eTable::update is not overloaded");
        return ESTATUS_FAILED;
    }

    /* Remove rows from table.
     */
    virtual void remove(
        const os_char *whereclause,
        os_int tflags = 0)
    {
        osal_debug_error("eTable::remove is not overloaded");
    }

    /* Select rows from table.
     */
    virtual eStatus select(
        const os_char *whereclause,
        etable_select_callback *callback,
        os_int tflags = 0)
    {
        osal_debug_error("eTable::select is not overloaded");
        return ESTATUS_FAILED;
    }

    /*@}*/

    /**
    ************************************************************************************************
      Helper function to implement table.
    ************************************************************************************************
    */
    eContainer *process_configuration(
        eContainer *configuration,
        os_int *nro_columns);

    eWhere *set_where(
        const os_char *whereclause);

    /* eWhere *get_where(); */
};

#endif
