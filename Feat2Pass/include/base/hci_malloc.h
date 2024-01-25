
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
 *	@file	hci_malloc.h
 *	@ingroup common_base_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	Header file to define API functions related to memory allocation package.
 *
 *   ; Implementation of efficient memory allocation deallocation for
 *     multiple dimensional arrays.
 */


#ifndef __HCILAB_MALLOC_H__
#define __HCILAB_MALLOC_H__

#include <stdlib.h>
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif
#include "base/hci_type.h"
#include "base/base_lib.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * The following functions are similar to the malloc family, except that they have
 * two additional parameters, caller_file and caller_line, for error reporting.
 * All functions print a diagnostic message and exit if any error occurs.
 */

HCILAB_PUBLIC HCI_BASE_API void *
__hci_calloc__(size_t n_elem, 
			   size_t elem_size,
			   const char *caller_file,
			   hci_int32 caller_line);

HCILAB_PUBLIC HCI_BASE_API void *
__hci_malloc__(size_t size,
			   const char *caller_file,
			   hci_int32 caller_line);

HCILAB_PUBLIC HCI_BASE_API void *
__hci_realloc__(void *ptr,
				size_t new_size,
				const char *caller_file,
				hci_int32 caller_line);

/**
 * Like strdup, except that if an error occurs it prints a diagnostic message
 * and exits.
 */
HCILAB_PUBLIC HCI_BASE_API char *
__hci_salloc__(const char *origstr,
			   const char *caller_file,
			   hci_int32 caller_line);

/**
 * Allocate a 2-D array and return ptr to it (ie, ptr to vector of ptrs).
 * The data area is allocated in one block so it can also be treated as a 1-D array.
 */
HCILAB_PUBLIC HCI_BASE_API void **
__hci_calloc_2d__(size_t d1,
				  size_t d2,		///< In: #elements in the 2 dimensions
				  size_t elemsize,	///< In: Size (#bytes) of each element
				  const char *caller_file,
				  hci_int32 caller_line);	///< In

/**
 * Allocate a 3-D array and return ptr to it.
 * The data area is allocated in one block so it can also be treated as a 1-D array.
 */
HCILAB_PUBLIC HCI_BASE_API void ***
__hci_calloc_3d__(size_t d1,
				  size_t d2,
				  size_t d3,	///< In: #elems in the dims
				  size_t elemsize,		///< In: Size (#bytes) per element
				  const char *caller_file,
				  hci_int32 caller_line);	///< In

/**
 * Overlay a 3-D array over a previously allocated storage area.
 */
HCILAB_PUBLIC HCI_BASE_API void ***
__hci_alloc_3d_ptr(size_t d1,
				   size_t d2,
				   size_t d3,
				   void *store,
				   size_t elem_size,
				   const char *caller_file,
				   hci_int32 caller_line);

/**
 * Overlay a 2-D array over a previously allocated storage area.
 */
HCILAB_PUBLIC HCI_BASE_API void **
__hci_alloc_2d_ptr(size_t d1,
				   size_t d2,
				   void *store,
				   size_t elem_size,
				   const char *caller_file,
				   hci_int32 caller_line);

/** Test and free a 1-D array 
 */
HCILAB_PUBLIC HCI_BASE_API void
hci_free(void *ptr);

/**
   Free a 2-D array (ptr) previously allocated by hci_calloc_2d 
*/

HCILAB_PUBLIC HCI_BASE_API void
hci_free_2d(void **ptr);


/** 
    Free a 3-D array (ptr) previously allocated by hci_calloc_3d 
*/
HCILAB_PUBLIC HCI_BASE_API void
hci_free_3d(void ***ptr);

/**
 * Macros to simplify the use of above functions.
 * One should use these, rather than target functions directly.
 */

/**
 * Macro for __hci_calloc__
 */
#define hci_calloc(n,sz)	__hci_calloc__((n),(sz),__FILE__,__LINE__)

/**
 * Macro for __hci_malloc__
 */
#define hci_malloc(sz)		__hci_malloc__((sz),__FILE__,__LINE__)

/**
 * Macro for __hci_realloc__
 */
#define hci_realloc(ptr,sz)	__hci_realloc__(ptr,(sz),__FILE__,__LINE__)

/**
 * Macro for __hci_salloc__
 */

#define hci_salloc(ptr)		__hci_salloc__(ptr,__FILE__,__LINE__)

/**
 * Macro for __hci_calloc_2d__
 */

#define hci_calloc_2d(d1,d2,sz)	__hci_calloc_2d__((d1),(d2),(sz),__FILE__,__LINE__)

/**
 * Macro for __hci_calloc_3d__
 */

#define hci_calloc_3d(d1,d2,d3,sz) __hci_calloc_3d__((d1),(d2),(d3),(sz),__FILE__,__LINE__)

/**
 * Macro for __hci_calloc_2d_ptr
 */

#define hci_alloc_2d_ptr(d1, d2, bf, sz)    __hci_alloc_2d_ptr((d1), (d2), (bf), (sz), __FILE__, __LINE__)

/**
 * Macro for __hci_calloc_3d_ptr
 */

#define hci_alloc_3d_ptr(d1, d2, d3, bf, sz) __hci_alloc_3d_ptr((d1), (d2), (d3), (bf), (sz), __FILE__, __LINE__)


#ifdef __cplusplus
}
#endif

#endif	// #ifndef __HCILAB_MALLOC_H__

