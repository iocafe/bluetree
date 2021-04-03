/**

  @file    econtainer.cpp
  @brief   Simple object container.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  The container object is like a box holding a set of child objects.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "eobjects.h"


/* eContainer property names.
 */
const os_char
    econtp_text[] = "text";


/**
****************************************************************************************************
  Constructor.
****************************************************************************************************
*/
eContainer::eContainer(
    eObject *parent,
    e_oid oid,
    os_int flags)
    : eObject(parent, oid, flags)
{
}


/**
****************************************************************************************************
  Virtual destructor.
****************************************************************************************************
*/
eContainer::~eContainer()
{
}


/**
****************************************************************************************************

  @brief Clone object

  The eContainer::clone function clones and object including object's children.
  Names will be left detached in clone.

  @param  parent Parent for the clone.
  @param  oid Object identifier for the clone.
  @param  aflags 0 for default operation. EOBJ_NO_MAP not to map names.
  @return Pointer to the clone.

****************************************************************************************************
*/
eObject *eContainer::clone(
    eObject *parent,
    e_oid id,
    os_int aflags)
{
    eObject *clonedobj;
    clonedobj = new eContainer(parent, id == EOID_CHILD ? oid() : id, flags());
    clonegeneric(clonedobj, aflags|EOBJ_CLONE_ALL_CHILDREN);
    return clonedobj;
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
void eContainer::setupclass()
{
    const os_int cls = ECLASSID_CONTAINER;

    /* Add the class to class list.
     */
    os_lock();
    eclasslist_add(cls, (eNewObjFunc)newobj, "eContainer");
    addpropertys(cls, ECONTP_TEXT, econtp_text, "text", EPRO_PERSISTENT|EPRO_NOONPRCH);
    propertysetdone(cls);
    os_unlock();
}


/**
****************************************************************************************************

  @brief Get next child container identified by oid.

  The eVariable::nextc() function returns pointer to the next child container of this object.

  @param   id Object idenfifier. Default value EOID_CHILD specifies to count a child objects,
           which are not flagged as an attachment. Value EOID_ALL specifies to get count all
           child objects, regardless wether these are attachment or not. Other values
           specify object identifier, only children with that specified object identifier
           are searched for.

  @return  Pointer to the first child container, or OS_NULL if none found.

****************************************************************************************************
*/
eContainer *eContainer::nextc(
    e_oid id)
{
    if (mm_handle == OS_NULL) return OS_NULL;
    eHandle *h = mm_handle->next(id);
    while (h)
    {
        if (h->object()->classid() == ECLASSID_CONTAINER)
            return eContainer::cast(h->object());

        h = h->next(id);
    }
    return OS_NULL;
}


/**
****************************************************************************************************

  @brief Write container content to stream.

  The eContainer::writer() function serializes the container to stream. This writes only the
  content, use eObject::write() to save also class information, attachements, etc.

  @param  stream The stream to write to.
  @param  flags Serialization flags.

  @return If successfull the function returns ESTATUS_SUCCESS (0). If writing object to stream
          fails, value ESTATUS_WRITING_OBJ_FAILED is returned. Assume that all nonzero values
          indicate an error.

****************************************************************************************************
*/
eStatus eContainer::writer(
    eStream *stream,
    os_int flags)
{
    /* Version number. Increment if new serialized items are added to the object,
       and check for new version's items in read() function.
     */
    const os_int version = 0;
    eObject *child;

    /* Begin the object and write version number.
     */
    if (stream->write_begin_block(version)) goto failed;

    /* Write child count to stream (no attachments).
     */
    if (*stream << childcount())  goto failed;

    /* Write childern (no attachments).
     */
    for (child = first(); child; child = child->next())
    {
        child->write(stream, flags);
    }

    /* End the object.
     */
    if (stream->write_end_block()) goto failed;

    /* Object succesfully written.
     */
    return ESTATUS_SUCCESS;

    /* Writing object failed.
     */
failed:
    return ESTATUS_WRITING_OBJ_FAILED;
}


/**
****************************************************************************************************

  @brief Read container content from stream.

  The eContainer::reader() function reads serialized container from stream. This function
  reads only the object content. To read whole object including attachments, names, etc,
  use eObject::read().

  @param  stream The stream to read from.
  @param  flags Serialization flags.

  @return If successfull the function returns ESTATUS_SUCCESS (0). If writing object to stream
          fails, value ESTATUS_READING_OBJ_FAILED is returned. Assume that all nonzero values
          indicate an error.

****************************************************************************************************
*/
eStatus eContainer::reader(
    eStream *stream,
    os_int flags)
{
    eObject *child;
    os_int version;
    os_long count;

    /* Read object start mark and version number.
     */
    if (stream->read_begin_block(&version)) goto failed;

    /* Read child count (no attachments).
     */
    if (*stream >> count)  goto failed;

    /* Read children
     */
    while (count-- > 0) {
        child = read(stream, flags);
        if (child == OS_NULL) goto failed;
    }

    /* End the object.
     */
    if (stream->read_end_block()) goto failed;

    /* Object succesfully read.
     */
    return ESTATUS_SUCCESS;

    /* Reading object failed.
     */
failed:
    return ESTATUS_READING_OBJ_FAILED;
}


#if E_SUPPROT_JSON
/**
****************************************************************************************************

  @brief Write container specific content to stream as JSON.

  The eContainer::json_writer() function writes class specific object content to stream as JSON.

  @param  stream The stream to write to.
  @param  sflags Serialization flags. Typically EOBJ_SERIALIZE_DEFAULT.
  @param  indent Indentation depth, 0, 1... Writes 2x this spaces at beginning of a line.

  @return If successfull the function returns ESTATUS_SUCCESS (0). If writing object to stream
          fails, value ESTATUS_WRITING_OBJ_FAILED is returned. Assume that all nonzero values
          indicate an error.

****************************************************************************************************
*/
eStatus eContainer::json_writer(
    eStream *stream,
    os_int sflags,
    os_int indent)
{
    eObject *child;
    os_boolean comma = OS_FALSE, started = OS_FALSE;

    /* Write childern (no attachments).
     */
    for (child = first(); child; child = child->next())
    {
        if (!started) {
            if (json_puts(stream, "[")) goto failed;
            started = OS_TRUE;
        }

        if (child->json_write(stream, sflags, indent+1, &comma)) goto failed;
    }

    if (started) {
        if (json_indent(stream, indent, EJSON_NEW_LINE_BEFORE)) goto failed;
        if (json_puts(stream, "]")) goto failed;
    }

    return ESTATUS_SUCCESS;

failed:
    return ESTATUS_FAILED;
}
#endif


/**
****************************************************************************************************

  @brief Delete all children except attachments.

  The eContainer::clear() function deletes all object's children, excluding attachments.
  @return None.

****************************************************************************************************
*/
void eContainer::clear()
{
    eObject *o;

    while ((o = first())) {
        delete(o);
    }
}


/**
****************************************************************************************************

  @brief Collect information about this matrix for tree browser, etc.

  The eContainer::object_info function fills in item (eVariable) to contain information
  about this object for tree browser view.

  @param   item Pointer to eVariable to set up with object information.
  @param   name Object's name if known. OS_NULL if object is not named or name is
           unknown at this time.
  @param   appendix Pointer to eSet into which to store property flags. The stored property
           flags indicate if object has namespace, children, or properties.
  @param   target Path "within object" when browsing a tree which is not made out
           of actual eObjects. For example OS file system directory.

****************************************************************************************************
*/
void eContainer::object_info(
    eVariable *item,
    eVariable *name,
    eSet *appendix,
    const os_char *target)
{
    eObject::object_info(item, name, appendix, target);
    item->setpropertys(EVARP_ATTR, "button=open");
    appendix->setl(EBROWSE_RIGHT_CLICK_SELECTIONS, EBROWSE_OPEN);
}


/**
****************************************************************************************************

  @brief Information for opening object has been requested, send it.

  The object has received ECMD_INFO request and it needs to return back information
  for opening the object.

  @param   envelope Message envelope. Contains command, target and source paths and
           message content, etc.
  @return  None.

****************************************************************************************************
*/
void eContainer::send_open_info(
    eEnvelope *envelope)
{
    eContainer *request, *reply;
    eVariable *v;
    eName *name;
    os_int command = EBROWSE_OPEN;

    /* Get command
     */
    request = eContainer::cast(envelope->content());
    if (request->classid() != ECLASSID_CONTAINER) return;
    if (request) {
        v = request->firstv(EOID_PARAMETER);
        if (v) {
            command = v->geti();
        }
    }

    /* The "open" selection shows the variables and matrices in the container.
     */
    if (command == EBROWSE_OPEN)
    {
        /* Created container for reply content.
         */
        reply = new eContainer(this, EOID_ITEM, EOBJ_IS_ATTACHMENT);

        eVariable tmp;
        propertyv(ECONTP_TEXT, &tmp);
        if (tmp.isempty()) {
            name = primaryname();
            if (name) tmp = *name;
        }
        if (!tmp.isempty()) {
            reply->setpropertyv(ECONTP_TEXT, &tmp);
        }

        send_open_info_helper(reply);

        /* Send reply to caller
         */
        message(ECMD_OPEN_REPLY, envelope->source(),
            envelope->target(), reply, EMSG_DEL_CONTENT, envelope->context());
    }

    /* Otherwise use default implementation for properties, etc.
     */
    else {
        eObject::send_open_info(envelope);
    }
}


/**
****************************************************************************************************

  @brief Helper function for send_open_info()

****************************************************************************************************
*/
void eContainer::send_open_info_helper(
    eContainer *reply)
{
    eObject *o;
    eName *name;
    eVariable *item;
    eContainer *cont;
    os_int cid;
    os_boolean is_variable, is_container, is_matrix;

    for (name = eObject::ns_firstv(); name; name = name->ns_next(OS_FALSE))
    {
        o = name->parent();
        cid = o->classid();

        is_variable = eclasslist_isinstanceof(cid, ECLASSID_VARIABLE);
        is_container = is_matrix = OS_FALSE;
        if (!is_variable) {
            is_container = eclasslist_isinstanceof(cid, ECLASSID_CONTAINER);
            if (!is_container) {
                is_matrix = eclasslist_isinstanceof(cid, ECLASSID_MATRIX);
            }
        }

        if (is_variable || is_matrix || is_container)
        {
            item = new eVariable(reply, cid);
            item->setv(name);
            if (is_container) {
                cont = new eContainer(item, EOID_APPENDIX, EOBJ_IS_ATTACHMENT);
                ((eContainer*)o)->send_open_info_helper(cont);
                if (cont->childcount() < 1) delete cont;
            }
        }
    }
}
