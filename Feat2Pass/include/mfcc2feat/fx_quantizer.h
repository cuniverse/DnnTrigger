
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
 *	@file	fx_quantizer.h
 *	@ingroup mfcc2feat_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	Feature Quantization library
 */

#ifndef __FX_QUANTIZER_H__
#define __FX_QUANTIZER_H__

#include "base/hci_type.h"
#include "basic_op/fixpoint.h"
#include "wave2mfcc/fx_mfcc_common.h"
#include "fx_mfcc2feat.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 *	load feature quantizer data
 */
hci_int32
FX_QUANTIZER_loadFeatureQuantizer(const char *szMfccQuantizerFile,	///< (i) feature quantizer file
								  FEAT_Quantizer *mfccQuantizer,	///< (o) feature quantizer
								  hci_int16 dimFeat					///< (i) feature dimension
);

/**
 *	quantize feature vector
 */
hci_int32
FX_QUANTIZER_quantizeFeature(Feature_UserData *pFeatureData,	///< (o) feature user data
							 FEAT_Quantizer *mfccQuantizer,		///< (i) feature quantizer
							 hci_int16 dimFeat					///< (i) feature dimension
);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef __FX_QUANTIZER_H__
