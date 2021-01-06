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
#include "extensions/netservice/enetservice.h"


/**
****************************************************************************************************

  @brief Create "user account" table.

  The eNetService::create_user_account_table function...

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
void eNetService::create_user_account_table()
{
    eContainer *configuration, *columns;
    eVariable *column;

    m_persistent_accounts = new ePersistent(this);
    m_account_matrix = new eMatrix(m_persistent_accounts);
    m_account_matrix->addname("accounts");
    m_account_matrix->setpropertys(ETABLEP_TEXT, "accounts");

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
        "User or device name. Examples: \'candy3\',\n"
        "\'ispy*\', or \'frank*.iocafenet\'");

    column = new eVariable(columns);
    column->addname("accept", ENAME_NO_MAP);
    column->setpropertys(EVARP_TEXT, "accept");
    column->setpropertyi(EVARP_TYPE, OS_STR);
    column->setpropertys(EVARP_ATTR, "list=\"none,ecom,iocom,*\"");
    column->setpropertys(EVARP_TTIP,
        "Accepted incoming connections for the user's.\n"
        "- \'none\': Connections by this user are not allowed.\n"
        "- \'ecom\': eobjects communication protocol (for glass user interface, etc).\n"
        "- \'iocom\': IO device communication protocol.\n"
        "- \'*\': all protocols are accepted.\n");

    column = new eVariable(columns);
    column->addname("privileges", ENAME_NO_MAP);
    column->setpropertyi(EVARP_TYPE, OS_CHAR);
    column->setpropertys(EVARP_ATTR, "enum=\"1.quest,2.user,3.admin\"");
    column->setpropertys(EVARP_TTIP,
        "What this user is allowed to User priviliges.\n"
        "- \'quest\': view data and parameters, user cannot change anything.\n"
        "- \'user\': view and modify parameters necessary to use the system.\n"
        "- \'admin\': user can do anything, like upgrade software and change system configuration.\n");

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
    column->setpropertys(EVARP_ATTR, "tstamp=\"yy,sec\",nosave");
    column->setpropertys(EVARP_TTIP,
        "Time stamp of the last successfull logon");

    /* ETABLE_ADOPT_ARGUMENT -> configuration will be released from memory.
     */
    m_account_matrix->configure(configuration, ETABLE_ADOPT_ARGUMENT);
    m_account_matrix->setflags(EOBJ_TEMPORARY_CALLBACK);

    m_persistent_accounts->load_file("accounts.eo");

    if (m_account_matrix->nrows() == 0) {
        add_user_account("ispy", "pass", 7, 3);
    }
}


void eNetService::add_user_account(
    const os_char *user_name,
    const os_char *password,
    os_int accept,
    os_int privileges,
    os_int row_nr)
{
    eContainer row;
    eVariable *element;

    if (row_nr > 0) {
        element = new eVariable(&row);
        element->addname("ix", ENAME_NO_MAP);
        element->setl(row_nr);
    }

    element = new eVariable(&row);
    element->addname("user", ENAME_NO_MAP);
    element->sets(user_name);

    if (password) {
        element = new eVariable(&row);
        element->addname("password", ENAME_NO_MAP);
        element->sets(password);
    }

    if (accept >= 0) {
        element = new eVariable(&row);
        element->addname("accept", ENAME_NO_MAP);
        element->setl(accept);
    }

    if (privileges >= 0) {
        element = new eVariable(&row);
        element->addname("privileges", ENAME_NO_MAP);
        element->setl(privileges);
    }

    m_account_matrix->insert(&row);
}
