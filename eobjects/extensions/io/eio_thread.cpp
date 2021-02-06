/**

  @file    eio_thread.cpp
  @brief   Thread to run the IO.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "extensions/io/eio.h"

/* Property names.
 */
/* const os_char
    eiothreadp_end_pont_table_modif_count[] = "publish",
    eiothreadp_end_point_config_count[] = "epconfigcnt",
    eiothreadp_connect_table_modif_count[] = "connect",
    eiothreadp_lighthouse_change_count[] = "lighthouse";
*/

/**
****************************************************************************************************
  Constructor.
****************************************************************************************************
*/
eioThread::eioThread(
    eObject *parent,
    e_oid oid,
    os_int flags)
    : eThread(parent, oid, flags)
{
    // initproperties();
    // ns_create();
}


/**
****************************************************************************************************
  Virtual destructor.
****************************************************************************************************
*/
eioThread::~eioThread()
{
}


/**
****************************************************************************************************

  @brief Add the class to class list and class'es properties to it's property set.

  The eVariable::setupclass function adds the class to class list and class'es properties to
  it's property set. The class list enables creating new objects dynamically by class identifier,
  which is used for serialization reader functions. The property set stores static list of
  class'es properties and metadata for those.

****************************************************************************************************
*/
void eioThread::setupclass()
{
    const os_int cls = ECLASSID_EIO_THREAD;

    /* Add the class to class list.
     */
    os_lock();
    eclasslist_add(cls, (eNewObjFunc)newobj, "eioThread");
    /* addpropertyl(cls, EIOTHREADP_END_POINT_TABLE_MODIF_COUNT, eiothreadp_end_pont_table_modif_count,
        -1, "end point table modif count", EPRO_DEFAULT);
    addpropertyl(cls, EIOTHREADP_END_POINT_CONFIG_COUNT, eiothreadp_end_point_config_count,
        0, "end point config count", EPRO_NOONPRCH);
    addpropertyl(cls, EIOTHREADP_CONNECT_TABLE_MODIF_COUNT, eiothreadp_connect_table_modif_count,
        -1, "connect table modif count", EPRO_DEFAULT);
    addpropertyl(cls, EIOTHREADP_LIGHTHOUSE_CHANGE_COUNT, eiothreadp_lighthouse_change_count,
        0, "lighthouse change count", EPRO_DEFAULT);
    propertysetdone(cls); */
    os_unlock();
}


/**
****************************************************************************************************

  @brief Function to process incoming messages.

  The eioThread::onmessage function handles messages received by object. If this function
  doesn't process message, it calls parent class'es onmessage function.

  Send UDP broadcasts by timer hit.

  @param   envelope Message envelope. Contains command, target and source paths and
           message content, etc.
  @return  None.

****************************************************************************************************
*/
void eioThread::onmessage(
    eEnvelope *envelope)
{
    /* If at final destination for the message.
     */
    if (*envelope->target()=='\0')
    {
        switch (envelope->command())
        {
            case ECMD_TIMER: /* No need to do anything, timer is used just to break event wait */
                return;

            default:
                break;
        }
    }

    /* Call parent class'es onmessage.
     */
    eThread::onmessage(envelope);
}


/**
****************************************************************************************************

  @brief Called to inform the class about property value change (override).

  The onpropertychange() function is called when class'es property changes, unless the
  property is flagged with EPRO_NOONPRCH.
  If property is flagged as EPRO_SIMPLE, this function shuold save the property value
  in class members and and return it when simpleproperty() is called.

  @param   propertynr Property number of changed property.
  @param   x Variable containing the new value.
  @param   flags
  @return  If successfull, the function returns ESTATUS_SUCCESS (0). Nonzero return values do
           indicate that there was no property with given property number.

****************************************************************************************************
*/
/* eStatus eioThread::onpropertychange(
    os_int propertynr,
    eVariable *x,
    os_int flags)
{
    os_int count;

    switch (propertynr)
    {
        case EIOTHREADP_END_POINT_TABLE_MODIF_COUNT:
            count = x->geti();
            if (count != m_end_point_table_modif_count) {
                m_end_point_table_modif_count = count;
                m_configure_end_points = OS_TRUE;
                os_get_timer(&m_end_point_config_timer);
                set_timer(100);
            }
            break;


        default:
            return eObject::onpropertychange(propertynr, x, flags);
    }

    return ESTATUS_SUCCESS;
} */


/* Overloaded eThread function to initialize new thread. Called after eioThread object is created.
 */
void eioThread::initialize(
    eContainer *params)
{
}


/* Overloaded eThread function to perform thread specific cleanup when thread exists: Release
   resources allocated for maintain client. This is a "pair" to initialize function.
 */
void eioThread::finish()
{
}


/**
****************************************************************************************************

  @brief Maintain connections and end points, thread main loop.

  The eioThread::run() function
  The data is collected from "connect" and "endpoint" tables, both these tables are global
  and owned by eProcess.

****************************************************************************************************
*/
void eioThread::run()
{

    while (OS_TRUE)
    {
        alive();
        if (exitnow()) {
            break;
        }


osal_debug_error("HERE IO THREAD LOOP")        ;

        /* if (!m_configure_end_points &&
            !m_configure_connections)
        {
            set_timer(0);
        } */
    }

}





/**
****************************************************************************************************

  @brief Set timer period, how often to recive timer messages.

  This function sets how oftern onmessage() is called with ECMD_TIMER argument by timer.
  Call this function instead of calling timer() directlt to avois repeated set or clear
  of the timer period.

  @param timer_ms Timer hit period in milliseconds, repeated. Set 0 to disable timer.

****************************************************************************************************
*/
/* void eioThread::set_timer(
    os_int timer_ms)
{
    if (timer_ms != m_timer_ms) {
        m_timer_ms = timer_ms;
        timer(timer_ms);
    }
}
*/



/**
****************************************************************************************************

  @brief Start IO thread.

****************************************************************************************************
*/
void eio_start_thread(
    eioRoot *eio_root,
    eThreadHandle *io_thread_handle)
{
    eioThread *t;

    /* Create and start thread to listen for t UDP multicasts,
       name it "_t" in process name space.
     */
    t = new eioThread();
    t->addname("//_iothread");

    t->set_iocom_root(eio_root->iocom_root());

    /* t->bind(EIOTHREADP_END_POINT_TABLE_MODIF_COUNT, netservice_name,
        enetservp_endpoint_table_change_counter); */

    t->start(io_thread_handle);
}

