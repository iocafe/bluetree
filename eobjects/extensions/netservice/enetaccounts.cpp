/**

  @file    enetaccounts.cpp
  @brief   User accounts.
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
#include "extensions/netservice/enetservice.h"


/**
****************************************************************************************************

  @brief Create "user accounts" table.

  The eNetService::create_user_accounts_table function...

    "accounts": [
        {
            "user": "ispy*",
            "privileges": "admin",
            "password": "pass"
        },
        {
            "user": "frank*.iocafenet",
            "privileges": "server",
            "password": "pass"
        }
    ]

****************************************************************************************************
*/
void eNetService::create_user_accounts_table()
{
    eContainer *configuration, *columns;
    eVariable *column;

    m_persistent_accounts = new ePersistent(this);
    m_accounts_matrix = new eMatrix(m_persistent_accounts);
    m_accounts_matrix->addname("accounts");

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
    column->addname("user", ENAME_NO_MAP);
    column->setpropertys(EVARP_TEXT, "name");
    column->setpropertyi(EVARP_TYPE, OS_STR);
    column->setpropertys(EVARP_TTIP,
        "Device or user name. Examples: \'candy3\',\n"
        "\'ispy*\', or \'frank*.iocafenet\'");

    column = new eVariable(columns);
    column->addname("privileges", ENAME_NO_MAP);
    column->setpropertyi(EVARP_TYPE, OS_CHAR);
    column->setpropertys(EVARP_ATTR, "enum=\"0,none,1.quest,2.user,3.admin\"");

    column = new eVariable(columns);
    column->addname("password", ENAME_NO_MAP);
    column->setpropertyi(EVARP_TYPE, OS_STR);
    column->setpropertys(EVARP_TEXT, "password");

    column = new eVariable(columns);
    column->addname("active_logons", ENAME_NO_MAP);
    column->setpropertyi(EVARP_TYPE, OS_INT);
    column->setpropertys(EVARP_TEXT, "active logons");
    column->setpropertys(EVARP_ATTR, "nosave");
    column->setpropertys(EVARP_TTIP,
        "Number of users/devices, that are currently\n"
        "logged on using this account");

    column = new eVariable(columns);
    column->addname("logon_tstamp", ENAME_NO_MAP);
    column->setpropertys(EVARP_TEXT, "last logon");
    column->setpropertyi(EVARP_TYPE, OS_LONG);
    column->setpropertys(EVARP_ATTR, "tstamp=\"yy,sec\"");
    column->setpropertys(EVARP_TTIP,
        "Time stamp of the last successfull logon");

    /* ETABLE_ADOPT_ARGUMENT -> configuration will be released from memory.
     */
    m_accounts_matrix->configure(configuration, ETABLE_ADOPT_ARGUMENT);
    m_accounts_matrix->setflags(EOBJ_TEMPORARY_CALLBACK);

    m_persistent_accounts->load_file("accounts.eo");
}
