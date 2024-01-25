
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
 *	@file	glist.h
 *	@ingroup common_base_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	Header file to define API functions for maintaining a generic, linear linked-list structure.
 *
 * Only insert at the head of the list.  A convenient little
 * linked-list package, but a double-edged sword: the user must keep
 * track of the data type within the linked list elements.  When it
 * was first written, there was no selective deletions except to
 * destroy the entire list.  This is modified in later version. 
 * 
 * (C++ would be good for this, but that's a double-edged sword as well.)
 */


#ifndef __BASELIB_GLIST_H__
#define __BASELIB_GLIST_H__

#include <stdlib.h>
#include "base/hci_type.h"
#include "base/base_lib.h"

#ifdef __cplusplus
extern "C" {
#endif

/** A node in a generic list */
typedef struct gnode_s {
	anytype_t data;			///< See hci_type.h
	struct gnode_s *next;	///< Next node in list
} gnode_t;
typedef gnode_t *glist_t;	///< Head of a list of gnodes


/** Access macros, for convenience 
 */
#define gnode_ptr(g)		((g)->data.ptr)
#define gnode_int32(g)		((g)->data.i_32)
#define gnode_uint32(g)		((g)->data.ui_32)
#define gnode_float32(g)	((g)->data.fl_32)
#define gnode_float64(g)	((g)->data.fl_64)
#define gnode_next(g)		((g)->next)


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
PowerASR_Base_addGListPtr(glist_t g,  ///< a link list
						  void *ptr   ///< a pointer
);
  
HCILAB_PUBLIC HCI_BASE_API glist_t
PowerASR_Base_addGListInt32(glist_t g,		///< a link list
							hci_int32 val	///< an integer value
);

HCILAB_PUBLIC HCI_BASE_API glist_t
PowerASR_Base_addGListUInt32(glist_t g,		///< a link list
							 hci_uint32 val	///< an unsigned integer value
);

HCILAB_PUBLIC HCI_BASE_API glist_t
PowerASR_Base_addGListFloat32(glist_t g,		///< a link list
							  hci_float32 val	///< a float32 value
);

HCILAB_PUBLIC HCI_BASE_API glist_t
PowerASR_Base_addGListFloat64(glist_t g,		///< a link list
							  hci_float64 val	///< a float64 value
);



/**
 * Create and insert a new list node, with the given user-defined data, after
 * the given generic node gn.  gn cannot be NULL.

 * @return	return ptr to the newly created gnode_t.
 */

HCILAB_PUBLIC HCI_BASE_API gnode_t *
PowerASR_Base_insertGListPtr(gnode_t *gn,	///< a generic node which ptr will be inserted after it
							 void *ptr		///< pointer inserted
);

HCILAB_PUBLIC HCI_BASE_API gnode_t *
PowerASR_Base_insertGListInt32(gnode_t *gn,		///< a generic node which a value will be inserted after it
							   hci_int32 val	///< int32 inserted
);

HCILAB_PUBLIC HCI_BASE_API gnode_t *
PowerASR_Base_insertGListUInt32(gnode_t *gn,	///< a generic node which a value will be inserted after it
								hci_uint32 val	///< uint32 inserted
);

HCILAB_PUBLIC HCI_BASE_API gnode_t *
PowerASR_Base_insertGListFloat32(gnode_t *gn,		///< a generic node which a value will be inserted after it
								 hci_float32 val	///< float32 inserted
);

HCILAB_PUBLIC HCI_BASE_API gnode_t *
PowerASR_Base_insertGListFloat64(gnode_t *gn,		///< a generic node which a value will be inserted after it
								 hci_float64 val	///< float64 inserted
);

/**
 * Delete a list node, with the given user-defined data, after
 * the given generic node gn.  gn cannot be NULL.
 *
 * @return Return ptr to the newly created gnode_t.
 *
 * It is more a mirror image of glist_add_* family of functions.
 */

HCILAB_PUBLIC HCI_BASE_API gnode_t *
PowerASR_Base_deleteGList(gnode_t *gn	///< a generic node which ptr will be deleted after it.
);

/**
 * Check the given glist to see if it already contains the given value (of appropriate type).
 * In the case of the ptr, only the pointer values are compared, not the data pointed to by them.
 * 
 *	@return Return value: 1 if match found, 0 if not.
 */
HCILAB_PUBLIC HCI_BASE_API hci_int32
PowerASR_Base_checkDuplicateGListPtr(glist_t g,
									 void *val	///< List and value to check for
);

HCILAB_PUBLIC HCI_BASE_API hci_int32
PowerASR_Base_checkDuplicateGListInt32(glist_t g,
									   hci_int32 val	///< value to check for
);

HCILAB_PUBLIC HCI_BASE_API hci_int32
PowerASR_Base_checkDuplicateGListUInt32(glist_t g,
										hci_uint32 val	///< value to check for
);

HCILAB_PUBLIC HCI_BASE_API hci_int32
PowerASR_Base_checkDuplicateGListFloat32(glist_t g,
										 hci_float32 val	///< value to check for
);

HCILAB_PUBLIC HCI_BASE_API hci_int32
PowerASR_Base_checkDuplicateGListFloat64(glist_t g,
										 hci_float64 val	///< value to check for
);


/**
 * Reverse the order of the given glist.  (glist_add() adds to the head; one might
 * ultimately want the reverse of that.)
 *
 * @note NOTE: The list is reversed "in place"; i.e., no new memory is allocated.
 *
 * @return The head of the new list.
 */
HCILAB_PUBLIC HCI_BASE_API glist_t
PowerASR_Base_reverseGList(glist_t g	///< input link list
);


/**
 * Count the number of element in a given link list 
 *
 *	@return the number of elements in the given glist_t
 */
HCILAB_PUBLIC HCI_BASE_API hci_int32
PowerASR_Base_countGList(glist_t g	///< input link list
);


/**
 * Apply the given function to the user-defined data.ptr for each node in the list.
 * (Again, too bad there's no function overloading in C.)
 */
HCILAB_PUBLIC HCI_BASE_API void
PowerASR_Base_applyGListPtr(glist_t g, void (*func)(void *));
HCILAB_PUBLIC HCI_BASE_API void
PowerASR_Base_applyGListInt32(glist_t g, void (*func)(hci_int32));
HCILAB_PUBLIC HCI_BASE_API void
PowerASR_Base_applyGListUInt32(glist_t g, void (*func)(hci_uint32));
HCILAB_PUBLIC HCI_BASE_API void
PowerASR_Base_applyGListFloat32(glist_t g, void (*func)(hci_float32));
HCILAB_PUBLIC HCI_BASE_API void
PowerASR_Base_applyGListFloat64(glist_t g, void (*func)(hci_float64));


/**
 * Free the given generic list; user-defined data contained within is not
 * automatically freed.  The caller must have done that already.
 */
HCILAB_PUBLIC HCI_BASE_API void
PowerASR_Base_freeGList(glist_t g
);


/**
 * Free the user-defined data (i.e., g->data.ptr) contained at each node of the given
 * glist (using myfree()).  Then free the glist.  "datasize" is the size of the
 * user-defined data at each node, and is needed by myfree().
 */
HCILAB_PUBLIC HCI_BASE_API void
PowerASR_Base_myfreeGList(glist_t g,
						  hci_int32 datasize
);


/**
 * Free the given node, gn, of a glist, pred being its predecessor in the list.
 *
 * @return Return ptr to the next node in the list after the freed node.
 */
HCILAB_PUBLIC HCI_BASE_API gnode_t *
PowerASR_Base_freeGNode(gnode_t *gn,
						gnode_t *pred
);

/**
 * @return Return the last node in the given list.
 */
HCILAB_PUBLIC HCI_BASE_API gnode_t *
PowerASR_Base_getTailInGList(glist_t g
);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef __BASELIB_GLIST_H__
