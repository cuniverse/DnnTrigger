
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
 *	@file	hci_FrontEnd.c
 *	@ingroup frontend_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	PowerASR Front-End API functions.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
#include <windows.h>
#endif

#include "base/hci_malloc.h"
#include "base/hci_msg.h"
#include "base/parse_config.h"
#include "base/case.h"
#include "basic_op/basic_op.h"

#include "speex/hci_speex.h"
#include "wiener/hci_wiener.h"
#include "wave2mfcc/hci_wave2mfcc.h"
#include "epd/hci_epd.h"
#include "mfcc2feat/hci_fx_mfcc2feat.h"
#include "frontend/hci_FrontEnd.h"



//#include "D:/TRAINING_관련/POWERDSR_KWS2/PowerDSR_KWS_SRC/PowerDSR_LIB/include/wave2mfcc/fx_mfcc_common.h" //yowon 2011-12-06
#include "wave2mfcc/fx_mfcc_common.h"
/**
 *	@struct inner data structure of PowerASR Front-End engine
 */
typedef struct 
{
	/// utility modules
	PowerASR_BasicOP *pBasicOP;					///< basic fixed-point operation library

	/// front-end modules
	PowerASR_NR_Wiener *pNR_Wiener;				///< Wiener filter noise-reducer
	PowerASR_FX_Wave2Mfcc *pFX_Wave2Mfcc;		///< wave-to-MFCC converter
	PowerASR_EPD *pEpd;							///< speech end-point detector
	PowerASR_FX_Mfcc2Feat* pFX_Mfcc2Feat;		///< MFCC-to-feature converter
	

	/// configuration files
	char *pszWave2MfccCfgFile;				///< configuration file for wave-to-MFCC converter
	char *pszWave2MfccCfgFile16k;				///< configuration file for wave-to-MFCC converter
	char *pszWave2MfccCfgFile8k;				///< configuration file for wave-to-MFCC converter
	char *pszEpdCfgFile;					///< configuration file for speech end-point detector
	char *pszMfcc2FeatCfgFile;				///< configuration file for MFCC-to-feature converter

	hci_int32 nLenLogSpeechMargin;


	float *refSilFeat;						/// KSH 15. 10. 19 // LG u+

	int nSampleRate;				// jyb 16. 01. 12

} FrontEnd_Inner;

// local functions
#ifdef __cplusplus
extern "C" {
#endif 

/**
 * setup default environments for PowerASR front-end modules
 */
HCILAB_PRIVATE hci_int32
_FrontEnd_defaultConfigurations(PowerASR_FrontEnd *pThis);		///< (i/o) pointer to the ASR front-end engine


/**
 * setup environments from a given configuration file
 */
HCILAB_PRIVATE hci_int32
_FrontEnd_loadConfigurations(PowerASR_FrontEnd *pThis,		///< (i/o) pointer to the ASR front-end engine
							 const char *pszWorkingDir,		///< (i) working directory
							 const char *pszConfigFile);	///< (i) ASR main configuration file

#ifdef __cplusplus
}
#endif


/**
 *	Create a new ASR Front-End engine.
 *
 *	@return ASR Front-End engine struct. 실패 시 null 리턴.
 */
HCILAB_PUBLIC HCI_FE_API PowerASR_FrontEnd*
PowerASR_FrontEnd_new()
{
	PowerASR_FrontEnd *pFE = 0;
	FrontEnd_Inner *pInner = 0;
	FILE* fp_refSilFeature_rb = NULL;
	//KSH

	pFE = (PowerASR_FrontEnd *) hci_malloc( sizeof(PowerASR_FrontEnd) );

	if ( pFE ) {
		memset(pFE, 0, sizeof(PowerASR_FrontEnd));

		pInner = (FrontEnd_Inner *) hci_malloc( sizeof(FrontEnd_Inner) );
		if ( pInner ) {
			memset(pInner, 0, sizeof(FrontEnd_Inner));
			pFE->pInner = (void *)pInner;

			pInner->pBasicOP = PowerASR_BasicOP_new();

			// create front-end modules
			pInner->pNR_Wiener = PowerASR_NR_Wiener_new();
			if (0 == pInner->pNR_Wiener) {
				HCIMSG_ERROR("[Warning] cannot create Wiener Noise Reducer !!\n");
				return pFE;
			}

			pInner->pFX_Wave2Mfcc = PowerASR_FX_Wave2Mfcc_new();
			if (0 == pInner->pFX_Wave2Mfcc) {
				HCIMSG_ERROR("[Warning] cannot create FX wave-to-mfcc converter !!\n");
				return pFE;
			}

			pInner->pEpd = PowerASR_EPD_new();
			if (0 == pInner->pEpd) {
				HCIMSG_ERROR("[Warning] cannot create end-point detector !!\n");
				return pFE;
			}

			pInner->pFX_Mfcc2Feat = PowerASR_FX_Mfcc2Feat_new();
			if (0 == pInner->pFX_Mfcc2Feat) {
				HCIMSG_ERROR("[Warning] cannot create FX mfcc-to-feat converter !!\n");
				return pFE;
			}

			//---Start of reading silence feature for appending 20 frames reference silence feature - KSH_20151018

			fp_refSilFeature_rb = fopen("./res/refSilFeature.dat","rb");
			if (fp_refSilFeature_rb != NULL) {
				int silLen = 0;
				fseek(fp_refSilFeature_rb, 0, SEEK_END);
				silLen = ftell(fp_refSilFeature_rb);
				fseek(fp_refSilFeature_rb, 0, SEEK_SET);
				pInner->refSilFeat = (float*) malloc(silLen);
				if (pInner->refSilFeat != NULL) {
					fread(pInner->refSilFeat, 1, silLen, fp_refSilFeature_rb);
				}
				fclose(fp_refSilFeature_rb);
			} else {
				pInner->refSilFeat = NULL;
			}
			//---End of reading silence feature for appending 20 frames reference silence feature - KSH_20151018
		}

	}
	else {
		HCIMSG_ERROR("cannot create PowerASR front-end engine.\n");
	}

	return pFE;
}


/**
 *	Delete the ASR front-end engine.
 *
 *	@return none.
 */
HCILAB_PUBLIC HCI_FE_API void
PowerASR_FrontEnd_delete(PowerASR_FrontEnd *pThis)		///< (i/o) pointer to the ASR front-end engine
{
	FrontEnd_Inner *pInner = 0;

	if (0 == pThis) {
		return;
	}

	pInner = (FrontEnd_Inner *) pThis->pInner;

	if (pInner) {
		if (pInner->pBasicOP) {
			PowerASR_BasicOP_delete(pInner->pBasicOP);
			pInner->pBasicOP = 0;
		}

		// destroy front-end modules
		if (pInner->pNR_Wiener) {
			PowerASR_NR_Wiener_delete(pInner->pNR_Wiener);
			pInner->pNR_Wiener = 0;
		}

		if (pInner->pFX_Wave2Mfcc) {
			PowerASR_FX_Wave2Mfcc_delete(pInner->pFX_Wave2Mfcc);
			pInner->pFX_Wave2Mfcc = 0;
		}

		if (pInner->pEpd) {
			PowerASR_EPD_delete(pInner->pEpd);
			pInner->pEpd = 0;
		}

		if (pInner->pFX_Mfcc2Feat) {
			PowerASR_FX_Mfcc2Feat_delete(pInner->pFX_Mfcc2Feat);
			pInner->pFX_Mfcc2Feat = 0;
		}

		if (pInner->pszWave2MfccCfgFile) {
			hci_free(pInner->pszWave2MfccCfgFile);
			pInner->pszWave2MfccCfgFile = 0;
		}
		if (pInner->pszEpdCfgFile) {
			hci_free(pInner->pszEpdCfgFile);
			pInner->pszEpdCfgFile = 0;
		}
		if (pInner->pszMfcc2FeatCfgFile) {
			hci_free(pInner->pszMfcc2FeatCfgFile);
			pInner->pszMfcc2FeatCfgFile = 0;
		}

		if ( pInner ) { hci_free(pInner); pInner = 0; }
	}

	if ( pThis ) { hci_free(pThis); pThis = 0; }
}


/**
 *	Set-up environments for ASR front-end engine,\n and allocate necessary memories.
 *
 *	음성인식용 front-end module에 필요한 knowledge base와 설정 조건을 로딩한다.
 *
 *	@return return 0 if ASR front-end environments are set-up correctly, otherwise return -1.
 */
HCILAB_PUBLIC HCI_FE_API hci_int32
PowerASR_FrontEnd_openFrontEndEngine(PowerASR_FrontEnd *pThis,		///< (i/o) pointer to the ASR front-end engine
									 const char *pszWorkingDir,		///< (i) working directory name
									 const char *pszConfigFile,		///< (i) main ASR configuration file
									 const hci_int32 nSampleRate)	///< (i) sampling frequency in Hz
{
	FrontEnd_Inner *pInner = 0;
	hci_logadd_t *pLogAddTbl_FE = 0;
	hci_int32 bSetup = 0;

	if (0 == pThis) {
		return -1;
	}

	pInner = (FrontEnd_Inner *) pThis->pInner;

	if (0 == pInner) {
		return -1;
	}
	pInner->nSampleRate = nSampleRate;

	if (0 == pszConfigFile) {
		bSetup = _FrontEnd_defaultConfigurations(pThis);
	}
	else {
		bSetup = _FrontEnd_loadConfigurations(pThis, pszWorkingDir, pszConfigFile);
	}

	if (pInner->pBasicOP) {
		pLogAddTbl_FE = PowerASR_BasicOP_getLogAdditionTable(pInner->pBasicOP);
	}

	// open Front-End modules
	if (pInner->pNR_Wiener) {
		if ((-1) == PowerASR_NR_Wiener_openWienerNR(pInner->pNR_Wiener,
													pInner->pszEpdCfgFile,
													nSampleRate)) {
			return -2;
		}
	}
	// bgy

	if (pInner->pFX_Wave2Mfcc) {
		if (nSampleRate == 16000) {
			if ((-1) == PowerASR_FX_Wave2Mfcc_openWave2MfccConverter(pInner->pFX_Wave2Mfcc,
																	 pInner->pszWave2MfccCfgFile16k,
																	 nSampleRate,
																	 pLogAddTbl_FE)) {
				return -3;
			}
		} else if (nSampleRate == 8000) {
			if ((-1) == PowerASR_FX_Wave2Mfcc_openWave2MfccConverter(pInner->pFX_Wave2Mfcc,
																	 pInner->pszWave2MfccCfgFile8k,
																	 nSampleRate,
																	 pLogAddTbl_FE)) {
				return -3;
			}
		} else {
			if ((-1) == PowerASR_FX_Wave2Mfcc_openWave2MfccConverter(pInner->pFX_Wave2Mfcc,
																	 pInner->pszWave2MfccCfgFile,
																	 nSampleRate,
																	 pLogAddTbl_FE)) {
				return -3;
			}
		}
	}

	if (pInner->pEpd) {
		if ((-1) == PowerASR_EPD_openEPDetector(pInner->pEpd,
												pInner->pszEpdCfgFile)) {
			return -4;
		}
	}

	if (pInner->pFX_Mfcc2Feat) {
		if ((-1) == PowerASR_FX_Mfcc2Feat_openMfcc2FeatConverter(pInner->pFX_Mfcc2Feat,
																 pszWorkingDir,
																 pInner->pszMfcc2FeatCfgFile)) {
			return -5;
		}
	}

	return bSetup;

}
HCILAB_PUBLIC HCI_FE_API hci_int32 PowerASR_FrontEnd_releaseFrontEndWiener(FrontEnd_UserData *pUserThis,PowerASR_FrontEnd *pFronEndThis)
{
	Wiener_UserData *pWienerUser_Inner;
	PowerASR_NR_Wiener *pWiener_Inner;

	pWienerUser_Inner = &pUserThis->dataWiener;
	pWiener_Inner = (PowerASR_NR_Wiener *)pFronEndThis;

	PowerASR_NR_Wiener_releaseWienerNR(pWiener_Inner, pWienerUser_Inner);
	return 1;
}

/**
 *	Free memories allocated to the ASR front-end engine.
 *
 *	@return return 0 if all memories are freed successfully, otherwise return -1.
 */
HCILAB_PUBLIC HCI_FE_API hci_int32
PowerASR_FrontEnd_closeFrontEndEngine(PowerASR_FrontEnd *pThis)		///< (i/o) pointer to the ASR front-end engine
{
	FrontEnd_Inner *pInner = 0;

	if (0 == pThis) {
		return -1;
	}

	pInner = (FrontEnd_Inner *) pThis->pInner;

	if (0 == pInner) {
		return -1;
	}

	// close front-end modules

	if (pInner->pNR_Wiener) {
		PowerASR_NR_Wiener_closeWienerNR(pInner->pNR_Wiener);
	}

	if (pInner->pFX_Wave2Mfcc) {
		PowerASR_FX_Wave2Mfcc_closeWave2MfccConverter(pInner->pFX_Wave2Mfcc);
	}

	if (pInner->pEpd) {
		PowerASR_EPD_closeEPDetector(pInner->pEpd);
	}

	if (pInner->pFX_Mfcc2Feat) {
		PowerASR_FX_Mfcc2Feat_closeMfcc2FeatConverter(pInner->pFX_Mfcc2Feat);
	}

	return 0;
}


/**
 *	Initialize data buffers for front-end modules of ASR engine.
 *
 *	@return return 0 if user-specific front-end data buffers are initialized successfully, otherwise return -1.
 */
HCILAB_PUBLIC HCI_FE_API hci_int32
/*
PowerASR_FrontEnd_initializeFrontEndforASR(PowerASR_FrontEnd *pThis,				///< (i) pointer to the ASR front-end engine
										   FrontEnd_UserData *pChannelDataFE,		///< (o) channel-specific front-end data struct
										   const hci_flag bInitFE,					///< (i) flag to initialize front-end user data
										   const hci_flag bContinuousEPD)			///< (i) flag to continuous-mode EPD
*/
PowerASR_FrontEnd_initializeFrontEndforASR(PowerASR_FrontEnd *pThis,				///< (i) pointer to the ASR front-end engine
										   FrontEnd_UserData *pChannelDataFE,		///< (o) channel-specific front-end data struct
										   const hci_flag bInitFE,					///< (i) flag to initialize front-end user data
										   const hci_flag bContinuousEPD,char *cmsModelName)			///< (i) flag to continuous-mode EPD
{
	FrontEnd_Inner *pInner = 0;

	if (0 == pThis) {
		return -1;
	}

	pInner = (FrontEnd_Inner *) pThis->pInner;

	if (0 == pInner) {
		return -1;
	}

	if (pInner->pNR_Wiener) {
		if ((-1) == PowerASR_NR_Wiener_initializeWienerNR(pInner->pNR_Wiener,
														  &pChannelDataFE->dataWiener)) {
			HCIMSG_ERROR("[Warning] cannot initialize Wiener Noise Reducer !!\n");
			return -1;
		}
	}

	if (pInner->pFX_Wave2Mfcc) {
		if ((-1) == PowerASR_FX_Wave2Mfcc_initializeWave2MfccConverter(pInner->pFX_Wave2Mfcc,
																	   &pChannelDataFE->dataMfcc)) {
			HCIMSG_ERROR("[Warning] cannot initialize FX wave-to-mfcc converter !!\n");
			return -1;
		}
	}

	if (pInner->pEpd) {
		if ((-1) == PowerASR_EPD_initializeEPDetector(pInner->pEpd,
													  &pChannelDataFE->dataEpd,
													  bContinuousEPD)) {
			HCIMSG_ERROR("[Warning] cannot initialize end-point detector !!\n");
			return -1;
		}
	}

	if (pInner->pFX_Mfcc2Feat) {
		if ((-1) == PowerASR_FX_Mfcc2Feat_initializeMfcc2FeatConverter(pInner->pFX_Mfcc2Feat,
																	   &pChannelDataFE->dataASRFeat,
																	   bInitFE)) {
			HCIMSG_ERROR("[Warning] cannot initialize mfcc-to-feat converter !!\n");
			return -1;
		}
	}

	pChannelDataFE->refSilFeat  = pInner->refSilFeat; 

	pChannelDataFE->dataFX.frameCnt = 0;
	pChannelDataFE->dataFX.nLenTimeOutTh    = 100 * pInner->nSampleRate;
	pChannelDataFE->dataFX.nLenSpeechTh     = 100 * pInner->nSampleRate;

	//bgy
	//pChannelDataFE->dataFX.nFeatDim         = pChannelDataFE->dataASRFeat.featStream.dimFeat;
	pChannelDataFE->dataFX.nFeatDim         = pChannelDataFE->dataASRFeat.featStream->dimFeat;

	pChannelDataFE->dataFX.nFrameSampleSize = pInner->nSampleRate / 100;//KSH_20151218 : nSampleRate에 따라 변화
	pChannelDataFE->dataFX.nFEStatus        = OPEN_BE_CHANNEL;
	

	pChannelDataFE->dataFX.nFEStatus       = INIT_BE_STATE;

	pChannelDataFE->dataFX.nLenProcessWave = 0;
	pChannelDataFE->dataFX.nLenSendFrame   = 0;
	pChannelDataFE->dataFX.bEOS            = FALSE;
	pChannelDataFE->dataFX.bFXComplete     = FALSE;
	pChannelDataFE->dataFX.bUnusualSpeech  = FALSE;
	pChannelDataFE->dataFX.bExit           = FALSE;
	pChannelDataFE->dataFX.bReceiveRefSilFeat = 0;
	pChannelDataFE->dataFX.nRefSilFeatSize = 0;

//	_PowerDSR_FE_setFeatureNormalizer( nChannelID );
	
	return 0;
}


/**
 *	 Release data buffers for front-end modules of ASR engine.
 *
 *	@return return 0 if user-specific front-end data buffers are released successfully, otherwise return -1.
 */
HCILAB_PUBLIC HCI_FE_API hci_int32
PowerASR_FrontEnd_terminateFrontEndforASR(PowerASR_FrontEnd *pThis,				///< (i) pointer to the ASR front-end engine
										  FrontEnd_UserData *pChannelDataFE)	///< (o) channel-specific front-end data struct
{
	FrontEnd_Inner *pInner = 0;

	if (0 == pThis) {
		return -1;
	}

	pInner = (FrontEnd_Inner *) pThis->pInner;

	if (0 == pInner) {
		return -1;
	}

#ifdef __AUDIORECOG_API__
	// kklee 20150922
	if (pChannelDataFE->st) {
		DioAudioRecogDestroy(pChannelDataFE->st);
		pChannelDataFE->st = 0;
	}
#endif

	return PowerASR_FX_Mfcc2Feat_terminateMfcc2FeatConverter(pInner->pFX_Mfcc2Feat, &pChannelDataFE->dataASRFeat );
}


/**
 *	frame-by-frame feature extraction.\n
 *	-# noise reduction (Wiener filtering)
 *	-# wave-to-MFCC conversion
 *	-# end-point detection
 *	-# MFCC-to-feature conversion (iff current frame is on speech region)
 *
 *	@return return the status of output feature vector.
 */

HCILAB_PUBLIC HCI_FE_API M2F_Status
PowerASR_FrontEnd_framebyframeFeatureExtraction(PowerASR_FrontEnd *pThis,			///< (i) pointer to the ASR front-end engine
												FrontEnd_UserData *pChannelDataFE,	///< (o) channel-specific front-end data struct
												hci_int16 *pFrameWave,				///< (i/o) original frame wave buffer, noise-reduced wave buffer
												hci_flag bEOS)						///< (i) flag to end-of-speech
{
	FrontEnd_Inner *pInner = (FrontEnd_Inner *)pThis->pInner;
	Wiener_UserData *pWienerData = &pChannelDataFE->dataWiener;
	MFCC_UserData *pMfccData = &pChannelDataFE->dataMfcc;
	EPD_UserData *pEpdData = &pChannelDataFE->dataEpd;
	Feature_UserData *pASRFeat = &pChannelDataFE->dataASRFeat;
	DSR_FX_DATA *pFX = &pChannelDataFE->dataFX;

	hci_flag epd_flag = 0;
	M2F_Status m2f_state = M2F_FALSE;
	W2M_Status w2m_stat;
	hci_int32 nLenOutWave = 0;
	ARData* dioEpd = pChannelDataFE->st;

	if (!pFrameWave && bEOS) {
		pASRFeat->epd_result.epd_state = UTTER_END;
		pASRFeat->epd_result.frame_class = SIL_FRAME;
		m2f_state = PowerASR_FX_Mfcc2Feat_convertMfccStream2FeatureVector(
			pInner->pFX_Mfcc2Feat, pASRFeat, pMfccData, FALSE);
		return m2f_state;
	}

	if (!pFrameWave)	return M2F_FALSE;


	nLenOutWave = PowerASR_NR_Wiener_procFrameBuffer(pInner->pNR_Wiener, pWienerData, pFrameWave);
	if (nLenOutWave != pFX->nFrameSampleSize)	return M2F_FALSE;


	w2m_stat = PowerASR_FX_Wave2Mfcc_convertFrameSample2Mfcc(
		pInner->pFX_Wave2Mfcc, pMfccData, pFrameWave, (hci_int16)(pWienerData->flagVAD/2));
	if (W2M_TRUE != w2m_stat)	return M2F_FALSE;

#if 0
	if (pEpdData->bUseDNNEPD) {	// use DNN EPD
		dioEpd->rfft = pMfccData->xfft;
		dioEpd->rfft_size = pMfccData->xfft_len;
		dioEpd->mfcc = pMfccData->mfccVec;
		dioEpd->mfcc_size = pMfccData->nDimMFCC;
		DioAudioEPDProc(pFrameWave, pFX->nFrameSampleSize, dioEpd);
		pEpdData->nPrevState = pEpdData->nCurrentState;
		pEpdData->bDetectStartPt = pEpdData->bDetectEndPt = FALSE;
		epd_flag = EPD_STATUS_NOT_DETECTED;

		pEpdData->nEpdFrame = (hci_int32)dioEpd->frameNum;

		if ((dioEpd->range.stat == 2) && (pEpdData->nPrevState == 1)) {
			pEpdData->bDetectStartPt = TRUE;
		}
		pEpdData->nStartFrame = HCI_MAX(dioEpd->range.start, 0);
		pEpdData->nCurrentState = (hci_int16)dioEpd->range.stat;

		pEpdData->frame_class = pWienerData->flagVAD; //pWienerData->flagVAD; //dioEpd->frame_class;
		pEpdData->nEndFrame = dioEpd->range.end;
		pEpdData->bResetStartPt = FALSE;

		//length는 끝점이 잡혔을 경우에 값이 들어간다
		if (dioEpd->range.stat == 4) {
			epd_flag = EPD_STATUS_DETECTED;
			pEpdData->nCurrentState = UTTER_END;
			pEpdData->frame_class = SIL_FRAME;
			pEpdData->bDetectEndPt = TRUE;
		}
	} else {	// use Energy-based EPD
		epd_flag = PowerASR_EPD_detectEndPoint(pInner->pEpd, pEpdData,
			pMfccData->mfccVec[pMfccData->nDimMFCC-1], pMfccData->frameEn, pMfccData->nSpecEntropy,
			pWienerData->flagVAD, pWienerData->bSpeechFound);
	}
#else
	pEpdData->nPrevState = pEpdData->nCurrentState;
	pEpdData->bDetectStartPt = pEpdData->bDetectEndPt = FALSE;
	epd_flag = EPD_STATUS_NOT_DETECTED;

	if (UTTER_START == pEpdData->nPrevState) {
		pEpdData->nCurrentState = SIL_SPEECH;
		pEpdData->nStartFrame = 1;
		pEpdData->bDetectStartPt = TRUE;
	}
	else if (SIL_SPEECH == pEpdData->nPrevState) {
		pEpdData->nCurrentState = CORE_SPEECH;
	}

	pEpdData->frame_class = 3; // voiced
	pEpdData->bResetStartPt = FALSE;
#endif


	if (bEOS) {
		pASRFeat->epd_result.epd_state = UTTER_END;
		pASRFeat->epd_result.frame_class = SIL_FRAME;
		m2f_state = PowerASR_FX_Mfcc2Feat_convertMfccStream2FeatureVector(
			pInner->pFX_Mfcc2Feat, pASRFeat, pMfccData, FALSE);
		return m2f_state;
	}

	pEpdData->nStartFrame = HCI_MAX(pEpdData->nStartFrame, 0);
	pASRFeat->epd_result.epd_state   = pEpdData->nCurrentState;
	pASRFeat->epd_result.frame_class = pEpdData->frame_class;
	pASRFeat->epd_result.nEpdFrame   = pEpdData->nEpdFrame;
	pASRFeat->epd_result.nStartFrame = pEpdData->nStartFrame;
	pASRFeat->epd_result.nEndFrame   = pEpdData->nEndFrame;
	if (pEpdData->bDetectStartPt) {	// likely start point of speech was detected
		pASRFeat->mfccStream.epd_state = SIL_SPEECH;	// necessary to initialize feature data
		pASRFeat->lenFeatStream = 0;
	}
	else if (pEpdData->bResetStartPt) {
		pASRFeat->lenFeatStream = 0;
	}

	//pASRFeat->epd_result.frame_class = VOICED_FRAME;
	m2f_state = PowerASR_FX_Mfcc2Feat_convertMfccStream2FeatureVector(
		pInner->pFX_Mfcc2Feat, pASRFeat, pMfccData, TRUE);

	return m2f_state;
}


/**
 *	 Get seed feature normalization vector.
 *
 *	@return return 0 if seed feature normalization vectors are copied successfully, otherwise return -1.
 */
HCILAB_PUBLIC HCI_FE_API hci_int32
PowerASR_FrontEnd_getSeedFeatureNormalizer(PowerASR_FrontEnd *pThis,			///< (i) pointer to the ASR front-end engine
										   FEAT_Normalizer *pOutFeatNorm)		///< (o) feature normalization vectors
{
	FrontEnd_Inner *pInner = 0;

	if (0 == pThis) {
		return -1;
	}
	if (0 == pOutFeatNorm) {
		return -1;
	}

	pInner = (FrontEnd_Inner *) pThis->pInner;

	if (0 == pInner) {
		return -1;
	}

	if (pInner->pFX_Mfcc2Feat) {
		if ((-1) == PowerASR_FX_Mfcc2Feat_getFeatureNormalizer( pInner->pFX_Mfcc2Feat,
																pOutFeatNorm ) ) {
			HCIMSG_ERROR("[Warning] cannot get feature normalization vectors !!\n");
			return -1;
		}
	}
	else {
		return -1;
	}

	return 0;
}

/**
 *	 Set feature normalization vectors.
 *
 *	@return return 0 if seed feature normalization vectors are set successfully, otherwise return -1.
 */
/*
HCILAB_PUBLIC HCI_FE_API hci_int32
PowerASR_FrontEnd_setFeatureNormalizer(PowerASR_FrontEnd *pThis,				///< (i) pointer to the ASR front-end engine
									   FrontEnd_UserData *pChannelDataFE,		///< (o) channel-specific front-end data struct
									   const FEAT_Normalizer *pSeedFeatNorm)	///< (i) feature normalization vectors
*/
HCILAB_PUBLIC HCI_FE_API hci_int32
PowerASR_FrontEnd_setFeatureNormalizer(PowerASR_FrontEnd *pThis,				///< (i) pointer to the ASR front-end engine
									   FrontEnd_UserData *pChannelDataFE,		///< (o) channel-specific front-end data struct
									   const FEAT_Normalizer *pSeedFeatNorm,char *cmsModelName,const LONG nChannelID)	///< (i) feature normalization vectors
{

	FrontEnd_Inner *pInner = 0;
	MFCC_Stream *pMfccStream = 0;

	if (0 == pThis) {
		return -1;
	}
	if (0 == pChannelDataFE) {
		return -1;
	}
	if (0 == pSeedFeatNorm) {
		return -1;
	}

	pInner = (FrontEnd_Inner *) pThis->pInner;

	if (0 == pInner) {
		return -1;
	}

	if (pInner->pFX_Mfcc2Feat) {
		pMfccStream = &pChannelDataFE->dataASRFeat.mfccStream;
/*
		if ((-1) == PowerASR_FX_Mfcc2Feat_setFeatureNormalizer( pInner->pFX_Mfcc2Feat,
																pMfccStream,
																pSeedFeatNorm ) ) {
			HCIMSG_ERROR("[Warning] cannot set feature normalization vectors !!\n");
			return -1;
		}
*/
		//if ((-1) == PowerASR_FX_Mfcc2Feat_setFeatureNormalizer( pInner->pFX_Mfcc2Feat,//KSH
		//														pMfccStream,
		//														pSeedFeatNorm, cmsModelName, nChannelID) ) {
		//	HCIMSG_ERROR("[Warning] cannot set feature normalization vectors !!\n");
		//	return -1;
		//}
		memcpy(&pChannelDataFE->gCMSVector,pSeedFeatNorm, sizeof(FEAT_Normalizer));
		if ((-1) == PowerASR_FX_Mfcc2Feat_setClientFeatureNormalizer(pInner->pFX_Mfcc2Feat,//KSH
														pMfccStream,
														NULL, 
														cmsModelName, 
														nChannelID,
														&pChannelDataFE->gCMSVector)) {
			HCIMSG_ERROR("[Warning] cannot set feature normalization vectors !!\n");
			return -1;
		}
	}
	else {
		return -1;
	}

	return 0;
}

HCILAB_PUBLIC HCI_FE_API hci_int32
PowerASR_FrontEnd_setClientFeatureNormalizer(PowerASR_FrontEnd *pThis,				///< (i) pointer to the ASR front-end engine
									   FrontEnd_UserData *pChannelDataFE,		///< (o) channel-specific front-end data struct
									   const FEAT_Normalizer *pSeedFeatNorm,char *cmsModelName,const LONG nChannelID)	///< (i) feature normalization vectors
{
	FrontEnd_Inner *pInner = 0;
	MFCC_Stream *pMfccStream = 0;

	if (0 == pThis) {
		return -1;
	}
	if (0 == pChannelDataFE) {
		return -1;
	}
	if (0 == pSeedFeatNorm) {
		return -1;
	}

	pInner = (FrontEnd_Inner *) pThis->pInner;

	if (0 == pInner) {
		return -1;
	}
	
	//memcpy(&pChannelDataFE->gCMSVector, pSeedFeatNorm, sizeof(FEAT_Normalizer) );
	if (pInner->pFX_Mfcc2Feat) {
		pMfccStream = &pChannelDataFE->dataASRFeat.mfccStream;

		if ((-1) == PowerASR_FX_Mfcc2Feat_setClientFeatureNormalizer( pInner->pFX_Mfcc2Feat,//KSH
																pMfccStream,
																pSeedFeatNorm, cmsModelName, nChannelID, &pChannelDataFE->gCMSVector) ) {
			HCIMSG_ERROR("[Warning] cannot set feature normalization vectors !!\n");
			return -1;
		}
	}
	else {
		return -1;
	}

	return 0;
}



HCILAB_PUBLIC HCI_FE_API hci_int32
PowerASR_FrontEnd_getCMSSeed(int CHID,FEAT_Normalizer *lf)	///< (i) feature normalization vectors
{
	
	return PowerASR_FX_Mfcc2Feat_getCMSSeed(CHID,lf);
}


/**
 *	get marginal length of log-speech data in sample count.
 *
 *	@return return the marginal length of log-speech data in sample count.
 */
HCILAB_PUBLIC HCI_FE_API hci_int32
PowerASR_FrontEnd_getLogSpeechMargin(PowerASR_FrontEnd *pThis)		///< (i) pointer to the ASR front-end engine
{
	FrontEnd_Inner *pInner = 0;

	if ( !pThis ) return 0;

	pInner = (FrontEnd_Inner *) pThis->pInner;

	return pInner->nLenLogSpeechMargin;
}


/**
 * setup default environments for PowerASR front-end modules
 */
HCILAB_PRIVATE hci_int32
_FrontEnd_defaultConfigurations(PowerASR_FrontEnd *pThis)		///< (i/o) pointer to the ASR front-end engine
{
	FrontEnd_Inner *pInner = 0;

	if (0 == pThis) {
		return -1;
	}

	pInner = (FrontEnd_Inner *) pThis->pInner;

	if (0 == pInner) {
		return -1;
	}

	return 0;
}


/**
 * setup environments from a given configuration file
 */
HCILAB_PRIVATE hci_int32
_FrontEnd_loadConfigurations(PowerASR_FrontEnd *pThis,		///< (i/o) pointer to the ASR front-end engine
							 const char *pszWorkingDir,		///< (i) working directory
							 const char *pszConfigFile)		///< (i) ASR main configuration file
{
	FrontEnd_Inner *pInner = 0;
	char *pszValue = 0;
	char szFileName[512] = { 0 };

	if (0 == pThis) {
		return -1;
	}

	pInner = (FrontEnd_Inner *) pThis->pInner;

	if (0 == pInner) {
		return -1;
	}

	sprintf(szFileName, "%s/%s", pszWorkingDir, pszConfigFile);
	if (0 != PowerASR_Base_parseConfigFile(szFileName)) {
		HCIMSG_ERROR("parseConfigFile failed (%s).\n", szFileName);
		return -1;
	}

	// Wave-to-Mfcc converter config. file
	pszValue = PowerASR_Base_getArgumentValue("WAVE2MFCC_CFG_FILE");
	if (pszValue) {
		sprintf(szFileName, "%s%s", pszWorkingDir, pszValue);
		pInner->pszWave2MfccCfgFile = (char *)hci_salloc(szFileName);
	}
	// Wave-to-Mfcc converter config. file
	pszValue = PowerASR_Base_getArgumentValue("WAVE2MFCC_CFG_FILE_16K");
	if (pszValue) {
		sprintf(szFileName, "%s%s", pszWorkingDir, pszValue);
		pInner->pszWave2MfccCfgFile16k = (char *)hci_salloc(szFileName);
	} else {
		pInner->pszWave2MfccCfgFile16k = NULL;
	}
	// Wave-to-Mfcc converter config. file
	pszValue = PowerASR_Base_getArgumentValue("WAVE2MFCC_CFG_FILE_8K");
	if (pszValue) {
		sprintf(szFileName, "%s%s", pszWorkingDir, pszValue);
		pInner->pszWave2MfccCfgFile8k = (char *)hci_salloc(szFileName);
	} else {
		pInner->pszWave2MfccCfgFile8k = NULL;
	}

	// EPD config. file
	pszValue = PowerASR_Base_getArgumentValue("EPD_CFG_FILE");
	if (pszValue) {
		sprintf(szFileName, "%s%s", pszWorkingDir, pszValue);
		pInner->pszEpdCfgFile = (char *)hci_salloc(szFileName);
	}

	// Mfcc-to-Feat converter config. file
	pszValue = PowerASR_Base_getArgumentValue("MFCC2FEAT_CFG_FILE");
	if (pszValue) {
		sprintf(szFileName, "%s%s", pszWorkingDir, pszValue);
		pInner->pszMfcc2FeatCfgFile = (char *)hci_salloc(szFileName);
	}

	// log-speech-data margin length in msec
	pszValue = PowerASR_Base_getArgumentValue("SPEECH_LOG_MARGIN");
	if (pszValue) {
		pInner->nLenLogSpeechMargin = atoi(pszValue);
		pInner->nLenLogSpeechMargin *= 16;
	}
	else {
		pInner->nLenLogSpeechMargin = 200 * 16;
	}

	PowerASR_Base_closeConfigurations();

	return 0;
}

// end of file
