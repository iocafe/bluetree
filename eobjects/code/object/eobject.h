/**

  @file    eobject.h
  @brief   Object base class.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

  The eObject is base class for all objects.

  - Functions to manage object hierarchy and idenfify objects.
  - Functions to clone objects.
  - Serialization functions.
  - Messaging.
  - Functions to access object properties.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef EOBJECT_H_
#define EOBJECT_H_
#include "eobjects.h"

class eHandle;
class eRoot;
class eVariable;
class eMatrix;
class eSet;
class eContainer;
class eNameSpace;
class eName;
class eStream;
class eEnvelope;
class eThread;
class ePointer;
class ePropertyBinding;
class eRowSetBinding;

/* Flags for message()
 */
#define EMSG_DEFAULT 0
#define EMSG_NO_REPLIES 1
#define EMSG_KEEP_CONTENT 0
#define EMSG_KEEP_CONTEXT 0
#define EMSG_NO_RESOLVE 2
#define EMSG_NO_NEW_SOURCE_OIX 4
#define EMSG_NO_ERRORS 8
#define EMSG_INTERTHREAD 16 /* Message has been passed from thread to another */
#define EMSG_DEL_CONTENT 128
#define EMSG_DEL_CONTEXT 256
#define EMSG_CAN_BE_ADOPTED 512 /* Internal: True if envelope or message can be adopted */
#define EMSG_IGNORE_MISSING_PROPERTY  8092 /* Used for propertyv, etc */
#define EMSG_HAS_CONTENT 2 /* Special flag to be passed over connection only */
#define EMSG_HAS_CONTEXT 4 /* Special flag to be passed over connection only */

/* Macro to debug object type casts.
 */
#if OSAL_DEBUG == 0
  #define e_assert_type(o,id)
#else
  #define e_assert_type(o,id) if (o) osal_debug_assert(eclasslist_isinstanceof((o)->classid(), (id)));
#endif

/* Flags for addproperty() function.
 */
#define EPRO_DEFAULT 0
#define EPRO_PERSISTENT EOBJ_CUST_FLAG1
#define EPRO_METADATA EOBJ_CUST_FLAG2
#define EPRO_SIMPLE EOBJ_CUST_FLAG3
#define EPRO_NOONPRCH EOBJ_CUST_FLAG4
#define EPRO_NOPACK EOBJ_CUST_FLAG5
#define EPRO_EARLYPRCH EOBJ_CUST_FLAG6
#define EPRO_RDONLY EOBJ_CUST_FLAG7

/* Flags for propertyset() and firstp() functions.
 */
#define EPRO_NO_ERRORS EMSG_NO_ERRORS

/* Serialization flags eObject::write(), eObject::read() and clonegeeric() functions.
 */
#define EOBJ_SERIALIZE_DEFAULT 0
#define EOBJ_JSON_ONLY_CONTENT 0x10
#define EOBJ_JSON_LIST_NAMESPACE 0x20
#define EOBJ_JSON_EXPAND_NAMESPACE 0x40

/* Flags for json_indent()
 */
#if E_SUPPROT_JSON
#define EJSON_NO_NEW_LINE 0
#define EJSON_NEW_LINE_BEFORE 1
#define EJSON_NEW_LINE_ONLY 2
#endif

/* Browse object identifier numbers (used as object id and as bit field in browse flags).
 */
#define EBROWSE_THIS_OBJECT 1
#define EBROWSE_NSPACE 2
#define EBROWSE_CHILDREN 4
#define EBROWSE_ALL_CHILDREN 8
#define EBROWSE_PROPERTIES 16
#define EBROWSE_BROWSE_FLAGS 32

/* More browse object identifier numbers (used bot object id but not in bit fields).
  - EBROWSE_OBJECT_FLAGS: EOBJ_IS_ATTACHMENT, EOBJ_CUST_FLAG1, etc as returned
    by eObject::flags().
  - EBROWSE_RIGHT_CLICK_SELECTIONS: Which right click selections to show "open" in right
    click popup menu for this object. Like "open", "graph", etc. in right click menu.
 */
#define EBROWSE_OBJECT_FLAGS 33
#define EBROWSE_RIGHT_CLICK_SELECTIONS 34

/* Browse appendix indices.
 */
#define EBROWSE_PATH 1
#define EBROWSE_IPATH 2
#define EBROWSE_ITEM_TYPE 3

/* Bits for EBROWSE_RIGHT_CLICK_SELECTIONS.
 */
#define EBROWSE_OPEN 1
#define EBROWSE_GRAPH 2
#define EBROWSE_ASSEMBLY 4

/* Reason for parent callback, see eObject::docallback() and eObject::oncallback() functions.
 */
typedef enum {
    ECALLBACK_VARIABLE_VALUE_CHANGED,
    ECALLBACK_SERVER_BINDING_CONNECTED,
    ECALLBACK_SERVER_BINDING_DISCONNECTED,
    ECALLBACK_TABLE_CONTENT_CHANGED,
    ECALLBACK_PERSISTENT_CHANGED,
    ECALLBACK_STATUS_CHANGED
}
eCallbackEvent;

/* Short marking for allocating temporary child object.
 */
#define ETEMPORARY this, EOID_TEMPORARY, EOBJ_TEMPORARY_ATTACHMENT

/* Name space identifiers as static strings. eobj_this_ns is default
   for ns_first and ns_firstv functions()
 */
extern const os_char eobj_process_ns[];
extern const os_char eobj_thread_ns[];
extern const os_char eobj_parent_ns[];
extern const os_char eobj_this_ns[];
extern const os_char eobj_no_ns[];


/**
****************************************************************************************************
  The eObject is base class for all the library objects.
****************************************************************************************************
*/
class eObject
{
    friend class eHandle;
    friend class eRoot;
    friend class ePointer;

    /**
    ************************************************************************************************
      Constructors and destructor
    ************************************************************************************************
    */

protected:
    /* Private constructor for new eObject.
     */
    eObject(
        eObject *parent = OS_NULL,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT);

private:
    /* Disable C++ standard copy constructor and assignment operator. Assignment is implemnted
        class specifically, and copy as clone() function which also positions object in object
        tree.
     */
    eObject(eObject const&);
    eObject& operator=(eObject const&);

    /* Create root helper object and handles for root and root helper.
     */
    void makeroot(
        e_oid id,
        os_int flags);

public:
    /* Delete eObject, virtual destructor.
     */
    virtual ~eObject();

    /* Cloning object.
     */
    virtual eObject *clone(
        eObject *parent,
        e_oid id = EOID_CHILD,
        os_int aflags = EOBJ_DEFAULT);

    /* Adopting object as child of this object.
     */
    void adopt(
        eObject *parent,
        e_oid id = EOID_CHILD,
        os_int aflags = EOBJ_DEFAULT);

    /* Adopting object as child of this object.
     */
    inline void adoptat(
        eObject *beforethis,
        e_oid id = EOID_CHILD) { adopt(beforethis, id, EOBJ_BEFORE_THIS); }

    /* Helper function for clone functionality.
     */
    void clonegeneric(
        eObject *clonedobj,
        os_int aflags);

    /* Get class identifier and name.
     */
    virtual os_int classid() {return ECLASSID_OBJECT; }
    const os_char *classname();

    /* Check if object is instance of class, or instance of a derived class.
     */
    os_boolean isinstanceof(
        os_int cid);

    /* Allocate new child object by class identifier.
     */
    inline eObject *newchild(
        os_int cid,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT)
    {
        return newobject(this, cid, id, flags);
    }

    /* Allocate new object by class identifier.
     */
    static eObject *newobject(
        eObject *parent,
        os_int cid,
        e_oid id = EOID_ITEM,
        os_int flags = EOBJ_DEFAULT);

    inline eHandle *handle()
    {
        return mm_handle;
    }

    /* Enable or disable timer for this object.
     */
    void timer(
        os_long period_ms);

    virtual os_int compare(
        eObject *x,
        os_int flags = 0)
    {
        OSAL_UNUSED(x);
        OSAL_UNUSED(flags);
        return -1;
    }

#if EOVERLOAD_NEW_AND_DELETE
    /**
    ************************************************************************************************

      @name Memory allocation

      Memory for objects is allocated by overloaded new and delete operators. These map the
      memory allocation to OSAL memory management.

    ************************************************************************************************
    */

    /* Overloaded new operator calls os_malloc().
     */
    void* operator new(
        size_t);

    /* Overloaded delete operator calls os_free().
     */
    void operator delete(
        void *buf);

#endif

    /**
    ************************************************************************************************
      Object flags, etc
    ************************************************************************************************
    */

    /** Get object flags.
     */
    inline os_int flags()
    {
        if (mm_handle) return mm_handle->m_oflags;
        return EOBJ_DEFAULT;
    }

    /** Set specified object flags.
     */
    inline void setflags(
        os_int flags)
    {
        if (mm_handle) mm_handle->setflags(flags);
    }

    /** Clear specified object flags.
     */
    inline void clearflags(
        os_int flags)
    {
        if (mm_handle) mm_handle->clearflags(flags);
    }

    /** If object can be cloned?
     */
    inline os_boolean isclonable()
    {
        if (mm_handle) return mm_handle->isclonable();
        return OS_TRUE;
    }

    /** Check if object is an attachment. Returns nonzero if object is an attachment.
     */
    inline os_boolean isattachment()
    {
        if (mm_handle) return mm_handle->isattachment();
        return OS_FALSE;
    }

    /** Check if object is a serializable attachment.
     */
    inline os_boolean isserattachment()
    {
        if (mm_handle) return mm_handle->isserattachment();
        return OS_FALSE;
    }

    /** Compare base class funcion returns -1 except if comparing to object itseld.
        The function allows us to call compare on eObject base class.
     */
    virtual os_int compare(
        eVariable *x,
        os_int flags = 0)
    {
        return ((eObject*)x == this) ? 0 : -1;
    }

    /**
    ************************************************************************************************
      Object hierarcy
    ************************************************************************************************
    */
    /** Get object identifier.
     */
    inline os_int oid()
    {
        if (mm_handle) return mm_handle->oid();
        return EOID_ITEM;
    }

    /* Recommended size for oixstr() buffer.
     */
    #define E_OIXSTR_BUF_SZ (2 * OSAL_NBUF_SZ+2)

    /** Convert oix and ucnt to string.
     */
    void oixstr(
        os_char *buf,
        os_memsz bufsz);

    /** Get oix and ucnt from string.
     */
    os_short oixparse(
        os_char *str,
        e_oix *oix,
        os_int *ucnt);

    /** Get number of childern.
     */
    inline os_int childcount(
        e_oid id = EOID_CHILD)
    {
        if (mm_handle) return mm_handle->childcount(id);
        return 0;
    }

    /** Get parent object of tis object.
     */
    inline eObject *parent()
    {
        return mm_parent;
    }

    /** Get grandparent of this object.
     */
    inline eObject *grandparent()
    {
        if (mm_parent)
        {
            return mm_parent->mm_parent;
        }
        return OS_NULL;
    }

    /** Get parent object of specific class and/or
        with specific object identifier.
     */
    eObject *parent(
        os_int cid,
        e_oid id = EOID_ALL,
        bool check_this = false);

    /** Check if this object is decendent of specific ancestor.
     */
    os_boolean isdecendentof(
        eObject *ancestor);

    /** Get thread object.
     */
    eThread *thread();

    /* Get the first child object identified by oid.
     */
    eObject *first(
        e_oid id = EOID_CHILD,
        os_boolean exact_match = OS_TRUE);

    /* Get the first child eVariable identified by oid.
     */
    eVariable *firstv(
        e_oid id = EOID_CHILD);

    /* Get the first child eMatrix identified by oid.
     */
    eMatrix *firstm(
        e_oid id = EOID_CHILD);

    /* Get the first child eSet identified by oid.
     */
    eSet *firsts(
        e_oid id = EOID_CHILD);

    /* Get the first child eContainer identified by oid.
     */
    eContainer *firstc(
        e_oid id = EOID_CHILD);

    /* Get the first child name identified by oid.
     */
    eName *firstn(
        e_oid id = EOID_NAME);

    /* Get last child object identified by oid.
     */
    eObject *last(
        e_oid id = EOID_CHILD);

    /* Get next object identified by oid.
     */
    eObject *next(
        e_oid id = EOID_CHILD);

    /* Get previous object identified by oid.
     */
    eObject *prev(
        e_oid id = EOID_CHILD);

    /* Verify whole object tree.
     */
#if EOBJECT_DBTREE_DEBUG
    inline void verify_whole_tree() {if (mm_handle) mm_handle->verify_whole_tree();}
#endif


    /**
    ************************************************************************************************
      Serialization

      Writing object to stream or reading it from stream. Application should use read() and write
      functions to read and write objects with additional information. writer() and reader()
      are class specific implementations for class content only.
    ************************************************************************************************
    */

    /* Write object to stream.
     */
    eStatus write(
        eStream *stream,
        os_int flags);

    /* Read object from stream as new child object.
     */
    eObject *read(
        eStream *stream,
        os_int sflags);

    /* Called by write() to write class specific object content.
       The eObject base class provides just dummy place holder
       function for classes which do not need serialization.
     */
    virtual eStatus writer(
        eStream *stream,
        os_int sflags)
    {
        osal_debug_error("serialization failed, writer not implemented");
        return ESTATUS_FAILED;
    }

    /* Called by read() to read class specific object content.
        The eObject base class provides just dummy place holder
        function for classes which do not need serialization.
     */
    virtual eStatus reader(
        eStream *stream,
        os_int sflags)
    {
        osal_debug_error("serialization failed, reader not implemented");
        return ESTATUS_FAILED;
    }

    /* Save this object to a file
     */
    eStatus save(
        const os_char *path);

    /* Load object from a file
     */
    eObject *load(
        const os_char *path);


#if E_SUPPROT_JSON
    /**
    ************************************************************************************************

      @name JSON serialization.

      This includes reading and writing object as from/to stream as JSON. Object can be desccribed
      as valus of it's properties and it's children.

    ************************************************************************************************
    */

    /* Write object to stream as JSON.
     */
    eStatus json_write(
        eStream *stream,
        os_int sflags = EOBJ_JSON_LIST_NAMESPACE,
        os_int indent = -1,
        os_boolean *comma = OS_NULL);

    /* Read object from JSON stream.
     */
    eObject *json_read(
        eStream *stream,
        os_int sflags = EOBJ_SERIALIZE_DEFAULT);

    /* Print object as JSON to console.
     */
    void print_json(os_int sflags = EOBJ_JSON_LIST_NAMESPACE);

    /* Called to check if object has class specific content. If there is no class
       specific JSON content, json_writer or json_reader should not be called.
     */
    virtual os_boolean has_json_content()
    {
        return OS_FALSE;
    }

    /* Class specific part of JSON writer.
     */
    virtual eStatus json_writer(
        eStream *stream,
        os_int sflags = EOBJ_SERIALIZE_DEFAULT,
        os_int indent = 0);
#endif

    /**
    ************************************************************************************************

      @name Names and namespaces.

      The name and name space functionality is based on eNameSpace and eName classes.

    ************************************************************************************************
    */

    /* Create name space for this object.
     */
    void ns_create(
        const os_char *namespace_id = OS_NULL);

    /* Delete this object's name space.
     */
    void ns_delete();

    /* Find eName by name and name space.
     */
    eName *ns_first(
        const os_char *name = OS_NULL,
        const os_char *namespace_id = eobj_this_ns);

    /* Find eName by name and name space.
     */
    eName *ns_firstv(
        eVariable *name = OS_NULL,
        const os_char *namespace_id = eobj_this_ns);

    /* Find object by name.
     */
    eObject *ns_get(
        const os_char *name,
        const os_char *namespace_id = eobj_this_ns,
        os_int cid = ECLASSID_OBJECT);

    eVariable *ns_getv(
        const os_char *name,
        const os_char *namespace_id = eobj_this_ns);

    eContainer *ns_getc(
        const os_char *name,
        const os_char *namespace_id = eobj_this_ns);

    /* Info bits for findnamespace().
     */
    #define E_INFO_PROCES_NS 1
    #define E_INFO_ABOVE_CHECKPOINT 2

    /* Find name space by name space ID.
     */
    eNameSpace *findnamespace(
        const os_char *namespace_id = OS_NULL,
        os_int *info = OS_NULL,
        eObject *checkpoint = OS_NULL);

    /* Give name to this object.
     */
    eName *addname(
        const os_char *name,
        os_int flags = 0, /* 0 = ENAME_PARENT_NS */
        const os_char *namespace_id = OS_NULL);

    /* Get first name given to this this object.
     */
    eName *primaryname(
        os_int nstype = -1);

    /* Add integer value as name to parent namespace.
     */
    eName *addintname(
        os_long x,
        os_int flags = 0);

    /* Flags for map() function: Attach all names of child object (this) and it's childen to
       name spaces. If a name is already mapped, it is not remapped.
    */
    #define E_ATTACH_NAMES 1

    /* Flags for map() function: Copy m_root pointer (pointer to eRoot of a tree structure)
       from child object (this) to all child objects of it.
    */
    #define E_SET_ROOT_POINTER 8

    /* Flags for map() function: Detach names of child object (this) and it's childen from name
       spaces above this object in tree structure.
      */
    #define E_DETACH_FROM_NAMESPACES_ABOVE 16

    /* Attach/detach names in tree sturcture to name spaces. Set eRoot pointers.
     */
    void map(os_int mflags);

    /* Get object by name.
     */
    eObject *byname(
        const os_char *name,
        os_boolean name_match = OS_TRUE);

    /* Get object by name.
     */
    eObject *byintname(
        os_long x,
        os_boolean name_match = OS_TRUE);


    /**
    ************************************************************************************************

      @name Messages

      Any object may send a message to an another object by calling message() function.
      When an object receives a message, it's onmessage function gets called.

    ************************************************************************************************
    */

    /* Send message.
     */
    void message(
        eEnvelope *envelope);

    /* Send message.
     */
    void message(
        os_int command,
        const os_char *target,
        const os_char *source = OS_NULL,
        eObject *content = OS_NULL,
        os_int mflags = EMSG_DEFAULT,
        eObject *context = OS_NULL);

    /* Reply with ECMD_NO_TARGET to indicate that target object was not found.
     */
    void notarget(
        eEnvelope *envelope,
        const os_char *msg = OS_NULL);

    /* Reply to message, often with ECMD_NO_TARGET, ECMD_OK or ECMD_ERROR command.
     */
    void reply(
        os_int command,
        eEnvelope *envelope,
        const os_char *msg = OS_NULL);

    /* Process an incoming message.
     */
    virtual void onmessage(
        eEnvelope *envelope);

    /* Set property by sending a message.
     */
    void setpropertyv_msg(
        const os_char *remotepath,
        eVariable *x,
        const os_char *propertyname = OS_NULL,
        os_int mflags = EMSG_KEEP_CONTENT|EMSG_NO_REPLIES);

    void setpropertyo_msg(
        const os_char *remotepath,
        eObject *x,
        const os_char *propertyname = OS_NULL,
        os_int mflags = EMSG_KEEP_CONTENT|EMSG_NO_REPLIES);

    void setpropertyl_msg(
        const os_char *remotepath,
        os_long x,
        const os_char *propertyname = OS_NULL);

    void setpropertyd_msg(
        const os_char *remotepath,
        os_double x,
        const os_char *propertyname = OS_NULL);

    void setpropertys_msg(
        const os_char *remotepath,
        const os_char *x,
        const os_char *propertyname = OS_NULL);


    /**
    ************************************************************************************************

      @name Object properties

      Any object may send a message to an another object by calling message() function.
      When an object receives a message, it's onmessage function gets called.

    ************************************************************************************************
    */

    /* Add property to property set (any type).
     */
    static eVariable *addproperty(
        os_int cid,
        os_int propertynr,
        const os_char *propertyname,
        const os_char *text = osal_str_empty,
        os_int pflags = EPRO_DEFAULT);

    /* Add integer property to property set.
     */
    static eVariable *addpropertyb(
        os_int cid,
        os_int propertynr,
        const os_char *propertyname,
        const os_char *text,
        os_int pflags = EPRO_DEFAULT);

    /* Add integer property to property set (initial/default value).
     */
    static eVariable *addpropertyb(
        os_int cid,
        os_int propertynr,
        const os_char *propertyname,
        os_boolean x,
        const os_char *text,
        os_int pflags = EPRO_DEFAULT);

    /* Add integer property to property set.
     */
    static eVariable *addpropertyl(
        os_int cid,
        os_int propertynr,
        const os_char *propertyname,
        const os_char *text,
        os_int pflags = EPRO_DEFAULT);

    /* Add integer property to property set (initial/default value).
     */
    static eVariable *addpropertyl(
        os_int cid,
        os_int propertynr,
        const os_char *propertyname,
        os_long x,
        const os_char *text,
        os_int pflags = EPRO_DEFAULT);

    /* Add double property to property set.
     */
    static eVariable *addpropertyd(
        os_int cid,
        os_int propertynr,
        const os_char *propertyname,
        const os_char *text,
        os_int digs,
        os_int pflags = EPRO_DEFAULT);

    /* Add double property to property set.
     */
    static eVariable *addpropertyd(
        os_int cid,
        os_int propertynr,
        const os_char *propertyname,
        os_double x,
        const os_char *text,
        os_int digs,
        os_int pflags = EPRO_DEFAULT);

    /* Add string property to property set.
     */
    static eVariable *addpropertys(
        os_int cid,
        os_int propertynr,
        const os_char *propertyname,
        const os_char *text,
        os_int pflags = EPRO_DEFAULT);

    /* Add string property to property set.
     */
    static eVariable *addpropertys(
        os_int cid,
        os_int propertynr,
        const os_char *propertyname,
        const os_char *x,
        const os_char *text,
        os_int pflags = EPRO_DEFAULT);

    /* Property set for class done, complete it.
     */
    static void propertysetdone(
        os_int cid);

    /* Get pointer to class'es property set.
     */
    eContainer *propertyset(
        os_int flags = EPRO_DEFAULT);

    /* Get pointer to class'es first static property.
     */
    eVariable *firstp(
        e_oid id = EOID_CHILD,
        os_int flags = EPRO_DEFAULT);

    /* Initialize properties to default values.
     */
    void initproperties();

    /* Property name to number.
     */
    os_int propertynr(
        const os_char *propertyname);

    /* Property number to name.
     */
    os_char *propertyname(
        os_int propertynr);

    /* Set property value from variable.
     */
    void setpropertyv(
        os_int propertynr,
        eVariable *x,
        eObject *source = OS_NULL,
        os_int flags = 0);

    /* Set object as property value.
     */
    void setpropertyo(
        os_int propertynr,
        eObject *x,
        os_int mflags = EMSG_KEEP_CONTENT);

    /* Set property value as integer.
     */
    void setpropertyl(
        os_int propertynr,
        os_long x);

    inline void setpropertyi(
        os_int propertynr,
        os_int x) {setpropertyl(propertynr, x);}

    /* Set property value as double.
     */
    void setpropertyd(
        os_int propertynr,
        os_double x);

    /* Set property value as string.
     */
    void setpropertys(
        os_int propertynr,
        const os_char *x);

    /* Forward simple property change to bindings.
     */
    void propertychanged(
        os_int propertynr);

    /* Forward property change trough bindings.
     */
    void forwardproperty(
        os_int propertynr,
        eVariable *x,
        eObject *source,
        os_int flags);

    /* Get property value.
     */
    eStatus propertyv(
        os_int propertynr,
        eVariable *x,
        os_int flags = 0);

    os_long propertyl(
        os_int propertynr);

    inline os_int propertyb(
        os_int propertynr) {return (os_boolean)propertyl(propertynr);}

    inline os_int propertyi(
        os_int propertynr) {return (os_int)propertyl(propertynr);}

    os_double propertyd(
        os_int propertynr);

    /* Called when property value changes.
     */
    virtual eStatus onpropertychange(
        os_int propertynr,
        eVariable *x,
        os_int flags) {return ESTATUS_UNKNOWN_PROPERTY_NR; }

    /* Get value of simple property.
     */
    virtual eStatus simpleproperty(
        os_int propertynr,
        eVariable *x);


    /**
    ************************************************************************************************

      @name Binding

      Any object may send a message to an another object by calling message() function.
      When an object receives a message, it's onmessage function gets called.

    ************************************************************************************************
    */
    /* Bind properties.
     */
    void bind(
        os_int localpropertynr,
        const os_char *remotepath,
        const os_char *remoteproperty,
        os_int bflags = 0);

    /* Bind properties, remote property .
     */
    void bind(
        os_int localpropertynr,
        const os_char *remotepath,
        os_int bflags = 0);

    /* Create bindings container for the object.
     */
    eContainer *bindings_container();

    /* Get object's the first property binding.
     */
    ePropertyBinding *firstpb(
        e_oid id = EOID_CHILD);

    /* Get object's first row set binding.
     */
    eRowSetBinding *firstrb(
        e_oid id);

    /* Check if this object has server side bindings.
     */
    os_boolean is_bound();


    /**
    ************************************************************************************************

      @name Child callbacks

      Callbacks from child objects to parent object.

    ************************************************************************************************
    */

    /* Process a callback from a child object.
     */
    virtual eStatus oncallback(
        eCallbackEvent event,
        eObject *obj,
        eObject *appendix);

    /* Call parent object's docallback function.
     */
    void docallback(
        eCallbackEvent event,
        eObject *appendix = OS_NULL);

    /* Check if parent callback flag is set for this object. Returns 0 if not, nonzero if
     * callback is set.
     */
    inline os_int hascallback()
        {return flags() & (EOBJ_PERSISTENT_CALLBACK|EOBJ_TEMPORARY_CALLBACK); }

protected:

    /**
    ************************************************************************************************
      Protected functions
    ************************************************************************************************
    */

    void message_within_thread(
        eEnvelope *envelope,
        const os_char *namespace_id);

    void message_process_ns(
        eEnvelope *envelope);

    /* Information for opening object has been requested, send it.
     */
    virtual void send_open_info(
        eEnvelope *envelope);

    /* Object information request by tree browser node, etc. Reply to it.
     */
    void send_browse_info(
        eEnvelope *envelope);

    /* Collect information about this object for tree browser.
     */
    virtual void object_info(
        eVariable *item,
        eVariable *name,
        eSet *appendix,
        const os_char *target);

    /* List names in this object's namespace. Used for browsing.
     */
    virtual void browse_list_namespace(
        eContainer *content,
        const os_char *target);

    /* List child objects. Used for browsing.
     */
    void browse_list_children(
        eContainer *content,
        os_int browse_flags);

    /* List object properties. Used for browsing.
     */
    void browse_list_properties(
        eContainer *content);

    void message_oix(
        eEnvelope *envelope);

    /* Forward message by object index within thread's object tree.
     */
    void onmessage_oix(
        eEnvelope *envelope);

    /* Create server side of property binding.
     */
    void srvbind(
        eEnvelope *envelope);

protected:
    /* Delete all child objects.
     */
    // void delete_children();

    void map2(
        eHandle *handle,
        os_int mflags);

    void mapone(
        eHandle *handle,
        os_int mflags);

#if E_SUPPROT_JSON
    /* Write optional comma and new line to terminate the
       line and optional intendation for next line to JSON output.
     */
    eStatus json_indent(
        eStream *stream,
        os_int indent,
        os_int iflags = EJSON_NEW_LINE_BEFORE,
        os_boolean *comma = OS_NULL);

    /* Write string to JSON output.
     */
    eStatus json_puts(
        eStream *stream,
        const os_char *str);

    /* Write quoted string to JSON output.
     */
    eStatus json_putqs(
        eStream *stream,
        const os_char *str);

    /* Write long integer to JSON output.
     */
    eStatus json_putl(
        eStream *stream,
        os_long x);

    /* Write variable value to JSON output.
     */
    eStatus json_putv(
        eStream *stream,
        eVariable *p,
        eVariable *value,
        os_int sflags,
        os_int indent);

    /* Append list item string to variable if bit is set in flags.
     */
    void json_append_list_item(
        eVariable *list,
        const os_char *item,
        os_int flags,
        os_int bit);
#endif

    /* Pointer to object's handle.
     */
    eHandle *mm_handle;

    /** Pointer to parent object of this object. (THIS SHOULD PERHAPS MOVE TO OBJ)
     */
    eObject *mm_parent;

};

#endif



