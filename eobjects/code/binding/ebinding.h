/**

  @file    ebinding.h
  @brief   Binding base class for properties, DB tables and files.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    12.1.2016

  This base class serves derived classes for property, selection to table and file to handle
  bindings.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef EBINDING_H_
#define EBINDING_H_
#include "eobjects.h"


/**
****************************************************************************************************
  Defines
****************************************************************************************************
*/

/* Property numbers for ePropertyBinding and eRowSetBinding.
 */
#define EBINDP_BFLAGS 40
#define EBINDP_STATE 41

extern const os_char ebindp_bflags[];
extern const os_char ebindp_state[];

/* Binding flags (used by both ePropertyBinding and eRowSetBinding).
 */
#define EBIND_DEFAULT       0
#define EBIND_ATTR          0x0001
#define EBIND_CLIENTINIT    0x0002
#define EBIND_NOFLOWCLT     0x0004
#define EBIND_METADATA      0x0008

#define EBIND_CLIENT        0x0010  /* do not give as argument */
#define EBIND_TEMPORARY     0x0020
#define EBIND_BIND_ROWSET   0x0040  /* This is eRowSetBinding. do not give as argument */
#define EBIND_CHANGED       0x0400  /* do not give as argument */
#define EBIND_INTERTHREAD   0x0800  /* do not give as argument */

#define EBIND_TYPE_MASK     EBIND_BIND_ROWSET
#define EBIND_SER_MASK     (EBIND_TYPE_MASK|EBIND_CLIENTINIT|EBIND_NOFLOWCLT|EBIND_METADATA|EBIND_ATTR)

/* Binding states.
 */
#define E_BINDING_UNUSED    0
#define E_BINDING_NOW       1
#define E_BINDING_OK        2

/* Maximum number of forwards befoew waiting for acknowledge.
 */
#define EBIND_MAX_ACK_COUNT 3



/**
****************************************************************************************************

  @brief Binding base class.

  The eBinding is base class for different types of bindings, namely property, table/selection
  and file/handle bindings. It implements functionality mostly common to all three binding
  types.

****************************************************************************************************
*/
class eBinding : public eObject
{
public:

    /**
    ************************************************************************************************
      Generic eObject functionality.
    ************************************************************************************************
    */
    /* Constructor.
     */
    eBinding(
        eObject *parent = OS_NULL,
        e_oid id = EOID_RITEM,
        os_int flags = EOBJ_DEFAULT);

    /* Virtual destructor.
     */
    virtual ~eBinding();

    /* Clone object.
     */
    virtual eObject *clone(
        eObject *parent,
        e_oid id = EOID_CHILD,
        os_int aflags = 0);

    /* Casting eObject pointer to eBinding pointer.
     */
    inline static eBinding *cast(
        eObject *o)
    {
        return (eBinding*)o;
    }

    /* Get class identifier.
     */
    virtual os_int classid() {return ECLASSID_BINDING;}

    /* Write binding content to stream.
     */
    virtual eStatus writer(
        eStream *stream,
        os_int flags);

    /* Read binding content from stream.
     */
    virtual eStatus reader(
        eStream *stream,
        os_int flags);

    /* Bind the server end.
     */
    virtual void srvbind(
        eObject *obj,
        eEnvelope *envelope) {}

    /* Get binding flags.
     */
    inline os_int bflags()
    {
        return m_bflags;
    }

    /* Get bind path.
     */
    inline const os_char *bindpath() {return m_bindpath; }


    /**
    ************************************************************************************************
      Binding related functionality for the class.

      These are either implementations of common binding functionality, or virtual fuctions
      to be overridden by specific binding class.
    ************************************************************************************************
    */
protected:

    /* Connect client eBinding to server eBinding.
     */
    void bind_base(
        const os_char *objpath,
        eSet *parameters,
        os_boolean adopt_parameters);

    /* Base classs function to bind the server end.
     */
    void srvbind_base(
        eEnvelope *envelopee,
        eObject *reply);

    /* Complete the client end of binding by base class.
     */
    void cbindok_base(
        eEnvelope *envelope);

    /* Mark property value, etc changed.
     */
    inline void setchanged()
    {
        m_bflags |= EBIND_CHANGED;
    }

    /* Mark property value, has not been changed after forwarding it.
     */
    inline void forwarddone()
    {
        m_bflags &= ~EBIND_CHANGED;
        m_ackcount++;
    }

    /* Check if property value should be forwarded now?
     */
    inline os_int forwardnow()
    {
        return (m_bflags & EBIND_CHANGED) &&
                m_state == E_BINDING_OK &&
                (m_ackcount < EBIND_MAX_ACK_COUNT ||
                 (m_bflags & EBIND_NOFLOWCLT) ||
                 (m_bflags & EBIND_INTERTHREAD) == 0);
    }

    /* Virtual function to forward property value trough binding.
     */
    virtual void forward(
        eVariable *x = OS_NULL,
        os_boolean delete_x = OS_FALSE) {}

    /* Send ACK
     */
    void sendack_base(
        eEnvelope *envelope);

    /* Ack received.
     */
    void ack_base(
        eEnvelope *envelope);

    /* Save object path.
     */
    void set_objpath(
        const os_char *objpath);

    /* Set bind path.
     */
    void set_bindpath(
        const os_char *bindpath);

    /* Disconnect the binding and release allocated memory.
     */
    void disconnect(
        os_boolean keep_objpath = OS_FALSE);


    /**
    ************************************************************************************************
      Member variables.

      The member variables hold information where to bind (for client binding) and current
      binding state.
    ************************************************************************************************
    */
    /** Client: Path to object to bind to as given as argument to bind().
        Server: Always OS_NULL.
     */
    os_char *m_objpath;

    /** Unique path to eBinding which we are bound to.
     */
    os_char *m_bindpath;

    /** Binding flags.
     */
    os_short m_bflags;

    /** Size of object path allocation in bytes.
     */
    os_short m_objpathsz;

    /** Size of bind path allocation in bytes.
     */
    os_short m_bindpathsz;

    /** Number of ECMD_FWRD messages sent but have not been acknowledged.
     */
    os_char m_ackcount;

    /** Binding state, one of: E_BINDING_UNUSED (0), E_BINDING_NOW (1) or E_BINDING_OK.
     */
    os_char m_state;
};

#endif
