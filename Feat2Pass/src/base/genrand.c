
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
 *	@file	genrand.c
 *	@ingroup common_base_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	a portable random generator.
 *
 * A high performance which applied Mersene twister primes to generate
 * random number. If probably seeded, the random generator can achieve 
 * 19937-bits period.  For technical detail.  Please take a look at 
 * (FIXME! Need to search for the web site.)
 */


#include <stdio.h>
#include "base/genrand.h"

/** Period parameters */
#define N 624
#define M 397
#define MATRIX_A 0x9908b0dfUL		///< constant vector a
#define UPPER_MASK 0x80000000UL		///< most significant w-r bits
#define LOWER_MASK 0x7fffffffUL		///< least significant r bits

static unsigned long mt[N];     ///< the array for the state vector
static int mti = N + 1;         ///< mti==N+1 means mt[N] is not initialized

// local functions

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/**
 *	initializes mt[N] with a seed
 */
HCILAB_PRIVATE void
_init_genrand(unsigned long s);

/**
 *	generates a random number on [0,0xffffffff]-interval
 */
HCILAB_PRIVATE unsigned long
_genrand_int32(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

/**
 *	Initialize the seed of the random generator. 
 *
 *	@see _init_genrand
 */
HCILAB_PUBLIC HCI_BASE_API void
__hci_genrand_seed__(unsigned long s)
{
	_init_genrand(s);
}

/**
 *	Generates a random number on [0,0x7fffffff]-interval 
 *
 *	@see _genrand_int32
 */
HCILAB_PUBLIC HCI_BASE_API long
__hci_genrand_int31__(void)
{
    return (long) (_genrand_int32() >> 1);
}

/**
 *	Generates a random number on (0,1)-real-interval 
 *
 *	@see _genrand_int32
 */
HCILAB_PUBLIC HCI_BASE_API double
__hci_genrand_real1__(void)
{
    return _genrand_int32() * (1.0 / 4294967295.0);
    /* divided by 2^32-1 */
}

/**
 *	Generates a random number on (0,1)-real-interval 
 *
 *	@see _genrand_int32
 */
HCILAB_PUBLIC HCI_BASE_API double
__hci_genrand_real2__(void)
{
    return _genrand_int32() * (1.0 / 4294967296.0);
    /* divided by 2^32 */
}

/**
 *	Generates a random number on (0,1)-real-interval 
 *
 *	@see _genrand_int32
 */
HCILAB_PUBLIC HCI_BASE_API double
__hci_genrand_real3__(void)
{
    return (((double) _genrand_int32()) + 0.5) * (1.0 / 4294967296.0);
    /* divided by 2^32 */
}

/**
 *	Generates a random number on [0,1) with 53-bit resolution
 *
 *	@see _genrand_int32
 */
HCILAB_PUBLIC HCI_BASE_API double
__hci_genrand_res53__(void)
{
    unsigned long a = _genrand_int32() >> 5, b = _genrand_int32() >> 6;
    return (a * 67108864.0 + b) * (1.0 / 9007199254740992.0);
}


/**
 *	initializes mt[N] with a seed
 */
HCILAB_PRIVATE void
_init_genrand(unsigned long s)
{
    mt[0] = s & 0xffffffffUL;
    for (mti = 1; mti < N; mti++) {
        mt[mti] =
            (1812433253UL * (mt[mti - 1] ^ (mt[mti - 1] >> 30)) + mti);
        /* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
        /* In the previous versions, MSBs of the seed affect   */
        /* only MSBs of the array mt[].                        */
        /* 2002/01/09 modified by Makoto Matsumoto             */
        mt[mti] &= 0xffffffffUL;
        /* for >32 bit machines */
    }
}

/**
 *	generates a random number on [0,0xffffffff]-interval
 */
HCILAB_PRIVATE unsigned long
_genrand_int32(void)
{
    unsigned long y = 0x0UL;
    static unsigned long mag01[2] = { 0x0UL, MATRIX_A };
    /* mag01[x] = x * MATRIX_A  for x=0,1 */

    if (mti >= N) {             /* generate N words at one time */
        int kk = 0;

        if (mti == N + 1)       /* if init_genrand() has not been called, */
            _init_genrand(5489UL);       /* a default initial seed is used */

        for (kk = 0; kk < N - M; kk++) {
            y = (mt[kk] & UPPER_MASK) | (mt[kk + 1] & LOWER_MASK);
            mt[kk] = mt[kk + M] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        for (; kk < N - 1; kk++) {
            y = (mt[kk] & UPPER_MASK) | (mt[kk + 1] & LOWER_MASK);
            mt[kk] = mt[kk + (M - N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        y = (mt[N - 1] & UPPER_MASK) | (mt[0] & LOWER_MASK);
        mt[N - 1] = mt[M - 1] ^ (y >> 1) ^ mag01[y & 0x1UL];

        mti = 0;
    }

    y = mt[mti++];

    /* Tempering */
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680UL;
    y ^= (y << 15) & 0xefc60000UL;
    y ^= (y >> 18);

    return y;
}

// end of file
