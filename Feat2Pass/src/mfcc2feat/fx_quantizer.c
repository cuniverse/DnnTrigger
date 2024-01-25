
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
 *	@file	fx_quantizer.c
 *	@ingroup mfcc2feat_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	Feature Quantization library
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "base/hci_msg.h"
#include "basic_op/basic_op.h"
#include "mfcc2feat/fx_quantizer.h"

//#define QUANT_ASCII_INPUT	1		// if feature quantizer data are saved in binary format, don't define QUANT_ASCII_INPUT

// local functions
#ifdef __cplusplus
extern "C" {
#endif 

/**
 *	quantize feature vector
 */
HCILAB_PRIVATE hci_int32
_FX_QUANTIZER_normalizeFeature(hci_mfcc_t *pFeatVec,		///< (i/o) original/output feature vector
							   hci_mfcc_t *quantMean,		///< (i) normalization mean vector
							   hci_mfcc_t *quantInvStdev,	///< (i) normalization variance vector
							   hci_int16 dimFeat);			///< (i) feature dimension

/**
 *	find quantization level of input feature
 */
HCILAB_PRIVATE hci_uint8
_FX_QUANTIZER_findQuantizationLevel(hci_mfcc_t inputFeat,			///< (i) input feature value
									hci_mfcc_t *quantLevelMfcc,		///< (i) quantization boundary vector
									hci_int32 numMfccQuantLevel);	///< (i) quantization level count

#ifdef __cplusplus
}
#endif

/**
 *	load feature quantizer data
 */
hci_int32
FX_QUANTIZER_loadFeatureQuantizer(const char *szMfccQuantizerFile,	///< (i) feature quantizer file
								  FEAT_Quantizer *mfccQuantizer,	///< (o) feature quantizer
								  hci_int16 dimFeat)				///< (i) feature dimension
{
	FILE *fpQuant = 0;

#ifdef QUANT_ASCII_INPUT
	hci_int16 i = 0;
	char szTemp[2][256];

	if ((fpQuant=fopen(szMfccQuantizerFile,"r")) == 0) {
		HCIMSG_ERROR("cannot open FeatQuantizer file (%s).\n", szMfccQuantizerFile);
		return -1;
	}

	// load shift value vector to normalize feature vector
	fscanf(fpQuant, "%s%s", szTemp[0], szTemp[1]);
	for (i = 0; i < dimFeat; i++) {
#ifdef FIXED_POINT_FE
		fscanf(fpQuant, "%d", &(mfccQuantizer->quantMean[i]));
#else	// !FIXED_POINT_FE
		fscanf(fpQuant, "%f", &(mfccQuantizer->quantMean[i]));
#endif	// #ifdef FIXED_POINT_FE
	}

	// load scale factor vector to normalize feature vector
	fscanf(fpQuant, "%s%s", szTemp[0], szTemp[1]);
	for (i = 0; i < dimFeat; i++) {
#ifdef FIXED_POINT_FE
		fscanf(fpQuant, "%d", &(mfccQuantizer->quantInvStdev[i]));
#else	// !FIXED_POINT_FE
		fscanf(fpQuant, "%f", &(mfccQuantizer->quantInvStdev[i]));
#endif	// #ifdef FIXED_POINT_FE
	}

	// load number of feature quantization level
	fscanf(fpQuant, "%s%s%d", szTemp[0], szTemp[1], &(mfccQuantizer->numMfccQuantLevel));

	// load feature quantization level values
	fscanf(fpQuant, "%s%s", szTemp[0], szTemp[1]);
	for (i = 0; i < mfccQuantizer->numMfccQuantLevel-1; i++) {
#ifdef FIXED_POINT_FE
		fscanf(fpQuant, "%d", &(mfccQuantizer->quantLevelMfcc[i]));
#else	// !FIXED_POINT_FE
		fscanf(fpQuant, "%f", &(mfccQuantizer->quantLevelMfcc[i]));
#endif	// #ifdef FIXED_POINT_FE
	}

	fclose(fpQuant);

#else	// !QUANT_ASCII_INPUT

	if ((fpQuant=fopen(szMfccQuantizerFile,"rb")) == 0) {
		HCIMSG_ERROR("cannot open FeatQuantizer file (%s).\n", szMfccQuantizerFile);
		return -1;
	}

	// load shift value vector to normalize feature vector
	fread(mfccQuantizer->quantMean, sizeof(hci_mfcc_t), (size_t)dimFeat, fpQuant);

	// load scale factor vector to normalize feature vector
	fread(mfccQuantizer->quantInvStdev, sizeof(hci_mfcc_t), (size_t)dimFeat, fpQuant);

	// load number of feature quantization level
	fread(&(mfccQuantizer->numMfccQuantLevel), sizeof(hci_int32), 1, fpQuant);

	// load feature quantization level values
	fread(mfccQuantizer->quantLevelMfcc, sizeof(hci_mfcc_t), (size_t)(mfccQuantizer->numMfccQuantLevel-1), fpQuant);

	fclose(fpQuant);

#endif	// #ifdef QUANT_ASCII_INPUT

	return 0;
}


/**
 *	quantize feature vector
 */
hci_int32
FX_QUANTIZER_quantizeFeature(Feature_UserData *pFeatureData,	///< (o) feature user data
							 FEAT_Quantizer *mfccQuantizer,		///< (i) feature quantizer
							 hci_int16 dimFeat)					///< (i) feature dimension
{
	hci_int32 startDim = 0;
	hci_mfcc_t *pSrcFeatVec = 0;
	hci_mfcc_t *pEndFeatVec = 0;
	hci_asr_feat_t *pOutFeatVec = 0;

	if (pFeatureData->lenFeatStream >= MAX_LEN_FEAT_FRAME) {
		HCIMSG_WARN("[Warning] MAX_LEN_FEAT_FRAME overflow !!\n");
		return -1;
	}

	pSrcFeatVec = pFeatureData->feat;
	pEndFeatVec = pSrcFeatVec + dimFeat;

	if (sizeof(hci_asr_feat_t) == 1) {

		// feature normalization
		_FX_QUANTIZER_normalizeFeature(pSrcFeatVec,
									   mfccQuantizer->quantMean,
									   mfccQuantizer->quantInvStdev,
									   dimFeat);

		// feature quantization
		startDim = pFeatureData->lenFeatStream * dimFeat;
		//bgy
		//pOutFeatVec = pFeatureData->featStream.asr_feat + startDim;
		pOutFeatVec = pFeatureData->featStream->asr_feat + startDim;
		while (pSrcFeatVec < pEndFeatVec) {
			*pOutFeatVec = _FX_QUANTIZER_findQuantizationLevel(*pSrcFeatVec,
				 												mfccQuantizer->quantLevelMfcc,
 																mfccQuantizer->numMfccQuantLevel);
			pSrcFeatVec++;
			pOutFeatVec++;
		}
	}
	else {
		startDim = pFeatureData->lenFeatStream * dimFeat;
		//bgy
		//pOutFeatVec = pFeatureData->featStream.asr_feat + startDim;
		pOutFeatVec = pFeatureData->featStream->asr_feat + startDim;
		memcpy(pOutFeatVec, pSrcFeatVec, (size_t)dimFeat*sizeof(hci_asr_feat_t));
	}

//    pFeatureData->lenFeatStream = PowerASR_BasicOP_add_32_32(pFeatureData->lenFeatStream, 1);
	pFeatureData->lenFeatStream = PowerASR_BasicOP_add_32_32(pFeatureData->lenFeatStream, 1) % MAX_LEN_FEAT_FRAME;//yowon 2016-07-25

	return 0;
}


/**
 *	quantize feature vector
 */
HCILAB_PRIVATE hci_int32
_FX_QUANTIZER_normalizeFeature(hci_mfcc_t *pFeatVec,		///< (i/o) original/output feature vector
							   hci_mfcc_t *quantMean,		///< (i) normalization mean vector
							   hci_mfcc_t *quantInvStdev,	///< (i) normalization variance vector
							   hci_int16 dimFeat)			///< (i) feature dimension
{
	hci_mfcc_t *pMFCC = pFeatVec;
	hci_mfcc_t *pLastMFCC = pFeatVec + dimFeat;
	hci_mfcc_t *pMean = quantMean;
	hci_mfcc_t *pWgt = quantInvStdev;
#ifdef FIXED_POINT_FE
	hci_mfcc64 L_val = 0;
#endif	// #ifdef FIXED_POINT_FE

	while (pMFCC < pLastMFCC) {
#ifdef FIXED_POINT_FE
		*pMFCC -= (*pMean);										// Q15.32
		L_val   = (hci_mfcc64)(*pMFCC) * (hci_mfcc64)(*pWgt);	// Q30.64
		*pMFCC  = (hci_mfcc_t)(L_val>>15);						// Q15.32
#else	// !FIXED_POINT_FE
		*pMFCC -= (*pMean);
		*pMFCC *= (*pWgt);
#endif	// #ifdef FIXED_POINT_FE
		pMFCC++;
		pMean++; pWgt++;
	}

	return 0;
}

/**
 *	find quantization level of input feature
 */
HCILAB_PRIVATE hci_uint8
_FX_QUANTIZER_findQuantizationLevel(hci_mfcc_t inputFeat,			///< (i) input feature value
									hci_mfcc_t *quantLevelMfcc,		///< (i) quantization boundary vector
									hci_int32 numMfccQuantLevel)	///< (i) quantization level count
{
	hci_int16 lower = 0, upper = 0, mid = 0;
	hci_mfcc_t diff = 0;

	lower = 0;
	upper = numMfccQuantLevel - 2;
	while (lower <= upper) {
		mid = ((lower+upper)>>1);
		diff = inputFeat - quantLevelMfcc[mid];
		if (diff > 0) lower = mid + 1;
		else if (diff < 0) upper = mid - 1;
		else {
			return (hci_uint8)mid;
		}
	}

	return (hci_uint8)lower;
}

/* end of file */






















