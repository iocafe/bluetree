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

class eRowSetBinding;
class eRowSet;
class eDBM;

/**
****************************************************************************************************
  Defines, etc.
****************************************************************************************************
*/

/* eRowSet property numbers. These are used also for eRowSetBinding.
   Make sure that EBINDP_BFLAGS and EBINDP_STATE
   do not overlap with these numbers.
 */
#define ERSETP_NROWS 18
#define ERSETP_NCOLUMNS 19
#define ERSETP_DBM_PATH 20
#define ERSETP_TABLE_NAME 21
#define ERSETP_WHERE_CLAUSE 22
#define ERSETP_REQUESTED_COLUMNS 23
#define ERSETP_LIMIT 24
#define ERSETP_PAGE_MODE 25
#define ERSETP_ROW_MODE 26
#define ERSETP_TZONE 27
#define ERSETP_HAS_CALLBACK 28
#define ERSETP_IX_COLUMN_NAME 29
#define ERSETP_CONFIGURATION 30


/* Matrix property names.
 */
extern const os_char
    ersetp_nrows[],
    ersetp_ncolumns[],
    ersetp_dbm_path[],
    ersetp_table_name[],
    ersetp_where_clause[],
    ersetp_requested_columns[],
    ersetp_limit[],
    ersetp_page_mode[],
    ersetp_row_mode[],
    ersetp_tzone[],
    ersetp_has_callback[],
    ersetp_ix_column_name[],
    ersetp_configuration[];


/* Enumeration of row set callback reasons.
 */
typedef enum ersetCallbackReason
{
    ERSET_TABLE_BINDING_COMPLETE,
    ERSET_INITIAL_DATA_RECEIVED,
    ERSET_MODIFICATIONS_RECEIVED
    /* ERSET_INSERT,
    ERSET_UPDATE,
    ERSET_REMOVE */
}
ersetCallbackEvent;

/* Row set callback information.
 */
typedef struct ersetCallbackInfo
{
    ersetCallbackEvent event;

    os_long first_ix, last_ix;
    os_boolean sortorder_changed;
}
ersetCallbackInfo;

/* Row set callback function
 */
typedef void erset_callback_func(
    eRowSet *rset,
    ersetCallbackInfo *info,
    eObject *context);


/**
****************************************************************************************************
  RowSet class.
****************************************************************************************************
*/
class eRowSet : public eTable
{
    friend class eRowSetBinding;

public:
    /**
    ************************************************************************************************
      Generic eObject functionality.
    ************************************************************************************************
    */
    /* Constructor.
     */
    eRowSet(
        eObject *parent = OS_NULL,
        e_oid id = EOID_ITEM,
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

    /**
    ************************************************************************************************
      Overloaded table interface functions.
    ************************************************************************************************
    */
    /* Insert rows into table. Row can be one row or container with multiple rows.
     */
    virtual void insert(
        eContainer *rows,
        os_int tflags = 0,
        eDBM *dbm = OS_NULL);

    /* Update a row or rows of a table.
     */
    virtual eStatus update(
        const os_char *where_clause,
        eContainer *row,
        os_int tflags = 0,
        eDBM *dbm = OS_NULL);

    /* Remove rows from table.
     */
    virtual void remove(
        const os_char *where_clause,
        os_int tflags = 0,
        eDBM *dbm = OS_NULL);

    /* Select rows from table.
     */
    virtual eStatus select(
        const os_char *where_clause,
        eContainer *columns,
        eSelectParameters *prm,
        os_int tflags = 0);


    /**
    ************************************************************************************************
      RowSet functions - these function treat rowset as organized to rows and columns.
    ************************************************************************************************
    */
    /* Set path to DBM.
     */
    inline void set_dbm(
        const os_char *dbm_path)
    {
        setpropertys(ERSETP_DBM_PATH, dbm_path);
    }

    /* Set table name.
     */
    inline void set_table(
        const os_char *table_name)
    {
        setpropertys(ERSETP_TABLE_NAME, table_name);
    }

    /* Set page mode.
     */
    inline void set_page_mode(
        os_int page_mode)
    {
        setpropertyl(ERSETP_PAGE_MODE, page_mode);
    }

    /* Set row mode.
     */
    inline void set_row_mode(
        os_int row_mode)
    {
        setpropertyl(ERSETP_ROW_MODE, row_mode);
    }

    /* Set row mode.
     */
    inline void set_tzone(
        eObject *tz)
    {
        setpropertyo(ERSETP_TZONE, tz);
    }

    /* Set callback function, when data is received or updated.
     */
    void set_callback(
        erset_callback_func *func,
        eObject *context)
    {
        m_callback = func;
        m_context = context;
    }

    /* Select rows from table.
     */
    void select(
        const os_char *where_clause,
        eContainer *columns,
        os_int limit = 0,
        os_int bflags = 0);

    /* Get rowset height and width.
     */
    inline os_int nrows() {return m_nrows; }
    inline os_int ncolumns() {return m_ncolumns; }

    /* Index column name and number.
     */
    inline const os_char *ix_column_name()
        {if (m_ix_column_name) { return m_ix_column_name->gets(); } return OS_NULL; }
    inline os_int ix_column_nr() {return m_ix_column_nr; }

    /* Get row set columns.
     */
    inline eContainer *columns() {
        if (m_configuration) {return m_configuration->firstc(EOID_TABLE_COLUMNS); }
        return OS_NULL;
    }

protected:
    /**
    ************************************************************************************************
      Internal functions.
    ************************************************************************************************
    */

    /* Table structure has been received, store it and do callback.
     */
    void client_binding_complete(
        eContainer *cont);

    /* All initial data has been received
     */
    void initial_data_complete(
        eContainer *sync_storage);

    /* Do trigged inserts and updates.
     */
    void trigged_insert_or_update(
        eMatrix *m);

    /* Do trigged removes on this row set.
     */
    void trigged_remove(
        os_long ix_value);

    /* Do callback that trigged modifications have been completed.
     */
    void trigged_modifications_complete(
        eContainer *trigged_changes);

    /**
    ************************************************************************************************
      Member variables
    ************************************************************************************************
    */

    /** Number of rows.
     */
    os_int m_nrows;

    /** Number of columns.
     */
    os_int m_ncolumns;

    /** Table configuration (as selected).
     */
    eContainer *m_configuration;

    /** Index column name, OS_NULL if not set.
     */
    eVariable *m_ix_column_name;

    /** Index column number, 0...
     */
    os_int m_ix_column_nr;

    /** Own change, prevent recursion.
     */
    os_short m_own_change;

    /** DBM path
     */
    eVariable *m_dbm_path;

    /** Select parameters.
     */
    eSelectParameters m_prm;

    /** Flag indicating that DBM path or table name has changed and row set needs to be rebound.
     */
    os_boolean m_rebind;

    /** Pointer to row set callback function.
     */
    erset_callback_func *m_callback;

    /** Pointer to row set callback context.
     */
    eObject *m_context;
};

#endif
