
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
 *	@file	fx_agc.h
 *	@ingroup mfcc2feat_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	EN (energy normalization) library
 */

#ifndef __FX_AGC_H__
#define __FX_AGC_H__

#include "base/hci_type.h"
#include "basic_op/fixpoint.h"
#include "wave2mfcc/fx_mfcc_common.h"
#include "fx_mfcc2feat.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 *	initialize maximum log energy for current utterance
 */
hci_int32
FX_AGC_initializeMaxLogEnergy(MFCC_Stream *pMfccStream,		///< (i/o) pointer to mfcc stream
							  EN_Type typeEN				///< (i) EN type
);

/**
 *	live-mode log-energy normalization
 */
hci_mfcc_t
FX_AGC_liveEnergyNormalization(MFCC_Stream *pMfccStream,	///< (i/o) pointer to mfcc stream
							   hci_mfcc_t currentLogE,		///< (i) log energy at current frame
							   EN_Type typeEN				///< (i) EN type
);

/**
 *	batch-mode log-energy normalization
 */
hci_mfcc_t
FX_AGC_batchEnergyNormalization(hci_mfcc_t frameLogE,	///< (i) frame log-energy
								hci_mfcc_t maxLogE,		///< (i) maximum log-energy
								hci_mfcc_t minLogE		///< (i) minimum log-energy
);

/**
 *	update maximum log energy
 */
hci_int32
FX_AGC_updateMaxLogEnergy(MFCC_Stream *pMfccStream,		///< (i/o) pointer to mfcc stream
						  EN_Type typeEN				///< (i) EN type
);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef __FX_AGC_H__
