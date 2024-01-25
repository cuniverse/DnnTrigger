
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
 *	@file	fx_compressframe.c
 *	@ingroup mfcc2feat_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	frame compression library
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "basic_op/basic_op.h"
#include "mfcc2feat/fx_compressframe.h"

// local functions
#ifdef __cplusplus
extern "C" {
#endif 

/**
 *	calculate distance between two feature vectors
 */
HCILAB_PRIVATE hci_mfcc32
_FX_FC_calcFeatureDistance(hci_mfcc_t *xfeat,		///< (i) a feature vector, Q15.32
						   hci_mfcc_t *yfeat,		///< (i) a feature vector, Q15.32
						   hci_mfcc_t *wgt,			///< (i) weight vector
						   hci_int16 dimFeat);		///< (i) feature dimension

/**
 *	calculate distance between two feature vectors
 */
HCILAB_PRIVATE hci_mfcc32
_FX_FC_interpolateFeatures(hci_mfcc_t *ofeat,		///< (o) output feature vector, Q15.32
						   hci_mfcc_t *xfeat,		///< (i) a feature vector, Q15.32
						   hci_mfcc_t *yfeat,		///< (i) a feature vector, Q15.32
						   hci_mfcc_t wgtSmooth,	///< (i) interpolation weight, Q15.32
						   hci_int16 dimFeat);		///< (i) feature dimension

#ifdef __cplusplus
}
#endif


/**
 *	compress feature frames
 */
hci_int32
FX_FC_compressFrames(MFCC_Stream *pMfccStream,	///< (i/o) pointer to mfcc stream
					 MFCC_Cell *pCurCell,		///< (i) current MFCC cell
					 const hci_int16 dimMFCC,	///< (i) static MFCC dimension
					 FC_Type typeFC,			///< (i) FC type
					 hci_mfcc32 distThresh,		///< (i) distance threshold in variable frame rate analysis
					 hci_int32 nMaxFrameDrop)	///< (i) count of consecutive frame drops in variable frame rate analysis
{
	MFCC_Cell *pPreCell = 0;
	hci_mfcc32 L_val_dist = 0;
	hci_int16 nFrameCount = 0;
	hci_int16 nModFrame = 0;
	hci_mfcc_t outFeat[DIM_CEPSTRUM+2];
	hci_mfcc_t *pCepWgt = 0;
	hci_mfcc_t smoothWgt = 0;

	pPreCell = pMfccStream->preCell[0];
	nFrameCount = pMfccStream->curLenSpeech;

	switch(typeFC) {
		case FC_AUTO_VFR:	// automatic FC based on variable frame rate analysis (only at odd frames); [NOT VERIFIED]
			if (pPreCell) {
				MFCC_Cell *pActiveCell = 0;
				hci_int32 n = 0;
				pCepWgt = pMfccStream->userFeatNorm.cepVarVoiced;
				for ( n = 0; n < nMaxFrameDrop && pMfccStream->preCell[n] ; n++) {
					if ( pMfccStream->preCell[n]->bActive != SKIP_MFCC ) {
						pActiveCell = pMfccStream->preCell[n];
						break;
					}
				}
				if ( pActiveCell ) {
					hci_flag bFrameDrop = 1;
					hci_int32 nDropFrames = 0;
					hci_int16 nOrder = (hci_int16)(dimMFCC-1);
					for ( n = 0; n < nMaxFrameDrop && pMfccStream->preCell[n] ; n++) {
						L_val_dist = _FX_FC_calcFeatureDistance(pCurCell->mfccVec,
																pMfccStream->preCell[n]->mfccVec,
																pCepWgt,
																nOrder);
						nDropFrames++;
						if ( L_val_dist >= distThresh ) {
							bFrameDrop = 0;
							break;
						}
						if ( pMfccStream->preCell[n] == pActiveCell ) break;
					}
					if ( bFrameDrop ) {
#ifdef FIXED_POINT_FE
						hci_float32 fWgt = 0;
						fWgt = (hci_float32)nDropFrames / (hci_float32)(nDropFrames + 1);
						smoothWgt = FLOAT2FIX32_ANY(fWgt, 15);
#else
						smoothWgt = (hci_mfcc_t)nDropFrames / (hci_mfcc_t)(nDropFrames + 1);
#endif
						_FX_FC_interpolateFeatures( outFeat,
													pActiveCell->normVec,
													pCurCell->normVec,
													smoothWgt,
													dimMFCC);
						memcpy(pActiveCell->featVec, outFeat, sizeof(outFeat));
						pActiveCell->frame_class = HCI_MAX(pActiveCell->frame_class, pCurCell->frame_class);
						pCurCell->bActive = SKIP_MFCC;
					}
					else if ( pPreCell->bActive != SKIP_MFCC ) {
#ifdef FIXED_POINT_FE
						smoothWgt = (1<<14);
#else
						smoothWgt = 0.5f;
#endif
						_FX_FC_interpolateFeatures( outFeat,
													pPreCell->normVec,
													pCurCell->normVec,
													smoothWgt,
													dimMFCC);
						memcpy(pPreCell->featVec, outFeat, sizeof(outFeat));
					}
				}
				else if ( pPreCell->bActive != SKIP_MFCC ) {
#ifdef FIXED_POINT_FE
					smoothWgt = FLOAT2FIX32_ANY(0.5f, 15);
#else
					smoothWgt = 0.5f;
#endif
					_FX_FC_interpolateFeatures( outFeat,
												pPreCell->normVec,
												pCurCell->normVec,
												smoothWgt,
												dimMFCC);
					memcpy(pPreCell->featVec, outFeat, sizeof(outFeat));
				}
				pPreCell = pCurCell;
				for ( n = nMaxFrameDrop - 1; n > 0; n--) {
					pMfccStream->preCell[n] = pMfccStream->preCell[n-1];
				}
			}
			else {
				memcpy(pCurCell->featVec, pCurCell->normVec, sizeof(pCurCell->mfccVec));
				pPreCell = pCurCell;
			}
			break;
		case FC5TO4:		// 20% frame compression (5 frames --> 4 frames : (0,1) --> 0, (1,2) --> 1, (2,3) --> 2, (3,4) --> 3)
			nModFrame = nFrameCount%5;
			if (pPreCell && nModFrame) {
#ifdef FIXED_POINT_FE
				smoothWgt = FLOAT2FIX32_ANY(0.5f, 15);
#else
				smoothWgt = 0.5f;
#endif
				_FX_FC_interpolateFeatures(outFeat,
										   pPreCell->normVec,
										   pCurCell->normVec,
										   smoothWgt,
										   dimMFCC);
				memcpy(pCurCell->featVec, outFeat, sizeof(outFeat));
				if (nModFrame == 1) {
					pCurCell->frame_class = HCI_MAX(pPreCell->frame_class, pCurCell->frame_class);
				}
			}
			else {
				pCurCell->bActive = SKIP_MFCC;
			}
			pPreCell = pCurCell;
			break;
		case FC4TO3:		// 25% frame compression (4 frames --> 3 frames : 0 --> 0, (1,2) --> 1, 3 --> 2)
			nModFrame = nFrameCount%4;
			if (pPreCell && 2 == nModFrame) {
#ifdef FIXED_POINT_FE
				smoothWgt = FLOAT2FIX32_ANY(0.5f, 15);
#else
				smoothWgt = 0.5f;
#endif
				_FX_FC_interpolateFeatures(outFeat,
										   pPreCell->normVec,
										   pCurCell->normVec,
										   smoothWgt,
										   dimMFCC);
				pCurCell->bActive = SKIP_MFCC;
				memcpy(pPreCell->featVec, outFeat, sizeof(outFeat));
				pPreCell->frame_class = HCI_MAX(pPreCell->frame_class, pCurCell->frame_class);
			}
			else {
				memcpy(pCurCell->featVec, pCurCell->normVec, sizeof(pCurCell->mfccVec));
			}
			pPreCell = pCurCell;
			break;
		case FC5TO3:		// 40% frame compression (5 frames --> 3 frames : (0,1) --> 0, 2 --> 1, (3,4) --> 2)
			nModFrame = nFrameCount%5;
			if (pPreCell && (1 == nModFrame || 4 == nModFrame)) {
#ifdef FIXED_POINT_FE
				smoothWgt = FLOAT2FIX32_ANY(0.5f, 15);
#else
				smoothWgt = 0.5f;
#endif
				_FX_FC_interpolateFeatures(outFeat,
										   pPreCell->normVec,
										   pCurCell->normVec,
										   smoothWgt,
										   dimMFCC);
				pCurCell->bActive = SKIP_MFCC;
				memcpy(pPreCell->featVec, outFeat, sizeof(outFeat));
				pPreCell->frame_class = HCI_MAX(pPreCell->frame_class, pCurCell->frame_class);
			}
			else {
				memcpy(pCurCell->featVec, pCurCell->normVec, sizeof(pCurCell->mfccVec));
			}
			pPreCell = pCurCell;
			break;
		case FC2TO1:		// 50% frame compression (2 frames --> 1 frame : (0,1) --> 0)
			nModFrame = nFrameCount%2;
			if (pPreCell && nModFrame) {
#ifdef FIXED_POINT_FE
				smoothWgt = FLOAT2FIX32_ANY(0.5f, 15);
#else
				smoothWgt = 0.5f;
#endif
				_FX_FC_interpolateFeatures(outFeat,
										   pPreCell->normVec,
										   pCurCell->normVec,
										   smoothWgt,
										   dimMFCC);
				pCurCell->bActive = SKIP_MFCC;
				memcpy(pPreCell->featVec, outFeat, sizeof(outFeat));
				pPreCell->frame_class = HCI_MAX(pPreCell->frame_class, pCurCell->frame_class);
			}
			else {
				memcpy(pCurCell->featVec, pCurCell->normVec, sizeof(pCurCell->mfccVec));
			}
			pPreCell = pCurCell;
			break;
		default:			// none FC
			memcpy(pCurCell->featVec, pCurCell->normVec, sizeof(pCurCell->mfccVec));
			pPreCell = pCurCell;
			break;
	}

	pMfccStream->preCell[0] = pPreCell;
//	pMfccStream->curLenSpeech = PowerASR_BasicOP_add_16_16(pMfccStream->curLenSpeech, 1);
	pMfccStream->curLenSpeech = PowerASR_BasicOP_add_32_32(pMfccStream->curLenSpeech, 1);//yowon 2012-09-01

	return 0;
}


/**
 *	calculate distance between two feature vectors
 */
HCILAB_PRIVATE hci_mfcc32
_FX_FC_calcFeatureDistance(hci_mfcc_t *xfeat,		///< (i) a feature vector, Q15.32
						   hci_mfcc_t *yfeat,		///< (i) a feature vector, Q15.32
						   hci_mfcc_t *wgt,			///< (i) weight vector
						   hci_int16 dimFeat)		///< (i) feature dimension
{
	hci_mfcc_t *pX = xfeat;
	hci_mfcc_t *pY = yfeat;
	hci_mfcc_t *pW = wgt;
	hci_mfcc_t *pLast = xfeat + dimFeat;
	hci_mfcc32 dist = 0;
	hci_mfcc32 val_d = 0;

#ifdef FIXED_POINT_FE

	while (pX < pLast) {
		val_d = (*pX++) - (*pY++);									// Q15.32
		val_d = PowerASR_BasicOP_shiftRight_32_32(val_d, 8);		// Q7.32
		dist += HCI_SQ(val_d);										// Q14.32
	}
	dist = PowerASR_BasicOP_shiftRight_32_32(dist, 6);				// Q8.32

#else	// !FIXED_POINT_FE

	while (pX < pLast) {
		val_d = (*pX++) - (*pY++);
		val_d *= *pW++;		// 2010-10-30
		dist += HCI_SQ(val_d);
	}

#endif	// #ifdef FIXED_POINT_FE

	return dist;
}


/**
 *	calculate distance between two feature vectors
 */
HCILAB_PRIVATE hci_mfcc32
_FX_FC_interpolateFeatures(hci_mfcc_t *ofeat,		///< (o) output feature vector, Q15.32
						   hci_mfcc_t *xfeat,		///< (i) a feature vector, Q15.32
						   hci_mfcc_t *yfeat,		///< (i) a feature vector, Q15.32
						   hci_mfcc_t wgtSmooth,	///< (i) interpolation weight, Q15.32
						   hci_int16 dimFeat)		///< (i) feature dimension
{
	hci_mfcc_t *pO = ofeat;
	hci_mfcc_t *pX = xfeat;
	hci_mfcc_t *pY = yfeat;
	hci_mfcc_t *pLast = xfeat + dimFeat;

#ifdef FIXED_POINT_FE

	while (pX+1 < pLast) {
		*pO = PowerASR_BasicOP_add_32_32(*pX, *pY);
		*pO = PowerASR_BasicOP_shiftRight_32_32(*pO, 1);
		pO++; pX++; pY++;
	}
	*pO = HCI_MAX(*pX, *pY);		// log-energy term

#else	// !FIXED_POINT_FE

	while (pX+1 < pLast) {
		*pO = wgtSmooth * (*pX) + (1.0f - wgtSmooth) * (*pY);
		pO++; pX++; pY++;
	}
	*pO = HCI_MAX(*pX, *pY);

#endif	// #ifdef FIXED_POINT_FE

	return 0;
}


/* end of file */






















