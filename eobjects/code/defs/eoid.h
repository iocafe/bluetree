/**

  @file    eoid.h
  @brief   Enumeration of object identifiers.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

  This header file defines object identifiers used by eobjects library.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef EOID_H_
#define EOID_H_
#include "eobjects.h"

/**
****************************************************************************************************

  @name Object identifiers used by eobjects library.

  The object identifiers from -99 to -1 are reserved for eobject library. Idenfifiers from
  -199 to -100 are reserved for egui library. Idenfifiers from -1099 to -1000 are for special
  use markings.

****************************************************************************************************
*/

/** This is default object identifier, which specifies object simply as list item.
    If used without parent object, indicates primitive object, typically eVariable, which
    is not part of tree hierarchy. This makes object use less memory
    and to be faster to allocate and free, but it cannot be root or child object.
 */
#define EOID_ITEM -1

/** Attachment: This object is name.
 */
#define EOID_NAME -2

/** Generic identifier for flags.
 */
#define EOID_FLAGS -3

/** Content, used for envelopes, etc.
 */
#define EOID_CONTENT -4

/** Context, used for envelopes, etc.
 */
#define EOID_CONTEXT -5

/** Object's stored properties eContainer.
 */
#define EOID_PROPERTIES -6

/** Object's bindings container.
 */
#define EOID_BINDINGS -7

/** Micellenous parameter.
 */
#define EOID_PARAMETER -8

/** This is temporary attachment which should be mostly igniored..
 */
#define EOID_TEMPORARY -10

/** This object is root helper of object tree.
 */
#define EOID_ROOT_HELPER -11

/** Attachment: This object is namespace.
 */
#define EOID_NAMESPACE -12

/** Attachment: This object pointer target link (ePointer use only).
 */
#define EOID_PPTR_TARGET -13

/** Attachment: This object is general purpose appendix.
 */
#define EOID_APPENDIX -14

/** Path, generic.
 */
#define EOID_PATH -15


/** Table/matrix related identifiers.
 */
#define EOID_TABLE_CONFIGURATION -40
#define EOID_TABLE_COLUMNS -41
#define EOID_TABLE_CONTENT -42
#define EOID_TABLE_WHERE -43
#define EOID_TABLE_NAME -44
#define EOID_TABLE_IX_COLUMN_NAME -45
#define EOID_TABLE_ATTR -46

#define EOID_TABLE_CLIENT_BINDING -55
#define EOID_TABLE_SERVER_BINDING -56

#define EOID_DBM -60

/** This is special object identifier that can be given as argument to object hierarchy
    functions, like first(). The EOID_CHILD specifies to get a child object,
    which is not flagged as attachment.
 */
#define EOID_CHILD -1000

/** This is special object identifier that can be given as argument to object hierarchy
    functions, like first(). The EOID_ALL specifies to get a child object,
    including child objects which are flagged as attachments.
 */
#define EOID_ALL -1001

/** Object identifier EOID_INTERNAL is used for attachments, which are to be ignored
    in loops trough object's children.
 */
#define EOID_INTERNAL -1003


#endif
