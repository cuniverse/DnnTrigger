
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
 *	@file	case.h
 *	@ingroup common_base_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	Header file to define upper/lower case conversion/handling API functions.
 */


#ifndef __BASELIB_CASE_H__
#define __BASELIB_CASE_H__

#include "base/hci_type.h"
#include "base/base_lib.h"

#ifdef __cplusplus
extern "C" {
#endif

/** 
 * Return upper case form for c 
 */
#define UPPER_CASE(c)	((((c) >= 'a') && ((c) <= 'z')) ? (c-32) : c)

/**
 * Return lower case form for c 
 */
#define LOWER_CASE(c)	((((c) >= 'A') && ((c) <= 'Z')) ? (c+32) : c)


/** 
 * Convert str to all upper case.
 * @param str is a string.
 */
HCILAB_PUBLIC HCI_BASE_API void 
PowerASR_Base_uppercase(char *str);

/** 
 * Convert str to all lower case
 * @param str is a string.
 */
HCILAB_PUBLIC HCI_BASE_API void 
PowerASR_Base_lowercase(char *str);

/**
 * Case insensitive string compare.
 * @return Return the usual -1, 0, +1, depending on
 *  str1 <, =, > str2 (case insensitive, of course).
 * @param str1 is the first string.
 * @param str2 is the second string. 
 * @warning FIXME! The implementation is incorrect!
 */
HCILAB_PUBLIC HCI_BASE_API hci_int32 
PowerASR_Base_strnocasecmp(const char *str1,
						   const char *str2);

/**
 * Case insensitive string compare.
 * @return Return the usual -1, 0, +1, depending on
 *  str1 <, =, > str2 (case insensitive, of course).
 * @param str1 is the first string.
 * @param str2 is the second string. 
 * @param n is comparison length.
 * @warning FIXME! The implementation is incorrect!
 */
HCILAB_PUBLIC HCI_BASE_API hci_int32 
PowerASR_Base_strnocasencmp(const char *str1,
						    const char *str2,
						    const int n);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef __BASELIB_CASE_H__
