/**

  @file    eio_info.cpp
  @brief   Handling signal information from IO device, IO root.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "extensions/io/eio.h"


/** Working state structure while adding signals to dynamic incormation.
 */
typedef struct eioInfoParserState
{
    /** Pointer to iocom root object
     */
    iocRoot *root;

    eioMblkInfo minfo;

    eioSignalInfo sinfo;

    /** Resize memory blocks while parsing flag.
     */
    os_boolean resize_mblks;

    /** Current type as enumeration value, like OS_SHORT. This is set to default
        at beginning of memory block and modified by "type" tag.
     */
    osalTypeId current_type_id;

    /** Maximum address within memory block (first unused).
     */
    os_int max_addr;

    /** Latest information parsed from JSON
     */
    const os_char *tag;             /* Latest tag or key, '-' for array elements */
    const os_char *signal_type_str; /* Signal type specified in JSON, like "short" */
    os_int signal_addr;             /* Signal address as specified in JSON */

    /** For assemblies.
     */
    const os_char *assembly_name;
    const os_char *exp_str;
    const os_char *imp_str;

    /** Trick to get memory block name before processing signals. "groups" position
        is stored here to return to signals after memory block name has been received.
     */
    osalJsonIndex mblk_groups_jindex;
    os_boolean mblk_groups_jindex_set;
    osalJsonIndex mblk_signals_jindex;
    os_boolean mblk_signals_jindex_set;

}
eioInfoParserState;


/**
****************************************************************************************************

  @brief Callback function to add dynamic device information.

  The eioRoot::info_callback() function is called when device information data is received from
  connection or when connection status changes.

  @param   handle Memory block handle.
  @param   start_addr Address of first changed byte.
  @param   end_addr Address of the last changed byte.
  @param   flags Reserved  for future.
  @param   context Application specific pointer passed to this callback function.

  @return  None.

****************************************************************************************************
*/
void eioRoot::info_callback(
    struct iocHandle *handle,
    os_int start_addr,
    os_int end_addr,
    os_ushort flags,
    void *context)
{
    eioRoot *t = (eioRoot*)context;
    iocRoot *root;
    iocMemoryBlock *mblk;
    osalJsonIndex jindex;
    osalStatus s;
    eStatus es;
    eioInfoParserState state;
    OSAL_UNUSED(start_addr);
    OSAL_UNUSED(flags);
    OSAL_UNUSED(context);

    /* If actual data received (not connection status change).
     */
    if (end_addr < 0) return;

    /* Get memory block pointer and start synchronization.
     */
    mblk = ioc_handle_lock_to_mblk(handle, &root);
    if (mblk == OS_NULL) return;

    s = osal_create_json_indexer(&jindex, mblk->buf, mblk->nbytes, 0);
    if (s) goto getout;

    os_memclear(&state, sizeof(state));
    state.root = root;
#if IOC_MBLK_SPECIFIC_DEVICE_NAME
    state.minfo.device_name = mblk->device_name;
    state.minfo.device_nr = mblk->device_nr;
    state.minfo.network_name = mblk->network_name;
#else
    state.minfo.device_name = root->device_name;
    state.minfo.device_nr = root->device_nr;
    state.minfo.network_name = mblk->network_name;
#endif
    state.minfo.eio_root = t;
    state.resize_mblks = OS_TRUE;

    os_lock();

    es = t->process_info_block(&state, osal_str_empty, &jindex);
    if (es) goto getout;

    os_unlock();

getout:
    ioc_unlock(root);
}


/**
****************************************************************************************************

  @brief Processing packed JSON, handle {} blocks.

  The eioRoot::process_info_block() function is called to process a block in packed JSON. General
  goal here is to move IO signals information from packed JSON to dynamic information structures,
  so this information can be seached quickly when needed. Synchronization ioc_lock() must be on
  when this function is called.

  @param   droot Pointer to dynamic information root structure.
  @param   state Structure holding current JSON parsing state.
  @param   array_tag Name of array from upper level of JSON structure.
  @param   jindex Current packed JSON parsing position.
  @return  OSAL_SUCCESS if all is fine, other values indicate an error.

****************************************************************************************************
*/
eStatus eioRoot::process_info_block(
    struct eioInfoParserState *state,
    const os_char *array_tag,
    osalJsonIndex *jindex)
{
    osalJsonItem item;
    osalStatus s;
    eStatus es;
    os_boolean is_signal_block, is_mblk_block, is_assembly_block;
    os_char array_tag_buf[16];

    /* If this is beginning of signal block.
     */
    is_signal_block = OS_FALSE;
    is_mblk_block = OS_FALSE;
    is_assembly_block = OS_FALSE;
    if (!os_strcmp(state->tag, "-"))
    {
        if (!os_strcmp(array_tag, "signals"))
        {
            is_signal_block = OS_TRUE;
            state->signal_addr = -1;
            state->sinfo.n = 1;
            state->sinfo.ncolumns = 1;
            state->signal_type_str = OS_NULL;
            state->sinfo.signal_name = OS_NULL;
        }
        else if (!os_strcmp(array_tag, "mblk"))
        {
            is_mblk_block = OS_TRUE;
            state->sinfo.addr = 0;
            state->max_addr = 0;
            state->current_type_id = OS_USHORT;
        }
        else if (!os_strcmp(array_tag, "assembly"))
        {
            is_assembly_block = OS_TRUE;
            state->sinfo.addr = 0;
            state->max_addr = 0;
            state->current_type_id = OS_USHORT;
        }
    }

    while (!(s = osal_get_json_item(jindex, &item)))
    {
        if (item.code == OSAL_JSON_END_BLOCK)
        {
            /* If end of signal block and we got memory block name, generate the signal
             */
            if (is_signal_block)
            {
                if (state->minfo.mblk_name == OS_NULL || state->sinfo.group_name == OS_NULL) {
                    return ESTATUS_SUCCESS;
                }
                return new_signal_by_info(state);
            }
            if (is_mblk_block && state->resize_mblks)
            {
                resize_memory_block_by_info(state);
            }
            if (is_assembly_block) {
                return new_assembly_by_info(state);
            }
            return ESTATUS_SUCCESS;
        }

        if (item.code == OSAL_JSON_END_ARRAY)
        {
            return ESTATUS_FAILED;
        }

        state->tag = item.tag_name;
        switch (item.code)
        {
            case OSAL_JSON_START_BLOCK:
                es = process_info_block(state, array_tag, jindex);
                if (es) return es;
                break;

            case OSAL_JSON_START_ARRAY:
                os_strncpy(array_tag_buf, state->tag, sizeof(array_tag_buf));
                if (!os_strcmp(array_tag_buf, "groups") && state->minfo.mblk_name == OS_NULL) {
                    state->mblk_groups_jindex = *jindex;
                    state->mblk_groups_jindex_set = OS_TRUE;
                }
                if (!os_strcmp(array_tag_buf, "signals") && state->sinfo.group_name == OS_NULL) {
                    state->mblk_signals_jindex = *jindex;
                    state->mblk_signals_jindex_set = OS_TRUE;
                }
                es = process_info_array(state, array_tag_buf, jindex);
                if (es) return es;
                break;

            case OSAL_JSON_VALUE_STRING:
                if (!os_strcmp(state->tag, "name"))
                {
                    if (!os_strcmp(array_tag, "mblk"))
                    {
                        state->minfo.mblk_name = item.value.s;

                        if (state->mblk_groups_jindex_set) {
                            es = process_info_array(state, "groups", &state->mblk_groups_jindex);
                            if (es) return es;
                        }
                    }

                    else if (!os_strcmp(array_tag, "assembly"))
                    {
                        state->assembly_name = item.value.s;
                    }

                    else if (!os_strcmp(array_tag, "groups"))
                    {
                        state->sinfo.group_name = item.value.s;

                        if (state->mblk_signals_jindex_set) {
                            es = process_info_array(state, "signals", &state->mblk_signals_jindex);
                            if (es) return es;
                        }
                    }

                    else if (!os_strcmp(array_tag, "signals"))
                    {
                        state->sinfo.signal_name = item.value.s;
                    }
                }

                else if (!os_strcmp(state->tag, "type"))
                {
                    state->signal_type_str = item.value.s;
                }
                else if (!os_strcmp(state->tag, "exp"))
                {
                    state->exp_str = item.value.s;
                }
                else if (!os_strcmp(state->tag, "imp"))
                {
                    state->imp_str = item.value.s;
                }
                break;

            case OSAL_JSON_VALUE_INTEGER:
                if (!os_strcmp(array_tag, "signals"))
                {
                    if (!os_strcmp(state->tag, "addr"))
                    {
                        state->signal_addr = (os_int)item.value.l;
                    }
                    else if (!os_strcmp(state->tag, "array"))
                    {
                        state->sinfo.n = (os_int)item.value.l;
                    }
                    else if (!os_strcmp(state->tag, "ncolumns"))
                    {
                        state->sinfo.ncolumns = (os_int)item.value.l;
                    }

                }
                break;

            case OSAL_JSON_VALUE_FLOAT:
            case OSAL_JSON_VALUE_NULL:
            case OSAL_JSON_VALUE_TRUE:
            case OSAL_JSON_VALUE_FALSE:
                break;

            default:
                return ESTATUS_FAILED;
        }
    }

    return ESTATUS_SUCCESS;
}


/**
****************************************************************************************************

  @brief Processing packed JSON, handle arrays.

  The ioc_dinfo_process_array() function is called to process array in packed JSON. General goal
  here is to move IO signals information from packed JSON to dynamic information structures,
  so this information can be seached quickly when needed. Synchronization ioc_lock() must be on
  when this function is called.

  @param   droot Pointer to dynamic information root structure.
  @param   state Structure holding current JSON parsing state.
  @param   array_tag Name of array from upper level of JSON structure.
  @param   jindex Current packed JSON parsing position.
  @return  OSAL_SUCCESS if all is fine, other values indicate an error.

****************************************************************************************************
*/
eStatus eioRoot::process_info_array(
    eioInfoParserState *state,
    const os_char *array_tag,
    osalJsonIndex *jindex)
{
    osalJsonItem item;
    osalStatus s;
    eStatus es;

    while (!(s = osal_get_json_item(jindex, &item)))
    {
        if (item.code == OSAL_JSON_END_BLOCK)
        {
            return ESTATUS_FAILED;
        }

        if (item.code == OSAL_JSON_END_ARRAY)
        {
            return ESTATUS_SUCCESS;
        }

        if (!os_strcmp(array_tag, "mblk"))
        {
            state->minfo.mblk_name = OS_NULL;
            state->mblk_groups_jindex_set = OS_FALSE;
        }

        if (!os_strcmp(array_tag, "groups"))
        {
            state->sinfo.group_name = OS_NULL;
            state->mblk_signals_jindex_set = OS_FALSE;
        }

        state->tag = item.tag_name;

        switch (item.code)
        {
            case OSAL_JSON_START_BLOCK:
                es = process_info_block(state, array_tag, jindex);
                if (es) return es;
                break;

            case OSAL_JSON_START_ARRAY:
                es = process_info_array(state, array_tag, jindex);
                if (es) return es;
                break;

            case OSAL_JSON_VALUE_STRING:
            case OSAL_JSON_VALUE_INTEGER:
            case OSAL_JSON_VALUE_FLOAT:
            case OSAL_JSON_VALUE_NULL:
            case OSAL_JSON_VALUE_TRUE:
            case OSAL_JSON_VALUE_FALSE:
                break;

            default:
                return ESTATUS_FAILED;
        }
    }

    return ESTATUS_SUCCESS;
}


/**
****************************************************************************************************

  @brief Add IO signal to dynamic information.

  The new_signal_by_info() function adds a new IO signal to dynamic information. This
  function is called when parting packed JSON in info block. Synchronization ioc_lock()
  must be on when this function is called.

  @param   state Structure holding current JSON parsing state.
  @return  OSAL_SUCCESS if all is fine, other values indicate an error.

****************************************************************************************************
*/
eStatus eioRoot::new_signal_by_info(
    eioInfoParserState *state)
{
    osalTypeId signal_type_id;
    os_int n, sz;

    if (state->signal_type_str)
    {
        signal_type_id = osal_typeid_from_name(state->signal_type_str);
        state->current_type_id = signal_type_id;
    }
    else
    {
        signal_type_id = state->current_type_id;
    }

    /* We must accept address 0 as valid setting mark unspecified address with -1 */
    if (state->signal_addr >= 0)
    {
        state->sinfo.addr = state->signal_addr;
    }

    n = state->sinfo.n;
    if (n < 1) n = 1;

    state->sinfo.flags = signal_type_id;
    new_signal(&state->minfo, &state->sinfo);

    switch(signal_type_id)
    {
        case OS_BOOLEAN:
            if (n == 1)
            {
                state->sinfo.addr++;
            }
            else
            {
                sz = (n + 7)/8 + 1;
                state->sinfo.addr += sz;
            }
            break;

        case OS_STR:
            state->sinfo.addr += n + 1;
            break;

        default:
            sz = (os_int)osal_type_size(signal_type_id);
            state->sinfo.addr += n * sz + 1;
            break;
    }

    /* Record first unused address to allow automatic resizing
     */
    if (state->sinfo.addr > state->max_addr)
    {
        state->max_addr = state->sinfo.addr;
    }

    return ESTATUS_SUCCESS;
}


/**
****************************************************************************************************

  @brief Add an assembly to dynamic information.

  The new_assembly_by_infoinfo()...

  @param   state Structure holding current JSON parsing state.
  @return  OSAL_SUCCESS if all is fine, other values indicate an error.

****************************************************************************************************
*/
eStatus eioRoot::new_assembly_by_info(
    eioInfoParserState *state)
{
    eioAssemblyParams prm;
    eVariable device_id;

    os_memclear(&prm, sizeof(prm));
    prm.name = state->assembly_name;
    prm.type_str = state->signal_type_str;
    prm.exp_str = state->exp_str;
    prm.imp_str = state->imp_str;
    prm.timeout_ms = 0; /* This could come from JSON. 0 selects default timeout. */
    prm.prefix = "rec_"; /* This could come from JSON. */

    device_id = state->minfo.device_name;
    device_id += state->minfo.device_nr;
    new_assembly(device_id.gets(), state->minfo.network_name , &prm);
    return ESTATUS_SUCCESS;
}


/**
****************************************************************************************************

  @brief Processing packed JSON, resize a memory block.

  The ioc_resize_memory_block_by_info() function resized a memory block (By making it bigger,
  if needed. Memory block will never be shrunk). This function is used at IO device to
  configure signals and memory block sizes by information in JSON. Synchronization ioc_lock()
  must be on when this function is called.

  @param   state Structure holding current JSON parsing state.
  @return  None.

****************************************************************************************************
*/
void eioRoot::resize_memory_block_by_info(
    eioInfoParserState *state)
{
    iocRoot *root;
    iocMemoryBlock *mblk;
    os_char *newbuf;
    os_int sz;

    root = state->root;
    sz = state->max_addr;
    if (sz < IOC_MIN_MBLK_SZ) sz = IOC_MIN_MBLK_SZ;

#if IOC_MBLK_SPECIFIC_DEVICE_NAME==0
    if (root->device_nr != state->device_nr) return;
    if (os_strcmp(root->device_name, state->device_name)) return;
#endif

    for (mblk = root->mblk.first;
         mblk;
         mblk = mblk->link.next)
    {
#if IOC_MBLK_SPECIFIC_DEVICE_NAME
        if (mblk->device_nr != state->minfo.device_nr) continue;
        if (os_strcmp(mblk->device_name, state->minfo.device_name)) continue;
#endif
        if (os_strcmp(mblk->mblk_name, state->minfo.mblk_name)) continue;

        if (sz > mblk->nbytes)
        {
            if (mblk->buf_allocated)
            {
                newbuf = ioc_malloc(root, sz, OS_NULL);
                if (newbuf == OS_NULL) return;
                os_memcpy(newbuf, mblk->buf, mblk->nbytes);
                ioc_free(root, mblk->buf, mblk->nbytes);
                mblk->buf = newbuf;
                mblk->nbytes = sz;
            }
#if OSAL_DEBUG
            else
            {
                osal_debug_error("Attempt to resize static memory block");
            }
#endif
        }
        break;
    }
}
