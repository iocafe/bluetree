/**

  @file    econnection.cpp
  @brief   Connection base class.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    17.5.2020

  The eConnection class is part of message envelope transport from process to another, either
  within computer or in network. For a process to listen for connections from an another process
  it created eEndPoint object. When the second process connects to it it creates eConnection,
  which actively connects to the eEndPoint IP/port of the first process. When eEndPoint accepts
  the connection, it creates a connection object for the accepted socket, etc. Both of the two
  processes connected together, have their own eConnection object.
  eConnection object is visible in process'es object tree. When eConnection receives a message,
  it passes it trough the socket, etc, and the eConnection in the second forwards it as if
  the envelope came from the eConnection itself. The eConnectio wraps a stream, either eSocket
  or eSerial, and uses it to pass data over socket or serial port.
  The eConnection is class derived from eThread. It always runs at it's own thread.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "eobjects.h"
#include "iocom.h"

/* Connection property names.
 */
const os_char
    econnp_classid[] = "classid",
    econnp_ipaddr[] = "ipaddr",
    econnp_isopen[] = "isopen",
    econnp_enable[] = "enable";


/**
****************************************************************************************************

  @brief eConnection constructor.

  Creates empty unconnected connection. Clears member variables and allocates eVariable for IP
  address, and eContainer for first initialization bufffer and eContainers for memorised client
  and server bindings.

****************************************************************************************************
*/
eConnection::eConnection(
    eObject *parent,
    e_oid id,
    os_int flags)
    : eThread(parent, id, flags)
{
    m_stream_classid = ECLASSID_OSSTREAM;
    m_ipaddr = new eVariable(this);
    m_stream = OS_NULL;
    m_initbuffer = new eContainer(this);
    m_initialized = OS_FALSE;
    m_connected = OS_FALSE;
    m_connection_failed_once = OS_FALSE;
    m_new_writes = OS_FALSE;
    m_fast_timer_enabled = -1;
    m_is_server = OS_FALSE;
    m_envelope = OS_NULL;
    m_enable = OS_TRUE;
    m_authentication_frame_sent = OS_FALSE;
    m_authentication_frame_received = OS_FALSE;
    m_auth_send_buf = OS_NULL;
    m_auth_recv_buf = OS_NULL;
    m_client_bindings = new eContainer(this);
    m_client_bindings->ns_create();
    m_server_bindings = new eContainer(this);
    m_server_bindings->ns_create();
    os_get_timer(&m_last_send);
}


/**
****************************************************************************************************

  @brief Virtual destructor.

  Closes the connection, if it is open.

  @return  None.

****************************************************************************************************
*/
eConnection::~eConnection()
{
    close();

    if (m_auth_send_buf) {
        os_free(m_auth_send_buf, sizeof(iocSwitchboxAuthenticationFrameBuffer));
        m_auth_send_buf = OS_NULL;
    }
    if (m_auth_recv_buf) {
        os_free(m_auth_recv_buf, sizeof(iocSwitchboxAuthenticationFrameBuffer));
        m_auth_recv_buf = OS_NULL;
    }
}


/**
****************************************************************************************************

  @brief Add eConnection to class list and class'es properties to it's property set.

  The eConnection::setupclass function adds eConnection to class list and class'es properties to
  it's property set. The class list enables creating new objects dynamically by class identifier,
  which is used for serialization reader functions. The property set stores static list of
  class'es properties and metadata for those.

****************************************************************************************************
*/
void eConnection::setupclass()
{
    const os_int cls = ECLASSID_CONNECTION;
    eVariable *p;

    /* Synchronize, add the class to class list and properties to property set.
     */
    os_lock();
    eclasslist_add(cls, (eNewObjFunc)newobj, "eConnection", ECLASSID_THREAD);
    addproperty(cls, ECONNP_CLASSID, econnp_classid, "class ID", EPRO_PERSISTENT|EPRO_SIMPLE);
    addproperty(cls, ECONNP_IPADDR, econnp_ipaddr, "IP", EPRO_PERSISTENT|EPRO_SIMPLE);
    p = addpropertyb(cls, ECONNP_ISOPEN, econnp_isopen, OS_FALSE, "is open", EPRO_SIMPLE);
    p->setpropertys(EVARP_ATTR, "rdonly");
    addpropertyb(cls, ECONNP_ENABLE, econnp_enable, OS_TRUE, "enable", EPRO_DEFAULT);
    propertysetdone(cls);
    os_unlock();
}


/**
****************************************************************************************************

  @brief Called to inform the class about property value change (override).

  The onpropertychange() function is called when class'es property changes, unless the
  property is flagged with EPRO_NOONPRCH.
  If property is flagged as EPRO_SIMPLE, this function shuold save the property value
  in class members and and return it when simpleproperty() is called.

  @param   propertynr Property number of the changed property.
  @param   x eVariable containing the new value.
  @param   flags
  @return  If successfull, the function returns ESTATUS_SUCCESS (0). Nonzero return values do
           indicate that there was no property with given property number.

****************************************************************************************************
*/
eStatus eConnection::onpropertychange(
    os_int propertynr,
    eVariable *x,
    os_int flags)
{
    switch (propertynr)
    {
        case ECONNP_ISOPEN:
            m_connected = x->getb();
            break;

        case ECONNP_CLASSID:
            m_stream_classid = x->geti();
            close();
            break;

        case ECONNP_IPADDR:
            if (x->compare(m_ipaddr))
            {
                m_ipaddr->setv(x);
                close();
                open();
            }
            break;

        case ECONNP_ENABLE:
            m_enable = (os_boolean)x->getl();
            if (!m_enable) {
                close();
            }
            break;

        default:
            return eThread::onpropertychange(propertynr, x, flags);
    }

    return ESTATUS_SUCCESS;
}


/**
****************************************************************************************************

  @brief Get value of simple property (override).

  The simpleproperty() function stores the current value of a simple property into variable x.

  @param   propertynr Property number.
  @param   x eVariable into which to store the property value.
  @return  If property with property number was stored in x, the function returns
           ESTATUS_SUCCESS (0). Nonzero return values indicate that property with
           given number was not among simple properties.

****************************************************************************************************
*/
eStatus eConnection::simpleproperty(
    os_int propertynr,
    eVariable *x)
{
    switch (propertynr)
    {
        case ECONNP_ISOPEN:
            x->setl(m_connected);
            break;

        case EENDPP_CLASSID:
            x->setl(m_stream_classid);
            break;

        case ECONNP_IPADDR:
            x->setv(m_ipaddr);
            break;

        default:
            return eThread::simpleproperty(propertynr, x);
    }
    return ESTATUS_SUCCESS;
}


/**
****************************************************************************************************

  @brief Add "_r" to object's name space for browsing.

  The eConnection::browse_list_namespace function lists named children, grandchildren, etc,
  when name is mapped to name space of this object. Each list item is a variable.

  @param   content Pointer to container into which to place list items.
  @param   target When browsing structure which is not made out of eObjects,
           this can be path within the object (like file system, etc).
  @param   None.

****************************************************************************************************
*/
void eConnection::browse_list_namespace(
    eContainer *content,
    const os_char *target)
{
    eVariable *item;
    eSet *appendix;

    item = new eVariable(content, EBROWSE_NSPACE);
    appendix = new eSet(item, EOID_APPENDIX, EOBJ_IS_ATTACHMENT);
    appendix->sets(EBROWSE_PATH, "_r");
    appendix->sets(EBROWSE_IPATH, "_r");
    item->setpropertys(EVARP_TEXT, "route");

    eThread::browse_list_namespace(content, target);
}


/**
****************************************************************************************************

  @brief Process messages.

  The onmessage function handles message envelopes received by the eConnection. If message
  envelope is not message to eConnection (has path, etc), it to be forwarded tough the
  socket, ect, to another process.

  If connection (socket, etc) has been closed, this function tries periodically to reopen it.
  First connection attempt is already done when IP address has been set and connection is
  being initialized.

  @param   envelope Message envelope. Contains command, target and source paths and
           message content, etc.
  @return  None.

****************************************************************************************************
*/
void eConnection::onmessage(
    eEnvelope *envelope)
{
    os_char *p, c;

    /* If target string start with "_r/" or is "_r", route. This is for browsing.
     */
    p = envelope->target();
    c = *p;
    if (c == '_' && p[1] == 'r') if (p[2] == '\0' || p[2] == '/')
    {
        /* Skip "_r" oe "_r/"
         */
        envelope->move_target_pos(p[2] == '\0' ? 2 : 3);
        goto routeit;
    }

    /* If this is envelope to be routed trough connection.
     */
    if (c != '_' && c != '\0')
    {
routeit:
        /* If currently connected, write envelope immediately.
         */
        if (m_connected)
        {
            /* Check for binding related messages, memorize bindings through
               this connection.
             */
            monitor_binds(envelope);

            /* Write the envelope to socket. Close socket if writing fails.
             */
            if (write(envelope)) close();
        }

        /* Not connected.
         */
        else
        {
            /* If connection has not failed yet, buffer message envelopes
               to be sent when connection is established for the first time.
             */
            if (!m_connection_failed_once)
            {
                if (envelope->flags() & EMSG_CAN_BE_ADOPTED)
                {
                    envelope->adopt(m_initbuffer);
                }
                else
                {
                    envelope->clone(m_initbuffer);
                }
            }

            /* Otherwise connection has been tied and failed already,
               reply with notarget message.
             */
            else
            {
                /* Check for binding related messages, memorize bindings
                   through this connection.
                 */
                monitor_binds(envelope);

                /* Send notaget message to indicate the messagecannot
                   be sent now.
                 */
                notarget(envelope);
            }
        }

        return;
    }

    /* If this is periodic timer message to this object.
     */
    if (c == '\0') if (envelope->command() == ECMD_TIMER)
    {
        /* If stream is open, send keepalive.
         */
        if (m_connected)
        {
            if (os_has_elapsed(&m_last_send, 20000))
            {
                if (m_stream->writechar(E_STREAM_KEEPALIVE))
                {
                    close();
                    return;
                }
                if (m_stream->flush())
                {
                    close();
                    return;
                }
                os_get_timer(&m_last_send);
            }
        }

        /* Otherwise try to reopen the socket if it is closed.
         */
        else
        {
            open();
        }

        return;
    }

    /* Call base class'es message handling.
     */
    eThread::onmessage(envelope);
}


/**
****************************************************************************************************

  @brief Initialize the object.

  The initialize() function is called when new object is fully constructed.

  The function eConnection initialized by setting m_initialized flag, and if IP address is known
  tries to open connection (socket, etc) to listening end point of another process.

  @param   params Parameters for the new thread.
  @return  None.

****************************************************************************************************
*/
void eConnection::initialize(
    eContainer *params)
{
    m_initialized = OS_TRUE;
    open();
}


/**
****************************************************************************************************

  @brief Run the connection.

  The eConnection::run() function keeps moves data trough the connection.
  For most of the time function waits in select call, until data should be read from/written
  to socket, or the eConnection object receives message, either to itself or one to be forwarded
  trough the connection.

  @return  None.

****************************************************************************************************
*/
void eConnection::run()
{
    osalSelectData selectdata;
    os_long try_again_ms = osal_rand(3000, 4000);
    eStatus s, auth_s;

    /* Run as long as thread is not requested to exit.
     */
    while (!exitnow())
    {
        /* If we have connected socket, wait for socket or thread event.
         */
        if (m_stream)
        {
            auth_s = handle_authentication_frames();
            /* If we are still authenticating, do not start the real communication.
             */
            if (auth_s == ESTATUS_PENDING) {
                os_sleep(50);
                continue;
            }
            if (ESTATUS_IS_ERROR(auth_s)) {
                close();
                os_sleep(50);
                continue;
            }

            /* Set slow timer for keepalive messages. About 1 per 30 seconds.
               This allows socket library to detect dead socket, and keeps
               sockets which are connected trough system which disconnects
               at inactivity enabled.
             */
            if (m_fast_timer_enabled != 0)
            {
                timer(try_again_ms + 27000);
                m_fast_timer_enabled = 0;
            }

            /* Wait for socket or thread event. The function will return error if
               socket is disconnected. Structure "selectdata" is set regardless of
               return code, for example read and close can be returned at same time,
               and thread event with anything else.
             */
            s = m_stream->select(&m_stream, 1, trigger(), &selectdata, 0, OSAL_STREAM_DEFAULT);
            if (s)
            {
                close();
                continue;
            }

            if (!m_connected)
            {
                connected();
            }

            /* Call alive() to process messages. If stream gets closed, step out here.
             */
            alive(EALIVE_RETURN_IMMEDIATELY);
            if (m_stream == OS_NULL) {
                continue;
            }

            /* If message queue for incoming messages is empty, flush writes.
             */
            if (/* m_message_queue->first() == OS_NULL && */ m_new_writes)
            {
                if (m_stream->writechar(E_STREAM_FLUSH))
                {
                    close();
                    continue;
                }
                if (m_stream->flush())
                {
                    close();
                    continue;
                }
                os_get_timer(&m_last_send);
                m_new_writes = OS_FALSE;
            }

            /* Read objects, as long we have whole objects to read.
             */
            while (m_stream->flushcount() > 0)
            {
                if (read()) {
                    close();
                    break;
                }
            }
        }

        /* No socket, wait for thread events and process them. Try periodically to open
           socket.
         */
        else
        {
            /* Enable faster timer, to try to reconnect about once per 3 seconds.
               broken sockets.
             */
            if (m_fast_timer_enabled != 1)
            {
                timer(try_again_ms);
                m_fast_timer_enabled = 1;
            }

            alive(EALIVE_RETURN_IMMEDIATELY);

            if (/* m_connection_failed_once && */ m_is_server ||
                (!m_enable && !has_client_bindings()))
            {
                break;
            }

            open();

            if (m_stream == OS_NULL) {
                alive(EALIVE_WAIT_FOR_EVENT);
            }
        }
    }
}


/**
****************************************************************************************************

  @brief New connection, transfer authentication frames to both directions.

  The eConnection::handle_authentication_frames() function sends and receives an authentication
  frame.

  @return  ESTATUS_SUCCESS Authentication frame has been received and processed.
           ESTATUS_PENDING Authentication frame not yet send, but no error thus far.
           Other return values indicate an error.

****************************************************************************************************
*/
eStatus eConnection::handle_authentication_frames()
{
    osalStatus ss;

    if (!m_authentication_frame_received) {
        if (m_auth_recv_buf == OS_NULL) {
            m_auth_recv_buf = (iocSwitchboxAuthenticationFrameBuffer*)
                os_malloc(sizeof(iocSwitchboxAuthenticationFrameBuffer), OS_NULL);
            os_memclear(m_auth_recv_buf, sizeof(iocSwitchboxAuthenticationFrameBuffer));
        }

        iocAuthenticationResults results;
        ss = icom_switchbox_process_authentication_frame(m_stream->osstream(),
            m_auth_recv_buf, &results);
        if (ss == OSAL_COMPLETED) {
            os_free(m_auth_recv_buf, sizeof(iocSwitchboxAuthenticationFrameBuffer));
            m_auth_recv_buf = OS_NULL;
            m_authentication_frame_received = OS_TRUE;
        }
        else if (ss != OSAL_PENDING) {
            osal_debug_error("eConnection: Valid authentication frame was not received");
            return ESTATUS_FAILED;
        }
    }

    /* If this is client, we cannot send authentication frame before receiving one from server.
     */
    if (!m_authentication_frame_sent && (m_is_server || m_authentication_frame_received))
    // if (!m_authentication_frame_sent)
    {
        os_char auto_password[IOC_PASSWORD_SZ];
        iocSwitchboxAuthenticationParameters prm;
        os_memclear(&prm, sizeof(prm));

        if (m_auth_send_buf == OS_NULL) {
            m_auth_send_buf = (iocSwitchboxAuthenticationFrameBuffer*)
                os_malloc(sizeof(iocSwitchboxAuthenticationFrameBuffer), OS_NULL);
            os_memclear(m_auth_send_buf, sizeof(iocSwitchboxAuthenticationFrameBuffer));

            if (m_is_server) {
                prm.network_name = eglobal->process_id;
                prm.user_name = "srv";
                prm.password = "pw";
            }
            else {
                /* If this is user interface, etc application?
                 */
                if (eglobal->active_login.user_name[0]) {
                    prm.user_name = eglobal->active_login.user_name;
                    /* We need to make sure that this is TLS connection and that we have accepted certificate before
                     * we can send password */
                    prm.password = eglobal->active_login.password;
                    prm.network_name = eglobal->process_id; /* Add network name to connect to table */
                }

                /* Otherwise this is independent process (no UI) connecting to other service or switchbox.
                 */
                else {
                    /* Should we be able to be able to set user and password? This would allow setting
                       up server account in advance. */
                    prm.user_name = eglobal->process_id;
                    osal_get_auto_password(auto_password, sizeof(auto_password));
                    prm.password = auto_password;
                    prm.network_name = eglobal->process_id; /* Add network name to connect to table */
                }
            }
        }

        ss = ioc_send_switchbox_authentication_frame(m_stream->osstream(),
            m_auth_send_buf, &prm);
        if (ss == OSAL_COMPLETED) {
            os_free(m_auth_send_buf, sizeof(iocSwitchboxAuthenticationFrameBuffer));
            m_auth_send_buf = OS_NULL;
            m_authentication_frame_sent = OS_TRUE;
        }
        else if (ss != OSAL_PENDING) {
            osal_debug_error("eConnection: Failed to send authentication frame");
            return ESTATUS_FAILED;
        }
    }

    if (!m_authentication_frame_sent ||
        !m_authentication_frame_received)
    {
        os_timeslice();
        m_stream->flush();
        return ESTATUS_PENDING;
    }
    return ESTATUS_SUCCESS;
}


/**
****************************************************************************************************

  @brief Incoming connection has been accepted.

  The eConnection::accepted() function adopts connected incoming stream and starts communicating
  through it.

  @return  None.

****************************************************************************************************
*/
eStatus eConnection::accepted(
    eStream *stream)
{
    eStatus s;

    delete m_stream;
    m_stream = stream;
    stream->adopt(this);

    s = connected();
    m_is_server = OS_TRUE;
    return s;
}


/**
****************************************************************************************************

  @brief Open the socket, etc. connechtion.

  The eConnection::open() connects socket, etc, to listening end point of another process.
  If the socket is already open, this object has not been initialized or IP address has
  not been set, this function.

  Memver variable m_socket points to socket, etc. if the eSocket oject exists and is connected
  (or being connected). If there is not open socket, the m_stream is OS_NULL.

  @return  None.

****************************************************************************************************
*/
void eConnection::open()
{
    eStatus s;

    /* Do nothing if we are socket exists, initialize() has not been called, we do not have
       IP address, or connection is disabled.
     */
    if (m_stream || !m_initialized || m_ipaddr->isempty() || !m_enable)
    {
        return;
    }

    /* New by class ID. Usually eSocket.
     */
    m_stream = (eStream*)newchild(m_stream_classid);

    /* Open the socket, etc.
     */
    s = m_stream->open(m_ipaddr->gets(), OSAL_STREAM_CONNECT|OSAL_STREAM_SELECT);
    if (s)
    {
        osal_debug_error_int("osal_stream_open failed: ", s);
        delete m_stream;
        m_stream = OS_NULL;
        return;
    }

    /* No new writes to socket, etc. yet
     */
    m_new_writes = OS_FALSE;
    m_authentication_frame_sent = OS_FALSE;
    m_authentication_frame_received = OS_FALSE;
    if (m_auth_send_buf) {
        os_free(m_auth_send_buf, sizeof(iocSwitchboxAuthenticationFrameBuffer));
        m_auth_send_buf = OS_NULL;
    }
    if (m_auth_recv_buf) {
        os_free(m_auth_recv_buf, sizeof(iocSwitchboxAuthenticationFrameBuffer));
        m_auth_recv_buf = OS_NULL;
    }
}


/**
****************************************************************************************************

  @brief Close the connection.

  The eConnection::close() function calls disconnected() to inform bindings and set
  connection state, then closes underlying stream and clears all member veriables
  for current connection state.

  @return  None.

****************************************************************************************************
*/
void eConnection::close()
{
    if (m_stream == OS_NULL) return;

    /* Write disconnect character.
     */
    if (m_connected)
    {
        m_stream->writechar(E_STREAM_DISCONNECT);
        m_stream->writechar(E_STREAM_FLUSH);
        m_stream->flush();
    }

    /* Inform bindings, set connection state to disconnected.
     */
    disconnected();

    /* Close thre stream
     */
    if (m_stream)
    {
        m_stream->close();
        delete m_stream;
        m_stream = OS_NULL;
    }
}


/**
****************************************************************************************************

  @brief Connection established event detected, act on it.

  The eConnection::connected() function:
  - Writes all queued data forward to connection.
  - Inform client bindings that theu can be reestablished.
  - Marks connection connected.

  @return If successfull, the function returns ESTATUS_SUCCESS. Other return values indicate
          an error and stream is to be closed.

****************************************************************************************************
*/
eStatus eConnection::connected()
{
    eEnvelope *envelope;
    eObject *mark;
    eName *name;
    eStatus s;

    /* Inform client bindings that the binding can be reestablished.
     */
    for (mark = m_client_bindings->first(); mark; mark = mark->next())
    {
        name = mark->firstn();
        message(ECMD_REBIND, name->gets());
    }

    /* Write everything in initialization buffer.
     */
    while ((envelope = eEnvelope::cast(m_initbuffer->first())))
    {
        /* Check for binding related messages, memorize bindings through this connection.
         */
        monitor_binds(envelope);

        if (write(envelope)) return ESTATUS_FAILED;
        delete envelope;
    }

    /* Mark that we are connected.
     */
    setpropertyl(ECONNP_ISOPEN, OS_TRUE);

    /* If we have something to write, flush it now.
     */
    if (m_new_writes)
    {
        s = m_stream->writechar(E_STREAM_FLUSH);
        if (s) return s;
        s = m_stream->flush();
        if (s) return s;
        os_get_timer(&m_last_send);
        m_new_writes = OS_FALSE;
    }

    return ESTATUS_SUCCESS;
}


/**
****************************************************************************************************

  @brief Closing connection (connection failed, disconnected event, etc), act on it.

  The eConnection::connected() function:
  - Send notarget to all items in initialization queue?
  - Inform all bindings that that there is no connection.
  - Marks connection disconnected and that connection has failed once.

  @return  None.

****************************************************************************************************
*/
void eConnection::disconnected()
{
    eEnvelope *envelope;
    eVariable *mark;
    eName *name;

    while ((envelope = eEnvelope::cast(m_initbuffer->first())))
    {
        /* Check for binding related messages, memorize bindings through this connection.
         */
        monitor_binds(envelope);
        notarget(envelope);
        delete envelope;
    }

    /* Inform all bindings that the connection is lost.
     */
    for (mark = m_client_bindings->firstv(); mark; mark = mark->nextv())
    {
        if (mark->getl()) {
            name = mark->firstn();
            message(ECMD_SRV_UNBIND, name->gets());
            mark->setl(0);
        }
    }
    for (mark = m_server_bindings->firstv(); mark; mark = mark->nextv())
    {
        if (mark->getl()) {
            name = mark->firstn();
            message(ECMD_UNBIND, name->gets());
            mark->setl(0);
        }
    }

    setpropertyl(ECONNP_ISOPEN, OS_FALSE);
    m_connection_failed_once = OS_TRUE;
    m_initbuffer->clear();
}


/**
****************************************************************************************************

  @brief Monitor messages for bind and unbind.

  The eConnection::monitor_binds() function maintains memorized client and server bindings.

  @param  envelope Envelope to write to connection.
  @return None.

****************************************************************************************************
*/
void eConnection::monitor_binds(
    eEnvelope *envelope)
{
    eContainer *bindings;
    eVariable *mark;
    os_int command;
    os_char *source;

    command = envelope->command();
    switch (command)
    {
        /* Bind request sent by Client binding, or client binding deleted.
         */
        case ECMD_BIND:
        case ECMD_BIND_RS:
        case ECMD_UNBIND:
            bindings = m_client_bindings;
            break;

        /* Server binding reply to ECMD_BIND/ECMD_BIND_RS, or server binding deleted.
         */
        case ECMD_BIND_REPLY:
        case ECMD_SRV_UNBIND:
            bindings = m_server_bindings;
            break;

        default:
            return;
    }

    source = envelope->source();
    mark = eVariable::cast(bindings->byname(source));

    switch (command)
    {
       case ECMD_BIND:
       case ECMD_BIND_RS:
       case ECMD_BIND_REPLY:
            if (mark == OS_NULL) {
                mark = new eVariable(bindings);
                mark->addname(source);
            }
            mark->setl(OS_TRUE);
            break;

        case ECMD_UNBIND:
        case ECMD_SRV_UNBIND:
            if (mark) {
                delete mark;
            }
            break;
    }
}


/**
****************************************************************************************************

  @brief Send an envelope to another process.

  The eConnection::write() function writes an envelope to socket, etc. stream.

  @param  envelope Envelope to write to connection.
  @return If successfull, the function returns ESTATUS_SUCCESS. Other return values indicate
          an error and stream is to be closed.

****************************************************************************************************
*/
eStatus eConnection::write(
    eEnvelope *envelope)
{
    eStatus s;

    if (m_stream == OS_NULL) return ESTATUS_FAILED;

    s = envelope->writer(m_stream, EOBJ_SERIALIZE_DEFAULT);
    if (!s) m_new_writes = OS_TRUE;
    return s;
}


/**
****************************************************************************************************

  @brief Read an envelope received from another process and pass it as messages.

  The eConnection::read() function reads an envelope from socket, etc. stream and forwards the
  envelope trough mormal messaging.

  @return If successfull, the function returns ESTATUS_SUCCESS. Other return values indicate
          an error and stream is to be closed.

****************************************************************************************************
*/
eStatus eConnection::read()
{
    eStatus s;

    if (m_stream == OS_NULL) return ESTATUS_FAILED;

    if (m_envelope == OS_NULL) {
        m_envelope = new eEnvelope(this);
    }

    s = m_envelope->reader(m_stream, EOBJ_SERIALIZE_DEFAULT);
    if (s == ESTATUS_NO_WHOLE_MESSAGES_TO_READ) {
        return ESTATUS_SUCCESS;
    }
    if (s) {
        delete(m_envelope);
        m_envelope = OS_NULL;
        return s;
    }

    if (*m_envelope->target() == '\0') {
        m_envelope->prependtarget("//");
    }
    else {
        m_envelope->prependtarget("/");
    }

    if ((m_envelope->mflags() & EMSG_NO_REPLIES) == 0)
    {
        os_char buf[E_OIXSTR_BUF_SZ+3];
        oixstr(buf, sizeof(buf));
        os_strncat(buf, "/_r", sizeof(buf));
        m_envelope->prependsource(buf);
    }
    m_envelope->addmflags(EMSG_NO_NEW_SOURCE_OIX);
    message(m_envelope);
    m_envelope = OS_NULL;
    return ESTATUS_SUCCESS;
}

