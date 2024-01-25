
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
 *	@file	linklist.h
 *	@ingroup common_base_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	generic module for efficient memory management of linked list elements of various sizes
 *
 *  a separate list for each size.  Elements must be a multiple of a pointer size.
 */

#ifndef __BASELIB_LINKLIST_H__
#define __BASELIB_LINKLIST_H__

#include "base/hci_type.h"
#include "base/base_lib.h"

#ifdef __cplusplus
extern "C" {
#endif

/** 
 *  Allocate a link-list element of given size and return pointer to it 
 */
HCILAB_PUBLIC HCI_BASE_API void *
__listelem_alloc__ (hci_int32 elemsize,
					char *file,
					hci_int32 line
);

/** 
 *  Macro of __listelem_alloc__
 */
#define hci_listelem_alloc(sz)	__listelem_alloc__((sz),__FILE__,__LINE__)

/**
 *	Free link-list element of given size 
 */
HCILAB_PUBLIC HCI_BASE_API void
hci_listelem_free (void *elem,
				   hci_int32 elemsize
);

/**
 *	Print number of allocation, numer of free operation stats 
 */
HCILAB_PUBLIC HCI_BASE_API void
hci_linklist_stats ( void );


#ifdef __cplusplus
}
#endif

#endif	// #ifndef __BASELIB_LINKLIST_H__
