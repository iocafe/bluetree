/**

  @file    esocket.cpp
  @brief   TCP socket class.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    14.9.2020

  TCP socket class eOsStream encodes and buffers data and calls OSAL's stream functions to
  read/write the socket. This class is used by eConnection and eEndPoint classes.

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
  Clears member variables.

****************************************************************************************************
*/
eOsStream::eOsStream(
    eObject *parent,
    e_oid id,
    os_int flags)
    : eStream(parent, id, flags)
{
    m_stream = OS_NULL;
}


/**
****************************************************************************************************

  @brief Virtual destructor.
  Closes the OS socket if it is open.

****************************************************************************************************
*/
eOsStream::~eOsStream()
{
    close();
}


/**
****************************************************************************************************

  @brief Add eOsStream to class list and class'es properties to it's property set.

  The eOsStream::setupclass function adds eOsStream to class list and class'es properties to
  it's property set. The class list enables creating new objects dynamically by class identifier,
  which is used for serialization reader functions. The property set stores static list of
  class'es properties and metadata for those.

****************************************************************************************************
*/
void eOsStream::setupclass()
{
    const os_int cls = ECLASSID_OSSTREAM;

    /* Add the class to class list.
     */
    os_lock();
    eclasslist_add(cls, (eNewObjFunc)newobj, "eOsStream");
    os_unlock();
}


/**
****************************************************************************************************

  @brief Open a socket.

  The open() function opens a socket. The function can either connect a socket or listen to
  specific TCP port.

  @param  parameters Socket parameters, a list string or direct value.
          Address and port to connect to, or interface and port to listen for.
          Socket IP address and port can be specified either as value of "addr" item
          or directly in parameter sstring. For example "192.168.1.55:20" or "localhost:12345"
          specify IPv4 addressed. If only port number is specified, which is often
          useful for listening socket, for example ":12345".
          IPv4 address is automatically recognized from numeric address like
          "2001:0db8:85a3:0000:0000:8a2e:0370:7334", but not when address is specified as string
          nor for empty IP specifying only port to listen. Use brackets around IP address
          to mark IPv6 address, for example "[localhost]:12345", or "[]:12345" for empty IP.

XXXXXXXXXXXXXXXXXX FLAGS
  @param  flags Flags for creating the socket. Bit fields, combination of:
          - OSAL_STREAM_CONNECT: Connect to specified socket port at specified IP address.
          - OSAL_STREAM_LISTEN: Open a socket to listen for incoming connections.
          - OSAL_STREAM_UDP_MULTICAST: Open a UDP multicast socket.
          - OSAL_STREAM_TCP_NODELAY: Disable Nagle's algorithm on TCP socket.
          - OSAL_STREAM_NO_REUSEADDR: Disable reusability of the socket descriptor.

  @return  If successfull, the function returns ESTATUS_SUCCESS. Other return values
           indicate an error.

****************************************************************************************************
*/
eStatus eOsStream::open(
    os_char *parameters,
    os_int flags)
{
    osalStatus s;

    /* If socket is already open.
     */
    if (m_stream) {
        osal_debug_error("eOsStream: stream is already open");
        return ESTATUS_FAILED;
    }

    const osalStreamInterface *iface = OSAL_SOCKET_IFACE;

    /* Open socket and return ESTATUS_SUCCESS or ESTATUS_FAILED.
     */
    m_stream = osal_stream_open(iface, parameters, OS_NULL, &s, flags);
    return s ? ESTATUS_FAILED : ESTATUS_SUCCESS;
}


/**
****************************************************************************************************

  @brief Close a socket.

  The eOsStream::close function closes underlaying operating system socket. If socket is not open
  function returns ESTATUS_FAILED and does nothing.

  @return If succesfull, the function returns ESTATUS_SUCCESS (0). Otherwise if socket is not
          open returns ESTATUS failed.

****************************************************************************************************
*/
eStatus eOsStream::close()
{
    if (m_stream == OS_NULL) {
        return ESTATUS_FAILED;
    }

    osal_stream_close(m_stream, OSAL_STREAM_DEFAULT);
    m_stream = OS_NULL;

    return ESTATUS_SUCCESS;
}


/**
****************************************************************************************************

  @brief Flush written data to socket.

  The eOsStream::flush function writes all data in output queue to socket. This uses
  eOsStream::select() function, which can also read received data while writing.
  This prevents the socket from getting stick if both ends are writing large amount of data
  at same time.

  @param  flags Ignored for now.
  @return If succesfull, the function returns ESTATUS_SUCCESS (0). Otherwise if socket is not
          open returns ESTATUS failed.

****************************************************************************************************
*/
eStatus eOsStream::flush(
    os_int flags)
{
    // osalSelectData selectdata;
    // eStream *strm;
    // eStatus s;

    if (m_stream == OS_NULL)
    {

        return ESTATUS_FAILED;
    }

    return osal_stream_flush(m_stream, OSAL_STREAM_DEFAULT)
        ? ESTATUS_FAILED : ESTATUS_SUCCESS;
}


/**
****************************************************************************************************

  @brief Write data to socket output buffer/to socket.

  The eOsStream::write function writes data first to output buffer. Then attempts to write
  data from output buffer into socket, as long as there are full frames and socket would
  not block.

  @return If succesfull, the function returns ESTATUS_SUCCESS (0). Otherwise if socket is not
          open returns ESTATUS failed.

****************************************************************************************************
*/
eStatus eOsStream::write(
    const os_char *buf,
    os_memsz buf_sz,
    os_memsz *nwritten)
{
    if (m_stream == OS_NULL)
    {
        *nwritten = 0;
        return ESTATUS_FAILED;
    }


return ESTATUS_FAILED;
}


/**
****************************************************************************************************

  @brief Read data to soket input buffer, fill in by readinf from socket.

  The eOsStream::read function first tries to read data from socket input buffer.
  It there is not enough data in input buffer, the function tries to read more
  data from the socket.

  @param  buf Ponter to buffer where to place the data read.
  @param  buf_sz Buffer size in bytes.
  @param  nread Pointer integer into which number of bytes read is stored.
          OS_NULL if not needed.
          Less or equal to buf_sz.
  @param  flags Ignored, set zero for now.

  @return If succesfull, the function returns ESTATUS_SUCCESS (0). Otherwise if error
          the function returns ESTATUS_FAILED.

****************************************************************************************************
*/
eStatus eOsStream::read(
    os_char *buf,
    os_memsz buf_sz,
    os_memsz *nread,
    os_int flags)
{
#if 0
    eStatus s;
    osalSelectData selectdata;
    eStream *strm;
    os_memsz nrd, n;

    if (m_stream == OS_NULL)
    {
        if (nread) *nread = 0;
        return ESTATUS_FAILED;
    }

    /* Try to read socket.
     */
    s = read_socket();
    if (s) return s;

    n = 0;
    while (OS_TRUE)
    {
        /* Try to get from queue.
         */
        m_in->read(buf + n, buf_sz, &nrd);
        buf_sz -= nrd;
        n += nrd;
        buf += nrd;
        if (buf_sz <= 0) break;

        /* Let select handle data transfers.
         */
        strm = this;
        select(&strm, 1, OS_NULL, &selectdata, OSAL_STREAM_DEFAULT);
        if (selectdata.errorcode)
        {
            if (nread) *nread = n;
            return ESTATUS_FAILED;
        }
    }

    if (nread) *nread = n;
#endif
    return ESTATUS_SUCCESS;
}


/**
****************************************************************************************************

  @brief Write character, typically control code.

  The eOsStream::writechar function writes character or control code.

  @param  c Character 0-255 or control code > 255 to write.
  @return If succesfull, the function returns ESTATUS_SUCCESS (0). Other return values indicate
          an error.

****************************************************************************************************
*/
eStatus eOsStream::writechar(
    os_int c)
{
#if 0
    /* Write the character to output queue.
     */
    m_out->writechar(c);

    /* If we have whole frame buffered, try to write data to socket.
     */
    return write_socket(OS_FALSE);
#endif
    return ESTATUS_SUCCESS;
}


/**
****************************************************************************************************

  @brief Read character or control code.

  The eOsStream::readchar function reads character or control code.

  @return If succesfull, the function returns Character 0-255. Return value
          E_STREM_END_OF_DATA indicates broken socket.

****************************************************************************************************
*/
os_int eOsStream::readchar()
{
    return E_STREM_END_OF_DATA;
#if 0
    os_int c;
    eStatus s;
    osalSelectData selectdata;
    eStream *strm;


    if (m_stream == OS_NULL)
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
        s = read_socket();
        if (s) return E_STREM_END_OF_DATA;

        /* Try to get from queue.
         */
        c = m_in->readchar();
        if (c != E_STREM_END_OF_DATA) return c;

        /* Let select handle data transfers.
         */
        strm = this;
        select(&strm, 1, OS_NULL, &selectdata, OSAL_STREAM_DEFAULT);
        if (selectdata.errorcode) return E_STREM_END_OF_DATA;
    }
#endif
}


/**
****************************************************************************************************

  @brief Wait for socket or thread event.

  The eOsStream::select() function waits for socket or thread events. Socket evens are typically
  lilke "read": data can be read from socket, "write": data can be written to socket,
  "connect": Socket connected, "close": Socket closed.
  Thread evens indicate that there are messages to the thread to be processed.

  @param  streams Array of socket stream pointers. This function waits for socket events from
          all these streams.
  @oaram  nstreams Number of items in streams array.
  @param  evnt Operating system event to wait for.
  @param  selectdata Pointer to structure in which to fill information about the event.
          This includes error code.
  @param  flags Reserved, set 0 for now.

  @return None.

****************************************************************************************************
*/
void eOsStream::select(
    eStream **streams,
    os_int nstreams,
    osalEvent evnt,
    osalSelectData *selectdata,
    os_int flags)
{
#if 0
    osalStatus s;
    eOsStream **sockets, *so;
    osalStream osalsock[OSAL_SOCKET_SELECT_MAX];
    os_int i;

    sockets = (eOsStream**)streams;

    if (nstreams == 1)
    {
        s = osal_stream_select(&sockets[0]->m_stream, 1, evnt,
            selectdata, OSAL_STREAM_DEFAULT);
    }
    else
    {
        for (i = 0; i<nstreams; i++)
        {
            osalsock[i] = sockets[i]->m_stream;
        }

        osal_stream_select(osalsock, nstreams, evnt,
            selectdata, OSAL_STREAM_DEFAULT);
    }

    i = selectdata->stream_nr;
    if (selectdata->errorcode == OSAL_SUCCESS &&
        i >= 0 && i < nstreams)
    {
        so = sockets[i];

        if (selectdata->eventflags & OSAL_STREAM_READ_EVENT)
        {
            selectdata->errorcode = so->read_socket();
            if (selectdata->errorcode) return;
        }

        if (selectdata->eventflags & OSAL_STREAM_WRITE_EVENT)
        {
            selectdata->errorcode = so->write_socket(OS_FALSE);
        }
    }
#endif
}


/**
****************************************************************************************************

  @brief Accept incoming connection.

  The eOsStream::accept() function accepts an incoming connection.

  @param  newstrem Pointer to newly allocated eOsStream to set up for this accepted connection.
  @param  flags Reserved, set 0 for now.
  @return ESTATUS_SUCCESS indicates that connection has succesfully been accepted.
          OSAL_NO_NEW_CONNECTION indicates that there were no new connections.
          Other return values indicate an error.

****************************************************************************************************
*/
eStream *eOsStream::accept(
    os_int flags,
    eStatus *s,
    eObject *parent,
    e_oid id)
{
    eOsStream *new_stream;
    osalStatus osal_s;
    osalStream new_osal_stream;
    os_char remoteip[128];

    new_osal_stream = osal_stream_accept(m_stream, remoteip, sizeof(remoteip),
        &osal_s, OSAL_STREAM_DEFAULT);

    if (new_osal_stream)
    {
        /* Save OSAL socket handle
         */
        new_stream = new eOsStream(parent, id);
        new_stream->m_stream = new_osal_stream;

        if (s) {
            *s = ESTATUS_SUCCESS;
        }
        return new_stream;
    }

    if (s) {
        *s = ESTATUS_FROM_OSAL_STATUS(osal_s);
    }

    return OS_NULL;
}



