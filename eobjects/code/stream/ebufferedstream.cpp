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
    m_flags = 0;
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


/**
****************************************************************************************************

  @brief Set up queue buffers and select optional encoding.

  @param  in_sz Maximum input queue size in bytes. This sets maximum input queue size for situation
          when a process receives data from socket, etc, continuously, but doesn't process it.
          There are other mecahnisms in code to prevent this. This is just last stopcap to
          prevent running out of memory on errornous use situation.
  @param  out_sz Maximum output queue size in bytes. See in_sz.
  @param  flags Flags to set up buffering, same as for the open stream function. Bit fields.
          - OSAL_STREAM_PLAIN: Disable stream encoding.
          - OSAL_STREAM_UNBUFFERED: Disable buffering.

  @return  If successfull, the function returns ESTATUS_SUCCESS. Other return values
           indicate an error.

****************************************************************************************************
*/
eStatus eBufferedStream::setup_queues(
    os_memsz in_sz,
    os_memsz out_sz,
    os_int flags)
{

    // OSAL_STREAM_ENCODE_ON_WRITE
 //       - OSAL_STREAM_ENCODE_ON_WRITE

    return ESTATUS_SUCCESS;
}

#if 0
    /* Write all data to queue.
     */
    m_out->write(buf, buf_sz, nwritten);

    /* If we have one frame buffered, try to write data to socket frame at a time.
     */
    return write_socket(OS_FALSE);

#endif
