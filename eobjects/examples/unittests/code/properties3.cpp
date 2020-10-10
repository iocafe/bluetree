/**

  @file    properties1.cpp
  @brief   Example code object properties.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    10.10.2020

  This example demonstrates setting up a new class with properties, and how to react to property
  value changes.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "eobjects.h"
#include "properties.h"
#include <stdio.h>

/* Every class needs to have unique class identifier (classid). Class identifier is is 32 bit
   integer. Class identifiers starting from ECLASSID_APP_BASE are reserved for the application.
 */
#define MY_CLASS_ID (ECLASSID_APP_BASE + 1)

/* Enumeration of p3MyClass properties. Normally these would be in header file.
 */
#define EMYCLASSP_CELCIUS 10
#define EMYCLASSP_FAHRENHEIT 12
#define EMYCLASSP_OPINION 14

static const os_char emyclassp_celcius[] = "C";
static const os_char emyclassp_fahrenheit[] = "F";
static const os_char emyclassp_opinion[] = "opinion";


/**
****************************************************************************************************
  Example class with properties.
****************************************************************************************************
*/
class p3MyClass : public eObject
{
public:
    /* Constructor.
     */
    p3MyClass(
        eObject *parent = OS_NULL,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
        : eObject(parent, id, flags)
    {
        initproperties();
    }

    /* Get class identifier.
     */
    virtual os_int classid()
    {
        return MY_CLASS_ID;
    }

    /* Add p3MyClass'es properties to class'es property set.
    */
    static void setupclass()
    {
        const os_int cls = MY_CLASS_ID;
        eVariable *p;

        os_lock();
        p = addpropertyd(cls, EMYCLASSP_CELCIUS, emyclassp_celcius, 20.0, "celcius", EPRO_PERSISTENT);
        p->setpropertys(EVARP_UNIT, "C");

        p = addpropertyd(cls, EMYCLASSP_FAHRENHEIT, emyclassp_fahrenheit, "fahrenheit", EPRO_NOONPRCH);
        p->setpropertys(EVARP_UNIT, "F");
        p->setpropertyl(EVARP_DIGS, 5);

        addpropertys(cls, EMYCLASSP_OPINION, emyclassp_opinion, "opinion", EPRO_NOONPRCH);
        os_unlock();
    }

    /* This gets called when property value changes
     */
    virtual eStatus onpropertychange(
        os_int propertynr,
        eVariable *x,
        os_int flags)
    {
        os_double c, f;

        switch (propertynr)
        {
            case EMYCLASSP_CELCIUS:
                c = x->getd();
                printf ("calculating C -> F\n");

                f = c * 9.0 / 5.0 + 32.0;
                setpropertyd(EMYCLASSP_FAHRENHEIT, f);
                if (f < 70) setpropertys(EMYCLASSP_OPINION, "cold");
                else if (f < 80) setpropertys(EMYCLASSP_OPINION, "ok");
                else setpropertys(EMYCLASSP_OPINION, "hot");
                break;

            default:
                return ESTATUS_FAILED;
        }

        return ESTATUS_SUCCESS;
    }
};


/**
****************************************************************************************************

  @brief Property example 3.

  The property_example_3() function sets up new class p3MyClass and uses for Celcius
  to Fahrenheit conversion. Purpose of this is to show how class can react to property changes.

  @return  None.

****************************************************************************************************
*/
void property_example_3()
{
    p3MyClass *converter;
    eVariable v, u;
    os_double f;

    /* Adds the p3MyClass to class list and creates property set for the class.
     */
    p3MyClass::setupclass();

    converter = new p3MyClass();

    f = converter->propertyd(EMYCLASSP_FAHRENHEIT);
    converter->propertyv(EMYCLASSP_OPINION, &v);
    printf ("initial F = %f, opinion = %s\n", f, v.gets());

    converter->setpropertyd(EMYCLASSP_CELCIUS, 40.0);
    f = converter->propertyd(EMYCLASSP_FAHRENHEIT);
    converter->propertyv(EMYCLASSP_OPINION, &v);
    printf ("initial F = %f, opinion = %s\n", f, v.gets());

    delete converter;
}
