/**

  @file    estatus.h
  @brief   Enumeration of status codes.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

  Many eobjects library functions return status codes. Enumeration eStatus defines these
  codes.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef ESTATUS_H_
#define ESTATUS_H_
#include "eobjects.h"

/**
****************************************************************************************************

  @brief Enumeration of status codes.

  Many eobjects library functions return status codes. Enumeration eStatus defines these
  codes. These and EOSAL status codes can be mixed.

  - Status codes from 0 to 49 are reserved for EOSAL reutrn codes which do not indicate an error.
  - Status codes from 50 to 99 are reseved for EOBJECTS return codes which do not indicate an error.
  - Status codes from 100 to 399 are reserved for EOSAL error codes.
  - Status codes from 500 to 799 are reserved for EOBJECTS error codes.
  - Commonly used ESTATUS_SUCCESS and ESTATUS_FAILED are same as OSAL_SUCCESS and
    OSAL_STATUS_FAILED and use EOSAL status values.

****************************************************************************************************
*/
typedef enum
{
    /** Success. The ESTATUS_SUCCESS (0) is returned when function call succeeds.
     */
    ESTATUS_SUCCESS = OSAL_SUCCESS,

    /** Same as ESTATUS_SUCCESS but indicates that nothing was actually done. This is used
     *  by few functions to indicate that no change was made and further processing
     *  of change is unnecessary.
     */
    ESTATUS_NO_CHANGES = OSAL_NOTHING_TO_DO,

    /** General operation pending (not indicating an error).
     */
    ESTATUS_PENDING = OSAL_PENDING,

    /** Accept stream function: No new incoming connection.
     */
    ESTATUS_NO_NEW_CONNECTION = OSAL_NO_NEW_CONNECTION,

    /** General purpose FALSE.
     */
    ESTATUS_FALSE = 50,

    /** No more data available from stream. Like end of file or
     *  end of memory buffer or end of file.
     */
    ESTATUS_STREAM_END = OSAL_END_OF_FILE,

    /** Unidentified failure.
     */
    ESTATUS_FAILED = OSAL_STATUS_FAILED,

    /** Operation is not supported for this operating system/hardware platform/etc.
     */
    ESTATUS_NOT_SUPPORTED = OSAL_STATUS_NOT_SUPPORTED,

    /** Object properties, properties not supported for the class.
     */
    ESTATUS_NO_CLASS_PROPERTY_SUPPORT = 500,

    /** Object serialization. Writing object to stream has failed.
     */
    ESTATUS_WRITING_OBJ_FAILED = 505,

    /** Object serialization. Reading object from stream has failed.
     */
    ESTATUS_READING_OBJ_FAILED = 510,

    /** Names, mapping name to name space failed.
     */
    ESTATUS_NAME_MAPPING_FAILED = 520,

    /** Names, name already mapped, skipping remapping.
     */
    ESTATUS_NAME_ALREADY_MAPPED = 525,

    /** Properties, simpleproperty() did not find property.
     */
    ESTATUS_NO_SIMPLE_PROPERTY_NR = 530,

    /** Properties, onpropertychange() did not find property.
     */
    ESTATUS_UNKNOWN_PROPERTY_NR = 531,

    /** Reading float from stream failed
     */
    ESTATUS_STREAM_FLOAT_ERROR = 535,

    /** Special case, we received invisible flush count character which changed
        the flush count to zero (no more whole objects buffered in stream).
        Only returned by eEnvelope::reader().
     */
    ESTATUS_NO_WHOLE_MESSAGES_TO_READ = 540,

    /** Generic timeout code.
     */
    ESTATUS_TIMEOUT = 550,

    /** Buffer overflow, streams, eQueue
     */
    ESTATUS_BUFFER_OVERFLOW = 555
}
eStatus;

/* Macro to type cast EOSAL status to eStatus
 */
#define ESTATUS_FROM_OSAL_STATUS(s) ((eStatus)(s))

/* Macro to determine if returned status is error.
 */
#define ESTATUS_IS_ERROR(s) ((s) >= ESTATUS_FAILED)


#endif
