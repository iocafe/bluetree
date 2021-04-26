/**

  @file    equeue.h
  @brief   Queue buffer class.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

  Queue buffers data, typically for reading from or writing to stream. See equeue.cpp for more
  information.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef EQUEUE_H_
#define EQUEUE_H_
#include "eobjects.h"

/** Memory is buffered as blocks within queue. Block header structure is:
 */
typedef struct eQueueBlock
{
    /** Pointer to the next older queued block.
     */
    struct eQueueBlock *older;

    /** Pointer to the next newer queued block.
     */
    struct eQueueBlock *newer;

    /** Queue head index (inside newest)
     */
    os_int head;

    /** Queue tail index (inside oldest)
     */
    os_int tail;

    /** Size of queued block excluting the header.
     */
    os_int sz;
}
eQueueBlock;


/**
****************************************************************************************************
  Queue stream data.
****************************************************************************************************
*/
class eQueue : public eStream
{
public:
    /**
    ************************************************************************************************
      Generic eObject functionality.
    ************************************************************************************************
    */
    /* Constructor.
     */
    eQueue(
        eObject *parent = OS_NULL,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT);

    /* Virtual destructor.
     */
    virtual ~eQueue();

    /* Casting eObject pointer to eQueue pointer.
     */
    inline static eQueue *cast(
        eObject *o)
    {
        e_assert_type(o, ECLASSID_QUEUE)
        return (eQueue*)o;
    }

    /* Get class identifier.
    */
    virtual os_int classid() {return ECLASSID_QUEUE; }

    /* Static function to add class to propertysets and class list.
     */
    static void setupclass();

    /* Static constructor function.
    */
    static eQueue *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new eQueue(parent, id, flags);
    }


    /**
    ************************************************************************************************
      eStream functionality.
    ************************************************************************************************
    */
    /* Open the queue.
     */
    virtual eStatus open(
        const os_char *parameters,
        os_int flags = 0);

    /* Close the queue.
     */
    virtual eStatus close();

    /* Write data to queue.
     */
    virtual eStatus write(
        const os_char *buf,
        os_memsz buf_sz);

    /* Read data from queue (Extended function, returns nread).
     */
    eStatus readx(
        os_char *buf,
        os_memsz buf_sz,
        os_memsz *nread,
        os_int flags = 0);

    /* Read data from queue.
     */
    virtual inline eStatus read(
        os_char *buf,
        os_memsz buf_sz,
        os_int flags)
    {
        os_memsz nread;
        eStatus s = readx(buf, buf_sz, &nread, flags);
        if (nread != buf_sz) s = ESTATUS_FAILED;
        return s;
    }

    /* Write character to queue, typically control code.
     */
    virtual eStatus writechar(
        os_int c);

    /* Read character or control code.
     */
    virtual os_int readchar();

    os_memsz bytes();

    /** Number of incoming flush controls in queue at the moment. Requires OSAL_FLUSH_CTRL_COUNT
        and OSAL_STREAM_DECODE_ON_READ flags for open().
     */
    virtual os_int flushcount()
    {
        return m_flush_count;
    }

private:

    /**
    ************************************************************************************************
      Internal functions.
    ************************************************************************************************
    */
    /* Allocate new block and join it to queue.
     */
    void newblock();

    /* Detach oldest block from queue and free memory allocatd for it.
     */
    void delblock();

    /* Write data to queue, encode while writing.
     */
    eStatus write_encoded(
        const os_char *buf,
        os_memsz buf_sz);

    /* Write data to queue without modification.
     */
    eStatus write_plain(
        const os_char *buf,
        os_memsz buf_sz);

    /** Put character to queue.
     */
    inline eStatus putcharacter(
        os_int c)
    {
        os_int nexthead;

        /* Get next head.
         */
        nexthead = m_newest->head + 1;
        if (nexthead >= m_newest->sz) nexthead = 0;

        /* If this block is full, move on to next block
         */
        if (nexthead == m_newest->tail)
        {
            if (m_nblocks >= m_max_blocks) {
                return ESTATUS_BUFFER_OVERFLOW;
            }

            newblock();
            nexthead = 1;
        }

        *(((os_char*)m_newest) + sizeof(eQueueBlock) + m_newest->head) = (os_char)c;
        m_newest->head = nexthead;

        return ESTATUS_SUCCESS;
    }

    /* Finish with last write so also previous character has been processed.
     */
    eStatus complete_last_write();

    /* Read and decode data.
     */
    void read_decoded(
        os_char *buf,
        os_memsz buf_sz,
        os_memsz *nread);

    /* Read data as is.
     */
    void read_plain(
        os_char *buf,
        os_memsz buf_sz,
        os_memsz *nread,
        os_int flags);

    /** Check if queue has data.
     */
    inline os_char hasedata()
    {
        if (m_newest != m_oldest) return OS_TRUE;
        return m_oldest->head != m_oldest->tail;
    }

    /** Get character from queue.
     */
    inline os_char getcharacter()
    {
        os_char c;
        os_int nexttail;

        c = *(((os_char*)m_oldest) + sizeof(eQueueBlock) + m_oldest->tail);

        /* Get next head.
         */
        nexttail = m_oldest->tail + 1;
        if (nexttail >= m_oldest->sz) nexttail = 0;
        m_oldest->tail = nexttail;

        /* If this block is now empty, and it is not only block, delete it.
         */
        if (nexttail == m_oldest->head)
        {
            if (m_oldest != m_newest) delblock();
        }

        return c;
    }

    /**
    ************************************************************************************************
      Member variables.
    ************************************************************************************************
    */

    /** Oldest block in the queue.
     */
    eQueueBlock *m_oldest;

    /** Latest block added to queue.
     */
    eQueueBlock *m_newest;

    /** Number of bytes in the queue.
     */
    os_memsz m_bytes;

    /** Flags for the queue. Flags OSAL_STREAM_ENCODE_ON_WRITE and
        OSAL_STREAM_DECODE_ON_READ are observed.
     */
    os_int m_flags;

    /** Maximum queue memory allocation in bytes.
     */
    os_int m_max_blocks;

    /** Current memory allocation in bytes.
     */
    os_int m_nblocks;

    /** Previous character
     */
    os_int m_wr_prevc;

    /** Number of times same has repeated afterwards (0 = character has occurred once).
     */
    os_int m_wr_count;

    /** Repeat count when unpacking repeated character (RLE).
     */
    os_int m_rd_repeat_count;

    /** Repeated character.
     */
    os_int m_rd_repeat_char;

    /** Previous character, while reading.
     */
    os_int m_rd_prevc;

    /** The character before that one.
     */
    os_int m_rd_prev2c;

    /** Number of incoming flush controls in queue at the moment.
     */
    os_int m_flush_count;

    /** Last character of previous write_plain() call.
     */
    os_uchar m_flushctrl_last_c;
};

#endif
