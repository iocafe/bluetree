/**

  @file    eio_root.h
  @brief   Object representing and IO root.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

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
struct eioAssemblyParams;

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

    iocRoot *root;              /* Pointer to iocom root object */
    iocMemoryBlock *mblk;       /* Pointer to IOCOM memory block */
    eioRoot *eio_root;          /* IO object hierarchy root (time stamps). */
}
eioMblkInfo;

typedef struct eioSignalInfo {
    const os_char *signal_name;
    const os_char *group_name;
    os_int addr;
    os_int n;                   /* Number of elements in array, 1 if not array */
    os_int ncolumns;            /* Number of columns when array holds matrix, 1 otherwise. */
    os_int flags;               /* Signal type + flags */
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


    /**
    ************************************************************************************************
      IO root functions.
    ************************************************************************************************
    */
    /* Connect root object to IOCOM.
     */
    void setup(iocRoot *iocom_root);

    inline iocRoot *iocom_root() {return m_iocom_root; }

    inline void set_time_now(os_long ti) { m_time_now = ti; }
    inline os_long time_now() {return m_time_now; }

    inline osalEvent io_trigger() {return m_io_trigger; }
    inline void save_io_trigger(osalEvent io_trigger) {m_io_trigger = io_trigger; }
    inline void trig_io() {if (m_io_trigger) osal_event_set(m_io_trigger); }

    void run(os_long ti);

    /* Add or remove an assebly to run list.
     */
    void assembly_to_run_list(
        eioAssembly *assembly,
        os_boolean enable);

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

    eioNetwork *get_network(
        const os_char *network_name);

    /* Mark network object disconnected and delete it, if it is unused.
     */
    void disconnected(
        eioMblkInfo *minfo);

    void new_signal(
        eioMblkInfo *minfo,
        eioSignalInfo *sinfo);

    void new_assembly(
        const os_char *device_id,
        const os_char *network_name,
        struct eioAssemblyParams *prm);

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

    eStatus new_signal_by_info(
        eioInfoParserState *state);

    eStatus new_assembly_by_info(
        eioInfoParserState *state);

    void resize_memory_block_by_info(
        eioInfoParserState *state);

    /**
    ************************************************************************************************
      Member variables
    ************************************************************************************************
    */

    /** IOCOM root object!
     */
    iocRoot *m_iocom_root;

    /* Time stamp now.
     */
    os_long m_time_now;

    osalEvent m_io_trigger;

    /* List of ePointers to assemblies to run.
     */
    eContainer *m_run_assemblies;
};


/* Initialize IO network structure classes and start IO thread.
 */
eioRoot *eio_initialize(
    iocRoot *iocom_root,
    eObject *parent);

#endif
