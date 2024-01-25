
/* ====================================================================
 * Copyright (c) 2007 HCI LAB. 
 * ALL RIGHTS RESERVED.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are prohibited provided that permissions by HCI LAB
 * are not given.
 *
 * ====================================================================
 *
 */

/**
 *	@file	glist.c
 *	@ingroup common_base_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	Modules for maintaining a generic, linear linked-list structure.
 *
 * Only insert at the head of the list.  A convenient little
 * linked-list package, but a double-edged sword: the user must keep
 * track of the data type within the linked list elements.  When it
 * was first written, there was no selective deletions except to
 * destroy the entire list.  This is modified in later version. 
 * 
 * (C++ would be good for this, but that's a double-edged sword as well.)
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32_WCE
#include <assert.h>
#endif	// #ifndef _WIN32_WCE

#include "base/glist.h"
#include "base/hci_malloc.h"


/**
 *	Create and insert a new list node, with the given user-defined data, at the HEAD
 *	of the given generic list.
 *
 *	@return Return the new list thus formed.
 *
 *	g may be NULL to indicate an initially empty list.
 *	(Too bad there's no function overloading.)
 */
HCILAB_PUBLIC HCI_BASE_API glist_t
PowerASR_Base_addGListPtr(glist_t g,	///< a link list
						  void *ptr )	///< a pointer
{
    gnode_t *gn = 0;

    gn = (gnode_t *) hci_calloc(1, sizeof(gnode_t));
    gn->data.ptr = ptr;
    gn->next = g;
    return ((glist_t) gn);      // Return the new head of the list
}
  
HCILAB_PUBLIC HCI_BASE_API glist_t
PowerASR_Base_addGListInt32(glist_t g,		///< a link list
							hci_int32 val )	///< an integer value
{
    gnode_t *gn = 0;

    gn = (gnode_t *) hci_calloc(1, sizeof(gnode_t));
    gn->data.i_32 = val;
    gn->next = g;
    return ((glist_t) gn);      // Return the new head of the list
}

HCILAB_PUBLIC HCI_BASE_API glist_t
PowerASR_Base_addGListUInt32(glist_t g,			///< a link list
							 hci_uint32 val )	///< an unsigned integer value
{
    gnode_t *gn = 0;

    gn = (gnode_t *) hci_calloc(1, sizeof(gnode_t));
    gn->data.ui_32 = val;
    gn->next = g;
    return ((glist_t) gn);      // Return the new head of the list
}

HCILAB_PUBLIC HCI_BASE_API glist_t
PowerASR_Base_addGListFloat32(glist_t g,		///< a link list
							  hci_float32 val )	///< a float32 value
{
    gnode_t *gn = 0;

    gn = (gnode_t *) hci_calloc(1, sizeof(gnode_t));
    gn->data.fl_32 = val;
    gn->next = g;
    return ((glist_t) gn);      // Return the new head of the list
}

HCILAB_PUBLIC HCI_BASE_API glist_t
PowerASR_Base_addGListFloat64(glist_t g,		///< a link list
							  hci_float64 val )	///< a float64 value
{
    gnode_t *gn = 0;

    gn = (gnode_t *) hci_calloc(1, sizeof(gnode_t));
    gn->data.fl_64 = val;
    gn->next = g;
    return ((glist_t) gn);      // Return the new head of the list
}


/**
 * Create and insert a new list node, with the given user-defined data, after
 * the given generic node gn.  gn cannot be NULL.

 * @return	return ptr to the newly created gnode_t.
 */

HCILAB_PUBLIC HCI_BASE_API gnode_t *
PowerASR_Base_insertGListPtr(gnode_t *gn,	///< a generic node which ptr will be inserted after it
							 void *ptr )	///< pointer inserted
{
    gnode_t *newgn = 0;

    newgn = (gnode_t *) hci_calloc(1, sizeof(gnode_t));
    newgn->data.ptr = ptr;
    newgn->next = gn->next;
    gn->next = newgn;

    return newgn;
}

HCILAB_PUBLIC HCI_BASE_API gnode_t *
PowerASR_Base_insertGListInt32(gnode_t *gn,		///< a generic node which a value will be inserted after it
							   hci_int32 val )	///< int32 inserted
{
    gnode_t *newgn = 0;

    newgn = (gnode_t *) hci_calloc(1, sizeof(gnode_t));
    newgn->data.i_32 = val;
    newgn->next = gn->next;
    gn->next = newgn;

    return newgn;
}

HCILAB_PUBLIC HCI_BASE_API gnode_t *
PowerASR_Base_insertGListUInt32(gnode_t *gn,		///< a generic node which a value will be inserted after it
								hci_uint32 val )	///< uint32 inserted
{
    gnode_t *newgn = 0;

    newgn = (gnode_t *) hci_calloc(1, sizeof(gnode_t));
    newgn->data.ui_32 = val;
    newgn->next = gn->next;

    gn->next = newgn;

    return newgn;
}

HCILAB_PUBLIC HCI_BASE_API gnode_t *
PowerASR_Base_insertGListFloat32(gnode_t *gn,		///< a generic node which a value will be inserted after it
								 hci_float32 val )	///< float32 inserted
{
    gnode_t *newgn = 0;

    newgn = (gnode_t *) hci_calloc(1, sizeof(gnode_t));
    newgn->data.fl_32 = val;
    newgn->next = gn->next;
    gn->next = newgn;

    return newgn;
}

HCILAB_PUBLIC HCI_BASE_API gnode_t *
PowerASR_Base_insertGListFloat64(gnode_t *gn,		///< a generic node which a value will be inserted after it
								 hci_float64 val )	///< float64 inserted
{
    gnode_t *newgn = 0;

    newgn = (gnode_t *) hci_calloc(1, sizeof(gnode_t));
    newgn->data.fl_64 = val;
    newgn->next = gn->next;
    gn->next = newgn;

    return newgn;
}


/**
 * Delete a list node, with the given user-defined data, after
 * the given generic node gn.  gn cannot be NULL.
 *
 * @return Return ptr to the newly created gnode_t.
 *
 * It is more a mirror image of glist_add_* family of functions.
 */

HCILAB_PUBLIC HCI_BASE_API gnode_t *
PowerASR_Base_deleteGList(gnode_t *gn )	///< a generic node which ptr will be deleted after it.
{
    gnode_t *newgn = 0;

    newgn = gn->next;
    if (newgn) {
        hci_free((char *) gn);
        return newgn;
    }
    else {
        return gn;
    }
}


/**
 * Check the given glist to see if it already contains the given value (of appropriate type).
 * In the case of the ptr, only the pointer values are compared, not the data pointed to by them.
 * 
 *	@return Return value: 1 if match found, 0 if not.
 */
HCILAB_PUBLIC HCI_BASE_API hci_int32
PowerASR_Base_checkDuplicateGListPtr(glist_t g,
									 void *val )	///< List and value to check for
{
    gnode_t *gn = 0;

    for (gn = g; gn; gn = gnode_next(gn)) {
        if (gnode_ptr(gn) == val) {
            return 1;
		}
	}

    return 0;
}

HCILAB_PUBLIC HCI_BASE_API hci_int32
PowerASR_Base_checkDuplicateGListInt32(glist_t g,
									   hci_int32 val )	///< value to check for
{
    gnode_t *gn = 0;

    for (gn = g; gn; gn = gnode_next(gn)) {
        if (gnode_int32(gn) == val) {
            return 1;
		}
	}

    return 0;
}

HCILAB_PUBLIC HCI_BASE_API hci_int32
PowerASR_Base_checkDuplicateGListUInt32(glist_t g,
										hci_uint32 val )	///< value to check for
{
    gnode_t *gn = 0;

    for (gn = g; gn; gn = gnode_next(gn)) {
        if (gnode_uint32(gn) == val) {
            return 1;
		}
	}

    return 0;
}

HCILAB_PUBLIC HCI_BASE_API hci_int32
PowerASR_Base_checkDuplicateGListFloat32(glist_t g,
										 hci_float32 val )	///< value to check for
{
    gnode_t *gn = 0;

    for (gn = g; gn; gn = gnode_next(gn)) {
        if (gnode_float32(gn) == val) {
            return 1;
		}
	}

    return 0;
}

HCILAB_PUBLIC HCI_BASE_API hci_int32
PowerASR_Base_checkDuplicateGListFloat64(glist_t g,
										 hci_float64 val )	///< value to check for
{
    gnode_t *gn = 0;

    for (gn = g; gn; gn = gnode_next(gn)) {
        if (gnode_float64(gn) == val) {
            return 1;
		}
	}

    return 0;
}


/**
 * Reverse the order of the given glist.  (glist_add() adds to the head; one might
 * ultimately want the reverse of that.)
 *
 * @note NOTE: The list is reversed "in place"; i.e., no new memory is allocated.
 *
 * @return The head of the new list.
 */
HCILAB_PUBLIC HCI_BASE_API glist_t
PowerASR_Base_reverseGList(glist_t g )	///< input link list
{
    gnode_t *gn = 0;
	gnode_t *nextgn = 0;
    gnode_t *rev = 0;

    rev = NULL;
    for (gn = g; gn; gn = nextgn) {
        nextgn = gn->next;

        gn->next = rev;
        rev = gn;
    }

    return rev;
}


/**
 * Count the number of element in a given link list 
 *
 *	@return the number of elements in the given glist_t
 */
HCILAB_PUBLIC HCI_BASE_API hci_int32
PowerASR_Base_countGList(glist_t g )	///< input link list
{
    gnode_t *gn = 0;
    hci_int32 n;

    for (gn = g, n = 0; gn; gn = gn->next, n++);
    return n;
}


/**
 * Apply the given function to the user-defined data.ptr for each node in the list.
 * (Again, too bad there's no function overloading in C.)
 */
HCILAB_PUBLIC HCI_BASE_API void
PowerASR_Base_applyGListPtr(glist_t g,
							void (*func)(void *))
{
    gnode_t *gn = 0;

    for (gn = g; gn; gn = gn->next)
        (*func) (gn->data.ptr);
}

HCILAB_PUBLIC HCI_BASE_API void
PowerASR_Base_applyGListInt32(glist_t g,
							  void (*func)(hci_int32))
{
    gnode_t *gn = 0;

    for (gn = g; gn; gn = gn->next)
        (*func) (gn->data.i_32);
}

HCILAB_PUBLIC HCI_BASE_API void
PowerASR_Base_applyGListUInt32(glist_t g,
							   void (*func)(hci_uint32))
{
    gnode_t *gn = 0;

    for (gn = g; gn; gn = gn->next)
        (*func) (gn->data.ui_32);
}

HCILAB_PUBLIC HCI_BASE_API void
PowerASR_Base_applyGListFloat32(glist_t g,
								void (*func)(hci_float32))
{
    gnode_t *gn = 0;

    for (gn = g; gn; gn = gn->next)
        (*func) (gn->data.fl_32);
}

HCILAB_PUBLIC HCI_BASE_API void
PowerASR_Base_applyGListFloat64(glist_t g,
								void (*func)(hci_float64))
{
    gnode_t *gn = 0;

    for (gn = g; gn; gn = gn->next)
        (*func) (gn->data.fl_64);
}


/**
 * Free the given generic list; user-defined data contained within is not
 * automatically freed.  The caller must have done that already.
 */
HCILAB_PUBLIC HCI_BASE_API void
PowerASR_Base_freeGList(glist_t g )
{
    gnode_t *gn = 0;

    while (g) {
        gn = g;
        g = gn->next;
        hci_free((void *) gn);
    }
}


/**
 * Free the user-defined data (i.e., g->data.ptr) contained at each node of the given
 * glist (using myfree()).  Then free the glist.  "datasize" is the size of the
 * user-defined data at each node, and is needed by myfree().
 */
HCILAB_PUBLIC HCI_BASE_API void
PowerASR_Base_myfreeGList(glist_t g,
						  hci_int32 datasize )
{
    gnode_t *gn = 0;

    while (g) {
        gn = g;
        g = gn->next;
        hci_free((char *) (gn->data.ptr));
        hci_free((char *) gn);
    }
}


/**
 * Free the given node, gn, of a glist, pred being its predecessor in the list.
 *
 * @return Return ptr to the next node in the list after the freed node.
 */
HCILAB_PUBLIC HCI_BASE_API gnode_t *
PowerASR_Base_freeGNode(gnode_t *gn,
						gnode_t *pred )
{
    gnode_t *next = 0;

    next = gn->next;
    if (pred) {
#ifndef _WIN32_WCE
        assert(pred->next == gn);
#endif	// #ifndef _WIN32_WCE

        pred->next = next;
    }

    hci_free((char *) gn);

    return next;
}

/**
 * @return Return the last node in the given list.
 */
HCILAB_PUBLIC HCI_BASE_API gnode_t *
PowerASR_Base_getTailInGList(glist_t g )
{
    gnode_t *gn = 0;

    if (!g)
        return NULL;

    for (gn = g; gn->next; gn = gn->next);
    return gn;
}


// end of file