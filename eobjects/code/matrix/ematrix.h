/**

  @file    ematrix.h
  @brief   Matrix class.
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
#ifndef EMATRIX_H_
#define EMATRIX_H_
#include "eobjects.h"

class eBuffer;


/**
****************************************************************************************************
  Defines
****************************************************************************************************
*/
/*@{*/

/* Matrix property numbers.
 */
#define EMTXP_DATATYPE 20
#define EMTXP_NROWS 21
#define EMTXP_NCOLUMNS 22
#define EMTXP_CONFIGURATION 30

/* Matrix property names.
 */
extern const os_char
    emtxp_datatype[],
    emtxp_nrows[],
    emtxp_ncolumns[],
    emtxp_configuration[];


/**
****************************************************************************************************

  @brief Matrix class.

  The eMatrix is base class for tables. Underlying table can be eMatrix, eSelection or database
  table class.


****************************************************************************************************
*/
class eMatrix : public eTable
{
    /**
    ************************************************************************************************

      @name Generic object functionality.

      These functions enable using objects of this class as generic eObjects.

    ************************************************************************************************
    */
    /*@{*/
public:
    /* Constructor.
     */
    eMatrix(
        eObject *parent = OS_NULL,
        e_oid id = EOID_RITEM,
        os_int flags = EOBJ_DEFAULT);

    /* Virtual destructor.
     */
    virtual ~eMatrix();

    /* Clone object.
     */
    virtual eObject *clone(
        eObject *parent,
        e_oid id = EOID_CHILD,
        os_int aflags = 0);

    /* Casting eObject pointer to eMatrix pointer.
     */
    inline static eMatrix *cast(
        eObject *o)
    {
        e_assert_type(o, ECLASSID_MATRIX)
        return (eMatrix*)o;
    }

    /* Get class identifier.
     */
    virtual os_int classid() {return ECLASSID_MATRIX; }

    /* Static function to add class to propertysets and class list.
     */
    static void setupclass();

    /* Static constructor function for generating instance by class list.
     */
    static eMatrix *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new eMatrix(parent, id, flags);
    }

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

    /* Write matrix content to stream.
     */
    virtual eStatus writer(
        eStream *stream,
        os_int flags);

    /* Read matrix content from stream.
     */
    virtual eStatus reader(
        eStream *stream,
        os_int flags);

#if E_SUPPROT_JSON
    /* Write matrix specific content to stream as JSON.
     */
    virtual eStatus json_writer(
        eStream *stream,
        os_int sflags,
        os_int indent);
#endif

    /*@}*/

    /**
    ************************************************************************************************

      @name Table function overrides.

      X...

    ************************************************************************************************
    */
    /*@{*/

    /* Configure the table.
     */
    virtual void configure(
        eContainer *configuration,
        os_int tflags = 0);

    /* Insert rows into table.
     * Row can be one row or container with multiple rows.
     */
    virtual void insert(
        eContainer *rows,
        os_int tflags = 0)
    {}

    /* Update a row or rows of a table.
     */
    virtual void update(
        eVariable *where,
        eContainer *row,
        os_int tflags = 0)
    {}

    /* Remove rows from table.
     */
    virtual void remove(
        eVariable *where,
        os_int tflags = 0)
    {}

    /* Select rows from table.
     */
    virtual void select(
        eVariable *where,
        os_int tflags = 0)
    {}
    /*@}*/


    /**
    ************************************************************************************************

      @name Matrix functions.

      These function treat matrix as organized to rows and columns.

    ************************************************************************************************
    */
    /*@{*/

    /* Allocate matrix.
     */
    void allocate(
        osalTypeId type,
        os_int nrows = 0,
        os_int ncolumns = 0);

    /* Release all allocated data, empty the matrix.
     */
    void clear();

    /* Get matrix width.
     */
    os_int width();

    /* Get matrix height.
     */
    os_int height();

    /* Store value into matrix.
     */
    void setv(
        os_int row,
        os_int column,
        eVariable *x,
        os_int mflags = 0);

    /* Store integer value into matrix.
     */
    void setl(
        os_int row,
        os_int column,
        os_long x);

    /* Store double value into matrix.
     */
    void setd(
        os_int row,
        os_int column,
        os_double x);

    /* Store string value into matrix.
     */
    void sets(
        os_int row,
        os_int column,
        const os_char *x);

    /* Store value into matrix.
     */
    void seto(
        os_int row,
        os_int column,
        eObject *x,
        os_int mflags = 0);

    /* Clear matrix value to be empty.
     */
    void clear(
        os_int row,
        os_int column);

    /* Get value from matrix.
     */
    os_boolean getv(
        os_int row,
        os_int column,
        eVariable *x);

    /* Get value from matrix as integer.
     */
    os_long getl(
        os_int row,
        os_int column,
        os_boolean *hasvalue = OS_NULL);

    /* Get value from matrix as double.
     */
    os_double getd(
        os_int row,
        os_int column,
        os_boolean *hasvalue = OS_NULL);

    /*@}*/


protected:
    /**
    ************************************************************************************************

      @name Internal to matrix.

      Protected functions and member variables.

    ************************************************************************************************
    */
    /*@{*/

    /* Make sure that neither row or column number is negative.
     */
    inline eStatus checknegative(
        os_int row,
        os_int column)
    {
        if (row < 0 || column < 0)
        {
            osal_debug_error("ematrix.cpp: row or column number is negative.");
            return ESTATUS_FAILED;
        }
        return ESTATUS_SUCCESS;
    }

    /* Write consequent non-empty matrix elements to stream.
     */
    eStatus elementwrite(
        eStream *stream,
        os_int first_full_ix,
        os_int full_count,
        os_int sflags);

    /* Resize the matrix.
     */
    void resize(
        osalTypeId datatype,
        os_int nrows,
        os_int ncolumns);

    /* Get pointer to data for element and if m_type is OS_OBJECT also type for element.
     */
    os_char *getptrs(
        os_int row,
        os_int column,
        os_char **typeptr,
        os_int flags,
        eBuffer **pbuffer = OS_NULL);

    /* Get or allocate eBuffer by buffer number (oid).
     */
    eBuffer *getbuffer(
        os_int buffer_nr,
        os_int flags);

    /* Free memory buffer and any objects and strings allocated for it.
     */
    void releasebuffer(
        eBuffer *buffer);

    void emptyobject(
        os_char *dataptr,
        os_char *typeptr);

    /* How many bytes are needed for matrix datatype
     */
    os_short typesz(
        osalTypeId datatype);

    /* How many element we can fit in memory block?
     */
    os_int elems_per_block();

    /** Matrix data type.
     */
    osalTypeId m_datatype;

    /** Matrix data size in bytes.
     */
    os_short m_typesz;

    /** Matrix element size in bytes. This is typesz + 1 for object data type;
     */
    os_short m_elemsz;

    /** Number of rows.
     */
    os_int m_nrows;

    /** Number of columns.
     */
    os_int m_ncolumns;

    /** Matrix configuration, OS_NULL if not set.
     */
    eContainer *m_configuration;

    /** To prevent recursive resizing.
     */
    os_int m_own_change;

    /*@}*/
};

#endif
