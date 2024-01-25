
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
 *	@file	fixpoint.h
 *	@ingroup basic_op_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	Fixed-point arithmetic macros.
 */

#ifndef __BASELIB_FIXPOINT_H__
#define __BASELIB_FIXPOINT_H__

#include "base/hci_type.h"
#include <limits.h>

#define DEFAULT_RADIX (12)

/** Default radix point for fixed-point */
/* #undef DEFAULT_RADIX */

/** Use Q15 fixed-point computation */
/* #undef FIXED16 */

/** Use fixed-point computation */
/* #undef FIXED_POINT */
//#define FIXED_POINT 1

// #if defined(FIXED16) && !defined(FIXED_POINT)
// #define FIXED_POINT
// #endif

//#define FIXED_POINT_FE		///< if defined, use fixed-point front-end APIs
//#define FIXED_POINT_BE		///< if defined, use fixed-point back-end APIs

//#define USE_SCALED_LL

#ifdef USE_SCALED_LL
#define FLOOR_SCORE	(MIN_INT16>>1)
typedef	hci_int16	hci_score_t;
#else
#define FLOOR_SCORE	(MIN_INT32>>1)
typedef	hci_int32	hci_score_t;
#endif

/** Fixed-point computation type. */
typedef hci_int32 hci_fixed32;
typedef hci_int16 hci_fixed16;

/** Convert floating point to fixed point. */
#define FLOAT2FIX32_ANY(x,radix) \
	(((x)<0.0) ? \
	((hci_fixed32)((x)*(hci_float32)(1<<(radix)) - 0.5)) \
	: ((hci_fixed32)((x)*(hci_float32)(1<<(radix)) + 0.5)))
#define FLOAT2FIX16_ANY(x,radix) \
	(((x)<0.0) ? \
	((hci_fixed16)((x)*(hci_float32)(1<<(radix)) - 0.5)) \
	: ((hci_fixed16)((x)*(hci_float32)(1<<(radix)) + 0.5)))
#define FLOAT2FIX32(x) FLOAT2FIX32_ANY(x,DEFAULT_RADIX)
#define FLOAT2FIX16(x) FLOAT2FIX16_ANY(x,DEFAULT_RADIX)
/** Convert fixed point to floating point. */
#define FIX2FLOAT_ANY(x,radix) ((hci_float32)(x)/(1<<(radix)))
#define FIX2FLOAT(x) FIX2FLOAT_ANY(x,DEFAULT_RADIX)


#define HCI_ROUND16(a)      ((hci_int16)( ((a) >= 0) ? ((hci_int16)((a) + 0.5)) : ((hci_int16)((a) - 0.5))))
#define HCI_ROUND32(a)      ( ((a) >= 0) ? ((hci_int32)((a) + 0.5)) : ((hci_int32)((a) - 0.5)))

/**
 * Multiply two fixed point numbers with an arbitrary radix point.
 *
 * A veritable multiplicity of implementations exist, starting with
 * the fastest ones...
 */
#if defined(__arm__) /* Actually this is StrongARM-specific... */
#define FIXMUL32(a,b) FIXMUL32_ANY(a,b,DEFAULT_RADIX)
#define FIXMUL16(a,b) FIXMUL16_ANY(a,b,DEFAULT_RADIX)
#define FIXMUL32_ANY(a,b,r) ({				\
      int cl, ch, _a = a, _b = b;			\
      asm ("smull %0, %1, %2, %3\n"			\
	   "mov %0, %0, lsr %4\n"			\
	   "orr %0, %0, %1, lsl %5\n"			\
	   : "=&r" (cl), "=&r" (ch)			\
	   : "r" (_a), "r" (_b), "i" (r), "i" (32-(r)));\
      cl; })
/* FIXMUL16_ANY is not safe. Only use when overflows are not occurred !! */
#define FIXMUL16_ANY(a,b,radix) ((hci_fixed16)(((hci_int32)((a)*(b)))>>(radix)))
#elif defined(BFIN) && DEFAULT_RADIX == 16
/* Blackfin magic */
#undef FIXMUL32
#undef FIXMUL16
/* Use the accumulators for the 16.16 case (probably not as efficient as it could be). */
#define FIXMUL32(a,b) ({					\
      int c, _a = a, _b = b;				\
	asm("%0.L = %1.l * %2.l (FU);\n\t"		\
	    "%0.H = %1.h * %2.h (IS);\n\t"		\
	    "A1 = %0;\n\t"				\
	    "A1 += %1.h * %2.l (IS, M);\n\t"		\
	    "%0 = (A1 += %2.h * %1.l) (IS, M);\n\t"	\
	    : "=&W" (c)					\
	    : "d" (_a), "d" (_b)			\
	    : "A1", "cc");					\
      c; })
#define FIXMUL16(a,b) FIXMUL16_ANY(a,b,DEFAULT_RADIX)
#define FIXMUL32_ANY(a,b,radix) ((hci_fixed32)(((hci_int64)(a)*(b))>>(radix)))
/* FIXMUL16_ANY is not safe. Only use when overflows are not occurred !! */
#define FIXMUL16_ANY(a,b,radix) ((hci_fixed16)(((hci_int32)((a)*(b)))>>(radix)))
#elif defined(HAVE_LONG_LONG) && SIZEOF_LONG_LONG == 8
#define FIXMUL32(a,b) FIXMUL32_ANY(a,b,DEFAULT_RADIX)
#define FIXMUL16(a,b) FIXMUL16_ANY(a,b,DEFAULT_RADIX)
#define FIXMUL32_ANY(a,b,radix) ((hci_fixed32)(((hci_int64)(a)*(b))>>(radix)))
/* FIXMUL16_ANY is not safe. Only use when overflows are not occurred !! */
#define FIXMUL16_ANY(a,b,radix) ((hci_fixed16)(((hci_int32)((a)*(b)))>>(radix)))
#else /* Most general case where 'long long' doesn't exist or is slow. */
#define FIXMUL32(a,b) FIXMUL32_ANY(a,b,DEFAULT_RADIX)
#define FIXMUL16(a,b) FIXMUL16_ANY(a,b,DEFAULT_RADIX)
#define FIXMUL32_ANY(a,b,radix) \
	(hci_fixed32)(((((hci_uint32)(a))&((1<<(radix))-1))	    \
		   * (((hci_uint32)(b))&((1<<(radix))-1)) >> (radix))       \
	+ (((((hci_int32)(a))>>(radix)) * (((hci_int32)(b))>>(radix))) << (radix)) \
	+ ((((hci_uint32)(a))&((1<<(radix))-1)) * (((hci_int32)(b))>>(radix))) \
	+ ((((hci_uint32)(b))&((1<<(radix))-1)) * (((hci_int32)(a))>>(radix))))
/* FIXMUL16_ANY is not safe. Only use when overflows are not occurred !! */
#define FIXMUL16_ANY(a,b,radix) \
	(hci_fixed16)(((((hci_uint32)(a))&((1<<(radix))-1))	    \
		   * (((hci_uint32)(b))&((1<<(radix))-1)) >> (radix))       \
	+ (((((hci_int32)(a))>>(radix)) * (((hci_int32)(b))>>(radix))) << (radix)) \
	+ ((((hci_uint32)(a))&((1<<(radix))-1)) * (((hci_int32)(b))>>(radix))) \
	+ ((((hci_uint32)(b))&((1<<(radix))-1)) * (((hci_int32)(a))>>(radix))))
#endif

#endif // #ifndef __BASELIB_FIXPOINT_H__
