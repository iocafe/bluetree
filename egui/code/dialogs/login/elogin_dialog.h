/**

  @file    elogin_dialog.h
  @brief   User login dialog.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    15.9.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef ELOGIN_DIALOG_H_
#define ELOGIN_DIALOG_H_
#include "egui.h"


/**
****************************************************************************************************
  eLoginDialog class.
****************************************************************************************************
*/
/* Current login data. This is set up as basic C structure instead of eobjects data structure
   to make doubly sure that this data cannot be accessed by browsing, etc. generic method.
 */
typedef struct {
    os_char user_name[OSAL_LONG_USER_NAME_SZ];
    os_char password[OSAL_SECRET_STR_SZ];
    os_boolean display_row;
    os_boolean save_password;
}
eLoginRow;

#define ELOGIN_MAX_ROWS  4

typedef struct
{
    /* Log in rows (pre filles user names and perhaps passwords).
     */
    eLoginRow rows[ELOGIN_MAX_ROWS];

    /* Currently selected row.
     */
    os_int selected_row;

    /* To validate that this structure is loaded and decrypted correctly.
     */
    os_ushort checksum;
}
eLoginData;


/**
****************************************************************************************************
  eLoginDialog class.
****************************************************************************************************
*/
class eLoginDialog : public eWindow
{
    /**
    ************************************************************************************************
      Generic eObject functionality.
    ************************************************************************************************
    */
public:
    /* Constructor.
     */
    eLoginDialog(
        eObject *parent = OS_NULL,
        e_oid id = EOID_GUI_WINDOW,
        os_int flags = EOBJ_DEFAULT);

    /* Virtual destructor.
     */
    virtual ~eLoginDialog();

    /* Clone object.
     */
    virtual eObject *clone(
        eObject *parent,
        e_oid id = EOID_CHILD,
        os_int aflags = 0);

    /* Casting eObject pointer to eLoginDialog pointer.
     */
    inline static eLoginDialog *cast(
        eObject *o)
    {
        e_assert_type(o, EGUICLASSID_LOGIN_DIALOG)
        return (eLoginDialog*)o;
    }

    /* Get class identifier.
     */
    virtual os_int classid() {return EGUICLASSID_LOGIN_DIALOG; }

    /* Static function to add class to propertysets and class list.
     */
    static void setupclass();

    /* Called when property value changes.
     */
    virtual eStatus onpropertychange(
        os_int propertynr,
        eVariable *x,
        os_int flags);

    /* Static constructor function for generating instance by class list.
     */
    static eLoginDialog *newobj(
        eObject *parent,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return new eLoginDialog(parent, id, flags);
    }


    /**
    ************************************************************************************************
      GUI component functionality (eComponent)
    ************************************************************************************************
    */
    /* Draw the component.
     */
    virtual eStatus draw(
        eDrawParams& prm);

protected:

    /**
    ************************************************************************************************
      Protected functions
    ************************************************************************************************
    */

    void set_select(
        os_int select_row,
        os_boolean can_open_password_dialog);

    void setup_default_data();


    /**
    ************************************************************************************************
      Member variables.
    ************************************************************************************************
    */

    // eAutoLabel m_label_title;
    //eStrBuffer m_text;
    eLoginData m_data;
    os_int m_show_popup;
    os_int m_popup_row;
    os_char m_password_buf[OSAL_SECRET_STR_SZ];
};


#endif
