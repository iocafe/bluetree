/**

  @file    eobject_messages.cpp
  @brief   Object base class.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  The eObject base class functionality related to message transport and processing.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "eobjects.h"


/**
****************************************************************************************************

  @brief Send message.

  The eObject::message() function sends message. The message will be recieved as onmessage call
  by another object.

  @param   command
  @param   target
  @param   source
  @param   content
  @param   mflags
  @param   context
  @return  None.

****************************************************************************************************
*/
void eObject::message(
    os_int command,
    const os_char *target,
    const os_char *source,
    eObject *content,
    os_int mflags,
    eObject *context)
{
    eEnvelope *envelope;
    eObject *parent;

    /* We use eRoot as pasent, in case object receiving message gets deleted.
       parent = this is just fallback mechanim.
     */
    if (mm_handle)
    {
        parent = mm_handle->m_root;
    }
    else
    {
        parent = this;
    }

    envelope = new eEnvelope(parent, EOBJ_IS_ATTACHMENT);
    envelope->setcommand(command);
    envelope->setmflags(mflags & ~(EMSG_DEL_CONTENT|EMSG_DEL_CONTEXT));
    envelope->settarget(target);
    if (source) envelope->prependsource(source);
    envelope->setcontent(content, mflags);
    envelope->setcontext(context, mflags);
    message(envelope);
}


/**
****************************************************************************************************

  @brief Send message.

  The eObject::message() function sends message. The message will be recieved as onmessage call
  by another object.

  @param   envelope Message envelope to send. Contains command, target and source paths and
           message content, etc.
  @return  None.

****************************************************************************************************
*/
void eObject::message(
    eEnvelope *envelope)
{
    os_char *target, *namespace_id;
    os_memsz sz;

    /* Resolve path.
     */
    if ((envelope->mflags() & EMSG_NO_RESOLVE) == 0)
    {
        envelope->addmflags(EMSG_NO_RESOLVE);
    }

    /* Add oix to source path when needed.
     */
    if ((envelope->mflags() & (EMSG_NO_REPLIES|EMSG_NO_NEW_SOURCE_OIX)) == 0)
    {
        envelope->prependsourceoix(this);
        envelope->addmflags(EMSG_NO_NEW_SOURCE_OIX);
    }

    target = envelope->target();

    switch (*target)
    {
      /* Process or thread name space.
       */
      case '/':
        /* If process name space.
         */
        if (target[1] == '/')
        {
            envelope->move_target_pos(2);
            message_process_ns(envelope);
        }

        /* Otherwise thread name space.
         */
        else
        {
            envelope->move_target_pos(1);
            message_within_thread(envelope, E_THREAD_NS);
        }
        return;

      /* Targer specified using object index
       */
      case '@':
        message_oix(envelope);
        return;

      /* Parent or this object's name space
       */
      case '.':
        /* If this object's name space.
         */
        if (target[1] == '/' || target[1] == '\0')
        {
            envelope->move_target_over_objname(1);
            message_within_thread(envelope, eobj_this_ns);
            return;
        }

        /* Otherwise parent name space.
         */
        else if (target[1] == '.')
             if (target[2] == '/' || target[2] == '\0')
        {
            envelope->move_target_over_objname(2);
            message_within_thread(envelope, E_PARENT_NS);
            return;
        }
        break;
    }

    /* Name or user specified name space.
     */
    eVariable nspacevar;
    envelope->nexttarget(&nspacevar);
    namespace_id = nspacevar.gets(&sz);
    envelope->move_target_over_objname((os_short)sz-1);

    message_within_thread(envelope, namespace_id);
}


/**
****************************************************************************************************

  @brief Send message within thread.

  The eObject::message_process_ns is helper function for eObject::message() to send message
  trough process name space. It finds to which thread's object tree the message target belongs
  to and places message to that thread's message queue.

  @param   envelope Message envelope to send. Contains command, target and source paths and
           message content, etc.
  @return  None.

****************************************************************************************************
*/
void eObject::message_within_thread(
    eEnvelope *envelope,
    const os_char *namespace_id)
{
    eNameSpace *nspace;
    eVariable objname;
    eName *name;
    os_memsz sz;

    nspace = findnamespace(namespace_id);
    if (nspace == OS_NULL) goto getout;

    /* Get next object name in target path.
        Remember length of object name.
     */
    envelope->nexttarget(&objname);
    objname.gets(&sz);

    /* Find the name in process name space. Done with objname.
     */
    name = nspace->findname(&objname);
    if (name == OS_NULL)
    {
        goto getout;
    }

    name->parent()->onmessage(envelope);
    delete envelope;
    return;

getout:

    /* Send "no target" reply message to indicate that recipient was not found.
     */
    if ((envelope->mflags() & EMSG_NO_REPLIES) == 0)
    {
        message (ECMD_NO_TARGET, envelope->source(),
            envelope->target(), OS_NULL,
            EMSG_DEL_CONTEXT,
            envelope->context());
    }

#if OSAL_DEBUG
    /* Report "no target: error
     */
    if ((envelope->mflags() & EMSG_NO_ERRORS) == 0)
    {
        osal_debug_error("message() failed: Name or namespace not found within thread");
    }
#endif

    delete envelope;
}


/**
****************************************************************************************************

  @brief Send message.

  The eObject::message_process_ns is helper function for eObject::message() to send message
  trough process name space. It finds to which thread's object tree the message target belongs
  to and places message to that thread's message queue.

  @param   envelope Message envelope to send. Contains command, target and source paths and
           message content, etc.
  @return  None.

****************************************************************************************************
*/
void eObject::message_process_ns(
    eEnvelope *envelope)
{
    eNameSpace *process_ns;
    eName *name, *nextname;
    eThread *thread;
    os_memsz sz;
    os_char buf[E_OIXSTR_BUF_SZ], *oname, c;
    os_boolean multiplethreads;

    /* If this is message to process ?
     */
    c = *envelope->target();

    if (c == '@')
    {
        message_oix(envelope);
        return;
    }

    /* Get pointer to process namespace. This is never NULL (or if it is, it is programming error).
     */
    process_ns = eglobal_process_ns();

    if (c == '\0')
    {
        /* Synchronize.
         */
        os_lock();

        if (eglobal->process == OS_NULL)
        {
            os_unlock();
#if OSAL_DEBUG
            osal_debug_error("message() failed: eobjects library not initialized");
#endif
            goto getout;
        }

        eglobal->process->queue(envelope);

        /* Done, finish with synchronization and return.
         */
        os_unlock();
        return;
    }

    /* Otherwise message to named object.
     */
    else
    {
        eVariable objname;

        /* Get next object name in target path.
           Remember length of object name.
         */
        envelope->nexttarget(&objname);
        oname = objname.gets(&sz);

        /* Synchronize.
         */
        os_lock();

        /* Find the name in process name space.
         */
        name = process_ns->findname(&objname);

        /* If name not found: End synchronization/clen up, reply with ECMD_NOTARGET
           and return.
         */
        if (name == OS_NULL)
        {
            os_unlock();
#if OSAL_DEBUG
            if ((envelope->flags() & EMSG_NO_ERRORS) == 0)
            {
                osal_debug_error("message() failed: Name not found in process NS");
                osal_debug_error(objname.gets());
            }
#endif
            goto getout;
        }

        /* Get thread to which the named object belongs to.
         */
        thread = name->thread();
        if (thread == OS_NULL)
        {
            os_unlock();
#if OSAL_DEBUG
            if ((envelope->flags() & EMSG_NO_ERRORS) == 0)
            {
                osal_debug_error("message() failed: Name in process NS has no eThread as root");
            }
#endif
            goto getout;
        }

        /* Check if targeted to multiple threads.
         */
        multiplethreads = OS_FALSE;
        for (nextname = name->ns_next(); nextname; nextname = nextname->ns_next())
        {
            if (nextname->thread() != thread)
            {
                multiplethreads = OS_TRUE;
                break;
            }
        }

        /* Single thread target (common case).
         */
        if (!multiplethreads)
        {
            /* If this is not message to thread itself.
             */
            if (thread != name->parent())
            {
                /* If object name is not already oix, convert to one.
                 */
                if (*oname != '@')
                {
                    envelope->move_target_over_objname((os_short)sz - 1);
                    name->parent()->oixstr(buf, sizeof(buf));
                    envelope->prependtarget(buf);
                }
            }
            else
            {
                /* Remove object name from envelope's target path.
                 */
                envelope->move_target_over_objname((os_short)sz - 1);
            }

            /* Move the envelope to thread's message queue.
             */
            thread->queue(envelope);
        }

        /* Multiple threads.
         */
        else
        {
            /* Save target path in envelope without name of next target.
             */
            envelope->move_target_over_objname((os_short)sz - 1);

            eVariable savedtarget, mytarget;
            savedtarget.sets(envelope->target());

            while (name)
            {
                nextname = name->ns_next();

                /* Get thread object pointer.
                 */
                thread = name->thread();

                /* If message is not to thread itself.
                   Here we do replace the name with oix string, even
                   if it is already that. We do this simply because
                   this is unusual case where efficiency doesn not
                   matter much.
                 */
                if (thread != name->parent())
                {
                    name->parent()->oixstr(buf, sizeof(buf));
                    mytarget.sets(buf);
                    if (!savedtarget.isempty()) mytarget.appends("/");
                    mytarget.appendv(&savedtarget);
                    envelope->settarget(mytarget.gets());
                }
                else
                {
                    envelope->settarget(savedtarget.gets());
                }

                /* Queue the envelope and move on. If this is last target for
                   the envelope, allow adopting the envelope.
                 */
                thread->queue(envelope, nextname == OS_NULL);
                name = nextname;
            }
        }

        /* End synchronization
         */
        os_unlock();
    }

    return;

getout:
    /* Send "no target" reply message to indicate that recipient was not found.
     */
    if ((envelope->mflags() & EMSG_NO_REPLIES) == 0)
    {
        message (ECMD_NO_TARGET, envelope->source(),
            envelope->target(), OS_NULL, EMSG_DEL_CONTEXT, envelope->context());
    }

    delete envelope;
}


/**
****************************************************************************************************

  @brief Send message.

  The eObject::message_oix is helper function for eObject::message() to send message
  using object index string, like "@11_1". It finds to which thread's object tree the message
  target belongs to. If this is in same object tree as the sender of the message message,
  then object's onmessage function is called directly. If target belongs to different object
  three from sender, the function places message to target thread's message queue.

  @param   envelope Message envelope to send. Contains command, target and source paths and
           message content, etc.
  @return  None.

****************************************************************************************************
*/
void eObject::message_oix(
    eEnvelope *envelope)
{
    eHandle *handle;
    eThread *thread;
    e_oix oix;
    os_int ucnt;
    os_short count;

    /* Parse object index and use count from string.
     */
    count = oixparse(envelope->target(), &oix, &ucnt);
    if (count == 0)
    {
#if OSAL_DEBUG
        if ((envelope->flags() & EMSG_NO_ERRORS) == 0)
        {
            osal_debug_error("message() failed: object index format error, not \"@11_2\" format");
        }
#endif
        goto getout;
    }

    /* Synchnronize and find handle pointer.
     */
    os_lock();
    handle = eget_handle(oix);
    if (ucnt != handle->m_ucnt)
    {
        os_unlock();
#if OSAL_DEBUG
        if ((envelope->flags() & EMSG_NO_ERRORS) == 0)
        {
            osal_debug_error("message() failed: target object has been deleted");
        }
#endif
        goto getout;
    }

    /* If object is in same root tree (same thread), end syncronization and call function.
     */
    if (mm_handle->m_root == handle->m_root)
    {
        /* Advance in target path.
         */
        envelope->move_target_over_objname(count);

        os_unlock();
        handle->m_object->onmessage(envelope);
        delete envelope;
        return;
    }

    /* Otherwise different threads.
     */
    osal_debug_assert(handle->m_root);
    thread = eThread::cast(handle->m_root->parent());
    if (thread == handle->m_object) envelope->move_target_over_objname(count);

    /* Place the envelope in thread's message queue.
     */
    if (thread)
    {
        thread->queue(envelope);
    }
    else
    {
        delete envelope;
    }

    /* Finish with synchronization and return.
     */
    os_unlock();
    return;

getout:
    /* Send "no target" reply message to indicate that recipient was not found.
     */
    if ((envelope->mflags() & EMSG_NO_REPLIES) == 0)
    {
        message (ECMD_NO_TARGET, envelope->source(),
            envelope->target(), OS_NULL, EMSG_DEL_CONTEXT, envelope->context());
    }

    delete envelope;
}


/**
****************************************************************************************************

  @brief Function to process incoming messages.

  The eObject::onmessage function handles messages received by object.

  @param   envelope Message envelope. Contains command, target and source paths and
           message content, etc.
  @return  None.

****************************************************************************************************
*/
void eObject::onmessage(
    eEnvelope *envelope)
{
    os_char *target;
    eNameSpace *nspace;
    eName *name, *nextname;
    os_memsz sz;
    os_int command;

    target = envelope->target();

    switch (*target)
    {
        /* Message to child object using object idenfifier.
         */
        case '@':
            onmessage_oix(envelope);
            break;

        /* Message to this object.
         */
        case '\0':
            command = envelope->command();
            switch (command)
            {
              case ECMD_BIND:
                srvbind(envelope);
                return;

              case ECMD_UNBIND:
                /* THIS IS TRICKY: WE NEED TO FIND BINDING BY SOURCE
                    PATH AND FORWARD THIS TO IT */
                break;

              case ECMD_INFO_REQUEST:
                send_browse_info(envelope);
                return;
            }
            osal_debug_error("onmessage(): Message not processed");
            break;

        /* Messages to internal names
         */
        case '_':
            command = envelope->command();
            /* If properties
             */
            if (target[1] == 'p')
            {
                /* Commands to specific property.
                 */
                if (target[2] == '/')
                {
                    switch (command)
                    {
                      case ECMD_SETPROPERTY:
                        setpropertyv(propertynr(target+3),
                            eVariable::cast(envelope->content()));
                        return;
                    }
                }
            }

            /* continues...
             */

        /* Messages to named child objects.
         */
        default:
            {
                eVariable objname;
                envelope->nexttarget(&objname);
                objname.gets(&sz);
                envelope->move_target_over_objname((os_short)sz-1);

                nspace = eNameSpace::cast(first(EOID_NAMESPACE));
                if (nspace) {
                    name = nspace->findname(&objname);
                }
                else {
                    name = OS_NULL;
                }
            }
            if (name == OS_NULL) goto getout;

            do
            {
                nextname = name->ns_next();
                name->parent()->onmessage(envelope);
                name = nextname;
            }
            while (name);

            break;
    }

    return;

getout:
    /* Send "no target" reply message to indicate that recipient was not found.
     */
    if ((envelope->mflags() & EMSG_NO_REPLIES) == 0)
    {
        message (ECMD_NO_TARGET, envelope->source(),
            envelope->target(), OS_NULL, EMSG_KEEP_CONTENT, envelope->context());
    }

#if OSAL_DEBUG
    /* Show error message.
     */
    if ((envelope->mflags() & EMSG_NO_ERRORS) == 0)
    {
        osal_debug_error("onmessage() failed: target not found");
    }
#endif
}


/**
****************************************************************************************************

  @brief Object information request by tree browser node, etc. Reply to it.

  The eObject::send_browse_info function is called when the object received ECMD_INFO_REQUEST
  message. It send object information back with ECMD_INFO_REPLY.

  @param   envelope Message envelope. Contains command, target and source paths and
           message content, etc.
  @return  None.

****************************************************************************************************
*/
void eObject::send_browse_info(
    eEnvelope *envelope)
{
    eContainer *content;
    eVariable *item;
    eName *name;

    content = new eContainer;

    /* Store information about this object.
     */
    item = new eVariable(content, EBROWSE_THIS_OBJECT);
    name = firstname();
    object_info(item, name);

    /* If this object has name space, list named objects.
     */
    browse_list_namespace(content);

    /* List child objects. Used for browsing.
     */
    browse_list_children(content);

    /* List object properties
     */
    browse_list_properties(content);

    /* Send reply to caller
     */
    message(ECMD_INFO_REPLY, envelope->source(),
        envelope->target(), content, EMSG_DEL_CONTENT, envelope->context());
}


/* List names in this object's namespace. Used for browsing.
 */
void eObject::object_info(
    eVariable *item,
    eName *name)
{
    eVariable text;

    if (name) {
        text = *name;
        text += " [";
    }
    else {
        text = "[";
    }
    text += classname();
    text += "]";

    item->setpropertyv(EVARP_TEXT, &text);
}


/* List names in this object's namespace. Used for browsing.
 */
void eObject::browse_list_namespace(
    eContainer *content)
{
    eName *name;
    eVariable *item;
    eSet *appendix;
    eObject *obj;
    os_char buf[E_OIXSTR_BUF_SZ];
    bool is_process;

    is_process = (classid() == ECLASSID_PROCESS);

    for (name = eObject::ns_firstv(); name; name = name->ns_next(OS_FALSE))
    {
        item = new eVariable(content, EBROWSE_IN_NSPACE);
        appendix = new eSet(item, EOID_APPENDIX, EOBJ_IS_ATTACHMENT);
        appendix->set(EBROWSE_PATH, name);

        /** Get oix and ucnt as string.
         */
        name->parent()->oixstr(buf, sizeof(buf));
        appendix->sets(EBROWSE_IPATH, buf);

        // item->addname(name->gets(), ENAME_THIS_NS|ENAME_NO_MAP);

        // item->sets(EBROWSE_ITEM_NAMESPACE_ID, name->namespaceid());
        // item->setl(EBROWSE_NAME_IS_MAPPED, name->is_mapped());

        if (!is_process) {
            obj = name->parent();
            obj->object_info(item, name);
        }
        else {
            item->setpropertyv(EVARP_TEXT, name);
        }
    }
}


/* List child objects. Used for browsing.
 */
void eObject::browse_list_children(
    eContainer *content)
{
    eObject *child;
    eVariable *item;
    eSet *appendix;
    eName *name;
    os_char buf[E_OIXSTR_BUF_SZ];
    os_int oid = EOID_ALL;

    for (child = first(oid); child; child = child->next(oid))
    {
        item = new eVariable(content, EBROWSE_CHILD);
        appendix = new eSet(item, EOID_APPENDIX, EOBJ_IS_ATTACHMENT);

        name = child->firstname();
        if (name) {
            appendix->set(EBROWSE_PATH, name);
        }

        /** Get oix and ucnt as string.
         */
        child->oixstr(buf, sizeof(buf));
        appendix->sets(EBROWSE_IPATH, buf);

        child->object_info(item, name);
    }
}

/* List child objects. Used for browsing.
 */
void eObject::browse_list_properties(
    eContainer *content)
{
    eVariable *p, *item, value;
    eSet *appendix;
    eName *name;
    os_int propertynr;

    for (p = firstp(EOID_CHILD, EPRO_NO_ERRORS); p; p = p->nextp())
    {
        item = eVariable::cast(p->clone(content, EBROWSE_PROPERTY,
            EOBJ_NO_CLONED_NAMES|EOBJ_NO_MAP));

        propertynr = p->oid();
        propertyv(propertynr, &value);
        *item = value;

        appendix = new eSet(item, EOID_APPENDIX, EOBJ_IS_ATTACHMENT);
        name = p->firstname();
        if (name) {
            appendix->set(EBROWSE_IPATH, name);
            delete name;
        }
    }
}


/**
****************************************************************************************************

  @brief Forward message by object index within thread's object tree.

  The eObject::onmessage_oix forwards message using object index string, like "@11_1".
  This function works only within thread.

  @param   envelope Message envelope to send. Contains command, target and source paths and
           message content, etc.
  @return  None.

****************************************************************************************************
*/
void eObject::onmessage_oix(
    eEnvelope *envelope)
{
    eHandle *handle;
    e_oix oix;
    os_int ucnt;
    os_short count;

    /* Parse object index and use count from string.
     */
    count = oixparse(envelope->target(), &oix, &ucnt);
    if (count == 0)
    {
#if OSAL_DEBUG
        if ((envelope->flags() & EMSG_NO_ERRORS) == 0)
        {
            osal_debug_error("onmessage() failed: object index format error, not \"@11_2\" format");
        }
#endif
        goto getout;
    }

    /* Find handle pointer.
     */
    handle = eget_handle(oix);
    if (ucnt != handle->m_ucnt)
    {
#if OSAL_DEBUG
        if ((envelope->flags() & EMSG_NO_ERRORS) == 0)
        {
            osal_debug_error("message() failed: target object has been deleted");
        }
#endif
        goto getout;
    }

    /* Here object must be in same root tree (same thread).
     */
    osal_debug_assert(mm_handle != OS_NULL);
    osal_debug_assert(mm_handle->m_root == handle->m_root);

    /* Advance in target path and call function.
     */
    envelope->move_target_over_objname(count);
    handle->m_object->onmessage(envelope);

    return;

getout:
    /* Send "no target" reply message to indicate that recipient was not found.
     */
    if ((envelope->mflags() & EMSG_NO_REPLIES) == 0)
    {
        message (ECMD_NO_TARGET, envelope->source(),
            envelope->target(), OS_NULL, EMSG_DEL_CONTEXT, envelope->context());
    }
}
