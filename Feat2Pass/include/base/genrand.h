
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
 *	@file	genrand.h
 *	@ingroup common_base_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	Header file to define API functions related to a portable random generator.
 *
 * A high performance which applied Mersene twister primes to generate
 * random number. If probably seeded, the random generator can achieve 
 * 19937-bits period.  For technical detail.  Please take a look at 
 * (FIXME! Need to search for the web site.)
 */


#ifndef __BASELIB_GENRAND_H__
#define __BASELIB_GENRAND_H__

#define S3_RAND_MAX_INT32 0x7fffffff
#include <stdio.h>
#include "base/hci_type.h"
#include "base/base_lib.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 *	Macros to simplify calling of random generator function.
 */
#define hci_rand_seed(s) __hci_genrand_seed__(s);
#define hci_rand_int31() __hci_genrand_int31__()
#define hci_rand_real()  __hci_genrand_real3__()
#define hci_rand_res53() __hci_genrand_res53__()

/**
 *	Initialize the seed of the random generator. 
 */
HCILAB_PUBLIC HCI_BASE_API void
__hci_genrand_seed__(unsigned long s);

/**
 *	Generates a random number on [0,0x7fffffff]-interval 
 */
HCILAB_PUBLIC HCI_BASE_API long
__hci_genrand_int31__(void);

/**
 *	Generates a random number on (0,1)-real-interval 
 */
HCILAB_PUBLIC HCI_BASE_API double
__hci_genrand_real1__(void);
HCILAB_PUBLIC HCI_BASE_API double
__hci_genrand_real2__(void);
HCILAB_PUBLIC HCI_BASE_API double
__hci_genrand_real3__(void);

/**
 *	Generates a random number on [0,1) with 53-bit resolution
 */
HCILAB_PUBLIC HCI_BASE_API double
__hci_genrand_res53__(void);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef __BASELIB_GENRAND_H__



