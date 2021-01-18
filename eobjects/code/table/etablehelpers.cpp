/**

  @file    etablehelpers.cpp
  @brief   Helper functions for using tables.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "eobjects.h"


/**
****************************************************************************************************

  @brief Get column index by name.

  Gets column index 0... from by column name table configuration's "columns" container.
  This function should be used when it is known that column does exist, like programmatically
  created tables: If column is not found by name, it is treated as program error.

  @param   column_name Name of the column to search for.
  @param   columns eContainer holding table column information in table configuration.

  @return  Column index from 0 to number of columns. -1 if column was not found.

****************************************************************************************************
*/
os_int etable_column_ix(
    const os_char *column_name,
    eContainer *columns)
{
    eObject *col;

    if (columns == OS_NULL) {
        osal_debug_error_str("etable_column_ix: NULL columns pointer when searching ", column_name);
        return -1;
    }

    col = columns->byname(column_name);
    if (col == OS_NULL) {
        osal_debug_error_str("etable_column_ix: Unknown column ", column_name);
        return -1;
    }

    return col->oid();
}
