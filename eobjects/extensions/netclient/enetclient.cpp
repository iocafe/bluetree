/**

  @file    enetclient.cpp
  @brief   enet client implementation.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "eobjects.h"
#include "extensions/netclient/enetclient.h"


/**
****************************************************************************************************

  @brief Constructor.

  Name this object and create connections table.

****************************************************************************************************
*/
eNetClient::eNetClient(
    eObject *parent,
    e_oid oid,
    os_int flags)
    : eThread(parent, oid, flags)
{
    eContainer *configuration, *columns;
    eVariable *column;

    addname("client", ENAME_PROCESS_NS|ENAME_TEMPORARY|ENAME_PRIMARY|ENAME_UNIQUE);
    ns_create("client");

    m_connections = new eMatrix(this, EOID_ITEM);
    m_connections->addname("connections", ENAME_PARENT_NS|ENAME_TEMPORARY);

    configuration = new eContainer();

    columns = new eContainer(configuration);
    columns->addname("columns", ENAME_PARENT_NS|ENAME_NO_MAP);

    column = new eVariable(columns);
    column->addname("ix", ENAME_PARENT_NS|ENAME_NO_MAP);
    column->setpropertys(EVARP_TEXT, "rivi");

    column = new eVariable(columns);
    column->addname("connected", ENAME_PARENT_NS|ENAME_NO_MAP);
    column->setpropertyi(EVARP_TYPE, OS_STR);

    column = new eVariable(columns);
    column->addname("connectto", ENAME_PARENT_NS|ENAME_NO_MAP);

    m_connections->configure(configuration, 0 /*os_int tflags */);

    initproperties();
}


/**
****************************************************************************************************
  Virtual destructor.
****************************************************************************************************
*/
eNetClient::~eNetClient()
{
}


/**
****************************************************************************************************

  @brief Clone object

  The eNetClient::clone function clones and object including object's children.
  Names will be left detached in clone.

  @param  parent Parent for the clone.
  @param  oid Object identifier for the clone.
  @param  aflags 0 for default operation. EOBJ_NO_MAP not to map names.
  @return Pointer to the clone.

****************************************************************************************************
*/
eObject *eNetClient::clone(
    eObject *parent,
    e_oid id,
    os_int aflags)
{
    eObject *clonedobj;
    clonedobj = new eNetClient(parent, id == EOID_CHILD ? oid() : id, flags());
    clonegeneric(clonedobj, aflags|EOBJ_CLONE_ALL_CHILDREN);
    return clonedobj;
}


/**
****************************************************************************************************

  @brief Add the class to class list and class'es properties to it's property set.

  The eVariable::setupclass function adds the class to class list and class'es properties to
  it's property set. The class list enables creating new objects dynamically by class identifier,
  which is used for serialization reader functions. The property set stores static list of
  class'es properties and metadata for those.

****************************************************************************************************
*/
void eNetClient::setupclass()
{
    const os_int cls = ECLASSID_CONTAINER;

    /* Add the class to class list.
     */
    os_lock();
    eclasslist_add(cls, (eNewObjFunc)newobj, "eNetClient");
    os_unlock();
}




void enet_start_client(
    eThreadHandle *client_thread_handle)
{
    eNetClient *net_client;

    /* Start net client as a thread.
     */
    net_client = new eNetClient();
    net_client->start(client_thread_handle);
}
