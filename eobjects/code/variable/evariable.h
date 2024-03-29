/**

  @file    evariable.h
  @brief   Dynamically typed variables.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

  Dynamically typed variables...

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef EVARIABLE_H_
#define EVARIABLE_H_
#include "eobjects.h"

class eValueX;

/**
****************************************************************************************************
  Defines
****************************************************************************************************
*/

/** Internal string buffer size. Maximum size of string which can be stored within eVariable
    without separate memory allocation.
 */
#define EVARIABLE_STRBUF_SZ ((os_memsz)(sizeof(os_memsz)*2 + sizeof(os_char*) - sizeof(os_uchar)))

/* Variable property numbers. Avoid changing these numbers, used by derived and other classess.
 */
#define EVARP_VALUE 1
#define EVARP_TYPE 2
#define EVARP_TEXT 3
#define EVARP_UNIT 4
#define EVARP_ATTR 5
#define EVARP_DEFAULT 6
#define EVARP_GROUP 8
#define EVARP_ABBR 10
#define EVARP_TTIP 11
#define EVARP_DIGS 12
#define EVARP_MIN 15
#define EVARP_MAX 16
#define EVARP_GAIN 17
#define EVARP_OFFSET 18
#define EVARP_SBITS 20
#define EVARP_TSTAMP 21
#define EVARP_CONF 26

/* Variable property names.
 */
extern const os_char
    evarp_value[],
    evarp_digs[],
    evarp_text[],
    evarp_abbr[],
    evarp_ttip[],
    evarp_unit[],
    evarp_min[],
    evarp_max[],
    evarp_type[],
    evarp_attr[],
    evarp_group[],
    evarp_default[],
    evarp_gain[],
    evarp_offset[],
    evarp_sbits[],
    evarp_tstamp[],
    evarp_conf[];


/* Value to indicate that decimal digits is not set.
 */
#define EVARP_DIGS_UNDEFINED 15

/* Default number of decimal digits for floating point number.
 */
#define EVARP_DEFAULT_DIGS 2


/**
****************************************************************************************************
  Internal flags.

  The m_vflags contains variable type, plus additional information:
  - Lowest 8 bits are reserved for variable type, EVAR_TYPE_MASK is used to mask type out.
  - Next 8 bits are reserved for number of digits after decimal point. Used with variable
    type OS_DOUBLE to convert variable value to string.
****************************************************************************************************
*/
#define EVAR_TYPE_MASK   0x001F
#define EVAR_DDIGS_MASK  0x03E0
#define EVAR_DDIGS_SHIFT 5
#define EVAR_NOSAVE      0x0400
#define EVAR_STRBUF_ALLOCATED 0x2000

/* Serialize type and number of decimal digits in flags.
 */
#define EVAR_SERIALIZATION_MASK 0x03FF

/** The EVAR_IS_RED is used by eName class to position names in red/black index.
 */
#define EVAR_IS_RED 0x4000


/**
****************************************************************************************************
  The eVariable is dynamically typed variable, which can store numbers, string and objects.
****************************************************************************************************
*/
class eVariable : public eObject
{
public:

    /**
    ************************************************************************************************
      Generic eObject functionality.
    ************************************************************************************************
    */
    /* Constructor.
     */
    eVariable(
        eObject *parent = OS_NULL,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_EROOT_OPTIONAL);

    /* Virtual destructor.
     */
    virtual ~eVariable();

    /* Clone the eVariable.
     */
    virtual eObject *clone(
        eObject *parent,
        e_oid id = EOID_CHILD,
        os_int aflags = 0);

    /* Casting eObject pointer to eVariable pointer.
     */
    inline static eVariable *cast(
        eObject *o)
    {
        e_assert_type(o, ECLASSID_VARIABLE)
        return (eVariable*)o;
    }

    /* Get class identifier.
     */
    virtual os_int classid() {return ECLASSID_VARIABLE; }

    /* Static function to add class to propertysets and class list.
     */
    static void setupclass();

    /* Add class'es properties to property set.
     */
    static void setupproperties(
        os_int cls);

    /* Static constructor function for generating instance by class list.
     */
    static eVariable *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new eVariable(parent, id, flags);
    }

    /* Get next variable identified by oid.
     */
    eVariable *nextv(
        e_oid id = EOID_CHILD);

    /* Get next static property identified by oid.
     */
    inline eVariable *nextp(
        e_oid id = EOID_CHILD) {return nextv(id); }

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

    /* Write variable to stream.
     */
    virtual eStatus writer(
        eStream *stream,
        os_int flags);

    /* Read variable from stream.
     */
    virtual eStatus reader(
        eStream *stream,
        os_int flags);

    /**
    ************************************************************************************************
      Get variable type info.
    ************************************************************************************************
    */
    /** Get value data type.
     */
    inline osalTypeId type() const
    {
        return (osalTypeId)(m_vflags & EVAR_TYPE_MASK);
    }

    /** Get number of digits after decimal point.
     */
    inline os_int digs() const
    {
        return (m_vflags & EVAR_DDIGS_MASK) >> EVAR_DDIGS_SHIFT;
    }

    /** Set number of digits after decimal point.
     */
    void setdigs(os_int ddigs);


    /**
    ************************************************************************************************
      Set variable value

      The set*() functions set variable value. The variable type is set when value is set,
      for example setl() sets variable type to be an integer, or sets() to be a string.
      The clear() function makes variable empty.
    ************************************************************************************************
    */
    /* Empty the variable value.
     */
    void clear();

    /* Set long integer value to variable.
     */
    void setl(
        const os_long x);

    /* Set double value to variable.
     */
    void setd(
        const os_double x);

    /* Set string value to variable.
     */
    void sets(
        const os_char *x,
        os_memsz max_chars = -1);

    /* Copy or move variable value from another variable.
     */
    void setv(
        eVariable *x,
        os_boolean move_value = OS_FALSE);

    /* Set object as variable value.
     */
    void seto(
        eObject *x,
        os_boolean adopt_x = OS_FALSE);

    /* Set pointer value to variable.
     */
    void setp(
        os_pointer x);


    /**
    ************************************************************************************************
      Get variable value

      The get*() functions return variable value as specific data type, type conversion is
      done as needed. The isempty() function checks if variable is empty.
      The variable type or value is not modified by these functions.
    ************************************************************************************************
    */
    /* Check if variable is empty.
     */
    os_boolean isempty();

    /** Get variable value as shorter integer types.
     */
    inline os_boolean getb() { return (os_boolean)getl(); }
    inline os_short geth() { return (os_short)getl(); }
    inline os_ushort getuh() { return (os_ushort)getl(); }
    inline os_int geti() { return (os_int)getl(); }

    /* Get variable value as long integer.
     */
    os_long getl();

    /* Get variable value as double.
     */
    os_double getd();

    /* Get variable value as string.
     */
    os_char *gets(
        os_memsz *sz = OS_NULL);

    /* Release memory allocated for temporary buffer by gets() function.
     */
    void gets_free();

    /* Get pointer to object contained by variable.
     */
    eObject *geto();

    /* Get pointer to extended value container by variable.
     */
    eValueX *getx();

    /* Get pointer stored in variable.
     */
    os_pointer getp();

    /* Allocate buffer to hold a string.
     */
    os_char *allocate(
        os_memsz nchars);


    /**
    ************************************************************************************************
      Appending to variable value
    ************************************************************************************************
    */
    /* Append string to variable.
     */
    void appends(
        const os_char *x);

    /* Append known length string to variable value.
     */
    void appends_nbytes(
        const os_char *str,
        os_memsz nchars);

    void appendl(
        os_long x);

    /* Append variable value as string to this variable.
     */
    void appendv(
        eVariable *x);


    /**
    ************************************************************************************************
      Extended value
    ************************************************************************************************
    */
    /* Get state bits. If the value is not extended, the function returns OSAL_STATE_CONNECTED.
     */
    inline os_int sbits() {if (type() != OS_OBJECT) return OSAL_STATE_CONNECTED; return sbits_internal();}

    /* Get time stamp. If the value is not extended, the function returns 0.
     */
    inline os_long tstamp() {if (type() != OS_OBJECT) return 0; return tstamp_internal();}


    /**
    ************************************************************************************************
      Micellenous
    ************************************************************************************************
    */
    /* Compare two variables.
     */
    virtual os_int compare(
        eObject *xx,
        os_int flags = 0);

    /* Remove white space from beginning and end of string values.
     */
    void removeblancs();

    /* Automatically type string containing integer or floating point value.
     */
    os_boolean autotype(
        os_boolean modify_value);

    /* Convert variable value to string and get string pointer.
     */
    os_char *tostring();

    /* Check if object name is oix.
     */
    os_boolean is_oix();

    /* If this variable is not to be saved.
     */
    inline os_boolean is_nosave()
        {return (m_vflags & EVAR_NOSAVE) ? OS_TRUE : OS_FALSE; }

    /* Remove new lines from string.
     */
    void singleline();

    /* Make string lower case.
     */
    void tolower();

    /* Check if variable value as string starts with text given as argument.
     */
    os_boolean starts_with(
        os_char *text);

    /* Remove stuff from path what becomes unnecessary when oix is appended to it.
     */
    os_boolean clean_to_append_oix();

    /** Check if temporary string buffer has been allocated.
     */
    inline os_boolean tmpstrallocated()
    {
        return (type() != OS_STR && m_value.valbuf.tmpstr)
            ? OS_TRUE : OS_FALSE;
    }


    /**
    ************************************************************************************************
      Operator overloads

      The operator overloads are implemented for convinience, and map to the member functions.
      Using operator overloads may lead to more readable code, but may also confuse the
      reader.
    ************************************************************************************************
    */

    /** Operator "=", setting variable value.
     */
    inline const os_char operator=(const os_char x) { setl(x); return x; }
    inline const os_uchar operator=(const os_uchar x) { setl(x); return x; }
    inline const os_short operator=(const os_short x) { setl(x); return x; }
    inline const os_ushort operator=(const os_ushort x) { setl(x); return x; }
    inline const os_int operator=(const os_int x) { setl(x); return x; }
    inline const os_uint operator=(const os_uint x) { setl(x); return x; }
    inline const os_long operator=(const os_long x) { setl(x); return x; }
    inline const os_float operator=(const os_float x) { setd(x); return x; }
    inline const os_double operator=(const os_double x) { setd(x); return x; }
    inline const os_char *operator=(const os_char *x) { sets(x); return x; }
    inline void operator=(eVariable& x) { setv(&x); }

    /** Operator "+=", appending variable value.
     */
    inline const os_char *operator+=(const os_char *x) { appends(x); return x; }
    inline void operator+=(eVariable& x) { appendv(&x); }
    inline void operator+=(os_long x) { appendl(x); } // MORE CONFUSING THAN USEFUL?


protected:
    /**
    ************************************************************************************************
      Internal functions
    ************************************************************************************************
    */
    /* Collect information about this object for tree browser.
     */
    virtual void object_info(
        eVariable *item,
        eVariable *name,
        eSet *appendix,
        const os_char *target);

    /** Set data type.
     */
    inline void settype(osalTypeId type_id)
    {
        m_vflags &= ~EVAR_TYPE_MASK;
        m_vflags |= (type_id & EVAR_TYPE_MASK);
    }

    inline void cleartmpstr()
    {
        if (tmpstrallocated()) gets_free();
    }

    os_int sbits_internal();
    os_long tstamp_internal();

    /**
    ************************************************************************************************
      Member variables
    ************************************************************************************************
    */
    /** Internal flags. Contains variable data type, number of digits after decimal
        point and string allocation information.
     */
    os_short m_vflags;

    /** Union which hold the variable value.
     */
    union
    {
        /** Long string values are stored in separate buffer allocated os_malloc()
            function. allocated is number of allocated bytes, used is number of used bytes
            including terminating null character and ptr is pointer to allocated buffer.
         */
        struct
        {
            os_memsz allocated;
            os_memsz used;
            os_char *ptr;
        }
        strptr;

        /** Short strings are stored in "buf" within this structure. "used" is number of
            used bytes including terminating null character.
         */
        struct
        {
            os_char buf[EVARIABLE_STRBUF_SZ];
            os_uchar used;
        }
        strbuf;

        /** Integer, 64 bit integer, double or object pointer is stored in valbuf
            structure. The tmpstr is pointer to temporary string generated by gets.
         */
        struct
        {
            /** Value, union which contains primitive values as different types.
             */
            union
            {
                os_long l;
                os_double d;
                eObject *o;
                os_pointer p;
            }
            v;

            /** Pointerto temporary string buffer generated by gets().
             */
            os_char *tmpstr;

            /** Temporary buffer size in bytes.
             */
            os_memsz tmpstr_sz;
        }
        valbuf;
    }
    m_value;
};

#endif
