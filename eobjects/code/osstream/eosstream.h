/**

  @file    eosstream.h
  @brief   Wrap EOSAL stream as eStream object.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef EOSSTREAM_H_
#define EOSSTREAM_H_
#include "eobjects.h"

class eQueue;

/**
****************************************************************************************************
  EOSAL library stream as eobjects stream.
****************************************************************************************************
*/
class eOsStream : public eStream
{
public:
    /**
    ************************************************************************************************
      Generic eObject functionality.
    ************************************************************************************************
    */
    /* Constructor.
     */
    eOsStream(
        eObject *parent = OS_NULL,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT);

    /* Virtual destructor.
     */
    virtual ~eOsStream();

    /* Casting eObject pointer to eOsStream pointer.
     */
    inline static eOsStream *cast(
        eObject *o)
    {
        e_assert_type(o, ECLASSID_OSSTREAM)
        return (eOsStream*)o;
    }

    /* Get class identifier.
    */
    virtual os_int classid() {return ECLASSID_OSSTREAM; }

    /* Static function to add class to propertysets and class list.
     */
    static void setupclass();

    /* Static constructor function.
    */
    static eOsStream *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new eOsStream(parent, id, flags);
    }


    /**
    ************************************************************************************************
      eStream functionality.
    ************************************************************************************************
    */
    /* Open socket.
     */
    virtual eStatus open(
        os_char *parameters,
        os_int flags = 0);

    /* Close socket.
     */
    virtual eStatus close();

    /* Flush written data to socket.
     */
    virtual eStatus flush(
        os_int flags = 0);

    /* Write data to stream.
     */
    virtual eStatus write(
        const os_char *buf,
        os_memsz buf_sz,
        os_memsz *nwritten = OS_NULL);

    /* Read data from stream.
     */
    virtual eStatus read(
        os_char *buf,
        os_memsz buf_sz,
        os_memsz *nread = OS_NULL,
        os_int flags = 0);

    /* Write character, typically control code.
     */
    virtual eStatus writechar(
        os_int c);

    /* Read character or control code.
     */
    virtual os_int readchar();

    /* Number of incoming flush controls in queue at the moment.
     */
    /* virtual os_int flushcount()
    {
        if (m_in) return m_in->flushcount();
        return -1;
    } */

    /* Wait for socket or thread event.
     */
    virtual void select(
        eStream **streams,
        os_int nstreams,
        osalEvent evnt,
        osalSelectData *selectdata,
        os_int flags);

    /* Accept incoming connection.
     */
    virtual eStream *accept(
        os_int flags,
        eStatus *s = OS_NULL,
        eObject *parent = OS_NULL,
        e_oid id = EOID_ITEM);

    virtual eStatus accept(
        eStream *newstream,
        os_int flags) {return ESTATUS_FAILED;}


protected:

    /**
    ************************************************************************************************
      Member variables.
    ************************************************************************************************
    */
    /* EOSAL stream
     */
    osalStream m_stream;
};

#endif
