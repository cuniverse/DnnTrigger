
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
 *	@file	linklist.c
 *	@ingroup common_base_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	generic module for efficient memory management of linked list elements of various sizes
 *
 *  a separate list for each size.  Elements must be a multiple of a pointer size.
 */

#include <stdio.h>
#include <stdlib.h>

#include "base/hci_msg.h"
#include "base/hci_malloc.h"
#include "base/linklist.h"


/**
 * @struct linked list
 *
 * A separate linked list for each element-size.  Element-size must be a multiple
 * of pointer-size.
 */
typedef struct list_s {
    char **freelist;            ///< ptr to first element in freelist
    struct list_s *next;        ///< Next linked list
    hci_int32 elemsize;         ///< #(char *) in element
    hci_int32 blocksize;        ///< #elements to alloc if run out of free elments
    hci_int32 blk_alloc;        ///< #Alloc operations before increasing blocksize
    hci_int32 n_alloc;
    hci_int32 n_freed;
} list_t;
static list_t *head = NULL;

#define MIN_ALLOC	50      ///< Min #elements to allocate in one block


/** 
 *  Allocate a link-list element of given size and return pointer to it 
 */
HCILAB_PUBLIC HCI_BASE_API void *
__listelem_alloc__(hci_int32 elemsize,
				   char *caller_file,
				   hci_int32 caller_line)
{
    hci_int32 j = 0;
	char **cpp = 0;
	char *cp = 0;
	list_t *prev = 0;
	list_t *curlist = 0;

    /* Find list for elemsize, if existing */
    prev = NULL;
    for (curlist = head; curlist && (curlist->elemsize != elemsize); curlist = curlist->next) {
        prev = curlist;
	}

    if (!curlist) {
        /* New list element size encountered, create new list entry */
        if ((elemsize % sizeof(void *)) != 0) {
            HCIMSG_ERROR
                ("List item size (%d) not multiple of sizeof(void *)\n",
                 elemsize);
			return 0;
		}

        curlist = (list_t *) hci_calloc(1, sizeof(list_t));
        curlist->freelist = NULL;
        curlist->elemsize = elemsize;
        curlist->blocksize = MIN_ALLOC;
        curlist->blk_alloc = (1 << 18) / (curlist->blocksize * sizeof(elemsize));
        curlist->n_alloc = 0;
        curlist->n_freed = 0;

        /* Link entry at head of list */
        curlist->next = head;
        head = curlist;
    }
    else if (prev) {
        /* List found; move entry to head of list */
        prev->next = curlist->next;
        curlist->next = head;
        head = curlist;
    }

    /* Allocate a new block if list empty */
    if (curlist->freelist == NULL) {
        /* Check if block size should be increased (if many requests for this size) */
        if (curlist->blk_alloc == 0) {
            curlist->blocksize <<= 1;
            curlist->blk_alloc =
                (1 << 18) / (curlist->blocksize * sizeof(elemsize));
            if (curlist->blk_alloc <= 0)
                curlist->blk_alloc = (hci_int32) 0x70000000;   /* Limit blocksize to new value */
        }

        /* Allocate block */
        cpp = curlist->freelist =
            (char **) __hci_calloc__((size_t)curlist->blocksize, (size_t)elemsize,
                                     caller_file, caller_line);
        cp = (char *) cpp;
        for (j = curlist->blocksize - 1; j > 0; --j) {
            cp += elemsize;
            *cpp = cp;
            cpp = (char **) cp;
        }
        *cpp = NULL;
        --(curlist->blk_alloc);
    }

    /* Unlink and return first element in freelist */
    cp = (char *) (curlist->freelist);
    curlist->freelist = (char **) (*(curlist->freelist));
    (curlist->n_alloc)++;

    return (cp);
}


/**
 *	Free link-list element of given size 
 */
HCILAB_PUBLIC HCI_BASE_API void
hci_listelem_free(void *elem,
				  hci_int32 elemsize)
{
    char **cpp = 0;
    list_t *prev = 0;
	list_t *curlist = 0;

    /* Find list for elemsize */
    prev = NULL;
    for (curlist = head; curlist && (curlist->elemsize != elemsize); curlist = curlist->next) {
        prev = curlist;
	}

    if (!curlist) {
        HCIMSG_ERROR("Unknown list item size: %d\n", elemsize);
		return;
    }
    else if (prev) {
        /* List found; move entry to head of list */
        prev->next = curlist->next;
        curlist->next = head;
        head = curlist;
    }

    /*
     * Insert freed item at head of list.
     * NOTE: skipping check for size being multiple of (void *).
     */
    cpp = (char **) elem;
    *cpp = (char *) curlist->freelist;
    curlist->freelist = cpp;
    (curlist->n_freed)++;
}


/**
 *	Print number of allocation, numer of free operation stats 
 */
HCILAB_PUBLIC HCI_BASE_API void
hci_linklist_stats(void)
{
    list_t *curlist = 0;
    char **cpp = 0;
    hci_int32 n = 0;

    HCIMSG_INFO("Linklist stats:\n");
    for (curlist = head; curlist; curlist = curlist->next) {
        for (n = 0, cpp = curlist->freelist; cpp;
             cpp = (char **) (*cpp), n++);
        printf
            ("\telemsize %d, #alloc %d, #freed %d, #freelist %d\n",
             curlist->elemsize, curlist->n_alloc, curlist->n_freed, n);
    }
}


// end of file