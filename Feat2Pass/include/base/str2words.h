
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
 *	@file	str2words.h
 *	@ingroup common_base_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	Convert a line to an array of words
 */

#ifndef __LIBUTIL_STR2WORDS_H__
#define __LIBUTIL_STR2WORDS_H__

#include "base/hci_type.h"
#include "base/base_lib.h"

#ifdef __cplusplus
extern "C" {
#endif

/** 
 * Convert a line to an array of "words", based on space-like separators.  A word
 * is a string with no whitespace chars in it.
 * Note that the string line is modified as a result: NULL chars are placed after
 * every word in the line.
 *
 * @return Return value: No. of words found; -1 if no. of words in line exceeds n_wptr.
 */
HCILAB_PUBLIC HCI_BASE_API hci_int32
PowerASR_Base_str2words(char *line,				/**< In: line to be parsed */
						char **words,			/**< In/Out: Array of pointers to words found in line.
													 The array must be allocated by the caller */
						hci_int32 max_nWords	/**< In: Size of wptr array */
);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef __LIBUTIL_STR2WORDS_H__
