/**

  @file    edbm.cpp
  @brief   DBM class.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    15.10.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "eobjects.h"


/**
****************************************************************************************************
  Constructor.
****************************************************************************************************
*/
eDBM::eDBM(
    eObject *parent,
    e_oid id,
    os_int flags)
    : eObject(parent, id, flags)
{
    m_trigger_columns = OS_NULL;
    m_minix = OS_LONG_MIN;
    m_maxix = OS_LONG_MAX;
}


/**
****************************************************************************************************
  Virtual destructor.
****************************************************************************************************
*/
eDBM::~eDBM()
{
}


/**
****************************************************************************************************

  @brief Add eDBM to class list.

  The eDBM::setupclass function adds newobj function pointer to class list. This enables creating
  new objects dynamically by class identifier, which is used for serialization reader()
  functions.

  This needs to be called after eBuffer:setupclass(), there is a dependency in setup.

****************************************************************************************************
*/
void eDBM::setupclass()
{
    const os_int cls = ECLASSID_DBM;

    /* Add the class to class list.
     */
    os_lock();
    eclasslist_add(cls, (eNewObjFunc)newobj, "eDBM");
    propertysetdone(cls);
    os_unlock();
}


/**
****************************************************************************************************

  @brief Clone object

  The eDBM::clone function clones an eDBM.

  @param  parent Parent for the clone.
  @param  id Object identifier for the clone.
  @param  aflags 0 for default operation. EOBJ_NO_MAP not to map names.
  @return Pointer to the clone.

****************************************************************************************************
*/
eObject *eDBM::clone(
    eObject *parent,
    e_oid id,
    os_int aflags)
{
    eDBM *clonedobj;

    clonedobj = new eDBM(parent, id == EOID_CHILD ? oid() : id, flags());

    clonegeneric(clonedobj, aflags);
    return clonedobj;
}


/**
****************************************************************************************************

  @brief Function to process incoming messages.

  The eDBM::onmessage function handles messages received by object. If this function
  doesn't process message, it calls parent class'es onmessage function.

  @param   envelope Message envelope. Contains command, target and source paths and
           message content, etc.

****************************************************************************************************
*/
void eDBM::onmessage(
    eEnvelope *envelope)
{
    eObject *content;
    eVariable *table_name;
    eContainer *configuration, *rows, *row;
    eVariable *where_clause;

    /* If at final destination for the message.
     */
    if (*envelope->target()=='\0')
    {
        switch (envelope->command())
        {
            case ECMD_RSET_SELECT:
                forward_select_message_to_binding(envelope);
                return;

            case ECMD_CONFIGURE_TABLE:
                content = envelope->content();
                if (content) {
                    configuration = content->firstc(EOID_TABLE_CONFIGURATION);
                    if (configuration) {
                        configure(configuration, get_tflags(content));
                        return;
                    }
                }
                osal_debug_error("eDBM:Faulty ECMD_CONFIGURE_TABLE message received");
                return;

            case ECMD_INSERT_ROWS_TO_TABLE:
                content = envelope->content();
                if (content) {
                    table_name = firstv(EOID_TABLE_NAME);
                    rows = content->firstc(EOID_TABLE_CONTENT);
                    if (rows) {
                        insert(table_name, rows, get_tflags(content));
                        return;
                    }
                }
                osal_debug_error("eDBM:Faulty ECMD_INSERT_ROWS_TO_TABLE message received");
                return;

            case ECMD_UPDATE_TABLE_ROWS:
                content = envelope->content();
                if (content) {
                    table_name = firstv(EOID_TABLE_NAME);
                    where_clause = content->firstv(EOID_TABLE_WHERE);
                    row = content->firstc(EOID_TABLE_CONTENT);
                    if (where_clause && row) {
                        update(table_name, where_clause, row, get_tflags(content));
                        return;
                    }
                }
                osal_debug_error("eDBM:Faulty ECMD_UPDATE_TABLE_ROWS message received");
                return;


            case ECMD_REMOVE_ROWS_FROM_TABLE:
                content = envelope->content();
                if (content) {
                    table_name = firstv(EOID_TABLE_NAME);
                    where_clause = content->firstv(EOID_TABLE_WHERE);
                    if (where_clause) {
                        remove(table_name, where_clause, get_tflags(content));
                        return;
                    }
                }
                osal_debug_error("eDBM:Faulty ECMD_REMOVE_ROWS_FROM_TABLE message received");

            default:
                break;
        }
    }

    /* Call parent class'es onmessage.
     */
    eObject::onmessage(envelope);
}


/**
****************************************************************************************************

  @brief Forward select sent early to binding.

  When binding a row set, the first select is typically called at client before binding is
  complete done bedone often before the binding reply has been received. Thus select
  client can send ECMD_RSET_SELECT to DBM path instead of binding path. In that case,
  this function forwards ECMD_RSET_SELECT to binding.

  @param  envelope Message envelope, command ECMD_RSET_SELECT.
  @return None.

****************************************************************************************************
*/
void eDBM::forward_select_message_to_binding(
    eEnvelope *envelope)
{
    eContainer *bindings;
    eBinding *binding;
    os_char *source;

    bindings = firstc(EOID_BINDINGS);
    if (bindings == OS_NULL) return;
    source = envelope->source();

    for (binding = eBinding::cast(bindings->first(EOID_TABLE_SERVER_BINDING));
         binding;
         binding = eBinding::cast(binding->next(EOID_TABLE_SERVER_BINDING)))
    {
        if ((binding->bflags() & (EBIND_CLIENT|EBIND_BIND_ROWSET)) != EBIND_BIND_ROWSET) continue;
        if (!os_strcmp(binding->bindpath(), source)) break;
    }

    if (binding) {
        binding->onmessage(envelope);
    }
}


/**
****************************************************************************************************

  @brief Get tflags from message content.

  @param   content Message content.
  @return  tflags Value of tflags specified in content, 0 if not set.

****************************************************************************************************
*/
os_int eDBM::get_tflags(
    eObject *content)
{
    eVariable *f;
    os_int tflags;

    tflags = 0;
    f = content->firstv(EOID_FLAGS);
    if (f) tflags = f->geti();
    return tflags;
}


/**
****************************************************************************************************

  @brief Configure a table.

  The eDBM::configure function configures an underlying table. This function
  - Stores/modifies table column configuration.
  - Adds initial data rows to empty matrix.

  @param   configuration Table configuration, columns.
  @param   tflags Set 0 for default configuration. Set ETABLE_ADOPT_ARGUMENT to adopt/delete
           configuration. If set the configuration object pointer must not be used after the
           function call returns.

****************************************************************************************************
*/
void eDBM::configure(
    eContainer *configuration,
    os_int tflags)
{
    eTable *table;
    table = eTable::cast(parent());
    table->configure(configuration, tflags);
}


/**
****************************************************************************************************

  @brief Insert rows into table.

  The eDBM::insert() function inserts one or more new rows to table.

  @param   table_name Name of the table, can be OS_NULL for matrix.
  @param   rows For single for: eContainer holding a eVariables for each element to set.
           Multiple rows: eContainer holding a eContainers for each row to insert. Each row
           container contains eVariable for each element to set.
  @param   tflags Reserved for future, set 0 for now.

****************************************************************************************************
*/
void eDBM::insert(
    eVariable *table_name,
    eContainer *rows,
    os_int tflags)
{
    eTable *table;
    table = get_table(table_name);
    if (table) {
        table->insert(rows, tflags, this);
    }
}


/**
****************************************************************************************************

  @brief Update a row or rows of a table.

  @param   table_name Name of the table, can be OS_NULL for matrix.
  @param   where_clause String containing range and/or actual where clause. This selects which
           rows are updated.
  @param   row A row of updated data. eContainer holding an eVariable for each element (column)
           to update. eVariable name is column name.
  @param   tflags Reserved for future, set 0 for now.


****************************************************************************************************
*/
void eDBM::update(
    eVariable *table_name,
    eVariable *where_clause,
    eContainer *row,
    os_int tflags)
{
    eTable *table;
    table = get_table(table_name);
    if (table) {
        table->update(where_clause->gets(), row, tflags, this);
    }
}


/**
****************************************************************************************************

  @brief Remove rows from the table.

  @param   table_name Name of the table, can be OS_NULL for matrix.
  @param   where_clause String containing range and/or actual where clause. This selects which
           rows are to be removed.
  @param   tflags Reserved for future, set 0 for now.

****************************************************************************************************
*/
void eDBM::remove(
    eVariable *table_name,
    eVariable *where_clause,
    os_int tflags)
{
    eTable *table;
    table = get_table(table_name);
    if (table) {
        table->remove(where_clause->gets(), tflags, this);
    }
}


/**
****************************************************************************************************

  @brief Solve column wildcards, requested_columns -> resolved_configuration.

  This generates table configuration tree as "resolved congufuration" which includes
  expilicetly the table columns matching to the requested configuration.

  Order specified in requested columns is preserved in resolved configuration.

  @param   resolved_configuration Pointer to eContainer into which to store the resulting
           table configuration.
  @param   requested_columns Pointer to eContainer holding selected columns.

****************************************************************************************************
*/
void eDBM::solve_table_configuration(
    eContainer *resolved_configuration,
    eContainer *requested_columns,
    eVariable *table_name)
{
    eTable *table;
    eContainer *sconfiguration, *scolumns, *dcolumns;
    eObject *sitem;
    eVariable *reqcol, *scol, *dcol, *name;
    os_char *namestr;

    table = get_table(table_name);
    if (table == OS_NULL) return;

    resolved_configuration->clear();
    sconfiguration = table->configuration();
    scolumns = sconfiguration->firstc(EOID_TABLE_COLUMNS);

    for (sitem = sconfiguration->first(); sitem; sitem = sitem->next())
    {
        if (sitem != scolumns && sitem->oid() != EOID_TABLE_CONTENT) {
            sitem->clone(resolved_configuration);
        }
    }

    if (scolumns) {
        dcolumns = new eContainer(resolved_configuration, EOID_TABLE_COLUMNS);
        dcolumns->ns_create();

        for (reqcol = requested_columns->firstv(); reqcol; reqcol = reqcol->nextv())
        {
            name = (eVariable*)reqcol->primaryname();
            if (name == OS_NULL) {
                name = reqcol;
            }
            namestr = name->gets();

            if (!os_strcmp(namestr, "*")) {
                for (scol = scolumns->firstv(); scol; scol = scol->nextv())
                {
                    if (dcolumns->byname(namestr) == OS_NULL) {
                        scol->clone(dcolumns, EOID_ITEM);
                    }
                }

            }
            else if (dcolumns->byname(namestr) == OS_NULL)
            {
                scol = eVariable::cast(scolumns->byname(namestr));
                if (scol) {
                    scol->clone(dcolumns, EOID_ITEM);
                }
                else
                {
                    dcol = new eVariable(dcolumns, EOID_ITEM);
                    dcol->addname(namestr);
                }
            }
        }
    }
}


/**
****************************************************************************************************

  @brief Get pointer to table object derived from eTable.

  @param   table_name eVariable holding table name. Can be OS_NULL if not needed (eMatrix)
  @return  pointer to object derived from eTable or OS_NULL if none mches to table name.

****************************************************************************************************
*/
eTable *eDBM::get_table(
    eVariable *table_name)
{
    eTable *table;
    table = eMatrix::cast(parent());

    osal_debug_assert(table != OS_NULL);
    return table;
}


/**
****************************************************************************************************

  @brief Select data from table.

  Selects data from underlying table, at simplest table can be eMatrix. Rows to be selected are
  specified by where clause. Column which to get are listed in "columns" list. The selected data
  is returned trough the callback function.

  @param   whereclause String containing range and/or actual where clause. Asterik "*" selects
           all rows.
  @param   columns List of columns to get. eContainer holding an eVariable for each column
           to select. eVariable name is column name, or column name can also be stored as
           variable value. Column name "*" is wildcard which selects all columns.
  @param   prm Select parameters. Includes pointer to callback function.
  @param   tflags Reserved for future, set 0 for now.

  @return  OSAL_SUCCESS if ok. Other values indicate an error.

****************************************************************************************************
*/
eStatus eDBM::select(
    const os_char *where_clause,
    eContainer *columns,
    eSelectParameters *prm,
    os_int tflags)
{
    eTable *table;

    table = get_table(prm->table_name);
    if (table == OS_NULL) {
        return ESTATUS_FAILED;
    }

    return table->select(where_clause, columns, prm, tflags);
}


/**
****************************************************************************************************

  @brief Generate merged trigged data based on all server side row set bindings.

  Called when data is selected by server side eRowSetBinding, server side binding is unbound.

  Merge all currently valid selections and get list of used columns used (either in where clause
  or in columns data). Save also minimum and maximum index value which covers all selections.

  The trigger data is passed to table (holding the data) at insert, update or remove call.
  This allows dynamic inserts, updates and removed to active selections without reselecting
  the data.

****************************************************************************************************
*/
void eDBM::generate_trigger_data()
{
    eContainer *bindings;
    eBinding *binding;
    eRowSetBinding *rbinding;
    eWhere *w;
    eContainer *list;
    eVariable *v, *u;
    eName *n;
    os_char *nstr;
    os_long ix;

    // Setup reactive data.

    delete m_trigger_columns;
    m_trigger_columns = new eContainer(this);
    m_trigger_columns->ns_create();

    /* Set reversed to start with (no matches)
     */
    m_minix = OS_LONG_MAX;
    m_maxix = OS_LONG_MIN;

    bindings = firstc(EOID_BINDINGS);
    if (bindings == OS_NULL) return ;
    for (binding = eBinding::cast(bindings->first(EOID_TABLE_SERVER_BINDING));
         binding;
         binding = eBinding::cast(binding->next(EOID_TABLE_SERVER_BINDING)))
    {
        /* Only server side row set bindings.
         */
        if ((binding->bflags() & (EBIND_CLIENT|EBIND_BIND_ROWSET)) != EBIND_BIND_ROWSET) continue;
        rbinding = eRowSetBinding::cast(binding);

        /* Set smallest min, max range which includes all binding index ranges.
         */
        ix = rbinding->minix();
        if (ix < m_minix) m_minix = ix;
        ix = rbinding->maxix();
        if (ix > m_maxix) m_maxix = ix;

        w = rbinding->where();
        if (w) {
            list = w->variables();
            if (list) {
                for (v = list->firstv(); v; v = v->nextv()) {
                    n = v->primaryname();
                    if (n) {
                        nstr = n->gets();
                        if (m_trigger_columns->byname(nstr) == OS_NULL) {
                            u = new eVariable(m_trigger_columns);
                            u->addname(nstr);
                        }
                    }
                }
            }
        }

        list = rbinding->columns();
        if (list) {
            for (v = list->firstv(); v; v = v->nextv()) {
                n = v->primaryname();
                if (n) {
                    nstr = n->gets();
                    if (m_trigger_columns->byname(nstr) == OS_NULL) {
                        u = new eVariable(m_trigger_columns);
                        u->addname(nstr);
                    }
                }
            }
        }
    }
}


/**
****************************************************************************************************

  @brief Get current trigger data.

  returns trigger data set by refresh_trigger_data().

  Called by insert, update and remove in this C file.

  @param
  @return

****************************************************************************************************
*/
/* eStatus eDBM::get_trigger_data(
{
    Clear all trigged data messages in bindings

    Return trigger data.
}
*/


/**
****************************************************************************************************

  @brief X

  Callback from table implementation (eMatrix, etc) when trigger update matching to trigger
  period has been changed.

  @param   table_name eVariable holding table name. Can be OS_NULL if not needed (eMatrix)
  @return  pointer to object derived from eTable or OS_NULL if none mches to table name.

****************************************************************************************************
*/
/* eStatus eDBM::trigger_callback(Row *xx)
{
    // * Loop trough the active selections

        // If index range matces

        // If where clause evaluates ok

        // Append information about insert, update or remove to up. to tigged data message
}

*/


/**
****************************************************************************************************

  @brief X

  Send trigged data messages to clients.


****************************************************************************************************
*/
/* eStatus eDBM::send_trigged_data()
 *
 * {
 * }

*/
