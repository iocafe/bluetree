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

/* Default maximum queue sizes (for catastrophic failure on program error are 100MB).
   These can be overridden by compiler define.
 */
#ifndef ESTREAM_IN_QUEUE_SZ
#define ESTREAM_IN_QUEUE_SZ 100000000L
#endif
#ifndef ESTREAM_OUT_QUEUE_SZ
#define ESTREAM_OUT_QUEUE_SZ 100000000L
#endif

/**
****************************************************************************************************
  Constructor. Clears member variables.
****************************************************************************************************
*/
eOsStream::eOsStream(
    eObject *parent,
    e_oid id,
    os_int flags)
    : eBufferedStream(parent, id, flags)
{
    m_stream = OS_NULL;
    m_iface = OS_NULL;
    m_use_select = OS_TRUE;
}


/**
****************************************************************************************************
  Virtual destructor. Closes the OS stream if it is open.
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
    eclasslist_add(cls, (eNewObjFunc)newobj, "eOsStream", ECLASSID_STREAM);
    os_unlock();
}


/**
****************************************************************************************************

  @brief Open an eosal stream stream.

  The open() function opens a TLS socket, TCP socket, serial port, file, or memory buffer.
  For socket, the function can either connect a socket or listen to specific TCP port (optionally
  on spefic network interface).

  @param  parameters Socket parameters, a list string or direct value.
          Address and port to connect to, or interface and port to listen for.
          Socket IP address and port can be specified either as value of "addr" item
          or directly in parameter sstring. For example "socket:192.168.1.55:20" or
          "socket:localhost:12345" specify IPv4 addressed.

          To listen for a socket port on all network interfaces specify port number number
          without IP address. For example "tls::12345".

          IPv6 address is automatically recognized from numeric address like
          "tls:2001:0db8:85a3:0000:0000:8a2e:0370:7334", but not when address is specified as string
          nor for empty IP specifying only port to listen. Use brackets around IP address
          to mark IPv6 address, for example "tls:[localhost]:12345", or "tls:[]:12345" for empty IP.

          - "tls:", TLS socket.
          - "socket:" TCP socket.
          - "serial:" Serial communication.
          - "bluetooth:" Blue tooth specific implementation. On Windows/Linux blue tooth is
             just a serial port, and this choice is not valid.
          - "file:" File on local file system.
          - "buffer:" Memory buffer. This can be used to serialize and encode objects on
             the fly, like to store serialized object into database column.

  @param  flags Flags for the open function. Bit fields, some common ones:
          - OSAL_STREAM_CONNECT: Connect to specified socket port at specified IP address.
          - OSAL_STREAM_LISTEN: Open a socket to listen for incoming connections.
          - OSAL_STREAM_TCP_NODELAY: Disable Nagle's algorithm on TCP socket.
          - OSAL_STREAM_NO_REUSEADDR: Disable reusability of the socket descriptor.

  @return  If successfull, the function returns ESTATUS_SUCCESS. Other return values
           indicate an error.

****************************************************************************************************
*/
eStatus eOsStream::open(
    const os_char *parameters,
    os_int flags)
{
    osalStatus s;
    os_memsz len;

    /* Names and pointers of the supported OSAL interfaces.
     */
    typedef struct {
        const os_char *name;
        const osalStreamInterface *iface;
        os_boolean use_select;
    }
    eIfaceListItem;

    const static eIfaceListItem iface_list[] = {
#if OSAL_SOCKET_SUPPORT
#if OSAL_TLS_SUPPORT
        {"tls:", OSAL_TLS_IFACE, OS_TRUE},
#endif
        {"socket:", OSAL_SOCKET_IFACE, OS_TRUE},
#endif
#if OSAL_SERIAL_SUPPORT
        {"serial:", OSAL_SERIAL_IFACE, OS_TRUE},
#endif
#if OSAL_BLUETOOTH_SUPPORT
        {"bluetooth:", OSAL_BLUETOOTH_IFACE, OS_TRUE},
#endif
#if OSAL_FILESYS_SUPPORT
        {"file:", OSAL_FILE_IFACE, OS_FALSE},
#endif
#if OSAL_STREAM_BUFFER_SUPPORT
        {"buffer:", OSAL_STREAM_BUFFER_IFACE, OS_FALSE},
#endif
        {OS_NULL, OS_NULL, OS_FALSE}
    };
    const eIfaceListItem *item;

    /* If stream is already open.
     */
    if (m_stream) {
        osal_debug_error("eOsStream: stream is already open");
        return ESTATUS_FAILED;
    }

    /* Find interface by name, and skip interface in "parameters" string.
     */
    for (item = iface_list; item->name; item++) {
        len = os_strlen(item->name) - 1;
        if (!os_strncmp(parameters, item->name, len)) {
            parameters += len;
            break;
        }
    }
    if (item->name == OS_NULL) {
        osal_debug_error("eOsStream::open: interface, like \"tls:\" not in open() parameters");
        return ESTATUS_FAILED;
    }
    m_iface = item->iface;
    m_use_select = item->use_select;

    /* Open socket and return ESTATUS_SUCCESS or ESTATUS_FAILED. Save flags.
     */
    m_stream = m_iface->stream_open(parameters, OS_NULL, &s, flags);
    if (s) return ESTATUS_FROM_OSAL_STATUS(s);

    /* Setup queues to buffer outgoing and incoming data.
     */
    return setup_queues(ESTREAM_IN_QUEUE_SZ, ESTREAM_OUT_QUEUE_SZ, flags);
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

    m_iface->stream_close(m_stream, OSAL_STREAM_DEFAULT);
    m_stream = OS_NULL;
    delete_queues();

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
    OSAL_UNUSED(flags);

    if (m_out == OS_NULL) {
        return ESTATUS_FAILED;
    }

    buffer_to_stream(OS_TRUE);

    return m_iface->stream_flush(m_stream, OSAL_STREAM_DEFAULT)
        ? ESTATUS_FAILED : ESTATUS_SUCCESS;
}


/**
****************************************************************************************************

  @brief Write data to output buffer.

  If buffering is used, the eOsStream::write function writes data to output buffer. Data is not
  written to socket or other stream by this function, but only when flush is called.
  If buffering is not used, data is written directly to underlying EOSAL stream.

  @param   buf Pointer to data to write.
  @param   buf_sz Number of bytes to write.

  @return  If succesfull, the function returns ESTATUS_SUCCESS (0). Other return values indicate
           an error.

****************************************************************************************************
*/
eStatus eOsStream::write(
    const os_char *buf,
    os_memsz buf_sz)
{
    if (m_out == OS_NULL) {
        return ESTATUS_FAILED;
    }

    /* Write all data to buffer.
     */
    return m_out->write(buf, buf_sz);
}


/**
****************************************************************************************************

  @brief Write buffered data to underlying stream.

  The eOsStream::buffered_write function writes data to socket or other output stream.
  Buffered socket flush calls this function to actually write the data to stream.

  @param   buf Pointer to data to write.
  @param   buf_sz Number of bytes to write.
  @param   nwritten Pointer to integer where to store number of bytes written.
           This is set to number of bytes actually written, which may be less than buf_sz.
           This argument must not be OS_NULL.

  @return  If succesfull, the function returns ESTATUS_SUCCESS (0). Other return values indicate
           an error.

****************************************************************************************************
*/
eStatus eOsStream::buffered_write(
    const os_char *buf,
    os_memsz buf_sz,
    os_memsz *nwritten)
{
    osalStatus s;

    if (m_stream == OS_NULL)
    {
        *nwritten = 0;
        return ESTATUS_FAILED;
    }

    s = m_iface->stream_write(m_stream, buf, buf_sz, nwritten, OSAL_STREAM_DEFAULT);
    return ESTATUS_FROM_OSAL_STATUS(s);
}


/**
****************************************************************************************************

  @brief Read data to soket input buffer, fill in by readinf from socket.

  The eOsStream::read function first tries to read data from socket input buffer.
  It there is not enough data in input buffer, the function tries to read more
  data from the socket.

  @param  buf Ponter to buffer where to place the data read.
  @param  buf_sz Buffer size in bytes.
  @param  flags Ignored, set zero for now.

  @return If succesfull, the function returns ESTATUS_SUCCESS (0). Otherwise if error
          the function returns ESTATUS_FAILED.

****************************************************************************************************
*/
eStatus eOsStream::read(
    os_char *buf,
    os_memsz buf_sz,
    os_int flags)
{
    eStatus s = ESTATUS_SUCCESS;
    osalSelectData selectdata;
    osalEvent trigger = OS_NULL;
    os_boolean set_trigger = OS_FALSE;
    eThread *thrd;
    eStream *strm;
    os_memsz nrd, n;
    os_timer start_t = 0;

    if (m_stream == OS_NULL)
    {
        return ESTATUS_FAILED;
    }

    /* Get Pointer to thread object. Used to interrupt the read if
       the thread is requested to exit.
     */
    thrd = thread();
    if (thrd) {
        trigger = thrd->trigger();
    }

    n = 0;
    while (OS_TRUE)
    {
        /* Try to read the stream.
         */
        s = stream_to_buffer();
        if (s && s != ESTATUS_STREAM_END) break;

        /* Try to get from queue.
         */
        m_in->readx(buf + n, buf_sz, &nrd);
        buf_sz -= nrd;
        n += nrd;
        buf += nrd;
        if (buf_sz <= 0) {
            s = ESTATUS_SUCCESS;
            break;
        }

        if (start_t == 0) {
            os_get_timer(&start_t);
        }
        else {
            if (os_has_elapsed(&start_t, 30000)) /* timeout 30 sec */
            {
                s = ESTATUS_FAILED;
                break;
            }
        }

        /* Let select handle data transfers.
         */
        if (m_use_select) {
            strm = this;
            s = select(&strm, 1, trigger, &selectdata, 2000, OSAL_STREAM_DEFAULT);
            if (s) {
                break;
            }
        }
        else if (s == ESTATUS_STREAM_END && nrd == 0) {
            break;
        }

        if (trigger) {
            if (selectdata.stream_nr ==  OSAL_STREAM_NR_CUSTOM_EVENT) {
                set_trigger = OS_TRUE;
            }

            if (thrd->exitnow()) {
                s = ESTATUS_FAILED;
                break;
            }
        }
    }

    if (set_trigger) {
        osal_event_set(trigger);
    }

#if OSAL_DEBUG
    if (s) {
        osal_debug_error("eOsStream::read failed, corrupted object?");
    }
#endif
    return s;
}


/**
****************************************************************************************************

  @brief Read data from underlying stream to buffer.

  The eOsStream::buffered_read function writes data to socket or other output stream.

  @param   buf Pointer to buffer where to store data read.
  @param   buf_sz Maximum number of bytes to read.
  @param   nread Pointer to integer where to store number of bytes read.
           This is set to number of bytes actually read, which may be less than buf_sz.
           This argument must not be OS_NULL.

  @return  If succesfull, the function returns ESTATUS_SUCCESS (0). Return value
           ESTATUS_STREAM_END (OSAL_END_OF_FILE) indicates that we are reading file
           and end of file has been reached, it is not real error. Other return
           values indicate an error.

****************************************************************************************************
*/
eStatus eOsStream::buffered_read(
    os_char *buf,
    os_memsz buf_sz,
    os_memsz *nread)
{
    osalStatus s;

    if (m_stream == OS_NULL)
    {
        *nread = 0;
        return ESTATUS_FAILED;
    }

    s = m_iface->stream_read(m_stream, buf, buf_sz, nread, OSAL_STREAM_DEFAULT);
    return ESTATUS_FROM_OSAL_STATUS(s);
}




/**
****************************************************************************************************

  @brief Wait for socket or thread event.

  The eOsStream::select() function waits for socket or thread events. Socket evens are typically
  lilke "read": data can be read from socket, "write": data can be written to socket,
  "connect": Socket connected, "close": Socket closed.
  Thread evens indicate that there are messages to the thread to be processed.

  @param   streams Array of socket stream pointers. This function waits for socket events from
           all these streams.
  @oaram   nstreams Number of items in streams array.
  @param   evnt Operating system event to wait for.
  @param   selectdata Pointer to structure in which to fill information about the event.
           This includes error code.
  @param   timeout_ms Maximum time to wait in select, ms. If zero, timeout is not used (infinite).
  @param   flags Reserved, set 0 for now.

  @return  None.

****************************************************************************************************
*/
eStatus eOsStream::select(
    eStream **streams,
    os_int nstreams,
    osalEvent evnt,
    osalSelectData *selectdata,
    os_int timeout_ms,
    os_int flags)
{
    osalStatus s;
    eOsStream **osstreams;
    osalStream osalsock[OSAL_SOCKET_SELECT_MAX];
    os_int i;

    if (m_use_select)
    {
        osstreams = (eOsStream**)streams;

        if (nstreams == 1)
        {
            s = m_iface->stream_select(&osstreams[0]->m_stream, 1, evnt,
                selectdata, timeout_ms, OSAL_STREAM_DEFAULT);
        }
        else
        {
            for (i = 0; i<nstreams; i++)
            {
                osalsock[i] = osstreams[i]->m_stream;
            }

            s = m_iface->stream_select(osalsock, nstreams, evnt,
                selectdata, timeout_ms, OSAL_STREAM_DEFAULT);
        }
        if (s) return ESTATUS_FROM_OSAL_STATUS(s);
    }

    if (m_in) {
        return stream_to_buffer();
    }

    return ESTATUS_SUCCESS;
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
    eStatus es;
    osalStatus osal_s;
    osalStream new_osal_stream;
    os_char remoteip[128];

    new_osal_stream = m_iface->stream_accept(m_stream, remoteip, sizeof(remoteip),
        &osal_s, OSAL_STREAM_DEFAULT);

    if (new_osal_stream)
    {
        /* Save OSAL socket handle
         */
        new_stream = new eOsStream(parent, id);
        new_stream->m_stream = new_osal_stream;
        new_stream->m_iface = m_iface;

        es = new_stream->setup_queues(ESTREAM_IN_QUEUE_SZ, ESTREAM_OUT_QUEUE_SZ, flags);
        if (es) {
            delete new_stream;
            new_stream = OS_NULL;
        }

        if (s) {
            *s = es;
        }
        return new_stream;
    }

    if (s) {
        *s = ESTATUS_FROM_OSAL_STATUS(osal_s);
    }

    return OS_NULL;
}

