
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
 *	@file	str2words.c
 *	@ingroup common_base_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	Convert a line to an array of words
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "base/str2words.h"

// local functions

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/**
 * check whether a given char is a kind of space-like characters.
 *	# space-like characters; whitespace, newline, carriage return, tab
 *  # if yes, return 1. oterwise, return 0.
 */
HCILAB_PRIVATE hci_int32
_isSpaceLikeChar(char c);

#ifdef __cplusplus
}
#endif /* __cplusplus */


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
						hci_int32 max_nWords)	/**< In: Size of wptr array */
{
    hci_int32 i = 0;	// #words found so far
	hci_int32 n = 0;	// For scanning through the input string

    for (;;) {

        // Skip space-like characters before next word 
        for (; line[i] && (_isSpaceLikeChar(line[i])); i++);
        if (!line[i])
            break;

        if (n >= max_nWords) {
            /*
             * Pointer array size insufficient.  Restore NULL chars inserted so far
             * to space chars.  Not a perfect restoration, but better than nothing.
             */
            for (; i >= 0; --i)
                if (line[i] == '\0')
                    line[i] = ' ';

            return -1;
        }

        // Scan to end of word
        words[n++] = line + i;
        for (; line[i] && (!_isSpaceLikeChar(line[i])); i++);
        if (!line[i])
            break;
        line[i++] = '\0';
    }

    return n;
}


/**
 * check whether a given char is a kind of space-like characters.
 *	# space-like characters; whitespace, newline, carriage return, tab
 *  # if yes, return 1. oterwise, return 0.
 */
HCILAB_PRIVATE hci_int32
_isSpaceLikeChar(char c)
{
	if ( isspace((unsigned char) c) || c=='\n' || c=='\r' || c=='\t' ) {
		return 1;
	}
	else {
		return 0;
	}
}

// end of file
