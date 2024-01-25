
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
 *	@file	fx_delta.h
 *	@ingroup mfcc2feat_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	time-derivative feature extraction library
 */

#ifndef __FX_DELTA_H__
#define __FX_DELTA_H__

#include "base/hci_type.h"
#include "basic_op/fixpoint.h"
#include "wave2mfcc/fx_mfcc_common.h"
#include "fx_mfcc2feat.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * compute time derivative coefficients in live mode
 */
M2F_Status
FX_DELTA_computeLiveDerivatives(MFCC_Stream *pMfccStream,		///< (i/o) pointer to mfcc stream
								hci_mfcc_t *pOutMfccVec,		///< (o) output mfcc vector
								MFCC_Cell *pCurCell,			///< (i) current mfcc cell
								Mfcc2FeatParameters *pFXVar		///< (i) config. struct for mfcc-to-feature converter
);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef __FX_DELTA_H__
