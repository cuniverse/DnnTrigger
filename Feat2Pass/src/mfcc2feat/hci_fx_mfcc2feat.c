
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
 *	@file	hci_fx_mfcc2feat.c
 *	@ingroup mfcc2feat_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	PowerASR mfcc-to-feature converter library
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "base/hci_macro.h"
#include "base/hci_malloc.h"
#include "base/hci_msg.h"
#include "base/parse_config.h"
#include "base/case.h"
//#include "frontend/powerdsr_frontend.h"//KSH_20150422

#include "mfcc2feat/hci_fx_mfcc2feat.h"
#include "mfcc2feat/fx_mfcc2feat.h"
#include "mfcc2feat/fx_cms.h"
#include "mfcc2feat/fx_agc.h"
#include "mfcc2feat/fx_quantizer.h"

#if 0//original
#include "dnn/PowerAI_BaseCommon_Struct.h"
#include "dnn/PowerAI_BaseCommon.h"
#else//yowon 2015-04-16
//#include "../../include/dnn/PowerAI_BaseCommon_Struct.h"
//#include "../../include/dnn/PowerAI_BaseCommon.h"
#endif
/** inner data struct for mfcc-to-feature converter */
typedef struct 
{
	Mfcc2FeatParameters paraMfcc2Feat;	///< parameters for mfcc-to-feature converter
} FX_Mfcc2Feat_Inner;

// local functions
#ifdef __cplusplus
extern "C" {
#endif 

/**
 * setup default environments for FX post-processor
 */
HCILAB_PRIVATE hci_int32
_FX_Mfcc2Feat_defaultConfigurations(PowerASR_FX_Mfcc2Feat *pThis);


/**
 * setup environments from a given configuration file
 */
HCILAB_PRIVATE hci_int32
_FX_Mfcc2Feat_loadConfigurations(PowerASR_FX_Mfcc2Feat *pThis,
								 const char *pszHomeDir,
								 const char *pszConfigFile);

#ifdef __cplusplus
}
#endif


/**
 *	create a new mfcc-to-feature converter.
 *
 *	@return Return the pointer to a newly created mfcc-to-feature converter
 */
HCILAB_PUBLIC HCI_MFCC2FEAT_API PowerASR_FX_Mfcc2Feat*
PowerASR_FX_Mfcc2Feat_new()
{
	PowerASR_FX_Mfcc2Feat *pFX_Mfcc2Feat = 0;
	FX_Mfcc2Feat_Inner *pInner = 0;

	pFX_Mfcc2Feat = (PowerASR_FX_Mfcc2Feat *) hci_malloc( sizeof(PowerASR_FX_Mfcc2Feat) );

	if ( pFX_Mfcc2Feat ) {
		memset(pFX_Mfcc2Feat, 0, sizeof(PowerASR_FX_Mfcc2Feat));

		pInner = (FX_Mfcc2Feat_Inner *) hci_malloc( sizeof(FX_Mfcc2Feat_Inner) );
		if ( pInner ) {
			memset(pInner, 0, sizeof(FX_Mfcc2Feat_Inner));
			pFX_Mfcc2Feat->pInner = (void *)pInner;
		}
	}
	else {
		HCIMSG_ERROR("cannot create FX mfcc-to-feature converter.\n");
	}

	return pFX_Mfcc2Feat;
}


/**
 *	delete the mfcc-to-feature converter.
 */
HCILAB_PUBLIC HCI_MFCC2FEAT_API void
PowerASR_FX_Mfcc2Feat_delete(PowerASR_FX_Mfcc2Feat *pThis)
{
	FX_Mfcc2Feat_Inner *pInner = 0;

	if (0 == pThis) {
		return;
	}

	pInner = (FX_Mfcc2Feat_Inner *) pThis->pInner;

	if ( pInner ) hci_free(pInner);
	hci_free(pThis);
}


/**
 *	set-up environments for mfcc-to-feature converter,
 *	and allocate necessary memories.
 *
 *	@return return 0 if Mfcc2Feature environments are set-up correctly, otherwise return -1.
 */
HCILAB_PUBLIC HCI_MFCC2FEAT_API hci_int32
PowerASR_FX_Mfcc2Feat_openMfcc2FeatConverter(PowerASR_FX_Mfcc2Feat *pThis,	///< (i/o) pointer to the mfcc-to-feature converter
											 const char *pszHomeDir,		///< (i) working directory name
											 const char *pszConfigFile)		///< (i) MFCC-to-Feature configuration file
{
	FX_Mfcc2Feat_Inner *pInner = 0;
	hci_int32 bSetup = 0;

	if (0 == pThis) {
		return -1;
	}

	pInner = (FX_Mfcc2Feat_Inner *) pThis->pInner;

	if (0 == pszConfigFile) {
		bSetup = _FX_Mfcc2Feat_defaultConfigurations(pThis);
	}
	else {
		bSetup = _FX_Mfcc2Feat_loadConfigurations(pThis, pszHomeDir, pszConfigFile);
	}

	if (0 != bSetup) {
		return -1;
	}

	return bSetup;
}


/**
 *	free memories allocated to the mfcc-to-feature converter.
 *
 *	@return return 0 if all memories are freed successfully, otherwise return -1.
 */
HCILAB_PUBLIC HCI_MFCC2FEAT_API hci_int32
PowerASR_FX_Mfcc2Feat_closeMfcc2FeatConverter(PowerASR_FX_Mfcc2Feat *pThis)	///< (i/o) pointer to the mfcc-to-feature converter
{
	return (pThis ? 0 : -1);
}


/**
 *	initialize data buffers for mfcc-to-feature converter.
 *
 *	@return return 0 if Mfcc2Feature data buffers are initialized successfully, otherwise return -1.
 */
HCILAB_PUBLIC HCI_MFCC2FEAT_API hci_int32
PowerASR_FX_Mfcc2Feat_initializeMfcc2FeatConverter(PowerASR_FX_Mfcc2Feat *pThis,	///< (i) pointer to the mfcc-to-feature converter
												   Feature_UserData *pFeatureData,	///< (i/o) pointer to user-specific feature data struct
												   hci_flag bResetNorm)				///< (i) flag to reset feature normalizer
{
	FX_Mfcc2Feat_Inner *pInner = 0;
	Mfcc2FeatParameters *pFXVar = 0;
	MFCC_Stream *pMfccStream = 0;

	if (0 == pThis) {
		return -1;
	}

	pInner = (FX_Mfcc2Feat_Inner *) pThis->pInner;
	pFXVar = &pInner->paraMfcc2Feat;
	pMfccStream = &pFeatureData->mfccStream;

	pFeatureData->lenFeatStream = 0;
	//bgy
	//memset(&pFeatureData->featStream, 0, sizeof(ASR_FEATURE));
	memset(pFeatureData->featStream, 0, sizeof(ASR_FEATURE));
	memset(pFeatureData->feat, 0, sizeof(pFeatureData->feat));

	if (bResetNorm) {
		memset(pMfccStream, 0, sizeof(pFeatureData->mfccStream));
		memcpy(&pMfccStream->userFeatNorm, &(pFXVar->seedFeatNorm), sizeof(pFXVar->seedFeatNorm));
		memcpy(&pMfccStream->baseFeatNorm, &(pFXVar->seedFeatNorm), sizeof(pFXVar->seedFeatNorm));
	}
	else {
		int i = 0;
		pMfccStream->curLenSpeech = 0;
		pMfccStream->epd_state = UTTER_START;
		pMfccStream->nLenOutputMfcc = 0;
		pMfccStream->nLenDropFrame = 0;
		pMfccStream->nLenSkipFrame = 0;
		pMfccStream->nCurrentMfccCell = 0;
		pMfccStream->nOutputMfccCell = 0;
		for ( i = 0; i < SIZE_MFCC_HISTORY; i++) {
			pMfccStream->preCell[i]  = 0;
		}
	}

	pMfccStream->maxLenStream  = SIZE_MFCC_BUFFER;
	pMfccStream->curLenSilence = pFXVar->winSilDrop;
	//bgy
	//pFeatureData->featStream.dimFeat = pFXVar->dimFeat;
	pFeatureData->featStream->dimFeat = pFXVar->dimFeat;

	memset(pMfccStream->mfccPool, 0, sizeof(pMfccStream->mfccPool));

//	if (FALSE == pFXVar->bLiveMode) {
	if (FALSE == pFXVar->bLiveMode || 1) {//for dnn ksh
		if (0 == pFeatureData->utterStream.sent_feat) {
			pFeatureData->utterStream.sent_feat = (hci_mfcc_t *) hci_malloc(MAX_LEN_FEAT_FRAME*DIM_FEATURE*sizeof(hci_mfcc_t));
		}
		memset(pFeatureData->utterStream.sent_feat, 0, MAX_LEN_FEAT_FRAME*DIM_FEATURE*sizeof(hci_mfcc_t));
		if (0 == pFeatureData->utterStream.frame_class) {
			pFeatureData->utterStream.frame_class = (FrameClass *) hci_malloc(MAX_LEN_FEAT_FRAME*sizeof(FrameClass));
		}
		memset(pFeatureData->utterStream.frame_class, 0, MAX_LEN_FEAT_FRAME*sizeof(FrameClass));
	}

	FX_CMS_initializeCepstrumMean(pMfccStream, pFXVar->dimMFCC, pFXVar->typeCMS, pFXVar->bCMSwVAD);
	FX_AGC_initializeMaxLogEnergy(pMfccStream, pFXVar->typeEN);

	memset( pFeatureData->mfccHist, 0, MAX_LEN_FEAT_FRAME * sizeof(pFeatureData->mfccHist) );
	pFeatureData->nSizeMfccHist = 0;

	return 0;
}


/**
 *	free user data buffers for mfcc-to-feature converter.
 *
 *	@return return 0 if Mfcc2Feature data buffers are released successfully, otherwise return -1.
 */
HCILAB_PUBLIC HCI_MFCC2FEAT_API hci_int32
PowerASR_FX_Mfcc2Feat_terminateMfcc2FeatConverter(PowerASR_FX_Mfcc2Feat *pThis,		///< (i) pointer to the mfcc-to-feature converter
												  Feature_UserData *pFeatureData)	///< (i/o) pointer to user-specific feature data struct
{
	FX_Mfcc2Feat_Inner *pInner = 0;
	Mfcc2FeatParameters *pFXVar = 0;

	if (0 == pThis) {
		return -1;
	}

	pInner = (FX_Mfcc2Feat_Inner *) pThis->pInner;
	pFXVar = &pInner->paraMfcc2Feat;

	/*if (pFXVar->typeCMS == LIVE_CMS || pFXVar->typeCMS == LIVE_CMVN) {
		FX_CMS_saveCepstralMeanVector(&pFeatureData->mfccStream.baseFeatNorm, pFXVar->szCepMeanFile);
	}*/

//	if (FALSE == pFXVar->bLiveMode) { // for dnn ksh
	if (FALSE == pFXVar->bLiveMode || 1) {
		if (pFeatureData->utterStream.sent_feat) {
			hci_free(pFeatureData->utterStream.sent_feat);
			pFeatureData->utterStream.sent_feat = 0;
		}
		if (pFeatureData->utterStream.frame_class) {
			hci_free(pFeatureData->utterStream.frame_class);
			pFeatureData->utterStream.frame_class = 0;
		}
	}

	return 0;
}


/**
 *	free convert input MFCC stream into feature vectors.
 *
 *	@return Return one of the following values:
 *		- return M2F_FAIL if Mfcc2Feature operation failed
 *		- return M2F_FALSE if returned outputs don't exist
 *		- return M2F_TRUE if returned outputs exist
 *		- return M2F_RESET if previous returned outputs have to be reset
 *		- return M2F_COMPLETE if Mfcc2Feature operation completed
 *
 *	@see FX_Mfcc2Feat_convertMfccStream2FeatureVector()
 */
HCILAB_PUBLIC HCI_MFCC2FEAT_API M2F_Status
PowerASR_FX_Mfcc2Feat_convertMfccStream2FeatureVector(PowerASR_FX_Mfcc2Feat *pThis,		///< (i) pointer to the mfcc-to-feature converter
													  Feature_UserData *pFeatureData,	///< (o) channel-specific output feature data
													  MFCC_UserData *pMfccData,			///< (i) channel-specific MFCC data struct
													  hci_flag bUseEpdResult)			///< (i) flag to use frame-by-frame EPD results
{
	FX_Mfcc2Feat_Inner *pInner = 0;
	Mfcc2FeatParameters *pFXVar = 0;
	M2F_Status m2f_status = M2F_FALSE;

	if (0 == pThis) {
		return M2F_FAIL;
	}

	pInner = (FX_Mfcc2Feat_Inner *) pThis->pInner;
	pFXVar = &pInner->paraMfcc2Feat;

	if ((size_t)pFeatureData->lenFeatStream >= MAX_LEN_FEAT_FRAME) {
		return M2F_FAIL;
	}

	// mfcc-to-feature conversion
	if (bUseEpdResult) {
		m2f_status = FX_Mfcc2Feat_convertMfccStream2FeatureVector(pFeatureData,
																  pFXVar,
																  pMfccData);
	}
	else {
		m2f_status = FX_Mfcc2Feat_convertMfccStream2FeatureVectorWithoutEPD(pFeatureData,
																			pFXVar,
																			pMfccData);
	}

	// in batch mode, cannot emit feature vectors
	if (M2F_TRUE == m2f_status && FALSE == pFXVar->bLiveMode) {
		m2f_status = M2F_FALSE;
	}

	return m2f_status;
}


/**
 *	add a new feature vector into feature stream.
 *
 *	@return return 0 if feature insertion is completed, otherwise return -1.
 */
HCILAB_PUBLIC HCI_MFCC2FEAT_API hci_int32
PowerASR_FX_Mfcc2Feat_addFeatureVector(PowerASR_FX_Mfcc2Feat *pThis,		///< (i) pointer to the mfcc-to-feature converter
									   Feature_UserData *pFeatureData)		///< (o) channel-specific output feature data
{
	FX_Mfcc2Feat_Inner *pInner = 0;
	Mfcc2FeatParameters *pFXVar = 0;

	if (0 == pThis) {
		return -1;
	}

	pInner = (FX_Mfcc2Feat_Inner *) pThis->pInner;
	pFXVar = &pInner->paraMfcc2Feat;

	// feature quantization
	FX_QUANTIZER_quantizeFeature(pFeatureData,
								 &pFXVar->mfccQuantizer,
								 pFXVar->dimFeat);

	return 0;
}


/**
 *	get feature normalization vector.
 *
 *	@return return 0 if feature normalization vectors are copied successfully, otherwise return -1.
 */
HCILAB_PUBLIC HCI_MFCC2FEAT_API hci_int32
PowerASR_FX_Mfcc2Feat_getFeatureNormalizer(PowerASR_FX_Mfcc2Feat *pThis,	///< (i) pointer to the mfcc-to-feature converter
										   FEAT_Normalizer *pOutFeatNorm)	///< (o) feature normalization vectors
{
	FX_Mfcc2Feat_Inner *pInner = 0;
	Mfcc2FeatParameters *pFXVar = 0;

	if (0 == pThis) {
		return -1;
	}
	if (0 == pOutFeatNorm) {
		return -1;
	}

	pInner = (FX_Mfcc2Feat_Inner *) pThis->pInner;
	pFXVar = &pInner->paraMfcc2Feat;

	memcpy(pOutFeatNorm, &(pFXVar->seedFeatNorm), sizeof(pFXVar->seedFeatNorm));

	return 0;
}


int seedID[128];
FEAT_Normalizer *luSeed[128] = {0,};


HCILAB_PUBLIC HCI_MFCC2FEAT_API hci_int32
	PowerASR_FX_Mfcc2Feat_getCMSSeed(int CHID,FEAT_Normalizer *lf)	///< (i) seed feature normalization vectors
{

	lf =luSeed[CHID];
	return seedID[CHID];
}
/**
 *	set feature normalization vector.
 *
 *	@return return 0 if feature normalization vectors are set successfully, otherwise return -1.
 */
/*
HCILAB_PUBLIC HCI_MFCC2FEAT_API hci_int32
PowerASR_FX_Mfcc2Feat_setFeatureNormalizer(PowerASR_FX_Mfcc2Feat *pThis,			///< (i) pointer to the mfcc-to-feature converter
										   MFCC_Stream *pMfccStream,				///< (o) pointer to user-specific mfcc data struct
										   const FEAT_Normalizer *pSeedFeatNorm)	///< (i) seed feature normalization vectors
*/
HCILAB_PUBLIC HCI_MFCC2FEAT_API hci_int32
PowerASR_FX_Mfcc2Feat_setFeatureNormalizer(PowerASR_FX_Mfcc2Feat *pThis,			///< (i) pointer to the mfcc-to-feature converter
										   MFCC_Stream *pMfccStream,				///< (o) pointer to user-specific mfcc data struct
										   const FEAT_Normalizer *pSeedFeatNorm,char *cmsModelName,const LONG nChannelID)	///< (i) seed feature normalization vectors
{

	//float cepMean[12];
	FX_Mfcc2Feat_Inner *pInner = 0;
	Mfcc2FeatParameters *pFXVar = 0;
	int n,cmsProfileIndex;
	
	FILE *saveCmsFp_w;
	char saveCmsFName[255];
	float cepMeanSil[12];
	float cepMeanUnVoiced[12];
	float cepMeanVoiced[12];
	float cepInvStdSil[12];
	float cepInvStdUnVoiced[12];
	float cepInvStdVoiced[12];

	if (0 == pThis) {
		return -1;
	}
	if (0 == pMfccStream) {
		return -1;
	}
	if (0 == pSeedFeatNorm) {
		return -1;
	}

	pInner = (FX_Mfcc2Feat_Inner *) pThis->pInner;
	pFXVar = &pInner->paraMfcc2Feat;

	//CMS Profile 적용 //KSH
/*
	memcpy( &pMfccStream->userFeatNorm, pSeedFeatNorm, sizeof(FEAT_Normalizer) );
	memcpy( &pMfccStream->baseFeatNorm, &pMfccStream->userFeatNorm, sizeof(pMfccStream->userFeatNorm) );
*/
	//CMS Model Name 찾기//
	if(pFXVar->flagCmsProfile == TRUE)//CMS Profile List File이 있으면
	{
		cmsProfileIndex = -1;
		for(n=1;n<pFXVar->numCmsModels;n++)//n=0이면 unknown임.
		{
			if(strcmp(cmsModelName,pFXVar->szCMS_ProfileModelNameList[n]) == 0)
			{
				cmsProfileIndex = n;
				//PowerDSR_FE_SetUpdateFlag(n,1);
			}
			/*else
			{
				PowerDSR_FE_SetUpdateFlag(n,0);
			}*/
		}

		//해당 Model에 대한 CMS Vector적용//

		if(cmsProfileIndex == -1)//CMS_Model List에 등록된 단말기가 없으면 기본 CMS_Vector사용
		{
			seedID[nChannelID]=0;
			luSeed[nChannelID] = pSeedFeatNorm;

			memcpy( &pMfccStream->userFeatNorm, pSeedFeatNorm, sizeof(FEAT_Normalizer) );
			memcpy( &pMfccStream->baseFeatNorm, &pMfccStream->userFeatNorm, sizeof(pMfccStream->userFeatNorm) );
		}
		else//CMS_Model List에서 등록된 단말기를 찾으면 해당 단말기의 CMS Vector사용
		{
	//		memcpy(&pSeedFeatNorm,&pFXVar->seedFeatNorm_ForCmsProfileType[cmsProfileIndex],sizeof(FEAT_Normalizer));
			seedID[nChannelID]=cmsProfileIndex;
			luSeed[nChannelID]=&pFXVar->seedFeatNorm_ForCmsProfileType[cmsProfileIndex];
			memcpy( &pMfccStream->userFeatNorm, &pFXVar->seedFeatNorm_ForCmsProfileType[cmsProfileIndex], sizeof(FEAT_Normalizer) );
			memcpy( &pMfccStream->baseFeatNorm, &pMfccStream->userFeatNorm, sizeof(pMfccStream->userFeatNorm) );
		}	
	}
	else//CMS Profile List File이 없으면
	{
		memcpy( &pMfccStream->userFeatNorm, pSeedFeatNorm, sizeof(FEAT_Normalizer) );
		memcpy( &pMfccStream->baseFeatNorm, &pMfccStream->userFeatNorm, sizeof(pMfccStream->userFeatNorm) );				
	}

	//debugging KSH
	//memcpy(&cepMean,&pMfccStream->userFeatNorm.cepMeanSilence,sizeof(pMfccStream->userFeatNorm.cepMeanSilence));
#ifdef _DEBUG
	sprintf(saveCmsFName,"%s_CMS_Vector.txt",cmsModelName);
	saveCmsFp_w = fopen(saveCmsFName,"wt");
	memcpy(&cepMeanSil,&pMfccStream->userFeatNorm.cepMeanSilence,sizeof(pMfccStream->userFeatNorm.cepMeanSilence));
	memcpy(&cepMeanUnVoiced,&pMfccStream->userFeatNorm.cepMeanUnvoiced,sizeof(pMfccStream->userFeatNorm.cepMeanUnvoiced));
	memcpy(&cepMeanVoiced,&pMfccStream->userFeatNorm.cepMeanVoiced,sizeof(pMfccStream->userFeatNorm.cepMeanVoiced));
	memcpy(&cepInvStdSil,&pMfccStream->userFeatNorm.cepVarSilence,sizeof(pMfccStream->userFeatNorm.cepVarSilence));
	memcpy(&cepInvStdUnVoiced,&pMfccStream->userFeatNorm.cepVarUnvoiced,sizeof(pMfccStream->userFeatNorm.cepVarUnvoiced));
	memcpy(&cepInvStdVoiced,&pMfccStream->userFeatNorm.cepVarVoiced,sizeof(pMfccStream->userFeatNorm.cepVarVoiced));
	fprintf(saveCmsFp_w,"Cep Norm\n");
	fprintf(saveCmsFp_w,"<Cep MeanSilence>\n");
	fprintf(saveCmsFp_w,"%f %f %f %f %f %f %f %f %f %f %f %f\n",
		cepMeanSil[0],cepMeanSil[1],cepMeanSil[2],cepMeanSil[3],cepMeanSil[4],cepMeanSil[5],cepMeanSil[6],cepMeanSil[7],cepMeanSil[8],cepMeanSil[9],cepMeanSil[10],cepMeanSil[11],cepMeanSil[12]);

	fprintf(saveCmsFp_w,"<Cep MeanUnVoiced>\n");
	fprintf(saveCmsFp_w,"%f %f %f %f %f %f %f %f %f %f %f %f\n",
		cepMeanUnVoiced[0],cepMeanUnVoiced[1],cepMeanUnVoiced[2],cepMeanUnVoiced[3],cepMeanUnVoiced[4],cepMeanUnVoiced[5],cepMeanUnVoiced[6],cepMeanUnVoiced[7],cepMeanUnVoiced[8],cepMeanUnVoiced[9],cepMeanUnVoiced[10],cepMeanUnVoiced[11],cepMeanUnVoiced[12]);

	fprintf(saveCmsFp_w,"<Cep MeanVoiced>\n");
	fprintf(saveCmsFp_w,"%f %f %f %f %f %f %f %f %f %f %f %f\n",
		cepMeanVoiced[0],cepMeanVoiced[1],cepMeanVoiced[2],cepMeanVoiced[3],cepMeanVoiced[4],cepMeanVoiced[5],cepMeanVoiced[6],cepMeanVoiced[7],cepMeanVoiced[8],cepMeanVoiced[9],cepMeanVoiced[10],cepMeanVoiced[11],cepMeanVoiced[12]);

	fprintf(saveCmsFp_w,"<Cep Inverse Std Silence>\n");
	fprintf(saveCmsFp_w,"%f %f %f %f %f %f %f %f %f %f %f %f\n",
		cepInvStdSil[0],cepInvStdSil[1],cepInvStdSil[2],cepInvStdSil[3],cepInvStdSil[4],cepInvStdSil[5],cepInvStdSil[6],cepInvStdSil[7],cepInvStdSil[8],cepInvStdSil[9],cepInvStdSil[10],cepInvStdSil[11],cepInvStdSil[12]);

	fprintf(saveCmsFp_w,"<Cep Inverse Std UnVoiced>\n");
	fprintf(saveCmsFp_w,"%f %f %f %f %f %f %f %f %f %f %f %f\n",
		cepInvStdUnVoiced[0],cepInvStdUnVoiced[1],cepInvStdUnVoiced[2],cepInvStdUnVoiced[3],cepInvStdUnVoiced[4],cepInvStdUnVoiced[5],cepInvStdUnVoiced[6],cepInvStdUnVoiced[7],cepInvStdUnVoiced[8],cepInvStdUnVoiced[9],cepInvStdUnVoiced[10],cepInvStdUnVoiced[11],cepInvStdUnVoiced[12]);

	fprintf(saveCmsFp_w,"<Cep Inverse Std Voiced>\n");
	fprintf(saveCmsFp_w,"%f %f %f %f %f %f %f %f %f %f %f %f\n",
		cepInvStdVoiced[0],cepInvStdVoiced[1],cepInvStdVoiced[2],cepInvStdVoiced[3],cepInvStdVoiced[4],cepInvStdVoiced[5],cepInvStdVoiced[6],cepInvStdVoiced[7],cepInvStdVoiced[8],cepInvStdVoiced[9],cepInvStdVoiced[10],cepInvStdVoiced[11],cepInvStdVoiced[12]);

	fclose(saveCmsFp_w);
#endif
	FX_CMS_initializeCepstrumMean(pMfccStream, pFXVar->dimMFCC, pFXVar->typeCMS, pFXVar->bCMSwVAD);
	FX_AGC_initializeMaxLogEnergy(pMfccStream, pFXVar->typeEN);

	return 0;
}

HCILAB_PUBLIC HCI_MFCC2FEAT_API hci_int32
PowerASR_FX_Mfcc2Feat_setClientFeatureNormalizer(PowerASR_FX_Mfcc2Feat *pThis,			///< (i) pointer to the mfcc-to-feature converter
										   MFCC_Stream *pMfccStream,				///< (o) pointer to user-specific mfcc data struct
										   const FEAT_Normalizer *pSeedFeatNorm,
										   char *cmsModelName,
										   const LONG nChannelID, 
										   FEAT_Normalizer *plSeedFeatNorm)	///< (i) seed feature normalization vectors
{
	FX_Mfcc2Feat_Inner *pInner = 0;
	Mfcc2FeatParameters *pFXVar = 0;
	int n,cmsProfileIndex;
	
	FILE *saveCmsFp_w;
	char saveCmsFName[255];
	float cepMeanSil[12];
	float cepMeanUnVoiced[12];
	float cepMeanVoiced[12];
	float cepInvStdSil[12];
	float cepInvStdUnVoiced[12];
	float cepInvStdVoiced[12];

	if (0 == pThis) {
		return -1;
	}
	if (0 == pMfccStream) {
		return -1;
	}
	

	pInner = (FX_Mfcc2Feat_Inner *) pThis->pInner;
	pFXVar = &pInner->paraMfcc2Feat;

	/*if (0 == pSeedFeatNorm) {
		
		return -1;
	}*/

	//CMS Profile 적용 //KSH
/*
	memcpy( &pMfccStream->userFeatNorm, pSeedFeatNorm, sizeof(FEAT_Normalizer) );
	memcpy( &pMfccStream->baseFeatNorm, &pMfccStream->userFeatNorm, sizeof(pMfccStream->userFeatNorm) );
*/
	//CMS Model Name 찾기//
	if(pFXVar->flagCmsProfile == TRUE)//CMS Profile List File이 있으면
	{
		cmsProfileIndex = -1;
		for(n=1;n<pFXVar->numCmsModels;n++)//n=0이면 unknown임.
		{
			if(strcmp(cmsModelName,pFXVar->szCMS_ProfileModelNameList[n]) == 0)
			{
				cmsProfileIndex = n;
				//PowerDSR_FE_SetUpdateFlag(n,1);
			}
			/*else
			{
				PowerDSR_FE_SetUpdateFlag(n,0);
			}*/
		}

		//해당 Model에 대한 CMS Vector적용//
		
		
		if(cmsProfileIndex == -1)//CMS_Model List에 등록된 단말기가 없으면 기본 CMS_Vector사용
		{
			if (pSeedFeatNorm != NULL) {
				memcpy(plSeedFeatNorm, pSeedFeatNorm, sizeof(FEAT_Normalizer));
			}
			seedID[nChannelID]=0;
			luSeed[nChannelID]=plSeedFeatNorm;

			memcpy( &pMfccStream->userFeatNorm, luSeed[nChannelID], sizeof(FEAT_Normalizer) );
			memcpy( &pMfccStream->baseFeatNorm, &pMfccStream->userFeatNorm, sizeof(pMfccStream->userFeatNorm) );
		}
		else//CMS_Model List에서 등록된 단말기를 찾으면 해당 단말기의 CMS Vector사용
		{
			if (pSeedFeatNorm == NULL) {
				pSeedFeatNorm = &pFXVar->seedFeatNorm_ForCmsProfileType[cmsProfileIndex];
			}
			memcpy(plSeedFeatNorm, pSeedFeatNorm, sizeof(FEAT_Normalizer));

			seedID[nChannelID]=cmsProfileIndex;
			luSeed[nChannelID]=plSeedFeatNorm;
			memcpy( &pMfccStream->userFeatNorm, luSeed[nChannelID], sizeof(FEAT_Normalizer) );
			memcpy( &pMfccStream->baseFeatNorm, &pMfccStream->userFeatNorm, sizeof(pMfccStream->userFeatNorm) );
		}	
	}
	else//CMS Profile List File이 없으면
	{
		if (pSeedFeatNorm != NULL) {
			memcpy(plSeedFeatNorm, pSeedFeatNorm, sizeof(FEAT_Normalizer));
		}
		memcpy( &pMfccStream->userFeatNorm, plSeedFeatNorm, sizeof(FEAT_Normalizer) );
		memcpy( &pMfccStream->baseFeatNorm, &pMfccStream->userFeatNorm, sizeof(pMfccStream->userFeatNorm) );				
	}

	FX_CMS_initializeCepstrumMean(pMfccStream, pFXVar->dimMFCC, pFXVar->typeCMS, pFXVar->bCMSwVAD);
	FX_AGC_initializeMaxLogEnergy(pMfccStream, pFXVar->typeEN);

	return 0;
}



/**
 *	update feature normalization vector.
 *
 *	@return return 0 if feature normalization vectors are updated successfully, otherwise return -1.
 */
HCILAB_PUBLIC HCI_MFCC2FEAT_API hci_int32
PowerASR_FX_Mfcc2Feat_updateFeatureNormalizer(PowerASR_FX_Mfcc2Feat *pThis,			///< (i) pointer to the mfcc-to-feature converter
											  MFCC_Stream *pMfccStream)				///< (o) pointer to user-specific mfcc data struct
{
	FX_Mfcc2Feat_Inner *pInner = 0;
	Mfcc2FeatParameters *pFXVar = 0;

	if (0 == pThis) {
		return -1;
	}
	if (0 == pMfccStream) {
		return -1;
	}

	pInner = (FX_Mfcc2Feat_Inner *) pThis->pInner;
	pFXVar = &pInner->paraMfcc2Feat;

	FX_CMS_updateCepstralMeanVector(pMfccStream, pFXVar);
	if (pFXVar->typeEN != NO_EN) {
		FX_AGC_updateMaxLogEnergy(pMfccStream, pFXVar->typeEN);
	}

//	FX_CMS_initializeCepstrumMean(pMfccStream, pFXVar->typeCMS, pFXVar->bCMSwVAD);
	FX_CMS_initializeCepstrumMean(pMfccStream, pFXVar->dimMFCC, pFXVar->typeCMS, pFXVar->bCMSwVAD);
	FX_AGC_initializeMaxLogEnergy(pMfccStream, pFXVar->typeEN);

	return 0;
}


/**
 *	get the length of feature frames given speech duration in sec.
 *
 *	@return return maximum length of feature frames.
 */
HCILAB_PUBLIC HCI_MFCC2FEAT_API LONG
PowerASR_FX_Mfcc2Feat_SpeechDuration2FrameLength(PowerASR_FX_Mfcc2Feat *pThis,		///< (i) pointer to the mfcc-to-feature converter
												 const LONG nDurSpeechSec)			///< (i) speech duration in sec
{
	FX_Mfcc2Feat_Inner *pInner = 0;
	Mfcc2FeatParameters *pFXVar = 0;
	LONG nCountFrames = 0L;
	
	if (0 == pThis) {
		return -1;
	}
	
	pInner = (FX_Mfcc2Feat_Inner *) pThis->pInner;
	pFXVar = &pInner->paraMfcc2Feat;

	switch ( pFXVar->typeFC ) {
		case NO_FC:
		case FC_AUTO_VFR:
			nCountFrames = nDurSpeechSec * 100;
			break;
		case FC5TO4:
			nCountFrames = nDurSpeechSec * 80;
			break;
		case FC5TO3:
			nCountFrames = nDurSpeechSec * 60;
			break;
		case FC4TO3:
			nCountFrames = nDurSpeechSec * 75;
			break;
		case FC2TO1:
			nCountFrames = nDurSpeechSec * 50;
			break;
		default:
			break;
	}

	return nCountFrames;
}

 
/**
 * setup default environments for mfcc-to-feature converter
 */
HCILAB_PRIVATE hci_int32
_FX_Mfcc2Feat_defaultConfigurations(PowerASR_FX_Mfcc2Feat *pThis)
{
	FX_Mfcc2Feat_Inner *pInner = 0;
	Mfcc2FeatParameters *pFXVar = 0;
	hci_float32 fValue = 0;

	if (0 == pThis) {
		return -1;
	}

	pInner = (FX_Mfcc2Feat_Inner *) pThis->pInner;
	pFXVar = &pInner->paraMfcc2Feat;

	pFXVar->typeCMS      = NO_CMS;	// NO_CMS, BATCH_CMS, LIVE_CMS, RECUR_CMS, BATCH_CMVN, LIVE_CMVN, RECUR_CMVN
	pFXVar->typeEN       = NO_EN;	// NO_EN, BATCH_EN, LIVE_EN
	pFXVar->typeFC       = NO_FC;	// NO_FC, FC_AUTO_VFR, FC5TO4, FC4TO3, FC5TO3, FC2TO1
	pFXVar->bUseLogE     = TRUE;	// FALSE, TRUE
	pFXVar->bUseDif      = TRUE;	// FALSE, TRUE
	pFXVar->bUse3rdDif   = FALSE;	// FALSE, TRUE
	pFXVar->bUseAcc      = TRUE;	// FALSE, TRUE
	pFXVar->bSilenceDrop = TRUE;	// FALSE, TRUE
	pFXVar->bCMSwVAD     = TRUE;	// FALSE, TRUE
	pFXVar->bUseSilenceMean = TRUE;	// FALSE, TRUE
	pFXVar->winDif       = 2;
	pFXVar->winAcc       = 4;
	pFXVar->win3rdDif    = 5;
	pFXVar->winSilDrop   = 5;

	// in case of RECUR_CMS, RECUR_CMVN		( 1 - lamba^N = 1 / 2^0.5 , N >= 20)
	if (RECUR_CMS == pFXVar->typeCMS || RECUR_CMVN == pFXVar->typeCMS) {
#ifdef FIXED_POINT_FE
		pFXVar->forgetF_Voice = 32113;			// Q15.16
		pFXVar->forgetF_Unvoice = 32113;		// Q15.16
#else	// !FIXED_POINT_FE
		pFXVar->forgetF_Voice = 0.98f;
		pFXVar->forgetF_Unvoice = 0.98f;
#endif	// #ifdef FIXED_POINT_FE
	}

	pFXVar->dimMFCC = DIM_CEPSTRUM + 1;

	if (pFXVar->bUseLogE) {
		pFXVar->dimFeat  = DIM_CEPSTRUM + 1;
	}
	else {
		pFXVar->dimFeat  = DIM_CEPSTRUM;
	}
	if (pFXVar->bUseDif) {
		pFXVar->dimFeat += DIM_CEPSTRUM + 1;
	}
	if (pFXVar->bUseAcc) {
		pFXVar->dimFeat += DIM_CEPSTRUM + 1;
	}
	if (pFXVar->bUse3rdDif) {
		pFXVar->dimFeat += DIM_CEPSTRUM + 1;
	}

#ifdef FIXED_POINT_FE
	pFXVar->distThresh   = 5120;	// Q8.32
#else	// !FIXED_POINT_FE
	pFXVar->distThresh   = 20.0f;
#endif	// #ifdef FIXED_POINT_FE
	pFXVar->nMaxFrameDrop = 1;

//	strcpy(pFXVar->szCepMeanFile,"cep_mean_vec.seed");
strcpy(pFXVar->szCepMeanFile,"./res/FeatNorm.Clean.Cep12.dat"); // by mshan
#ifdef FIXED_POINT_FE
	strcpy(pFXVar->szMfccQuantFile, "mfcc_quant.dat");
#endif	// #ifdef FIXED_POINT_FE

	if ((pFXVar->typeCMS == LIVE_CMS || pFXVar->typeCMS == LIVE_CMVN || pFXVar->typeEN == LIVE_EN) &&
		FX_CMS_loadSeedCepstralMeanVector(&pFXVar->seedFeatNorm, pFXVar->szCepMeanFile, pFXVar->dimMFCC, pFXVar->typeCMS) == -1) {
		HCIMSG_ERROR("loadSeedCepstralMeanVector failed (%s).\n", pFXVar->szCepMeanFile);
		return -1;
	}

	if (1 == sizeof(hci_asr_feat_t)) {
		if (-1 == FX_QUANTIZER_loadFeatureQuantizer(pFXVar->szMfccQuantFile,
													&pFXVar->mfccQuantizer,
													pFXVar->dimFeat)) {
			HCIMSG_ERROR("loadFeatureQuantizer failed (%s).\n", pFXVar->szMfccQuantFile);
			return -1;
		}
	}

	pFXVar->bLiveMode = TRUE;
	if (BATCH_CMS == pFXVar->typeCMS || BATCH_CMVN == pFXVar->typeCMS || BATCH_EN == pFXVar->typeEN) {
		pFXVar->bLiveMode = FALSE;
	}

	if (pFXVar->bUseAcc) {
		pFXVar->nFXDelay = pFXVar->winAcc;
	}
	else if (pFXVar->bUseDif) {
		pFXVar->nFXDelay = pFXVar->winDif;
	}
	else {
		pFXVar->nFXDelay = 0;
	}
	if (pFXVar->bSilenceDrop) {
		pFXVar->nFXDelay = HCI_MAX(pFXVar->nFXDelay, pFXVar->winSilDrop);
	}

	return 0;
}

int com_count=0;
char cos_model_list[100][256];

/**
 * setup environments from a given configuration file
 */
HCILAB_PRIVATE hci_int32
_FX_Mfcc2Feat_loadConfigurations(PowerASR_FX_Mfcc2Feat *pThis,
								 const char *pszHomeDir,
								 const char *pszConfigFile)
{
	FX_Mfcc2Feat_Inner *pInner = 0;
	Mfcc2FeatParameters *pFXVar = 0;
	char *pszValue = 0;

	if (0 == pThis) {
		return -1;
	}

	pInner = (FX_Mfcc2Feat_Inner *) pThis->pInner;
	pFXVar = &pInner->paraMfcc2Feat;

	if (0 != PowerASR_Base_parseConfigFile(pszConfigFile)) {
		HCIMSG_ERROR("[MFCC2FEAT] parseConfigFile failed (%s).\n", pszConfigFile);
		return -1;
	}

	// CMS type
	pszValue = PowerASR_Base_getArgumentValue("CMS_TYPE");
	if (pszValue) {
		if (0 == PowerASR_Base_strnocasecmp(pszValue,"NO_CMS")) {
			pFXVar->typeCMS = NO_CMS;
		}
		else if (0 == PowerASR_Base_strnocasecmp(pszValue,"LIVE_CMS")) {
			pFXVar->typeCMS = LIVE_CMS;
		}
		else if (0 == PowerASR_Base_strnocasecmp(pszValue,"BATCH_CMS")) {
			pFXVar->typeCMS = BATCH_CMS;
		}
		else if (0 == PowerASR_Base_strnocasecmp(pszValue,"RECUR_CMS")) {
			pFXVar->typeCMS = RECUR_CMS;
		}
		else if (0 == PowerASR_Base_strnocasecmp(pszValue,"LIVE_CMVN")) {
			pFXVar->typeCMS = LIVE_CMVN;
		}
		else if (0 == PowerASR_Base_strnocasecmp(pszValue,"BATCH_CMVN")) {
			pFXVar->typeCMS = BATCH_CMVN;
		}
		else if (0 == PowerASR_Base_strnocasecmp(pszValue,"RECUR_CMVN")) {
			pFXVar->typeCMS = RECUR_CMVN;
		}
		else {
			HCIMSG_ERROR("invalid CMS type: %s.\n", pszValue);
			goto lb_err_mfcc2feat_loadConfig;
		}
	}
	else {
		HCIMSG_WARN("[MFCC2FEAT] CMS_TYPE not assigned.\n");
		goto lb_err_mfcc2feat_loadConfig;
	}

	// EN type
	pszValue = PowerASR_Base_getArgumentValue("EN_TYPE");
	if (pszValue) {
		if (0 == PowerASR_Base_strnocasecmp(pszValue,"NO_EN")) {
			pFXVar->typeEN = NO_EN;
		}
		else if (0 == PowerASR_Base_strnocasecmp(pszValue,"LIVE_EN")) {
			pFXVar->typeEN = LIVE_EN;
		}
		else if (0 == PowerASR_Base_strnocasecmp(pszValue,"BATCH_EN")) {
			pFXVar->typeEN = BATCH_EN;
		}
		else {
			HCIMSG_ERROR("invalid EN type: %s.\n", pszValue);
			goto lb_err_mfcc2feat_loadConfig;
		}
	}
	else {
		HCIMSG_WARN("[MFCC2FEAT] EN_TYPE not assigned.\n");
		goto lb_err_mfcc2feat_loadConfig;
	}

	// FC type
	pszValue = PowerASR_Base_getArgumentValue("FC_TYPE");
	if (pszValue) {
		if (0 == PowerASR_Base_strnocasecmp(pszValue,"NO_FC")) {
			pFXVar->typeFC = NO_FC;
		}
		else if (0 == PowerASR_Base_strnocasecmp(pszValue,"FC_AUTO_VFR")) {
			pFXVar->typeFC = FC_AUTO_VFR;
		}
		else if (0 == PowerASR_Base_strnocasecmp(pszValue,"FC5TO4")) {
			pFXVar->typeFC = FC5TO4;
		}
		else if (0 == PowerASR_Base_strnocasecmp(pszValue,"FC4TO3")) {
			pFXVar->typeFC = FC4TO3;
		}
		else if (0 == PowerASR_Base_strnocasecmp(pszValue,"FC5TO3")) {
			pFXVar->typeFC = FC5TO3;
		}
		else if (0 == PowerASR_Base_strnocasecmp(pszValue,"FC2TO1")) {
			pFXVar->typeFC = FC2TO1;
		}
		else {
			HCIMSG_ERROR("invalid FC type: %s.\n", pszValue);
			goto lb_err_mfcc2feat_loadConfig;
		}
	}
	else {
		HCIMSG_WARN("[MFCC2FEAT] FC_TYPE not assigned.\n");
		goto lb_err_mfcc2feat_loadConfig;
	}

	// flag to use log energy
	pszValue = PowerASR_Base_getArgumentValue("DO_USE_LOG_ENERGY");
	if (pszValue) {
		if (0 == PowerASR_Base_strnocasecmp(pszValue,"yes")) {
			pFXVar->bUseLogE = TRUE;
		}
		else {
			pFXVar->bUseLogE = FALSE;
		}
	}
	else {
		HCIMSG_WARN("[MFCC2FEAT] DO_USE_LOG_ENERGY not assigned.\n");
		goto lb_err_mfcc2feat_loadConfig;
	}

	// flag to use differential coeff.
	pszValue = PowerASR_Base_getArgumentValue("DO_USE_DIFFERENTIAL");
	if (pszValue) {
		if (0 == PowerASR_Base_strnocasecmp(pszValue,"yes")) {
			pFXVar->bUseDif = TRUE;
		}
		else {
			pFXVar->bUseDif = FALSE;
		}
	}
	else {
		HCIMSG_WARN("[MFCC2FEAT] DO_USE_DIFFERENTIAL not assigned.\n");
		goto lb_err_mfcc2feat_loadConfig;
	}

	// flag to use acceleration coeff.
	pszValue = PowerASR_Base_getArgumentValue("DO_USE_ACCELERATION");
	if (pszValue) {
		if (0 == PowerASR_Base_strnocasecmp(pszValue,"yes")) {
			pFXVar->bUseAcc = TRUE;
		}
		else {
			pFXVar->bUseAcc = FALSE;
		}
	}
	else {
		HCIMSG_WARN("[MFCC2FEAT] DO_USE_ACCELERATION not assigned.\n");
		goto lb_err_mfcc2feat_loadConfig;
	}

	// flag to use 3rd differential coeff.
	pszValue = PowerASR_Base_getArgumentValue("DO_USE_THIRD_DIFFERENTIAL");
	if (pszValue) {
		if (0 == PowerASR_Base_strnocasecmp(pszValue,"yes")) {
			pFXVar->bUse3rdDif = TRUE;
		}
		else {
			pFXVar->bUse3rdDif = FALSE;
		}
	}
	else {
		HCIMSG_WARN("[MFCC2FEAT] DO_USE_THIRD_DIFFERENTIAL not assigned.\n");
		goto lb_err_mfcc2feat_loadConfig;
	}

	// flag to silence dropping
	pszValue = PowerASR_Base_getArgumentValue("DO_DROP_SILENCE");
	if (pszValue) {
		if (0 == PowerASR_Base_strnocasecmp(pszValue,"yes")) {
			pFXVar->bSilenceDrop = TRUE;
		}
		else {
			pFXVar->bSilenceDrop = FALSE;
		}
	}
	else {
		HCIMSG_WARN("[MFCC2FEAT] DO_DROP_SILENCE not assigned.\n");
		goto lb_err_mfcc2feat_loadConfig;
	}


	// flag to silence dropping
	pszValue = PowerASR_Base_getArgumentValue("DO_DROP_SILENCE_IN_SPEECH");
	if (pszValue) {
		if (0 == PowerASR_Base_strnocasecmp(pszValue,"yes")) {
			pFXVar->bSilenceDropInSpeech = TRUE;
		}
		else {
			pFXVar->bSilenceDropInSpeech = FALSE;
		}
	}
	else {
		HCIMSG_WARN("[MFCC2FEAT] DO_DROP_SILENCE not assigned.\n");
		goto lb_err_mfcc2feat_loadConfig;
	}


	// flag to VAD-dependent CMS
	pszValue = PowerASR_Base_getArgumentValue("DO_VAD_DEPENDENT_CMS");
	if (pszValue) {
		if (0 == PowerASR_Base_strnocasecmp(pszValue,"yes")) {
			pFXVar->bCMSwVAD = TRUE;
		}
		else {
			pFXVar->bCMSwVAD = FALSE;
		}
	}
	else {
		HCIMSG_WARN("[MFCC2FEAT] DO_VAD_DEPENDENT_CMS not assigned.\n");
		goto lb_err_mfcc2feat_loadConfig;
	}

	// flag to use silence cepstral mean vector
	if (pFXVar->bCMSwVAD == FALSE) {
		pszValue = PowerASR_Base_getArgumentValue("DO_USE_SILENCE_CEPMEAN");
		if (pszValue) {
			if (0 == PowerASR_Base_strnocasecmp(pszValue,"yes")) {
				pFXVar->bUseSilenceMean = TRUE;
			}
			else {
				pFXVar->bUseSilenceMean = FALSE;
			}
		}
		else {
			HCIMSG_WARN("[MFCC2FEAT] DO_USE_SILENCE_CEPMEAN not assigned.\n");
			goto lb_err_mfcc2feat_loadConfig;
		}
	}
	else {
		pFXVar->bUseSilenceMean = TRUE;
	}

	// window length for differential coefficients
	pszValue = PowerASR_Base_getArgumentValue("WINLEN_DIFFE");
	if (pszValue) {
		pFXVar->winDif = (hci_int16)atoi(pszValue);
	}
	else {
		HCIMSG_WARN("[MFCC2FEAT] WINLEN_DIFFE not assigned.\n");
		goto lb_err_mfcc2feat_loadConfig;
	}

	// window length for acceleration coefficients
	pszValue = PowerASR_Base_getArgumentValue("WINLEN_ACCEL");
	if (pszValue) {
		pFXVar->winAcc = (hci_int16)atoi(pszValue);
	}
	else {
		HCIMSG_WARN("[MFCC2FEAT] WINLEN_ACCEL not assigned.\n");
		goto lb_err_mfcc2feat_loadConfig;
	}

	// window length for 3rd differential coefficients
	pszValue = PowerASR_Base_getArgumentValue("WINLEN_THIRD_DIFF");
	if (pszValue) {
		pFXVar->win3rdDif = (hci_int16)atoi(pszValue);
	}
	else {
		HCIMSG_WARN("[MFCC2FEAT] WINLEN_THIRD_DIFF not assigned.\n");
		goto lb_err_mfcc2feat_loadConfig;
	}

	// window length for silence dropping
	if (pFXVar->bSilenceDrop) {
		pszValue = PowerASR_Base_getArgumentValue("WINLEN_DROPSIL");
		if (pszValue) {
			pFXVar->winSilDrop = (hci_int16)atoi(pszValue);
		}
		else {
			HCIMSG_WARN("[MFCC2FEAT] WINLEN_DROPSIL not assigned.\n");
			goto lb_err_mfcc2feat_loadConfig;
		}
	}
	else {
		pFXVar->winSilDrop = 0;
	}

	// static MFCC dimension
	pszValue = PowerASR_Base_getArgumentValue("DIM_MFCC");
	if (pszValue) {
		pFXVar->dimMFCC = (hci_int16)atoi(pszValue);
	}
	else {
		HCIMSG_WARN("[MFCC2FEAT] DIM_MFCC not assigned.\n");
		goto lb_err_mfcc2feat_loadConfig;
	}

	// feature dimension
	pszValue = PowerASR_Base_getArgumentValue("DIM_FEATURE");
	if (pszValue) {
		pFXVar->dimFeat = (hci_int16)atoi(pszValue);
		if ( pFXVar->dimFeat > DIM_FEATURE ) {
			HCIMSG_WARN("[MFCC2FEAT] DIM_FEATURE overflow (%d/%d).\n", pFXVar->dimFeat, DIM_FEATURE);
			goto lb_err_mfcc2feat_loadConfig;
		}
	}
	else {
		HCIMSG_WARN("[MFCC2FEAT] DIM_FEATURE not assigned.\n");
		goto lb_err_mfcc2feat_loadConfig;
	}

	// distance threshold in FC_AUTO_VFR
	if (pFXVar->typeFC == FC_AUTO_VFR) {
		pszValue = PowerASR_Base_getArgumentValue("DIST_THRESHOLD");
		if (pszValue) {
#ifdef FIXED_POINT_FE
			pFXVar->distThresh = (hci_mfcc32)atoi(pszValue);
#else	// !FIXED_POINT_FE
			pFXVar->distThresh = (hci_mfcc32)atof(pszValue);
#endif	// #ifdef FIXED_POINT_FE
		}
		else {
			HCIMSG_WARN("[MFCC2FEAT] DIST_THRESHOLD not assigned.\n");
			goto lb_err_mfcc2feat_loadConfig;
		}
	}
	
	// count of consecutive frame drops in FC_AUTO_VFR
	if (pFXVar->typeFC == FC_AUTO_VFR) {
		pszValue = PowerASR_Base_getArgumentValue("CONT_FRAME_DROP");
		if (pszValue) {
			pFXVar->nMaxFrameDrop = atoi(pszValue);
			pFXVar->nMaxFrameDrop = HCI_MIN(pFXVar->nMaxFrameDrop, SIZE_MFCC_HISTORY);
		}
		else {
			HCIMSG_WARN("[MFCC2FEAT] CONT_FRAME_DROP not assigned.\n");
			goto lb_err_mfcc2feat_loadConfig;
		}
	}

	if (pFXVar->typeCMS == RECUR_CMS || pFXVar->typeCMS == RECUR_CMVN) {

		// forgetting factor in recursive feature normalization (voiced regions)
		pszValue = PowerASR_Base_getArgumentValue("FORGET_FACTOR_VOICE");
		if (pszValue) {
#ifdef FIXED_POINT_FE
			pFXVar->forgetF_Voice = (hci_mfcc16)atoi(pszValue);
#else	// !FIXED_POINT_FE
			pFXVar->forgetF_Voice = (hci_mfcc16)atof(pszValue);
#endif	// #ifdef FIXED_POINT_FE
		}
		else {
			HCIMSG_WARN("[MFCC2FEAT] FORGET_FACTOR_VOICE not assigned.\n");
			goto lb_err_mfcc2feat_loadConfig;
		}

		// forgetting factor in recursive feature normalization (un-voiced regions)
		pszValue = PowerASR_Base_getArgumentValue("FORGET_FACTOR_UNVOICE");
		if (pszValue) {
#ifdef FIXED_POINT_FE
			pFXVar->forgetF_Unvoice = (hci_mfcc16)atoi(pszValue);
#else	// !FIXED_POINT_FE
			pFXVar->forgetF_Unvoice = (hci_mfcc16)atof(pszValue);
#endif	// #ifdef FIXED_POINT_FE
		}
		else {
			HCIMSG_WARN("[MFCC2FEAT] FORGET_FACTOR_UNVOICE not assigned.\n");
			goto lb_err_mfcc2feat_loadConfig;
		}
	}

	// cepstral mean vector seed file
	if (pFXVar->typeCMS == LIVE_CMS || pFXVar->typeCMS == LIVE_CMVN || pFXVar->typeEN == LIVE_EN 
		|| pFXVar->typeCMS == RECUR_CMS || pFXVar->typeCMS == RECUR_CMVN) {
		pszValue = PowerASR_Base_getArgumentValue("CEP_MEAN_FILE");
		if (pszValue) {
			//strcpy(pFXVar->szCepMeanFile, pszValue);
			sprintf(pFXVar->szCepMeanFile, "%s%s", pszHomeDir, pszValue);
		}
		else {
			HCIMSG_WARN("[MFCC2FEAT] CEP_MEAN_FILE not assigned.\n");
			goto lb_err_mfcc2feat_loadConfig;
		}
	}

	// MFCC quantizer file
	if (1 == sizeof(hci_asr_feat_t)) {		// only if use quantized feature stream
		pszValue = PowerASR_Base_getArgumentValue("MFCC_QUANTIZER_FILE");
		if (pszValue) {
			//strcpy(pFXVar->szMfccQuantFile, pszValue);
			sprintf(pFXVar->szMfccQuantFile, "%s%s", pszHomeDir, pszValue);
		}
		else {
			HCIMSG_WARN("[MFCC2FEAT] MFCC_QUANTIZER_FILE not assigned.\n");
			goto lb_err_mfcc2feat_loadConfig;
		}
	}

	// MFCC CMS_ProfileList file
	
	pszValue = PowerASR_Base_getArgumentValue("CMS_PROFILES_LIST");
	if (pszValue) {
		//strcpy(pFXVar->szCepMeanFile, pszValue);
		sprintf(pFXVar->szCMS_ProfileListFName, "%s%s", pszHomeDir, pszValue);
	}
	else {
		HCIMSG_WARN("[MFCC2FEAT] CMS_PROFILES_LIST not assigned.\n");
		goto lb_err_mfcc2feat_loadConfig;
	}
	

	PowerASR_Base_closeConfigurations();

//	if ((pFXVar->typeCMS == LIVE_CMS || pFXVar->typeCMS == LIVE_CMVN || pFXVar->typeEN == LIVE_EN || pFXVar->typeCMS == RECUR_CMS || pFXVar->typeCMS == RECUR_CMVN) &&
	if ((pFXVar->typeCMS == BATCH_CMS || pFXVar->typeCMS == LIVE_CMS || pFXVar->typeCMS == LIVE_CMVN || pFXVar->typeEN == LIVE_EN || pFXVar->typeCMS == RECUR_CMS || pFXVar->typeCMS == RECUR_CMVN) &&
		FX_CMS_loadSeedCepstralMeanVector(&pFXVar->seedFeatNorm, pFXVar->szCepMeanFile, pFXVar->dimMFCC, pFXVar->typeCMS) == -1) {
		HCIMSG_ERROR("seedCepMeanFile failed (%s).\n", pFXVar->szCepMeanFile);
		return -1;
	}


	//bgy
	if ((pFXVar->typeCMS == BATCH_CMS || pFXVar->typeCMS == LIVE_CMS || pFXVar->typeCMS == LIVE_CMVN || pFXVar->typeEN == LIVE_EN || pFXVar->typeCMS == RECUR_CMS || pFXVar->typeCMS == RECUR_CMVN)) {
		hci_int32 ret = FX_CMS_loadCmsProfileList(
			pFXVar->seedFeatNorm_ForCmsProfileType, pFXVar->szCMS_ProfileListFName, pszHomeDir,
			(char **)pFXVar->szCMS_ProfileModelNameList, (char **)pFXVar->szCMS_ProfileCmsVecFNameList,
			&pFXVar->numCmsModels, &pFXVar->flagCmsProfile, pFXVar->dimMFCC, pFXVar->typeCMS);
		if (0 != ret) {
			HCIMSG_ERROR("seedCepMeanFile failed (%s).\n", pFXVar->szCepMeanFile);
			return -1;
		}
	}//KSH

	com_count=pFXVar->numCmsModels;
	memcpy(cos_model_list,&pFXVar->szCMS_ProfileModelNameList[0][0],100*256);

	if (1 == sizeof(hci_asr_feat_t)) {
		if (-1 == FX_QUANTIZER_loadFeatureQuantizer(pFXVar->szMfccQuantFile,
													&pFXVar->mfccQuantizer,
													pFXVar->dimFeat)) {
			HCIMSG_ERROR("loadFeatureQuantizer failed (%s).\n", pFXVar->szMfccQuantFile);
			return -1;
		}
	}

	pFXVar->bLiveMode = TRUE;
	if (BATCH_CMS == pFXVar->typeCMS || BATCH_EN == pFXVar->typeEN) {
		pFXVar->bLiveMode = FALSE;
	}

	if (pFXVar->bUse3rdDif) {
		pFXVar->nFXDelay = pFXVar->win3rdDif;
	}
	else if (pFXVar->bUseAcc) {
		pFXVar->nFXDelay = pFXVar->winAcc;
	}
	else if (pFXVar->bUseDif) {
		pFXVar->nFXDelay = pFXVar->winDif;
	}
	else {
		pFXVar->nFXDelay = 0;
	}
	
	if (pFXVar->bSilenceDrop) {
		pFXVar->nFXDelay = HCI_MAX(pFXVar->nFXDelay, pFXVar->winSilDrop);
	}

	return 0;

lb_err_mfcc2feat_loadConfig:

	PowerASR_Base_closeConfigurations();

	return -1;
}

int GetCMSList(char * arr){
	int i=0;
	for(i=0;i<com_count;i++)
	{
		memcpy(arr+256*i,cos_model_list[i],256);
		
	}
	
	return com_count;
}

// end of file
