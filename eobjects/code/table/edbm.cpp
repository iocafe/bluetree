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
    eContainer *configuration, *rows, *row;
    eVariable *whereclause;

    /* If at final destination for the message.
     */
    if (*envelope->target()=='\0')
    {
        switch (envelope->command())
        {
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
                    rows = content->firstc(EOID_TABLE_CONTENT);
                    if (rows) {
                        insert(rows, get_tflags(content));
                        return;
                    }
                }
                osal_debug_error("eDBM:Faulty ECMD_INSERT_ROWS_TO_TABLE message received");
                return;

            case ECMD_UPDATE_TABLE_ROWS:
                content = envelope->content();
                if (content) {
                    whereclause = content->firstv(EOID_TABLE_WHERE);
                    row = content->firstc(EOID_TABLE_CONTENT);
                    if (whereclause && row) {
                        update(whereclause, row, get_tflags(content));
                        return;
                    }
                }
                osal_debug_error("eDBM:Faulty ECMD_UPDATE_TABLE_ROWS message received");
                return;


            case ECMD_REMOVE_ROWS_FROM_TABLE:
                content = envelope->content();
                if (content) {
                    whereclause = content->firstv(EOID_TABLE_WHERE);
                    if (whereclause) {
                        remove(whereclause, get_tflags(content));
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

  @param   rows For single for: eContainer holding a eVariables for each element to set.
           Multiple rows: eContainer holding a eContainers for each row to insert. Each row
           container contains eVariable for each element to set.
  @param   tflags Reserved for future, set 0 for now.

****************************************************************************************************
*/
void eDBM::insert(
    eContainer *rows,
    os_int tflags)
{
    eTable *table;
    table = eMatrix::cast(parent());
    table->insert(rows, tflags);
}


/**
****************************************************************************************************

  @brief Update a row or rows of a table.

  @param   whereclause String containing range and/or actual where clause. This selects which
           rows are updated.
  @param   row A row of updated data. eContainer holding an eVariable for each element (column)
           to update. eVariable name is column name.
  @param   tflags Reserved for future, set 0 for now.


****************************************************************************************************
*/
void eDBM::update(
    eVariable *whereclause,
    eContainer *row,
    os_int tflags)
{
    eTable *table;
    table = eMatrix::cast(parent());
    table->update(whereclause->gets(), row, tflags);
}


/**
****************************************************************************************************

  @brief Remove rows from the table.

  @param   whereclause String containing range and/or actual where clause. This selects which
           rows are to be removed.
  @param   tflags Reserved for future, set 0 for now.

****************************************************************************************************
*/
void eDBM::remove(
    eVariable *whereclause,
    os_int tflags)
{
    eTable *table;
    table = eMatrix::cast(parent());
    table->remove(whereclause->gets(), tflags);
}


/**
****************************************************************************************************

  @brief Solve column wildcards, requested_columns -> resolved_columns.

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
    eContainer *requested_columns)
{
    eTable *table;
    eContainer *sconfiguration, *scolumns, *dcolumns, *sitem;
    eVariable *reqcol, *scol, *dcol, *name;
    os_char *namestr;

    table = eMatrix::cast(parent());
    resolved_configuration->clear();

    sconfiguration = table->configuration();
    scolumns = sconfiguration->firstc(EOID_TABLE_COLUMNS);

    for (sitem = sconfiguration->firstc(); sitem; sitem = sitem->nextc())
    {
        if (sitem != scolumns && sitem->oid() != EOID_TABLE_CONTENT) {
            sitem->clone(resolved_configuration);
        }
    }

    if (scolumns) {
        dcolumns = new eContainer(resolved_configuration, EOID_TABLE_COLUMNS);
        dcolumns->ns_create();

        for (reqcol = requested_columns->firstv(); reqcol; reqcol = reqcol->nextv()) {
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
