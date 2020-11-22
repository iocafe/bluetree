/**

  @file    eendpoint.cpp
  @brief   End point class.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    14.9.2020

  The eEndPoint is socket end point listening to specific TCP port for new connections.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "eobjects.h"

/* End point property names.
 */
const os_char
    eendpp_classid[] = "classid",
    eendpp_ipaddr[] = "ipaddr",
    eendpp_isopen[] = "isopen";


/**
****************************************************************************************************
  Constructor.
****************************************************************************************************
*/
eEndPoint::eEndPoint(
    eObject *parent,
    e_oid id,
    os_int flags)
    : eThread(parent, id, flags)
{
    /** Clear member variables and allocate eVariable for IP address.
     */
    m_stream = OS_NULL;
    m_initialized = OS_FALSE;
    m_open_failed = OS_FALSE;
    m_stream_classid = ECLASSID_OSSTREAM;
    m_ipaddr = new eVariable(this);
}


/**
****************************************************************************************************
  Virtual destructor.
****************************************************************************************************
*/
eEndPoint::~eEndPoint()
{
    close();
}


/**
****************************************************************************************************

  @brief Add eEndPoint to class list and class'es properties to it's property set.

  The eEndPoint::setupclass function adds eEndPoint to class list and class'es properties to
  it's property set. The class list enables creating new objects dynamically by class identifier,
  which is used for serialization functions. The property set stores static list of
  class'es properties and metadata for those.

****************************************************************************************************
*/
void eEndPoint::setupclass()
{
    const os_int cls = ECLASSID_ENDPOINT;
    eVariable *p;

    /* Synchronize, add the class to class list and properties to property set.
     */
    os_lock();
    eclasslist_add(cls, (eNewObjFunc)newobj, "eEndPoint");
    addproperty(cls, EENDPP_CLASSID, eendpp_classid, "class ID", EPRO_PERSISTENT|EPRO_SIMPLE);
    addproperty(cls, EENDPP_IPADDR, eendpp_ipaddr, "IP", EPRO_PERSISTENT|EPRO_SIMPLE);
    p = addpropertyl(cls, EENDPP_ISOPEN, eendpp_isopen, OS_FALSE, "is open", EPRO_NOONPRCH);
    p->setpropertys(EVARP_ATTR, "rdonly;chkbox");
    os_unlock();
}


/**
****************************************************************************************************

  @brief Called to inform the class about property value change (override).

  The onpropertychange() function is called when class'es property changes, unless the
  property is flagged with EPRO_NOONPRCH.
  If property is flagged as EPRO_SIMPLE, this function shuold save the property value
  in class members and and return it when simpleproperty() is called.

  Notice for change logging: Previous value is still valid when this function is called.
  You can get the old value by calling property() function inside onpropertychange()
  function.

  @param   propertynr Property number of changed property.
  @param   x Variable containing the new value.
  @param   flags
  @return  If successfull, the function returns ESTATUS_SUCCESS (0). Nonzero return values do
           indicate that there was no property with given property number.

****************************************************************************************************
*/
eStatus eEndPoint::onpropertychange(
    os_int propertynr,
    eVariable *x,
    os_int flags)
{
    switch (propertynr)
    {
        case EENDPP_CLASSID:
            m_stream_classid = (os_int)x->getl();
            close();
            open();
            break;

        case EENDPP_IPADDR:
            if (x->compare(m_ipaddr))
            {
                m_ipaddr->setv(x);
                close();
                open();
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

  The simpleproperty() function stores current value of simple property into variable x.

  @param   propertynr Property number to get.
  @param   x Variable into which to store the property value.
  @return  If property with property number was stored in x, the function returns
           ESTATUS_SUCCESS (0). Nonzero return values indicate that property with
           given number was not among simple properties.

****************************************************************************************************
*/
eStatus eEndPoint::simpleproperty(
    os_int propertynr,
    eVariable *x)
{
    switch (propertynr)
    {
        case EENDPP_CLASSID:
            x->setl(m_stream_classid);
            break;

        case EENDPP_IPADDR:
            x->setv(m_ipaddr);
            break;

        default:
            return eThread::simpleproperty(propertynr, x);
    }
    return ESTATUS_SUCCESS;
}


/**
****************************************************************************************************

  @brief Initialize the object.

  The initialize() function is called when new object is fully constructed.
  It marks end point object initialized, and opens listening end point if ip address
  for it is already set.

  @param   params Parameters for the new thread. Not used.

****************************************************************************************************
*/
void eEndPoint::initialize(
    eContainer *params)
{
    osal_console_write("initializing worker\n");

    m_initialized = OS_TRUE;
    open();
}


/**
****************************************************************************************************

  @brief End point main loop.

  The eEndPoint::run() function contains the main loop of eEndPoint thread. The run loop
  function waits for socket or thread events. When either is received, the function
  processes messages received by eEndPoint thread and tries to accept incoming socket
  connection.

  If a incoming is successfully accepted, a pointer to new eStream object returned:
  It creates an eConnection object, set it to use the accepted stream and start it
  as it's own thread.

****************************************************************************************************
*/
void eEndPoint::run()
{
    osalSelectData selectdata;
    eStream *newstream;
    eConnection *c;
    eStatus s;

    while (!exitnow())
    {
        /* If we are listening a socket port.
         */
        if (m_stream)
        {
            /* Wait forever for an incoming socket or thread event.
             */
            m_stream->select(&m_stream, 1, trigger(), &selectdata, 0, OSAL_STREAM_DEFAULT);
            osal_trace2("select pass");

            /* Call alive() to process thread events.
             */
            alive(EALIVE_RETURN_IMMEDIATELY);

            /* Try to accept incoming connection, if a incoming is successfully
             * accepted, a pointer to new eStream object returned: Create eConnection
             * object, set it to use the accepted stream and start it as own thread.
             */
            newstream = m_stream->accept(OSAL_STREAM_DEFAULT, &s, this, EOID_ITEM);
            if (newstream)
            {
                c = new eConnection();
                c->addname("//connection");
                s = c->accepted(newstream);
                if (s) {
                    delete c;
                    osal_debug_error_int("accepted() failed: ", s);
                }
                else {
                    c->start(); /* After this c pointer is useless */
                    osal_trace3("stream accepted");
                }
            }
            else if (s != ESTATUS_NO_NEW_CONNECTION)
            {
                osal_debug_error_int("accept() failed: ", s);
            }
        }

        /* Not listening for socket port, either configuration properties
           have not been set, or opening the port has failed. If open
           has failed, keep on retrying in case other process has
           reserved the port and happens to release it.
         */
        else
        {
            if (m_open_failed) {
                alive(EALIVE_RETURN_IMMEDIATELY);
                open();
                if (m_open_failed) {
                    os_sleep(500);
                }
            }
            else {
                alive(EALIVE_WAIT_FOR_EVENT);
            }
        }

        osal_trace2("eEndPoint running");
    }
}


/**
****************************************************************************************************

  @brief Open the end point.

  The eEndPoint::open() starts listening a socket port for incoming connections.

****************************************************************************************************
*/
void eEndPoint::open()
{
    eStatus s;

    m_open_failed = OS_FALSE;
    if (m_stream || !m_initialized || m_ipaddr->isempty()) return;

    /* New stream by class ID.
     */
    m_stream = (eStream*)newchild(m_stream_classid);

    s = m_stream->open(m_ipaddr->gets(), OSAL_STREAM_LISTEN|OSAL_STREAM_SELECT);
    if (s)
    {
        osal_debug_error_str("Opening listening stream failed", m_ipaddr->gets());
        delete m_stream;
        m_stream = OS_NULL;
        m_open_failed = OS_TRUE;
    }
    else
    {
        setpropertyl(EENDPP_ISOPEN, OS_TRUE);
    }
}


/**
****************************************************************************************************

  @brief Close the end point.

  The eEndPoint::close() function closes the listening socket.

****************************************************************************************************
*/
void eEndPoint::close()
{
    if (m_stream == OS_NULL) return;

    setpropertyl(EENDPP_ISOPEN, OS_FALSE);

    delete m_stream;
    m_stream = OS_NULL;
}
