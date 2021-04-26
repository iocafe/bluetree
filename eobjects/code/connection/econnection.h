/**

  @file    econnection.h
  @brief   Connection base class.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

  eConnection class, related to message envelope transport betweeen processes. See econnection.cpp
  for more information.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef ECONNECTION_H_
#define ECONNECTION_H_
#include "eobjects.h"

struct iocSwitchboxAuthenticationFrameBuffer;

/**
****************************************************************************************************
  Defines
****************************************************************************************************
*/

/* Enumeration of connection's properties.
 */
#define ECONNP_CLASSID 5
#define ECONNP_IPADDR 10
#define ECONNP_ISOPEN 15
#define ECONNP_ENABLE 20

/* Connection property names.
 */
extern const os_char
    econnp_classid[],
    econnp_ipaddr[],
    econnp_isopen[],
    econnp_enable[];


/**
****************************************************************************************************

  @brief Connection class.

  The eConnection passes message to other process trough TCP/IP socket or serial port.

****************************************************************************************************
*/
class eConnection : public eThread
{
public:
    /* Constructor.
     */
    eConnection(
        eObject *parent = OS_NULL,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT);

    /* Virtual destructor.
     */
    virtual ~eConnection();

    /* Casting eObject pointer to eConnection pointer.
     */
    inline static eConnection *cast(
        eObject *o)
    {
        e_assert_type(o, ECLASSID_CONNECTION)
        return (eConnection*)o;
    }

    /* Get class identifier.
    */
    virtual os_int classid() {return ECLASSID_CONNECTION; }

    /* Static function to add class to propertysets and class list.
     */
    static void setupclass();

    /* Static constructor function.
    */
    static eConnection *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new eConnection(parent, id, flags);
    }

    /* Function to process messages to this object.
     */
    virtual void onmessage(
        eEnvelope *envelope);

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

    /* List names in this object's namespace. Here we list files and folders.
     */
    virtual void browse_list_namespace(
        eContainer *content,
        const os_char *target);

    /* Initialize the object.
     */
    virtual void initialize(
        eContainer *params = OS_NULL);

    /* Run the connection.
     */
    virtual void run();

    virtual eStatus accepted(
        eStream *stream);

    /* Send authentication frame to the socket.
     */
    void send_authentication_frame();


protected:

    /**
    ************************************************************************************************
      Protected member functions.
    ************************************************************************************************
    */

    /* New connection: network selection, certificate copy, transfer authentication frames.
     */
    eStatus handshake_and_authentication();

    /* Open the connection (connect)
     */
    void open();

    /* Stop and close the connection.
     */
    void close();

    /* Connection established event detected, act on it.
     */
    eStatus connected();

    /* Closing connection (connection failed, disconnected event, etc), act on it.
     */
    void disconnected();

    /* Monitor messages for bind and unbind.
     */
    void monitor_binds(
        eEnvelope *envelope);

    /* Monitor messages for bind and unbind.
     */
    os_boolean has_client_bindings()
        {return m_client_bindings->childcount() > 0; }

    /* Write an envelope to the connection.
     */
    eStatus write(
        eEnvelope *envelope);

    /* Read an envelope from connection.
     */
    eStatus read();


    /**
    ************************************************************************************************
      Member variables.
    ************************************************************************************************
    */

    /** Stream class identifier. Specifies stream class to use.
     */
    os_int m_stream_classid;

    /* Timer for last send, used to generate keepalives.
     */
    os_timer m_last_send;

    /** IP address and optionally port number to connect to.
     */
    eVariable *m_ipaddr;

    /** stream handle. OS_NULL if socket is not open.
     */
    eStream *m_stream;

    /** Empty envelope beging read or reserved for next read.
     */
    eEnvelope *m_envelope;

    /** Memorized client bindings.
     */
    eContainer *m_client_bindings;

    /** Memorized server bindings.
     */
    eContainer *m_server_bindings;

    /** Buffer for messages when attempting connection for the first time.
     */
    eContainer *m_initbuffer;

    /** Connection initailized flag.
     */
    os_boolean m_initialized;

    /** Connected at this moment flag.
     */
    os_boolean m_connected;

    /** Connection has been attempted and it has failed at least once.
     */
    os_boolean m_connection_failed_once;

    /** Connection is enabled, property setting.
     */
    os_boolean m_enable;

    /** Reconnect timer enabled. -1 = not set, 0 = slow timer, 1 = fast timer.
     */
    os_char m_fast_timer_enabled;

    /** New data has been written to stream, but the stream has not been
        flushed yet.
     */
    os_boolean m_new_writes;

    /** Server end of the connection, delete the connection if case socket fails.
     */
    os_boolean m_is_server;

    /** Current hand shake state.
     */
    iocHandshakeState m_handshake;

    /** Flag indicating that switchbox network selection and certificate copy check handshake
     *  has been completed.
     */
    os_boolean m_handshake_ready;

    /** Flag indicating that the authentication frame has been sent after the connection was opened.
     */
    os_boolean m_authentication_frame_sent;

    /** Flag indicating that the authentication frame has received sent after the connection was opened.
     */
    os_boolean m_authentication_frame_received;

    /** Buffer for receiving authentication frame. OS_NULL if the buffer is not allocated.
     */
    struct iocSwitchboxAuthenticationFrameBuffer *m_auth_send_buf;

    /** Buffer for sending authentication frame. OS_NULL if the buffer is not allocated.
     */
    struct iocSwitchboxAuthenticationFrameBuffer *m_auth_recv_buf;
};

#endif
