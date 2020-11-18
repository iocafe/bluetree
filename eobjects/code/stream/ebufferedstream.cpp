/**

  @file    ebufferedstream.cpp
  @brief   Stream buffering and encoding.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    14.9.2020

  Add buffering and encoding functionality to eStream.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "eobjects.h"


/**
****************************************************************************************************

  @brief Constructor.

  X...

  @return  None.

****************************************************************************************************
*/
eBufferedStream::eBufferedStream(
    eObject *parent,
    e_oid id,
    os_int flags)
    : eStream(parent, id, flags)
{
    m_in = OS_NULL;
    m_out = OS_NULL;
}


/**
****************************************************************************************************

  @brief Virtual destructor.

  X...

  @return  None.

****************************************************************************************************
*/
eBufferedStream::~eBufferedStream()
{
    delete m_in;
    delete m_out;
}
