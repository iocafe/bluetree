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
  Constructor.
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
    m_flushnow = OS_FALSE;
    m_send_size = 3900;
}


/**
****************************************************************************************************
  Virtual destructor.
****************************************************************************************************
*/
eBufferedStream::~eBufferedStream()
{
    delete_queues();
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
          - OSAL_STREAM_LISTEN: Do nothing.

  @return  If successfull, the function returns ESTATUS_SUCCESS. Other return values
           indicate an error.

****************************************************************************************************
*/
eStatus eBufferedStream::setup_queues(
    os_memsz in_sz,
    os_memsz out_sz,
    os_int flags)
{
    os_char nbuf[OSAL_NBUF_SZ];

    /* If we are listening, delete any queues.
     */
    if (flags & OSAL_STREAM_LISTEN)
    {
        delete_queues();
    }

    /* Otherwise connecting or accepting a socket, create the queues.
     */
    else
    {
        if (m_in == OS_NULL) m_in = new eQueue(this);
        if (m_out == OS_NULL) m_out = new eQueue(this);
        m_in->close();
        m_out->close();
        osal_int_to_str(nbuf, sizeof(nbuf), in_sz);
        m_in->open(nbuf, OSAL_STREAM_DECODE_ON_READ|OSAL_FLUSH_CTRL_COUNT|OSAL_STREAM_SELECT);
        osal_int_to_str(nbuf, sizeof(nbuf), out_sz);
        m_out->open(nbuf, OSAL_STREAM_ENCODE_ON_WRITE|OSAL_STREAM_SELECT);
    }

    m_flags = flags;
    return ESTATUS_SUCCESS;
}


/**
****************************************************************************************************

  @brief Delete queue buffers.

****************************************************************************************************
*/
void eBufferedStream::delete_queues()
{
    delete m_in;
    delete m_out;
    m_in = m_out = OS_NULL;
}


/**
****************************************************************************************************

  @brief Write from intenal buffer m_out to the stream.

  The eSocket::write_socket() function writes data from m_out queue to socket.
  If flushnow is not set, the function does nothing until m_out holds enough data for at least
  one ethernet frame. All data from m_out queue which can be sent immediately without wait,
  is written to socket.

  The derived stream class must implement buffered_write() to write to the stream.

  @param  flushnow If OS_TRUE, even single buffered byte is written. Otherwise waits until
          enough bytes for ethernet frame are buffered before writing.
  @return If no error detected, the function returns ESTATUS_SUCCESS.
          Other return values indicate an error and that socket is to be disconnected.

****************************************************************************************************
*/
eStatus eBufferedStream::buffer_to_stream(
    os_boolean flushnow)
{
    os_memsz n, nread, nwritten;
    os_char *buf = OS_NULL;
    eStatus s = ESTATUS_SUCCESS;

    m_flushnow |= flushnow;

    while (OS_TRUE)
    {
        n = m_out->bytes();
        if ((n < m_send_size && !m_flushnow) || n < 1) {
            if (n < 1) m_flushnow = OS_FALSE;
            break;
        }

        if (buf == OS_NULL) {
            buf = os_malloc(m_send_size, OS_NULL);
        }

        m_out->readx(buf, m_send_size, &nread, OSAL_STREAM_PEEK);
        if (nread == 0) {
            break;
        }

        s = buffered_write(buf, nread, &nwritten);
        if (s) {
            break;
        }

        if (nwritten <= 0) {
            break;
        }

        m_out->readx(OS_NULL, nwritten, &nread);
    }

    if (buf) {
        os_free(buf, m_send_size);
    }

    return s;
}


/**
****************************************************************************************************

  @brief Read from stream into intenal buffer m_in.

  The eBufferedStream::stream_to_buffer function reads data from socket and places it to
  m_in queue. All available data from socket is read.

  The derived stream class must implement buffered_read() to read to the stream.

  @return If no error detected, the function returns ESTATUS_SUCCESS.
          Other return values indicate an error and that socket is to be disconnected.

****************************************************************************************************
*/
eStatus eBufferedStream::stream_to_buffer()
{
    os_memsz nread;
    os_char buf[2048];
    eStatus s, s2;

    do
    {
        s = buffered_read(buf, sizeof(buf), &nread);
        if ((s && s != ESTATUS_STREAM_END) || nread == 0) {
            break;
        }

        s2 = m_in->write(buf, nread);
        if (s2) {
            s = s2;
            break;
        }
    }
    while (s != ESTATUS_STREAM_END);

    return s;
}


/**
****************************************************************************************************

  @brief Write character, typically control code.

  The eOsStream::writechar function writes character or control code.

  @param  c Character 0-255 or control code > 255 to write.
  @return If succesfull, the function returns ESTATUS_SUCCESS (0). Other return values indicate
          an error.
          Return value ESTATUS_BUFFER_OVERFLOW from unbuffered stream indicates that byte
          could not be written.

****************************************************************************************************
*/
eStatus eBufferedStream::writechar(
    os_int c)
{
    /* If we got output queue (buffered stream), append to the queue.
     */
    if (m_out) {
        return m_out->writechar(c);
    }

    return ESTATUS_FAILED;
}


/**
****************************************************************************************************

  @brief Read character or control code.

  The eOsStream::readchar function reads character or control code.

  @return If succesfull, the function returns Character 0-255. Return value
          E_STREM_END_OF_DATA indicates broken socket.

****************************************************************************************************
*/
os_int eBufferedStream::readchar()
{
    os_int c;
    eStatus s;
    eStream *strm;

    if (m_in == OS_NULL)
    {
        return E_STREM_END_OF_DATA;
    }

    while (OS_TRUE)
    {
        /* Try to get from queue.
         */
        c = m_in->readchar();
        if (c != E_STREM_END_OF_DATA) return c;

        /* Try to read socket.
         */
        s = stream_to_buffer();
        if (s) return E_STREM_END_OF_DATA;

        /* Try to get from queue.
         */
        c = m_in->readchar();
        if (c != E_STREM_END_OF_DATA) return c;

        /* Let select handle data transfers.
         */
        strm = this;
        s = select(&strm, 1, OS_NULL, 0, OSAL_STREAM_DEFAULT);
        if (s) return E_STREM_END_OF_DATA;
    }
}
