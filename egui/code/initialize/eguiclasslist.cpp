/**

  @file    eguiclasslist.cpp
  @brief   Class list for egui library.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    10.9.2020

  Maintain list of classes which can be created dynamically by class ID.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "egui.h"

/**
****************************************************************************************************

  @brief Initialize class list and property sets.

  The eguiclasslist_initialize function must be called before any egui library objects are created.

****************************************************************************************************
*/
void eguiclasslist_initialize()
{
    /* eVariable should be first to add to class list followed by then eSet and eContainer.
       Reason is that these same classes are used to store description of classes, including
       themselves.
     */
    eGui::setupclass();
    eComponent::setupclass();
    ePopup::setupclass();
    eWindow::setupclass();
    eLineEdit::setupclass();
    eTreeNode::setupclass();
    eButton::setupclass();
    eTableView::setupclass();
    eTableColumn::setupclass();
    eParameterList::setupclass();
    eCameraView::setupclass();
    eGameController::setupclass();
    eLoginDialog::setupclass();
}

/* Release resources allocated for the class list.
 */
void eguiclasslist_release()
{
}
