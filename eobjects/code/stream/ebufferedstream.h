/**

  @file    ebufferedstream.h
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
#pragma once
#ifndef EBUFFEREDSTREAM_H_
#define EBUFFEREDSTREAM_H_
#include "eobjects.h"


/**
****************************************************************************************************
  eBufferedStream base class.
****************************************************************************************************
*/
class eBufferedStream : public eStream
{
public:
    /**
    ************************************************************************************************
      Generic eObject functionality.
    ************************************************************************************************
    */
    /* Constructor.
     */
    eBufferedStream(
        eObject *parent = OS_NULL,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT);

    /* Virtual destructor.
     */
    virtual ~eBufferedStream();

    /* Casting eObject pointer to eBufferedStream pointer.
     */
    inline static eBufferedStream *cast(
        eObject *o)
    {
        e_assert_type(o, ECLASSID_BUFFERED_STREAM)
        return (eBufferedStream*)o;
    }

    /* Get class identifier.
     */
    virtual os_int classid() {return ECLASSID_BUFFERED_STREAM; }

    /**
    ************************************************************************************************
      eStream functionality.
    ************************************************************************************************
    */

    /* Write character, typically control code.
     */
    virtual eStatus writechar(
        os_int c);

    /* Read character or control code.
     */
    virtual os_int readchar();

    /**
    ************************************************************************************************
     Buffering, in and out queues.
    ************************************************************************************************
    */

    /* Set up queue buffers and select optional encoding.
     */
    eStatus setup_queues(
        os_memsz in_sz,
        os_memsz out_sz,
        os_int flags);

    void delete_queues();


    /* Write data to stream.
     */
    eStatus write_out_queue(
        const os_char *buf,
        os_memsz buf_sz,
        os_memsz *nwritten = OS_NULL)
    {
        if (nwritten != OS_NULL) *nwritten = 0;
        return ESTATUS_SUCCESS;
    }

    /* Read data from stream.
     */
    eStatus get_out_queue(
        os_char *buf,
        os_memsz buf_sz,
        os_memsz *nread = OS_NULL,
        os_int flags = 0)
    {
        if (nread != OS_NULL) *nread = 0;
        return ESTATUS_SUCCESS;
    }

protected:
    /* Actually write to to implementing stream (called by flush())
     */
    virtual eStatus buffered_write(
        const os_char *buf,
        os_memsz buf_sz,
        os_memsz *nwritten)
    {
        osal_debug_error("buffered_write not implemented by derived class");
        return ESTATUS_FAILED;
    }

    virtual eStatus buffered_read(
        os_char *buf,
        os_memsz buf_sz,
        os_memsz *nread)
    {
        osal_debug_error("buffered_read not implemented by derived class");
        return ESTATUS_FAILED;
    }

    eStatus buffer_to_stream(
        os_boolean flushnow);

    eStatus stream_to_buffer();

    /** Input queue (buffer).
     */
    eQueue *m_in;

    /** Output queue (buffer).
     */
    eQueue *m_out;

    /** We start sending after buffering 3900 bytes even there is more coming.
     */
    os_memsz m_send_size;

    /** Saved flags.
     */
    os_int m_flags;

    os_boolean m_flushnow;
};

#endif
