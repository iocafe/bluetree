/**

  @file    enettrusted.cpp
  @brief   User trusted.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "extensions/netservice/enetservice.h"


/**
****************************************************************************************************

  @brief Create "trusted certificates" table.

  The eNetService::create_trusted_certificate_table function creates table which lists path
  to certificates trusted by this client


****************************************************************************************************
*/
void eNetService::create_trusted_certificate_table()
{
    eContainer *configuration, *columns;
    eVariable *column;

    m_persistent_trusted = new ePersistent(this);
    m_trusted_matrix = new eMatrix(m_persistent_trusted);
    m_trusted_matrix->addname("trusted");
    m_trusted_matrix->setpropertys(ETABLEP_TEXT, "trusted certificates");

    configuration = new eContainer(this);
    columns = new eContainer(configuration, EOID_TABLE_COLUMNS);
    columns->addname("columns", ENAME_NO_MAP);

    /* For matrix as a table row number is always the first column in configuration.
     */
    column = new eVariable(columns);
    column->addname("ix", ENAME_NO_MAP);
    column->setpropertys(EVARP_TEXT, "row");
    column->setpropertyi(EVARP_TYPE, OS_INT);

    column = new eVariable(columns);
    column->addname("certificate file", ENAME_NO_MAP);
    column->setpropertys(EVARP_TEXT, "");
    column->setpropertyi(EVARP_TYPE, OS_STR);
    column->setpropertys(EVARP_TTIP,
        "Path to server certificate trusted by this client");

    /* ETABLE_ADOPT_ARGUMENT -> configuration will be released from memory.
     */
    m_trusted_matrix->configure(configuration, ETABLE_ADOPT_ARGUMENT);
    m_trusted_matrix->setflags(EOBJ_TEMPORARY_CALLBACK);

    m_persistent_trusted->load_file("_trusted.eo");
}


