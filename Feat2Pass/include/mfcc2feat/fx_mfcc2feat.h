
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
 *	@file	fx_mfcc2feat.h
 *	@ingroup mfcc2feat_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	mfcc-to-feature interface for ASR feature extraction
 */

#ifndef __FX_MFCC2FEAT_H__
#define __FX_MFCC2FEAT_H__

#include "base/hci_type.h"
#include "basic_op/fixpoint.h"
#include "wave2mfcc/fx_mfcc_common.h"
#include "epd/epd_common.h"
#include "mfcc2feat/hci_fx_mfcc2feat.h"

/**
 *	@enum cepstral mean subtraction (CMS) type
 */
typedef enum {
		NO_CMS,					///< none CMS
		BATCH_CMS,				///< batch-mode CMS (utterance-level)
		LIVE_CMS,				///< live-mode CMS
		RECUR_CMS,				///< recursive CMS (N-frames delayed)
		BATCH_CMVN,				///< batch-mode CMN & CVN
		LIVE_CMVN,				///< live-mode CMN & CVN
		RECUR_CMVN				///< recursive CMN & CVN (N-frames delayed)
} CMS_Type;

/**
 *	@enum energy normalization type
 */
typedef enum {
		NO_EN,					///< none EN
		BATCH_EN,				///< batch-mode EN (utterance-level)
		LIVE_EN					///< live-mode EN
} EN_Type;

/**
 *	@enum frame compression type
 */
typedef enum {
		NO_FC,					///< none FC
		FC_AUTO_VFR,			///< automatic FC based on variable frame rate analysis
		FC5TO4,					///< 20% frame compression (5 frames --> 4 frames)
		FC4TO3,					///< 25% frame compression (4 frames --> 3 frames)
		FC5TO3,					///< 40% frame compression (5 frames --> 3 frames)
		FC2TO1					///< 50% frame compression (2 frames --> 1 frame)
} FC_Type;

/** Structure holding feature quantizer parameters. */
typedef struct {
	hci_int32 numMfccQuantLevel;					///< number of MFCC quantization level
	hci_mfcc_t quantMean[DIM_FEATURE];				///< shift factor to normalize feature vectors
	hci_mfcc_t quantInvStdev[DIM_FEATURE];			///< scale factor to normalize feature vectors
	hci_mfcc_t quantLevelMfcc[QUANT_LEVEL_MFCC];	///< MFCC quantization level
} FEAT_Quantizer;

/** Structure holding front-end parameters. */
typedef struct {
	CMS_Type typeCMS;								///< CMS type
	EN_Type typeEN;									///< EN type
	FC_Type typeFC;									///< FC type
	hci_int16 bUseLogE;								///< flag to use log energy
	hci_int16 bUseDif;								///< flag to use differential coeff.
	hci_int16 bUseAcc;								///< flag to use acceleration coeff.
	hci_int16 bUse3rdDif;							///< flag to use 3rd differential coeff.
	hci_int16 bSilenceDrop;							///< flag to silence dropping
	hci_int16 bSilenceDropInSpeech;					///< flag to silence dropping in Speech
	hci_int16 bCMSwVAD;								///< flag to VAD-based CMS
	hci_int16 bLiveMode;							///< flag to live mode
	hci_int16 bUseSilenceMean;						///< flag to use silence cepstral mean vector
	hci_int16 winDif;								///< window length for differential coeff.
	hci_int16 winAcc;								///< window length for acceleration coeff.
	hci_int16 win3rdDif;							///< window length for 3rd differential coeff.
	hci_int16 winSilDrop;							///< window length for silence dropping
	hci_int16 dimMFCC;								///< MFCC dimension
	hci_int16 dimFeat;								///< feature dimension
	hci_mfcc16 forgetF_Voice;						///< forgetting factor in recursive feature normalization
	hci_mfcc16 forgetF_Unvoice;						///< forgetting factor in recursive feature normalization
	hci_mfcc32 distThresh;							///< feature distance threshold in FC_AUTO_VFR
	hci_int32 nMaxFrameDrop;						///< count of consecutive frame drops in FC_AUTO_VFR
	hci_int32 nFXDelay;								///< delayed frame count in FX process
	FEAT_Normalizer seedFeatNorm;					///< seed of feature normalizer
	FEAT_Quantizer mfccQuantizer;					///< feature quantizer
	char szMfccQuantFile[256];						///< filename that MFCC quantization data were saved.
	char szCepMeanFile[256];						///< filename that cepstral mean vector was saved.
	char szCMS_ProfileListFName[256];				///KSH CMS Profile List File Name
	char szCMS_ProfileModelNameList[100][256];		///KSH CMS Profile Model List Buf
	char szCMS_ProfileCmsVecFNameList[100][256];	///KSH CMS Profile CMS Vector List Buf
	int numCmsModels;								///KSH num CMS Profiles
	int flagCmsProfile;								///KSH flag for CMS_Profile List existence or no existence
	
	
	FEAT_Normalizer seedFeatNorm_ForCmsProfileType[256];	///KSH seed of feature normalizer for a CMS Profile


} Mfcc2FeatParameters;

#ifdef __cplusplus
extern "C" {
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
FX_Mfcc2Feat_convertMfccStream2FeatureVector(Feature_UserData *pFeatureData,		///< (o) feature user data
											 Mfcc2FeatParameters *pFXVar,			///< (i) config. struct for mfcc-to-feature converter
											 MFCC_UserData *pMfccData				///< (i) MFCC user data
);


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
													   MFCC_UserData *pMfccData				///< (i) MFCC user data
);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef __FX_MFCC2FEAT_H__
