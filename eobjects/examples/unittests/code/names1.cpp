/**

  @file    names1.cpp
  @brief   Example code about naming objects.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  This example demonstrates how to name objects.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "eobjects.h"
#include "names.h"

void names_example1()
{
    eContainer
        c;

    eVariable
        *v;

    eName
        *n;

    /* Enable name space for container.
     */
    c.ns_create();

    /* Create variable as child of container and give it a value.
     */
    v = new eVariable(&c);
    v->sets("myvalue");
    v->addname("myname");

    /* Create variable as child of container and give it a value.
     */
    v = new eVariable(&c);
    v->sets("yourvalue");
    v->addname("yourname");

    /* Create variable as child of container and give it a value.
     */
    v = new eVariable(&c);
    v->sets("hervalue");
    v->addname("hername");

    c.handle()->verify_whole_tree();

    /* List all names in conteiner's namespace.
     */
    for (n = c.ns_first(); n; n = n->ns_next(OS_FALSE))
    {
        osal_console_write(n->gets());
        osal_console_write(" ");
        v = eVariable::cast(n->parent());
        osal_console_write(v->gets());
        osal_console_write("\n");
    }

    /* Print your value.
     */
    v = c.ns_getv("yourname");
    if (v)
    {
        osal_console_write(v->gets());
        osal_console_write("\n");
    }
}
