
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
 *	@file	powerdsr_frontend.c
 *	@ingroup interface_src
 *	@date	2009/01/07
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	HCILAB DSR Front-End API functions.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
#include <windows.h>
#include <winbase.h>
#include <process.h>
#include <direct.h>
#endif
#if defined(LINUX) || defined(UNIX)
#include <pthread.h>
#endif

#include "base/hci_malloc.h"
#include "base/hci_msg.h"

#include "frontend/hci_FrontEnd.h"
#include "frontend/powerdsr_frontend.h"
#include "frontend/wave_format.h"

#ifndef _CHECK_LICENSE
#define	_CHECK_LICENSE
#endif

#ifdef WIN32
#undef _CHECK_LICENSE
#endif

#undef _CHECK_LICENSE

#ifdef _CHECK_LICENSE
#include "license/HCI_License.h"
#endif	// _CHECK_LICENSE

#ifndef WIN32
#define HANDLE		hci_int32
#define DWORD		unsigned long
#endif

PowerASR_FrontEnd* g_DSR_FE = 0;

// jybyeon 16. 01. 12
PowerASR_FrontEnd* g_DSR_FE_8k = 0;

//FrontEnd_UserData* g_chanDSRFE[MAX_DSR_FE_CHANNEL];
//MFCC_POOL* gMfccHist[MAX_DSR_FE_CHANNEL];
//ASR_FEATURE* gASRReature = NULL; //[MAX_DSR_FE_CHANNEL];
//hci_flag g_bOccupiedCh_FE[MAX_DSR_FE_CHANNEL];
//LONG g_nCHNL_FE = MAX_DSR_FE_CHANNEL;

FrontEnd_UserData** g_chanDSRFE = NULL;
MFCC_POOL** gMfccHist = NULL;
ASR_FEATURE* gASRReature = NULL;
hci_flag* g_bOccupiedCh_FE = NULL;
LONG g_nCHNL_FE = 0;

FEAT_Normalizer g_FeatNorm;

#ifdef WIN32
HANDLE g_hThreadMutex_FE = 0;
#endif

#if defined(LINUX) || defined(UNIX)
pthread_mutex_t    g_mutex_FE;
#endif

// local functions
#ifdef __cplusplus
extern "C" {
#endif 

/**
 *	get idle DSR front-end channel index.
 */
static LONG
_PowerDSR_FE_getIdleChannel();

/**
 *	disable the status of DSR front-end channel
 */
static BOOL
_PowerDSR_FE_disableChannel(const LONG nChannelID );			///< (i) channel index

/**
 *	set feature normalization vectors for a new front-end channel.
 */
static BOOL
_PowerDSR_FE_setFeatureNormalizer(const LONG nChannelID, char *cmsModelName);		///< (i) channel index

/**
 *	update feature normalization vectors.
 */
static BOOL
_PowerDSR_FE_updateFeatureNormalizer(const LONG nChannelID );		///< (i) channel index

/**
 *	initialize front-end modules
 */
HCILAB_PRIVATE VOID
_PowerDSR_FE_initializeFrontEnd(LONG nChanID);

#ifdef __cplusplus
}
#endif


/**
 *	Create a new DSR Front-End engine,\n and set-up environments for DSR front-end modules.
 *
 *	@return return POWERDSR_FE_CONNECTED if DSR front-end engine was connected successfully, otherwise return POWERDSR_FE_FAILED.
 */
HCILAB_PUBLIC POWERDSR_FE_API LONG
PowerDSR_FE_Connect(const CHAR* pszASRPath,				///< (i) ASR main path
					const CHAR* pszConfigFile,			///< (i) main ASR configuration file
					int nMaxChannelCount				///< (i) Max Channel Count
					)
{
	hci_int32	nResult = 0;
	hci_int32	iChan = 0;
	int channelID = 0;


	if (0 == pszConfigFile || 0 == pszASRPath) {
		return POWERDSR_FE_NO_CFG_FILE;
	}
	if (nMaxChannelCount < 0) {
		return POWERDSR_FE_FAILED;
	}

	g_nCHNL_FE = nMaxChannelCount;

	/*
	FrontEnd_UserData** g_chanDSRFE = NULL;
	MFCC_POOL** gMfccHist = NULL;
	ASR_FEATURE* gASRReature = NULL;
	hci_flag* g_bOccupiedCh_FE;
	LONG g_nCHNL_FE = 0;

	
	*/
	// Create FrontEnd_UserData
	g_chanDSRFE = (FrontEnd_UserData**)calloc(g_nCHNL_FE, sizeof(FrontEnd_UserData*));
	if (g_chanDSRFE == NULL) {
		PowerDSR_FE_Disconnect();
		return POWERDSR_FE_FAILED;
	}
	gMfccHist = (MFCC_POOL**)calloc(g_nCHNL_FE, sizeof(MFCC_POOL*));
	if (gMfccHist == NULL) {
		PowerDSR_FE_Disconnect();
		return POWERDSR_FE_FAILED;
	}
	g_bOccupiedCh_FE = (hci_flag*)calloc(g_nCHNL_FE, sizeof(hci_flag));
	if (g_bOccupiedCh_FE == NULL) {
		PowerDSR_FE_Disconnect();
		return POWERDSR_FE_FAILED;
	}

	// Create MFCCPool Alloc
	for (channelID = 0; channelID < g_nCHNL_FE; channelID++) {
		gMfccHist[channelID] = (MFCC_POOL*)calloc(MAX_LEN_FEAT_FRAME, sizeof(MFCC_POOL));
	}

	gASRReature = (ASR_FEATURE*)calloc(g_nCHNL_FE, sizeof(ASR_FEATURE));
	if (gASRReature == NULL) {
		return POWERDSR_FE_FAILED;
	}
	
	// create/open ASR front-end engine
	if (0 == g_DSR_FE) {
		g_DSR_FE = PowerASR_FrontEnd_new();
		if (0 == g_DSR_FE) {
			return POWERDSR_FE_FAILED;
		}
		
		nResult = PowerASR_FrontEnd_openFrontEndEngine(g_DSR_FE, pszASRPath, pszConfigFile, 16000);
		if (0 != nResult) {
			PowerASR_FrontEnd_closeFrontEndEngine(g_DSR_FE);
			PowerASR_FrontEnd_delete(g_DSR_FE);
			g_DSR_FE = 0;
			return POWERDSR_FE_FAILED;
		}	
	}

	if (0 == g_DSR_FE_8k) {
		g_DSR_FE_8k = PowerASR_FrontEnd_new();
		if (0 == g_DSR_FE_8k) {
			return POWERDSR_FE_FAILED;
		}
		
		nResult = PowerASR_FrontEnd_openFrontEndEngine(g_DSR_FE_8k, pszASRPath, pszConfigFile, 8000);
		if (0 != nResult) {
			PowerASR_FrontEnd_closeFrontEndEngine(g_DSR_FE_8k);
			PowerASR_FrontEnd_delete(g_DSR_FE_8k);
			g_DSR_FE_8k = 0;
			
			PowerASR_FrontEnd_closeFrontEndEngine(g_DSR_FE);
			PowerASR_FrontEnd_delete(g_DSR_FE);
			g_DSR_FE = 0;

			return POWERDSR_FE_FAILED;
		}	
	}



	// initialize channel status
	for ( iChan = 0; iChan < g_nCHNL_FE ; iChan++ ) {
		g_chanDSRFE[iChan] = 0;
	}

	// create mutex handle to manage processing thread allocation
#ifdef WIN32
	g_hThreadMutex_FE = CreateMutex( NULL, FALSE, NULL );
	if (g_hThreadMutex_FE == NULL) 
	{
		fprintf(stderr, "[ERROR] CreateMutex error: %d\n", GetLastError() );
		return POWERDSR_FE_FAILED;
	}
#endif	// #ifdef WIN32

#if defined(LINUX) || defined(UNIX)
	if( pthread_mutex_init( &g_mutex_FE, NULL ) != 0 ){
		fprintf(stderr, "[ERROR] mutex_init error:\n" );
		return POWERDSR_FE_FAILED;
	}
#endif	// #if defined(LINUX) || defined(UNIX)
	
	// initialize feature normalization vectors for continuous feature normalization
	memset(&g_FeatNorm, 0, sizeof(g_FeatNorm));
	PowerASR_FrontEnd_getSeedFeatureNormalizer( g_DSR_FE, &g_FeatNorm );
	PowerASR_FrontEnd_getSeedFeatureNormalizer( g_DSR_FE_8k, &g_FeatNorm );

	return POWERDSR_FE_CONNECTED;
}


/**
 *	Destroy the DSR Front-End engine.
 *
 *	@return none.
 */
HCILAB_PUBLIC POWERDSR_FE_API VOID
PowerDSR_FE_Disconnect()
{
	LONG iChan = 0;
	int channelID = 0;

	for ( iChan = 0; iChan < g_nCHNL_FE ; iChan++ ) {
		if ( g_bOccupiedCh_FE[iChan] || g_chanDSRFE[iChan] ) {
			PowerASR_FrontEnd_releaseFrontEndWiener(g_chanDSRFE[iChan],g_DSR_FE);
			PowerDSR_FE_CloseChannel( iChan );
		}
	}
	
	if (g_DSR_FE) {
		PowerASR_FrontEnd_closeFrontEndEngine(g_DSR_FE);
		PowerASR_FrontEnd_delete(g_DSR_FE);
		g_DSR_FE = 0;
	}

	if (g_DSR_FE_8k) {
		PowerASR_FrontEnd_closeFrontEndEngine(g_DSR_FE_8k);
		PowerASR_FrontEnd_delete(g_DSR_FE_8k);
		g_DSR_FE_8k = 0;
	}
	
	for (channelID = 0; channelID < g_nCHNL_FE; channelID++) {
		if (gMfccHist[channelID] != NULL) {
			free(gMfccHist[channelID]);
			gMfccHist[channelID] = NULL;
		}
	}
	 
	if (gASRReature != NULL) {
		free(gASRReature);
		gASRReature = NULL;
	}

	if (g_bOccupiedCh_FE != NULL) {
		free(g_bOccupiedCh_FE);
		g_bOccupiedCh_FE = NULL;
	}

	
	if (gMfccHist != NULL) {
		free(gMfccHist);
		gMfccHist = NULL;
	}

	if (g_chanDSRFE != NULL) {
		free(g_chanDSRFE);
		g_chanDSRFE = NULL;
	}

	g_nCHNL_FE = 0;

#ifdef WIN32
	if ( g_hThreadMutex_FE ) { CloseHandle(g_hThreadMutex_FE); g_hThreadMutex_FE = 0;  }
#endif	// #ifdef WIN32

#if defined(LINUX) || defined(UNIX)
	pthread_mutex_destroy(&g_mutex_FE);
#endif	// #if defined(LINUX) || defined(UNIX)

}


/**
 *	Open a new DSR front-end channel.
 *	Use 16K and 8K for Feature Extractor  jybyeon 16. 01. 12
 *	@return return idle DSR front-end channel index. If all DSR front-end channels are busy, return -1.
 */
HCILAB_PUBLIC POWERDSR_FE_API LONG
PowerDSR_FE_OpenChannel(char *cmsModelFName, int bUse16k) {
	PowerASR_FrontEnd* pGlobalData = NULL;
	FrontEnd_UserData*	pChanData = 0;
	DSR_FX_DATA*		pFX = 0;
	hci_int32 			nSampleRate = 16000;
	LONG 				nChannelID = 0L;
	LONG				ret = -1L;

	if (bUse16k == TRUE) {
		pGlobalData = g_DSR_FE;
		nSampleRate = 16000;
	} else {
		pGlobalData = g_DSR_FE_8k;
		nSampleRate = 8000;
	}

	if ( 0 == pGlobalData ) return ret;

	nChannelID = _PowerDSR_FE_getIdleChannel();
	if ( nChannelID < 0L || nChannelID >= g_nCHNL_FE ) {
		return ret;
	}
	if ( g_chanDSRFE[nChannelID] ) {
		_PowerDSR_FE_disableChannel( nChannelID );
		return ret;
	}

	pChanData = (FrontEnd_UserData *) hci_calloc( 1, sizeof(FrontEnd_UserData) );
	if (0 == pChanData) {
		_PowerDSR_FE_disableChannel( nChannelID );
		return ret;
	}
	pChanData->nSampleRate = nSampleRate;
	pChanData->dataASRFeat.mfccHist = gMfccHist[nChannelID];
	pChanData->dataASRFeat.featStream = &gASRReature[nChannelID];
	g_chanDSRFE[nChannelID] = pChanData;

	PowerASR_FrontEnd_initializeFrontEndforASR(pGlobalData, pChanData, TRUE, TRUE, cmsModelFName);//KSH

	pChanData->st = NULL;
#ifdef __AUDIORECOG_API__
	// DNN EPD init
	if (bUse16k) {
		pChanData->st = DioAudioRecogCreate(NULL);	// conf/dioar_cfg.txt
	}
	else {
		pChanData->st = DioAudioRecogCreate("conf/dioar_8k.ini");
	}

	if (!pChanData->st) {
		HCIMSG_ERROR("[Error] cannot create DNN End point detector !!\n");
		return -1;
	}
#endif

	_PowerDSR_FE_initializeFrontEnd( nChannelID );
	_PowerDSR_FE_setFeatureNormalizer( nChannelID, cmsModelFName);//KSH
	PowerASR_SPEEX_initializeWBDecoding( &pChanData->dataSpeex,  nSampleRate); // Speex Initial


	// jybyeon // 
	//pFX = &pChanData->dataFX;
	//pFX->nLenTimeOutTh    = 100 * nSampleRate;
	//pFX->nLenSpeechTh     = 100 * nSampleRate;
	//pFX->nFeatDim         = pChanData->dataASRFeat.featStream.dimFeat;
	//pFX->nFrameSampleSize = nSampleRate/100;//KSH_20151218 : nSampleRate에 따라 변화
	//pFX->nFEStatus        = OPEN_BE_CHANNEL;

	return nChannelID;
}


//
///**
// *	Open a new DSR front-end channel.
// *
// *	@return return idle DSR front-end channel index. If all DSR front-end channels are busy, return -1.
// */
//HCILAB_PUBLIC POWERDSR_FE_API LONG
//PowerDSR_FE_OpenChannel(char *cmsModelFName)
//{
//	FrontEnd_UserData*	pChanData = 0;
//	DSR_FX_DATA*		pFX = 0;
//	hci_int32 			nSampleRate = 16000;
//	LONG 				nChannelID = 0L;
//	LONG				ret = -1L;
//
//	if ( 0 == g_DSR_FE ) return ret;
//
//	nChannelID = _PowerDSR_FE_getIdleChannel();
//	if ( nChannelID < 0L || nChannelID >= g_nCHNL_FE ) {
//		return ret;
//	}
//	if ( g_chanDSRFE[nChannelID] ) {
//		_PowerDSR_FE_disableChannel( nChannelID );
//		return ret;
//	}
//
//	pChanData = (FrontEnd_UserData *) hci_calloc( 1, sizeof(FrontEnd_UserData) );
//	if (0 == pChanData) {
//		_PowerDSR_FE_disableChannel( nChannelID );
//		return ret;
//	}
//	memset( pChanData, 0, sizeof(FrontEnd_UserData) );
//
//	g_chanDSRFE[nChannelID] = pChanData;
//
////	PowerASR_FrontEnd_initializeFrontEndforASR(g_DSR_FE, pChanData, TRUE, TRUE);
//	PowerASR_FrontEnd_initializeFrontEndforASR(g_DSR_FE, pChanData, TRUE, TRUE, cmsModelFName);//KSH
//
//	// kklee 20150922
//	if (pChanData->st == NULL)
//	{
//		pChanData->st = DioAudioRecogCreate(NULL);
//		if (0 == pChanData->st){
//			HCIMSG_ERROR("[Warning] cannot create DNN End point detector !!\n");
//			return -1;
//		}
//	}
//
//	pFX = &pChanData->dataFX;
//	pFX->nLenTimeOutTh    = 100 * nSampleRate;
//	pFX->nLenSpeechTh     = 100 * nSampleRate;
//	pFX->nFeatDim         = pChanData->dataASRFeat.featStream.dimFeat;
//	pFX->nFrameSampleSize = 160;
//	pFX->nFEStatus        = OPEN_BE_CHANNEL;
//
//	return nChannelID;
//}


/**
 *	Close an DSR front-end channel.
 *
 *	@return none.
 */
HCILAB_PUBLIC POWERDSR_FE_API VOID
PowerDSR_FE_CloseChannel(const LONG nChannelID)				///< (i) DSR front-end channel index
{
	PowerASR_FrontEnd *g_FE = 0;
	FrontEnd_UserData*	pChannelData = 0;
	DSR_FX_DATA*		pFX = 0;

	if ( 0 == g_DSR_FE ) return;
	if ( nChannelID < 0L || nChannelID >= g_nCHNL_FE ) return;

	pChannelData = g_chanDSRFE[nChannelID];

	if ( !pChannelData ) return;

	pFX = &pChannelData->dataFX;
//	if ( pFX->nFEStatus == CLOSE_BE_CHANNEL ) return;
	pFX->bExit     = TRUE;
	pFX->nFEStatus = CLOSE_BE_CHANNEL;

#ifdef __AUDIORECOG_API__
	// kklee 20150922
	if (pChannelData->st) {
		DioAudioRecogDestroy(pChannelData->st);
		pChannelData->st = 0;
	}
#endif
	if (pChannelData->nSampleRate == 16000) {
		g_FE = g_DSR_FE;
	} else {
		g_FE = g_DSR_FE_8k;
	}
	PowerASR_SPEEX_releaseWBDecoding( &pChannelData->dataSpeex );

	PowerASR_FrontEnd_terminateFrontEndforASR(g_FE, pChannelData);

	_PowerDSR_FE_disableChannel( nChannelID );

}


/**
 *	Initialize a DSR front-end engine.
 *
 *	- Initialize DSR front-end engine.
 *
 *	@return return 0 if DSR front-end engine was initialized successfully, otherwise return -1.
 */
HCILAB_PUBLIC POWERDSR_FE_API LONG
//PowerDSR_FE_InitializeFrontEndEngine(const LONG nChannelID)			///< (i) DSR front-end channel index
PowerDSR_FE_InitializeFrontEndEngine(const LONG nChannelID,char *cmsModelName)//KSH			///< (i) DSR front-end channel index
{
//	FrontEnd_UserData*	pChannelData = 0;
//	DSR_FX_DATA*		pFX = 0;
//	DWORD				dwWaitResult = 0;
//	hci_flag			bInitFE = FALSE;
//
//	if ( 0 == g_DSR_FE ) return -1L;
//	if ( nChannelID < 0L || nChannelID >= g_nCHNL_FE ) return -1L;
//
//	pChannelData = g_chanDSRFE[nChannelID];
//	if (0 == pChannelData) return -1L;
//	
//	pFX = &pChannelData->dataFX;
//	pFX->frameCnt = 0;
//
//
//	// check current Front-End status
////	if ( pFX->nFEStatus != OPEN_BE_CHANNEL && pFX->nFEStatus != END_BE_STATE ) {
////		return -1L;
////	}
//	
//	// initialize front-end modules
////	PowerASR_FrontEnd_initializeFrontEndforASR(g_DSR_FE, pChannelData, FALSE, TRUE );
//
//
//	PowerASR_FrontEnd_initializeFrontEndforASR(g_DSR_FE, pChannelData, FALSE, TRUE,cmsModelName);//KSH
////	_PowerDSR_FE_initializeFrontEnd( nChannelID );
////
////	pFX->nFEStatus       = INIT_BE_STATE;
////	pFX->nLenProcessWave = 0;
////	pFX->nLenSendFrame   = 0;
////	pFX->bEOS            = FALSE;
////	pFX->bFXComplete     = FALSE;
////	pFX->bUnusualSpeech  = FALSE;
////	pFX->bExit           = FALSE;
////	pFX->bReceiveRefSilFeat = 0;
////	pFX->nRefSilFeatSize = 0;
////
//////	_PowerDSR_FE_setFeatureNormalizer( nChannelID );
////	_PowerDSR_FE_setFeatureNormalizer( nChannelID ,cmsModelName);//KSH
////
////
////	PowerASR_SPEEX_initializeWBDecoding( &pChannelData->dataSpeex ); // Speex Initial

	return 0L;
}


/**
 *	Release a DSR front-end engine.
 *
 *	- Release DSR front-end engine.
 *
 *	@return none.
 */
HCILAB_PUBLIC POWERDSR_FE_API VOID
PowerDSR_FE_ReleaseFrontEndEngine(const LONG nChannelID)			///< (i) DSR front-end channel index
{
	FrontEnd_UserData*	pChannelData = 0;
	DSR_FX_DATA*		pFX = 0;
	PowerASR_FrontEnd* g_FE = NULL;
	
	if ( 0 == g_DSR_FE ) return;
	if ( nChannelID < 0L || nChannelID >= g_nCHNL_FE ) return;

	pChannelData = g_chanDSRFE[nChannelID];
	
	if (0 == pChannelData) return;
	if (pChannelData->nSampleRate == 16000) {
		g_FE = g_DSR_FE;
	} else {
		g_FE = g_DSR_FE_8k;
	}


	pFX = &pChannelData->dataFX;
	
	PowerASR_SPEEX_releaseWBDecoding( &pChannelData->dataSpeex );

	// check current Front-End status
// 	if ( pFX->nFEStatus != INIT_BE_STATE && pFX->nFEStatus != MAIN_BE_STATE && pFX->nFEStatus != RETURN_BE_RESULT ) {
// 		return;
// 	}


	// Wiener Release // 15.12.03 JYBYEON Mem Leck Check
	PowerASR_FrontEnd_releaseFrontEndWiener(g_chanDSRFE[nChannelID], g_FE);


	pFX->nFEStatus = END_BE_STATE;
	pFX->bExit     = TRUE;
	PowerASR_FrontEnd_terminateFrontEndforASR(g_FE, pChannelData);

}


/**
 *	Convert encoded speech stream into feature stream.
 *
 *	@return Return values:
 *		- RECEIV_OK : normal status
 *		- TIME_OVER : time-over
 *		- EPD_FOUND : end-point detected
 *		- FX_FAILED : invalid front-end status
 */
HCILAB_PUBLIC POWERDSR_FE_API LONG
PowerDSR_FE_EncodedSpeechStream2FeatureStream(const LONG nChannelID,		///< (i) DSR front-end channel index
											  hci_float32* pOutFeat,				///< (o) new feature stream data
											  LONG* nSizeFeat,				///< (o) size of feature stream data in bytes
											  CHAR* encData,				///< (i) speex encoded speech stream
											  LONG nDataSize,				///< (i) size of encoded speech stream in bytes
											  LONG nMaxFeatFrame,			///< (i) maximum size of feature stream in frame count
											  const LONG bEOS)				///< (i) flag to end-of-speech
{
	FrontEnd_UserData*	pChannelData = 0;
	SPEEX_DEC_DATA*		pSpeexData = 0;
	DSR_FX_DATA*		pFX = 0;
	EPD_UserData*		pEpdData = 0;
	Feature_UserData*	pASRFeat = 0;


	
	
	
	

	if ( 0 == g_DSR_FE ) return FX_FAILED;
	if ( nChannelID < 0L || nChannelID >= g_nCHNL_FE ) return FX_FAILED;

	pChannelData = g_chanDSRFE[nChannelID];
	if (0 == pChannelData) return FX_FAILED;

	pFX        = &pChannelData->dataFX;
	pSpeexData = &pChannelData->dataSpeex;
	pEpdData   = &pChannelData->dataEpd;
	pASRFeat   = &pChannelData->dataASRFeat;

	if ( pFX->bEOS || pFX->bFXComplete ) {
		*nSizeFeat = 0;
		pFX->bFXComplete = 0;
		if (pFX->bEOS < 0) pFX->bEOS = 0;
		//return EPD_RESET;
	}	else {
		if ( pFX->bTimeOut ) {
			*nSizeFeat = 0;
			return TIME_OVER;
		}
		else if ( pFX->bUnusualSpeech ) {
			*nSizeFeat = 0;
			return EPD_FOUND;
		}
	}

	if ( encData && nDataSize > 0L ) {
		PowerASR_SPEEX_doWBDecoding( pSpeexData, encData, nDataSize );
	}
	
	if ( bEOS && pFX->bEOS == FALSE ) {
		pFX->bEOS = TRUE;
	}
	if(bEOS==1 && pFX->nEndPoint==0){
		pEpdData->nEndFrame = pEpdData->nEpdFrame;

		pFX->nEndPoint       = pSpeexData->len_rec_wave;
	}
	if (bEOS==1 && pFX->bDetectEndPoint == FALSE) {
		pEpdData->nEndFrame = pEpdData->nEpdFrame;
		pFX->nEndPoint       = pSpeexData->len_rec_wave;
	}
	// if EOS decode last packet
	if (pFX->bEOS) {
		PowerASR_SPEEX_completeWBDecoding(pSpeexData);
	}
	

	return SpeechStream2FeatureStream(nChannelID, pOutFeat, nSizeFeat, nMaxFeatFrame, bEOS);
}


//check point
HCILAB_PUBLIC POWERDSR_FE_API LONG
PowerDSR_FE_SpeechStream2FeatureStream(const LONG nChannelID,		///< (i) DSR front-end channel index
											  hci_float32* pOutFeat,				///< (o) new feature stream data
											  LONG* nSizeFeat,				///< (o) size of feature stream data in bytes
											  short * waveData,				///< (i) PCM speech stream
											  LONG nDataSize,				///< (i) size of PCM speech stream in bytes
											  LONG nMaxFeatFrame,			///< (i) maximum size of feature stream in frame count
											  const LONG bEOS)				///< (i) flag to end-of-speech
{
	FrontEnd_UserData*	pChannelData = 0;
	SPEEX_DEC_DATA*		pSpeexData = 0;
	DSR_FX_DATA*		pFX = 0;
	EPD_UserData*		pEpdData = 0;
	Feature_UserData*	pASRFeat = 0;
	hci_int16			frameSample[160];
	M2F_Status			m2f_state = M2F_FALSE;
	LONG				nStartFrame = 0L;
	hci_int16			nCurrentEpdState = SIL_SPEECH;
	hci_flag			bResetStartPt = FALSE;

	if ( 0 == g_DSR_FE ) return FX_FAILED;
	if ( nChannelID < 0L || nChannelID >= g_nCHNL_FE ) return FX_FAILED;

	pChannelData = g_chanDSRFE[nChannelID];
	if (0 == pChannelData) return FX_FAILED;

	pFX        = &pChannelData->dataFX;
	pSpeexData = &pChannelData->dataSpeex;
	pEpdData   = &pChannelData->dataEpd;
	pASRFeat   = &pChannelData->dataASRFeat;

	if ( pFX->bEOS || pFX->bFXComplete ) {
		*nSizeFeat = 0;
		pFX->bFXComplete = 0;
		if (pFX->bEOS < 0) pFX->bEOS = 0;

	} else {
		if ( pFX->bTimeOut ) {
			*nSizeFeat = 0;
			return TIME_OVER;
		} else if ( pFX->bUnusualSpeech ) {
			*nSizeFeat = 0;
			return EPD_FOUND;
		}
	}
	
	if ( bEOS && pFX->bEOS == FALSE ) {
		pFX->bEOS = TRUE;
	}
	if(bEOS==1 && pFX->nEndPoint==0){	
		pEpdData->nEndFrame = pEpdData->nEpdFrame;
		pFX->nEndPoint       = pSpeexData->len_rec_wave;
	}
	if (bEOS==1 && pFX->bDetectEndPoint == FALSE) {
		pEpdData->nEndFrame = pEpdData->nEpdFrame;
		pFX->nEndPoint       = pSpeexData->len_rec_wave;
	}

	if ( nDataSize > 0 ) {
		if ( pSpeexData->len_rec_wave ) {
			pSpeexData->rec_wave = (short *) realloc( pSpeexData->rec_wave, 
								(pSpeexData->len_rec_wave + nDataSize) * sizeof(short) );
		}
		else  {
			//pSpeexData->rec_wave = (short *)calloc(nDataSize, sizeof(short));
			if (!pSpeexData->rec_wave)//yowon 2016-07-25
				pSpeexData->rec_wave = (short *) calloc( nDataSize, sizeof(short) );//yowon 2016-07-25
		}
		memcpy( pSpeexData->rec_wave + pSpeexData->len_rec_wave, waveData, nDataSize * sizeof(short) );	
		pSpeexData->len_rec_wave += nDataSize;
	}



	//pSpeexData->len_rec_wave=nDataSize;
	//pFX->nLenProcessWave=0;
	//pSpeexData->rec_wave=(short *)malloc(sizeof(short)*nDataSize);
	//memcpy(pSpeexData->rec_wave,waveData,sizeof(short)*nDataSize);


	return SpeechStream2FeatureStream(nChannelID, pOutFeat, nSizeFeat, nMaxFeatFrame, bEOS);
}


void ResetEpdUserData(EPD_UserData* pEpdData) {
	pEpdData->nCurrentState = SIL_SPEECH;
	pEpdData->nStartFrame    = pEpdData->nEndFrame  = pEpdData->nSpeechDur   = 0;
	pEpdData->nNumVoiceFrame = pEpdData->nLenSpeech = pEpdData->nLenSilence  = 0;
	pEpdData->speechEn       = pEpdData->noiseEn    = pEpdData->nCountVoiced = 0;
	pEpdData->bSpeechStarted = FALSE;
	pEpdData->bResetStartPt = TRUE;
}

HCILAB_PUBLIC POWERDSR_FE_API LONG SpeechStream2FeatureStream(	const LONG nChannelID,
																hci_float32* pOutFeat,				///< (o) new feature stream data
																LONG* nSizeFeat,					///< (o) size of feature stream data in bytes
																LONG nMaxFeatFrame,					///< (i) maximum size of feature stream in frame count
																const LONG bEOS) {

	FrontEnd_UserData* pChannelData = g_chanDSRFE[nChannelID];
	PowerASR_FrontEnd *Global_DSR_FE = 0;
	

	SPEEX_DEC_DATA*		pSpeexData = &pChannelData->dataSpeex;
	DSR_FX_DATA*		pFX = &pChannelData->dataFX;
	EPD_UserData*		pEpdData = &pChannelData->dataEpd;
	Feature_UserData*	pASRFeat = &pChannelData->dataASRFeat;

	hci_int16			frameSample[160];
	M2F_Status			m2f_state = M2F_FALSE;
	LONG				nStartFrame = 0L;
	hci_int16			nCurrentEpdState = SIL_SPEECH;
	hci_flag			bResetStartPt = FALSE;
	hci_flag			bEpdReset = FALSE;
	hci_flag bEosLastFrame = FALSE;


	if (pChannelData->nSampleRate == 16000) {
		Global_DSR_FE = g_DSR_FE;
	} else {
		Global_DSR_FE = g_DSR_FE_8k;
	}

	

	// frame-by-frame feature extraction
	while (!pFX->bExit && !pFX->bFXComplete && !pFX->bUnusualSpeech
			&& ((pFX->nLenProcessWave + pFX->nFrameSampleSize) <= pSpeexData->len_rec_wave) )
	{
		memcpy(frameSample, pSpeexData->rec_wave + pFX->nLenProcessWave, pFX->nFrameSampleSize * sizeof(short));
		pFX->frameCnt++;

		//if(pFX->frameCnt < 8) // 2차 SQA 버전  // BGY Skip Frame
		if (pFX->frameCnt < pEpdData->nFrontSkipFrameSize) {	//노트4 대응 버전
			pFX->nLenProcessWave += pFX->nFrameSampleSize;
			continue;//KSH_20150515
		}

		if (pFX->bEOS && ((pFX->nLenProcessWave + pFX->nFrameSampleSize) <= pSpeexData->len_rec_wave))
			bEosLastFrame = TRUE;
		m2f_state = PowerASR_FrontEnd_framebyframeFeatureExtraction(Global_DSR_FE, pChannelData, frameSample, bEosLastFrame);

		if (pASRFeat->lenFeatStream >= MAX_DEC_FRAME) {
			pFX->bTooLongSpeech  = TRUE;
			pFX->bUnusualSpeech  = TRUE;
			pFX->bDetectEndPoint = TRUE;
			pFX->bFXComplete     = TRUE;
			pFX->nEndPoint       = pSpeexData->len_rec_wave - 320;
			pFX->nStartPoint     = HCI_MAX(0, pEpdData->nStartFrame * pFX->nFrameSampleSize);
			pASRFeat->lenFeatStream = MAX_DEC_FRAME - 1;

			ResetEpdUserData(pEpdData);
#ifdef __AUDIORECOG_API__
			pChannelData->st->range.stat = 4;
#endif
		}
		else if (pEpdData->bDetectStartPt) {	// speech start-point was detected
			pFX->nStartPoint     = HCI_MAX(0, pEpdData->nStartFrame * pFX->nFrameSampleSize);
			pFX->bSpeechPeriod   = TRUE;
			bResetStartPt        = TRUE;
		}
		else if (m2f_state == M2F_COMPLETE) {	// feature extraction was completed
			pFX->bDetectEndPoint = TRUE;
			pFX->bFXComplete     = TRUE;
			if (bEosLastFrame == TRUE) {
				pEpdData->nEndFrame = pEpdData->nEpdFrame;
			}
			pFX->nEndPoint       = HCI_MIN(pSpeexData->len_rec_wave, pEpdData->nEndFrame * pFX->nFrameSampleSize);
			pFX->nStartPoint     = HCI_MAX(0, pEpdData->nStartFrame * pFX->nFrameSampleSize);

			ResetEpdUserData(pEpdData);
#ifdef __AUDIORECOG_API__
			pChannelData->st->range.stat = 4;
#endif
		}
		else if (pEpdData->bResetStartPt && pFX->bSpeechPeriod) {	// reset EPD status (previous start-point was canceled)
			pFX->bDetectStartPoint = FALSE;
			pFX->bSpeechPeriod     = FALSE;
			pFX->nStartPoint       = 0;
			bResetStartPt          = TRUE;
			bEpdReset = TRUE;
		}
#if 0 // ywjung 20160708
		if (!pFX->bFXComplete || !pFX->bUnusualSpeech) {
			if (m2f_state==M2F_FAIL	// check invalid speech input (too-long speech)
					|| (pFX->bSpeechPeriod && (pFX->nLenProcessWave - pFX->nStartPoint) > pFX->nLenSpeechTh))	// check too-long speech
			{
				pFX->bTooLongSpeech  = TRUE;
				pFX->bUnusualSpeech  = TRUE;
				pFX->bDetectEndPoint = TRUE;
				pFX->bFXComplete     = TRUE;
				pFX->nEndPoint       = pSpeexData->len_rec_wave - 320;
				pFX->nStartPoint     = HCI_MAX(0, pEpdData->nStartFrame * pFX->nFrameSampleSize);
			}

			// check time-out
			if (pFX->nLenProcessWave > pFX->nLenTimeOutTh && pFX->bSpeechPeriod == FALSE) {
				pFX->bTimeOut       = TRUE;
				pFX->bUnusualSpeech = TRUE;
			}
		}
#endif
		pFX->nLenProcessWave += pFX->nFrameSampleSize;
	}

	// EOS but not complete
	if (pFX->bEOS && !pFX->bExit && !pFX->bFXComplete && !pFX->bUnusualSpeech) {
		if (pFX->bSpeechPeriod) {
			m2f_state = PowerASR_FrontEnd_framebyframeFeatureExtraction(Global_DSR_FE, pChannelData, 0, TRUE);

			pFX->bDetectEndPoint = TRUE;
			pFX->bFXComplete = TRUE;
			if (pEpdData->nEndFrame) {
				pFX->nEndPoint = HCI_MIN(pSpeexData->len_rec_wave, pEpdData->nEndFrame * pFX->nFrameSampleSize);
			}
			else {
				pFX->nEndPoint = pSpeexData->len_rec_wave;
			}
			pFX->nStartPoint = HCI_MAX(0, pEpdData->nStartFrame * pFX->nFrameSampleSize);
			pASRFeat->lenFeatStream = HCI_MIN(pASRFeat->lenFeatStream, MAX_DEC_FRAME - 1);
		}
		else {
			pFX->bUnusualSpeech = TRUE;
		}
		ResetEpdUserData(pEpdData);
#ifdef __AUDIORECOG_API__
		pChannelData->st->range.stat = 4;
#endif
	}

	if ( pFX->bFXComplete && !pFX->bUnusualSpeech )
	{
		_PowerDSR_FE_updateFeatureNormalizer( nChannelID );
		pChannelData->nCountUtter += 1;
	}

	*nSizeFeat = 0;
	if ( pFX->bFXComplete ) {
		pOutFeat[0] = 1;
		*nSizeFeat  = 2;

	}
	else {
		pOutFeat[0] = 0;
	}

	if ( bResetStartPt ) {
		*nSizeFeat  = 2;
		pOutFeat[1] = 1;
		pFX->nLenSendFrame = 0;
		bEpdReset = TRUE;
	}
	else {
		pOutFeat[1] = 0;
	}

	if(bEpdReset)//KSH_20141208
	{
		*nSizeFeat  = 2;
		pOutFeat[1] = EPD_RESET;
		pFX->nLenSendFrame = 0;
		pFX->bReceiveRefSilFeat = 0;
	}
	else//KSH_20141208
	{
		bEpdReset = FALSE;
	}

#define LEN_REF_SIL 20	// num of reference silence frame

	// copy features to send
//	if ( pASRFeat->lenFeatStream > pFX->nLenSendFrame ) {
	if (pASRFeat->lenFeatStream > pFX->nLenSendFrame || pASRFeat->feat_count > 0) {//yowon 2016-07-25
		const size_t featBytes = pFX->nFeatDim * sizeof(hci_asr_feat_t);
//		const size_t nCopyFrame = HCI_MIN(pASRFeat->lenFeatStream - pFX->nLenSendFrame, nMaxFeatFrame);//yowon 2016-07-25
		size_t nCopyFrame = 1;//yowon 2016-07-25
		*nSizeFeat  = 2;

		if (pFX->bReceiveRefSilFeat == 0 && pChannelData->refSilFeat != NULL) {
			memcpy(&pOutFeat[*nSizeFeat], pChannelData->refSilFeat, LEN_REF_SIL * featBytes);
			*nSizeFeat += LEN_REF_SIL * pFX->nFeatDim;
			pFX->nRefSilFeatSize = LEN_REF_SIL;
		}
		pFX->bReceiveRefSilFeat = 1;

		pASRFeat->feat_count = 1;//yowon 2016-07-25

		memcpy(&pOutFeat[*nSizeFeat], pASRFeat->utterStream.sent_feat + pFX->nLenSendFrame * pFX->nFeatDim, nCopyFrame * featBytes);//for dnn yowon 2015-04-17

		/////////////////// speex data 초기화 yowon 2016-07-25 ////////////////////// 
		pSpeexData->len_rec_wave = 0;
		pFX->nLenProcessWave = 0;
		/////////////////// speex data 초기화 yowon 2016-07-25 ////////////////////// 

	/*    {
			int i;
			 FILE *fd = NULL;
			char temp[MAX_PATH] = {0,};
			sprintf(temp, "./LOG/OutFeat.txt");
			fd = fopen(temp, "a");
			for (i = 0; i < nCopyFrame; i++)
			{
				int j;
				for (j = 0; j < pFX->nFeatDim; j++) fprintf(fd, "%0.3f ", pOutFeat[i*pFX->nFeatDim + j + *nSizeFeat]);
				fprintf(fd, "\n");
			}
			fclose(fd);
		}*/

		//FILE* feat_binf = fopen("feat_binf.mfc", "ab");
		//fwrite(&pOutFeat[*nSizeFeat], nCopyFrame * pFX->nFeatDim, sizeof(pOutFeat[0]), feat_binf);
		//fclose(feat_binf);

		*nSizeFeat += nCopyFrame * pFX->nFeatDim;
//		pFX->nLenSendFrame += nCopyFrame;
		pFX->nLenSendFrame += nCopyFrame;
		pFX->nLenSendFrame = pFX->nLenSendFrame%MAX_LEN_FEAT_FRAME;//yowon 2016-07-25


	}

	if (!pFX->bEOS && !pFX->bDetectEndPoint && !pFX->bUnusualSpeech && !pFX->bTooLongSpeech && !pFX->bTimeOut) {
		if (pFX->bSpeechPeriod == TRUE) {
			pFX->nLenTimeOutTh = (100 * pChannelData->nSampleRate) + pFX->nLenProcessWave;
			return SPD_FOUND;
		}
		return RECEIV_OK;
	}

	if (pFX->bSpeechPeriod == FALSE) {
		pEpdData->nStartFrame = -1;
		pEpdData->nEndFrame = -1;
	}

	pFX->bSpeechPeriod = FALSE;

	pFX->nLenTimeOutTh = (100 * pChannelData->nSampleRate) + pFX->nLenProcessWave;//(MAX_DEC_FRAME * 160) + pFX->nLenProcessWave;
	pFX->bFXComplete = 0;
	pFX->bDetectEndPoint = 0;
	//pASRFeat->lenFeatStream = 0;
	//pFX->nLenSendFrame = 0;

	pFX->bUnusualSpeech = FALSE;
	pFX->bTooLongSpeech = FALSE;
	pFX->bReceiveRefSilFeat = FALSE;
	pEpdData->nCurrentState = UTTER_START;

#ifdef __AUDIORECOG_API__
	if ((pFX->bEOS) && (pEpdData->bUseDNNEPD == 1))
	{
		pChannelData->st->range.stat = 5;
		pChannelData->st->range.eof = TRUE;
		DioAudioEPDProc(NULL, 0, pChannelData->st);
	}
#endif

	if ( pFX->bTimeOut ) {
		pFX->bTimeOut = FALSE;
		return TIME_OVER;
	}

	if (pChannelData->refSilFeat != NULL) {
		memcpy(&pOutFeat[*nSizeFeat], pChannelData->refSilFeat, LEN_REF_SIL * pFX->nFeatDim * sizeof(hci_asr_feat_t));
		*nSizeFeat += LEN_REF_SIL * pFX->nFeatDim;
	}

	return EPD_FOUND;
}


static int
_Make_Wave_Header( WAVEFILEHEADER1 *WFHeader1,
				   WAVEFILEHEADER2 *WFHeader2,
				   LONG nCountSamples,
				   LONG nSampleRate)
{
	if ( !WFHeader1 || !WFHeader2 ) return -1;

	strcpy(WFHeader1->szRiff, "RIFF");
	strcpy(WFHeader1->szWave, "WAVEfmt ");
	WFHeader1->lFileSize = 2 * nCountSamples + sizeof(WAVEFILEHEADER1) + sizeof(WAVEFILEHEADER2) - 8;
	WFHeader1->lWaveFormatSize = 16;
	WFHeader1->nFormat = 1;
	WFHeader1->nChannels = 1;
	WFHeader1->lSamplesPerSec = nSampleRate;
	WFHeader1->lAvgBytesPerSec = nSampleRate * 2;
	WFHeader1->nBlockAlign = 2;
	WFHeader1->nBits = 16;

	strcpy(WFHeader2->szData, "data");
	WFHeader2->lDataSize = 2 * nCountSamples;

	return 0;
}

/**
 *	Save EPD wave stream.
 *
 *	@return none.
 */
HCILAB_PUBLIC POWERDSR_FE_API VOID
PowerDSR_FE_SaveEPDWaveStream(const LONG nChannelID,			///< (i) DSR front-end channel index
							  const CHAR *szWaveFile)			///< (i) EPD wave file
{
	FrontEnd_UserData *pChannelData = 0;
	DSR_FX_DATA *pFX = 0;
	SPEEX_DEC_DATA *pSpeexData = 0;
	SHORT *pEpdWave = 0;
	EPD_UserData *pEpdData = NULL;
	LONG nLenEpdWave = 0;
	int nStartPt = 0, nEndPt = 0;

	WAVEFILEHEADER1 WFHeader1;
	WAVEFILEHEADER2 WFHeader2;
	hci_int32 nLenMargin = 0;

	if ( 0 == g_DSR_FE ) return;
	if ( 0 == szWaveFile ) return;
	if ( nChannelID < 0L || nChannelID >= g_nCHNL_FE ) return;
	
	pChannelData = g_chanDSRFE[nChannelID];
	if (0 == pChannelData) return;

	pFX        = &pChannelData->dataFX;
	pSpeexData = &pChannelData->dataSpeex;
	pEpdData = &pChannelData->dataEpd;
	memset( &WFHeader1, 0, sizeof(WFHeader1) );
	memset( &WFHeader2, 0, sizeof(WFHeader2) );
	if (pFX->nFrameSampleSize == 160) {
		nLenMargin = PowerASR_FrontEnd_getLogSpeechMargin( g_DSR_FE );
	} else {
		nLenMargin = PowerASR_FrontEnd_getLogSpeechMargin( g_DSR_FE_8k );
	}
	

	if ( pFX->nEndPoint > pFX->nStartPoint && pSpeexData->rec_wave ) {
		FILE *fpWave = 0;
		fpWave = fopen(szWaveFile, "wb");
		if ( fpWave ) {
			LONG start = 0;
			LONG end = 0; 
			int nPadding = 0;
			PowerDSR_FE_GET_EPD_INFO(nChannelID, &nStartPt, &nEndPt, &nPadding);
			start = (nStartPt * pFX->nFrameSampleSize);// - nLenMargin;
			end = (nEndPt * pFX->nFrameSampleSize);// + nLenMargin;

			start = HCI_MAX(0, start);
			end   = HCI_MIN(pSpeexData->len_rec_wave, end);
			
			pEpdWave = pSpeexData->rec_wave + start;
			nLenEpdWave = end - start + 1;
			if (nLenEpdWave < 0) {
				fclose(fpWave);
				return;
			}

			_Make_Wave_Header( &WFHeader1, &WFHeader2, nLenEpdWave, pFX->nFrameSampleSize * 100 );
			fwrite(&WFHeader1, 1, sizeof(WFHeader1), fpWave);
			fwrite(&WFHeader2, 1, sizeof(WFHeader2), fpWave);
			fwrite(pEpdWave, sizeof(SHORT), nLenEpdWave, fpWave);
			fclose(fpWave);
		}
		
	}

}


/**
 *	get idle DSR front-end channel index.
 */
static LONG
_PowerDSR_FE_getIdleChannel()	
{
	DWORD dwWaitResult;
	LONG n = 0L;
	LONG ret = -1L;

#ifdef WIN32

	if ( 0 == g_hThreadMutex_FE ) {
		fprintf(stderr, " getIdleChannel : Mutex handle is null!\n");
		return ret;
	}

	dwWaitResult = WaitForSingleObject( g_hThreadMutex_FE, 5000L );
 
	switch ( dwWaitResult ) {
		case WAIT_OBJECT_0:

			for ( n = 0L ; n < g_nCHNL_FE ; n++ ) {
				if ( !g_bOccupiedCh_FE[n] ) {
					g_bOccupiedCh_FE[n] = TRUE;
					ReleaseMutex(g_hThreadMutex_FE);
					return n;
				}
			}
			ReleaseMutex(g_hThreadMutex_FE);
			break;

		case WAIT_ABANDONED:
			return ret;
		case WAIT_TIMEOUT:
			return ret;
	}

#else // !WIN32	

	pthread_mutex_lock( &g_mutex_FE );
	for ( n = 0L ; n < g_nCHNL_FE ; n++ ) {
		if ( !g_bOccupiedCh_FE[n] ) {
			g_bOccupiedCh_FE[n] = TRUE;
			pthread_mutex_unlock( &g_mutex_FE );
			return n;
		}
	}
	pthread_mutex_unlock( &g_mutex_FE );

#endif //WIN32

	return ret;
}

/**
 *	disable the status of DSR front-end channel.
 */
static BOOL
_PowerDSR_FE_disableChannel(const LONG nChannelID )		///< (i) channel index
{
	DWORD dwWaitResult;
	FrontEnd_UserData *pChannelData = 0;

	if (  nChannelID < 0L || nChannelID >= g_nCHNL_FE ) return FALSE;

#ifdef WIN32

	if ( 0 == g_hThreadMutex_FE ) return FALSE;

	dwWaitResult = WaitForSingleObject( g_hThreadMutex_FE, 5000L );
 
	switch ( dwWaitResult ) {
		case WAIT_OBJECT_0:
			pChannelData = g_chanDSRFE[nChannelID];
			if (pChannelData) {
				hci_free(pChannelData);
				pChannelData = 0;
			}
			g_chanDSRFE[nChannelID] = 0;
			g_bOccupiedCh_FE[nChannelID] = FALSE;
			ReleaseMutex(g_hThreadMutex_FE);
			break;
		case WAIT_ABANDONED:
			return FALSE;
		case WAIT_TIMEOUT:
			return FALSE;
	}

#else	// !WIN32

	pthread_mutex_lock( &g_mutex_FE );
	pChannelData = g_chanDSRFE[nChannelID];
	if ( pChannelData ) {
		hci_free(pChannelData);
		pChannelData = 0;
	}
	g_chanDSRFE[nChannelID] = 0;
	g_bOccupiedCh_FE[nChannelID] = FALSE;
	pthread_mutex_unlock( &g_mutex_FE );

#endif

	return TRUE;
}


/**
*	set feature normalization vectors for a new front-end channel.
*/
static BOOL
//_PowerDSR_FE_setFeatureNormalizer(const LONG nChannelID,char )		///< (i) thread channel index
_PowerDSR_FE_setFeatureNormalizer(const LONG nChannelID, char *cmsModelName )//KSH		///< (i) thread channel index
{
	DWORD dwWaitResult = 0;
	FrontEnd_UserData *pChannelData = 0;
	PowerASR_FrontEnd* Global_DSR_FE = NULL;
	if (  nChannelID < 0L || nChannelID >= g_nCHNL_FE ) return FALSE;
	
#ifdef WIN32
		
	dwWaitResult = WaitForSingleObject( g_hThreadMutex_FE, 5000L );
	
	switch ( dwWaitResult ) {
		case WAIT_OBJECT_0:
			pChannelData = g_chanDSRFE[nChannelID];
			if (pChannelData->nSampleRate == 16000) {
				Global_DSR_FE = g_DSR_FE;
			} else if (pChannelData->nSampleRate == 8000) {
				Global_DSR_FE = g_DSR_FE_8k;
			}
//			PowerASR_FrontEnd_setFeatureNormalizer( g_DSR_FE, pChannelData, &g_FeatNorm );
			PowerASR_FrontEnd_setFeatureNormalizer( Global_DSR_FE, pChannelData, &g_FeatNorm ,cmsModelName ,nChannelID);//KSH
			ReleaseMutex(g_hThreadMutex_FE);
			break;
		case WAIT_ABANDONED:
			return FALSE;
		case WAIT_TIMEOUT:
			return FALSE;
	}
	
#else	// !WIN32
	
	pthread_mutex_lock( &g_mutex_FE );
	pChannelData = g_chanDSRFE[nChannelID];
	if (pChannelData->nSampleRate == 16000) {
		Global_DSR_FE = g_DSR_FE;
	} else if (pChannelData->nSampleRate == 8000) {
		Global_DSR_FE = g_DSR_FE_8k;
}
	//PowerASR_FrontEnd_setFeatureNormalizer( g_DSR_FE, pChannelData, &g_FeatNorm );
	PowerASR_FrontEnd_setFeatureNormalizer( Global_DSR_FE, pChannelData, &g_FeatNorm ,cmsModelName ,nChannelID);//KSH
	pthread_mutex_unlock( &g_mutex_FE );
	
#endif

	return TRUE;
}
int updateFlag[128]={0,};
int updateCount[128]={0,};
void PowerDSR_FE_SetUpdateFlag(int n,int flag){
	updateFlag[n]=flag;
}
int PowerDSR_FE_getCount(int deviceID){
	return updateCount[deviceID];
}
extern FEAT_Normalizer *luSeed[128];


/**
*	update feature normalization vectors.
*/
static BOOL
_PowerDSR_FE_updateFeatureNormalizer(const LONG nChannelID )		///< (i) thread channel index
{
	DWORD dwWaitResult = 0;
	FrontEnd_UserData *pChannelData = 0;
	FEAT_Normalizer * lf=0;
	int seedID=0;	
	if (  nChannelID < 0L || nChannelID >= g_nCHNL_FE ) return FALSE;
	
#ifdef WIN32
	
	dwWaitResult = WaitForSingleObject( g_hThreadMutex_FE, 5000L );
	
	switch ( dwWaitResult ) {
		case WAIT_OBJECT_0:
			pChannelData = g_chanDSRFE[nChannelID];
			seedID = PowerASR_FrontEnd_getCMSSeed(nChannelID,lf);
			if (luSeed[nChannelID] != NULL) {
				updateCount[seedID]++;	
				memcpy( luSeed[nChannelID], &(pChannelData->dataASRFeat.mfccStream.userFeatNorm), sizeof(FEAT_Normalizer) );
			}
			ReleaseMutex(g_hThreadMutex_FE);
			break;
		case WAIT_ABANDONED:
			return FALSE;
		case WAIT_TIMEOUT:
			return FALSE;
	}
	
#else	// !WIN32
	
	pthread_mutex_lock( &g_mutex_FE );
	pChannelData = g_chanDSRFE[nChannelID];
	seedID = PowerASR_FrontEnd_getCMSSeed(nChannelID, lf);
	if (luSeed[nChannelID] != NULL) {
		updateCount[seedID]++;
		memcpy(luSeed[nChannelID], &(pChannelData->dataASRFeat.mfccStream.userFeatNorm), sizeof(FEAT_Normalizer));
	}
	pthread_mutex_unlock( &g_mutex_FE );
	
#endif

	return TRUE;
}


/**
 *	initialize front-end modules
 */
HCILAB_PRIVATE VOID
_PowerDSR_FE_initializeFrontEnd(LONG nChanID)
{
	DSR_FX_DATA *pFX = 0;

	if ( nChanID < 0L || nChanID >= g_nCHNL_FE ) return;

	pFX = &g_chanDSRFE[nChanID]->dataFX;

	pFX->nStartPoint          = 0;
	pFX->nEndPoint            = 0;
	pFX->bSpeechPeriod        = FALSE;
	pFX->bDetectStartPoint    = FALSE;
	pFX->bDetectEndPoint      = FALSE;
	pFX->bInitFX              = FALSE;
	pFX->bFXComplete	      = FALSE;
	pFX->bUnusualSpeech       = FALSE;
	pFX->bTooLongSpeech       = FALSE;
	pFX->bTooLowSNR           = FALSE;
	pFX->bClippedSpeech       = FALSE;
	pFX->bTimeOut             = FALSE;

}
int PowerDSR_FE_GetCMSList(char * arr){
	return GetCMSList(arr);
}

HCILAB_PUBLIC POWERDSR_FE_API LONG PowerDSR_FE_GET_SPEEX_INFO(const LONG nChannelID) {
	FrontEnd_UserData*	pChannelData = 0;
	SPEEX_DEC_DATA*		pSpeexData = 0;
	if ( 0 == g_DSR_FE ) return FX_FAILED;
	if ( nChannelID < 0L || nChannelID >= g_nCHNL_FE ) return FX_FAILED;
	pChannelData = g_chanDSRFE[nChannelID];
	if (0 == pChannelData) return FX_FAILED;
	pSpeexData = &pChannelData->dataSpeex;
	return pSpeexData->len_rec_wave;


}

HCILAB_PUBLIC POWERDSR_FE_API LONG PowerDSR_FE_GET_EPD_INFO(const LONG nChannelID,int *startFrame,int *endFrame, int *nPaddingSilFrame)
{
	FrontEnd_UserData*	pChannelData = 0;
	SPEEX_DEC_DATA*		pSpeexData = 0;
	DSR_FX_DATA*		pFX = 0;
	EPD_UserData*		pEpdData = 0;
	Feature_UserData*	pASRFeat = 0;
	ARData*             pDioEpd = 0; // kklee

	if ( 0 == g_DSR_FE ) return FX_FAILED;
	if ( nChannelID < 0L || nChannelID >= g_nCHNL_FE ) return FX_FAILED;

	pChannelData = g_chanDSRFE[nChannelID];
	if (0 == pChannelData) return FX_FAILED;
	pFX = &pChannelData->dataFX;	
	pEpdData   = &pChannelData->dataEpd;
	pSpeexData = &pChannelData->dataSpeex;
	
#ifdef __AUDIORECOG_API__
	if (pEpdData->bUseDNNEPD == 1 && FALSE) {
		int lRet = 0;
		pDioEpd = pChannelData->st;
		lRet = DioAudioEPDGetRange(startFrame, endFrame, pDioEpd);
		*startFrame += (pEpdData->nFrontSkipFrameSize  + 2 + 2);
		*endFrame += (pEpdData->nFrontSkipFrameSize + 2 + 2);
		*nPaddingSilFrame = pFX->bReceiveRefSilFeat;
		return lRet;
	}
#endif

	int nLenMargin = 0;
	int offset = pEpdData->nFrontSkipFrameSize  + 2 + 2;
	if (pFX->nFrameSampleSize == 160) {
		nLenMargin = PowerASR_FrontEnd_getLogSpeechMargin(g_DSR_FE);
	} else {
		nLenMargin = PowerASR_FrontEnd_getLogSpeechMargin(g_DSR_FE_8k);
	}
	if (pFX->nStartPoint != -1 && pFX->nEndPoint != -1) {
		*startFrame = HCI_MAX(0, (pFX->nStartPoint / pFX->nFrameSampleSize));
		*endFrame = HCI_MIN((pSpeexData->len_rec_wave / pFX->nFrameSampleSize), (pFX->nEndPoint / pFX->nFrameSampleSize));

		*startFrame += offset;
		*endFrame += offset;
	} else {
		*startFrame = 0;
		*endFrame = 0;
	}
	*nPaddingSilFrame = pFX->nRefSilFeatSize;
	return 0;
	
	//*startFrame = pFX->nStartPoint/pFX->nFrameSampleSize;
	//*endFrame = pFX->nEndPoint/pFX->nFrameSampleSize;
}

HCILAB_PUBLIC POWERDSR_FE_API LONG PowerDSR_FE_GET_CMS_VECTOR_INFO(const LONG nChannelID,FEAT_Normalizer *cmsVector)
{	
	char logT[1024] = {0,};
	FrontEnd_UserData *pChannelData = 0;
	pChannelData = g_chanDSRFE[nChannelID];

	if (  nChannelID < 0L || nChannelID >= g_nCHNL_FE ) return FALSE;

	memcpy(cmsVector, &(pChannelData->dataASRFeat.mfccStream.userFeatNorm), sizeof(FEAT_Normalizer) );
	return TRUE;
}

HCILAB_PUBLIC POWERDSR_FE_API LONG PowerDSR_FE_SET_CMS_VECTOR_INFO(const LONG nChannelID,char *cmsModelName,FEAT_Normalizer *cmsVector)
{
	DWORD dwWaitResult = 0;
	FrontEnd_UserData *pChannelData = 0;
	PowerASR_FrontEnd *Global_DSR_FE = NULL;
	if (  nChannelID < 0L || nChannelID >= g_nCHNL_FE ) return FALSE;
	

	pChannelData = g_chanDSRFE[nChannelID];

	if (cmsVector == NULL) {
		return FALSE;

	}


#ifdef WIN32
		
	dwWaitResult = WaitForSingleObject( g_hThreadMutex_FE, 5000L );
	
	switch ( dwWaitResult ) {
		case WAIT_OBJECT_0:
			if (pChannelData->nSampleRate == 16000) {
				Global_DSR_FE = g_DSR_FE;
			}
			else if (pChannelData->nSampleRate == 8000) {
				Global_DSR_FE = g_DSR_FE_8k;
			}
			PowerASR_FrontEnd_setClientFeatureNormalizer(Global_DSR_FE, pChannelData, cmsVector, cmsModelName, nChannelID);//KSH_20150921
			
			ReleaseMutex(g_hThreadMutex_FE);
			break;
		case WAIT_ABANDONED:
			return FALSE;
		case WAIT_TIMEOUT:
			return FALSE;
	}
	
#else	// !WIN32
	pthread_mutex_lock( &g_mutex_FE );
	if (pChannelData->nSampleRate == 16000) {
		Global_DSR_FE = g_DSR_FE;
	}
	else if (pChannelData->nSampleRate == 8000) {
		Global_DSR_FE = g_DSR_FE_8k;
	}
	PowerASR_FrontEnd_setClientFeatureNormalizer(Global_DSR_FE, pChannelData, cmsVector, cmsModelName, nChannelID);//KSH_20150921
	pthread_mutex_unlock( &g_mutex_FE );
	
#endif

	return TRUE;	

}

// end of file

