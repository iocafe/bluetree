/**

  @file    ematrix.cpp
  @brief   Matrix class.
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


/**
****************************************************************************************************

  @brief Configure matrix as table.

  The eMatrix::configure() function ...

  @param   configuration

****************************************************************************************************
*/
void eMatrix::configure(
        eContainer *configuration,
        os_int tflags)
{

    if (m_configuration) {
        delete m_configuration;
        m_configuration = OS_NULL;
    }
    if (configuration == OS_NULL) {
        return;
    }

    m_configuration = (eContainer*)configuration->clone(this,
        EOID_TABLE_CONFIGURATION, EOBJ_TEMPORARY_ATTACHMENT);
}


