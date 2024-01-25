
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
 *	@file	fx_cms.c
 *	@ingroup mfcc2feat_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	CMS (cepstral mean subtraction) library
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "base/hci_msg.h"
#include "basic_op/basic_op.h"
#include "mfcc2feat/fx_cms.h"

//#define CMV_ASCII_INPUT	1		// if cepstral mean vectors are saved in binary format, don't define CMV_ASCII_INPUT
#define SAVE_CMV

#define CMN_WIN			(hci_int16)500				///< window length to update feature normalization vectors in VAD-independent mode
#define CMN_WIN_FULL	(hci_int16)(2*CMN_WIN)		///< window length to update feature normalization vectors in VAD-dependent mode
#define CMN_UPDATE_COUNT	(hci_int16)3000			///< maximum window length for updating feature normalization vectors to prevent overflows !!!

/**
 *	load seed cepstral mean vector & max log-energy
 */
hci_int32
FX_CMS_loadCmsProfileList(
							FEAT_Normalizer *pSeedFeatNorm_ForCmsProfileType,
							char *szCMS_ProfileListFName,//KSH
							const char pszHomeDir[],
							char szCMS_ProfileModelList[][256],
							char szCMS_ProfileCmsVecFNameList[][256],
							int *pNumCmsModels,
							int *pFlagCmsProfile,
							const hci_int16 dimMFCC,
							CMS_Type typeCMS 
							) {
	FILE *cmsProfileListFName_r;
	hci_int16 dimCepstrum = dimMFCC - 1;
	int numCmsProfiles;

	*pFlagCmsProfile = TRUE;
	cmsProfileListFName_r = fopen(szCMS_ProfileListFName,"rt");
	if(cmsProfileListFName_r == NULL)
	{
		printf("Unable to open %s\n",szCMS_ProfileListFName);
		printf("Default CMS Vector is applicated..\n");
		*pFlagCmsProfile = FALSE;
	}
	if(*pFlagCmsProfile == TRUE)
	{
		numCmsProfiles = 0;
		for(;;)
		{
			if(fscanf(cmsProfileListFName_r,"%s %s",szCMS_ProfileModelList[numCmsProfiles],szCMS_ProfileCmsVecFNameList[numCmsProfiles]) == -1) break;
			numCmsProfiles++;
		}
		*pNumCmsModels = numCmsProfiles;
		fclose(cmsProfileListFName_r);

#ifdef CMV_ASCII_INPUT
		hci_int16 i = 0;
		char szTemp[2][256];
		hci_int32 itmp = 0;

		if ((fpCMV=fopen(szFeatNormFile,"rt")) == 0) {
			HCIMSG_ERROR("cannot open CepMean file (%s).\n", szFeatNormFile);
			return -1;
		}

		// load seed cepstral mean vector for non-speech period
		fscanf(fpCMV, "%s%s", szTemp[0], szTemp[1]);
		for (i = 0; i < dimCepstrum; i++) {
#ifdef FIXED_POINT_FE
			fscanf(fpCMV, "%d", &(pSeedFeatNorm->cepMeanSilence[i]));
#else	// !FIXED_POINT_FE
			fscanf(fpCMV, "%f", &(pSeedFeatNorm->cepMeanSilence[i]));
#endif	// #ifdef FIXED_POINT_FE
		}

		// load seed cepstral mean vector for unvoiced-speech period
		fscanf(fpCMV, "%s%s", szTemp[0], szTemp[1]);
		for (i = 0; i < dimCepstrum; i++) {
#ifdef FIXED_POINT_FE
			fscanf(fpCMV, "%d", &(pSeedFeatNorm->cepMeanUnvoiced[i]));
#else	// !FIXED_POINT_FE
			fscanf(fpCMV, "%f", &(pSeedFeatNorm->cepMeanUnvoiced[i]));
#endif	// #ifdef FIXED_POINT_FE
		}
	
		// load seed cepstral mean vector for voiced-speech period
		fscanf(fpCMV, "%s%s", szTemp[0], szTemp[1]);
		for (i = 0; i < dimCepstrum; i++) {
#ifdef FIXED_POINT_FE
			fscanf(fpCMV, "%d", &(pSeedFeatNorm->cepMeanVoiced[i]));
#else	// !FIXED_POINT_FE
			fscanf(fpCMV, "%f", &(pSeedFeatNorm->cepMeanVoiced[i]));
#endif	// #ifdef FIXED_POINT_FE
		}

		// load seed cepstral inverse standard deviation vector for non-speech period
		fscanf(fpCMV, "%s%s", szTemp[0], szTemp[1]);
		for (i = 0; i < dimCepstrum; i++) {
#ifdef FIXED_POINT_FE
			fscanf(fpCMV, "%d", &(pSeedFeatNorm->cepVarSilence[i]));
#else	// !FIXED_POINT_FE
			fscanf(fpCMV, "%f", &(pSeedFeatNorm->cepVarSilence[i]));
#endif	// #ifdef FIXED_POINT_FE
		}

		// load seed cepstral inverse standard deviation vector for unvoiced-speech period
		fscanf(fpCMV, "%s%s", szTemp[0], szTemp[1]);
		for (i = 0; i < dimCepstrum; i++) {
#ifdef FIXED_POINT_FE
			fscanf(fpCMV, "%d", &(pSeedFeatNorm->cepVarUnvoiced[i]));
#else	// !FIXED_POINT_FE
			fscanf(fpCMV, "%f", &(pSeedFeatNorm->cepVarUnvoiced[i]));
#endif	// #ifdef FIXED_POINT_FE
		}
	
		// load seed cepstral inverse standard deviation vector for voiced-speech period
		fscanf(fpCMV, "%s%s", szTemp[0], szTemp[1]);
		for (i = 0; i < dimCepstrum; i++) {
#ifdef FIXED_POINT_FE
			fscanf(fpCMV, "%d", &(pSeedFeatNorm->cepVarVoiced[i]));
#else	// !FIXED_POINT_FE
			fscanf(fpCMV, "%f", &(pSeedFeatNorm->cepVarVoiced[i]));
#endif	// #ifdef FIXED_POINT_FE
		}

		// load seed max log-energy
		fscanf(fpCMV, "%s%s", szTemp[0], szTemp[1]);
#ifdef FIXED_POINT_FE
		fscanf(fpCMV, "%d", &(pSeedFeatNorm->maxLogE));
#else	// !FIXED_POINT_FE
		fscanf(fpCMV, "%f", &(pSeedFeatNorm->maxLogE));
#endif	// #ifdef FIXED_POINT_FE

		// load seed min log-energy
		fscanf(fpCMV, "%s%s", szTemp[0], szTemp[1]);
#ifdef FIXED_POINT_FE
		fscanf(fpCMV, "%d", &(pSeedFeatNorm->minLogE));
#else	// !FIXED_POINT_FE
		fscanf(fpCMV, "%f", &(pSeedFeatNorm->minLogE));
#endif	// #ifdef FIXED_POINT_FE

		fclose(fpCMV);

#else	// !CMV_ASCII_INPUT
		for (int n = 0; n<numCmsProfiles; n++)
		{
			FILE *fpCMV = NULL;
			fpCMV = fopen(szCMS_ProfileCmsVecFNameList[n], "rb");
			if (NULL == fpCMV) {	// try to load from relative path
				char szAbsPath[512];
				sprintf(szAbsPath, "%s/%s", pszHomeDir, szCMS_ProfileCmsVecFNameList[n]);
				fpCMV = fopen(szAbsPath, "rb");
			}

			if (NULL == fpCMV) {
				HCIMSG_ERROR("cannot open CepMean file (%s).\n", szCMS_ProfileCmsVecFNameList);
				return -1;
			}

			size_t ret = fread(&pSeedFeatNorm_ForCmsProfileType[n], sizeof(FEAT_Normalizer), 1, fpCMV);
			if (1 != ret) {
				HCIMSG_ERROR("failed to load feature normalization vectors.\n");
				fclose(fpCMV);
				return -1;
			}

			fclose(fpCMV);	
		}
#endif	// #ifdef CMV_ASCII_INPUT
	}
	return 0;
}//KSH

hci_int32
FX_CMS_loadSeedCepstralMeanVector(FEAT_Normalizer *pSeedFeatNorm,	///< (o) feature normalizer data struct
								  const char *szFeatNormFile,		///< (i) feature normalizer file
								  const hci_int16 dimMFCC,			///< (i) MFCC feature dimension
								  CMS_Type typeCMS)					///< (i) feature normalization type
{
	FILE *fpCMV = 0;
	hci_int16 dimCepstrum = dimMFCC - 1;

#ifdef CMV_ASCII_INPUT
	hci_int16 i = 0;
	char szTemp[2][256];
	hci_int32 itmp = 0;

	if ((fpCMV=fopen(szFeatNormFile,"rt")) == 0) {
		HCIMSG_ERROR("cannot open CepMean file (%s).\n", szFeatNormFile);
		return -1;
	}

	// load seed cepstral mean vector for non-speech period
	fscanf(fpCMV, "%s%s", szTemp[0], szTemp[1]);
	for (i = 0; i < dimCepstrum; i++) {
#ifdef FIXED_POINT_FE
		fscanf(fpCMV, "%d", &(pSeedFeatNorm->cepMeanSilence[i]));
#else	// !FIXED_POINT_FE
		fscanf(fpCMV, "%f", &(pSeedFeatNorm->cepMeanSilence[i]));
#endif	// #ifdef FIXED_POINT_FE
	}

	// load seed cepstral mean vector for unvoiced-speech period
	fscanf(fpCMV, "%s%s", szTemp[0], szTemp[1]);
	for (i = 0; i < dimCepstrum; i++) {
#ifdef FIXED_POINT_FE
		fscanf(fpCMV, "%d", &(pSeedFeatNorm->cepMeanUnvoiced[i]));
#else	// !FIXED_POINT_FE
		fscanf(fpCMV, "%f", &(pSeedFeatNorm->cepMeanUnvoiced[i]));
#endif	// #ifdef FIXED_POINT_FE
	}
	
	// load seed cepstral mean vector for voiced-speech period
	fscanf(fpCMV, "%s%s", szTemp[0], szTemp[1]);
	for (i = 0; i < dimCepstrum; i++) {
#ifdef FIXED_POINT_FE
		fscanf(fpCMV, "%d", &(pSeedFeatNorm->cepMeanVoiced[i]));
#else	// !FIXED_POINT_FE
		fscanf(fpCMV, "%f", &(pSeedFeatNorm->cepMeanVoiced[i]));
#endif	// #ifdef FIXED_POINT_FE
	}

	// load seed cepstral inverse standard deviation vector for non-speech period
	fscanf(fpCMV, "%s%s", szTemp[0], szTemp[1]);
	for (i = 0; i < dimCepstrum; i++) {
#ifdef FIXED_POINT_FE
		fscanf(fpCMV, "%d", &(pSeedFeatNorm->cepVarSilence[i]));
#else	// !FIXED_POINT_FE
		fscanf(fpCMV, "%f", &(pSeedFeatNorm->cepVarSilence[i]));
#endif	// #ifdef FIXED_POINT_FE
	}

	// load seed cepstral inverse standard deviation vector for unvoiced-speech period
	fscanf(fpCMV, "%s%s", szTemp[0], szTemp[1]);
	for (i = 0; i < dimCepstrum; i++) {
#ifdef FIXED_POINT_FE
		fscanf(fpCMV, "%d", &(pSeedFeatNorm->cepVarUnvoiced[i]));
#else	// !FIXED_POINT_FE
		fscanf(fpCMV, "%f", &(pSeedFeatNorm->cepVarUnvoiced[i]));
#endif	// #ifdef FIXED_POINT_FE
	}
	
	// load seed cepstral inverse standard deviation vector for voiced-speech period
	fscanf(fpCMV, "%s%s", szTemp[0], szTemp[1]);
	for (i = 0; i < dimCepstrum; i++) {
#ifdef FIXED_POINT_FE
		fscanf(fpCMV, "%d", &(pSeedFeatNorm->cepVarVoiced[i]));
#else	// !FIXED_POINT_FE
		fscanf(fpCMV, "%f", &(pSeedFeatNorm->cepVarVoiced[i]));
#endif	// #ifdef FIXED_POINT_FE
	}

	// load seed max log-energy
	fscanf(fpCMV, "%s%s", szTemp[0], szTemp[1]);
#ifdef FIXED_POINT_FE
	fscanf(fpCMV, "%d", &(pSeedFeatNorm->maxLogE));
#else	// !FIXED_POINT_FE
	fscanf(fpCMV, "%f", &(pSeedFeatNorm->maxLogE));
#endif	// #ifdef FIXED_POINT_FE

	// load seed min log-energy
	fscanf(fpCMV, "%s%s", szTemp[0], szTemp[1]);
#ifdef FIXED_POINT_FE
	fscanf(fpCMV, "%d", &(pSeedFeatNorm->minLogE));
#else	// !FIXED_POINT_FE
	fscanf(fpCMV, "%f", &(pSeedFeatNorm->minLogE));
#endif	// #ifdef FIXED_POINT_FE

	fclose(fpCMV);

#else	// !CMV_ASCII_INPUT

	if ((fpCMV=fopen(szFeatNormFile,"rb")) == 0) {
		HCIMSG_ERROR("cannot open CepMean file (%s).\n", szFeatNormFile);
		return -1;
	}

	if (fread(pSeedFeatNorm, sizeof(FEAT_Normalizer), 1, fpCMV) != 1) {
		HCIMSG_ERROR("failed to load feature normalization vectors.\n");
		fclose(fpCMV);
		return -1;
	}

	fclose(fpCMV);

#endif	// #ifdef CMV_ASCII_INPUT

	if (typeCMS == RECUR_CMVN) {
		hci_mfcc_t *pNormMeanUnvoice = pSeedFeatNorm->cepMeanUnvoiced;
		hci_mfcc_t *pNormMeanVoice = pSeedFeatNorm->cepMeanVoiced;
		hci_mfcc_t *pNormMeanSil = pSeedFeatNorm->cepMeanSilence;
		hci_mfcc_t *pNormVarUnvoice = pSeedFeatNorm->cepVarUnvoiced;
		hci_mfcc_t *pNormVarVoice = pSeedFeatNorm->cepVarVoiced;
		hci_mfcc_t *pNormVarSil = pSeedFeatNorm->cepVarSilence;
		hci_mfcc_t *pLastVec = pSeedFeatNorm->cepMeanSilence + dimCepstrum;
#ifdef FIXED_POINT_FE
		hci_mfcc64 LL_val = 0;
		hci_mfcc32 L_val = 0;
#endif	// #ifdef FIXED_POINT_FE
		while (pNormMeanSil < pLastVec) {
#ifdef FIXED_POINT_FE
			L_val = PowerASR_BasicOP_divideShiftLeft_32_32(1, *pNormVarSil, 30);		// Q15.32 (=1/sigma)
			LL_val = (hci_mfcc64)L_val * (hci_mfcc64)L_val;								// Q30.64 (=1/sigma^2)
			LL_val += (hci_mfcc64)(*pNormMeanSil) * (hci_mfcc64)(*pNormMeanSil);		// Q30.64 (+ mean^2)
			*pNormVarSil = (hci_mfcc32)(LL_val>>15);									// Q15.32, mean(s^2)
			L_val = PowerASR_BasicOP_divideShiftLeft_32_32(1, *pNormVarVoice, 30);		// Q15.32 (=1/sigma)
			LL_val = (hci_mfcc64)L_val * (hci_mfcc64)L_val;								// Q30.64 (=1/sigma^2)
			LL_val += (hci_mfcc64)(*pNormMeanVoice) * (hci_mfcc64)(*pNormMeanVoice);	// Q30.64 (+ mean^2)
			*pNormVarVoice = (hci_mfcc32)(LL_val>>15);									// Q15.32, mean(s^2)
			L_val = PowerASR_BasicOP_divideShiftLeft_32_32(1, *pNormVarUnvoice, 30);		// Q15.32 (=1/sigma)
			LL_val = (hci_mfcc64)L_val * (hci_mfcc64)L_val;									// Q30.64 (=1/sigma^2)
			LL_val += (hci_mfcc64)(*pNormMeanUnvoice) * (hci_mfcc64)(*pNormMeanUnvoice);	// Q30.64 (+ mean^2)
			*pNormVarUnvoice = (hci_mfcc32)(LL_val>>15);									// Q15.32, mean(s^2)
#else	// !FIXED_POINT_FE
			*pNormVarSil = 1.0f / (float)HCI_SQ(*pNormVarSil) + HCI_SQ(*pNormMeanSil);	// mean(s^2)
			*pNormVarVoice = 1.0f / (float)HCI_SQ(*pNormVarVoice) + HCI_SQ(*pNormMeanVoice);
			*pNormVarUnvoice = 1.0f / (float)HCI_SQ(*pNormVarUnvoice) + HCI_SQ(*pNormMeanUnvoice);
#endif	// #ifdef FIXED_POINT_FE
			pNormMeanUnvoice++; pNormMeanVoice++; pNormMeanSil++;
			pNormVarUnvoice++;  pNormVarVoice++;  pNormVarSil++;
		}
	}

	return 0;
}


/**
 *	save cepstral mean vector & max log-energy
 */
hci_int32
FX_CMS_saveCepstralMeanVector(FEAT_Normalizer *pFeatNorm,			///< (i) feature normalizer
							  const char *pszCepMeanFile)			///< (i) feature normalization data file
{

#ifdef SAVE_CMV
	FILE *fpCMV = 0;

	if (0 == pFeatNorm) {
		return -1;
	}
	if (0 == pszCepMeanFile) {
		return -1;
	}
	
	if ((fpCMV=fopen(pszCepMeanFile,"wb")) == 0) {
		HCIMSG_ERROR("cannot create CepMean file (%s).\n", pszCepMeanFile);
		return -1;
	}

	fwrite(pFeatNorm, sizeof(FEAT_Normalizer), 1, fpCMV);
	
	fclose(fpCMV);

#endif

	return 0;
}

/**
 *	initialize accumulative terms of cepstral mean vectors for current utterance
 */
hci_int32
FX_CMS_initializeCepstrumMean(MFCC_Stream *pMfccStream,		///< (i/o) pointer to mfcc stream
							  const hci_int16 dimMFCC,		///< (i) MFCC feature dimension
							  CMS_Type typeCMS,				///< (i) feature normalization type
							  hci_int16 bCMSwVAD)			///< (i) flag to VAD-dependent CMS
{
	hci_mfcc32 L_val = 0;
	FEAT_Normalizer *pUserFeatNorm = 0;
	FEAT_Accumulator *pUserFeatAccum = 0;
	hci_int16 val_prior = 0;
	hci_int16 dimCepstrum = dimMFCC - 1;

	if (0 == pMfccStream) {
		return -1;
	}

	pUserFeatNorm  = &pMfccStream->userFeatNorm;
	pUserFeatAccum = &pMfccStream->userFeatAccum;

	switch (typeCMS) {
	case LIVE_CMS:
	case RECUR_CMS:
		{
			hci_mfcc_t *pAccmMeanUnvoice = pUserFeatAccum->cepMeanUnvoiced;
			hci_mfcc_t *pNormMeanUnvoice = pUserFeatNorm->cepMeanUnvoiced;
			hci_mfcc_t *pAccmMeanVoice = pUserFeatAccum->cepMeanVoiced;
			hci_mfcc_t *pNormMeanVoice = pUserFeatNorm->cepMeanVoiced;
			hci_mfcc_t *pAccmMeanSil = pUserFeatAccum->cepMeanSilence;
			hci_mfcc_t *pNormMeanSil = pUserFeatNorm->cepMeanSilence;
			hci_mfcc_t *pLastVec = pUserFeatNorm->cepMeanSilence + dimCepstrum;
			if (0 == bCMSwVAD) val_prior = CMN_WIN;
			else val_prior = CMN_WIN_FULL;
			pUserFeatAccum->countSilenceFrame  = val_prior;
			pUserFeatAccum->countVoicedFrame   = val_prior;
			pUserFeatAccum->countUnvoicedFrame = val_prior;
			while (pNormMeanSil < pLastVec) {
				*pAccmMeanSil++ = (hci_mfcc_t)val_prior * (*pNormMeanSil++);
				*pAccmMeanVoice++ = (hci_mfcc_t)val_prior * (*pNormMeanVoice++);
				*pAccmMeanUnvoice++ = (hci_mfcc_t)val_prior * (*pNormMeanUnvoice++);
			}
		}
		break;
	case LIVE_CMVN:
	case RECUR_CMVN:
		{
			hci_mfcc_t *pAccmMeanUnvoice = pUserFeatAccum->cepMeanUnvoiced;
			hci_mfcc_t *pNormMeanUnvoice = pUserFeatNorm->cepMeanUnvoiced;
			hci_mfcc_t *pAccmMeanVoice = pUserFeatAccum->cepMeanVoiced;
			hci_mfcc_t *pNormMeanVoice = pUserFeatNorm->cepMeanVoiced;
			hci_mfcc_t *pAccmMeanSil = pUserFeatAccum->cepMeanSilence;
			hci_mfcc_t *pNormMeanSil = pUserFeatNorm->cepMeanSilence;
			hci_mfcc64 *pAccmVarVoice = pUserFeatAccum->cepVarVoiced;
			hci_mfcc_t *pNormVarVoice = pUserFeatNorm->cepVarVoiced;
			hci_mfcc64 *pAccmVarUnvoice = pUserFeatAccum->cepVarUnvoiced;
			hci_mfcc_t *pNormVarUnvoice = pUserFeatNorm->cepVarUnvoiced;
			hci_mfcc64 *pAccmVarSil = pUserFeatAccum->cepVarSilence;
			hci_mfcc_t *pNormVarSil = pUserFeatNorm->cepVarSilence;
			hci_mfcc_t *pLastVec = pUserFeatNorm->cepMeanSilence + dimCepstrum;
			if (0 == bCMSwVAD) val_prior = CMN_WIN;
			else val_prior = CMN_WIN_FULL;
			pUserFeatAccum->countSilenceFrame  = val_prior;
			pUserFeatAccum->countVoicedFrame   = val_prior;
			pUserFeatAccum->countUnvoicedFrame = val_prior;
			while (pNormMeanSil < pLastVec) {
				*pAccmVarSil = (hci_mfcc64)(*pNormMeanSil) * (hci_mfcc64)(*pNormMeanSil);
#ifdef FIXED_POINT_FE
				L_val = PowerASR_BasicOP_divideShiftLeft_32_32(32768, *pNormVarSil, 15);
#else	// !FIXED_POINT_FE
				L_val = 1.0f / (*pNormVarSil);
#endif	// #ifdef FIXED_POINT_FE
				*pAccmVarSil += (hci_mfcc64)L_val * (hci_mfcc64)L_val;
				*pAccmVarSil *= (hci_mfcc64)val_prior;
				*pAccmVarVoice = (hci_mfcc64)(*pNormMeanVoice) * (hci_mfcc64)(*pNormMeanVoice);
#ifdef FIXED_POINT_FE
				L_val = PowerASR_BasicOP_divideShiftLeft_32_32(32768, *pNormVarVoice, 15);
#else	// !FIXED_POINT_FE
				L_val = 1.0f / (*pNormVarVoice);
#endif	// #ifdef FIXED_POINT_FE
				*pAccmVarVoice += (hci_mfcc64)L_val * (hci_mfcc64)L_val;
				*pAccmVarVoice *= (hci_mfcc64)val_prior;
				*pAccmVarUnvoice = (hci_mfcc64)(*pNormMeanUnvoice) * (hci_mfcc64)(*pNormMeanUnvoice);
#ifdef FIXED_POINT_FE
				L_val = PowerASR_BasicOP_divideShiftLeft_32_32(32768, *pNormVarUnvoice, 15);
#else	// !FIXED_POINT_FE
				L_val = 1.0f / (*pNormVarUnvoice);
#endif	// #ifdef FIXED_POINT_FE
				*pAccmVarUnvoice += (hci_mfcc64)L_val * (hci_mfcc64)L_val;
				*pAccmVarUnvoice *= (hci_mfcc64)val_prior;
				*pAccmMeanSil++ = (hci_mfcc_t)val_prior * (*pNormMeanSil++);
				*pAccmMeanVoice++ = (hci_mfcc_t)val_prior * (*pNormMeanVoice++);
				*pAccmMeanUnvoice++ = (hci_mfcc_t)val_prior * (*pNormMeanUnvoice++);
				pAccmVarVoice++; pNormVarVoice++;
				pAccmVarUnvoice++; pNormVarUnvoice++;
				pAccmVarSil++; pNormVarSil++;
			}
		}
		break;
 	case BATCH_CMS:
		pUserFeatAccum->countSilenceFrame  = 0;
		pUserFeatAccum->countVoicedFrame   = 0;
		pUserFeatAccum->countUnvoicedFrame = 0;
		memset(pUserFeatAccum->cepMeanSilence, 0, sizeof(pUserFeatAccum->cepMeanSilence));
		memset(pUserFeatAccum->cepMeanVoiced, 0, sizeof(pUserFeatAccum->cepMeanVoiced));
		memset(pUserFeatAccum->cepMeanUnvoiced, 0, sizeof(pUserFeatAccum->cepMeanUnvoiced));
		break;
	case BATCH_CMVN:
		pUserFeatAccum->countSilenceFrame = 0;
		pUserFeatAccum->countVoicedFrame   = 0;
		pUserFeatAccum->countUnvoicedFrame = 0;
		memset(pUserFeatAccum->cepMeanSilence, 0, sizeof(pUserFeatAccum->cepMeanSilence));
		memset(pUserFeatAccum->cepMeanVoiced, 0, sizeof(pUserFeatAccum->cepMeanVoiced));
		memset(pUserFeatAccum->cepMeanUnvoiced, 0, sizeof(pUserFeatAccum->cepMeanUnvoiced));
		memset(pUserFeatAccum->cepVarSilence, 0, sizeof(pUserFeatAccum->cepVarSilence));
		memset(pUserFeatAccum->cepVarVoiced, 0, sizeof(pUserFeatAccum->cepVarVoiced));
		memset(pUserFeatAccum->cepVarUnvoiced, 0, sizeof(pUserFeatAccum->cepVarUnvoiced));
		break;
	default:
		break;
	}

	return 0;
}

/**
 *	live-mode CMS
 */
hci_int32
FX_CMS_liveCMS(MFCC_Stream *pMfccStream,		///< (i/o) pointer to mfcc stream
			   MFCC_Cell *pCurCell,				///< (i) input MFCC cell
			   MFCC_Cell *pOutCell,				///< (o) output MFCC cell
			   Mfcc2FeatParameters *pFXVar)		///< (i) config. struct for mfcc-to-feature converter
{
	FEAT_Normalizer *pUserFeatNorm = 0;
	FEAT_Accumulator *pUserFeatAccum = 0;
	hci_mfcc_t *pInputMfcc = 0;
	hci_mfcc_t *pOutputMfcc = 0;
	hci_mfcc_t *pLastMFCC = 0;
	FrameClass frame_class = SIL_FRAME;
	CMS_Type typeCMS = pFXVar->typeCMS;

	pUserFeatNorm  = &pMfccStream->userFeatNorm;
	pUserFeatAccum = &pMfccStream->userFeatAccum;

	if (pCurCell) {
		pInputMfcc = pCurCell->mfccVec;
		pLastMFCC = pInputMfcc + pFXVar->dimMFCC - 1;
		frame_class = pCurCell->frame_class;
	}
	if (pOutCell) {
		pOutputMfcc = pOutCell->normVec;
	}

	// accumulate statistics of cepstral mean vector
	switch (typeCMS) {
	case LIVE_CMS:		// live-mode CMS with prior normalization vector
		if (SIL_FRAME == frame_class) {
			hci_mfcc_t *pAccMean = pUserFeatAccum->cepMeanSilence;
			hci_mfcc_t *pCepMean = pUserFeatNorm->cepMeanSilence;
			pUserFeatAccum->countSilenceFrame = PowerASR_BasicOP_add_32_32(pUserFeatAccum->countSilenceFrame, 2);
			while (pInputMfcc < pLastMFCC) {
				*pAccMean += 2 * (*pInputMfcc);
				*pOutputMfcc -= *pCepMean;
				pInputMfcc++; pOutputMfcc++; pAccMean++; pCepMean++;
			}
		}
		else if (UNVOICED_FRAME == frame_class) {
			hci_mfcc_t *pAccMeanSIL = pUserFeatAccum->cepMeanSilence;
			hci_mfcc_t *pCepMeanSIL = pUserFeatNorm->cepMeanSilence;
			hci_mfcc_t *pAccMeanSPCH = pUserFeatAccum->cepMeanUnvoiced;
			hci_mfcc_t *pCepMeanSPCH = pUserFeatNorm->cepMeanUnvoiced;
			pUserFeatAccum->countSilenceFrame = PowerASR_BasicOP_add_32_32(pUserFeatAccum->countSilenceFrame, 1);
			pUserFeatAccum->countUnvoicedFrame = PowerASR_BasicOP_add_32_32(pUserFeatAccum->countUnvoicedFrame, 2);
			while (pInputMfcc < pLastMFCC) {
				*pAccMeanSIL += (*pInputMfcc);
				*pAccMeanSPCH += 2 * (*pInputMfcc);
				*pOutputMfcc -= *pCepMeanSPCH;
				pInputMfcc++; pOutputMfcc++;
				pAccMeanSIL++; pAccMeanSPCH++;
				pCepMeanSIL++; pCepMeanSPCH++;
			}
		}
		else if (VOICED_FRAME == frame_class) {
			hci_mfcc_t *pAccMean = pUserFeatAccum->cepMeanVoiced;
			hci_mfcc_t *pCepMean = pUserFeatNorm->cepMeanVoiced;
			pUserFeatAccum->countVoicedFrame = PowerASR_BasicOP_add_32_32(pUserFeatAccum->countVoicedFrame, 2);
			while (pInputMfcc < pLastMFCC) {
				*pAccMean += 2 * (*pInputMfcc);
				*pOutputMfcc -= *pCepMean;
				pInputMfcc++; pOutputMfcc++; pAccMean++; pCepMean++;
			}
		}
		else {	// mixed frame
			hci_mfcc_t *pAccMeanSIL = pUserFeatAccum->cepMeanUnvoiced;
			hci_mfcc_t *pCepMeanSIL = pUserFeatNorm->cepMeanUnvoiced;
			hci_mfcc_t *pAccMeanSPCH = pUserFeatAccum->cepMeanVoiced;
			hci_mfcc_t *pCepMeanSPCH = pUserFeatNorm->cepMeanVoiced;
			pUserFeatAccum->countUnvoicedFrame = PowerASR_BasicOP_add_32_32(pUserFeatAccum->countUnvoicedFrame, 1);
			pUserFeatAccum->countVoicedFrame = PowerASR_BasicOP_add_32_32(pUserFeatAccum->countVoicedFrame, 1);
			while (pInputMfcc < pLastMFCC) {
				*pAccMeanSIL += (*pInputMfcc);
				*pAccMeanSPCH += (*pInputMfcc);
				*pOutputMfcc *= 2;
				*pOutputMfcc -= *pCepMeanSIL;
				*pOutputMfcc -= *pCepMeanSPCH;
#ifdef FIXED_POINT_FE
				*pOutputMfcc = PowerASR_BasicOP_shiftRight_32_32(*pOutputMfcc, 1);
#else	// !FIXED_POINT_FE
				*pOutputMfcc *= 0.5f;
#endif	// #ifdef FIXED_POINT_FE
				pInputMfcc++; pOutputMfcc++;
				pAccMeanSIL++; pAccMeanSPCH++;
				pCepMeanSIL++; pCepMeanSPCH++;
			}
		}
		if (pUserFeatAccum->countSilenceFrame >= CMN_UPDATE_COUNT) {
			FX_CMS_updateCepstralMeanVector(pMfccStream, pFXVar);
			FX_CMS_initializeCepstrumMean(pMfccStream, pFXVar->dimMFCC, pFXVar->typeCMS, pFXVar->bCMSwVAD);
		}
		break;
	case LIVE_CMVN:		// live-mode CMVN with prior normalization vector
		if (SIL_FRAME == frame_class) {
			hci_mfcc64 L_val = 0;
			hci_mfcc_t *pAccMean = pUserFeatAccum->cepMeanSilence;
			hci_mfcc_t *pCepMean = pUserFeatNorm->cepMeanSilence;
			hci_mfcc64 *pAccVar = pUserFeatAccum->cepVarSilence;
			hci_mfcc_t *pCepVar = pUserFeatNorm->cepVarSilence;
			pUserFeatAccum->countSilenceFrame = PowerASR_BasicOP_add_32_32(pUserFeatAccum->countSilenceFrame, 2);
			while (pInputMfcc < pLastMFCC) {
				*pAccMean += 2 * (*pInputMfcc);
				L_val = 2 * (hci_mfcc64)(*pInputMfcc) * (hci_mfcc64)(*pInputMfcc);
				*pAccVar  += L_val;
				L_val = (hci_mfcc64) ((*pOutputMfcc) - (*pCepMean));		// Q15.64
				L_val *= (*pCepVar);										// Q30.64
#ifdef FIXED_POINT_FE
				*pOutputMfcc = (hci_mfcc_t)(L_val>>15);						// Q15.32
#else	// !FIXED_POINT_FE
				*pOutputMfcc = (hci_mfcc_t)L_val;
#endif	// #ifdef FIXED_POINT_FE
				pInputMfcc++; pOutputMfcc++;
				pAccMean++; pCepMean++;
				pAccVar++; pCepVar++;
			}
		}
		else if (UNVOICED_FRAME == frame_class) {	// unvoiced frame
			hci_mfcc64 L_val = 0, L_val2 = 0;
			hci_mfcc_t *pAccMeanSIL = pUserFeatAccum->cepMeanSilence;
			hci_mfcc_t *pCepMeanSIL = pUserFeatNorm->cepMeanSilence;
			hci_mfcc_t *pAccMeanSPCH = pUserFeatAccum->cepMeanUnvoiced;
			hci_mfcc_t *pCepMeanSPCH = pUserFeatNorm->cepMeanUnvoiced;
			hci_mfcc64 *pAccVarSIL = pUserFeatAccum->cepVarSilence;
			hci_mfcc_t *pCepVarSIL = pUserFeatNorm->cepVarSilence;
			hci_mfcc64 *pAccVarSPCH = pUserFeatAccum->cepVarUnvoiced;
			hci_mfcc_t *pCepVarSPCH = pUserFeatNorm->cepVarUnvoiced;
			pUserFeatAccum->countSilenceFrame = PowerASR_BasicOP_add_32_32(pUserFeatAccum->countSilenceFrame, 1);
			pUserFeatAccum->countUnvoicedFrame = PowerASR_BasicOP_add_32_32(pUserFeatAccum->countUnvoicedFrame, 2);
			while (pInputMfcc < pLastMFCC) {
				*pAccMeanSIL  += *pInputMfcc;
				*pAccMeanSPCH += 2 * (*pInputMfcc);
				L_val = (hci_mfcc64)(*pInputMfcc) * (hci_mfcc64)(*pInputMfcc);
				*pAccVarSIL  += L_val;
				*pAccVarSPCH += 2 * L_val;
				L_val = (hci_mfcc64) ((*pOutputMfcc) - (*pCepMeanSPCH));		// Q15.64
				L_val *= (*pCepVarSPCH);										// Q30.64
#ifdef FIXED_POINT_FE
				*pOutputMfcc = (hci_mfcc_t)(L_val>>15);						// Q15.32
#else	// !FIXED_POINT_FE
				*pOutputMfcc = (hci_mfcc_t)L_val;
#endif	// #ifdef FIXED_POINT_FE
				pInputMfcc++; pOutputMfcc++;
				pAccMeanSIL++; pCepMeanSIL++;
				pAccMeanSPCH++; pCepMeanSPCH++;
				pAccVarSIL++; pCepVarSIL++;
				pAccVarSPCH++; pCepVarSPCH++;
			}
		}
		else if (VOICED_FRAME == frame_class) {
			hci_mfcc64 L_val = 0;
			hci_mfcc_t *pAccMean = pUserFeatAccum->cepMeanVoiced;
			hci_mfcc_t *pCepMean = pUserFeatNorm->cepMeanVoiced;
			hci_mfcc64 *pAccVar = pUserFeatAccum->cepVarVoiced;
			hci_mfcc_t *pCepVar = pUserFeatNorm->cepVarVoiced;
			pUserFeatAccum->countVoicedFrame = PowerASR_BasicOP_add_32_32(pUserFeatAccum->countVoicedFrame, 2);
			while (pInputMfcc < pLastMFCC) {
				*pAccMean += 2 * (*pInputMfcc);
				L_val = 2 * (hci_mfcc64)(*pInputMfcc) * (hci_mfcc64)(*pInputMfcc);
				*pAccVar  += L_val;
				L_val = (hci_mfcc64) ((*pOutputMfcc) - (*pCepMean));		// Q15.64
				L_val *= (*pCepVar);										// Q30.64
#ifdef FIXED_POINT_FE
				*pOutputMfcc = (hci_mfcc_t)(L_val>>15);						// Q15.32
#else	// !FIXED_POINT_FE
				*pOutputMfcc = (hci_mfcc_t)L_val;
#endif	// #ifdef FIXED_POINT_FE
				pInputMfcc++; pOutputMfcc++;
				pAccMean++; pCepMean++;
				pAccVar++; pCepVar++;
			}
		}
		else {	// mixed frame
			hci_mfcc64 L_val = 0, L_val2 = 0;
			hci_mfcc_t *pAccMeanSIL = pUserFeatAccum->cepMeanUnvoiced;
			hci_mfcc_t *pCepMeanSIL = pUserFeatNorm->cepMeanUnvoiced;
			hci_mfcc_t *pAccMeanSPCH = pUserFeatAccum->cepMeanVoiced;
			hci_mfcc_t *pCepMeanSPCH = pUserFeatNorm->cepMeanVoiced;
			hci_mfcc64 *pAccVarSIL = pUserFeatAccum->cepVarUnvoiced;
			hci_mfcc_t *pCepVarSIL = pUserFeatNorm->cepVarUnvoiced;
			hci_mfcc64 *pAccVarSPCH = pUserFeatAccum->cepVarVoiced;
			hci_mfcc_t *pCepVarSPCH = pUserFeatNorm->cepVarVoiced;
			pUserFeatAccum->countUnvoicedFrame = PowerASR_BasicOP_add_32_32(pUserFeatAccum->countUnvoicedFrame, 1);
			pUserFeatAccum->countVoicedFrame = PowerASR_BasicOP_add_32_32(pUserFeatAccum->countVoicedFrame, 1);
			while (pInputMfcc < pLastMFCC) {
				*pAccMeanSIL  += *pInputMfcc;
				*pAccMeanSPCH += *pInputMfcc;
				L_val = (hci_mfcc64)(*pInputMfcc) * (hci_mfcc64)(*pInputMfcc);
				*pAccVarSIL  += L_val;
				*pAccVarSPCH += L_val;
				L_val = (hci_mfcc64) ((*pOutputMfcc) - (*pCepMeanSIL));		// Q15.64
				L_val *= (*pCepVarSIL);										// Q30.64
				L_val2 = (hci_mfcc64) ((*pOutputMfcc) - (*pCepMeanSPCH));	// Q15.64
				L_val2 *= (*pCepVarSPCH);									// Q30.64
				L_val += L_val2;											// Q30.64
#ifdef FIXED_POINT_FE
				*pOutputMfcc = (hci_mfcc_t)(L_val>>16);						// Q15.32 (*0.5)
#else	// !FIXED_POINT_FE
				*pOutputMfcc = (hci_mfcc_t)(0.5f*L_val);
#endif	// #ifdef FIXED_POINT_FE
				pInputMfcc++; pOutputMfcc++;
				pAccMeanSIL++; pCepMeanSIL++;
				pAccMeanSPCH++; pCepMeanSPCH++;
				pAccVarSIL++; pCepVarSIL++;
				pAccVarSPCH++; pCepVarSPCH++;
			}
		}
		if (pUserFeatAccum->countSilenceFrame >= CMN_UPDATE_COUNT) {
			FX_CMS_updateCepstralMeanVector(pMfccStream, pFXVar);
			FX_CMS_initializeCepstrumMean(pMfccStream, pFXVar->dimMFCC, pFXVar->typeCMS, pFXVar->bCMSwVAD);
		}
		break;
	case BATCH_CMS:		// batch-mode CMS
		if (SIL_FRAME == frame_class) {
			hci_mfcc_t *pAccMean = pUserFeatAccum->cepMeanSilence;
			pUserFeatAccum->countSilenceFrame = PowerASR_BasicOP_add_32_32(pUserFeatAccum->countSilenceFrame, 2);
			while (pInputMfcc < pLastMFCC) {
				*pAccMean += 2 * (*pInputMfcc);
				pInputMfcc++; pAccMean++;
			}
		}
		else if (UNVOICED_FRAME == frame_class) {
			hci_mfcc_t *pAccMeanSIL = pUserFeatAccum->cepMeanSilence;
			hci_mfcc_t *pAccMeanSPCH = pUserFeatAccum->cepMeanUnvoiced;
            pUserFeatAccum->countSilenceFrame = PowerASR_BasicOP_add_32_32(pUserFeatAccum->countSilenceFrame, 1);
            pUserFeatAccum->countUnvoicedFrame = PowerASR_BasicOP_add_32_32(pUserFeatAccum->countUnvoicedFrame, 2);
			while (pInputMfcc < pLastMFCC) {
				*pAccMeanSIL += (*pInputMfcc);
				*pAccMeanSPCH += 2 * (*pInputMfcc);
				pInputMfcc++; pAccMeanSIL++; pAccMeanSPCH++;
			}
		}
		else if (VOICED_FRAME == frame_class) {
			hci_mfcc_t *pAccMean = pUserFeatAccum->cepMeanVoiced;
            pUserFeatAccum->countVoicedFrame = PowerASR_BasicOP_add_32_32(pUserFeatAccum->countVoicedFrame, 2);
			while (pInputMfcc < pLastMFCC) {
				*pAccMean += 2 * (*pInputMfcc);
				pInputMfcc++; pAccMean++;
			}
		}
		else {	// mixed frame
			hci_mfcc_t *pAccMeanSIL = pUserFeatAccum->cepMeanUnvoiced;
			hci_mfcc_t *pAccMeanSPCH = pUserFeatAccum->cepMeanVoiced;
            pUserFeatAccum->countUnvoicedFrame = PowerASR_BasicOP_add_32_32(pUserFeatAccum->countUnvoicedFrame, 1);
            pUserFeatAccum->countVoicedFrame = PowerASR_BasicOP_add_32_32(pUserFeatAccum->countVoicedFrame, 1);
			while (pInputMfcc < pLastMFCC) {
				*pAccMeanSIL += (*pInputMfcc);
				*pAccMeanSPCH += (*pInputMfcc);
				pInputMfcc++; pAccMeanSIL++; pAccMeanSPCH++;
			}
		}
		break;
	case BATCH_CMVN:		// batch-mode CMVN
		if (SIL_FRAME == frame_class) {
			hci_mfcc64 L_val = 0;
			hci_mfcc_t *pAccMean = pUserFeatAccum->cepMeanSilence;
			hci_mfcc64 *pAccVar = pUserFeatAccum->cepVarSilence;
            pUserFeatAccum->countSilenceFrame = PowerASR_BasicOP_add_32_32(pUserFeatAccum->countSilenceFrame, 2);
			while (pInputMfcc < pLastMFCC) {
				*pAccMean += 2 * (*pInputMfcc);
				L_val = 2 * (hci_mfcc64)(*pInputMfcc) * (hci_mfcc64)(*pInputMfcc);
				*pAccVar += L_val;
				pInputMfcc++; pAccMean++; pAccVar++;
			}
		}
		else if (UNVOICED_FRAME == frame_class) {
			hci_mfcc64 L_val = 0;
			hci_mfcc_t *pAccMeanSIL = pUserFeatAccum->cepMeanSilence;
			hci_mfcc64 *pAccVarSIL = pUserFeatAccum->cepVarSilence;
			hci_mfcc_t *pAccMeanSPCH = pUserFeatAccum->cepMeanUnvoiced;
			hci_mfcc64 *pAccVarSPCH = pUserFeatAccum->cepVarUnvoiced;
            pUserFeatAccum->countSilenceFrame = PowerASR_BasicOP_add_32_32(pUserFeatAccum->countSilenceFrame, 1);
            pUserFeatAccum->countUnvoicedFrame = PowerASR_BasicOP_add_32_32(pUserFeatAccum->countUnvoicedFrame, 2);
			while (pInputMfcc < pLastMFCC) {
				*pAccMeanSIL += (*pInputMfcc);
				*pAccMeanSPCH += 2 * (*pInputMfcc);
				L_val = (hci_mfcc64)(*pInputMfcc) * (hci_mfcc64)(*pInputMfcc);
				*pAccVarSIL += L_val;
				*pAccVarSPCH += 2 * L_val;
				pInputMfcc++;
				pAccMeanSIL++; pAccVarSIL++;
				pAccMeanSPCH++; pAccVarSPCH++;
			}
		}
		else if (VOICED_FRAME == frame_class) {
			hci_mfcc64 L_val = 0;
			hci_mfcc_t *pAccMean = pUserFeatAccum->cepMeanVoiced;
			hci_mfcc64 *pAccVar = pUserFeatAccum->cepVarVoiced;
            pUserFeatAccum->countVoicedFrame = PowerASR_BasicOP_add_32_32(pUserFeatAccum->countVoicedFrame, 2);
			while (pInputMfcc < pLastMFCC) {
				*pAccMean += 2 * (*pInputMfcc);
				L_val = 2 * (hci_mfcc64)(*pInputMfcc) * (hci_mfcc64)(*pInputMfcc);
				*pAccVar += L_val;
				pInputMfcc++; pAccMean++; pAccVar++;
			}
		}
		else {	// mixed frame
			hci_mfcc64 L_val = 0;
			hci_mfcc_t *pAccMeanSIL = pUserFeatAccum->cepMeanUnvoiced;
			hci_mfcc64 *pAccVarSIL = pUserFeatAccum->cepVarUnvoiced;
			hci_mfcc_t *pAccMeanSPCH = pUserFeatAccum->cepMeanVoiced;
			hci_mfcc64 *pAccVarSPCH = pUserFeatAccum->cepVarVoiced;
            pUserFeatAccum->countUnvoicedFrame = PowerASR_BasicOP_add_32_32(pUserFeatAccum->countUnvoicedFrame, 1);
            pUserFeatAccum->countVoicedFrame = PowerASR_BasicOP_add_32_32(pUserFeatAccum->countVoicedFrame, 1);
			while (pInputMfcc < pLastMFCC) {
				*pAccMeanSIL += (*pInputMfcc);
				*pAccMeanSPCH += (*pInputMfcc);
				L_val = (hci_mfcc64)(*pInputMfcc) * (hci_mfcc64)(*pInputMfcc);
				*pAccVarSIL += L_val;
				*pAccVarSPCH += L_val;
				pInputMfcc++;
				pAccMeanSIL++; pAccVarSIL++;
				pAccMeanSPCH++; pAccVarSPCH++;
			}
		}
		break;
	case RECUR_CMS:		// recursive CMS 
		if (pFXVar->bCMSwVAD) {
			if (SIL_FRAME == frame_class) {
				hci_mfcc_t *pCepMean = pUserFeatNorm->cepMeanSilence;
				hci_mfcc_t *pAccMean = pUserFeatAccum->cepMeanSilence;
				hci_mfcc16 val_compl = 0;
#ifdef FIXED_POINT_FE
				hci_int16 val_hi = 0, val_lo = 0;
				val_compl = PowerASR_BasicOP_subtract_16_16(0x7FFF, pFXVar->forgetF_Unvoice) + 1;
#else	// !FIXED_POINT_FE
				val_compl = 1.0f - pFXVar->forgetF_Unvoice;
#endif	// #ifdef FIXED_POINT_FE
                pUserFeatAccum->countSilenceFrame = PowerASR_BasicOP_add_32_32(pUserFeatAccum->countSilenceFrame, 2);
				while (pInputMfcc < pLastMFCC) {
					*pOutputMfcc -= *pCepMean;		//////
					*pAccMean += 2 * (*pInputMfcc);
#ifdef FIXED_POINT_FE
					PowerASR_BasicOP_separateBits_32_16(*pCepMean, &val_hi, &val_lo);
					*pCepMean = PowerASR_BasicOP_multiply_32_16_32(val_hi, val_lo, pFXVar->forgetF_Unvoice);
					PowerASR_BasicOP_separateBits_32_16(*pInputMfcc, &val_hi, &val_lo);
					*pCepMean += PowerASR_BasicOP_multiply_32_16_32(val_hi, val_lo, val_compl);
#else	// !FIXED_POINT_FE
					*pCepMean *= pFXVar->forgetF_Unvoice;
					*pCepMean += val_compl * (*pInputMfcc);
#endif	// #ifdef FIXED_POINT_FE
//					*pOutputMfcc -= *pCepMean;
					pInputMfcc++; pOutputMfcc++; pCepMean++; pAccMean++;
				}
			}
			else if (UNVOICED_FRAME == frame_class) {
				hci_mfcc_t *pCepMeanSIL = pUserFeatNorm->cepMeanSilence;
				hci_mfcc_t *pCepMeanSPCH = pUserFeatNorm->cepMeanUnvoiced;
				hci_mfcc_t *pAccMeanSIL = pUserFeatAccum->cepMeanSilence;
				hci_mfcc_t *pAccMeanSPCH = pUserFeatAccum->cepMeanUnvoiced;
				hci_mfcc16 val_wgt_voice = 0;
				hci_mfcc16 val_wgt_unvoice = 0;
				hci_mfcc16 val_compl_voice = 0;
				hci_mfcc16 val_compl_unvoice = 0;
				hci_flag bUpdateSilCMN = FALSE;
#ifdef FIXED_POINT_FE
				hci_int16 val_hi = 0, val_lo = 0;
				val_compl_voice = PowerASR_BasicOP_subtract_16_16(0x7FFF, pFXVar->forgetF_Unvoice) + 1;
				val_compl_voice = PowerASR_BasicOP_shiftRight_16_16(val_compl_voice, 1);
				val_compl_unvoice = PowerASR_BasicOP_subtract_16_16(0x7FFF, pFXVar->forgetF_Unvoice) + 1;
				val_compl_unvoice = PowerASR_BasicOP_shiftRight_16_16(val_compl_unvoice, 1);
				val_wgt_voice = PowerASR_BasicOP_subtract_16_16(0x7FFF, val_compl_voice);
				val_wgt_unvoice = PowerASR_BasicOP_subtract_16_16(0x7FFF, val_compl_unvoice);
#else	// !FIXED_POINT_FE
				val_compl_voice = 1.0f - pFXVar->forgetF_Unvoice;
				val_compl_unvoice = 0.5f * (1.0f - pFXVar->forgetF_Unvoice);
				val_wgt_voice = 1.0f - val_compl_voice;
				val_wgt_unvoice = 1.0f - val_compl_unvoice;
#endif	// #ifdef FIXED_POINT_FE
                if (bUpdateSilCMN) pUserFeatAccum->countSilenceFrame = PowerASR_BasicOP_add_32_32(pUserFeatAccum->countSilenceFrame, 1);
                pUserFeatAccum->countUnvoicedFrame = PowerASR_BasicOP_add_32_32(pUserFeatAccum->countUnvoicedFrame, 2);
				while (pInputMfcc < pLastMFCC) {
					*pOutputMfcc -= *pCepMeanSPCH;		//////
					if ( bUpdateSilCMN ) *pAccMeanSIL += (*pInputMfcc);
					*pAccMeanSPCH += 2 * (*pInputMfcc);
#ifdef FIXED_POINT_FE
					if ( bUpdateSilCMN ) {
					PowerASR_BasicOP_separateBits_32_16(*pCepMeanSIL, &val_hi, &val_lo);
					*pCepMeanSIL = PowerASR_BasicOP_multiply_32_16_32(val_hi, val_lo, val_wgt_unvoice);
//					PowerASR_BasicOP_separateBits_32_16(*pCepMeanSPCH, &val_hi, &val_lo);
//					*pCepMeanSPCH = PowerASR_BasicOP_multiply_32_16_32(val_hi, val_lo, val_wgt_voice);
					PowerASR_BasicOP_separateBits_32_16(*pInputMfcc, &val_hi, &val_lo);
					*pCepMeanSIL += PowerASR_BasicOP_multiply_32_16_32(val_hi, val_lo, val_compl_unvoice);
//					*pCepMeanSPCH += PowerASR_BasicOP_multiply_32_16_32(val_hi, val_lo, val_compl_voice);
					}
#else	// !FIXED_POINT_FE
					if ( bUpdateSilCMN ) {
						*pCepMeanSIL *= val_wgt_unvoice;
						*pCepMeanSIL += val_compl_unvoice * (*pInputMfcc);
					}
// 					*pCepMeanSPCH *= val_wgt_voice;
// 					*pCepMeanSPCH += val_compl_voice * (*pInputMfcc);
#endif	// #ifdef FIXED_POINT_FE
//					*pOutputMfcc -= *pCepMeanSPCH;
					pInputMfcc++; pOutputMfcc++;
					pCepMeanSIL++; pCepMeanSPCH++;
					pAccMeanSIL++; pAccMeanSPCH++;
				}
			}
			else if (VOICED_FRAME == frame_class) {
				hci_mfcc_t *pCepMean = pUserFeatNorm->cepMeanVoiced;
				hci_mfcc_t *pAccMean = pUserFeatAccum->cepMeanVoiced;
				hci_mfcc16 val_compl = 0;
#ifdef FIXED_POINT_FE
				hci_int16 val_hi = 0, val_lo = 0;
				val_compl = PowerASR_BasicOP_subtract_16_16(0x7FFF, pFXVar->forgetF_Voice) + 1;
#else	// !FIXED_POINT_FE
				val_compl = 1.0f - pFXVar->forgetF_Voice;
#endif	// #ifdef FIXED_POINT_FE
                pUserFeatAccum->countVoicedFrame = PowerASR_BasicOP_add_32_32(pUserFeatAccum->countVoicedFrame, 2);
				while (pInputMfcc < pLastMFCC) {
					*pOutputMfcc -= *pCepMean;
					*pAccMean += 2 * (*pInputMfcc);
/*
#ifdef FIXED_POINT_FE
					PowerASR_BasicOP_separateBits_32_16(*pCepMean, &val_hi, &val_lo);
					*pCepMean = PowerASR_BasicOP_multiply_32_16_32(val_hi, val_lo, pFXVar->forgetF_Voice);
					PowerASR_BasicOP_separateBits_32_16(*pInputMfcc, &val_hi, &val_lo);
					*pCepMean += PowerASR_BasicOP_multiply_32_16_32(val_hi, val_lo, val_compl);
#else	// !FIXED_POINT_FE
					*pCepMean *= pFXVar->forgetF_Voice;
					*pCepMean += val_compl * (*pInputMfcc);
#endif	// #ifdef FIXED_POINT_FE
*/
//					*pOutputMfcc -= *pCepMean;
					pInputMfcc++; pOutputMfcc++; pCepMean++; pAccMean++;
				}
			}
			else {	// mixed frame
				hci_mfcc_t *pCepMeanSIL = pUserFeatNorm->cepMeanUnvoiced;
				hci_mfcc_t *pCepMeanSPCH = pUserFeatNorm->cepMeanVoiced;
				hci_mfcc_t *pAccMeanSIL = pUserFeatAccum->cepMeanUnvoiced;
				hci_mfcc_t *pAccMeanSPCH = pUserFeatAccum->cepMeanVoiced;
				hci_mfcc16 val_wgt_voice = 0;
				hci_mfcc16 val_wgt_unvoice = 0;
				hci_mfcc16 val_compl_voice = 0;
				hci_mfcc16 val_compl_unvoice = 0;
#ifdef FIXED_POINT_FE
				hci_int16 val_hi = 0, val_lo = 0;
				val_compl_voice = PowerASR_BasicOP_subtract_16_16(0x7FFF, pFXVar->forgetF_Voice) + 1;
				val_compl_voice = PowerASR_BasicOP_shiftRight_16_16(val_compl_voice, 1);
				val_compl_unvoice = PowerASR_BasicOP_subtract_16_16(0x7FFF, pFXVar->forgetF_Unvoice) + 1;
				val_compl_unvoice = PowerASR_BasicOP_shiftRight_16_16(val_compl_unvoice, 1);
				val_wgt_voice = PowerASR_BasicOP_subtract_16_16(0x7FFF, val_compl_voice);
				val_wgt_unvoice = PowerASR_BasicOP_subtract_16_16(0x7FFF, val_compl_unvoice);
#else	// !FIXED_POINT_FE
				val_compl_voice = 0.5f * (1.0f - pFXVar->forgetF_Voice);
				val_compl_unvoice = 0.5f * (1.0f - pFXVar->forgetF_Unvoice);
				val_wgt_voice = 1.0f - val_compl_voice;
				val_wgt_unvoice = 1.0f - val_compl_unvoice;
#endif	// #ifdef FIXED_POINT_FE
                pUserFeatAccum->countUnvoicedFrame = PowerASR_BasicOP_add_32_32(pUserFeatAccum->countUnvoicedFrame, 1);
                pUserFeatAccum->countVoicedFrame = PowerASR_BasicOP_add_32_32(pUserFeatAccum->countVoicedFrame, 1);
				while (pInputMfcc < pLastMFCC) {
					*pOutputMfcc *= 2;
					*pOutputMfcc -= *pCepMeanSIL;
					*pOutputMfcc -= *pCepMeanSPCH;
#ifdef FIXED_POINT_FE
					*pOutputMfcc = PowerASR_BasicOP_shiftRight_32_32(*pOutputMfcc, 1);
#else	// !FIXED_POINT_FE
					*pOutputMfcc *= 0.5f;
#endif	// #ifdef FIXED_POINT_FE
					*pAccMeanSIL += (*pInputMfcc);
					*pAccMeanSPCH += (*pInputMfcc);
#ifdef FIXED_POINT_FE
//					PowerASR_BasicOP_separateBits_32_16(*pCepMeanSIL, &val_hi, &val_lo);
//					*pCepMeanSIL = PowerASR_BasicOP_multiply_32_16_32(val_hi, val_lo, val_wgt_unvoice);
//					PowerASR_BasicOP_separateBits_32_16(*pCepMeanSPCH, &val_hi, &val_lo);
//					*pCepMeanSPCH = PowerASR_BasicOP_multiply_32_16_32(val_hi, val_lo, val_wgt_voice);
//					PowerASR_BasicOP_separateBits_32_16(*pInputMfcc, &val_hi, &val_lo);
//					*pCepMeanSIL += PowerASR_BasicOP_multiply_32_16_32(val_hi, val_lo, val_compl_unvoice);
//					*pCepMeanSPCH += PowerASR_BasicOP_multiply_32_16_32(val_hi, val_lo, val_compl_voice);
#else	// !FIXED_POINT_FE
//					*pCepMeanSIL *= val_wgt_unvoice;
//					*pCepMeanSIL += val_compl_unvoice * (*pInputMfcc);
// 					*pCepMeanSPCH *= val_wgt_voice;
// 					*pCepMeanSPCH += val_compl_voice * (*pInputMfcc);
#endif	// #ifdef FIXED_POINT_FE
					pInputMfcc++; pOutputMfcc++;
					pCepMeanSIL++; pCepMeanSPCH++;
					pAccMeanSIL++; pAccMeanSPCH++;
				}
			}
		}
		else {	// if pFXVar->bCMSwVAD == FALSE
			hci_mfcc_t *pCepMean = pUserFeatNorm->cepMeanVoiced;
			hci_mfcc_t *pAccMean = pUserFeatAccum->cepMeanVoiced;
			hci_mfcc16 val_compl = 0;
#ifdef FIXED_POINT_FE
			hci_int16 val_hi = 0, val_lo = 0;
			val_compl = PowerASR_BasicOP_subtract_16_16(0x7FFF, pFXVar->forgetF_Voice) + 1;
#else	// !FIXED_POINT_FE
			val_compl = 1.0f - pFXVar->forgetF_Voice;
#endif	// #ifdef FIXED_POINT_FE
			if (frame_class!=SIL_FRAME || pFXVar->bUseSilenceMean) {
                pUserFeatAccum->countVoicedFrame = PowerASR_BasicOP_add_32_32(pUserFeatAccum->countVoicedFrame, 2);
			}
			while (pInputMfcc < pLastMFCC) {
				if (frame_class!=SIL_FRAME || pFXVar->bUseSilenceMean) {
					*pAccMean += 2 * (*pInputMfcc);
#ifdef FIXED_POINT_FE
					PowerASR_BasicOP_separateBits_32_16(*pCepMean, &val_hi, &val_lo);
					*pCepMean = PowerASR_BasicOP_multiply_32_16_32(val_hi, val_lo, pFXVar->forgetF_Voice);
					PowerASR_BasicOP_separateBits_32_16(*pInputMfcc, &val_hi, &val_lo);
					*pCepMean += PowerASR_BasicOP_multiply_32_16_32(val_hi, val_lo, val_compl);
#else	// !FIXED_POINT_FE
					*pCepMean *= pFXVar->forgetF_Voice;
					*pCepMean += val_compl * (*pInputMfcc);
#endif	// #ifdef FIXED_POINT_FE
				}
				*pOutputMfcc -= *pCepMean;
				pInputMfcc++; pOutputMfcc++; pCepMean++; pAccMean++;
			}
		}
		if (pUserFeatAccum->countSilenceFrame >= CMN_UPDATE_COUNT) {
			FX_CMS_updateCepstralMeanVector(pMfccStream, pFXVar);
			FX_CMS_initializeCepstrumMean(pMfccStream, pFXVar->dimMFCC, pFXVar->typeCMS, pFXVar->bCMSwVAD);
		}
		break;
	case RECUR_CMVN:		// recursive CMVN 
		if (pFXVar->bCMSwVAD) {
			if (SIL_FRAME == frame_class) {
				hci_mfcc_t *pCepMean = pUserFeatNorm->cepMeanSilence;
				hci_mfcc_t *pCepVar = pUserFeatNorm->cepVarSilence;
				hci_mfcc_t *pAccMean = pUserFeatAccum->cepMeanSilence;
				hci_mfcc64 *pAccVar = pUserFeatAccum->cepVarSilence;
				hci_mfcc32 L_val = 0;
				hci_mfcc16 val_compl = 0;
				hci_mfcc64 LL_val = 0;
#ifdef FIXED_POINT_FE
				hci_mfcc32 L_val_weight = 0;
				hci_int16 val_hi = 0, val_lo = 0;
				val_compl = PowerASR_BasicOP_subtract_16_16(0x7FFF, pFXVar->forgetF_Unvoice) + 1;
#else	// !FIXED_POINT_FE
				val_compl = 1.0f - pFXVar->forgetF_Unvoice;
#endif	// #ifdef FIXED_POINT_FE
                pUserFeatAccum->countSilenceFrame = PowerASR_BasicOP_add_32_32(pUserFeatAccum->countSilenceFrame, 2);
				while (pInputMfcc < pLastMFCC) {
					*pAccMean += 2 * (*pInputMfcc);
					LL_val     = 2 * (hci_mfcc64)(*pInputMfcc) * (hci_mfcc64)(*pInputMfcc);
					*pAccVar  += LL_val;
#ifdef FIXED_POINT_FE
					PowerASR_BasicOP_separateBits_32_16(*pCepMean, &val_hi, &val_lo);
					*pCepMean = PowerASR_BasicOP_multiply_32_16_32(val_hi, val_lo, pFXVar->forgetF_Unvoice);
					PowerASR_BasicOP_separateBits_32_16(*pInputMfcc, &val_hi, &val_lo);
					*pCepMean += PowerASR_BasicOP_multiply_32_16_32(val_hi, val_lo, val_compl);
					LL_val = (hci_mfcc64)(*pInputMfcc) * (hci_mfcc64)(*pInputMfcc);				// Q30.64
					LL_val = (LL_val>>15);														// Q15.64
					LL_val *= (hci_mfcc64)val_compl;											// Q30.64
					LL_val += (hci_mfcc64)pFXVar->forgetF_Unvoice * (hci_mfcc64)(*pCepVar);		// Q30.64 (=mean(s^2))
					*pCepVar = (hci_mfcc_t)(LL_val>>15);										// Q15.32
					LL_val = LL_val - (hci_mfcc64)(*pCepMean) * (hci_mfcc64)(*pCepMean);		// 30.64 (variance)
					L_val = (hci_mfcc32)(LL_val>>10);											// Q20.32
					L_val = HCI_MAX(L_val, MIN_VAR);
					L_val_weight = (hci_mfcc32)PowerASR_BasicOP_fixedSQRT_32_16(L_val);			// Q10.32 (standard deviation)
					L_val = *pOutputMfcc - *pCepMean;											// Q15.32
					*pOutputMfcc = PowerASR_BasicOP_divideShiftLeft_32_32(L_val, L_val_weight, 10);	// Q15.32
#else	// !FIXED_POINT_FE
					*pCepMean *= pFXVar->forgetF_Unvoice;
					*pCepMean += val_compl * (*pInputMfcc);			// update mean(s)
					*pCepVar *= pFXVar->forgetF_Unvoice;
					*pCepVar += val_compl * HCI_SQ(*pInputMfcc);	// update mean(s^2)
					L_val = *pCepVar - HCI_SQ(*pCepMean);			// estimate sigma^2
					L_val = HCI_MAX(L_val, MIN_VAR);
					*pOutputMfcc = (hci_mfcc_t)((*pOutputMfcc) - (*pCepMean)) / (hci_mfcc_t)sqrt((double)L_val);
#endif	// #ifdef FIXED_POINT_FE
					pInputMfcc++; pOutputMfcc++;
					pCepMean++; pCepVar++;
					pAccMean++; pAccVar++;
				}
			}
			else if (UNVOICED_FRAME == frame_class) {
				hci_mfcc_t *pCepMeanSIL = pUserFeatNorm->cepMeanSilence;
				hci_mfcc_t *pCepMeanSPCH = pUserFeatNorm->cepMeanUnvoiced;
				hci_mfcc_t *pCepVarSIL = pUserFeatNorm->cepVarSilence;
				hci_mfcc_t *pCepVarSPCH = pUserFeatNorm->cepVarUnvoiced;
				hci_mfcc_t *pAccMeanSIL = pUserFeatAccum->cepMeanSilence;
				hci_mfcc_t *pAccMeanSPCH = pUserFeatAccum->cepMeanUnvoiced;
				hci_mfcc64 *pAccVarSIL = pUserFeatAccum->cepVarSilence;
				hci_mfcc64 *pAccVarSPCH = pUserFeatAccum->cepVarUnvoiced;
				hci_mfcc32 L_val = 0;
				hci_mfcc_t orgFeat = 0;
				hci_mfcc16 val_wgt_voice = 0;
				hci_mfcc16 val_wgt_unvoice = 0;
				hci_mfcc16 val_compl_voice = 0;
				hci_mfcc16 val_compl_unvoice = 0;
				hci_mfcc64 LL_val = 0;
#ifdef FIXED_POINT_FE
				hci_mfcc64 LL_sq_feat = 0;
				hci_mfcc32 L_val_stdev_voice = 0;
				hci_mfcc32 L_val_stdev_unvoice = 0;
				hci_int16 val_hi = 0, val_lo = 0;
				val_compl_voice = PowerASR_BasicOP_subtract_16_16(0x7FFF, pFXVar->forgetF_Voice) + 1;
				val_compl_voice = PowerASR_BasicOP_shiftRight_16_16(val_compl_voice, 1);
				val_compl_unvoice = PowerASR_BasicOP_subtract_16_16(0x7FFF, pFXVar->forgetF_Unvoice) + 1;
				val_compl_unvoice = PowerASR_BasicOP_shiftRight_16_16(val_compl_unvoice, 1);
				val_wgt_voice = PowerASR_BasicOP_subtract_16_16(0x7FFF, val_compl_voice);
				val_wgt_unvoice = PowerASR_BasicOP_subtract_16_16(0x7FFF, val_compl_unvoice);
#else	// !FIXED_POINT_FE
				val_compl_voice = 0.5f * (1.0f - pFXVar->forgetF_Voice);
				val_compl_unvoice = 0.5f * (1.0f - pFXVar->forgetF_Unvoice);
				val_wgt_voice = 1.0f - val_compl_voice;
				val_wgt_unvoice = 1.0f - val_compl_unvoice;
#endif	// #ifdef FIXED_POINT_FE
                pUserFeatAccum->countUnvoicedFrame = PowerASR_BasicOP_add_32_32(pUserFeatAccum->countUnvoicedFrame, 2);
                pUserFeatAccum->countSilenceFrame = PowerASR_BasicOP_add_32_32(pUserFeatAccum->countSilenceFrame, 1);
				while (pInputMfcc < pLastMFCC) {
					*pAccMeanSIL  += *pInputMfcc;
					*pAccMeanSPCH += 2 * (*pInputMfcc);
					LL_val = (hci_mfcc64)(*pInputMfcc) * (hci_mfcc64)(*pInputMfcc);
					*pAccVarSIL  += LL_val;
					*pAccVarSPCH += 2 * LL_val;
#ifdef FIXED_POINT_FE
					PowerASR_BasicOP_separateBits_32_16(*pCepMeanSIL, &val_hi, &val_lo);
					*pCepMeanSIL = PowerASR_BasicOP_multiply_32_16_32(val_hi, val_lo, val_wgt_unvoice);
					PowerASR_BasicOP_separateBits_32_16(*pCepMeanSPCH, &val_hi, &val_lo);
					*pCepMeanSPCH = PowerASR_BasicOP_multiply_32_16_32(val_hi, val_lo, val_wgt_voice);
					PowerASR_BasicOP_separateBits_32_16(*pInputMfcc, &val_hi, &val_lo);
					*pCepMeanSIL += PowerASR_BasicOP_multiply_32_16_32(val_hi, val_lo, val_compl_unvoice);
					*pCepMeanSPCH += PowerASR_BasicOP_multiply_32_16_32(val_hi, val_lo, val_compl_voice);
					LL_val = (hci_mfcc64)(*pInputMfcc) * (hci_mfcc64)(*pInputMfcc);				// Q30.64
					LL_val = (LL_val>>15);														// Q15.64
					LL_sq_feat = LL_val;														// Q15.64
					LL_val = (hci_mfcc64)val_compl_unvoice * LL_sq_feat;						// Q30.64
					LL_val += (hci_mfcc64)val_wgt_unvoice * (hci_mfcc64)(*pCepVarSIL);			// Q30.64 (=mean(s^2))
					*pCepVarSIL = (hci_mfcc_t)(LL_val>>15);										// Q15.32
					LL_val = LL_val - (hci_mfcc64)(*pCepMeanSIL) * (hci_mfcc64)(*pCepMeanSIL);	// 30.64 (variance)
					L_val = (hci_mfcc32)(LL_val>>10);											// Q20.32
					L_val = HCI_MAX(L_val, MIN_VAR);
					L_val_stdev_unvoice = (hci_mfcc32)PowerASR_BasicOP_fixedSQRT_32_16(L_val);	// Q10.32 (standard deviation)
					LL_val = (hci_mfcc64)val_compl_voice * LL_sq_feat;							// Q30.64
					LL_val += (hci_mfcc64)val_wgt_voice * (hci_mfcc64)(*pCepVarSPCH);			// Q30.64 (=mean(s^2))
					*pCepVarSPCH = (hci_mfcc_t)(LL_val>>15);									// Q15.32
					LL_val = LL_val - (hci_mfcc64)(*pCepMeanSPCH) * (hci_mfcc64)(*pCepMeanSPCH);	// 30.64 (variance)
					L_val = (hci_mfcc32)(LL_val>>10);											// Q20.32
					L_val = HCI_MAX(L_val, MIN_VAR);
					L_val_stdev_voice = (hci_mfcc32)PowerASR_BasicOP_fixedSQRT_32_16(L_val);	// Q10.32 (standard deviation)
					orgFeat = *pOutputMfcc;
					L_val = orgFeat - *pCepMeanSPCH;											// Q15.32
					*pOutputMfcc = PowerASR_BasicOP_divideShiftLeft_32_32(L_val, L_val_stdev_voice, 10);	// Q15.32
#else	// !FIXED_POINT_FE
					*pCepMeanSIL *= val_wgt_unvoice;
					*pCepMeanSIL += val_compl_unvoice * (*pInputMfcc);
					*pCepMeanSPCH *= val_wgt_voice;
					*pCepMeanSPCH += val_compl_voice * (*pInputMfcc);
					*pCepVarSIL *= val_wgt_unvoice;
					*pCepVarSIL += val_compl_unvoice * HCI_SQ(*pInputMfcc);	// update mean(s^2)
					*pCepVarSPCH *= val_wgt_voice;
					*pCepVarSPCH += val_compl_voice * HCI_SQ(*pInputMfcc);	// update mean(s^2)
					L_val = *pCepVarSPCH - HCI_SQ(*pCepMeanSPCH);			// estimate sigma^2
					L_val = HCI_MAX(L_val, MIN_VAR);
					orgFeat = *pOutputMfcc;
					*pOutputMfcc = (hci_mfcc_t)(orgFeat - (*pCepMeanSIL)) / (hci_mfcc_t)sqrt((double)L_val);
#endif	// #ifdef FIXED_POINT_FE
					pInputMfcc++; pOutputMfcc++;
					pCepMeanSIL++; pCepMeanSPCH++;
					pCepVarSIL++; pCepVarSPCH++;
					pAccMeanSIL++; pAccMeanSPCH++;
					pAccVarSIL++; pAccVarSPCH++;
				}
			}
			else if (VOICED_FRAME == frame_class) {
				hci_mfcc_t *pCepMean = pUserFeatNorm->cepMeanVoiced;
				hci_mfcc_t *pCepVar = pUserFeatNorm->cepVarVoiced;
				hci_mfcc_t *pAccMean = pUserFeatAccum->cepMeanVoiced;
				hci_mfcc64 *pAccVar = pUserFeatAccum->cepVarVoiced;
				hci_mfcc32 L_val = 0;
				hci_mfcc16 val_compl = 0;
				hci_mfcc64 LL_val = 0;
#ifdef FIXED_POINT_FE
				hci_mfcc32 L_val_weight = 0;
				hci_int16 val_hi = 0, val_lo = 0;
				val_compl = PowerASR_BasicOP_subtract_16_16(0x7FFF, pFXVar->forgetF_Voice) + 1;
#else	// !FIXED_POINT_FE
				val_compl = 1.0f - pFXVar->forgetF_Voice;
#endif	// #ifdef FIXED_POINT_FE
                pUserFeatAccum->countVoicedFrame = PowerASR_BasicOP_add_32_32(pUserFeatAccum->countVoicedFrame, 2);
				while (pInputMfcc < pLastMFCC) {
					*pAccMean += 2 * (*pInputMfcc);
					LL_val     = 2 * (hci_mfcc64)(*pInputMfcc) * (hci_mfcc64)(*pInputMfcc);
					*pAccVar  += LL_val;
#ifdef FIXED_POINT_FE
					PowerASR_BasicOP_separateBits_32_16(*pCepMean, &val_hi, &val_lo);
					*pCepMean = PowerASR_BasicOP_multiply_32_16_32(val_hi, val_lo, pFXVar->forgetF_Voice);
					PowerASR_BasicOP_separateBits_32_16(*pInputMfcc, &val_hi, &val_lo);
					*pCepMean += PowerASR_BasicOP_multiply_32_16_32(val_hi, val_lo, val_compl);
					LL_val = (hci_mfcc64)(*pInputMfcc) * (hci_mfcc64)(*pInputMfcc);				// Q30.64
					LL_val = (LL_val>>15);														// Q15.64
					LL_val *= (hci_mfcc64)val_compl;											// Q30.64
					LL_val += (hci_mfcc64)pFXVar->forgetF_Voice * (hci_mfcc64)(*pCepVar);		// Q30.64 (=mean(s^2))
					*pCepVar = (hci_mfcc_t)(LL_val>>15);										// Q15.32
					LL_val = LL_val - (hci_mfcc64)(*pCepMean) * (hci_mfcc64)(*pCepMean);		// 30.64 (variance)
					L_val = (hci_mfcc32)(LL_val>>10);											// Q20.32
					L_val = HCI_MAX(L_val, MIN_VAR);
					L_val_weight = (hci_mfcc32)PowerASR_BasicOP_fixedSQRT_32_16(L_val);			// Q10.32 (standard deviation)
					L_val = *pOutputMfcc - *pCepMean;											// Q15.32
					*pOutputMfcc = PowerASR_BasicOP_divideShiftLeft_32_32(L_val, L_val_weight, 10);	// Q15.32
#else	// !FIXED_POINT_FE
					*pCepMean *= pFXVar->forgetF_Voice;
					*pCepMean += val_compl * (*pInputMfcc);			// update mean(s)
					*pCepVar *= pFXVar->forgetF_Voice;
					*pCepVar += val_compl * HCI_SQ(*pInputMfcc);	// update mean(s^2)
					L_val = *pCepVar - HCI_SQ(*pCepMean);			// estimate sigma^2
					L_val = HCI_MAX(L_val, MIN_VAR);
					*pOutputMfcc = (hci_mfcc_t)((*pOutputMfcc) - (*pCepMean)) / (hci_mfcc_t)sqrt((double)L_val);
#endif	// #ifdef FIXED_POINT_FE
					pInputMfcc++; pOutputMfcc++;
					pCepMean++; pCepVar++;
					pAccMean++; pAccVar++;
				}
			}
			else {	// mixed frame
				hci_mfcc_t *pCepMeanSIL = pUserFeatNorm->cepMeanUnvoiced;
				hci_mfcc_t *pCepMeanSPCH = pUserFeatNorm->cepMeanVoiced;
				hci_mfcc_t *pCepVarSIL = pUserFeatNorm->cepVarUnvoiced;
				hci_mfcc_t *pCepVarSPCH = pUserFeatNorm->cepVarVoiced;
 				hci_mfcc_t *pAccMeanSIL = pUserFeatAccum->cepMeanUnvoiced;
 				hci_mfcc_t *pAccMeanSPCH = pUserFeatAccum->cepMeanVoiced;
 				hci_mfcc64 *pAccVarSIL = pUserFeatAccum->cepVarUnvoiced;
 				hci_mfcc64 *pAccVarSPCH = pUserFeatAccum->cepVarVoiced;
				hci_mfcc32 L_val = 0;
				hci_mfcc_t orgFeat = 0;
				hci_mfcc16 val_wgt_voice = 0;
				hci_mfcc16 val_wgt_unvoice = 0;
				hci_mfcc16 val_compl_voice = 0;
				hci_mfcc16 val_compl_unvoice = 0;
				hci_mfcc64 LL_val = 0;
#ifdef FIXED_POINT_FE
				hci_mfcc64 LL_sq_feat = 0;
				hci_mfcc32 L_val_stdev_voice = 0;
				hci_mfcc32 L_val_stdev_unvoice = 0;
				hci_int16 val_hi = 0, val_lo = 0;
				val_compl_voice = PowerASR_BasicOP_subtract_16_16(0x7FFF, pFXVar->forgetF_Voice) + 1;
				val_compl_voice = PowerASR_BasicOP_shiftRight_16_16(val_compl_voice, 1);
				val_compl_unvoice = PowerASR_BasicOP_subtract_16_16(0x7FFF, pFXVar->forgetF_Unvoice) + 1;
				val_compl_unvoice = PowerASR_BasicOP_shiftRight_16_16(val_compl_unvoice, 1);
				val_wgt_voice = PowerASR_BasicOP_subtract_16_16(0x7FFF, val_compl_voice);
				val_wgt_unvoice = PowerASR_BasicOP_subtract_16_16(0x7FFF, val_compl_unvoice);
#else	// !FIXED_POINT_FE
				val_compl_voice = 0.5f * (1.0f - pFXVar->forgetF_Voice);
				val_compl_unvoice = 0.5f * (1.0f - pFXVar->forgetF_Unvoice);
				val_wgt_voice = 1.0f - val_compl_voice;
				val_wgt_unvoice = 1.0f - val_compl_unvoice;
#endif	// #ifdef FIXED_POINT_FE
                pUserFeatAccum->countUnvoicedFrame = PowerASR_BasicOP_add_32_32(pUserFeatAccum->countUnvoicedFrame, 1);
                pUserFeatAccum->countVoicedFrame = PowerASR_BasicOP_add_32_32(pUserFeatAccum->countVoicedFrame, 1);
				while (pInputMfcc < pLastMFCC) {
					*pAccMeanSIL  += *pInputMfcc;
					*pAccMeanSPCH += *pInputMfcc;
					LL_val = (hci_mfcc64)(*pInputMfcc) * (hci_mfcc64)(*pInputMfcc);
					*pAccVarSIL  += LL_val;
					*pAccVarSPCH += LL_val;
#ifdef FIXED_POINT_FE
					PowerASR_BasicOP_separateBits_32_16(*pCepMeanSIL, &val_hi, &val_lo);
					*pCepMeanSIL = PowerASR_BasicOP_multiply_32_16_32(val_hi, val_lo, val_wgt_unvoice);
					PowerASR_BasicOP_separateBits_32_16(*pCepMeanSPCH, &val_hi, &val_lo);
					*pCepMeanSPCH = PowerASR_BasicOP_multiply_32_16_32(val_hi, val_lo, val_wgt_voice);
					PowerASR_BasicOP_separateBits_32_16(*pInputMfcc, &val_hi, &val_lo);
					*pCepMeanSIL += PowerASR_BasicOP_multiply_32_16_32(val_hi, val_lo, val_compl_unvoice);
					*pCepMeanSPCH += PowerASR_BasicOP_multiply_32_16_32(val_hi, val_lo, val_compl_voice);
					LL_val = (hci_mfcc64)(*pInputMfcc) * (hci_mfcc64)(*pInputMfcc);				// Q30.64
					LL_val = (LL_val>>15);														// Q15.64
					LL_sq_feat = LL_val;														// Q15.64
					LL_val = (hci_mfcc64)val_compl_unvoice * LL_sq_feat;						// Q30.64
					LL_val += (hci_mfcc64)val_wgt_unvoice * (hci_mfcc64)(*pCepVarSIL);			// Q30.64 (=mean(s^2))
					*pCepVarSIL = (hci_mfcc_t)(LL_val>>15);										// Q15.32
					LL_val = LL_val - (hci_mfcc64)(*pCepMeanSIL) * (hci_mfcc64)(*pCepMeanSIL);	// 30.64 (variance)
					L_val = (hci_mfcc32)(LL_val>>10);											// Q20.32
					L_val = HCI_MAX(L_val, MIN_VAR);
					L_val_stdev_unvoice = (hci_mfcc32)PowerASR_BasicOP_fixedSQRT_32_16(L_val);	// Q10.32 (standard deviation)
					LL_val = (hci_mfcc64)val_compl_voice * LL_sq_feat;							// Q30.64
					LL_val += (hci_mfcc64)val_wgt_voice * (hci_mfcc64)(*pCepVarSPCH);			// Q30.64 (=mean(s^2))
					*pCepVarSPCH = (hci_mfcc_t)(LL_val>>15);									// Q15.32
					LL_val = LL_val - (hci_mfcc64)(*pCepMeanSPCH) * (hci_mfcc64)(*pCepMeanSPCH);	// 30.64 (variance)
					L_val = (hci_mfcc32)(LL_val>>10);											// Q20.32
					L_val = HCI_MAX(L_val, MIN_VAR);
					L_val_stdev_voice = (hci_mfcc32)PowerASR_BasicOP_fixedSQRT_32_16(L_val);	// Q10.32 (standard deviation)
					orgFeat = *pOutputMfcc;
					L_val = orgFeat - *pCepMeanSIL;												// Q15.32
					*pOutputMfcc = PowerASR_BasicOP_divideShiftLeft_32_32(L_val, L_val_stdev_unvoice, 10);	// Q15.32
					L_val = orgFeat - *pCepMeanSPCH;											// Q15.32
					*pOutputMfcc += PowerASR_BasicOP_divideShiftLeft_32_32(L_val, L_val_stdev_voice, 10);	// Q15.32
					*pOutputMfcc = PowerASR_BasicOP_shiftRight_32_32(*pOutputMfcc, 1);			// Q15.32
#else	// !FIXED_POINT_FE
					*pCepMeanSIL *= val_wgt_unvoice;
					*pCepMeanSIL += val_compl_unvoice * (*pInputMfcc);
					*pCepMeanSPCH *= val_wgt_voice;
					*pCepMeanSPCH += val_compl_voice * (*pInputMfcc);
					*pCepVarSIL *= val_wgt_unvoice;
					*pCepVarSIL += val_compl_unvoice * HCI_SQ(*pInputMfcc);	// update mean(s^2)
					*pCepVarSPCH *= val_wgt_voice;
					*pCepVarSPCH += val_compl_voice * HCI_SQ(*pInputMfcc);	// update mean(s^2)
					L_val = *pCepVarSIL - HCI_SQ(*pCepMeanSIL);			// estimate sigma^2
					L_val = HCI_MAX(L_val, MIN_VAR);
					orgFeat = *pOutputMfcc;
					*pOutputMfcc = (hci_mfcc_t)(orgFeat - (*pCepMeanSIL)) / (hci_mfcc_t)sqrt((double)L_val);
					L_val = *pCepVarSPCH - HCI_SQ(*pCepMeanSPCH);			// estimate sigma^2
					L_val = HCI_MAX(L_val, MIN_VAR);
					*pOutputMfcc += (hci_mfcc_t)((orgFeat - (*pCepMeanSPCH)) / (hci_mfcc_t)sqrt((double)L_val));
					*pOutputMfcc *= 0.5f;
#endif	// #ifdef FIXED_POINT_FE
					pInputMfcc++; pOutputMfcc++;
					pCepMeanSIL++; pCepMeanSPCH++;
					pCepVarSIL++; pCepVarSPCH++;
					pAccMeanSIL++; pAccMeanSPCH++;
					pAccVarSIL++; pAccVarSPCH++;
				}
			}
		}
		else {	// if pFXVar->bCMSwVAD == FALSE
			hci_mfcc_t *pCepMean = pUserFeatNorm->cepMeanVoiced;
			hci_mfcc_t *pCepVar = pUserFeatNorm->cepVarVoiced;
			hci_mfcc_t *pAccMean = pUserFeatAccum->cepMeanVoiced;
			hci_mfcc64 *pAccVar = pUserFeatAccum->cepVarVoiced;
			hci_mfcc32 L_val = 0;
			hci_mfcc16 val_compl = 0;
			hci_mfcc64 LL_val = 0;
#ifdef FIXED_POINT_FE
			hci_mfcc32 L_val_weight = 0;
			hci_int16 val_hi = 0, val_lo = 0;
			val_compl = PowerASR_BasicOP_subtract_16_16(0x7FFF, pFXVar->forgetF_Voice) + 1;
#else	// !FIXED_POINT_FE
			val_compl = 1.0f - pFXVar->forgetF_Voice;
#endif	// #ifdef FIXED_POINT_FE
			if (frame_class!=SIL_FRAME || pFXVar->bUseSilenceMean) {
                pUserFeatAccum->countVoicedFrame = PowerASR_BasicOP_add_32_32(pUserFeatAccum->countVoicedFrame, 2);
			}
			while (pInputMfcc < pLastMFCC) {
				if (frame_class!=SIL_FRAME || pFXVar->bUseSilenceMean) {
					*pAccMean += 2 * (*pInputMfcc);
					LL_val     = 2 * (hci_mfcc64)(*pInputMfcc) * (hci_mfcc64)(*pInputMfcc);
					*pAccVar  += LL_val;
				}
#ifdef FIXED_POINT_FE
				if (frame_class!=SIL_FRAME || pFXVar->bUseSilenceMean) {
					PowerASR_BasicOP_separateBits_32_16(*pCepMean, &val_hi, &val_lo);
					*pCepMean = PowerASR_BasicOP_multiply_32_16_32(val_hi, val_lo, pFXVar->forgetF_Voice);
					PowerASR_BasicOP_separateBits_32_16(*pInputMfcc, &val_hi, &val_lo);
					*pCepMean += PowerASR_BasicOP_multiply_32_16_32(val_hi, val_lo, val_compl);
					LL_val = (hci_mfcc64)(*pInputMfcc) * (hci_mfcc64)(*pInputMfcc);				// Q30.64
					LL_val = (LL_val>>15);														// Q15.64
					LL_val *= (hci_mfcc64)val_compl;											// Q30.64
					LL_val += (hci_mfcc64)pFXVar->forgetF_Voice * (hci_mfcc64)(*pCepVar);		// Q30.64 (=mean(s^2))
					*pCepVar = (hci_mfcc_t)(LL_val>>15);										// Q15.32
				}
				else {
					LL_val = (hci_mfcc64)(1<<15) * (hci_mfcc64)(*pCepVar);					// Q30.64 (=mean(s^2))
				}
				LL_val = LL_val - (hci_mfcc64)(*pCepMean) * (hci_mfcc64)(*pCepMean);		// 30.64 (variance)
				L_val = (hci_mfcc32)(LL_val>>10);											// Q20.32
				L_val = HCI_MAX(L_val, MIN_VAR);
				L_val_weight = (hci_mfcc32)PowerASR_BasicOP_fixedSQRT_32_16(L_val);			// Q10.32 (standard deviation)
				L_val = *pOutputMfcc - *pCepMean;											// Q15.32
				*pOutputMfcc = PowerASR_BasicOP_divideShiftLeft_32_32(L_val, L_val_weight, 10);	// Q15.32
#else	// !FIXED_POINT_FE
				if (frame_class!=SIL_FRAME || pFXVar->bUseSilenceMean) {
					*pCepMean *= pFXVar->forgetF_Voice;
					*pCepMean += val_compl * (*pInputMfcc);			// update mean(s)
					*pCepVar *= pFXVar->forgetF_Voice;
					*pCepVar += val_compl * HCI_SQ(*pInputMfcc);	// update mean(s^2)
				}
				L_val = *pCepVar - HCI_SQ(*pCepMean);			// estimate sigma^2
				L_val = HCI_MAX(L_val, MIN_VAR);
				*pOutputMfcc = (hci_mfcc_t)((*pOutputMfcc) - (*pCepMean)) / (hci_mfcc_t)sqrt((double)L_val);
#endif	// #ifdef FIXED_POINT_FE
				pInputMfcc++; pOutputMfcc++;
				pCepMean++; pCepVar++;
				pAccMean++; pAccVar++;
			}
		}
		if (pUserFeatAccum->countSilenceFrame >= CMN_UPDATE_COUNT) {
			FX_CMS_updateCepstralMeanVector(pMfccStream, pFXVar);
			FX_CMS_initializeCepstrumMean(pMfccStream, pFXVar->dimMFCC, pFXVar->typeCMS, pFXVar->bCMSwVAD);
		}
		break;
	default:
		break;
	}

	return 0;
}


/**
 *	batch-mode CMS
 */
hci_int32
FX_CMS_batchCMS(hci_mfcc_t *mfccVec,			///< (i/o) current MFCC input
				FEAT_Normalizer *pUserFeatNorm,	///< (i) feature normalizer struct
				FrameClass frame_class,			///< (i) silence/speech class for current frame
				const hci_int16 dimMFCC,		///< (i) MFCC feature dimension
				CMS_Type typeCMS)				///< (i) CMS type
{
	hci_mfcc_t *pMFCC = 0;
	hci_mfcc_t *pLastMFCC = 0;
	hci_int32 i = 0;
	hci_mfcc64 L_val = 0;
	hci_mfcc64 L_val2 = 0;

	pMFCC = mfccVec;
	//pLastMFCC = pMFCC + DIM_CEPSTRUM;
	pLastMFCC = pMFCC + dimMFCC - 1;

	// subtract cepstral mean at batch mode
	if (BATCH_CMS == typeCMS) {
		if (SIL_FRAME == frame_class) {
			hci_mfcc_t *pCepMean = pUserFeatNorm->cepMeanSilence;
			while (pMFCC < pLastMFCC) {
				*pMFCC -= *pCepMean;
				pMFCC++; pCepMean++;
			}
		}
		else if (UNVOICED_FRAME == frame_class) {
			hci_mfcc_t *pCepMean = pUserFeatNorm->cepMeanUnvoiced;
			while (pMFCC < pLastMFCC) {
				*pMFCC -= *pCepMean;
				pMFCC++; pCepMean++;
			}
		}
		else if (VOICED_FRAME == frame_class) {
			hci_mfcc_t *pCepMean = pUserFeatNorm->cepMeanVoiced;
			while (pMFCC < pLastMFCC) {
				*pMFCC -= *pCepMean;
				pMFCC++; pCepMean++;
			}
		}
		else {	// mixed frame; (1/2) * {2*Ci - U_spe - U_sil}
			hci_mfcc_t *pCepMeanSIL = pUserFeatNorm->cepMeanUnvoiced;
			hci_mfcc_t *pCepMeanSPCH = pUserFeatNorm->cepMeanVoiced;
			while (pMFCC < pLastMFCC) {
				*pMFCC *= 2;
				*pMFCC -= *pCepMeanSIL;
				*pMFCC -= *pCepMeanSPCH;
#ifdef FIXED_POINT_FE
				*pMFCC = (((*pMFCC)+1)>>1);
#else	// !FIXED_POINT_FE
				*pMFCC *= 0.5f;
#endif	// #ifdef FIXED_POINT_FE
				pMFCC++;
				pCepMeanSIL++; pCepMeanSPCH++;
			}
		}
	}
	else if (BATCH_CMVN == typeCMS) {	// do both CMS & CVN
		if (SIL_FRAME == frame_class) {
			hci_mfcc_t *pCepMean = pUserFeatNorm->cepMeanSilence;
			hci_mfcc_t *pCepVar = pUserFeatNorm->cepVarSilence;
			while (pMFCC < pLastMFCC) {
				L_val = (hci_mfcc64) ((*pMFCC) - (*pCepMean));		// Q15.64
				L_val *= (*pCepVar);								// Q30.64
#ifdef FIXED_POINT_FE
				*pMFCC = (hci_mfcc_t)(L_val>>15);					// Q15.32
#else	// !FIXED_POINT_FE
				*pMFCC = (hci_mfcc_t)L_val;
#endif	// #ifdef FIXED_POINT_FE
				pMFCC++; pCepMean++; pCepVar++;
			}
		}
		else if (VOICED_FRAME == frame_class) {
			hci_mfcc_t *pCepMean = pUserFeatNorm->cepMeanUnvoiced;
			hci_mfcc_t *pCepVar = pUserFeatNorm->cepVarUnvoiced;
			while (pMFCC < pLastMFCC) {
				L_val = (hci_mfcc64) ((*pMFCC) - (*pCepMean));		// Q15.64
				L_val *= (*pCepVar);								// Q30.64
#ifdef FIXED_POINT_FE
				*pMFCC = (hci_mfcc_t)(L_val>>15);					// Q15.32
#else	// !FIXED_POINT_FE
				*pMFCC = (hci_mfcc_t)L_val;
#endif	// #ifdef FIXED_POINT_FE
				pMFCC++; pCepMean++; pCepVar++;
			}
		}
		else if (VOICED_FRAME == frame_class) {
			hci_mfcc_t *pCepMean = pUserFeatNorm->cepMeanVoiced;
			hci_mfcc_t *pCepVar = pUserFeatNorm->cepVarVoiced;
			while (pMFCC < pLastMFCC) {
				L_val = (hci_mfcc64) ((*pMFCC) - (*pCepMean));		// Q15.64
				L_val *= (*pCepVar);								// Q30.64
#ifdef FIXED_POINT_FE
				*pMFCC = (hci_mfcc_t)(L_val>>15);					// Q15.32
#else	// !FIXED_POINT_FE
				*pMFCC = (hci_mfcc_t)L_val;
#endif	// #ifdef FIXED_POINT_FE
				pMFCC++; pCepMean++; pCepVar++;
			}
		}
		else {	// mixed frame
			hci_mfcc_t *pCepMeanSIL = pUserFeatNorm->cepMeanUnvoiced;
			hci_mfcc_t *pCepVarSIL = pUserFeatNorm->cepVarUnvoiced;
			hci_mfcc_t *pCepMeanSPCH = pUserFeatNorm->cepMeanVoiced;
			hci_mfcc_t *pCepVarSPCH = pUserFeatNorm->cepVarVoiced;
			while (pMFCC < pLastMFCC) {
				L_val = (hci_mfcc64) ((*pMFCC) - (*pCepMeanSIL));		// Q15.64
				L_val *= (*pCepVarSIL);									// Q30.64
				L_val2 = (hci_mfcc64) ((*pMFCC) - (*pCepMeanSPCH));		// Q15.64
				L_val2 *= (*pCepVarSPCH);								// Q30.64
				L_val += L_val2;										// Q30.64
#ifdef FIXED_POINT_FE
				*pMFCC = (hci_mfcc_t)(L_val>>16);						// Q15.32
#else	// !FIXED_POINT_FE
				*pMFCC = (hci_mfcc_t)(L_val * 0.5f);
#endif	// #ifdef FIXED_POINT_FE
				pMFCC++;
				pCepMeanSIL++; pCepVarSIL++;
				pCepMeanSPCH++; pCepVarSPCH++;
			}
		}
	}


	return 0;
}


/**
 *	update cepstral mean vector
 */
hci_int32
FX_CMS_updateCepstralMeanVector(MFCC_Stream *pMfccStream, 		///< (i/o) pointer to mfcc stream
								Mfcc2FeatParameters *pFXVar)	///< (i) config. struct for mfcc-to-feature converter
{
	FEAT_Normalizer *pUserFeatNorm = 0;
	FEAT_Accumulator *pUserFeatAccum = 0;
	hci_mfcc_t L_val_denom = 0;
	hci_mfcc_t L_val_numer = 0;
	hci_mfcc64 LL_val = 0;
	hci_int16 dimCepstrum = pFXVar->dimMFCC - 1;

// 	if (NO_CMS == pFXVar->typeCMS || RECUR_CMS == pFXVar->typeCMS || RECUR_CMVN == pFXVar->typeCMS) {
// 		return 0;
// 	}

	if ( NO_CMS == pFXVar->typeCMS ) {
		return 0;
	}

	//pUserFeatNorm  = &pMfccStream->userFeatNorm;
	pUserFeatNorm  = &pMfccStream->baseFeatNorm;
	pUserFeatAccum = &pMfccStream->userFeatAccum;

	if (TRUE == pFXVar->bCMSwVAD) {
		hci_mfcc_t *pCepMeanSIL = pUserFeatNorm->cepMeanSilence;
		hci_mfcc_t *pCepMeanSPCH = pUserFeatNorm->cepMeanVoiced;
		hci_mfcc_t *pCepMeanUnv = pUserFeatNorm->cepMeanUnvoiced;
		hci_mfcc_t *pAccMeanSIL = pUserFeatAccum->cepMeanSilence;
		hci_mfcc_t *pAccMeanSPCH = pUserFeatAccum->cepMeanVoiced;
		hci_mfcc_t *pAccMeanUnv = pUserFeatAccum->cepMeanUnvoiced;
		hci_mfcc_t *pLastVec = pCepMeanSIL + dimCepstrum;
		hci_int32 ncountSIL = pUserFeatAccum->countSilenceFrame;
		hci_int32 ncountSPCH = pUserFeatAccum->countVoicedFrame;
		hci_int32 ncountUnv = pUserFeatAccum->countUnvoicedFrame;
#ifdef FIXED_POINT_FE
		hci_mfcc_t L_val_denom_sil = 0;
		hci_mfcc_t L_val_denom_speech = 0;
		hci_mfcc_t L_val_denom_unv = 0;

		L_val_denom_sil    = (hci_mfcc_t)(ncountSIL<<15);		// Q15.32
		L_val_denom_speech = (hci_mfcc_t)(ncountSPCH<<15);		// Q15.32
		L_val_denom_unv    = (hci_mfcc_t)(ncountUnv<<15);		// Q15.32
		while (pCepMeanSIL < pLastVec) {
			*pCepMeanSIL = PowerASR_BasicOP_divideShiftLeft_32_32(*pAccMeanSIL, L_val_denom_sil, 15);
			*pCepMeanSPCH  = PowerASR_BasicOP_divideShiftLeft_32_32(*pAccMeanSPCH, L_val_denom_speech, 15);
			*pCepMeanUnv  = PowerASR_BasicOP_divideShiftLeft_32_32(*pAccMeanUnv, L_val_denom_unv, 15);
			pCepMeanSIL++; pCepMeanSPCH++; pCepMeanUnv++;
			pAccMeanSIL++; pAccMeanSPCH++; pAccMeanUnv++;
		}
		if (BATCH_CMVN == pFXVar->typeCMS || LIVE_CMVN == pFXVar->typeCMS) {
			hci_mfcc_t *pCepVarSIL = pUserFeatNorm->cepVarSilence;
			hci_mfcc_t *pCepVarSPCH = pUserFeatNorm->cepVarVoiced;
			hci_mfcc_t *pCepVarUnv = pUserFeatNorm->cepVarUnvoiced;
			hci_mfcc64 *pAccVarSIL = pUserFeatAccum->cepVarSilence;
			hci_mfcc64 *pAccVarSPCH = pUserFeatAccum->cepVarVoiced;
			hci_mfcc64 *pAccVarUnv = pUserFeatAccum->cepVarUnvoiced;
			pCepMeanSIL = pUserFeatNorm->cepMeanSilence;
			pCepMeanSPCH = pUserFeatNorm->cepMeanVoiced;
			pCepMeanUnv = pUserFeatNorm->cepMeanUnvoiced;
			while (pCepMeanSIL < pLastVec) {
				L_val_numer = L_val_denom_sil;					// Q15.32
				LL_val = (*pAccVarSIL);							// Q30.64
				LL_val -= (hci_mfcc64)ncountSIL * (hci_mfcc64)(*pCepMeanSIL) * (hci_mfcc64)(*pCepMeanSIL);	// Q30.64
				L_val_denom = (hci_mfcc_t)(LL_val>>30);			// Q0.32
				L_val_numer = PowerASR_BasicOP_divideShiftLeft_32_32(L_val_numer, L_val_denom, 15);	// Q30.32
				*pCepVarSIL = (hci_mfcc32)PowerASR_BasicOP_fixedSQRT_32_16(L_val_numer);	// Q15.32
				//*pCepVarSIL = PowerASR_BasicOP_fixedSQRT_32_32(L_val_numer);				// Q30.32
				//*pCepVarSIL = PowerASR_BasicOP_shiftRight_32_32(*pCepVarSIL, 15);			// Q15.32

				L_val_numer = L_val_denom_speech;				// Q15.32
				LL_val = (*pAccVarSPCH);						// Q30.64
				LL_val -= (hci_mfcc64)ncountSPCH * (hci_mfcc64)(*pCepMeanSPCH) * (hci_mfcc64)(*pCepMeanSPCH);	// Q30.64
				L_val_denom = (hci_mfcc_t)(LL_val>>30);			// Q0.32
				L_val_numer = PowerASR_BasicOP_divideShiftLeft_32_32(L_val_numer, L_val_denom, 15);	// Q30.32
				*pCepVarSPCH = (hci_mfcc32)PowerASR_BasicOP_fixedSQRT_32_16(L_val_numer);	// Q15.32
				//*pCepVarSPCH = PowerASR_BasicOP_fixedSQRT_32_32(L_val_numer);				// Q15.32
				//*pCepVarSPCH = PowerASR_BasicOP_shiftRight_32_32(*pCepVarSPCH, 15);			// Q15.32

				L_val_numer = L_val_denom_unv;					// Q15.32
				LL_val = (*pAccVarUnv);						// Q30.64
				LL_val -= (hci_mfcc64)ncountUnv * (hci_mfcc64)(*pCepMeanUnv) * (hci_mfcc64)(*pCepMeanUnv);	// Q30.64
				L_val_denom = (hci_mfcc_t)(LL_val>>30);			// Q0.32
				L_val_numer = PowerASR_BasicOP_divideShiftLeft_32_32(L_val_numer, L_val_denom, 15);	// Q30.32
				*pCepVarUnv = (hci_mfcc32)PowerASR_BasicOP_fixedSQRT_32_16(L_val_numer);	// Q15.32
				//*pCepVarUnv = PowerASR_BasicOP_fixedSQRT_32_32(L_val_numer);				// Q15.32
				//*pCepVarUnv = PowerASR_BasicOP_shiftRight_32_32(*pCepVarUnv, 15);			// Q15.32

				pCepMeanSIL++; pCepMeanSPCH++; pCepMeanUnv++;
				pCepVarSIL++; pCepVarSPCH++; pCepVarUnv++;
				pAccVarSIL++; pAccVarSPCH++; pAccVarUnv++;
			}
		}
#else	// !FIXED_POINT_FE
		while (pCepMeanSIL < pLastVec) {
			*pCepMeanSIL++ = (*pAccMeanSIL++) / (hci_mfcc_t)ncountSIL;
			*pCepMeanSPCH++  = (*pAccMeanSPCH++) / (hci_mfcc_t)ncountSPCH;
			*pCepMeanUnv++  = (*pAccMeanUnv++) / (hci_mfcc_t)ncountUnv;
		}
		if (BATCH_CMVN == pFXVar->typeCMS || LIVE_CMVN == pFXVar->typeCMS) {
			hci_mfcc_t *pCepVarSIL = pUserFeatNorm->cepVarSilence;
			hci_mfcc_t *pCepVarSPCH = pUserFeatNorm->cepVarVoiced;
			hci_mfcc_t *pCepVarUnv = pUserFeatNorm->cepVarUnvoiced;
			hci_mfcc_t *pAccVarSIL = pUserFeatAccum->cepVarSilence;
			hci_mfcc_t *pAccVarSPCH = pUserFeatAccum->cepVarVoiced;
			hci_mfcc_t *pAccVarUnv = pUserFeatAccum->cepVarUnvoiced;
			pCepMeanSIL = pUserFeatNorm->cepMeanSilence;
			pCepMeanSPCH = pUserFeatNorm->cepMeanVoiced;
			pCepMeanUnv = pUserFeatNorm->cepMeanUnvoiced;
			while (pCepMeanSIL < pLastVec) {
				L_val_numer = (*pAccVarSIL) / (hci_mfcc_t)ncountSIL;
				L_val_numer -= HCI_SQ(*pCepMeanSIL);
				*pCepVarSIL = 1.0f / (hci_mfcc_t)sqrt((double)L_val_numer);

				L_val_numer = (*pAccVarSPCH) / (hci_mfcc_t)ncountSPCH;
				L_val_numer -= HCI_SQ(*pCepMeanSPCH);
				*pCepVarSPCH = 1.0f / (hci_mfcc_t)sqrt((double)L_val_numer);
				
				L_val_numer = (*pAccVarUnv) / (hci_mfcc_t)ncountUnv;
				L_val_numer -= HCI_SQ(*pCepMeanUnv);
				*pCepVarUnv = 1.0f / (hci_mfcc_t)sqrt((double)L_val_numer);

				pCepMeanSIL++; pCepMeanSPCH++; pCepMeanUnv++;
				pCepVarSIL++; pCepVarSPCH++; pCepVarUnv++;
				pAccVarSIL++; pAccVarSPCH++; pAccVarUnv++;
			}
		}
#endif	// #ifdef FIXED_POINT_FE
	}
	else {
		hci_mfcc_t *pCepMeanSIL = pUserFeatNorm->cepMeanSilence;
		hci_mfcc_t *pCepMeanSPCH = pUserFeatNorm->cepMeanVoiced;
		hci_mfcc_t *pCepMeanUnv = pUserFeatNorm->cepMeanUnvoiced;
		hci_mfcc_t *pAccMeanSIL = pUserFeatAccum->cepMeanSilence;
		hci_mfcc_t *pAccMeanSPCH = pUserFeatAccum->cepMeanVoiced;
		hci_mfcc_t *pAccMeanUnv = pUserFeatAccum->cepMeanUnvoiced;
		hci_mfcc_t *pLastVec = pCepMeanSIL + dimCepstrum;
		hci_int32 ncountSIL = pUserFeatAccum->countSilenceFrame;
        hci_int32 ncountSPCH = pUserFeatAccum->countVoicedFrame;
        hci_int32 ncountUnv = pUserFeatAccum->countUnvoicedFrame;
		hci_mfcc32 L_val_count = 0;
		if (pFXVar->bUseSilenceMean) {
			L_val_denom = (hci_mfcc_t) (ncountSIL + ncountSPCH + ncountUnv);
		}
		else {
			L_val_denom = (hci_mfcc_t) (ncountSPCH + ncountUnv);
		}
#ifdef FIXED_POINT_FE
		L_val_count = (L_val_denom<<15);			// Q15.32
		L_val_denom = L_val_count;
		if (pFXVar->bUseSilenceMean) {
			while (pCepMeanSIL < pLastVec) {
				L_val_numer = (*pAccMeanSIL) + (*pAccMeanSPCH) + (*pAccMeanUnv);
				*pCepMeanSIL = PowerASR_BasicOP_divideShiftLeft_32_32(L_val_numer, L_val_denom, 15);
				*pCepMeanSPCH  = *pCepMeanSIL;
				*pCepMeanUnv  = *pCepMeanSIL;
				pCepMeanSIL++; pCepMeanSPCH++; pCepMeanUnv++;
				pAccMeanSIL++; pAccMeanSPCH++; pAccMeanUnv++;
			}
		}
		else {
			while (pCepMeanSIL < pLastVec) {
				L_val_numer = (*pAccMeanSPCH) + (*pAccMeanUnv);
				*pCepMeanSPCH = PowerASR_BasicOP_divideShiftLeft_32_32(L_val_numer, L_val_denom, 15);
				*pCepMeanSIL  = *pCepMeanSPCH;
				*pCepMeanUnv  = *pCepMeanSPCH;
				pCepMeanSIL++; pCepMeanSPCH++; pCepMeanUnv++;
				pAccMeanSIL++; pAccMeanSPCH++; pAccMeanUnv++;
			}
		}
		if (BATCH_CMVN == pFXVar->typeCMS || LIVE_CMVN == pFXVar->typeCMS) {
			hci_mfcc_t *pCepVarSIL = pUserFeatNorm->cepVarSilence;
			hci_mfcc_t *pCepVarSPCH = pUserFeatNorm->cepVarVoiced;
			hci_mfcc_t *pCepVarUnv = pUserFeatNorm->cepVarUnvoiced;
			hci_mfcc64 *pAccVarSIL = pUserFeatAccum->cepVarSilence;
			hci_mfcc64 *pAccVarSPCH = pUserFeatAccum->cepVarVoiced;
			hci_mfcc64 *pAccVarUnv = pUserFeatAccum->cepVarUnvoiced;
			pCepMeanSIL = pUserFeatNorm->cepMeanSilence;
			while (pCepMeanSIL < pLastVec) {
				L_val_numer = L_val_count;						// Q15.32
				LL_val = (*pAccVarSIL) + (*pAccVarSPCH) + (*pAccVarUnv);		// Q30.64
				LL_val -= (hci_mfcc64)(ncountSIL+ncountSPCH+ncountUnv) * (hci_mfcc64)(*pCepMeanSIL) * (hci_mfcc64)(*pCepMeanSIL);	// Q30.64
				L_val_denom = (hci_mfcc_t)(LL_val>>30);			// Q0.32
				L_val_numer = PowerASR_BasicOP_divideShiftLeft_32_32(L_val_numer, L_val_denom, 15);	// Q30.32
				*pCepVarSIL = (hci_mfcc32)PowerASR_BasicOP_fixedSQRT_32_16(L_val_numer);	// Q15.32
				//*pCepVarSIL = PowerASR_BasicOP_fixedSQRT_32_32(L_val_numer);				// Q15.32
				//*pCepVarSIL = PowerASR_BasicOP_shiftRight_32_32(*pCepVarSIL, 15);			// Q15.32
				*pCepVarSPCH  = *pCepVarSIL;
				*pCepVarUnv   = *pCepVarSIL;
				pCepMeanSIL++;
				pCepVarSIL++; pCepVarSPCH++; pCepVarUnv++;
				pAccVarSIL++; pAccVarSPCH++; pAccVarUnv++;
			}
		}
#else	// !FIXED_POINT_FE
		if (pFXVar->bUseSilenceMean) {
			while (pCepMeanSIL < pLastVec) {
				L_val_numer = (*pAccMeanSIL) + (*pAccMeanSPCH) + (*pAccMeanUnv);
				*pCepMeanSIL = L_val_numer / L_val_denom;
				*pCepMeanSPCH  = *pCepMeanSIL;
				*pCepMeanUnv   = *pCepMeanSIL;
				pCepMeanSIL++; pCepMeanSPCH++; pCepMeanUnv++;
				pAccMeanSIL++; pAccMeanSPCH++; pAccMeanUnv++;
			}
		}
		else {
			while (pCepMeanSIL < pLastVec) {
				*pCepMeanSPCH = ((*pAccMeanSPCH) + (*pAccMeanUnv)) / L_val_denom;
				*pCepMeanSIL  = *pCepMeanSPCH;
				*pCepMeanUnv  = *pCepMeanSPCH;
				pCepMeanSIL++; pCepMeanSPCH++; pCepMeanUnv++;
				pAccMeanSIL++; pAccMeanSPCH++; pAccMeanUnv++;
			}
		}
		if (BATCH_CMVN == pFXVar->typeCMS || LIVE_CMVN == pFXVar->typeCMS) {
			hci_mfcc_t *pCepVarSIL = pUserFeatNorm->cepVarSilence;
			hci_mfcc_t *pCepVarSPCH = pUserFeatNorm->cepVarVoiced;
			hci_mfcc_t *pCepVarUnv = pUserFeatNorm->cepVarUnvoiced;
			hci_mfcc64 *pAccVarSIL = pUserFeatAccum->cepVarSilence;
			hci_mfcc64 *pAccVarSPCH = pUserFeatAccum->cepVarVoiced;
			hci_mfcc64 *pAccVarUnv = pUserFeatAccum->cepVarUnvoiced;
			pCepMeanSIL = pUserFeatNorm->cepMeanSilence;
			while (pCepMeanSIL < pLastVec) {
				L_val_numer = (*pAccVarSIL) + (*pAccVarSPCH) + (*pAccVarUnv);
				L_val_numer /= L_val_denom;
				L_val_numer -= HCI_SQ(*pCepMeanSIL);
				*pCepVarSIL = 1.0f / (hci_mfcc_t)sqrt((double)L_val_numer);
				*pCepVarSPCH  = *pCepVarSIL;
				*pCepVarUnv = *pCepVarSIL;
				pCepMeanSIL++;
				pCepVarSIL++; pCepVarSPCH++; pCepVarUnv++;
				pAccVarSIL++; pAccVarSPCH++; pAccVarUnv++;
			}
		}
#endif	// #ifdef FIXED_POINT_FE
	}

	memcpy( &pMfccStream->userFeatNorm, &pMfccStream->baseFeatNorm, sizeof(pMfccStream->baseFeatNorm) );

	return 0;
}


/* end of file */






















