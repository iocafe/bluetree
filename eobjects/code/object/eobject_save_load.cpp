/**

  @file    eobject_save_load.cpp
  @brief   Object base class, saving object as a file.
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

   @brief Save this object to a file

   The eObject::save function serializes this object into a file.

   @param   path Relative path to target file within file system.

   @return  If the file was successfully writte, the function returns ESTATUS_SUCCESS.
            Other return values indicate an error.

****************************************************************************************************
*/
eStatus eObject::save(
    const os_char *path)
{
    eOsStream *stream;

    stream = new eOsStream(this, EOID_TEMPORARY, EOBJ_TEMPORARY_ATTACHMENT);

    stream->open("file:/coderoot/fsys/ukke.txt", OSAL_STREAM_WRITE);

    // classid
    // oid

    writer(stream, 0);

    stream->flush();
    stream->close();
    delete stream;
    return ESTATUS_SUCCESS;
}

/*
    eBuffer buf;

    json_write(&buf, sflags);
    buf.writechar('\0');

    osal_console_write(buf.ptr());
*/
