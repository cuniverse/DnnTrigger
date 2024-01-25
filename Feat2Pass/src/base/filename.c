
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
 *	@file	filename.c
 *	@ingroup common_base_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	file and path name operations.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "base/filename.h"


/**
 * Strip off leading path components from the given path and copy the base into base.
 * Caller must have allocated base.
 */
HCILAB_PUBLIC HCI_BASE_API void
PowerASR_Base_stripBaseNameFromPath(char *path,		///< (i) path string
									char *base)		///< (o) base string
{
    hci_int32 i = 0;
	hci_int32 l = 0;

    l = (hci_int32)strlen(path);
    for (i = l - 1; (i >= 0) && (path[i] != '/') && (path[i] != '\\'); --i);
    strcpy(base, path + i + 1);
}


/**
 * Strip off the smallest trailing file-extension suffix and copy
 * the rest into the given root argument.  Caller must have
 * allocated root.
 */
HCILAB_PUBLIC HCI_BASE_API void
PowerASR_Base_stripFileExtension(char *path,		///< (i) path/file string
								 char *root)		///< (o) file-extension suffix가 제거된 string
{
    hci_int32 i = 0;
	hci_int32 l = 0;

    l = (hci_int32)strlen(path);
    for (i = l - 1; (i >= 0) && (path[i] != '.'); --i);
    if (i < 0) {
        strcpy(root, path);     /* Didn't find a . */
	}
    else {
        path[i] = '\0';
        strcpy(root, path);
        path[i] = '.';
    }
}


// end of file
