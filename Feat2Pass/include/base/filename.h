
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
 *	@file	filename.h
 *	@ingroup common_base_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	Header file to define API functions related to file and path name operations.
 */


#ifndef __BASELIB_FILENAME_H__
#define __BASELIB_FILENAME_H__

#include "base/hci_type.h"
#include "base/base_lib.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Strip off leading path components from the given path and copy the base into base.
 * Caller must have allocated base.
 */
HCILAB_PUBLIC HCI_BASE_API void
PowerASR_Base_stripBaseNameFromPath(char *path,		///< (i) path string
									char *base);	///< (o) base string


/**
 * Strip off the smallest trailing file-extension suffix and copy
 * the rest into the given root argument.  Caller must have
 * allocated root.
 */
HCILAB_PUBLIC HCI_BASE_API void
PowerASR_Base_stripFileExtension(char *path,		///< (i) path/file string
								 char *root);		///< (o) file-extension suffix가 제거된 string

#ifdef __cplusplus
}
#endif

#endif	// #ifndef __BASELIB_FILENAME_H__
