/**

  @file    eobject_callback.cpp
  @brief   Object base class, callbacks.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  When variable value changes, new item is added to container, etc, the parent object may need
  to get informed.

  A class which needs to receive callbacks needs to overload eObject::oncallback() function.

  A object which has EOBJ_PERSISTENT_CALLBACK or EOBJ_TEMPORARY_CALLBACK flag set calls
  eObject::docallback to keep parent informed about changes.

  Difference of EOBJ_PERSISTENT_CALLBACK and EOBJ_TEMPORARY_CALLBACK is that the
  EOBJ_PERSISTENT_CALLBACK flags gets serialized and cloned with object, while
  the EOBJ_TEMPORARY_CALLBACK does not. Otherwise the effect of the flags is identical.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "eobjects.h"


/**
****************************************************************************************************

   @brief Process a callback from child object

   A child object can alert parent object by calling docallback(), which calls this this function
   of the parent object. This generic implementation passess the callback to parent object, if
   this object has either EOBJ_PERSISTENT_CALLBACK or EOBJ_TEMPORARY_CALLBACK flag set. If
   callback is not processed by parent object, EOBJ_TEMPORARY_CALLBACK flag is turned off.

   @param   event Reason for the callback, for example ECALLBACK_VARIABLE_VALUE_CHANGED
   @param   obj Pointer to the child object.
   @param   appendix Additional information about callback, event specific.

   @return  The callback function returns ESTATUS_SUCCESS, except if callback function is not
            implemented for the parent object class (default implementation).

****************************************************************************************************
*/
eStatus eObject::oncallback(
    eCallbackEvent event,
    eObject *obj,
    eObject *appendix)
{
    /* If we need to pass callback to parent class.
     */
    if (flags() & (EOBJ_PERSISTENT_CALLBACK|EOBJ_TEMPORARY_CALLBACK)) {
        eObject *p = parent();
        if (p) {
            if (p->oncallback(event, this, obj) == ESTATUS_SUCCESS) {
                return ESTATUS_SUCCESS;
            }
            clearflags(EOBJ_TEMPORARY_CALLBACK);
        }
    }

    /* Failed status indicates that the oncallback function is not implemented.
     */
    return ESTATUS_FAILED;
}


/**
****************************************************************************************************

   @brief Call parent object's docallback function.

   A child object can alert parent object by calling docallback(), which calls this this function
   of the parent object.

   This alerts parent object about a change of value, new child object or deleted child object,
   etc. Object's flag EOBJ_PERSISTENT_CALLBACK or EOBJ_TEMPORARY_CALLBACK selects if an object
   should call parent's oncallback function.

   @param   event Reason for the callback, for example ECALLBACK_VARIABLE_VALUE_CHANGED
   @param   appendix Additional information about callback, event specific.

   @return  The callback function returns ESTATUS_SUCCESS, except if callback function is not
            implemented for the parent object class (default implementation).
   @return  None.

****************************************************************************************************
*/
void eObject::docallback(
    eCallbackEvent event,
    eObject *appendix)
{
    if (flags() & (EOBJ_PERSISTENT_CALLBACK|EOBJ_TEMPORARY_CALLBACK))
    {
        eObject *p = parent();
        if (p) {
            if (p->oncallback(event, this, appendix) != ESTATUS_SUCCESS) {
                clearflags(EOBJ_TEMPORARY_CALLBACK);
            }
        }
    }
}
