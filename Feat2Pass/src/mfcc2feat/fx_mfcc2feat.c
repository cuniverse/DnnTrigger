
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
 *	@file	fx_mfcc2feat.c
 *	@ingroup mfcc2feat_src
 *	@date	2007/06/01
 *	@author	AOAIA¤(ijchoi@hcilab.co.kr) (AO)HCILAB(http://www.hcilab.co.kr)
 *	@brief	mfcc-to-feature interface for ASR feature extraction
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "base/hci_type.h"
#include "base/hci_msg.h"
#include "base/hci_macro.h"

#include "basic_op/fixpoint.h"
#include "basic_op/basic_op.h"
#include "wave2mfcc/fx_mfcc_common.h"

#include "mfcc2feat/fx_mfcc2feat.h"
#include "mfcc2feat/fx_cms.h"
#include "mfcc2feat/fx_agc.h"
#include "mfcc2feat/fx_dropframe.h"
#include "mfcc2feat/fx_compressframe.h"
#include "mfcc2feat/fx_delta.h"
#include "mfcc2feat/fx_quantizer.h"

// local functions
#ifdef __cplusplus
extern "C" {
#endif 

/**
 *	copy a new input mfcc vector into MFCC stream pool
 */
HCILAB_PRIVATE hci_int32
_FX_Mfcc2Feat_addMfccStream(Feature_UserData *pFeatureData,		///< (o) feature user data
							hci_mfcc_t *pInMfcc,				///< (i) a new MFCC vector
							FrameClass frame_class);			///< (i) speech/silence class of current frame

/**
 *	live-mode post-processing for feature extraction
 *		- CMS, EN, FC
 */
HCILAB_PRIVATE hci_int32
_FX_Mfcc2Feat_livePostProcessing(MFCC_Stream *pMfccStream,		///< (i/o) pointer to MFCC stream
								 MFCC_Cell *pCurCell,			///< (i/o) current MFCC cell
								 MFCC_Cell *pDelayCell,			///< (i/o) delayed processing MFCC cell
								 Mfcc2FeatParameters *pFXVar);	///< (i) config. struct for mfcc-to-feature converter

/**
 *	live-mode post-processing for feature extraction
 *		- CMS, EN, FC
 */
HCILAB_PRIVATE M2F_Status
_FX_Mfcc2Feat_appendFeatures(Feature_UserData *pFeatureData,	///< (i/o) feature user data
							 MFCC_Cell *pOutCell,				///< (o) output MFCC cell
							 Mfcc2FeatParameters *pFXVar);		///< (i) config. struct for mfcc-to-feature converter

/**
 *	store feature stream for batch-mode post-processing
 */
HCILAB_PRIVATE hci_int32
_FX_Mfcc2Feat_storeUtteranceFeature(Feature_UserData *pFeatureData,		///< (i/o) feature user data
									FrameClass frame_class,				///< (i) frame class (silence/speech/mixed)
									hci_int32 idFrame,					///< (i) frame index
									hci_int16 dimFeat);					///< (i) feature dimension

/**
 *	batch-mode post-processing for feature extraction
 *		- CMS, EN, Quantization
 */
HCILAB_PRIVATE hci_int32
_FX_Mfcc2Feat_batchPostProcessing(Feature_UserData *pFeatureData,	///< (i/o) feature user data
								  Mfcc2FeatParameters *pFXVar);		///< (i) config. struct for mfcc-to-feature converter

#ifdef __cplusplus
}
#endif



/**
 *	convert input MFCC stream into feature vectors.
 *
 *	@return Return one of the following values:
 *		- return M2F_FAIL if Mfcc2Feature operation failed
 *		- return M2F_FALSE if returned outputs don't exist
 *		- return M2F_TRUE if returned outputs exist
 *		- return M2F_RESET if previous returned outputs have to be reset
 *		- return M2F_COMPLETE if Mfcc2Feature operation completed
 */
M2F_Status
FX_Mfcc2Feat_convertMfccStream2FeatureVector(Feature_UserData *pFeatureData,	///< (o) feature user data
											 Mfcc2FeatParameters *pFXVar,		///< (i) config. struct for mfcc-to-feature converter
											 MFCC_UserData *pMfccData)			///< (i) MFCC user data
{
	M2F_Status m2f_state = M2F_FALSE;
	MFCC_Stream *pMfccStream = 0;
	EPD_FrameOut *epd_result = 0;
	MFCC_Cell *pCurCell = 0;
	MFCC_Cell *pOutCell = 0;
	MFCC_Cell *pDropCell = 0;
	MFCC_Cell *pFirstCell = 0;
	MFCC_Cell *pLastCell = 0;
	MFCC_Cell *pLastOutCell = 0;
	MFCC_Cell *pActiveCell = 0;
	hci_mfcc_t *pInMfcc = pMfccData->mfccVec; //  [3/22/2016 kklee] 이부분을 집중적으로 볼것
	hci_int32 cur_mfcc_cell = 0;
	hci_int32 norm_mfcc_cell = 0;
	hci_int32 durSilence = 0;
	hci_int32 nDelay = 0;
	
	MFCC_POOL *pCurMFCCData = 0;

	pMfccStream = &pFeatureData->mfccStream;
	pFirstCell  = pMfccStream->mfccPool;
	pLastCell   = pMfccStream->mfccPool + (pMfccStream->maxLenStream-1);

	pOutCell = pMfccStream->mfccPool + (pMfccStream->nOutputMfccCell+pMfccStream->maxLenStream)%pMfccStream->maxLenStream;

	epd_result = &pFeatureData->epd_result;

	// add a new input mfcc into stream pool
	if (pInMfcc) {
		cur_mfcc_cell = _FX_Mfcc2Feat_addMfccStream(pFeatureData,
													pInMfcc,
													epd_result->frame_class);

		pCurCell    = pMfccStream->mfccPool + cur_mfcc_cell;

		pCurCell->specEntropy = pMfccData->nSpecEntropy;

#ifdef SAVE_LOG
		_FX_Mfcc2Feat_saveInputFeatureCell(pCurCell,
										   pMfccData->nSpecEntropy,
										   pMfccData->avgNoiseEnergy,
										   pMfccData->mfccVec[DIM_CEPSTRUM],
										   epd_result->epd_state);
#endif	// #ifdef SAVE_LOG

		// frame dropping
		if (pFXVar->bSilenceDrop) {
			pDropCell = FX_FrameDrop_dropFrameWithLongSilence(pMfccStream, cur_mfcc_cell, pFXVar->winSilDrop);
			if ( pDropCell ) {
				pCurMFCCData = pFeatureData->mfccHist + (pDropCell->idInFrame % MAX_LEN_FEAT_FRAME);
				pCurMFCCData->bActive = DROP_MFCC;
			}
		}

		// live-mode feature post-processing (CMVN, EN, FC)
		_FX_Mfcc2Feat_livePostProcessing(pMfccStream,
										 pCurCell,
										 pCurCell,
										 pFXVar);
	}
	else {
		pCurCell = 0;
	}

	if ( pCurCell ) {
		pCurMFCCData = pFeatureData->mfccHist + (pCurCell->idInFrame % MAX_LEN_FEAT_FRAME);
		pCurMFCCData->bActive     = pCurCell->bActive;
		pCurMFCCData->bSpeech     = pCurCell->bSpeech;
		pCurMFCCData->frame_class = pCurCell->frame_class;
		memcpy(pCurMFCCData->mfccVec, pMfccData->mfccVec, sizeof(pCurCell->mfccVec));
		pFeatureData->nSizeMfccHist += 1;
	}

	switch(epd_result->epd_state) {
	case UTTER_START:
		break;
	case SIL_SPEECH:
		if (SIL_SPEECH != pMfccStream->epd_state) {
			hci_int32 i = 0;
			m2f_state = M2F_RESET;
			pActiveCell = pMfccStream->mfccPool;
			for (i = 0; i < pMfccStream->maxLenStream; i++, pActiveCell++) {
				pActiveCell->bSpeech = FALSE;
				pFeatureData->mfccHist[(pActiveCell->idInFrame % MAX_LEN_FEAT_FRAME)].bSpeech = FALSE;
			}
			pFeatureData->lenFeatStream  = 0;
		}
		pOutCell = 0;
		break;
	case CORE_SPEECH:
		if (SIL_SPEECH == pMfccStream->epd_state) {		// newly transit to CORE_SPEECH state (starting point)
			hci_int32 i = 0;
			pMfccStream->nLenOutputMfcc  = 0;
			pMfccStream->nLenSkipFrame   = 0;
			pMfccStream->nLenDropFrame   = 0;
			pMfccStream->nLenSpeechFrame = 0;
			pFeatureData->lenFeatStream  = 0;

			pActiveCell = pMfccStream->mfccPool;
			for (i = 0; i < pMfccStream->maxLenStream; i++, pActiveCell++) {
				pActiveCell->bSpeech = FALSE;
				pFeatureData->mfccHist[(pActiveCell->idInFrame % MAX_LEN_FEAT_FRAME)].bSpeech = FALSE;
			}

			// move processing cell to the starting-point mfcc cell
			nDelay = epd_result->nEpdFrame - epd_result->nStartFrame - 1;
			cur_mfcc_cell = (cur_mfcc_cell + pMfccStream->maxLenStream - nDelay)%pMfccStream->maxLenStream;
			pActiveCell = pMfccStream->mfccPool + cur_mfcc_cell;
			norm_mfcc_cell = cur_mfcc_cell;
			pOutCell = pMfccStream->mfccPool + (norm_mfcc_cell + pMfccStream->maxLenStream - pFXVar->nFXDelay)%pMfccStream->maxLenStream;
			pMfccStream->nOutputMfccCell = pMfccStream->nCurrentMfccCell - 1 - pFXVar->nFXDelay;

			while (pActiveCell != pCurCell) {
				pActiveCell->bSpeech = TRUE;
				pFeatureData->mfccHist[(pActiveCell->idInFrame % MAX_LEN_FEAT_FRAME)].bSpeech = TRUE;
				if (pOutCell->bSpeech ) {
					m2f_state = _FX_Mfcc2Feat_appendFeatures(pFeatureData,
															 pOutCell,
															 pFXVar);
					if (pOutCell->bActive==DROP_MFCC) pMfccStream->nLenDropFrame++;
					else if (pOutCell->bActive==SKIP_MFCC) pMfccStream->nLenSkipFrame++;
					pMfccStream->nLenSpeechFrame++;
				}
				if (pActiveCell == pLastCell) pActiveCell = pFirstCell;
				else pActiveCell++;
				if (pOutCell == pLastCell) pOutCell = pFirstCell;
				else pOutCell++;
			}
			pActiveCell->bSpeech = TRUE;
			pFeatureData->mfccHist[(pActiveCell->idInFrame % MAX_LEN_FEAT_FRAME)].bSpeech = TRUE;
			if (pOutCell->bSpeech) {
				m2f_state = _FX_Mfcc2Feat_appendFeatures(pFeatureData,
														 pOutCell,
														 pFXVar);
				if (pOutCell->bActive==DROP_MFCC) pMfccStream->nLenDropFrame++;
				else if (pOutCell->bActive==SKIP_MFCC) pMfccStream->nLenSkipFrame++;
				pMfccStream->nLenSpeechFrame++;
			}
			pMfccStream->nOutputMfccCell += 1;
			if (pFeatureData->lenFeatStream > 0) {
				m2f_state = M2F_TRUE;
			}
			else {
				m2f_state = M2F_FALSE;
			}
		}
		else if (SPEECH_SIL == pMfccStream->epd_state) {	// return to CORE_SPEECH state from SPEECH_SIL state

			// set previous temporary non-speech frames to speech frames
			pActiveCell = pCurCell;
//			while (FALSE == pActiveCell->bSpeech) {
			while ( pActiveCell != pOutCell ) { // (2010-10-30)
				pActiveCell->bSpeech = TRUE;
				if ( pActiveCell->frame_class == SIL_FRAME ) { // ·q￢÷ º??e·? ￠?·q ???e·e ¤a?￥”a. (2010-10-30)
					if (pFXVar->bSilenceDropInSpeech == TRUE) { /// 151118 jyb
						pActiveCell->bActive = DROP_MFCC;
					}
				}
				pFeatureData->mfccHist[(pActiveCell->idInFrame % MAX_LEN_FEAT_FRAME)].bSpeech = TRUE;
				if (pActiveCell == pFirstCell) pActiveCell = pLastCell;
				else pActiveCell--;
			}
			pCurCell->bSpeech = TRUE;
			pFeatureData->mfccHist[(pCurCell->idInFrame % MAX_LEN_FEAT_FRAME)].bSpeech = TRUE;

			// process MFCC cells in temporary silence region
			pLastOutCell = pMfccStream->mfccPool + (cur_mfcc_cell + pMfccStream->maxLenStream - pFXVar->nFXDelay)%pMfccStream->maxLenStream;
			while (pOutCell != pLastOutCell) {
				if (pOutCell->bSpeech) {
					M2F_Status current_m2f = M2F_FALSE;
					current_m2f = _FX_Mfcc2Feat_appendFeatures(pFeatureData,
															   pOutCell,
															   pFXVar);
					if (pOutCell->bActive==DROP_MFCC) pMfccStream->nLenDropFrame++;
					else if (pOutCell->bActive==SKIP_MFCC) pMfccStream->nLenSkipFrame++;
					pMfccStream->nLenSpeechFrame++;
					if (current_m2f == M2F_TRUE) {
						m2f_state = current_m2f;
					}
				}
				if (pOutCell == pLastCell) pOutCell = pFirstCell;
				else pOutCell++;
				pMfccStream->nOutputMfccCell += 1;
			}

			// process current mfcc cell
			if (pOutCell->bSpeech) {
				M2F_Status current_m2f = M2F_FALSE;
				current_m2f = _FX_Mfcc2Feat_appendFeatures(pFeatureData,
														   pOutCell,
														   pFXVar);
				if (pOutCell->bActive==DROP_MFCC) pMfccStream->nLenDropFrame++;
				else if (pOutCell->bActive==SKIP_MFCC) pMfccStream->nLenSkipFrame++;
				pMfccStream->nLenSpeechFrame++;
				if (current_m2f == M2F_TRUE) {
					m2f_state = current_m2f;
				}
			}
			pMfccStream->nOutputMfccCell += 1;
		}
		else {		// self-loop transition
			pCurCell->bSpeech = TRUE;
			pFeatureData->mfccHist[(pCurCell->idInFrame % MAX_LEN_FEAT_FRAME)].bSpeech = TRUE;
			// post-processing of feature extractor
			if (pOutCell->bSpeech) {
				m2f_state = _FX_Mfcc2Feat_appendFeatures(pFeatureData,
														 pOutCell,
														 pFXVar);
				if (pOutCell->bActive==DROP_MFCC) pMfccStream->nLenDropFrame++;
				else if (pOutCell->bActive==SKIP_MFCC) pMfccStream->nLenSkipFrame++;
				pMfccStream->nLenSpeechFrame++;
			}
			pMfccStream->nOutputMfccCell += 1;
		}
		break;
	case SPEECH_SIL:
		if ((epd_result->nEpdFrame-1) <= epd_result->nEndFrame) {
			pCurCell->bSpeech = TRUE;
			pFeatureData->mfccHist[(pCurCell->idInFrame % MAX_LEN_FEAT_FRAME)].bSpeech = TRUE;
			if (pOutCell->bSpeech) {
				m2f_state = _FX_Mfcc2Feat_appendFeatures(pFeatureData,
														 pOutCell,
														 pFXVar);
				if (pOutCell->bActive==DROP_MFCC) pMfccStream->nLenDropFrame++;
				else if (pOutCell->bActive==SKIP_MFCC) pMfccStream->nLenSkipFrame++;
				pMfccStream->nLenSpeechFrame++;
			}
			pMfccStream->nOutputMfccCell += 1;
		}
/*		else if (pOutCell->bSpeech) { // (2010-10-30)
			m2f_state = _FX_Mfcc2Feat_appendFeatures(pFeatureData,
													 pOutCell,
													 pFXVar);
			if (pOutCell->bActive==DROP_MFCC) pMfccStream->nLenDropFrame++;
			else if (pOutCell->bActive==SKIP_MFCC) pMfccStream->nLenSkipFrame++;
			pMfccStream->nLenSpeechFrame++;
			pMfccStream->nOutputMfccCell += 1;
		}*/
		break;
	case UTTER_END:

		// find speech ending MFCC cell
		pCurCell = pMfccStream->mfccPool + pMfccStream->nCurrentMfccCell%pMfccStream->maxLenStream;

		// post-processing for unprocessed speech MFCC cells
		while (pOutCell != pCurCell) {
			if (pOutCell->bSpeech==TRUE) {
				m2f_state = _FX_Mfcc2Feat_appendFeatures(pFeatureData,
														 pOutCell,
														 pFXVar);
				if (pOutCell->bActive==DROP_MFCC) pMfccStream->nLenDropFrame++;
				else if (pOutCell->bActive==SKIP_MFCC) pMfccStream->nLenSkipFrame++;
				pMfccStream->nLenSpeechFrame++;
			}
			if (pOutCell==pLastCell) pOutCell = pFirstCell;
			else pOutCell++;
		}

		m2f_state = M2F_COMPLETE;

		if (0 == pInMfcc && pFXVar->bSilenceDrop) {		// in case of file input, frame dropping in end of file
			if (pOutCell == pFirstCell) pActiveCell = pLastCell;
			else pActiveCell = pOutCell - 1;
			pOutCell = pActiveCell;
			durSilence = 0;
			while (pActiveCell->frame_class == SIL_FRAME && durSilence <= 2*pFXVar->winSilDrop) {
				durSilence++;
				if (pActiveCell == pFirstCell) pActiveCell = pLastCell;
				else pActiveCell--;
			}
			if (durSilence > pFXVar->winSilDrop) {
				hci_int32 i = 0;
				for (i = pFXVar->winSilDrop; i < durSilence; i++) {
					if (ACTIVE_MFCC == pOutCell->bActive && pOutCell->bSpeech) {
//						pFeatureData->lenFeatStream = PowerASR_BasicOP_subtract_16_16(pFeatureData->lenFeatStream, 1);
//						pMfccStream->nLenOutputMfcc = PowerASR_BasicOP_subtract_16_16(pMfccStream->nLenOutputMfcc, 1);
						pFeatureData->lenFeatStream = PowerASR_BasicOP_subtract_32_32(pFeatureData->lenFeatStream, 1) % MAX_LEN_FEAT_FRAME;//yowon 2012-09-01 yowon 2016-07-25
						pMfccStream->nLenOutputMfcc = PowerASR_BasicOP_subtract_32_32(pMfccStream->nLenOutputMfcc, 1) % MAX_LEN_FEAT_FRAME;//yowon 2012-09-01 yowon 2016-07-25

						pOutCell->bActive = DROP_MFCC;
						pFeatureData->mfccHist[(pOutCell->idInFrame % MAX_LEN_FEAT_FRAME)].bActive = DROP_MFCC;
						pMfccStream->nLenDropFrame++;
					}
					if (pOutCell == pFirstCell) pOutCell = pLastCell;
					else pOutCell--;
				}
			}
		}

		FX_CMS_updateCepstralMeanVector(pMfccStream, pFXVar);
		if (pFXVar->typeEN != NO_EN) {
			FX_AGC_updateMaxLogEnergy(pMfccStream, pFXVar->typeEN);
		}
		if (FALSE == pFXVar->bLiveMode) {
			_FX_Mfcc2Feat_batchPostProcessing(pFeatureData, pFXVar);
		}

		break;
	default:
		break;
	}
	pMfccStream->epd_state = epd_result->epd_state;

	return m2f_state;
}


/**
 *	convert input MFCC stream into feature vectors without using EPD data.
 *
 *	@return Return one of the following values:
 *		- return M2F_FAIL if Mfcc2Feature operation failed
 *		- return M2F_FALSE if returned outputs don't exist
 *		- return M2F_TRUE if returned outputs exist
 *		- return M2F_RESET if previous returned outputs have to be reset
 *		- return M2F_COMPLETE if Mfcc2Feature operation completed
 */
M2F_Status
FX_Mfcc2Feat_convertMfccStream2FeatureVectorWithoutEPD(Feature_UserData *pFeatureData,		///< (o) feature user data
													   Mfcc2FeatParameters *pFXVar,			///< (i) config. struct for mfcc-to-feature converter
													   MFCC_UserData *pMfccData)			///< (i) MFCC user data
{
	M2F_Status m2f_state = M2F_FALSE;
	MFCC_Stream *pMfccStream = 0;
	EPD_FrameOut *epd_result = 0;
	MFCC_Cell *pCurCell = 0;
	MFCC_Cell *pOutCell = 0;
	MFCC_Cell *pFirstCell = 0;
	MFCC_Cell *pLastCell = 0;
	MFCC_Cell *pActiveCell = 0;
	hci_mfcc_t *pInMfcc = pMfccData->mfccVec;
	hci_int32 cur_mfcc_cell = 0;
	hci_int32 i = 0;
	hci_int32 durSilence = 0;
	hci_int32 nDelay = 0;
	
	MFCC_POOL *pCurMFCCData = 0;
	int kk;
	pMfccStream = (MFCC_Stream *)&pFeatureData->mfccStream;
	pFirstCell  = (MFCC_Cell *)pMfccStream->mfccPool;
	pLastCell   = pMfccStream->mfccPool + (pMfccStream->maxLenStream-1);

	pOutCell = pMfccStream->mfccPool + (pMfccStream->nOutputMfccCell+pMfccStream->maxLenStream)%pMfccStream->maxLenStream;

	epd_result = &pFeatureData->epd_result;

	// add a new input mfcc into stream pool
	if (epd_result->epd_state != UTTER_END) {
		cur_mfcc_cell = _FX_Mfcc2Feat_addMfccStream(pFeatureData,
													pInMfcc,
													epd_result->frame_class);

		pCurCell = pMfccStream->mfccPool + cur_mfcc_cell;

		// frame dropping
		if (pFXVar->bSilenceDrop) {
			FX_FrameDrop_dropFrameWithLongSilence(pMfccStream, cur_mfcc_cell, pFXVar->winSilDrop);
		}
	}
	else {
		pCurCell = 0;
		pInMfcc = 0;
	}

//////////////////////// yowon test /////////////////////////
#if 0
	if(nFileLength < 300)
	{
        for(kk=0; kk<12; kk++)
                printf("%d : mfcc[%d] = %f\n",nFileLength,kk,pFirstCell[0].mfccVec[kk]);

/*        for(kk=0; kk<12; kk++)
                printf("%d : norm[%d] = %f\n",nFileLength,kk,pFirstCell[0].normVec[kk]);

        for(kk=0; kk<12; kk++)
                printf("%d : feat[%d] = %f\n",nFileLength,kk,pFirstCell[0].featVec[kk]);
*/
	}
#endif
//////////////////////// yowon test /////////////////////////
	if ( pCurCell ) {
		pCurMFCCData = pFeatureData->mfccHist + (pCurCell->idInFrame % MAX_LEN_FEAT_FRAME);
		pCurMFCCData->bActive     = pCurCell->bActive;
		pCurMFCCData->bSpeech     = pCurCell->bSpeech;
		pCurMFCCData->frame_class = pCurCell->frame_class;
		memcpy(pCurMFCCData->mfccVec, pMfccData->mfccVec, sizeof(pCurCell->mfccVec));
		pFeatureData->nSizeMfccHist += 1;
	}


	switch(epd_result->epd_state) {
	case UTTER_START:
	case SIL_SPEECH:
		// initialization
		m2f_state = M2F_RESET;
		FX_CMS_initializeCepstrumMean(pMfccStream, pFXVar->dimMFCC, pFXVar->typeCMS, pFXVar->bCMSwVAD);
		FX_AGC_initializeMaxLogEnergy(pMfccStream, pFXVar->typeEN);
		//pMfccStream->curLenSpeech   = 0;
		pMfccStream->nLenOutputMfcc = 0;
		pFeatureData->lenFeatStream = 0;

		if (pFXVar->bUse3rdDif) {
			nDelay = pFXVar->win3rdDif;
		}
		else if (pFXVar->bUseAcc) {
			nDelay = pFXVar->winAcc;
		}
		else if (pFXVar->bUseDif) {
			nDelay = pFXVar->winDif;
		}
		else {
			nDelay = 0;
		}
		if (pFXVar->bSilenceDrop) {
			nDelay += pFXVar->winSilDrop + 1;
		}
		pMfccStream->nOutputMfccCell = pMfccStream->nCurrentMfccCell - 1 - nDelay;

		pOutCell = 0;

		// post-processing
		pCurCell->bSpeech = TRUE;
		_FX_Mfcc2Feat_livePostProcessing(pMfccStream,
										 pCurCell,
										 pCurCell,
										 pFXVar);

		pCurMFCCData = pFeatureData->mfccHist + (pCurCell->idInFrame % MAX_LEN_FEAT_FRAME);
		pCurMFCCData->bActive     = pCurCell->bActive;
		pCurMFCCData->bSpeech     = pCurCell->bSpeech;

		break;
	case CORE_SPEECH:
		pCurCell->bSpeech = TRUE;
		_FX_Mfcc2Feat_livePostProcessing(pMfccStream,
										 pCurCell,
										 pCurCell,
										 pFXVar);
		
		pCurMFCCData = pFeatureData->mfccHist + (pCurCell->idInFrame % MAX_LEN_FEAT_FRAME);
		pCurMFCCData->bActive     = pCurCell->bActive;
		pCurMFCCData->bSpeech     = pCurCell->bSpeech;

		break;
	case SPEECH_SIL:
	case UTTER_END:
		if (pInMfcc) {
			pCurCell->bSpeech = TRUE;
			_FX_Mfcc2Feat_livePostProcessing(pMfccStream,
											 pCurCell,
											 pCurCell,
											 pFXVar);
		}
		pCurCell = pMfccStream->mfccPool + pMfccStream->nCurrentMfccCell%pMfccStream->maxLenStream;
		while (pOutCell != pCurCell) {
			if (pOutCell->bSpeech==TRUE) {
				m2f_state = _FX_Mfcc2Feat_appendFeatures(pFeatureData,
														 pOutCell,
														 pFXVar);
				pFeatureData->mfccHist[(pOutCell->idInFrame % MAX_LEN_FEAT_FRAME)].bActive = pOutCell->bActive;
			}
			if (pOutCell==pLastCell) pOutCell = pFirstCell;
			else pOutCell++;
		}

		if (pFXVar->bSilenceDrop) {		// in case of file input, frame dropping in end of file
			if (pOutCell == pFirstCell) pActiveCell = pLastCell;
			else pActiveCell = pOutCell - 1;
			pOutCell = pActiveCell;
			durSilence = 0;
			while (pActiveCell->frame_class == SIL_FRAME && durSilence <= 2*pFXVar->winSilDrop) {
				durSilence++;
				if (pActiveCell == pFirstCell) pActiveCell = pLastCell;
				else pActiveCell--;
			}
			if (durSilence > pFXVar->winSilDrop) {
				for (i = pFXVar->winSilDrop; i < durSilence; i++) {
					if (ACTIVE_MFCC == pOutCell->bActive && pOutCell->bSpeech) {
//						pFeatureData->lenFeatStream = PowerASR_BasicOP_subtract_16_16(pFeatureData->lenFeatStream, 1);
//						pMfccStream->nLenOutputMfcc = PowerASR_BasicOP_subtract_16_16(pMfccStream->nLenOutputMfcc, 1);
						pFeatureData->lenFeatStream = PowerASR_BasicOP_subtract_32_32(pFeatureData->lenFeatStream, 1) % MAX_LEN_FEAT_FRAME;//yowon 2012-09-01 yowon 2016-07-25;//yowon 2012-09-01
						pMfccStream->nLenOutputMfcc = PowerASR_BasicOP_subtract_32_32(pMfccStream->nLenOutputMfcc, 1) % MAX_LEN_FEAT_FRAME;//yowon 2012-09-01 yowon 2016-07-25;//yowon 2012-09-01

						pOutCell->bActive = DROP_MFCC;
						pFeatureData->mfccHist[(pOutCell->idInFrame % MAX_LEN_FEAT_FRAME)].bActive = DROP_MFCC;
					}
					if (pOutCell == pFirstCell) pOutCell = pLastCell;
					else pOutCell--;
				}
			}
		}

		pOutCell = 0;
		pCurCell = 0;
		m2f_state = M2F_COMPLETE;

		FX_CMS_updateCepstralMeanVector(pMfccStream, pFXVar);
		if (pFXVar->typeEN != NO_EN) {
			FX_AGC_updateMaxLogEnergy(pMfccStream, pFXVar->typeEN);
		}
		if (FALSE == pFXVar->bLiveMode) {
			_FX_Mfcc2Feat_batchPostProcessing(pFeatureData, pFXVar);
		}
		break;
	default:
		break;
	}
	pMfccStream->epd_state = epd_result->epd_state;

	// extract a feature vector
	if (pOutCell) {
		if (pOutCell->bSpeech==TRUE) {
			m2f_state = FX_DELTA_computeLiveDerivatives(pMfccStream,
														pFeatureData->feat,
														pOutCell,
														pFXVar);
			if (M2F_TRUE == m2f_state) {
				//bgy
				//pFeatureData->featStream.frame_class[pFeatureData->lenFeatStream] = pOutCell->frame_class;
//				pFeatureData->featStream->frame_class[pFeatureData->lenFeatStream] = pOutCell->frame_class;
				pFeatureData->featStream->frame_class[pFeatureData->lenFeatStream%MAX_LEN_FEAT_FRAME] = pOutCell->frame_class;//yowon 2016-07-25
				if (TRUE == pFXVar->bLiveMode) {
					FX_QUANTIZER_quantizeFeature(pFeatureData,
												 &pFXVar->mfccQuantizer,
												 pFXVar->dimFeat);
				}
				else {
					_FX_Mfcc2Feat_storeUtteranceFeature(pFeatureData,
														pOutCell->frame_class,
														pOutCell->idInFrame,
														pFXVar->dimFeat);
					m2f_state = FALSE;
				}
			}
			pFeatureData->mfccHist[(pOutCell->idInFrame % MAX_LEN_FEAT_FRAME)].bActive = pOutCell->bActive;
		}
	}
	pMfccStream->nOutputMfccCell += 1;

	return m2f_state;
}


/**
 *	copy a new input mfcc vector into MFCC stream pool
 */
HCILAB_PRIVATE hci_int32
_FX_Mfcc2Feat_addMfccStream(Feature_UserData *pFeatureData,		///< (o) feature user data
							hci_mfcc_t *pInMfcc,				///< (i) a new MFCC vector
							FrameClass frame_class)				///< (i) speech/silence class of current frame
{
	MFCC_Stream *pMfccStream = 0;
	MFCC_Cell *pMfccPool = 0;
	hci_int32 pos_cell = 0;

	pMfccStream = &pFeatureData->mfccStream;

	pos_cell = pMfccStream->nCurrentMfccCell%pMfccStream->maxLenStream;

	pMfccPool = pMfccStream->mfccPool + pos_cell;

	pMfccPool->bActive     = ACTIVE_MFCC;
	pMfccPool->bSpeech     = FALSE;
	pMfccPool->frame_class = frame_class;
	pMfccPool->idInFrame   = pMfccStream->nCurrentMfccCell;
	//pMfccPool->idOutFrame  = 0;

	memcpy(pMfccPool->mfccVec, pInMfcc, sizeof(pMfccPool->mfccVec));

    pMfccStream->nCurrentMfccCell = PowerASR_BasicOP_add_32_32(pMfccStream->nCurrentMfccCell, 1);

	return pos_cell;
}


/**
 *	live-mode post-processing for feature extraction
 *		- CMS, EN, FC
 */
HCILAB_PRIVATE hci_int32
_FX_Mfcc2Feat_livePostProcessing(MFCC_Stream *pMfccStream,		///< (i/o) pointer to MFCC stream
								 MFCC_Cell *pInCell,			///< (i) current MFCC cell
								 MFCC_Cell *pOutCell,			///< (i/o) delayed processing MFCC cell
								 Mfcc2FeatParameters *pFXVar)	///< (i) config. struct for mfcc-to-feature converter
{
	
	memcpy(pOutCell->normVec, pOutCell->mfccVec, sizeof(pOutCell->mfccVec));

	// live-mode energy normalization
	if (pOutCell &&  NO_EN != pFXVar->typeEN) {
		pOutCell->normVec[pFXVar->dimMFCC-1] = FX_AGC_liveEnergyNormalization(pMfccStream,
																			  pOutCell->mfccVec[pFXVar->dimMFCC-1],
																			  pFXVar->typeEN);
	}

	// live-mode cepstral mean subtraction
	if (NO_CMS != pFXVar->typeCMS) {
		FX_CMS_liveCMS(pMfccStream,
					   pInCell,
					   pOutCell,
					   pFXVar);
	}
	else {
		memcpy(pOutCell->normVec, pOutCell->mfccVec, (size_t)(pFXVar->dimMFCC-1) * sizeof(hci_mfcc_t));
	}

	// frame compression
	if (pOutCell) {
		if (NO_FC != pFXVar->typeFC) {
			FX_FC_compressFrames(pMfccStream,
								 pOutCell,
								 pFXVar->dimMFCC,
								 pFXVar->typeFC,
								 pFXVar->distThresh,
								 pFXVar->nMaxFrameDrop);
		}
		else {
			memcpy(pOutCell->featVec, pOutCell->normVec, sizeof(pOutCell->normVec));
		}
	}

	return 0;
}


/**
 *	live-mode post-processing for feature extraction
 *		- CMS, EN, FC
 */
HCILAB_PRIVATE M2F_Status
_FX_Mfcc2Feat_appendFeatures(Feature_UserData *pFeatureData,	///< (i/o) feature user data
							 MFCC_Cell *pOutCell,				///< (o) output MFCC cell
							 Mfcc2FeatParameters *pFXVar)		///< (i) config. struct for mfcc-to-feature converter
{

	MFCC_Stream *pMfccStream = &pFeatureData->mfccStream;
	M2F_Status m2f_state = M2F_FALSE;


	m2f_state = FX_DELTA_computeLiveDerivatives(pMfccStream,
												pFeatureData->feat,
												pOutCell,
												pFXVar);
	if (M2F_TRUE == m2f_state) {
		//bgy
		//pFeatureData->featStream.frame_class[pFeatureData->lenFeatStream] = pOutCell->frame_class;
//		pFeatureData->featStream->frame_class[pFeatureData->lenFeatStream] = pOutCell->frame_class;
		pFeatureData->featStream->frame_class[pFeatureData->lenFeatStream%MAX_LEN_FEAT_FRAME] = pOutCell->frame_class;//yowon 2016-07-25
		
		if (TRUE == pFXVar->bLiveMode) {
			//////20141021 bykim dnn //////
			/*FX_QUANTIZER_quantizeFeature(pFeatureData,
										 &pFXVar->mfccQuantizer,
										 pFXVar->dimFeat);*/
			
			_FX_Mfcc2Feat_storeUtteranceFeature(pFeatureData,
												pOutCell->frame_class,
												pOutCell->idInFrame,
												pFXVar->dimFeat);
			////////////////////////////////////////////

		}
		else {
			_FX_Mfcc2Feat_storeUtteranceFeature(pFeatureData,
												pOutCell->frame_class,
												pOutCell->idInFrame,
												pFXVar->dimFeat);
			m2f_state = FALSE;
		}
	}

	return m2f_state;
}


/**
 *	batch-mode post-processing for feature extraction
 *		- CMS, EN, Quantization
 */
HCILAB_PRIVATE hci_int32
_FX_Mfcc2Feat_batchPostProcessing(Feature_UserData *pFeatureData,	///< (i/o) feature user data
								  Mfcc2FeatParameters *pFXVar)		///< (i) config. struct for mfcc-to-feature converter
{
	hci_int32 nLenFrame = 0;
	size_t numBytes = 0;
	hci_mfcc_t *pSrcFeatVec = 0;
	hci_mfcc_t *pEndFeatVec = 0;
	FrameClass *frame_class_seq = 0;
	MFCC_Stream *pMfccStream = 0;
	FEAT_Normalizer *pUserFeatNorm = 0;

	nLenFrame = pFeatureData->lenFeatStream;
	pFeatureData->lenFeatStream = 0;

	numBytes = (size_t)pFXVar->dimFeat * sizeof(hci_mfcc_t);

	pSrcFeatVec     = pFeatureData->utterStream.sent_feat;
	pEndFeatVec     = pSrcFeatVec + nLenFrame * pFXVar->dimFeat;
	frame_class_seq = pFeatureData->utterStream.frame_class;
	pMfccStream     = &pFeatureData->mfccStream;
	pUserFeatNorm   = &pMfccStream->userFeatNorm;

	while (pSrcFeatVec < pEndFeatVec) {
		// copy feature vector
		memcpy(pFeatureData->feat, pSrcFeatVec, numBytes);

		// batch-mode cepstral mean subtraction
		FX_CMS_batchCMS(pFeatureData->feat,
						pUserFeatNorm,
						*frame_class_seq,
						pFXVar->dimMFCC,
						pFXVar->typeCMS);

		// batch-mode energy normalization
		if (pFXVar->bUseLogE) {
			pFeatureData->feat[pFXVar->dimMFCC-1] = FX_AGC_batchEnergyNormalization(pFeatureData->feat[pFXVar->dimMFCC-1],
																				    pUserFeatNorm->maxLogE,
																					pUserFeatNorm->minLogE);
		}

		// feature quantization
		FX_QUANTIZER_quantizeFeature(pFeatureData,
									 &pFXVar->mfccQuantizer,
									 pFXVar->dimFeat);

		// move pointers
		pSrcFeatVec += pFXVar->dimFeat;
		frame_class_seq++;
	}

	return 0;
}

//dnn
hci_float32 gMeanStd[] = {
0.011788094119083, 6.985671539687238,
0.009274618149691, 6.134080140874338,
0.000885882241945, 7.194933518141120,
0.005720853709987, 6.699167929498461,
-0.003704474858772, 5.654440518483623,
0.004667110375512, 5.980540621653553,
0.007851290271756, 5.801908733530758,
0.005055553321617, 5.556206194719458,
0.002409495531368, 5.652073181600533,
0.003625395771271, 5.396661157350352,
-0.000432656589299, 5.035599831759122,
0.000119460137260, 4.621328177209318,
-0.018360706068373, 6.720924258270762,
-0.007444288476043, 5.982327224711756,
-0.000775271336871, 6.544174961457808,
-0.000263807190624, 6.516283990716819,
-0.001465846754884, 5.918128282117850,
-0.005560273073402, 6.185774729819999,
-0.005432282386558, 6.247020177721401,
-0.001629876629621, 5.969784211276461,
-0.002265642696697, 6.048822470286223,
-0.002548064384705, 5.795847028517629,
-0.001391946389796, 5.490183782521877,
-0.000586520539613, 5.056868398190331,
-0.010888192772157, 2.850940792582561,
-0.008246285667145, 10.014358132269606,
-0.002167412616330, 9.234332580700956,
-0.001797783286686, 9.960137153390431,
-0.000847176304953, 10.046943047349064,
-0.001864160453820, 9.482222839333875,
-0.001999041094501, 9.888315118318131,
-0.001813156874421, 10.028284026883323,
-0.001384146654162, 9.653445377969785,
-0.001219172665939, 9.760353260438313,
-0.000594945199938, 9.380025585474620,
-0.000575928071913, 8.924209797248663,
-0.000390008295909, 8.272020793528318,
0.002326262274579, 4.251339408260174,
0.001366568128205, 11.582690155544450,
0.000530298345269, 11.137690754446881,
0.001333340006462, 11.980704726403868,
0.001127038523506, 12.134086564494773,
0.000921671157199, 11.958864202740175,
-0.000104466170669, 12.482548671854360,
-0.000237194220510, 12.695693554914659,
-0.000003998249422, 12.359714995470734,
0.000193919678598, 12.472780537409280,
-0.000053403389887, 12.039458330499881,
0.000137361439643, 11.495932588560574,
0.000269456074007, 10.731643419360555,
-0.000602330007656, 4.954663501332441,
}; //20141024 bykim

/**
 *	store feature stream for batch-mode post-processing
 */
HCILAB_PRIVATE hci_int32
_FX_Mfcc2Feat_storeUtteranceFeature(Feature_UserData *pFeatureData,		///< (i/o) feature user data
									FrameClass frame_class,				///< (i) frame class (silence/speech/mixed)
									hci_int32 idFrame,					///< (i) frame index
									hci_int16 dimFeat)					///< (i) feature dimension
{
	hci_int32 startDim = 0;
	hci_mfcc_t *pMfccVec = 0;
	size_t numBytes = 0;
	hci_int16 idx=0;//dnn
    //FILE *fd = fopen("./LOG/feat.txt", "a");
	if (pFeatureData->lenFeatStream >= MAX_LEN_FEAT_FRAME) {
		HCIMSG_WARN("[Warning] MAX_LEN_FEAT_FRAME overflow !!\n");
		return -1;
	}

	startDim = pFeatureData->lenFeatStream * dimFeat;
	pMfccVec = pFeatureData->utterStream.sent_feat + startDim;
	numBytes = (size_t)dimFeat*sizeof(hci_mfcc_t);

#if 1 //yowon 2015-05-21 feature normalize 제거
	memcpy(pMfccVec, pFeatureData->feat, numBytes);
	for(idx=0;idx<dimFeat;idx++){// added 20141024 bykim dnn
//		pMfccVec[idx] = pMfccVec[idx]-gMeanStd[2*idx];
//		pMfccVec[idx] = pMfccVec[idx]/gMeanStd[2*idx+1];
		
		pMfccVec[idx] = pMfccVec[idx];
		//pMfccVec[idx] = pMfccVec[idx];        
        
		//fprintf(fd, "%0.3f ", pMfccVec[idx]);        
    }
    //fprintf(fd, "\n");
    //fclose(fd);
#endif

#if 0// yowon 2015-04-16 no use gpu
	if(B_USE_GPU) DNN_GPU_copy_vector(pFeatureData->utterStream.sent_feat_gpu+startDim, pMfccVec, dimFeat, 0);
#endif

//	pFeatureData->utterStream.frame_class[pFeatureData->lenFeatStream] = frame_class;
//    pFeatureData->lenFeatStream = PowerASR_BasicOP_add_32_32(pFeatureData->lenFeatStream, 1);

	pFeatureData->utterStream.frame_class[pFeatureData->lenFeatStream%MAX_LEN_FEAT_FRAME] = frame_class;//yowon 2016-07-25
	pFeatureData->lenFeatStream = PowerASR_BasicOP_add_32_32(pFeatureData->lenFeatStream, 1) % MAX_LEN_FEAT_FRAME;//yowon 2016-07-25

	return 0;
}

/* end of file */






















