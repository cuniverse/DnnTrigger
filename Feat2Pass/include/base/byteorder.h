
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
 *	@file	byteorder.h
 *	@ingroup common_base_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	Header file to define Byte swapping ordering macros.
 */

#ifndef __BYTEORDER_H__
#define __BYTEORDER_H__

/** Macro to byteswap an int16 variable.  x = ptr to variable */
#define SWAP_INT16(x)	*(x) = ((0x00ff & (*(x))>>8) | (0xff00 & (*(x))<<8))

/** Macro to byteswap an int32 variable.  x = ptr to variable */
#define SWAP_INT32(x)	*(x) = ((0x000000ff & (*(x))>>24) | \
				(0x0000ff00 & (*(x))>>8) | \
				(0x00ff0000 & (*(x))<<8) | \
				(0xff000000 & (*(x))<<24))

/** Macro to byteswap a float32 variable.  x = ptr to variable */
#define SWAP_FLOAT32(x)	SWAP_INT32((hci_int32 *) x)

/** Macro to byteswap a float64 variable.  x = ptr to variable */
#define SWAP_FLOAT64(x)	{ int *low = (int *) (x), *high = (int *) (x) + 1,\
			      temp;\
			  SWAP_INT32(low);  SWAP_INT32(high);\
			  temp = *low; *low = *high; *high = temp;}

/** Rather confusing backwards compatibility macros for dealing with
 * explicitly big or little-endian data. 
 */
#ifdef WORDS_BIGENDIAN
#define SWAP_BE_64(x)
#define SWAP_BE_32(x)
#define SWAP_BE_16(x)
#define SWAP_LE_64(x) SWAP_FLOAT64(x)
#define SWAP_LE_32(x) SWAP_INT32(x)
#define SWAP_LE_16(x) SWAP_INT16(x)
#else
#define SWAP_LE_64(x)
#define SWAP_LE_32(x)
#define SWAP_LE_16(x)
#define SWAP_BE_64(x) SWAP_FLOAT64(x)
#define SWAP_BE_32(x) SWAP_INT32(x)
#define SWAP_BE_16(x) SWAP_INT16(x)
#endif

#endif	// #ifndef __BYTEORDER_H__
