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
class eDBM;

/**
****************************************************************************************************
  Defines
****************************************************************************************************
*/

/** Select callback function.
 */
typedef eStatus etable_select_callback(
    eTable *t,
    eMatrix *data,
    eObject *context);

/** Parameters for select function.
 */
typedef struct eSelectParameters {
    /** Pointer to callback function which will receive the data. The callback function may be
        called multiple times to receive data as matrices with N data rows in each. N is chosen
        for efficiency.
     */
    etable_select_callback *callback;

    /** Application specific context pointer to pass to callback function.
     */
    eObject *context;

    /** Table name
     */
    eVariable *table_name;

    /** Maximum number of rows to return with select. Positive number => return N first rows.
        Negative number => return N last rows.
     */
    os_int limit;

    os_int page_mode;
    os_int row_mode;
    eObject *tzone;
}
eSelectParameters;


/* tflags - table flags */
#define ETABLE_ADOPT_ARGUMENT         0x10000000
#define ETABLE_INSERT_OR_UPDATE       0x00008000
#define ETABLE_SERIALIZED_FLAGS_MASK  0x0000FFFF


/**
****************************************************************************************************
  The eTable is base class for tables. Underlying table can be eMatrix, eSelection or database
  table class.
****************************************************************************************************
*/
class eTable : public eObject
{
public:
    /**
    ************************************************************************************************
      Generic eObject functionality.
    ************************************************************************************************
    */
    /* Constructor.
     */
    eTable(
        eObject *parent = OS_NULL,
        e_oid id = EOID_ITEM,
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


    /**
    ************************************************************************************************
      Table functions.
    ************************************************************************************************
    */
    /* Configure the table.
     */
    virtual void configure(
        eContainer *configuration,
        os_int tflags = 0)
    {
        OSAL_UNUSED(configuration);
        OSAL_UNUSED(tflags);
        osal_debug_error("eTable::configure is not overloaded");
    }

    /* Get pointer to table configuration (eContainer).
     */
    virtual eContainer *configuration() {return OS_NULL;}

    /* Insert rows into table.
     * Row can be one row or container with multiple rows.
     */
    virtual void insert(
        eContainer *rows,
        os_int tflags = 0,
        eDBM *dbm = OS_NULL)
    {
        OSAL_UNUSED(rows);
        OSAL_UNUSED(tflags);
        OSAL_UNUSED(dbm);
        osal_debug_error("eTable::insert is not overloaded");
    }

    /* Update a row or rows of a table or insert a row to the table.
     */
    virtual eStatus update(
        const os_char *where_clause,
        eContainer *row,
        os_int tflags = 0,
        eDBM *dbm = OS_NULL)
    {
        OSAL_UNUSED(where_clause);
        OSAL_UNUSED(row);
        OSAL_UNUSED(tflags);
        OSAL_UNUSED(dbm);
        osal_debug_error("eTable::update is not overloaded");
        return ESTATUS_FAILED;
    }

    /* Remove rows from table.
     */
    virtual void remove(
        const os_char *where_clause,
        os_int tflags = 0,
        eDBM *dbm = OS_NULL)
    {
        OSAL_UNUSED(where_clause);
        OSAL_UNUSED(tflags);
        OSAL_UNUSED(dbm);
        osal_debug_error("eTable::remove is not overloaded");
    }

    /* Select rows from table.
     */
    virtual eStatus select(
        const os_char *where_clause,
        eContainer *cols,
        eSelectParameters *prm,
        os_int tflags = 0)
    {
        OSAL_UNUSED(where_clause);
        OSAL_UNUSED(cols);
        OSAL_UNUSED(prm);
        OSAL_UNUSED(tflags);
        osal_debug_error("eTable::select is not overloaded");
        return ESTATUS_FAILED;
    }

    /* Get pointer to name of index column (helper function). This needs to be overload
       by implementing class.
     */
    virtual eName *find_index_column_name() {return OS_NULL; }

    /* Find index column eVariable from row to insert (helper function).
     */
    eVariable *find_index_element(
        eContainer *row);

    /**
    ************************************************************************************************
      Helper function to implement table.
    ************************************************************************************************
    */
    eContainer *process_configuration(
        eContainer *configuration,
        os_int *nro_columns,
        os_int tflags);

    eWhere *set_where(
        const os_char *where_clause);

    /* eWhere *get_where(); */
};

#endif
