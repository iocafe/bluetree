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

class eDBM;
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
  Bind row set to dbm (table) binding class.
****************************************************************************************************
*/
class eRowSetBinding : public eBinding
{
public:

    /**
    ************************************************************************************************
      Generic eObject functionality.
    ************************************************************************************************
    */
    /* Constructor.
     */
    eRowSetBinding(
        eObject *parent = OS_NULL,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT);

    /* Virtual destructor.
     */
    virtual ~eRowSetBinding();

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

    /* Get pointer to columns list in table configuration, OS_NULL if none (server).
       This is list with real names, no wildcards.
     */
    inline eContainer *columns() {
        if (m_table_configuration == OS_NULL) return OS_NULL;
        return m_table_configuration->firstc(EOID_TABLE_COLUMNS);
    }

    /* Get pointer to compiled where object, OS_NULL if none (server).
     */
    inline eWhere *where() {return m_where; }

    /** Get index range minimum and maximum. Functions return OS_LONG_MIN and OS_LONG_MAX
        respectively if index range is not set.
     */
    inline os_long minix() {return m_minix;}
    inline os_long maxix() {return m_maxix;}


    /**
    ************************************************************************************************
      Row set binding functions
    ************************************************************************************************
    */

    /* Bind row set to table (client).
     */
    void bind(
        eVariable *dbm_path,
        eContainer *columns,
        eSelectParameters *prm,
        os_int bflags);

    /* Select data from table (client).
     */
    void select(
        const os_char *where_clause,
        os_int limit,
        os_int page_mode = -1,
        os_int row_mode = -1,
        eObject *tzone = OS_NULL);

    /* Bind the server end.
     */
    virtual void srvbind(
        eObject *obj,
        eEnvelope *envelope);

    /* Append "remove row" to trig data to send to row set.
     */
    void trigdata_append_remove(
        os_long ix_value);

    /* Append "insert or update row" to trig data to send to row set.
     */
    void trigdata_append_insert_or_update(
        os_long ix_value,
        eContainer *trigger_columns,
        eDBM *dbm);

protected:

    /**
    ************************************************************************************************
      Internal functions
    ************************************************************************************************
    */
    /* Send first message to initiate row set binding (client)
     */
    void bind2(
        const os_char *remotepath);

    /* Select data from underlying table (server).
     */
    void srvselect(
        eEnvelope *envelope);

    /* Callback to process srvselect() results (server).
     */
    static eStatus srvselect_callback(
        eTable *t,
        eMatrix *data,
        eObject *context);

    /* Finish the client end of binding (client).
     */
    void cbindok(
        eObject *obj,
        eEnvelope *envelope);

    /* Some or all selected data data received, save it (client).
     */
    void table_data_received(
        eEnvelope *envelope);

    /* Inform row set that all initial data has been received (client).
     */
    void initial_data_complete();

    /* Get pointer to eDBM object (server)
     */
    eDBM *srv_dbm();

    /* Get pointer to eRowSet object (client)
     */
    eRowSet *client_rowset();



    /**
    ************************************************************************************************
      Member variables.
    ************************************************************************************************
    */
    /** Select parameters as structure, pointers to values within m_pset.
     */
    eSelectParameters m_pstruct;

    /** Requested columns, pointer to eContainer within m_pset.
        This may contain wildcards.
     */
    eContainer *m_requested_columns;

    /** Table structure, includes list of columns, without wildcards.
     */
    eContainer *m_table_configuration;

    /** eVariable holding where clause as string, OS_NULL if not server select.
     */
    eVariable *m_where_clause;

    /** Compiled where clause including variables, OS_NULL if not server select
     */
    eWhere *m_where;

    /** Index range of select, OS_LONG_MIN, OS_LONG_MAX if not limited.
     */
    os_long m_minix, m_maxix;

    /** trigged "remove row" and "insert/update row" data for the binding.
     */
    eContainer *m_trigged_changes;

    /** Synchronized transfer of the results (server).
     */
    eSynchronized *m_sync_transfer;

    /** Synchronized transfer, matrix number 0... (server)
     */
    os_int m_sync_transfer_mtx_nr;

    /** Partly received synchronized transfer data, oid = matrix number 0... (client)
     */
    eContainer *m_sync_storage;

    /** Synchronized transfer timeout.
     */
    static const os_long m_timeout_ms = 30000;
};

#endif
