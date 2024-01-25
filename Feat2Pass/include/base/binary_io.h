
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
 *	@file	binary_io.h
 *	@ingroup common_base_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	Header file to define PowerASR binary file I/O API functions.
 *
 *	cross-platform binary I/O to read/write files
 */

#ifndef __BINARY_IO_H__
#define __BINARY_IO_H__

#include <stdio.h>
#include "base/hci_type.h"
#include "base/base_lib.h"
#include "base/byteorder.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BYTE_ORDER_MAGIC	(0x11223344)

/** "reversed senses" SWAP */
#if (__BIG_ENDIAN__)
#define REVERSE_SENSE_SWAP_INT16(x)  x = ( (((x)<<8)&0x0000ff00) | (((x)>>8)&0x00ff) )
#define REVERSE_SENSE_SWAP_INT32(x)  x = ( (((x)<<24)&0xff000000) | (((x)<<8)&0x00ff0000) | \
                         (((x)>>8)&0x0000ff00) | (((x)>>24)&0x000000ff) )
#else
#define REVERSE_SENSE_SWAP_INT16(x)
#define REVERSE_SENSE_SWAP_INT32(x)
#endif


/**
 *	fread with byte-swapping flag.
 *
 *	@return 로딩된 data element의 수를 return.
 */
HCILAB_PUBLIC HCI_BASE_API size_t 
PowerASR_Base_FREAD(void *buf,			///< (i) data buffer
					size_t el_sz,		///< (i) memory size of a data element
					size_t n_el,		///< (i) count of data elements
					FILE *fp,			///< (i) an input file pointer
					hci_int32 bSwap		///< (i) flag to byte-swap
);

/**
 *	fwrite with byte-swapping flag.
 *
 *	@return 저장된 data element의 수를 return.
 */
HCILAB_PUBLIC HCI_BASE_API size_t 
PowerASR_Base_FWRITE(void *buf,			///< (i) data buffer
					 size_t el_sz,		///< (i) memory size of a data element
					 size_t n_el,		///< (i) count of data elements
					 FILE *fp,			///< (i) an output file pointer
					 hci_int32 bSwap	///< (i) flag to byte-swap
);

#ifdef __cplusplus
}
#endif

#endif // #ifndef __BINARY_IO_H__
