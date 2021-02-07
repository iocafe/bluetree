/**

  @file    eio_root.h
  @brief   Object representing and IO root.
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
#ifndef EIO_ROOT_H_
#define EIO_ROOT_H_
#include "extensions/io/eio.h"

struct eioInfoParserState;

/**
****************************************************************************************************
  Defines
****************************************************************************************************
*/

typedef struct eioMblkInfo {
    const os_char *device_name;
    os_uint device_nr;
    const os_char *mblk_name;
    const os_char *network_name;

    iocMemoryBlock *mblk;
    iocRoot *root;
}
eioMblkInfo;

typedef struct eioSignalInfo {
    const os_char *signal_name;
    const os_char *group_name;
    os_int addr;
    os_int n;                   /* Number of elements in array, 1 if not array */
    os_int ncolumns;            /* Number of columns when array holds matrix, 1 otherwise. */
}
eioSignalInfo;


/**
****************************************************************************************************
  eioRoot is like a box of objects.
****************************************************************************************************
*/
class eioRoot : public eContainer
{
public:
    /* Constructor.
     */
    eioRoot(
        eObject *parent = OS_NULL,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT);

    /* Virtual destructor.
     */
    virtual ~eioRoot();

    /* Clone object.
     */
    virtual eObject *clone(
        eObject *parent,
        e_oid id = EOID_CHILD,
        os_int aflags = 0);

    /* Casting eObject pointer to eioRoot pointer.
     */
    inline static eioRoot *cast(
        eObject *o)
    {
        e_assert_type(o, ECLASSID_EIO_ROOT)
        return (eioRoot*)o;
    }

    /* Get class identifier.
     */
    virtual os_int classid() {return ECLASSID_EIO_ROOT; }

    /* Static function to add class to propertysets and class list.
     */
    static void setupclass();

    /* Static constructor function for generating instance by class list.
     */
    static eioRoot *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new eioRoot(parent, id, flags);
    }

    /* Function to process incoming messages.
     */
    virtual void onmessage(
        eEnvelope *envelope);

    /* Called when property value changes.
     */
    virtual eStatus onpropertychange(
        os_int propertynr,
        eVariable *x,
        os_int flags);

    /* A callback by a child object.
     */
    virtual eStatus oncallback(
        eCallbackEvent event,
        eObject *obj,
        eObject *appendix);

    /**
    ************************************************************************************************
      X..
    ************************************************************************************************
    */

    /* Connect root object to IOCOM.
     */
    void setup(iocRoot *iocom_root);

    inline iocRoot *iocom_root() {return m_iocom_root; }

    /** IO thread handle.
     */
    eThreadHandle m_io_thread_handle;

protected:
    /**
    ************************************************************************************************
      Internal functions, eio_root.cpp
    ************************************************************************************************
    */

    /* Root callback function (process network and device connect/disconnect, etc).
     */
    static void io_root_callback(
        struct iocRoot *root,
        iocEvent event,
        struct iocDynamicNetwork *dnetwork,
        struct iocMemoryBlock *mblk,
        void *context);

    /* Find or create a IO network object.
     */
    eioMblk *connected(
        eioMblkInfo *minfo);

    /* Mark network object disconnected and delete it, if it is unused.
     */
    void disconnected(
        eioMblkInfo *minfo);

    eioVariable *new_signal(
        eioMblkInfo *minfo,
        eioSignalInfo *sinfo,
        os_int flags);

    /**
    ************************************************************************************************
      Internal functions, eio_info.cpp
    ************************************************************************************************
    */
    /* Callback function to add dynamic device information.
     */
    static void info_callback(
        struct iocHandle *handle,
        os_int start_addr,
        os_int end_addr,
        os_ushort flags,
        void *context);

    eStatus process_info_block(
        struct eioInfoParserState *state,
        const os_char *array_tag,
        osalJsonIndex *jindex);

    eStatus process_info_array(
        eioInfoParserState *state,
        const os_char *array_tag,
        osalJsonIndex *jindex);

    eStatus ioc_new_signal_by_info(
        eioInfoParserState *state);

    void resize_memory_block_by_info(
        eioInfoParserState *state);


    /* Flags the peristent object changed (needs to be saved).
     */
    // void touch();


    /**
    ************************************************************************************************
      Member variables
    ************************************************************************************************
    */

    /** IOCOM root object!
     */
    iocRoot *m_iocom_root;

};


/* Initialize IO network structure classes and start IO thread.
 */
eioRoot *eio_initialize(
    iocRoot *iocom_root,
    eObject *parent);

/* Stop IO thread.
 */
void eio_stop_io_thread(
    eioRoot *eio_root);

#endif
