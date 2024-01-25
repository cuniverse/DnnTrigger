
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
 *	@file	binary_io.c
 *	@ingroup common_base_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	PowerASR binary file I/O API functions.
 *
 *	cross-platform binary I/O to read/write files
 */


#include <stdio.h>
#include <string.h>

#include "base/binary_io.h"
#include "base/hci_msg.h"

// local functions

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/**
 *	byte-swapping of buffer data
 */
HCILAB_PRIVATE void
_PowerASR_Base_swapBuffer(void *buf,		///< (i/o) data buffer
						  size_t el_sz,		///< (i) memory size of a data element
						  size_t n_el);		///< (i) count of data elements

#ifdef __cplusplus
}
#endif /* __cplusplus */

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
					hci_int32 bSwap)	///< (i) flag to byte-swap
{
	size_t nLoad = 0;

	nLoad = fread(buf, el_sz, n_el, fp);

    if (bSwap && nLoad)
        _PowerASR_Base_swapBuffer(buf, el_sz, nLoad);

    return nLoad;
}


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
					 hci_int32 bSwap)	///< (i) flag to byte-swap
{
	size_t nSave = 0;

    if (bSwap && n_el)
        _PowerASR_Base_swapBuffer(buf, el_sz, n_el);

	nSave = fwrite(buf, el_sz, n_el, fp);

    if (bSwap && n_el)
        _PowerASR_Base_swapBuffer(buf, el_sz, n_el);

    return nSave;
}


/**
 *	byte-swapping of buffer data
 */
HCILAB_PRIVATE void
_PowerASR_Base_swapBuffer(void *buf,		///< (i/o) data buffer
						  size_t el_sz,		///< (i) memory size of a data element
						  size_t n_el)		///< (i) count of data elements
{
    size_t i = 0;
    hci_uint16 *buf16 = 0;
    hci_uint32 *buf32 = 0;

	if (!n_el) return;

	switch (el_sz) {
    case 1:
        break;
    case 2:
        buf16 = (hci_uint16 *) buf;
        for (i = 0; i < n_el; i++)
            SWAP_INT16(buf16 + i);
        break;
    case 4:
        buf32 = (hci_uint32 *) buf;
        for (i = 0; i < n_el; i++)
            SWAP_INT32(buf32 + i);
        break;
    default:
        HCIMSG_ERROR("Unsupported elemsize for byteswapping: %d\n", el_sz);
        break;
    }
}

// end of file
