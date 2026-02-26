/**

  @file    queue1.cpp
  @brief   Queue unit test.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    27.11.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "eobjects.h"
#include "queue.h"


/**
****************************************************************************************************
  Container example.
****************************************************************************************************
*/
void queue_example1()
{
    eQueue q;
    os_char data[100000];
    os_char back[sizeof(data)];
    const os_char *text;
    os_memsz data_sz, in_queue, nread;
    eStatus s;
    os_int i;
    os_int count = 2;

    text = "pekka N\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAAk testaaaaapi";
    data_sz = os_strlen(text) - 1;
    os_strncpy(data, text, data_sz + 1);

    s = q.open(OS_NULL, OSAL_STREAM_ENCODE_ON_WRITE | OSAL_STREAM_DECODE_ON_READ);
    osal_debug_assert(s == ESTATUS_SUCCESS);

    for (i = 0; i < count; i++) {
        s = q.write(data, data_sz);
        osal_debug_assert(s == ESTATUS_SUCCESS);
    }

    in_queue = q.bytes();
    osal_debug_error_int("in queue: ", in_queue);

    s = q.readx(back, sizeof(back), &nread);
    osal_debug_assert(s == ESTATUS_SUCCESS);
    osal_debug_assert(nread == count * data_sz);

    back[nread] = '\0';
    osal_console_write(back);

    for (i = 0; i < count; i++) {
        if (os_memcmp(data, back + i * data_sz, data_sz)) {
            osal_debug_error("NOT SAME DATA BACK");
        }
    }

    q.close();
}

