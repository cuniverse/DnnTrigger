
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
 *	@brief	memory allocation package.
 *
 *   ; Implementation of efficient memory allocation deallocation for
 *     multiple dimensional arrays.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "base/hci_malloc.h"
#include "base/hci_msg.h"


/**
 * The following functions are similar to the malloc family, except that they have
 * two additional parameters, caller_file and caller_line, for error reporting.
 * All functions print a diagnostic message and exit if any error occurs.
 */

HCILAB_PUBLIC HCI_BASE_API void *
__hci_calloc__(size_t n_elem, 
			   size_t elem_size,
			   const char *caller_file,
			   hci_int32 caller_line)
{
    void *mem = 0;

    if ((mem = calloc(n_elem, elem_size)) == NULL) {
        HCIMSG_ERROR("calloc(%d,%d) failed from %s(%d)\n", n_elem,
                elem_size, caller_file, caller_line);
    }

    return mem;
}

HCILAB_PUBLIC HCI_BASE_API void *
__hci_malloc__(size_t size,
			   const char *caller_file,
			   hci_int32 caller_line)
{
    void *mem = 0;

    if ((mem = malloc(size)) == NULL) {
        HCIMSG_ERROR("malloc(%d) failed from %s(%d)\n", size,
                caller_file, caller_line);
	}

    return mem;
}

HCILAB_PUBLIC HCI_BASE_API void *
__hci_realloc__(void *ptr,
				size_t new_size,
				const char *caller_file,
				hci_int32 caller_line)
{
    void *mem = 0;

    if ((mem = realloc(ptr, new_size)) == NULL) {
        HCIMSG_ERROR("realloc(%d) failed from %s(%d)\n", new_size,
                caller_file, caller_line);
    }

    return mem;
}

/**
 * Like strdup, except that if an error occurs it prints a diagnostic message
 * and exits.
 */
HCILAB_PUBLIC HCI_BASE_API char *
__hci_salloc__(const char *origstr,
			   const char *caller_file,
			   hci_int32 caller_line)
{
    size_t len = 0;
    char *buf = 0;

	if ( !origstr ) return buf;

    len = strlen(origstr) + 1;
    buf = (char *) __hci_malloc__(len, caller_file, caller_line);

    strcpy(buf, origstr);

    return (buf);
}

/**
 * Allocate a 2-D array and return ptr to it (ie, ptr to vector of ptrs).
 * The data area is allocated in one block so it can also be treated as a 1-D array.
 */
HCILAB_PUBLIC HCI_BASE_API void **
__hci_calloc_2d__(size_t d1,
				  size_t d2,		///< In: #elements in the 2 dimensions
				  size_t elemsize,	///< In: Size (#bytes) of each element
				  const char *caller_file,
				  hci_int32 caller_line)	///< In
{
    char **ref = 0;
	char *mem = 0;
    size_t i = 0;
	size_t offset = 0;

    mem =
        (char *) __hci_calloc__(d1 * d2, elemsize, caller_file,
                                caller_line);
    ref =
        (char **) __hci_malloc__(d1 * sizeof(void *), caller_file,
                                 caller_line);

    for (i = 0, offset = 0; i < d1; i++, offset += d2 * elemsize)
        ref[i] = mem + offset;

    return ((void **) ref);
}

/**
 * Overlay a 2-D array over a previously allocated storage area.
 */
HCILAB_PUBLIC HCI_BASE_API void **
__hci_alloc_2d_ptr(size_t d1,
				   size_t d2,
				   void *store,
				   size_t elem_size,
				   const char *caller_file,
				   hci_int32 caller_line)
{
    void **out = 0;
	size_t i = 0;
	size_t j = 0;
    
    out = (void **)__hci_calloc__(d1, sizeof(void *), caller_file, caller_line);
    
    for (i = 0, j = 0; i < d1; i++, j += d2) {
		out[i] = &((char *)store)[j*elem_size];
    }

    return out;
}

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
				  hci_int32 caller_line)	///< In
{
    char ***ref1 = 0;
	char **ref2 = 0;
	char *mem = 0;
	size_t i = 0;
	size_t j = 0;
	size_t offset = 0;

    mem =
        (char *) __hci_calloc__(d1 * d2 * d3, elemsize, caller_file,
                                caller_line);
    ref1 =
        (char ***) __hci_malloc__(d1 * sizeof(void **), caller_file,
                                  caller_line);
    ref2 =
        (char **) __hci_malloc__(d1 * d2 * sizeof(void *), caller_file,
                                 caller_line);

    for (i = 0, offset = 0; i < d1; i++, offset += d2)
        ref1[i] = ref2 + offset;

    offset = 0;
    for (i = 0; i < d1; i++) {
        for (j = 0; j < d2; j++) {
            ref1[i][j] = mem + offset;
            offset += d3 * elemsize;
        }
    }

    return ((void ***) ref1);
}

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
				   hci_int32 caller_line)
{
    void **tmp1 = 0;
    void ***out = 0;
	size_t i = 0;
	size_t j = 0;
    
    tmp1 = __hci_calloc__(d1 * d2, sizeof(void *), caller_file, caller_line);

    out  = __hci_calloc__(d1, sizeof(void **), caller_file, caller_line);
    
    for (i = 0, j = 0; i < d1*d2; i++, j += d3) {
		tmp1[i] = &((char *)store)[j*elem_size];
    }
    
    for (i = 0, j = 0; i < d1; i++, j += d2) {
		out[i] = &tmp1[j];
    }
    
    return out;
}

/** Test and free a 1-D array 
 */
HCILAB_PUBLIC HCI_BASE_API void
hci_free(void *ptr)
{
    if (ptr) {
        free(ptr);
		ptr = 0;
	}
}

/**
   Free a 2-D array (ptr) previously allocated by hci_calloc_2d 
*/

HCILAB_PUBLIC HCI_BASE_API void
hci_free_2d(void **ptr)
{
    if (ptr) {
        hci_free(ptr[0]);
	hci_free(ptr);
		ptr = 0;
	}
}


/** 
    Free a 3-D array (ptr) previously allocated by hci_calloc_3d 
*/
HCILAB_PUBLIC HCI_BASE_API void
hci_free_3d(void ***ptr)
{
    if ( ptr ) {
		if (ptr[0])
        hci_free(ptr[0][0]);
        hci_free(ptr[0]);
    hci_free(ptr);
		ptr = 0;
	}
}


// end of file