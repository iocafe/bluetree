/**

  @file    eobject_save_load.cpp
  @brief   Object base class, saving object as a file and loading object from file.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

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

   @param   path OS path to target file.
   @return  If the file was successfully written, the function returns ESTATUS_SUCCESS.
            Other return values indicate an error.

****************************************************************************************************
*/
eStatus eObject::save(
    const os_char *path)
{
    eVariable tmp;
    eOsStream *stream;
    eStatus s;

    /* Open file as stream.
     */
    stream = new eOsStream(ETEMPORARY);
    tmp.sets("file:");
    tmp.appends(path);
    s = stream->open(tmp.gets(), OS_NULL, OSAL_STREAM_WRITE);
    if (s) goto failed;

    /* Write file content.
     */
    s = write(stream, OSAL_STREAM_DEFAULT);
    if (s) goto failed;

    /* Flush and close the file.
     */
    stream->flush();
    stream->close();

failed:
    delete stream;
    return s;
}


/**
****************************************************************************************************

   @brief Load object from a file

   The eObject::load function reads a serialized object from a file.

   @param   path OS path to source file.
   @return  Pointer to loaded object, or OS_NULL if reading file has failed.

****************************************************************************************************
*/
eObject *eObject::load(
    const os_char *path)
{
    eVariable tmp;
    eOsStream *stream;
    eObject *obj = OS_NULL;
    eStatus s;

    /* Open file as stream.
     */
    stream = new eOsStream(ETEMPORARY);
    tmp.sets("file:");
    tmp.appends(path);
    s = stream->open(tmp.gets(), OS_NULL, OSAL_STREAM_READ);
    if (s) goto failed;

    /* Read file content.
     */
    obj = read(stream, OSAL_STREAM_DEFAULT);
    if (obj) goto failed;

    /* close the file.
     */
    stream->close();

failed:
    delete stream;
    return obj;
}

/*
    eBuffer buf;

    json_write(&buf, sflags);
    buf.writechar('\0');

    osal_console_write(buf.ptr());
*/
