/**

  @file    eautolabel.h
  @brief   Generate hidden Dear ImGui labels by enumeration.
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
#ifndef EAUTOLABEL_H_
#define EAUTOLABEL_H_
#include "egui.h"

class eComponent;

#define E_AUTOLABEL_SZ 16

/**
****************************************************************************************************

  @brief eAutoLabel class.

  The eAutoLabel is automatically generated hidden label for IMGui.

****************************************************************************************************
*/
class eAutoLabel
{
public:
    /* Constructor and destructor.
     */
    eAutoLabel();

    /* Get pointer to label string.
     */
    os_char *get(eComponent *component);

protected:
    os_char m_label[E_AUTOLABEL_SZ];

};


#endif
