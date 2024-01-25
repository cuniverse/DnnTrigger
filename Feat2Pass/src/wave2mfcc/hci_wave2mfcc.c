
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
 *	@file	hci_wave2mfcc.c
 *	@ingroup wave2mfcc_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	PowerASR wave-to-mfcc converter library
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pffft.h"

#include "base/hci_malloc.h"
#include "base/hci_msg.h"
#include "base/parse_config.h"
#include "base/case.h"

#include "wave2mfcc/fx_mfcc.h"
#include "wave2mfcc/hci_wave2mfcc.h"

/** inner data struct for wave-to-mfcc converter */
typedef struct 
{
	MfccParameters paraMfcc;	///< parameters for MFCC extractor
} Wave2Mfcc_Inner;

// local functions
#ifdef __cplusplus
extern "C" {
#endif 

/**
 * setup default environments for wave-to-mfcc converter
 */
HCILAB_PRIVATE hci_int32
_FX_Wave2Mfcc_defaultConfigurations(PowerASR_FX_Wave2Mfcc *pThis,
									const hci_int32 nSampleRate);


/**
 * setup environments from a given configuration file
 */
HCILAB_PRIVATE hci_int32
_FX_Wave2Mfcc_loadConfigurations(PowerASR_FX_Wave2Mfcc *pThis,
								 const char *pszConfigFile);

#ifdef __cplusplus
}
#endif

/**
 *	create a new wave-to-mfcc converter.
 *
 *	@return Return the pointer to a newly created wave-to-mfcc converter
 */
HCILAB_PUBLIC HCI_WAVE2MFCC_API PowerASR_FX_Wave2Mfcc*
PowerASR_FX_Wave2Mfcc_new()
{
	PowerASR_FX_Wave2Mfcc *pFX_Wave2Mfcc = 0;
	Wave2Mfcc_Inner *pInner = 0;

	pFX_Wave2Mfcc = (PowerASR_FX_Wave2Mfcc *) hci_malloc( sizeof(PowerASR_FX_Wave2Mfcc) );

	if ( pFX_Wave2Mfcc ) {
		memset(pFX_Wave2Mfcc, 0, sizeof(PowerASR_FX_Wave2Mfcc));

		pInner = (Wave2Mfcc_Inner *) hci_malloc( sizeof(Wave2Mfcc_Inner) );
		if ( pInner ) {
			memset(pInner, 0, sizeof(Wave2Mfcc_Inner));
			pFX_Wave2Mfcc->pInner = (void *)pInner;
		}
	}
	else {
		HCIMSG_ERROR("cannot create FX wave-to-mfcc converter.\n");
	}

	return pFX_Wave2Mfcc;
}


/**
 *	delete the wave-to-mfcc converter.
 */
HCILAB_PUBLIC HCI_WAVE2MFCC_API void
PowerASR_FX_Wave2Mfcc_delete(PowerASR_FX_Wave2Mfcc *pThis)
{
	Wave2Mfcc_Inner *pInner = 0;

	if (0 == pThis) {
		return;
	}

	pInner = (Wave2Mfcc_Inner *) pThis->pInner;

	if ( pInner ) hci_free(pInner);
	hci_free(pThis);
}


/**
 *	set-up environments for wave-to-mfcc converter,
 *	and allocate necessary memories.
 *
 *	@return return 0 if Wave2Mfcc environments are set-up correctly, otherwise return -1.
 *
 *	@see FX_Wave2Mfcc_buildFeatureExtractor
 */
HCILAB_PUBLIC HCI_WAVE2MFCC_API hci_int32
PowerASR_FX_Wave2Mfcc_openWave2MfccConverter(PowerASR_FX_Wave2Mfcc *pThis,	///< (i/o) pointer to the wave-to-mfcc converter
											 const char *pszConfigFile,		///< (i) Wave2Mfcc configuration file
											 const hci_int32 nSampleRate,	///< (i) sampling frequency in Hz
											 hci_logadd_t *pLogAddTbl)		///< (i) pointer to log-addition table
{
	Wave2Mfcc_Inner *pInner = 0;
	hci_int32 bSetup = 0;

	if (0 == pThis) {
		return -1;
	}

	pInner = (Wave2Mfcc_Inner *) pThis->pInner;

	if (0 == pszConfigFile) {
		bSetup = _FX_Wave2Mfcc_defaultConfigurations(pThis, nSampleRate);
	}
	else {
		bSetup = _FX_Wave2Mfcc_loadConfigurations(pThis, pszConfigFile);
	}

	if (0 != bSetup) {
		return -1;
	}

	bSetup = FX_Wave2Mfcc_buildFeatureExtractor(&pInner->paraMfcc);

	pInner->paraMfcc.logAddTbl = pLogAddTbl;

	return bSetup;
}


/**
 *	free memories allocated to the wave-to-mfcc converter.
 *
 *	@return return 0 if all memories are freed successfully, otherwise return -1.
 */
HCILAB_PUBLIC HCI_WAVE2MFCC_API hci_int32
PowerASR_FX_Wave2Mfcc_closeWave2MfccConverter(PowerASR_FX_Wave2Mfcc *pThis)	///< (i/o) pointer to the wave-to-mfcc converter
{
	Wave2Mfcc_Inner* pInner = (Wave2Mfcc_Inner*)pThis->pInner;
	if (pInner->paraMfcc.fftSetup)
		pffft_destroy_setup(pInner->paraMfcc.fftSetup);
	return (pThis ? 0 : -1);
}


/**
 *	initialize data buffers for wave-to-mfcc converter.
 *
 *	@return return 0 if Wave2Mfcc data buffers are initialized successfully, otherwise return -1.
 */
HCILAB_PUBLIC HCI_WAVE2MFCC_API hci_int32
PowerASR_FX_Wave2Mfcc_initializeWave2MfccConverter(PowerASR_FX_Wave2Mfcc *pThis,	///< (i) pointer to the wave-to-mfcc converter
												   MFCC_UserData *pMfccData)		///< (o) channel-specific Wave2Mfcc data struct
{
	Wave2Mfcc_Inner *pInner = 0;

	if (0 == pThis) {
		return -1;
	}

	pInner = (Wave2Mfcc_Inner *) pThis->pInner;
	memset(pMfccData, 0, sizeof(MFCC_UserData));
	
	pMfccData->nNumMelBands = pInner->paraMfcc.nNumFilters;

	pMfccData->nDimMFCC = (hci_int16)(pInner->paraMfcc.nNumCepstra + 1);
	if (pInner->paraMfcc.addEntropy) {
		pMfccData->nDimMFCC += 1;
	}

	pMfccData->biasEntropy = pInner->paraMfcc.biasConstantEntropy;

	return 0;
}


/**
 *	convert a single input frame samples to a MFCC vector.
 *
 *	@return return W2M_FAIL, W2M_FALSE, W2M_TRUE.
 *		- return W2M_FAIL if Wave2Mfcc operation failed
 *		- return W2M_FALSE if returned outputs don't exist
 *		- return W2M_TRUE if returned outputs exist
 *
 *	@see FX_Wave2Mfcc_convertSingleFrameToMfccVector
 */
HCILAB_PUBLIC HCI_WAVE2MFCC_API W2M_Status
PowerASR_FX_Wave2Mfcc_convertFrameSample2Mfcc(PowerASR_FX_Wave2Mfcc *pThis,		///< (i) pointer to the wave-to-mfcc converter
											  MFCC_UserData *pMfccData,			///< (o) channel-specific Wave2Mfcc data struct
											  hci_int16 *pFrameBuf,				///< (i/o) an input frame buffer
											  const hci_int16 flagVAD)			///< (i) VAD flag (0 = unvoiced, 1 = voiced)
{
	Wave2Mfcc_Inner *pInner = 0;
	hci_int32 L_var_out = 0;

	if (0 == pThis) {
		return W2M_FAIL;
	}

	pInner = (Wave2Mfcc_Inner *) pThis->pInner;

	L_var_out = FX_Wave2Mfcc_convertSingleFrameToMfccVector(&pInner->paraMfcc,
															pMfccData,
															pFrameBuf,
															flagVAD);

	return (L_var_out==0) ? W2M_TRUE : W2M_FALSE;
}


/**
 * setup default environments for FX wave-to-mfcc converter
 *		- specific to a given sampling frequency
 */
HCILAB_PRIVATE hci_int32
_FX_Wave2Mfcc_defaultConfigurations(PowerASR_FX_Wave2Mfcc *pThis,
									const hci_int32 nSampleRate)
{
	Wave2Mfcc_Inner *pInner = 0;
	MfccParameters *pMfccVar = 0;
	hci_float32 fValue = 0;
	hci_float32 weightC0 = 0;

	if (0 == pThis) {
		return -1;
	}

	if (nSampleRate != 8000 && nSampleRate != 16000) {
		HCIMSG_ERROR("invalid sampling frequency: %d.\n", nSampleRate);
		return -1;
	}

	pInner = (Wave2Mfcc_Inner *) pThis->pInner;
	pMfccVar = &pInner->paraMfcc;

	pMfccVar->nSampleRate = nSampleRate;
	pMfccVar->nMFCCType   = HTK_MFCC;			// HTK_MFCC, ETSI_MFCC, DPS_MFCC, PDPS_MFCC
	pMfccVar->nEnergyType = TIME_ENERGY;		// TIME_ENERGY, FREQ_ENERGY
	if (8000 == nSampleRate) {
		pMfccVar->nFrameShift    = NB_FRAME_SHIFT;
		pMfccVar->nFrameSize     = NB_FRAME_SIZE;
		pMfccVar->nFFTSize       = NB_FFT_SIZE;
		pMfccVar->nNumFilters    = NUM_FILTERBANK;
		pMfccVar->nNumCepstra    = DIM_CEPSTRUM;
		pMfccVar->nLowerFiltFreq = NB_LOWER_FILT_FREQ;
		pMfccVar->nUpperFiltFreq = NB_UPPER_FILT_FREQ;
		if (pMfccVar->nMFCCType == ETSI_MFCC) {
			fValue = (hci_float32)PRE_EMPHASIS_ETSI;
			weightC0 = 0.6f;
		}
		else {
			fValue = (hci_float32)PRE_EMPHASIS_HTK;
			weightC0 = 0.0f;
		}
#ifdef FIXED_POINT_FE
		pMfccVar->PreEmphasis = FLOAT2FIX16_ANY(fValue,15);
		pMfccVar->weightC0    = FLOAT2FIX16_ANY(weightC0,15);
#else	// !FIXED_POINT_FE
		pMfccVar->PreEmphasis = (hci_mfcc16)fValue;
		pMfccVar->weightC0    = (hci_mfcc16)weightC0;
#endif	// #ifdef FIXED_POINT_FE
		if (pMfccVar->nMFCCType == HTK_MFCC) {
			pMfccVar->nCepLiftOrder = CEP_LIFTER_SIZE;
		}
		else {
			pMfccVar->nCepLiftOrder = 0;
		}
	}
	else {	// default setting at 16 kHz
		pMfccVar->nFrameShift    = BB_FRAME_SHIFT;
		pMfccVar->nFrameSize     = BB_FRAME_SIZE;
		pMfccVar->nFFTSize       = BB_FFT_SIZE;
		pMfccVar->nNumFilters    = NUM_FILTERBANK;
		pMfccVar->nNumCepstra    = DIM_CEPSTRUM;
		pMfccVar->nLowerFiltFreq = BB_LOWER_FILT_FREQ;
		pMfccVar->nUpperFiltFreq = BB_UPPER_FILT_FREQ;
		if (pMfccVar->nMFCCType == ETSI_MFCC) {
			fValue = (hci_float32)PRE_EMPHASIS_ETSI;
			weightC0 = 0.6f;
		}
		else {
			fValue = (hci_float32)PRE_EMPHASIS_HTK;
			weightC0 = 0.0f;
		}
#ifdef FIXED_POINT_FE
		pMfccVar->PreEmphasis = FLOAT2FIX16_ANY(fValue,15);
		pMfccVar->weightC0    = FLOAT2FIX16_ANY(weightC0,15);
#else	// !FIXED_POINT_FE
		pMfccVar->PreEmphasis = (hci_mfcc16)fValue;
		pMfccVar->weightC0    = (hci_mfcc16)weightC0;
#endif	// #ifdef FIXED_POINT_FE
		if (pMfccVar->nMFCCType == HTK_MFCC) {
			pMfccVar->nCepLiftOrder = CEP_LIFTER_SIZE;
		}
		else {
			pMfccVar->nCepLiftOrder = 0;
		}
	}
	pMfccVar->addEntropy = FALSE;

	pMfccVar->nHalfFFT = (pMfccVar->nFFTSize>>1);

	return 0;
}


/**
 * setup environments from a given configuration file
 */
HCILAB_PRIVATE hci_int32
_FX_Wave2Mfcc_loadConfigurations(PowerASR_FX_Wave2Mfcc *pThis,
								 const char *pszConfigFile)
{
	Wave2Mfcc_Inner *pInner = 0;
	MfccParameters *pMfccVar = 0;
	char *pszValue = 0;
	hci_float32 fValue = 0;

	if (0 == pThis) {
		return -1;
	}

	pInner = (Wave2Mfcc_Inner *) pThis->pInner;
	pMfccVar = &pInner->paraMfcc;

	if (0 != PowerASR_Base_parseConfigFile(pszConfigFile)) {
		HCIMSG_ERROR("parseConfigFile failed (%s).\n", pszConfigFile);
		return -1;
	}

	// sampling frequency in Hz
	pszValue = PowerASR_Base_getArgumentValue("SAMPLE_RATE");
	if (pszValue) {
		pMfccVar->nSampleRate = (hci_int16)atoi(pszValue);
		if (pMfccVar->nSampleRate != 8000 && pMfccVar->nSampleRate != 16000) {
			HCIMSG_ERROR("invalid sampling frequency: %d.\n", pMfccVar->nSampleRate);
			goto lb_err_wave2mfcc_loadConfig;
		}
	}
	else {
		HCIMSG_WARN("[WAVE2MFCC] SAMPLE_RATE not assigned.\n");
		goto lb_err_wave2mfcc_loadConfig;
	}

	// MFCC Type
	pszValue = PowerASR_Base_getArgumentValue("MFCC_TYPE");
	if (pszValue) {
		if (strcmp(pszValue, "HTK_MFCC") == 0) {
			pMfccVar->nMFCCType = HTK_MFCC;
		}
		else if (strcmp(pszValue, "ETSI_MFCC") == 0) {
			pMfccVar->nMFCCType = ETSI_MFCC;
		}
		else if (strcmp(pszValue, "DPS_MFCC") == 0) {
			pMfccVar->nMFCCType = DPS_MFCC;
		}
		else if (strcmp(pszValue, "PDPS_MFCC") == 0) {
			pMfccVar->nMFCCType = PDPS_MFCC;
		}
		else {
			HCIMSG_ERROR("invalid MFCC Type: %d.\n", pszValue);
			goto lb_err_wave2mfcc_loadConfig;
		}
	}
	else {
		HCIMSG_WARN("[WAVE2MFCC] MFCC_TYPE not assigned.\n");
		goto lb_err_wave2mfcc_loadConfig;
	}

	// Energy Type
	pszValue = PowerASR_Base_getArgumentValue("ENERGY_TYPE");
	if (pszValue) {
		if (strcmp(pszValue, "TIME") == 0) {
			pMfccVar->nEnergyType = TIME_ENERGY;
		}
		else if (strcmp(pszValue, "FREQ") == 0) {
			pMfccVar->nEnergyType = FREQ_ENERGY;
		}
		else {
			HCIMSG_ERROR("invalid MFCC Type: %d.\n", pszValue);
			goto lb_err_wave2mfcc_loadConfig;
		}
	}
	else {
		HCIMSG_WARN("[WAVE2MFCC] MFCC_TYPE not assigned.\n");
		goto lb_err_wave2mfcc_loadConfig;
	}

	// frame shift 
	pszValue = PowerASR_Base_getArgumentValue("FRAME_SHIFT");
	if (pszValue) {
		pMfccVar->nFrameShift = (hci_int16)atoi(pszValue);
	}
	else {
		HCIMSG_WARN("[WAVE2MFCC] FRAME_SHIFT not assigned.\n");
		goto lb_err_wave2mfcc_loadConfig;
	}

	// frame width 
	pszValue = PowerASR_Base_getArgumentValue("FRAME_SIZE");
	if (pszValue) {
		pMfccVar->nFrameSize = (hci_int16)atoi(pszValue);
	}
	else {
		HCIMSG_WARN("[WAVE2MFCC] FRAME_SIZE not assigned.\n");
		goto lb_err_wave2mfcc_loadConfig;
	}

	// FFT size
	pszValue = PowerASR_Base_getArgumentValue("FFT_SIZE");
	if (pszValue) {
		pMfccVar->nFFTSize = (hci_int16)atoi(pszValue);
		pMfccVar->nHalfFFT = (pMfccVar->nFFTSize>>1);
	}
	else {
		HCIMSG_WARN("[WAVE2MFCC] FFT_SIZE not assigned.\n");
		goto lb_err_wave2mfcc_loadConfig;
	}

	// number of filter-bank
	pszValue = PowerASR_Base_getArgumentValue("NUM_FILTER_BANK");
	if (pszValue) {
		pMfccVar->nNumFilters = (hci_int16)atoi(pszValue);
	}
	else {
		HCIMSG_WARN("[WAVE2MFCC] NUM_FILTER_BANK not assigned.\n");
		goto lb_err_wave2mfcc_loadConfig;
	}

	// cepstrum order
	pszValue = PowerASR_Base_getArgumentValue("CEPSTRUM_ORDER");
	if (pszValue) {
		pMfccVar->nNumCepstra = (hci_int16)atoi(pszValue);
	}
	else {
		HCIMSG_WARN("[WAVE2MFCC] CEPSTRUM_ORDER not assigned.\n");
		goto lb_err_wave2mfcc_loadConfig;
	}

	// lower frequency in mel-frequency filter bank
	pszValue = PowerASR_Base_getArgumentValue("LOWER_MEL_FREQ");
	if (pszValue) {
		pMfccVar->nLowerFiltFreq = (hci_int16)atoi(pszValue);
	}
	else {
		HCIMSG_WARN("[WAVE2MFCC] LOWER_MEL_FREQ not assigned.\n");
		goto lb_err_wave2mfcc_loadConfig;
	}

	// upper frequency in mel-frequency filter bank
	pszValue = PowerASR_Base_getArgumentValue("UPPER_MEL_FREQ");
	if (pszValue) {
		pMfccVar->nUpperFiltFreq = (hci_int16)atoi(pszValue);
		if (pMfccVar->nUpperFiltFreq > pMfccVar->nSampleRate) {
			HCIMSG_WARN("[WAVE2MFCC] UPPER_MEL_FREQ > SAMPLE_FREQ_RATE !!\n");
			goto lb_err_wave2mfcc_loadConfig;
		}
	}
	else {
		HCIMSG_WARN("[WAVE2MFCC] UPPER_MEL_FREQ not assigned.\n");
		goto lb_err_wave2mfcc_loadConfig;
	}

	// pre-emphasis coefficient
	pszValue = PowerASR_Base_getArgumentValue("PREEMPHASIS_ALPHA");
	if (pszValue) {
#ifdef FIXED_POINT_FE
		pMfccVar->PreEmphasis = (hci_mfcc16)atoi(pszValue);
#else	// !FIXED_POINT_FE
		pMfccVar->PreEmphasis = (hci_mfcc16)atof(pszValue);
#endif	// #ifdef FIXED_POINT_FE
	}
	else {
		HCIMSG_WARN("[WAVE2MFCC] PREEMPHASIS_ALPHA not assigned.\n");
		goto lb_err_wave2mfcc_loadConfig;
	}

	// weight value for C0 coefficient
	pszValue = PowerASR_Base_getArgumentValue("WEIGHT_C0");
	if (pszValue) {
#ifdef FIXED_POINT_FE
		pMfccVar->weightC0 = (hci_mfcc16)atoi(pszValue);
#else	// !FIXED_POINT_FE
		pMfccVar->weightC0 = (hci_mfcc16)atof(pszValue);
#endif	// #ifdef FIXED_POINT_FE
	}
	else {
		HCIMSG_WARN("[WAVE2MFCC] WEIGHT_C0 not assigned.\n");
		goto lb_err_wave2mfcc_loadConfig;
	}

	// cepstral liftering size
	pszValue = PowerASR_Base_getArgumentValue("CEP_LIFTER_ORDER");
	if (pszValue) {
		pMfccVar->nCepLiftOrder = (hci_int16)atoi(pszValue);
	}
	else {
		HCIMSG_WARN("[WAVE2MFCC] CEP_LIFTER_ORDER not assigned.\n");
		goto lb_err_wave2mfcc_loadConfig;
	}
	
	// bias constant term in entropy computation
	pszValue = PowerASR_Base_getArgumentValue("ENTROPY_BIAS");
	if (pszValue) {
		pMfccVar->biasConstantEntropy = (hci_int16)atoi(pszValue);
	}
	else {
		HCIMSG_WARN("[WAVE2MFCC] ENTROPY_BIAS not assigned.\n");
		goto lb_err_wave2mfcc_loadConfig;
	}

	// flag to use spectral entropy as feature
	pMfccVar->addEntropy = FALSE;
	pszValue = PowerASR_Base_getArgumentValue("DO_USE_SPEC_ENTROPY");
	if (pszValue) {
		if (PowerASR_Base_strnocasecmp(pszValue, "yes") == 0) {
			pMfccVar->addEntropy = TRUE;
		}
	}

	//minLogEnergy, maxLogEnergy, minLogPower 

	PowerASR_Base_closeConfigurations();

	return 0;

lb_err_wave2mfcc_loadConfig:

	PowerASR_Base_closeConfigurations();

	return -1;
}


// end of file
