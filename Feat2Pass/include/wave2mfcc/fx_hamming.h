
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
 *	@file	fx_hamming.h
 *	@ingroup wave2mfcc_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	Hamming windowing library
 */

#ifndef __FX_HAMMING_H__
#define __FX_HAMMING_H__

#include "base/hci_type.h"
#include "basic_op/fixpoint.h"
#include "wave2mfcc/fx_mfcc_common.h"

#ifndef hci_hamming_t
#ifdef FIXED_POINT_FE
typedef hci_fixed16 hci_hamming_t;
#else	// !FIXED_POINT_FE
typedef	hci_float32	hci_hamming_t;
#endif	// #ifdef FIXED_POINT_FE
#endif	// !hci_hamming_t

#ifdef __cplusplus
extern "C" {
#endif

/**
 *	create Hamming window vector
 */
void FX_SigProc_createHammingWindow(hci_hamming_t *pHamming,	///< (o) hamming window vector
									hci_int16 nFrameWidth		///< (i) frame width
);

/**
 *	apply Hamming windowing
 */
void FX_SigProc_applyHamming(hci_mfcc16 *pSample,		///< (i/o) input/output samples
							 hci_hamming_t *pHamming,	///< (i) hamming window vector
							 hci_int16 nFrameWidth		///< (i) window length
);

#ifdef __cplusplus
}
#endif

#endif // #ifndef __FX_HAMMING_H__
