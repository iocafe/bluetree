/**

  @file    estream.h
  @brief   Stream base class.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

  Stream base class sets up general way to interace with different types of streams.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef ESTREAM_H_
#define ESTREAM_H_
#include "eobjects.h"

/**
****************************************************************************************************

  @name Stream Control Character Codes
  @anchor eStreamCtrlChars

  For control character codes. 5 least significant bits must be zero, reserved for
  character repeat count or for version number.

****************************************************************************************************
*/

/** Main control character, starts control sequence while any other character in data is passed
    as is. This value is selected so that three most significant bits are 1 and rest are ramdom.
    So the rest of available control characters are 0x20, 0x40, 0x60, 0x80, 0xA0 and 0xC0.
 */
#define E_STREAM_CTRL_CHAR 0xE5

/** Beginning of object or other block.
 */
#define E_STREAM_CTRLCH_BEGIN_BLOCK 0x20

/** End of object or other block.
 */
#define E_STREAM_CTRLCH_END_BLOCK 0x40

/** Control character in data.
 */
#define E_STREAM_CTRLCH_IN_DATA 0x60

/** Stream has been disconnected.
 */
#define E_STREAM_CTRLCH_DISCONNECT 0x80

/** Stream flushed, contains whole objects.
 */
#define E_STREAM_CTRLCH_FLUSH 0xA0

/** Keep alive character.
 */
#define E_STREAM_CTRLCH_KEEPALIVE 0xC0

/** Mask for separating control character from or repeat count or version number.
 */
#define E_STREAM_CTRLCH_MASK 0xE0

/** Mask for separating repeat count or version number from control character.
 */
#define E_STREAM_COUNT_MASK 0x1F


/**
****************************************************************************************************

  @name Control codes for writechar() and readchar() functions.

  These are much the same as control characted codes, but above 8 bit data range. So these
  can be given as argument to writechar() function or returned by readchar() function (if
  the underlying stream supports control codes).

****************************************************************************************************
*/

/** Control codes start from here to be above 8 bit range
 */
#define E_STREAM_CTRL_BASE 512

/** Begin object or other block.
 */
#define E_STREAM_BEGIN (E_STREAM_CTRL_BASE + E_STREAM_CTRLCH_BEGIN_BLOCK)

/** End object or other block.
 */
#define E_STREAM_END (E_STREAM_CTRL_BASE + E_STREAM_CTRLCH_END_BLOCK)

/** Stream will be disconnected now.
 */
#define E_STREAM_DISCONNECT (E_STREAM_CTRL_BASE + E_STREAM_CTRLCH_DISCONNECT)

/** Stream flushed, contains whole objects.
 */
#define E_STREAM_FLUSH (E_STREAM_CTRL_BASE + E_STREAM_CTRLCH_FLUSH)

/** Keep alive mark, to be ignored by recepient.
 */
#define E_STREAM_KEEPALIVE (E_STREAM_CTRL_BASE + E_STREAM_CTRLCH_KEEPALIVE)

/** Special return values for readchar() to indicate that buffer has no more data.
 */
#define E_STREM_END_OF_DATA E_STREAM_CTRL_BASE

/** Mask for separating control code from version number.
 */
#define E_STREAM_CTRL_MASK 0xFFE0



/**
****************************************************************************************************
  Default socket port number for eobject communication. TCP ports 6371 - 6375 are unassigned.
****************************************************************************************************
*/

#define ECOM_DEFAULT_SOCKET_PORT 6371
#define ECOM_DEFAULT_SOCKET_PORT_STR "6371"
#define ECOM_DEFAULT_TLS_PORT 6374
#define ECOM_DEFAULT_TLS_PORT_STR "6374"


/**
****************************************************************************************************
  eStream base class.
****************************************************************************************************
*/
typedef struct eStreamOptions {
    os_char *cloud_name;
}
eStreamOptions;

/**
****************************************************************************************************
  eStream base class.
****************************************************************************************************
*/
class eStream : public eObject
{
public:
    /**
    ************************************************************************************************
      Generic eObject functionality.
    ************************************************************************************************
    */
    /* Constructor.
     */
    eStream(
        eObject *parent = OS_NULL,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT);

    /* Casting eObject pointer to eStream pointer.
     */
    inline static eStream *cast(
        eObject *o)
    {
        e_assert_type(o, ECLASSID_STREAM)
        return (eStream*)o;
    }

    /* Get class identifier.
     */
    virtual os_int classid() {return ECLASSID_STREAM; }

    /* Static function to add class to propertysets and class list.
     */
    static void setupclass();

    /**
    ************************************************************************************************
      eStream functionality - writing and reading stream.
    ************************************************************************************************
    */

    /* Open the stream.
     */
    virtual eStatus open(
        const os_char *parameters,
        eStreamOptions *opts = OS_NULL,
        os_int flags = 0)
    {
        return ESTATUS_SUCCESS;
    }

    /* Close the stream.
     */
    virtual eStatus close()
    {
        return ESTATUS_SUCCESS;
    }

    /* Flush written data to stream.
     */
    virtual eStatus flush(
        os_int flags = 0)
    {
        return ESTATUS_SUCCESS;
    }

    /* Write data to stream.
     */
    virtual eStatus write(
        const os_char *buf,
        os_memsz buf_sz)
    {
        return ESTATUS_SUCCESS;
    }

    /* Read data from stream.
     */
    virtual eStatus read(
        os_char *buf,
        os_memsz buf_sz,
        os_int flags = 0)
    {
        return ESTATUS_SUCCESS;
    }

    /* Write character, typically control code.
     */
    virtual eStatus writechar(
        os_int c);

    /* Read character or control code.
     */
    virtual os_int readchar();

    /** Number of incoming flush controls in queue at the moment.
     */
    virtual os_int flushcount()
    {
        return -1;
    }

    /* Wait for stream or thread event.
     */
    virtual eStatus select(
        eStream **streams,
        os_int nstreams,
        osalEvent evnt,
        os_int timeout_ms,
        os_int flags)
    {
        osal_debug_error("select() not implemented for stream class");
        return ESTATUS_FROM_OSAL_STATUS(OSAL_STATUS_NOT_SUPPORTED);
    }

    /* Accept incoming connection.
     */
    virtual eStream *accept(
        os_int flags,
        eStatus *s = OS_NULL,
        eObject *parent = OS_NULL,
        e_oid id = EOID_ITEM)
    {
        if (s) *s = ESTATUS_FAILED;
        return OS_NULL;
    }

    /** Begin an object, etc. block. This is for versioning, data may be added or changed later.
     */
    inline eStatus write_begin_block(
        os_int version)
    {
#if OSAL_DEBUG
        if ((os_uint)version >= 32)
            osal_debug_error("write_begin_block(): version must be 0...31");
#endif
        return writechar(E_STREAM_BEGIN | version);
    }

    /** End an object, etc. block. This skips data added by later versions of object.
     */
    inline eStatus write_end_block()
    {
        return writechar(E_STREAM_END);
    }

    eStatus read_begin_block(
        os_int *version)
    {
        os_int c;

        c = readchar();
        if (version) *version = (c & E_STREAM_COUNT_MASK);
        return (c & E_STREAM_CTRL_MASK) == E_STREAM_BEGIN
             ? ESTATUS_SUCCESS : ESTATUS_FAILED;
    }

    /** End an object, etc. block. This skips data added by later versions of object.
     */
    eStatus read_end_block();

    /* Write long integer value to stream.
     */
    eStatus putl(
        os_long x);

    /* Write float value to stream.
     */
    eStatus putf(
        os_float x);

    /* Write double precision floating point value to stream.
     */
    eStatus putd(
        os_double x);

    /* Write string to stream.
     */
    eStatus puts(
        const os_char *x);

    /* Write string from variable to stream.
     */
    eStatus puts(
        eVariable *x);

    /* Get long integer value from stream.
     */
    eStatus getl(
        os_long *x);

    /* Get float value from stream.
     */
    eStatus getf(
        os_float *x);

    /* Get double value from stream.
     */
    eStatus getd(
        os_double *x);

    /* Get string value from stream into variable.
     */
    eStatus gets(
        eVariable *x);

    /* Get OSAL stream handle, or OS_NULL if there is no OSAL handle.
     */
    virtual osalStream osstream() { return OS_NULL; }


    /**
    ************************************************************************************************

      @name Operator overloads

      The operator overloads are implemented for convinience, and map to the member functions.
      Using operator overloads may lead to more readable code, but may also confuse the
      reader.

    ************************************************************************************************
    */

    /** Operator "<<", storing values to stream.
     */
    inline eStatus operator<<(const os_char x) { return putl(x); }
    inline eStatus operator<<(const os_uchar x) { return putl(x); }
    inline eStatus operator<<(const os_short x) { return putl(x); }
    inline eStatus operator<<(const os_ushort x) { return putl(x); }
    inline eStatus operator<<(const os_int x) { return putl(x); }
    inline eStatus operator<<(const os_uint x) { return putl(x); }
    inline eStatus operator<<(const os_long x) { return putl(x); }
    inline eStatus operator<<(const os_float x) { return putf(x); }
    inline eStatus operator<<(const os_double x) { return putd(x); }
    inline eStatus operator<<(const os_char *x) { return puts(x); }
    inline eStatus operator<<(eVariable& x) { return puts(&x); }

    /** Operator ">>", getting values from stream.
     */
    inline eStatus operator>>(os_char& x) { os_long tx; eStatus s; s = getl(&tx); x=(os_char)tx; return s; }
    inline eStatus operator>>(os_uchar& x) { os_long tx; eStatus s; s = getl(&tx); x=(os_uchar)tx; return s; }
    inline eStatus operator>>(os_short& x) { os_long tx; eStatus s; s = getl(&tx); x=(os_short)tx; return s; }
    inline eStatus operator>>(os_ushort& x) { os_long tx; eStatus s; s = getl(&tx); x=(os_ushort)tx; return s; }
    inline eStatus operator>>(os_int& x) { os_long tx; eStatus s; s = getl(&tx); x=(os_int)tx; return s; }
    inline eStatus operator>>(os_uint& x) { os_long tx; eStatus s; s = getl(&tx); x=(os_uint)tx; return s; }
    inline eStatus operator>>(os_long& x) { return getl(&x); }
    inline eStatus operator>>(os_float& x) { return getf(&x); }
    inline eStatus operator>>(os_double& x) { return getd(&x); }
    inline eStatus operator>>(eVariable& x) { return gets(&x); }
};

#endif
