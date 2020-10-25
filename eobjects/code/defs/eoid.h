/**

  @file    eoid.h
  @brief   Enumeration of object identifiers.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

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
    This flag is argument for costructore
 */
#define EOID_ITEM -1

/** This is default object identifier for containers, threads and also specifies object as
    list item. But if used without parent object, indicates top of new object tree.
 */
/* #define EOID_RITEM -2 */

/** This object is root helper of object tree.
 */
#define EOID_ROOT_HELPER -3

/** Attachment: This object is name.
 */
#define EOID_NAME -10

/** Attachment: This object is namespace.
 */
#define EOID_NAMESPACE -11

/** Attachment: This object pointer target link (ePointer use only).
 */
#define EOID_PPTR_TARGET -12

/** Attachment: This object is general purpose appendix.
 */
#define EOID_APPENDIX -13

/** Generic identifier for flags.
 */
#define EOID_FLAGS -15

/** Content, used for envelopes, etc.
 */
#define EOID_CONTENT -30

/** Context, used for envelopes, etc.
 */
#define EOID_CONTEXT -31

/** Object's stored properties eContainer.
 */
#define EOID_PROPERTIES -33

/** Object's bindings container.
 */
#define EOID_BINDINGS -35

/** Table/matrix related identifiers.
 */
#define EOID_TABLE_CONFIGURATION -40
#define EOID_TABLE_COLUMNS -41
#define EOID_TABLE_CONTENT -42
#define EOID_TABLE_WHERE -43
#define EOID_TABLE_NAME -44
#define EOID_TABLE_ATTR -45

#define EOID_TABLE_CLIENT_BINDING -46
#define EOID_TABLE_SERVER_BINDING -47

#define EOID_DBM -50

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
