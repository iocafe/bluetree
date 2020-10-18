/**

  @file    erowset.h
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
#ifndef EROWSET_H_
#define EROWSET_H_
#include "eobjects.h"

/**
****************************************************************************************************
  Defines
****************************************************************************************************
*/

/* Row set property numbers.
 */
#define ERSETP_NROWS 21
#define ERSETP_NCOLUMNS 22
#define ERSETP_CONFIGURATION ETABLEP_CONFIGURATION

/* Matrix property names.
 */
extern const os_char
    ersetp_nrows[],
    ersetp_ncolumns[];

#define ersetp_configuration etablep_configuration


/**
****************************************************************************************************
  RowSet class.
****************************************************************************************************
*/
class eRowSet : public eTable
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
    eRowSet(
        eObject *parent = OS_NULL,
        e_oid id = EOID_RITEM,
        os_int flags = EOBJ_DEFAULT);

    /* Virtual destructor.
     */
    virtual ~eRowSet();

    /* Clone object.
     */
    virtual eObject *clone(
        eObject *parent,
        e_oid id = EOID_CHILD,
        os_int aflags = 0);

    /* Casting eObject pointer to eRowSet pointer.
     */
    inline static eRowSet *cast(
        eObject *o)
    {
        e_assert_type(o, ECLASSID_ROWSET)
        return (eRowSet*)o;
    }

    /* Get class identifier.
     */
    virtual os_int classid() {return ECLASSID_ROWSET; }

    /* Static function to add class to propertysets and class list.
     */
    static void setupclass();

    /* Static constructor function for generating instance by class list.
     */
    static eRowSet *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new eRowSet(parent, id, flags);
    }

    /* Called when property value changes.
     */
    virtual eStatus onpropertychange(
        os_int propertynr,
        eVariable *x,
        os_int flags);

    /* Get value of simple property.
     */
    virtual eStatus simpleproperty(
        os_int propertynr,
        eVariable *x);

#if E_SUPPROT_JSON
    /* Called to check if object has class specific content. If there is no class
       specific JSON content, json_writer or json_reader should not be called.
     */
    virtual os_boolean has_json_content()
    {
        return OS_TRUE;
    }

    /* Write rowset specific content to stream as JSON.
     */
    virtual eStatus json_writer(
        eStream *stream,
        os_int sflags,
        os_int indent);
#endif

    /*@}*/

    /**
    ************************************************************************************************
      Overloaded table interface functions.
    ************************************************************************************************
    */
    /*@{*/

    /* Insert rows into table.
     * Row can be one row or container with multiple rows.
     */
    virtual void insert(
        eContainer *rows,
        os_int tflags = 0);

    /* Update a row or rows of a table.
     */
    virtual eStatus update(
        const os_char *whereclause,
        eContainer *row,
        os_int tflags = 0);

    /* Remove rows from table.
     */
    virtual void remove(
        const os_char *whereclause,
        os_int tflags = 0);

    /* Select rows from table.
     */
    virtual eStatus select(
        const os_char *whereclause,
        eContainer *columns,
        eSelectParameters *prm,
        os_int tflags = 0);
    /*@}*/


    /**
    ************************************************************************************************

      @name RowSet functions.

      These function treat rowset as organized to rows and columns.

    ************************************************************************************************
    */
    /*@{*/

    /* Set path to DBM.
     */
    eStatus set_dbm(
        const os_char *dbm_path);

    /* Set table name.
     */
    eStatus set_table(
        const os_char *table_name);

    /* Set page mode.
     */
    eStatus set_page_mode(
        os_int page_mode);

    /* Set row mode.
     */
    eStatus set_row_mode(
        os_int row_mode);

    /* Set row mode.
     */
    eStatus set_callback(
        os_int func,
        eObject *context);

    /* Select rows from table.
     */
    eStatus select(
        const os_char *whereclause,
        eContainer *columns,
        os_int limit = 0);

    /* Get rowset width.
     */
    inline os_int ncolumns() {return m_ncolumns; }

    /* Get rowset height.
     */
    inline os_int nrows() {return m_nrows; }

    /*@}*/


protected:
    /**
    ************************************************************************************************

      @name Internal to rowset.

      Protected functions and member variables.

    ************************************************************************************************
    */
    /*@{*/

    /** Number of rows.
     */
    os_int m_nrows;

    /** Number of columns.
     */
    os_int m_ncolumns;

    /** RowSet columns configuration, column list, OS_NULL if not set.
        Index column is always first column on list.
     */
    eContainer *m_columns;

    /** Own change, prevent recursion.
     */
    os_short m_own_change;

    /*@}*/
};

#endif
