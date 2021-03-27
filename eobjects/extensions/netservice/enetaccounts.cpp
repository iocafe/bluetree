/**

  @file    enetaccounts.cpp
  @brief   User accounts, authentication and authorization.
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
            "user": "root*",
            "privileges": "admin",
            "password": "pass"
        },
        {
            "user": "frank*.cafenet",
            "privileges": "server",
            "password": "pass"
        }
    ]

****************************************************************************************************
*/
void eNetService::create_user_account_table(
    os_int flags)
{
    eContainer *configuration, *columns;
    eVariable *column, tmp;

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
    column->addname(enet_conn_enable, ENAME_NO_MAP);
    column->setpropertys(EVARP_TEXT, "enable");
    column->setpropertyi(EVARP_TYPE, OS_BOOLEAN);
    column->setpropertyi(EVARP_DEFAULT, OS_FALSE);
    column->setpropertys(EVARP_TTIP,
        "Enable this user account.");

    column = new eVariable(columns);
    column->addname("user", ENAME_NO_MAP);
    column->setpropertys(EVARP_TEXT, "name");
    column->setpropertyi(EVARP_TYPE, OS_STR);
    column->setpropertys(EVARP_TTIP,
        "User or device name. Examples: \'root\', \'mike\', \'candy3\',\n"
        "\'root*\', or \'frank*.cafenet\'");

    column = new eVariable(columns);
    column->addname("protocols", ENAME_NO_MAP);
    column->setpropertys(EVARP_TEXT, "protocols");
    column->setpropertyi(EVARP_TYPE, OS_STR);
    tmp = "list=\"";
    if (flags & ENET_ENABLE_EOBJECTS_SERVICE) { tmp += "ecom,"; }
    if (flags & ENET_ENABLE_IOCOM_SERVICE) { tmp += "iocom,"; }
    if (flags & ENET_ENABLE_SWITCHBOX_SERVICE) { tmp += "switchbox,"; }
    tmp += "*\"";
    column->setpropertys(EVARP_ATTR, tmp.gets());

    tmp = "Accepted incoming connection protocol(s):\n";
    if (flags & ENET_ENABLE_EOBJECTS_SERVICE) {
        tmp += "- \'ecom\': eobjects communication protocol (for glass user interface, etc).\n";
    }
    if (flags & ENET_ENABLE_IOCOM_SERVICE) {
        tmp += "- \'iocom\': IO device communication protocol.\n";
    }
    if (flags & ENET_ENABLE_SWITCHBOX_SERVICE) {
        tmp += "- \'switchbox\': Switchbox service protocol.\n";
    }
    tmp += "- \'*\': all protocols are accepted.\n";
    column->setpropertys(EVARP_TTIP, tmp.gets());

    column = new eVariable(columns);
    column->addname("privileges", ENAME_NO_MAP);
    column->setpropertyi(EVARP_TYPE, OS_CHAR);
    column->setpropertys(EVARP_ATTR, "enum=\"1.quest,2.user,3.root\"");
    column->setpropertys(EVARP_TTIP,
        "Privileges select what the user is allowed to do:\n"
        "- \'quest\': view data and parameters, user cannot change anything.\n"
        "- \'user\': normal user is allowed to view and modify most parameters, etc.\n"
        "- \'root\': administrate access, like upgrade software and change system configuration.\n");

    column = new eVariable(columns);
    column->addname("password", ENAME_NO_MAP);
    column->setpropertyi(EVARP_TYPE, OS_STR);
    column->setpropertys(EVARP_TEXT, "password");

    /* column = new eVariable(columns);
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
        "Time stamp of the last successfull logon"); */

    /* ETABLE_ADOPT_ARGUMENT -> configuration will be released from memory.
     */
    m_account_matrix->configure(configuration, ETABLE_ADOPT_ARGUMENT);
    m_account_matrix->setflags(EOBJ_TEMPORARY_CALLBACK);

    m_persistent_accounts->load_file("accounts.eo");

    if (m_account_matrix->nrows() == 0) {
        add_user_account(OS_TRUE, "root", "*", "*", 3);
        add_user_account(OS_FALSE, "user", "pass", "*", 2);
        add_user_account(OS_FALSE, "quest", "pass", "*", 1);
    }
}


/**
****************************************************************************************************

  @brief Add new user account.

****************************************************************************************************
*/
void eNetService::add_user_account(
    os_int enable,
    const os_char *user_name,
    const os_char *password,
    const os_char *accept,
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
    element->addname("enable", ENAME_NO_MAP);
    element->setl(enable);

    element = new eVariable(&row);
    element->addname("user", ENAME_NO_MAP);
    element->sets(user_name);

    if (password) {
        element = new eVariable(&row);
        element->addname("password", ENAME_NO_MAP);
        element->sets(password);
    }

    if (accept) {
        element = new eVariable(&row);
        element->addname("protocols", ENAME_NO_MAP);
        element->sets(accept);
    }

    if (privileges >= 0) {
        element = new eVariable(&row);
        element->addname("privileges", ENAME_NO_MAP);
        element->setl(privileges);
    }

    m_account_matrix->insert(&row);
}


/**
****************************************************************************************************

  @brief Callback from IOCOM get user authorization.

  This function is called by IOCOM library trough a function pointer to allow implementing
  user authentication mechanism for application.

  The allowed_networks is structure set up to hold list of networks which can be accessed
  trough the connection and privileges for each network. Must be released by
  ioc_release_allowed_networks().

****************************************************************************************************
*/
osalStatus eNetService::authorize_iocom_user(
    struct iocRoot *root,
    iocAllowedNetworkConf *allowed_networks,
    iocUser *user_account,
    os_char *ip,
    void *context)
{
    /* eNetService *netsrv;

    os_lock();
    netsrv = (eNetService *)context;

    os_unlock(); */

    return OSAL_SUCCESS;
}
