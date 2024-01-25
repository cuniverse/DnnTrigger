
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
 *	@file	hci_FrontEnd.h
 *	@ingroup frontend_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	Header file to define PowerASR Front-End API functions.
 */

#ifndef __HCILAB_FRONTEND_H__
#define __HCILAB_FRONTEND_H__

#include "common/hci_asr_common.h"

#if defined(HCI_MSC_32)
#ifdef HCI_FRONTEND_EXPORTS
#define HCI_FE_API __declspec(dllexport)
#elif defined(HCI_FRONTEND_IMPORTS)
#define HCI_FE_API __declspec(dllimport)
#else	// in case of static library
#define HCI_FE_API
#endif // #ifdef HCI_FRONTEND_EXPORTS
#elif defined(HCI_OS2)
#define HCI_FE_API
#else
#define HCI_FE_API HCI_USER
#endif

//////////////////////////////////////////////////////////////////////////

/* structures */

/** PowerASR Front-End Interface */
typedef struct {
	void *pInner;
} PowerASR_FrontEnd;

#ifdef __cplusplus
extern "C" {
#endif

//////////////////////////////////////////////////////////////////////////

/* API functions */

/**
 *	Create a new ASR Front-End engine.
 *
 *	@return ASR Front-End engine struct. 실패 시 null 리턴.
 */
HCILAB_PUBLIC HCI_FE_API PowerASR_FrontEnd*
PowerASR_FrontEnd_new();


/**
 *	Delete the ASR front-end engine.
 *
 *	@return none.
 */
HCILAB_PUBLIC HCI_FE_API void
PowerASR_FrontEnd_delete(PowerASR_FrontEnd *pThis	///< (i/o) pointer to the ASR front-end engine
);


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
									 const hci_int32 nSampleRate	///< (i) sampling frequency in Hz
);


/**
 *	Free memories allocated to the ASR front-end engine.
 *
 *	@return return 0 if all memories are freed successfully, otherwise return -1.
 */
HCILAB_PUBLIC HCI_FE_API hci_int32
PowerASR_FrontEnd_closeFrontEndEngine(PowerASR_FrontEnd *pThis	///< (i/o) pointer to the ASR front-end engine
);

HCILAB_PUBLIC HCI_FE_API hci_int32 PowerASR_FrontEnd_releaseFrontEndWiener(FrontEnd_UserData *pThis,PowerASR_FrontEnd *pFronEndThis);//AddByKSH_20150423

/**
 *	Initialize data buffers for front-end modules of ASR engine.
 *
 *	@return return 0 if user-specific front-end data buffers are initialized successfully, otherwise return -1.
 */
/*
HCILAB_PUBLIC HCI_FE_API hci_int32
PowerASR_FrontEnd_initializeFrontEndforASR(PowerASR_FrontEnd *pThis,				///< (i) pointer to the ASR front-end engine
										   FrontEnd_UserData *pChannelDataFE,		///< (o) channel-specific front-end data struct
										   const hci_flag bInitFE,					///< (i) flag to initialize front-end user data
										   const hci_flag bContinuousEPD			///< (i) flag to continuous-mode EPD
);
*/
HCILAB_PUBLIC HCI_FE_API hci_int32
PowerASR_FrontEnd_initializeFrontEndforASR(PowerASR_FrontEnd *pThis,				///< (i) pointer to the ASR front-end engine
										   FrontEnd_UserData *pChannelDataFE,		///< (o) channel-specific front-end data struct
										   const hci_flag bInitFE,					///< (i) flag to initialize front-end user data
										   const hci_flag bContinuousEPD,			///< (i) flag to continuous-mode EPD
										   char *cmsModelName
);

/**
 *	 Release data buffers for front-end modules of ASR engine.
 *
 *	@return return 0 if user-specific front-end data buffers are released successfully, otherwise return -1.
 */
HCILAB_PUBLIC HCI_FE_API hci_int32
PowerASR_FrontEnd_terminateFrontEndforASR(PowerASR_FrontEnd *pThis,				///< (i) pointer to the ASR front-end engine
										  FrontEnd_UserData *pChannelDataFE		///< (o) channel-specific front-end data struct
);

/**
 *	frame-by-frame feature extraction.\n
 *	-# noise reduction (Kalman filtering)
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
												hci_flag bEOS						///< (i) flag to end-of-speech
);

/**
 *	segment long feature stream.\n
 *
 *	@return none.
 */
HCILAB_PUBLIC HCI_FE_API void
PowerASR_FrontEnd_segmentFeatureStream(PowerASR_FrontEnd *pThis,			///< (i) pointer to the ASR front-end engine
									   FrontEnd_UserData *pChannelDataFE,	///< (o) channel-specific front-end data struct
									   const LONG nLenSegmentOverlapFrame	///< (i) count of overlapped frames between segments
);

void PowerASR_FrontEnd_postprocessSpeechFeature(PowerASR_FrontEnd *pThis,			///< (i) pointer to the ASR front-end engine
										   FrontEnd_UserData *pChannelDataFE	///< (o) channel-specific front-end data struct
);


/**
 *	 Get seed feature normalization vector.
 *
 *	@return return 0 if seed feature normalization vectors are copied successfully, otherwise return -1.
 */
HCILAB_PUBLIC HCI_FE_API hci_int32
PowerASR_FrontEnd_getSeedFeatureNormalizer(PowerASR_FrontEnd *pThis,			///< (i) pointer to the ASR front-end engine
										   FEAT_Normalizer *pOutFeatNorm		///< (o) feature normalization vectors
);

/**
*	 Set feature normalization vectors.
*
*	@return return 0 if seed feature normalization vectors are set successfully, otherwise return -1.
*/
/*
HCILAB_PUBLIC HCI_FE_API hci_int32
PowerASR_FrontEnd_setFeatureNormalizer(PowerASR_FrontEnd *pThis,				///< (i) pointer to the ASR front-end engine
									   FrontEnd_UserData *pChannelDataFE,		///< (o) channel-specific front-end data struct
									   const FEAT_Normalizer *pSeedFeatNorm		///< (i) feature normalization vectors
);
*/

HCILAB_PUBLIC HCI_FE_API hci_int32
	PowerASR_FrontEnd_getCMSSeed(int CHID,FEAT_Normalizer *);	///< (i) feature normalization vectors


HCILAB_PUBLIC HCI_FE_API hci_int32
PowerASR_FrontEnd_setFeatureNormalizer(PowerASR_FrontEnd *pThis,				///< (i) pointer to the ASR front-end engine
									   FrontEnd_UserData *pChannelDataFE,		///< (o) channel-specific front-end data struct
									   const FEAT_Normalizer *pSeedFeatNorm,		///< (i) feature normalization vectors
									   char *cmsModelName,const LONG nChannelID
);

HCILAB_PUBLIC HCI_FE_API hci_int32
PowerASR_FrontEnd_setClientFeatureNormalizer(PowerASR_FrontEnd *pThis,				///< (i) pointer to the ASR front-end engine
									   FrontEnd_UserData *pChannelDataFE,		///< (o) channel-specific front-end data struct
									   const FEAT_Normalizer *pSeedFeatNorm,		///< (i) feature normalization vectors
									   char *cmsModelName,const LONG nChannelID  //KSH_20150922
);

/**
 *	 estimate the length of feature frames from speech duration in sec.
 *
 *	@return return the length of feature frames.
 */
HCILAB_PUBLIC HCI_FE_API LONG
PowerASR_FrontEnd_estimateFrameLengthFromSpeechDuration(PowerASR_FrontEnd *pThis,		///< (i) pointer to the ASR front-end engine
														const LONG nDurSpeechSec		///< (i) speech duration in sec
);

/**
 *	get marginal length of log-speech data in sample count.
 *
 *	@return return the marginal length of log-speech data in sample count.
 */
HCILAB_PUBLIC HCI_FE_API hci_int32
PowerASR_FrontEnd_getLogSpeechMargin(PowerASR_FrontEnd *pThis			///< (i) pointer to the ASR front-end engine
);

#ifdef __cplusplus
}
#endif

#endif // #ifndef __HCILAB_FRONTEND_H__

