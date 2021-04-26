/**

  @file    enetstatus.cpp
  @brief   Application status table.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "extensions/netservice/enetservice.h"


/**
****************************************************************************************************

  @brief Create "process status" table.

  The eNetService::create_process_status_table function creates table to display which errors,
  etc are on.


****************************************************************************************************
*/
void eNetService::create_process_status_table()
{
    eContainer *configuration, *columns;
    eVariable *column;

    m_persistent_status = new ePersistent(this);
    m_status_matrix = new eMatrix(m_persistent_status);
    m_status_matrix->addname("status");
    m_status_matrix->setpropertys(ETABLEP_TEXT, "process status");

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
    column->addname("level", ENAME_NO_MAP);
    column->setpropertys(EVARP_TEXT, "level");
    column->setpropertyi(EVARP_TYPE, OS_CHAR);
    column->setpropertys(EVARP_ATTR, "enum=\"1.info,2.warning,3.error,\"");
    column->setpropertyi(EVARP_DEFAULT, 2);
    column->setpropertys(EVARP_TTIP,
        "Warning, error, info, etc");

    column = new eVariable(columns);
    column->addname("code", ENAME_NO_MAP);
    column->setpropertys(EVARP_TEXT, "code");
    column->setpropertyi(EVARP_TYPE, OS_STR);
    column->setpropertys(EVARP_DEFAULT, "stickstuck.gearbox.transmission");
    column->setpropertys(EVARP_TTIP,
        "Code");

    column = new eVariable(columns);
    column->addname("count", ENAME_NO_MAP);
    column->setpropertys(EVARP_TEXT, "count");
    column->setpropertyi(EVARP_TYPE, OS_LONG);
    column->setpropertyi(EVARP_DEFAULT, 0);
    column->setpropertys(EVARP_TTIP,
        "How many times this error occurred during last 24 hours.");

    column = new eVariable(columns);
    column->addname("description", ENAME_NO_MAP);
    column->setpropertys(EVARP_TEXT, "description");
    column->setpropertyi(EVARP_TYPE, OS_STR);
    column->setpropertys(EVARP_DEFAULT, "Gear randimizer stick stuck");
    column->setpropertys(EVARP_TTIP,
        "Message description");

    column = new eVariable(columns);
    column->addname("tstamp", ENAME_NO_MAP);
    column->setpropertys(EVARP_TEXT, "timestamp");
    column->setpropertyi(EVARP_TYPE, OS_LONG);
    column->setpropertys(EVARP_ATTR, "tstamp=\"yy,sec\"");
    column->setpropertys(EVARP_TTIP,
        "Time stamp of last message");

    column = new eVariable(columns);
    column->addname("internal", ENAME_NO_MAP);
    column->setpropertys(EVARP_TEXT, "internal");
    column->setpropertyi(EVARP_TYPE, OS_OBJECT);
    column->setpropertyi(EVARP_DEFAULT, 0);
    column->setpropertys(EVARP_TTIP,
        "Matrix containing hourly count for 24 last hours.");

    /* ETABLE_ADOPT_ARGUMENT -> configuration will be released from memory.
     */
    m_status_matrix->configure(configuration, ETABLE_ADOPT_ARGUMENT);
    m_status_matrix->setflags(EOBJ_TEMPORARY_CALLBACK);

    m_persistent_status->load_file("_status.eo");
}


