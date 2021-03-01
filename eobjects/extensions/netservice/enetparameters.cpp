/**

  @file    enetparameters.cpp
  @brief   Service parameters.
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

/* Forward referred local functions.
 */
static eVariable *add_service_prm(
    const os_char *name,
    const os_char *text,
    const os_char *tooltip,
    osalTypeId typ,
    ePersistent *cont);


/**
****************************************************************************************************

  @brief Create "service parameters" table.

  The eNetService::create_service_parameters function...

  @param  flags Bit fields, combination of ENET_ENABLE_IOCOM_CLIENT, ENET_ENABLE_EOBJECTS_CLIENT,
          ENET_ENABLE_IOCOM_SERVICE and ENET_ENABLE_EOBJECTS_SERVICE.

****************************************************************************************************
*/
void eNetService::create_service_parameters(
    os_int flags)
{
    m_persistent_parameters = new ePersistent(this);
    m_persistent_parameters->addname("parameters");
    m_persistent_parameters->ns_create();
    m_persistent_parameters->setpropertys(EPERP_TEXT, "net parameters");

    m_parameters.nickname = add_service_prm("nickname",
        "nickname",
        "Nickname to helps user to identify the device or process in network",
        OS_STR, m_persistent_parameters);

    if (flags & (ENET_ENABLE_IOCOM_SERVICE | ENET_ENABLE_EOBJECTS_SERVICE)) {
        m_parameters.enable_lighthouse_server = add_service_prm("lighthouseserv",
            "enable lighthouse multicasts",
            "If enabled, this service will send periodic UDP multicasts,\n"
            "so it can be detected within LAN segment.",
            OS_BOOLEAN, m_persistent_parameters);
        m_parameters.enable_lighthouse_server->setl(OS_TRUE);

        m_parameters.serv_cert_path = add_service_prm("servcert",
            "server certificate file",
            "Path to server certificate",
            OS_STR, m_persistent_parameters);
        m_parameters.serv_cert_path->sets("where I am?");

        m_parameters.serv_priv_key = add_service_prm("servkey",
            "server's' private key",
            "Path to server private key file",
            OS_STR, m_persistent_parameters);
        m_parameters.serv_priv_key->sets("where I am?");
    }

    m_persistent_parameters->load_file("_serv_prm.eo");

#if IOC_NICKGEN_SUPPORT
    /* Set global nickname, generate one if we do not have it.
     */
    if (m_parameters.nickname->isempty()) {
        ioc_generate_nickname(osal_global->nickname, OSAL_NICKNAME_SZ);
        m_parameters.nickname->sets(osal_global->nickname);
    }
    else {
        osal_set_nickname(m_parameters.nickname->gets());
    }
#elif OSAL_NICKNAME_SUPPORT
    osal_set_nickname(m_parameters.nickname->gets());
#endif

    m_persistent_parameters->setflags(EOBJ_TEMPORARY_CALLBACK);
}


/**
****************************************************************************************************

  @brief Callback when parameter value has changed.

  The NetService::parameter_changed function...

  @param  v pointer to changed parameter.

****************************************************************************************************
*/
void eNetService::parameter_changed(
    eObject *v)
{
#if OSAL_NICKNAME_SUPPORT
    if (v == m_parameters.nickname) {
        osal_set_nickname(m_parameters.nickname->gets());
    }
#endif
}


static eVariable *add_service_prm(
    const os_char *name,
    const os_char *text,
    const os_char *tooltip,
    osalTypeId typ,
    ePersistent *cont)
{
    eVariable *v = new eVariable(cont);
    v->addname(name);
    v->setpropertys(EVARP_TEXT, text);
    if (tooltip) v->setpropertys(EVARP_TTIP, tooltip);
    v->setpropertyl(EVARP_TYPE, typ);
    v->setflags(EOBJ_TEMPORARY_CALLBACK);
    return v;
}
