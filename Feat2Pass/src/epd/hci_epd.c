
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
 *	@file	hci_epd.c
 *	@ingroup epd_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	PowerASR end-point detector (using frame power / voice metric (sub-band SNRs) / spectral entropy)
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "base/hci_malloc.h"
#include "base/hci_msg.h"
#include "base/parse_config.h"
#include "base/case.h"

#include "epd/epd_main.h"
#include "epd/hci_epd.h"

/** inner data struct for end-point detector */
typedef struct 
{
	EpdParameters paraEpd;		///< EPD parameters
} EPD_Inner;

// local functions

/**
 * setup default environments for end-point detector
 */
HCILAB_PRIVATE hci_int32
_EPD_defaultConfigurations(PowerASR_EPD *pThis);


/**
 * setup environments from a given configuration file
 */
HCILAB_PRIVATE hci_int32
_EPD_loadConfigurations(PowerASR_EPD *pThis,
						const char *pszConfigFile);


/**
 *	create a new end-point detector
 *
 *	@return Return the pointer to a newly created end-point detector
 */
HCILAB_PUBLIC HCI_EPD_API PowerASR_EPD*
PowerASR_EPD_new()
{
	PowerASR_EPD *pEPD = 0;
	EPD_Inner *pInner = 0;

	pEPD = (PowerASR_EPD *) hci_malloc( sizeof(PowerASR_EPD) );

	if ( pEPD ) {
		memset(pEPD, 0, sizeof(PowerASR_EPD));

		pInner = (EPD_Inner *) hci_malloc( sizeof(EPD_Inner) );
		if ( pInner ) {
			memset(pInner, 0, sizeof(EPD_Inner));
			pEPD->pInner = (void *)pInner;
		}
	}
	else {
		HCIMSG_ERROR("cannot create end-point detector.\n");
	}

	return pEPD;
}


/**
 *	delete the end-point detector
 */
HCILAB_PUBLIC HCI_EPD_API void
PowerASR_EPD_delete(PowerASR_EPD *pThis)
{
	EPD_Inner *pInner = 0;

	if (0 == pThis) {
		return;
	}

	pInner = (EPD_Inner *) pThis->pInner;

	if ( pInner ) hci_free(pInner);
	hci_free(pThis);
}


/**
 *	set-up environments for end-point detector,
 *	and allocate necessary memories.
 *
 *	@return Return 0 if EPD environments are set-up correctly, otherwise return -1.
 */
HCILAB_PUBLIC HCI_EPD_API hci_int32
PowerASR_EPD_openEPDetector(PowerASR_EPD *pThis,			///< (i/o) pointer to the end-point detector
							const char *pszConfigFile)		///< (i) EPD configuration file
{
	if (0 == pThis) {
		return -1;
	}

	if (0 == pszConfigFile) {
		return _EPD_defaultConfigurations(pThis);
	}
	else {
		return _EPD_loadConfigurations(pThis, pszConfigFile);
	}
}


/**
 *	free memories allocated to the end-point detector.
 *
 *	@return Return 0 if all memories are freed successfully, otherwise return -1.
 */
HCILAB_PUBLIC HCI_EPD_API hci_int32
PowerASR_EPD_closeEPDetector(PowerASR_EPD *pThis)		///< (i/o) pointer to the end-point detector
{
	return (pThis ? 0 : -1);
}


/**
 *	initialize data buffers for end-point detector.
 *
 *	@return Return 0 if EPD user data are initialized successfully, otherwise return -1.
 */
HCILAB_PUBLIC HCI_EPD_API hci_int32
PowerASR_EPD_initializeEPDetector(PowerASR_EPD *pThis,
								  EPD_UserData *pEpdData,			///< (i/o) EPD user data
								  const hci_flag bContinuousEPD)	///< (i) flag to continuous-mode EPD
{
	EPD_Inner *pInner = 0;
	EpdParameters *pEpdPara = 0;

	if (0 == pThis) {
		return -1;
	}

	pInner = (EPD_Inner *)pThis->pInner;
	pEpdPara = &pInner->paraEpd;

	pEpdPara->bContinuousMode = bContinuousEPD;

	memset(pEpdData, 0, sizeof(EPD_UserData));

	pEpdData->nStartMargin = pEpdPara->nBeginMarginFrame;
	pEpdData->nEndMargin   = pEpdPara->nEndMarginFrame;
	pEpdData->nFrontSkipFrameSize = pEpdPara->nFrontSkipFrameSize;
	pEpdData->bUseDNNEPD = pEpdPara->bUseDNNEPD;
	return 0;
}


/**
 *	Given an input frame buffer, detect end-points of spoken utterance.
 *
 *	@return Return EPD status.
 */
HCILAB_PUBLIC HCI_EPD_API hci_flag
PowerASR_EPD_detectEndPoint(PowerASR_EPD *pThis,
							EPD_UserData *pEpdData,		///< (o) EPD user data
							hci_epd32 frameLogEnergy,	///< (i) frame log energy
							hci_epd16 frameLogPower,	///< (i) frame log power
							hci_epd32 specEntropy,		///< (i) spectral entropy
							hci_int32 flagNRVad,		///< (i) VAD output in noise-reduction module
							hci_int32 bSpeechFound)		///< (i) flag to speech found
{
	EPD_Inner *pInner = 0;

	if (0 == pThis) {
		return EPD_STATUS_SYSTEM_ERROR;
	}

	pInner = (EPD_Inner *) pThis->pInner;

	return EPD_detectSpeechBoundary(pEpdData,
									&pInner->paraEpd,
									specEntropy,
									frameLogEnergy,
									frameLogPower,
									flagNRVad,
									bSpeechFound);
}


/**
 * setup default environments for end-point detector
 */
HCILAB_PRIVATE hci_int32
_EPD_defaultConfigurations(PowerASR_EPD *pThis)
{
	EPD_Inner *pInner = 0;
	EpdParameters *pEpdVar = 0;

	if (0 == pThis) {
		return -1;
	}

	pInner = (EPD_Inner *) pThis->pInner;
	pEpdVar = &pInner->paraEpd;

	pEpdVar->nMaxSpeechFrame = MAX_SPEECH_DUR;
	pEpdVar->nMinSpeechFrame = MIN_SPEECH_DUR;
	pEpdVar->nMaxWaitFrame   = MAX_WAIT_TIME;
	pEpdVar->nMinSpeechPulseDur = MIN_SPEECH_PULSE_DUR;
	//pEpdVar->nMinVoicedFrame = 20;
	pEpdVar->nMinVoicedFrame = 15;
	pEpdVar->nBeginMarginFrame = 10;
	pEpdVar->nEndMarginFrame = 10;
	pEpdVar->bContinuousMode = 1;
	pEpdVar->nInitialFrame = 3;
	pEpdVar->nUselessFrame = 0;
	pEpdVar->nEndPauseFrameTh = 40;
	pEpdVar->nMidPauseFrameTh = 40;
	pEpdVar->nETUCountTh = 3;
	pEpdVar->nFrontSkipFrameSize = 8;
	pEpdVar->bUseDNNEPD = 0;
	//if (pEpdVar->nUselessFrame + pEpdVar->nInitialFrame < pEpdVar->nBeginMarginFrame) {
	//	pEpdVar->nInitialFrame += pEpdVar->nBeginMarginFrame - (pEpdVar->nUselessFrame + pEpdVar->nInitialFrame);
	//}
	if (pEpdVar->nEndPauseFrameTh <= pEpdVar->nEndMarginFrame) {
		pEpdVar->nEndPauseFrameTh = pEpdVar->nEndMarginFrame + 1;
	}

#ifdef FIXED_POINT_FE
	pEpdVar->loudSpeakCheckThresh = 2560;		// Q6.32(40)
	pEpdVar->softSpeakCheckThresh = 160;		// Q6.32(2.5)
	pEpdVar->constSNRTem = 12330;				// Q16.32, = (10/16) * (ln2/ln10)
#else	// !FIXED_POINT_FE
	pEpdVar->loudSpeakCheckThresh = 40.0f;
	pEpdVar->softSpeakCheckThresh = 2.5f;
	pEpdVar->constSNRTem = (hci_epd32)(10.0 * log(2.0));
	pEpdVar->constSNRTem /= (hci_epd32)(16.0 * log(10.0));
#endif	// #ifdef FIXED_POINT_FE

	return 0;
}


/**
 * setup environments from a given configuration file
 */
HCILAB_PRIVATE hci_int32
_EPD_loadConfigurations(PowerASR_EPD *pThis,
						const char *pszConfigFile)
{
	EPD_Inner *pInner = 0;
	EpdParameters *pEpdVar = 0;
	char *pszValue = 0;
	double threshSNR = 0;

	if (0 == pThis) {
		return -1;
	}

	pInner = (EPD_Inner *) pThis->pInner;
	pEpdVar = &pInner->paraEpd;

	if (0 != PowerASR_Base_parseConfigFile(pszConfigFile)) {
		HCIMSG_ERROR("parseConfigFile failed (%s).\n", pszConfigFile);
		return -1;
	}

	// maximum speech duration in frame count
	pszValue = PowerASR_Base_getArgumentValue("MAX_SPEECH_DUR");
	if (pszValue) {
		pEpdVar->nMaxSpeechFrame = (hci_int16)atoi(pszValue);
	}
	else {
		pEpdVar->nMaxSpeechFrame = MAX_SPEECH_DUR;
	}

	// minimum speech duration in frame count
	pszValue = PowerASR_Base_getArgumentValue("MIN_SPEECH_DUR");
	if (pszValue) {
		pEpdVar->nMinSpeechFrame = (hci_int16)atoi(pszValue);
	}
	else {
		pEpdVar->nMinSpeechFrame = MIN_SPEECH_DUR;
	}

	// maximum waiting time in frame count
	pszValue = PowerASR_Base_getArgumentValue("MAX_WAIT_TIME");
	if (pszValue) {
		pEpdVar->nMaxWaitFrame = (hci_int16)atoi(pszValue);
	}
	else {
		pEpdVar->nMaxWaitFrame   = MAX_WAIT_TIME;
	}

	// minimum total count of voiced frames
	pszValue = PowerASR_Base_getArgumentValue("VOICED_FRAME_COUNT_TH");
	if (pszValue) {
		pEpdVar->nMinVoicedFrame = (hci_int16)atoi(pszValue);
	}
	else {
		pEpdVar->nMinVoicedFrame = 15;
	}

	// minimum ETU duration in frame count
	pszValue = PowerASR_Base_getArgumentValue("MIN_ETU_COUNT");
	if (pszValue) {
		pEpdVar->nETUCountTh = (hci_int16)atoi(pszValue);
	}
	else {
		pEpdVar->nETUCountTh = 3;
	}

	// margin frames in starting point
	pszValue = PowerASR_Base_getArgumentValue("START_MARGIN");
	if (pszValue) {
		pEpdVar->nBeginMarginFrame = (hci_int16)atoi(pszValue);
	}
	else {
		pEpdVar->nBeginMarginFrame = 20;
	}

	// margin frames in ending point
	pszValue = PowerASR_Base_getArgumentValue("END_MARGIN");
	if (pszValue) {
		pEpdVar->nEndMarginFrame = (hci_int16)atoi(pszValue);
	}
	else {
		pEpdVar->nEndMarginFrame = 20;
	}
	
	// window length of median filter
	pszValue = PowerASR_Base_getArgumentValue("SIZE_MEDIAN_FILTER");
	if (pszValue) {
		pEpdVar->nSizeMedianFilter = (hci_int16)atoi(pszValue);
		if ( pEpdVar->nSizeMedianFilter > LEN_MEDIAN ) {
			pEpdVar->nSizeMedianFilter = LEN_MEDIAN;
		}
	}
	else {
		pEpdVar->nSizeMedianFilter = LEN_MEDIAN;
	}

	// flag to indicate continuous-mode EPD
	pszValue = PowerASR_Base_getArgumentValue("CONTINUOUS_MODE");
	if (pszValue) {
		if (0 == PowerASR_Base_strnocasecmp(pszValue,"yes")) {
			pEpdVar->bContinuousMode = 1;
		}
		else {
			pEpdVar->bContinuousMode = 0;
		}
	}
	else {
		pEpdVar->bContinuousMode = 1;
	}

	// number of initial frames to estimate EPD threshold
	pszValue = PowerASR_Base_getArgumentValue("NUM_INITIAL_FRAME");
	if (pszValue) {
		pEpdVar->nInitialFrame = (hci_int16)atoi(pszValue);
	}
	else {
		pEpdVar->nInitialFrame = 10;
	}

	// number of use-less frames discarded by EPD module
	pszValue = PowerASR_Base_getArgumentValue("NUM_USELESS_FRAME");
	if (pszValue) {
		pEpdVar->nUselessFrame = (hci_int16)atoi(pszValue);
	}
	else {
		pEpdVar->nUselessFrame = 0;
	}

	// number of pause frames to detect ending point of utterance
	pszValue = PowerASR_Base_getArgumentValue("END_PAUSE_FRAME_TH");
	if (pszValue) {
		pEpdVar->nEndPauseFrameTh = (hci_int16)atoi(pszValue);
	}
	else {
		pEpdVar->nEndPauseFrameTh = 40;
	}

	// number of pause frames to reset starting point of speech
	pszValue = PowerASR_Base_getArgumentValue("RESET_PAUSE_FRAME_TH");
	if (pszValue) {
		pEpdVar->nMidPauseFrameTh = (hci_int16)atoi(pszValue);
	}
	else {
		pEpdVar->nMidPauseFrameTh = 15;
	}

	// lower SNR threshold to identify the softness of utterance
	pszValue = PowerASR_Base_getArgumentValue("LOW_SNR_TH");
	if (pszValue) {
#ifdef FIXED_POINT_FE
		threshSNR = atof(pszValue);
		pEpdVar->softSpeakCheckThresh = (hci_epd32)(64*threshSNR);
#else	// !FIXED_POINT_FE
		pEpdVar->softSpeakCheckThresh = (hci_epd32)atof(pszValue);
#endif	// #ifdef FIXED_POINT_FE
	}
	else {
		threshSNR = 2.5;
#ifdef FIXED_POINT_FE
		pEpdVar->softSpeakCheckThresh = (hci_epd32)(64*threshSNR);
#else	// !FIXED_POINT_FE
		pEpdVar->softSpeakCheckThresh = (hci_epd32)threshSNR;
#endif	// #ifdef FIXED_POINT_FE
	}

	// upper SNR threshold to identify the loudness of utterance
	pszValue = PowerASR_Base_getArgumentValue("HIGH_SNR_TH");
	if (pszValue) {
#ifdef FIXED_POINT_FE
		threshSNR = atof(pszValue);
		pEpdVar->loudSpeakCheckThresh = (hci_epd32)(64*threshSNR);
#else	// !FIXED_POINT_FE
		pEpdVar->loudSpeakCheckThresh = (hci_epd32)atof(pszValue);
#endif	// #ifdef FIXED_POINT_FE
	}
	else {
		threshSNR = 40.0;
#ifdef FIXED_POINT_FE
		pEpdVar->loudSpeakCheckThresh = (hci_epd32)(64*threshSNR);
#else	// !FIXED_POINT_FE
		pEpdVar->loudSpeakCheckThresh = (hci_epd32)threshSNR;
#endif	// #ifdef FIXED_POINT_FE
	}



	// EPD Start Point Skip Frame Count Configure
	pszValue = PowerASR_Base_getArgumentValue("NUM_FRONT_SKIP_FRAME");
	if (pszValue) {
		pEpdVar->nFrontSkipFrameSize = (hci_int32)atoi(pszValue);
	}
	else {
		pEpdVar->nFrontSkipFrameSize = 8;
	}


	// DNN EPD Use Flag Configure
	pszValue = PowerASR_Base_getArgumentValue("USE_DNN_EPD");
	if (PowerASR_Base_strnocasecmp(pszValue, "yes") == 0) {
		pEpdVar->bUseDNNEPD = 1;
	} 
	else {
		pEpdVar->bUseDNNEPD = 0;
	}

	PowerASR_Base_closeConfigurations();

// 	if(pEpdVar->nUselessFrame + pEpdVar->nInitialFrame < pEpdVar->nBeginMarginFrame) {
// 		pEpdVar->nInitialFrame += pEpdVar->nBeginMarginFrame - (pEpdVar->nUselessFrame + pEpdVar->nInitialFrame);
// 	}

	if (pEpdVar->nEndPauseFrameTh <= pEpdVar->nEndMarginFrame) {
		pEpdVar->nEndPauseFrameTh = pEpdVar->nEndMarginFrame + 1;
	}

#ifdef FIXED_POINT_FE
	pEpdVar->constSNRTem = 12330;				// Q16.32, = (10/16) * (ln2/ln10)
#else	// !FIXED_POINT_FE
	pEpdVar->constSNRTem = (hci_epd32)(10.0 * log(2.0));
	pEpdVar->constSNRTem /= (hci_epd32)(16.0 * log(10.0));
#endif	// #ifdef FIXED_POINT_FE


	


	return 0;
}


// end of file
