/**

  @file    container1.cpp
  @brief   Example code about using eobjects container class.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  This example demonstrates how to use containers to hold multiple objects.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "eobjects.h"
#include "container.h"


/**
****************************************************************************************************

  @brief Application entry point.

  The emain() function is eobjects application's entry point.

  @param   argc Number of command line arguments.
  @param   argv Array of string pointers, one for each command line argument. UTF8 encoded.

  @return  None.

****************************************************************************************************
*/
void container_example1()
{
    eContainer
        c;

    eVariable
        *v;

    eObject
        *o;

    os_int
        i;

    e_oid
        oid;

    /* Create some variables in container in random order. Give object identifier also
       as variable value.
     */
    for (i = 0; i<4000; i++)
    {
        oid = (e_oid)osal_rand(0,29);
        v = new eVariable(&c, oid);
        *v = oid;
    }

    /* Delete some variables at random.
     */
    for (i = 0; i<4000; i++)
    {
        o = c.first((e_oid)osal_rand(0,19));
        delete o;
    }

    /* Display which variables are left.
     */
    for (v = c.firstv(); v; v = v->nextv())
    {
        osal_console_write(v->gets());
        osal_console_write(", ");
    }

#if EOBJECT_DBTREE_DEBUG
    c.handle()->verify_whole_tree();
#endif

    /* Finished.
     */
    osal_console_write("\nall done\n");
}

