
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
 *	@file	case.c
 *	@ingroup common_base_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	upper/lower case conversion/handling API functions.
 */


#include <stdlib.h>

#include "base/case.h"

/** 
 * Convert str to all upper case.
 * @param str is a string.
 */
HCILAB_PUBLIC HCI_BASE_API void 
PowerASR_Base_uppercase(register char *cp)
{
    if (cp) {
        while (*cp) {
            *cp = UPPER_CASE(*cp);
            cp++;
        }
    }
}


/** 
 * Convert str to all lower case
 * @param str is a string.
 */
HCILAB_PUBLIC HCI_BASE_API void 
PowerASR_Base_lowercase(register char *cp)
{
    if (cp) {
        while (*cp) {
            *cp = LOWER_CASE(*cp);
            cp++;
        }
    }
}


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
						   const char *str2)
{
    char c1, c2;

    if (str1 == str2) return 0;

    if (str1 && str2) {
        for (;;) {
            c1 = *(str1++);
            if (c1 != '\0') c1 = LOWER_CASE(c1);
            c2 = *(str2++);
            if (c2 != '\0') c2 = LOWER_CASE(c2);
            if (c1 != c2) {
				if ( c1 == '\0' ) return -1;
				else if ( c2 == '\0' ) return 1;
                else return (c1 - c2);
			}
            else if (c1 == '\0')	// iff c1 = c2 = '\0'
                return 0;
        }
    }
    else {
		return (str1 == 0) ? -1 : 1;
	}

    return 0;
}


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
						    const int n)
{
    char c1, c2;
	int i = 0;

    if (str1 == str2) return 0;

    if (str1 && str2 && n) {
        for (i = 0; i < n ; i++) {
            c1 = *(str1++);
            if (c1 != '\0') c1 = LOWER_CASE(c1);
            c2 = *(str2++);
            if (c2 != '\0') c2 = LOWER_CASE(c2);
            if (c1 != c2) {
				if ( c1 == '\0' ) return -1;
				else if ( c2 == '\0' ) return 1;
                else return (c1 - c2);
			}
            else if (c1 == '\0')	// iff c1 = c2 = '\0'
                return 0;
        }
    }
    else {
		return (str1 == 0) ? -1 : 1;
	}

    return 0;
}


// end of file