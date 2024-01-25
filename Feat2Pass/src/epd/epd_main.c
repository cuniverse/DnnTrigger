
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
 *	@file	epd_main.c
 *	@ingroup epd_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	Main modules for PowerASR end-point detector
 */

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>

#include "base/hci_macro.h"
#include "basic_op/basic_op.h"
#include "epd/epd_main.h"

/** Log-energy boundaries */
#ifdef FIXED_POINT_FE
#define TH_E1	(524288)			// Q15.32
#define TH_E2	(589824)			// Q15.32
#define TH_E3	(655360)			// Q15.32
#else	// !FIXED_POINT_FE
#define TH_E1	((hci_mfcc32)16)
#define TH_E2	((hci_mfcc32)18)
#define TH_E3	((hci_mfcc32)20)
#endif	// #ifdef FIXED_POINT_FE

// local functions
#ifdef __cplusplus
extern "C" {
#endif
	
/**
 *  detect end-of-utterance
 */
HCILAB_PRIVATE hci_flag
_EPD_detectEndOfUtterance(EPD_UserData *pEpdData,	///< (i/o) : epd data struct for each user (channel)
						  EpdParameters *pEpdVar,	///< (i) : epd configuration variables
						  hci_int32 bSpeechFound,	///< (i) : flag to speech found
						  hci_int32 epdVAD,			///< (i) : VAD frame class (NON_SPEECH/UNVOICED/MIXED/VOICED)
						  hci_epd32 specEntropy,	///< (i) : spectral entropy
						  hci_int32 nEpdFrameId);	///< (i) : epd frame index

/**
 *  estimate EPD threshold values
 */
HCILAB_PRIVATE hci_int32
_EPD_estimateEpdThreshold(EPD_UserData *pEpdData,		///< (i/o) : pointer to EPD data per user (channel)
						  hci_epd32 specEntropy,		///< (i) : spectral entropy (Q15.32)
						  hci_epd32 frameLogEnergy,		///< (i) : log frame energy (Q15.32)
						  hci_epd16 frameLogPower);		///< (i) : log frame power (Q0.16)

/**
 *  get re-estimated VAD flag
 */
HCILAB_PRIVATE hci_int32
_EPD_estimateFrameClass(EPD_UserData *pEpdData,			///< (i/o) : pointer to EPD data per user (channel)
						hci_epd32 specEntropy,			///< (i) : spectral entropy (Q15.32)
						hci_epd32 frameLogEnergy,		///< (i) : log frame energy (Q15.32)
						hci_epd16 frameLogPower,		///< (i) : log frame power (Q0.16)
						hci_int32 flagNRVad,			///< (i) : VAD output in noise-reduction module
						hci_int32 bSpeechFound);		///< (i) flag to speech found

/**
 *  accumulate average speech/noise log-energy to estimate SNR
 */
HCILAB_PRIVATE void
_EPD_accumulateSNRStatistics(EPD_UserData *pEpdData,		///< (i/o) : pointer to EPD data per user (channel)
							 hci_epd32 specEntropy,			///< (i) : spectral entropy (Q15.32)
							 hci_epd16 frameLogPower,		///< (i) : log frame power (Q0.16)
							 hci_int32 epdVAD);				///< (i) : VAD output in noise-reduction module

/**
 *  estimate SNR of current utterance
 */
HCILAB_PRIVATE hci_int32
_EPD_checkLoudnessOfUtterance(EPD_UserData *pEpdData,	///< (i/o) : epd data struct for each user (channel)
							  EpdParameters *pEpdVar);	///< (i) : epd configuration variables

/**
 *  median filtering
 */
HCILAB_PRIVATE hci_epd32
_EPD_applyMedianFilter(hci_epd32 *windowBuf,
					   hci_int16 nNumSample,
					   hci_int16 nLenWindow);

#ifdef __cplusplus
};
#endif


/**
 *  main function to detect start/end points of spoken utterance
 */
hci_flag
EPD_detectSpeechBoundary(EPD_UserData *pEpdData,		///< (i/o) : pointer to EPD data per user (channel)
						 EpdParameters *pEpdVar,		///< (i) : EPD config. variables
						 hci_epd32 specEntropy,			///< (i) : spectral entropy (Q15.32)
						 hci_epd32 frameLogEnergy,		///< (i) : log frame energy (Q15.32)
						 hci_epd16 frameLogPower,		///< (i) : log frame power (Q0.16)
						 hci_int32 flagNRVad,			///< (i) : VAD output in noise-reduction module
						 hci_int32 bSpeechFound)		///< (i) flag to speech found
{
	hci_int32 nFrameId = 0;
	hci_int32 epdVAD = 0;
	hci_flag epd_status = 0;
	hci_epd32 diffLogE = 0;
	hci_epd32 diffLogP = 0;

	pEpdData->nPrevState = pEpdData->nCurrentState;
	pEpdData->bDetectStartPt = pEpdData->bDetectEndPt = FALSE;
	pEpdData->bResetStartPt = FALSE;

	// compensate spectral entropy value
	diffLogE = frameLogEnergy - pEpdData->avgNoiseEnergy;
	diffLogP = frameLogPower - pEpdData->avgNoisePower;
	if ( bSpeechFound == 0 || flagNRVad == SIL_FRAME ) {
		specEntropy = 0.01f;
	}
	else if ( pEpdData->nEpdFrame > 20 && pEpdData->nCurrentState > SIL_SPEECH &&
		(diffLogE > 4.0f || diffLogP > 4.0f ) && 
		flagNRVad >= MIXED_FRAME && pEpdData->nNumPause < 30 ) {
		if ( pEpdData->nCurrentState == SPEECH_SIL && pEpdData->nNumPause >= 20 ) {
			if ( flagNRVad == VOICED_FRAME && bSpeechFound && specEntropy > 0.03f ) {
				specEntropy *= (1.0f + diffLogE / 8.0f);
			}
			else {
				specEntropy *= (1.0f + diffLogE / 40.0f);
			}
		}
		else {
			if ( diffLogE > 4.0f && diffLogP > 4.0f && bSpeechFound && flagNRVad == VOICED_FRAME && specEntropy > 0.02f ) {
				float wgt = 0;
				wgt = 1.0f + HCI_MAX( diffLogE/4.0f, diffLogP / 4.0f );
				specEntropy *= wgt;
			}
			else {
				float wgt = 0;
				wgt = 1.0f + HCI_MAX( diffLogE/8.0f, diffLogP / 8.0f );
				specEntropy *= wgt;
			}
		}
	}

	if ( bSpeechFound && pEpdData->nCurrentState==SPEECH_SIL && flagNRVad != VOICED_FRAME &&
		(specEntropy < 0.02f || flagNRVad < MIXED_FRAME) ) {
		bSpeechFound = 0;
	}

	// compensate VAD output
	epdVAD = _EPD_estimateFrameClass( pEpdData, specEntropy, frameLogEnergy, frameLogPower, flagNRVad, bSpeechFound );

	// accumulate average speech/noise log-energy to estimate SNR
	_EPD_accumulateSNRStatistics( pEpdData, specEntropy, frameLogPower, epdVAD );

	// estimate EPD thresholds
	_EPD_estimateEpdThreshold( pEpdData, specEntropy, frameLogEnergy, frameLogPower );

	// fill epd measure buffer for median filtering
	nFrameId = HCI_MIN(pEpdData->sizeBuf, pEpdVar->nSizeMedianFilter-1);
	pEpdData->recentValue[nFrameId] = specEntropy;
	pEpdData->sizeBuf = HCI_MIN(pEpdData->sizeBuf+1, pEpdVar->nSizeMedianFilter);

	// apply median filter to remove impulse-like frames
	pEpdData->medianValue = _EPD_applyMedianFilter(pEpdData->recentValue, 
									pEpdData->sizeBuf,
									pEpdVar->nSizeMedianFilter);

	// estimate count of consecutive voiced frames, then re-adjust spectral entropy
	if ( pEpdData->medianValue > pEpdData->upperVadThresh ) pEpdData->nCountVoiced += 1;
	else pEpdData->nCountVoiced = 0;

	if ( epdVAD == VOICED_FRAME || specEntropy > 0.2f || 
		(pEpdData->medianValue > 0.05f && diffLogE > 2.0f && diffLogP > 2.0f && bSpeechFound)) {
		pEpdData->curVoicedDur += 1;
		pEpdData->maxVoicedDur = HCI_MAX(pEpdData->maxVoicedDur, pEpdData->curVoicedDur);
		pEpdData->maxEndVoiceDur = HCI_MAX(pEpdData->maxEndVoiceDur, pEpdData->curVoicedDur);
	}
	else pEpdData->curVoicedDur = 0;

	// set frame-class & speech flag
	pEpdData->frame_class = (FrameClass)epdVAD;
	pEpdData->bFoundSpeech = bSpeechFound;
	pEpdData->maxSpecEntropy = HCI_MAX(pEpdData->maxSpecEntropy, pEpdData->medianValue);
	pEpdData->entropy[pEpdData->nEpdFrame%SIZE_VAD_BUFFER] = pEpdData->medianValue;
	pEpdData->vadHist[pEpdData->nEpdFrame%SIZE_VAD_BUFFER] = epdVAD;
	diffLogE = frameLogEnergy - pEpdData->avgNoiseEnergy;
	if ( pEpdData->nEpdFrame < 8 || (specEntropy <= 0.01f && diffLogE < 1.0f) ) {
		pEpdData->bNonSpeechFrame = 1;
		pEpdData->frame_class = SIL_FRAME;
	}
	else {
		pEpdData->bNonSpeechFrame = 0;
		if ( pEpdData->frame_class == SIL_FRAME && (specEntropy > 0.03f || diffLogE > 2.0f) ) {
			pEpdData->frame_class = UNVOICED_FRAME;
		}
	}

	nFrameId = PowerASR_BasicOP_add_32_32(pEpdData->nEpdFrame, 1);

	// detect end of utterance
	epd_status = _EPD_detectEndOfUtterance(pEpdData, pEpdVar, bSpeechFound, epdVAD, specEntropy, nFrameId);

	pEpdData->nEpdFrame = PowerASR_BasicOP_add_32_32(pEpdData->nEpdFrame, 1);

	return epd_status;
}


/**
 *  compare two values
 */
HCILAB_PRIVATE hci_int32
_EPD_compare_value(const void *_x, const void *_y)
{
	const hci_epd32 *x = (const hci_epd32*)_x;
	const hci_epd32 *y = (const hci_epd32*)_y;

	if ((*x) > (*y)) return 1;
	else if ((*x) < (*y)) return -1;
	else return 0;
}

/**
 *  median filtering
 */
HCILAB_PRIVATE hci_epd32
_EPD_applyMedianFilter(hci_epd32 *windowBuf,
					   hci_int16 nNumSample,
					   hci_int16 nLenWindow)
{
	hci_epd32 *pBuf = 0;
	hci_epd32 *pLastBuf = 0;
	hci_epd32 sortedBuf[LEN_MEDIAN];
	hci_epd32 medianValue = 0;
	hci_int16 nMid = 0;
	hci_int16 sizeBuf = 0;

	if (nNumSample < nLenWindow) {
		return windowBuf[nNumSample-1];
	}

	memcpy(sortedBuf, windowBuf, nNumSample * sizeof(hci_epd32));

	sizeBuf = HCI_MIN(nNumSample, nLenWindow);

	// sorting
	qsort((char *)sortedBuf, (size_t)sizeBuf, sizeof(hci_epd32), _EPD_compare_value);

	// pick up median value
	nMid = (sizeBuf>>1);
	medianValue = sortedBuf[nMid];

	// update buffer
	pLastBuf = windowBuf + nLenWindow;
	pBuf = windowBuf + 1;
	while (pBuf < pLastBuf) {
		*(pBuf-1) = *pBuf;
		pBuf++;
	}

	return medianValue;
}


/**
 *  detect end-of-utterance
 */
HCILAB_PRIVATE hci_int32
_EPD_detectEndOfUtterance(EPD_UserData *pEpdData,	///< (i/o) : epd data struct for each user (channel)
						  EpdParameters *pEpdVar,	///< (i) : epd configuration variables
						  hci_int32 bSpeechFound,	///< (i) : flag to speech found
						  hci_int32 epdVAD,			///< (i) : VAD frame class (NON_SPEECH/UNVOICED/MIXED/VOICED)
						  hci_epd32 specEntropy,	///< (i) : spectral entropy
						  hci_int32 nEpdFrameId)	///< (i) : epd frame index
{
#if 1 //yowon
	hci_epd32 SpeechOnsertEntropy = (hci_epd32)(0.10);
#else //org
	hci_epd32 SpeechOnsertEntropy = (hci_epd32)(0.15);
#endif
	hci_epd32 voicedEntropy = (hci_epd32)(0.2);
	hci_epd32 mixedEntropy = (hci_epd32)(0.1);
	hci_epd32 minSpeechEntropy = (hci_epd32)(0.05);
	int iFrm = 0, iBuf = 0;
	int iStartFrm = 0, iEndFrm = 0;

	switch (pEpdData->nCurrentState) {

		case UTTER_START:	// start-of-utterance state
			pEpdData->nCurrentState = SIL_SPEECH;
			break;

		case SIL_SPEECH:	// starting silence state
			if ( pEpdData->nCountVoiced >= pEpdVar->nETUCountTh ) {
					pEpdData->nCurrentState   = CORE_SPEECH;
				    pEpdData->nRealBeginPt    = nEpdFrameId - pEpdVar->nETUCountTh - 1;
				    pEpdData->nStartFrame     = HCI_MAX(pEpdData->nRealBeginPt - pEpdData->nStartMargin, 0);
					pEpdData->nRealEndPt      = 0;
					pEpdData->nEndFrame       = 0;
				pEpdData->nSpeechDur      = pEpdVar->nETUCountTh;
					pEpdData->nNumPause       = 0;
					pEpdData->bSpeechStarted  = TRUE;
					pEpdData->nNumVoiceFrame  = 0;
					pEpdData->bDetectStartPt  = TRUE;
				pEpdData->maxEndVoiceDur  = 0;
					for ( iFrm = pEpdData->nStartFrame; iFrm < nEpdFrameId; iFrm++) {
						iBuf = iFrm%SIZE_VAD_BUFFER;
					if ( pEpdData->entropy[iBuf] >= SpeechOnsertEntropy ) pEpdData->nNumVoiceFrame++;
				}
			}
			break;

		case CORE_SPEECH:	// center speech state
			if ( pEpdData->medianValue < pEpdData->lowerVadThresh ) {
				pEpdData->nCurrentState   = SPEECH_SIL;
				pEpdData->nRealEndPt      = nEpdFrameId;
				pEpdData->nEndFrame       = pEpdData->nRealEndPt + pEpdData->nEndMargin;
				pEpdData->nCountEndUpdate = 5;
				pEpdData->nNumPause       = 1;
				pEpdData->maxEndVoiceDur  = 0;
				/*
				if ( pEpdData->nSpeechDur > 20 && pEpdData->maxVoicedDur <= 3 ) {//org pEpdData->nSpeechDur 20
					pEpdData->nCurrentState   = SIL_SPEECH;
					pEpdData->nRealBeginPt    = 0;
					pEpdData->nStartFrame     = 0;
					pEpdData->nRealEndPt      = 0;
					pEpdData->nEndFrame       = 0;
					pEpdData->bSpeechStarted  = FALSE;
					pEpdData->bResetStartPt   = TRUE;
					pEpdData->nNumPause		  = 0;	
					pEpdData->nNumVoiceFrame  = 0;
					pEpdData->nSpeechDur      = 0;
					pEpdData->maxVoicedDur    = 0;
				}
				else if ( (pEpdData->nEndFrame-pEpdData->nStartFrame) > 30 && pEpdData->nSpeechDur >= 5 && pEpdData->maxVoicedDur <= (pEpdData->nEndFrame-pEpdData->nStartFrame)/10 ) {
					pEpdData->nCurrentState   = SIL_SPEECH;
					pEpdData->nRealBeginPt    = 0;
					pEpdData->nStartFrame     = 0;
					pEpdData->nRealEndPt      = 0;
					pEpdData->nEndFrame       = 0;
					pEpdData->bSpeechStarted  = FALSE;
					pEpdData->bResetStartPt   = TRUE;
					pEpdData->nNumPause       = 0;
					pEpdData->nNumVoiceFrame  = 0;
					pEpdData->nSpeechDur      = 0;
					pEpdData->maxVoicedDur    = 0;
				}
				else if ( pEpdData->nSpeechDur <= 3 && pEpdData->maxVoicedDur < voicedEntropy && pEpdData->maxVoicedDur <= 3 ) {
					pEpdData->nCurrentState   = SIL_SPEECH;
					pEpdData->nRealBeginPt    = 0;
					pEpdData->nStartFrame     = 0;
					pEpdData->nRealEndPt      = 0;
					pEpdData->nEndFrame       = 0;
					pEpdData->bSpeechStarted  = FALSE;
					pEpdData->bResetStartPt   = TRUE;
					pEpdData->nNumPause		  = 0;	
					pEpdData->nNumVoiceFrame  = 0;
					pEpdData->nSpeechDur      = 0;
					pEpdData->maxVoicedDur    = 0;
				}*/
			}
			else {
				if ( specEntropy > pEpdData->upperVadThresh ) pEpdData->nSpeechDur++;
				if ( specEntropy > SpeechOnsertEntropy ) pEpdData->nNumVoiceFrame++;
			}
			break;

		case SPEECH_SIL:	// ending silence state

			if ( specEntropy > pEpdData->upperVadThresh ) pEpdData->nSpeechDur++;
			if ( specEntropy > SpeechOnsertEntropy ) pEpdData->nNumVoiceFrame++;
#if 1//yowon
			else pEpdData->nNumPause++;
#else //org
			pEpdData->nNumPause++;
#endif

			if ( pEpdData->nNumPause > pEpdVar->nEndPauseFrameTh)			// reach likely end-of-utterance !!
			{   
				//if (pEpdData->nSpeechDur < pEpdVar->nMinSpeechFrame ||		// too short speech duration
				//	pEpdData->nNumVoiceFrame < pEpdVar->nMinVoicedFrame)// || 	// too small count of reliable voiced frames
				//	//pEpdData->maxVoicedDur < pEpdVar->nMinVoicedFrame )
                if ((pEpdData->nRealEndPt - pEpdData->nRealBeginPt - 20) < pEpdVar->nMinSpeechFrame ||
                    (pEpdData->nRealEndPt - pEpdData->nRealBeginPt - 20) < pEpdVar->nMinVoicedFrame) // kklee 2015-12-18
				{
					pEpdData->nCurrentState   = SIL_SPEECH;
					pEpdData->nRealBeginPt    = 0;
					pEpdData->nStartFrame     = 0;
					pEpdData->nRealEndPt      = 0;
					pEpdData->nEndFrame       = 0;
					pEpdData->bSpeechStarted  = FALSE;
					pEpdData->bResetStartPt   = TRUE;
					pEpdData->nNumPause		  = 0;	
					pEpdData->nNumVoiceFrame  = 0;
					pEpdData->nSpeechDur      = 0;
					pEpdData->maxVoicedDur    = 0;
				}
				else {														// reach verified end-of-utterance !!
					pEpdData->nCurrentState = UTTER_END;
					pEpdData->nEndFrame     = pEpdData->nRealEndPt + pEpdData->nEndMargin;
					pEpdData->nNumUtter     = PowerASR_BasicOP_add_32_32(pEpdData->nNumUtter, 1);
					pEpdData->bDetectEndPt  = TRUE;
				}
			}
			else 
			{
//				if ( pEpdData->nCountVoiced >= pEpdVar->nETUCountTh ) {	// return to speech mode
//				if ( pEpdData->nCountVoiced >= pEpdVar->nETUCountTh && pEpdData->maxEndVoiceDur >= pEpdVar->nETUCountTh ) {	// return to speech mode
				if ( pEpdData->nCountVoiced >= pEpdVar->nETUCountTh || pEpdData->maxEndVoiceDur >= pEpdVar->nETUCountTh ) {	// return to speech mode
					hci_epd32 onsetSpecEntropy = 0;
					iStartFrm = nEpdFrameId - 1;
					//iEndFrm = HCI_MAX(0, nEpdFrameId - pEpdData->nCountVoiced);
					iEndFrm = HCI_MAX(0, nEpdFrameId - pEpdVar->nETUCountTh);
					onsetSpecEntropy = 0;
					for ( iFrm = iStartFrm; iFrm >= iEndFrm; iFrm--) {
						iBuf = iFrm % SIZE_VAD_BUFFER;
						onsetSpecEntropy = HCI_MAX(onsetSpecEntropy, pEpdData->entropy[iBuf]);
					}
					if ( onsetSpecEntropy > voicedEntropy ) {
						if ( pEpdData->nNumPause < pEpdVar->nMidPauseFrameTh ) {
						    pEpdData->nCurrentState   = CORE_SPEECH;
						    pEpdData->nRealEndPt      = 0;
						    pEpdData->nEndFrame       = 0;
						    pEpdData->nNumPause       = 0;
					     }
					     //else if ( pEpdData->nCountVoiced >= 2 * pEpdVar->nETUCountTh ) {
						else if ( pEpdData->nCountVoiced >=  pEpdVar->nETUCountTh ) {
							pEpdData->nCurrentState   = CORE_SPEECH;
							pEpdData->nRealEndPt      = 0;
							pEpdData->nEndFrame       = 0;
							pEpdData->nNumPause       = 0;
				         }
				     }
				     else if ( onsetSpecEntropy >= SpeechOnsertEntropy && pEpdData->nNumPause < pEpdVar->nMidPauseFrameTh ) {
						 pEpdData->nCurrentState   = CORE_SPEECH;
						 pEpdData->nRealEndPt      = 0;
						 pEpdData->nEndFrame       = 0;
						 pEpdData->nNumPause       = 0;
				     }
				     else if ( pEpdData->nCountVoiced >= 5 && pEpdData->nNumPause < pEpdVar->nMidPauseFrameTh ) {
						 pEpdData->nCurrentState   = CORE_SPEECH;
						 pEpdData->nRealEndPt      = 0;
						 pEpdData->nEndFrame       = 0;
						 pEpdData->nNumPause       = 0;
				     }
			      }/*
				  else if ( pEpdData->nNumPause > pEpdVar->nMidPauseFrameTh )
				  {
					 if (pEpdData->nSpeechDur < pEpdVar->nMinSpeechFrame ||		// too short speech duration
						pEpdData->nNumVoiceFrame < pEpdVar->nMinVoicedFrame || 	// too small count of reliable voiced frames
						pEpdData->maxVoicedDur < 5 )
					 {
						pEpdData->nCurrentState   = SIL_SPEECH;
						pEpdData->nRealBeginPt    = 0;
						pEpdData->nStartFrame     = 0;
						pEpdData->nRealEndPt      = 0;
						pEpdData->nEndFrame       = 0;
						pEpdData->bSpeechStarted  = FALSE;
						pEpdData->bResetStartPt   = TRUE;
						pEpdData->nNumPause		  = 0;	
						pEpdData->nNumVoiceFrame  = 0;
						pEpdData->nSpeechDur      = 0;
						pEpdData->maxVoicedDur    = 0;
					}
				  }
				  else if ( pEpdData->nNumPause >= 20 && pEpdData->maxVoicedDur <= pEpdVar->nETUCountTh )
				  {
						pEpdData->nCurrentState   = SIL_SPEECH;
						pEpdData->nRealBeginPt    = 0;
						pEpdData->nStartFrame     = 0;
						pEpdData->nRealEndPt      = 0;
						pEpdData->nEndFrame       = 0;
						pEpdData->bSpeechStarted  = FALSE;
						pEpdData->bResetStartPt   = TRUE;
						pEpdData->nNumPause		  = 0;	
						pEpdData->nNumVoiceFrame  = 0;
						pEpdData->nSpeechDur      = 0;
						pEpdData->maxVoicedDur    = 0;
				 }*/
			}
			break;

		case UTTER_END:		// end-of-utterance state
			// re-initialize for continuous-mode end-point detection
			pEpdData->nCurrentState   = SIL_SPEECH;
			if (pEpdVar->bContinuousMode == FALSE) {
				pEpdData->nEpdFrame   = 0;
			}
			pEpdData->nStartFrame     = 0;
			pEpdData->nEndFrame       = 0;
			pEpdData->nSpeechDur      = 0;
			pEpdData->nNumVoiceFrame  = 0;
			pEpdData->nLenSpeech      = 0;
			pEpdData->nLenSilence     = 0;
			pEpdData->speechEn        = 0;
			pEpdData->noiseEn         = 0;
			pEpdData->nCountVoiced    = 0;
			pEpdData->bSpeechStarted  = FALSE;
			pEpdData->bResetStartPt   = TRUE;
			pEpdData->nStartMargin    = pEpdVar->nBeginMarginFrame;
			pEpdData->nEndMargin      = pEpdVar->nEndMarginFrame;
			break;

		default:
			break;

	}

	if (pEpdData->nCurrentState == UTTER_END) {
		return EPD_STATUS_DETECTED;
	}
	else {
		return EPD_STATUS_NOT_DETECTED;
	}
}



/**
 *  estimate EPD threshold values
 */
HCILAB_PRIVATE hci_int32
_EPD_estimateEpdThreshold(EPD_UserData *pEpdData,		///< (i/o) : pointer to EPD data per user (channel)
						  hci_epd32 specEntropy,		///< (i) : spectral entropy (Q15.32)
						  hci_epd32 frameLogEnergy,		///< (i) : log frame energy (Q15.32)
						  hci_epd16 frameLogPower)		///< (i) : log frame power (Q0.16)
{
	hci_int32 nInitNoiseFrame = 8;	//12;
	hci_epd32 threshEnergy = 0;
	hci_epd32 threshPower = 0;
	hci_epd32 lowerEntropyTh = 0;
	hci_epd32 upperEntropyTh = 0;
	hci_epd32 minNoiseEntropy = (hci_epd32)(0.01);
	hci_epd32 maxNoiseEntropy = (hci_epd32)(0.015);
	hci_epd32 minNoiseLogE = 13.0f;	// 15.0f;
	hci_epd32 maxNoiseLogE = 20.0f;	// 19.0f
	hci_epd32 minNoiseLogP = 9.0f;	// 10.0f;
	hci_epd32 diffLogE = 0;
	hci_epd16 diffLogP = 0;

	if (pEpdData->nEpdFrame < nInitNoiseFrame) {
		if ( pEpdData->nEpdFrame == 0 ) {
			pEpdData->avgNoiseEnergy = frameLogEnergy;
			pEpdData->avgNoisePower  = frameLogPower;
		}
		else if ( specEntropy < maxNoiseEntropy && (frameLogEnergy-pEpdData->avgNoiseEnergy) < 3.0f ) {
			pEpdData->avgNoiseEnergy = HCI_MAX( pEpdData->avgNoiseEnergy, frameLogEnergy );
			pEpdData->avgNoisePower = HCI_MAX( pEpdData->avgNoisePower, frameLogPower );
		}
		pEpdData->avgNoiseEnergy = HCI_MAX(pEpdData->avgNoiseEnergy, minNoiseLogE);
		pEpdData->avgNoiseEnergy = HCI_MIN(pEpdData->avgNoiseEnergy, maxNoiseLogE);
		pEpdData->avgNoisePower = HCI_MAX(pEpdData->avgNoisePower, minNoiseLogP);
		lowerEntropyTh = 0.04f;
		upperEntropyTh = 0.9f;
	}
	else if ( pEpdData->nCurrentState == SPEECH_SIL ) { 
		if (pEpdData->avgNoiseEnergy < TH_E1) {
			threshEnergy   = 2.0f;
			lowerEntropyTh = 0.05f;
			upperEntropyTh = 0.10f;
		}
		else if (pEpdData->avgNoiseEnergy < TH_E2) {
			threshEnergy   = 1.5f;
			lowerEntropyTh = 0.04f;
			upperEntropyTh = 0.08f;
		}
		else if (pEpdData->avgNoiseEnergy < TH_E3) {
			threshEnergy   = 1.2f;
			lowerEntropyTh = 0.03f;
			upperEntropyTh = 0.06f;
		}
		else {
			threshEnergy   = 1.0f;
			lowerEntropyTh = 0.02f;
			upperEntropyTh = 0.05f;
		}
		threshPower = threshEnergy;
		if ( specEntropy < maxNoiseEntropy ) {
			diffLogE = frameLogEnergy - pEpdData->avgNoiseEnergy;
			diffLogP = frameLogPower - pEpdData->avgNoisePower;
			if ( diffLogP < threshPower && diffLogE < threshEnergy && diffLogE > 0 )
			{
				pEpdData->avgNoiseEnergy += diffLogE * 0.03f;
				pEpdData->avgNoisePower += diffLogP * 0.03f;
			}
			else if ( diffLogE < 0 ) 
			{
				pEpdData->avgNoiseEnergy += diffLogE * 0.01f;
				pEpdData->avgNoisePower += diffLogP * 0.01f;
			}
		}
		pEpdData->avgNoiseEnergy = HCI_MAX(pEpdData->avgNoiseEnergy, minNoiseLogE);
		pEpdData->avgNoiseEnergy = HCI_MIN(pEpdData->avgNoiseEnergy, maxNoiseLogE);
		pEpdData->avgNoisePower = HCI_MAX(pEpdData->avgNoisePower, minNoiseLogP);
	}
	else {
		if (pEpdData->avgNoiseEnergy < TH_E1) {
			threshEnergy   = 2.0f;
			lowerEntropyTh = 0.03f;
			upperEntropyTh = 0.06f;
		}
		else if (pEpdData->avgNoiseEnergy < TH_E2) {
			threshEnergy   = 1.5f;
			lowerEntropyTh = 0.02f;
			upperEntropyTh = 0.05f;
		}
		else if (pEpdData->avgNoiseEnergy < TH_E3) {
			threshEnergy   = 1.2f;
			lowerEntropyTh = 0.02f;
			upperEntropyTh = 0.04f;
		}
		else {
			threshEnergy   = 1.0f;
			lowerEntropyTh = 0.01f;
			upperEntropyTh = 0.03f;
		}
		threshPower = threshEnergy;
		if ( specEntropy < maxNoiseEntropy ) {
			diffLogE = frameLogEnergy - pEpdData->avgNoiseEnergy;
			diffLogP = frameLogPower - pEpdData->avgNoisePower;
			if ( diffLogP < threshPower && diffLogE < threshEnergy && diffLogE > 0 )
			{
				pEpdData->avgNoiseEnergy += diffLogE * 0.03f;
				pEpdData->avgNoisePower += diffLogP * 0.03f;
			}
			else if ( diffLogE < 0 ) 
			{
				pEpdData->avgNoiseEnergy += diffLogE * 0.01f;
				pEpdData->avgNoisePower += diffLogP * 0.01f;
			}
		}
		pEpdData->avgNoiseEnergy = HCI_MAX(pEpdData->avgNoiseEnergy, minNoiseLogE);
		pEpdData->avgNoiseEnergy = HCI_MIN(pEpdData->avgNoiseEnergy, maxNoiseLogE);
		pEpdData->avgNoisePower = HCI_MAX(pEpdData->avgNoisePower, minNoiseLogP);
	}

	// update EPD upper threshold
	pEpdData->lowerVadThresh = minNoiseEntropy + lowerEntropyTh;
	pEpdData->upperVadThresh = minNoiseEntropy + upperEntropyTh;

	return 0;
}


/**
 *  get re-estimated VAD flag
 */
HCILAB_PRIVATE hci_int32
_EPD_estimateFrameClass(EPD_UserData *pEpdData,			///< (i/o) : pointer to EPD data per user (channel)
						hci_epd32 specEntropy,			///< (i) : spectral entropy (Q15.32)
						hci_epd32 frameLogEnergy,		///< (i) : log frame energy (Q15.32)
						hci_epd16 frameLogPower,		///< (i) : log frame power (Q0.16)
						hci_int32 flagNRVad,			///< (i) : VAD output in noise-reduction module
						hci_int32 bSpeechFound)			///< (i) flag to speech found
{
	hci_int32 epdVAD = 0;
	hci_epd32 avgNoisePower = 0;
	hci_epd32 threshNoisePower = 120.0f;
	hci_epd32 minNoiseEntropy =(hci_epd32)(0.01);	//(hci_epd32)(0.000031);
	hci_epd32 minSpeechEntropy = (hci_epd32)(0.05);
	hci_epd32 threshSpeechEntropy = (hci_epd32)(0.2);
	hci_epd32 SpeechOnsetEntropy = (hci_epd32)(0.25);
	hci_epd32 verifiedSpeechEntropy = (hci_epd32)(0.3);
	hci_epd32 threshLogEnergy = (hci_epd32)(20.0f);		//(hci_epd32)(17.0f);

	// compensate VAD output
	epdVAD = flagNRVad;

	switch (pEpdData->nCurrentState) {

		case UTTER_START:	// start-of-utterance state
			epdVAD = 0;
			break;

		case SIL_SPEECH:	// starting silence state
			if ( pEpdData->nLenSilence >= 4 ) avgNoisePower = pEpdData->noiseEn/(hci_epd32)pEpdData->nLenSilence;
			else avgNoisePower = threshNoisePower;
			if ( bSpeechFound ) {
				if ( avgNoisePower >= threshNoisePower ) {
					if ( specEntropy >= SpeechOnsetEntropy && frameLogEnergy >= threshLogEnergy ) epdVAD = HCI_MIN(epdVAD+1, VOICED_FRAME);
				}
				else {
					if ( specEntropy >= threshSpeechEntropy && frameLogEnergy >= threshLogEnergy ) epdVAD = VOICED_FRAME;
				}
			}
			else if ( flagNRVad ) {
				if ( specEntropy <= minNoiseEntropy) epdVAD = SIL_FRAME;
				else if ( specEntropy < minSpeechEntropy ) epdVAD = UNVOICED_FRAME;
				else if ( specEntropy < 0.1f ) epdVAD -= 1;
				epdVAD = HCI_MIN(epdVAD, MIXED_FRAME);
			}
			break;

		case CORE_SPEECH:	// center speech state
			if ( flagNRVad && specEntropy <= minNoiseEntropy ) epdVAD -= 1;
			if ( flagNRVad && specEntropy >= threshSpeechEntropy && frameLogEnergy >= threshLogEnergy ) epdVAD = HCI_MIN(epdVAD+1, 3);
			break;

		case SPEECH_SIL:	// ending silence state
			avgNoisePower = pEpdData->noiseEn/(hci_epd32)pEpdData->nLenSilence;
			if ( flagNRVad ) {
				if ( bSpeechFound ) {
					if ( avgNoisePower > threshNoisePower ) {
						if ( flagNRVad >= MIXED_FRAME && frameLogEnergy >= (threshLogEnergy + 2.0f) && specEntropy > minSpeechEntropy ) {
							epdVAD = VOICED_FRAME;
						}
						else if ( frameLogEnergy >= threshLogEnergy && specEntropy >= verifiedSpeechEntropy ) epdVAD = HCI_MIN(epdVAD+1, VOICED_FRAME);
						else if ( specEntropy <= minNoiseEntropy ) epdVAD -= 1;
						else if ( specEntropy < minSpeechEntropy && pEpdData->nNumPause >= 3 ) epdVAD -= 1;					
						else if ( flagNRVad==VOICED_FRAME && specEntropy < 3 * minSpeechEntropy ) epdVAD -= 1;
						else if ( !bSpeechFound && specEntropy < 0.02f && frameLogEnergy < 18.0f ) epdVAD -= 1;
					}
					else {
						if ( specEntropy <= minNoiseEntropy && frameLogEnergy < 18.0f ) epdVAD -= 1;
						else if ( frameLogEnergy >= threshLogEnergy || specEntropy >= threshSpeechEntropy ) epdVAD = HCI_MIN(epdVAD+1, VOICED_FRAME);
						else if ( !bSpeechFound && specEntropy < 0.02f && frameLogEnergy < 18.0f ) epdVAD -= 1;
					}
				}
				else {
					if ( specEntropy <= minNoiseEntropy) epdVAD = SIL_FRAME;
					else if ( specEntropy < minSpeechEntropy ) epdVAD = UNVOICED_FRAME;
					else if ( specEntropy < 0.1f ) epdVAD -= 1;
				}
			}
			break;

		default:
			break;

	}

	return epdVAD;
}


/**
 *  accumulate average speech/noise log-energy to estimate SNR
 */
HCILAB_PRIVATE void
_EPD_accumulateSNRStatistics(EPD_UserData *pEpdData,		///< (i/o) : pointer to EPD data per user (channel)
							 hci_epd32 specEntropy,			///< (i) : spectral entropy (Q15.32)
							 hci_epd16 frameLogPower,		///< (i) : log frame power (Q0.16)
							 hci_int32 epdVAD)				///< (i) : VAD output in noise-reduction module
{
	hci_epd32 minSpeechEntropy = (hci_epd32)(0.05);

	if ( epdVAD >= MIXED_FRAME || specEntropy > minSpeechEntropy ) {
		pEpdData->nLenSpeech = PowerASR_BasicOP_add_32_32(pEpdData->nLenSpeech, 1);
		pEpdData->speechEn += frameLogPower;
		if (pEpdData->nLenSpeech == 512) {	// to prevent overflow
#ifdef FIXED_POINT_FE
			pEpdData->speechEn >>= 1;
#else	// !FIXED_POINT_FE
			pEpdData->speechEn *= 0.5f;
#endif	// #ifdef FIXED_POINT_FE
			pEpdData->nLenSpeech >>= 1;
		}
	}
	else {
		pEpdData->nLenSilence = PowerASR_BasicOP_add_32_32(pEpdData->nLenSilence, 1);
		pEpdData->noiseEn += frameLogPower;
		if (pEpdData->nLenSilence == 512) {	// to prevent overflow
#ifdef FIXED_POINT_FE
			pEpdData->noiseEn >>= 1;
#else	// !FIXED_POINT_FE
			pEpdData->noiseEn *= 0.5f;
#endif	// #ifdef FIXED_POINT_FE
			pEpdData->nLenSilence >>= 1;
		}
	}

}

/**
 *  estimate SNR of current utterance
 *		- SNR = 10 * log_10 (Es/En)
 */
HCILAB_PRIVATE hci_int32
_EPD_checkLoudnessOfUtterance(EPD_UserData *pEpdData,	///< (i/o) : epd data struct for each user (channel)
							  EpdParameters *pEpdVar)	///< (i) : epd configuration variables
{
	hci_epd32 avgNoiseEn = 0;
	hci_epd32 avgSpeechEn = 0;
	hci_epd32 constTerm = 0;
	hci_epd32 val_SNR = 0;

	if (0 == pEpdData->nLenSilence) {
		return EPD_STATUS_LOW_FAIL;
	}

#ifdef FIXED_POINT_FE
	avgNoiseEn = PowerASR_BasicOP_divideShiftLeft_32_32(pEpdData->noiseEn,(hci_int32)pEpdData->nLenSilence,0);		// = (16/ln2) * ln(En), Q6.32
	avgSpeechEn = PowerASR_BasicOP_divideShiftLeft_32_32(pEpdData->speechEn,(hci_int32)pEpdData->nLenSpeech,0);		//  = (16/ln2) * ln(Es), Q6.32
	val_SNR = pEpdVar->constSNRTem * (avgSpeechEn - avgNoiseEn);	// Q22.32
	val_SNR = PowerASR_BasicOP_shiftRight_32_32(val_SNR, 16);		// Q6.32
	pEpdData->valueSNR = (hci_epd32)(val_SNR>>6);
#else	// !FIXED_POINT_FE
	avgNoiseEn = pEpdData->noiseEn/(hci_epd32)pEpdData->nLenSilence;		// = (16/ln2) * ln(En)
	avgSpeechEn = pEpdData->speechEn/(hci_epd32)pEpdData->nLenSpeech;		//  = (16/ln2) * ln(Es)
	val_SNR = pEpdVar->constSNRTem * (avgSpeechEn - avgNoiseEn);
	pEpdData->valueSNR = (hci_epd32)val_SNR;
#endif	// #ifdef FIXED_POINT_FE

	if (val_SNR < pEpdVar->softSpeakCheckThresh) {
		return EPD_STATUS_LOW_FAIL;
	}
// 	else if (val_SNR > pEpdVar->loudSpeakCheckThresh) {
// 		return EPD_STATUS_HIGH_FAIL;
// 	}
	else {
		return EPD_STATUS_DETECTED;
	}
}


// end of file
