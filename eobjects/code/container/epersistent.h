/**

  @file    epersistent.h
  @brief   Persistent container (saved to disc).
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
#ifndef EPERSISTENT_H_
#define EPERSISTENT_H_
#include "eobjects.h"


/**
****************************************************************************************************
  Defines
****************************************************************************************************
*/

/* Persistent object property numbers.
 */
#define EPERP_ROOT_PATH 10
#define EPERP_RELATIVE_PATH 20
#define EPERP_FILE 30
#define EPERP_SAVE_TIME_MS 40
#define EPERP_SAVE_LATEST_TIME_MS 50

/* Persistent object property names.
 */
extern const os_char
    eperp_root_path[],
    eperp_relative_path[],
    eperp_file[],
    eperp_save_time_ms[],
    eperp_save_latest_time_ms[];

/**
****************************************************************************************************
  ePersistent is like a box of objects.
****************************************************************************************************
*/
class ePersistent : public eContainer
{
public:
    /* Constructor.
     */
    ePersistent(
        eObject *parent = OS_NULL,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT);

    /* Virtual destructor.
     */
    virtual ~ePersistent();

    /* Clone object.
     */
    virtual eObject *clone(
        eObject *parent,
        e_oid id = EOID_CHILD,
        os_int aflags = 0);

    /* Casting eObject pointer to ePersistent pointer.
     */
    inline static ePersistent *cast(
        eObject *o)
    {
        e_assert_type(o, ECLASSID_PERSISTENT)
        return (ePersistent*)o;
    }

    /* Get class identifier.
     */
    virtual os_int classid() {return ECLASSID_PERSISTENT; }

    /* Static function to add class to propertysets and class list.
     */
    static void setupclass();

    /* Static constructor function for generating instance by class list.
     */
    static ePersistent *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new ePersistent(parent, id, flags);
    }

    /* Function to process incoming messages.
     */
    virtual void onmessage(
        eEnvelope *envelope);

    /* Called when property value changes.
     */
    virtual eStatus onpropertychange(
        os_int propertynr,
        eVariable *x,
        os_int flags);

    /* Process a callback from a child object.
     */
    virtual eStatus oncallback(
        eCallbackEvent event,
        eObject *obj,
        eObject *appendix);

    /* Load persistent object from local file system.
     */
    void load_file(
        const os_char *file_name);


protected:
    /**
    ************************************************************************************************
      Internal functions.
    ************************************************************************************************
    */

    /* Flags the peristent object changed (needs to be saved).
     */
    void touch();

    /* Check if enugh time has passed since last change to save the peristent data.
     */
    void check_save_timer();

    /* Save persistent object by sending it as message to file system.
     */
    void save_as_message();

    /* Get relative path, like "data/grumpy10"
     */
    void get_relative_path(
        eVariable *relative_path);

    /* Copy loaded data to use.
     */
    void use_loded_content(
        ePersistent *content);

    /* Copy loaded matrix data into used matrix.
     */
    void copy_loaded_matrix(
        eMatrix *dstm,
        eMatrix *srcm);

    /**
    ************************************************************************************************
      Member variables
    ************************************************************************************************
    */
    /* Timer value when the content was last changed.
     */
    os_timer m_latest_touch;

    /* Timer value of the first unsaved change.
     */
    os_timer m_oldest_touch;

    /* Save changes at this many milliseconds after last change.
     */
    os_long m_save_time;

    /* Save changes at latest after this many milliseconds.
     */
    os_long m_save_latest_time;

    /* Flag indicating that a timer has started.
     */
    os_boolean m_timer_set;
};

#endif
