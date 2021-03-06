/**

  @file    eio_assembly.h
  @brief   Assembly - collection of signals with specific functionality.
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
#ifndef EIO_ASSEMBLY_H_
#define EIO_ASSEMBLY_H_
#include "extensions/io/eio.h"

class eioDevice;

/**
****************************************************************************************************
  Defines
****************************************************************************************************
*/

typedef struct eioAssemblyParams
{
    const os_char *name;
    const os_char *type_str;
    const os_char *exp_str;
    const os_char *imp_str;
    os_int timeout_ms;
}
eioAssemblyParams;

/**
****************************************************************************************************
  eioAssembly is like a box of objects.
****************************************************************************************************
*/
class eioAssembly : public eVariable
{
public:
    /* Constructor.
     */
    eioAssembly(
        eObject *parent = OS_NULL,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT);

    /* Casting eObject pointer to eioAssembly pointer.
     */
    inline static eioAssembly *cast(
        eObject *o)
    {
        e_assert_type(o, ECLASSID_EIO_ASSEMBLY)
        return (eioAssembly*)o;
    }

    /* Get class identifier.
     */
    virtual os_int classid() {return ECLASSID_EIO_ASSEMBLY; }

    /* Static function to add class to propertysets and class list.
     */
    static void setupclass();

    /* Called when property value changes.
     */
    virtual eStatus onpropertychange(
        os_int propertynr,
        eVariable *x,
        os_int flags);

    /* Get value of simple property.
     */
    virtual eStatus simpleproperty(
        os_int propertynr,
        eVariable *x);

    /* Process a callback from a child object.
     */
    eStatus oncallback(
        eCallbackEvent event,
        eObject *obj,
        eObject *appendix);

    /**
    ************************************************************************************************
      Assembly specific functions.
    ************************************************************************************************
    */

    /* Set up a newly created eioAssebly, meningfull implementation in derived classes.
     */
    virtual eStatus setup(
        eioAssemblyParams *prm,
        iocRoot *iocom_root) = 0;

    virtual void run(os_long ti) = 0;


protected:
    /**
    ************************************************************************************************
      Internal functions.
    ************************************************************************************************
    */

    /* Decide value for "bound" flag.
     */
    void set_bound(
        eCallbackEvent event);

    /**
    ************************************************************************************************
      Member variables
    ************************************************************************************************
    */

    /* Someone is bound to (looking at) this assembly.
     */
    os_boolean m_bound;
};

#endif
