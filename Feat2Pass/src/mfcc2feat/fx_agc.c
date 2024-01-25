
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
 *	@file	fx_agc.c
 *	@ingroup mfcc2feat_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	EN (energy normalization) library
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "base/hci_msg.h"
#include "base/hci_macro.h"
#include "mfcc2feat/fx_agc.h"

#ifdef FIXED_POINT_FE
#define ESCALE 13		// = 0.1 , Q7.32
//#define LOGE_RANGE 377255	// = 50 * log(10) / 10,  Q15.32
#define LOGE_RANGE 491520	// = 50 * log(10) / 10,  Q15.32
//#define MIN_LOGE 425984	// Q15.32
#define MIN_LOGE 360448		// Q15.32
#else	// !FIXED_POINT_FE
#define ESCALE (0.1f)
//#define LOGE_RANGE (11.513f)
#define LOGE_RANGE (15.0f)
//#define MIN_LOGE (13.0f)
#define MIN_LOGE (11.0f)
#endif	// #ifdef FIXED_POINT_FE


/**
 *	initialize maximum log energy for current utterance
 */
hci_int32
FX_AGC_initializeMaxLogEnergy(MFCC_Stream *pMfccStream,		///< (i/o) pointer to mfcc stream
							  EN_Type typeEN)				///< (i) EN type
{
	FEAT_Normalizer *pUserFeatNorm = 0;
	FEAT_Accumulator *pUserFeatAccum = 0;

	if (0 == pMfccStream) {
		return -1;
	}
	else {
		pUserFeatNorm  = &pMfccStream->userFeatNorm;
		pUserFeatAccum = &pMfccStream->userFeatAccum;
		if (LIVE_EN == typeEN) {
			pUserFeatAccum->maxLogE = pUserFeatNorm->maxLogE;
			pUserFeatNorm->minLogE = pUserFeatNorm->maxLogE - LOGE_RANGE;
		}
		else {
			pUserFeatAccum->maxLogE = 0;
		}
		return 0;
	}
}


/**
 *	live-mode log-energy normalization
 */
hci_mfcc_t
FX_AGC_liveEnergyNormalization(MFCC_Stream *pMfccStream,	///< (i/o) pointer to mfcc stream
							   hci_mfcc_t currentLogE,		///< (i) log energy at current frame
							   EN_Type typeEN)				///< (i) EN type
{
	hci_mfcc_t logE = 0;
	FEAT_Normalizer *pUserFeatNorm = 0;
	FEAT_Accumulator *pUserFeatAccum = 0;

	pUserFeatNorm  = &pMfccStream->userFeatNorm;
	pUserFeatAccum = &pMfccStream->userFeatAccum;

	switch(typeEN) {
	case NO_EN:
		return currentLogE;
	case BATCH_EN:
		pUserFeatAccum->maxLogE = HCI_MAX(pUserFeatAccum->maxLogE, currentLogE);
		return currentLogE;
	case LIVE_EN:
		pUserFeatAccum->maxLogE = HCI_MAX(pUserFeatAccum->maxLogE, currentLogE);
		//logE = HCI_MAX(currentLogE, pUserFeatNorm->minLogE);
		logE = HCI_MAX(currentLogE, MIN_LOGE);
		logE -= pUserFeatNorm->maxLogE;	// Q15.32
#ifdef FIXED_POINT_FE
		logE *= ESCALE;					// Q22.32
		logE = (logE>>7);				// Q15.32
		logE += 0x00008000L;
#else	// !FIXED_POINT_FE
		logE *= ESCALE;
		logE += 1.0f;
#endif	// #ifdef FIXED_POINT_FE
		return logE;
	default:
		return currentLogE;
	}

}


/**
 *	batch-mode log-energy normalization
 */
hci_mfcc_t
FX_AGC_batchEnergyNormalization(hci_mfcc_t frameLogE,	///< (i) frame log-energy
								hci_mfcc_t maxLogE,		///< (i) maximum log-energy
								hci_mfcc_t minLogE)		///< (i) minimum log-energy
{
	hci_mfcc_t outLogE = 0;

	outLogE = HCI_MAX(frameLogE, minLogE);
	outLogE -= maxLogE;			// Q15.32
#ifdef FIXED_POINT_FE
	outLogE *= ESCALE;			// Q22.32
	outLogE = (outLogE>>7);		// Q15.32
	outLogE += 0x00008000L;
#else	// !FIXED_POINT_FE
	outLogE *= ESCALE;
	outLogE += 1.0f;
#endif	// #ifdef FIXED_POINT_FE

	return outLogE;
}


/**
 *	update maximum log energy
 */
hci_int32
FX_AGC_updateMaxLogEnergy(MFCC_Stream *pMfccStream,		///< (i/o) pointer to mfcc stream
						  EN_Type typeEN)				///< (i) EN type
{
	if (NO_EN != typeEN) {
		FEAT_Normalizer *pUserFeatNorm  = &pMfccStream->userFeatNorm;
		FEAT_Normalizer *pBaseFeatNorm  = &pMfccStream->baseFeatNorm;
		FEAT_Accumulator *pUserFeatAccum = &pMfccStream->userFeatAccum;

		pUserFeatNorm->maxLogE = pUserFeatAccum->maxLogE;
		pUserFeatNorm->minLogE = pUserFeatNorm->maxLogE - LOGE_RANGE;

		memcpy( pBaseFeatNorm, pUserFeatNorm, sizeof(*pBaseFeatNorm) );
	}

	return 0;
}

/* end of file */






















