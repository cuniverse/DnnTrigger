
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
 *	@file	powerdsr_frontend.h
 *	@ingroup interface_src
 *	@date	2009/01/07
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	Header file to define HCILAB DSR Front-End API functions.
 */

#ifndef __POWERDSR_FRONTEDN_H__
#define __POWERDSR_FRONTEDN_H__

#include "base/hci_type.h"
#include "wave2mfcc/fx_mfcc_common.h"//KSH_20150917

#if defined(HCI_MSC_32)
#ifdef POWERDSR_FRONTEND_EXPORTS
#define POWERDSR_FE_API __declspec(dllexport)
#elif defined(POWERDSR_FRONTEND_IMPORTS)
#define POWERDSR_FE_API __declspec(dllimport)
#else	// in case of static library
#define POWERDSR_FE_API
#endif // #ifdef POWERDSR_FRONTEND_EXPORTS
#elif defined(HCI_OS2)
#define POWERDSR_FE_API
#else
#define POWERDSR_FE_API HCI_USER
#endif

#ifndef VOID
#define	VOID	void
#endif

//////////////////////////////////////////////////////////////////////////

/* constants definitions */
//#include "LVCSR_Server_CommonDef.h"
//Channel Config BGY
//#define MAX_DSR_FE_CHANNEL			LVCSR_MAX_ENGINE // 128			///< maximum number of DSR front-end channels

/** return constants in case that creating of DSR front-end engine was requested */
#define POWERDSR_FE_CONNECTED	0
#define POWERDSR_FE_FAILED		(-1)
#define POWERDSR_FE_NO_CFG_FILE	(-2)
#define	POWERDSR_FE_LICENSE_ERROR	(-3)

/**< fx-status return values */
#define RECEIV_OK				0			// 보내준 압축음성을 받고 계속 보내라는 메시지
#define TIME_OVER				1			// 정해진 시간동안 음성이 없어 녹음 종료하라는 메시지
#define EPD_FOUND				2			// 음성인식 서버에서 음성 끝점을 검출하여 녹음 종료하라는 메시지
#define EPD_RESET				3			// KSH : EPD Reset일 경우 연속어 디코더 초기화 하기 위한 메세지
#define SPD_FOUND				4			// Start Point Detect
#define FX_FAILED				(-1)		// invalid front-end status 

//////////////////////////////////////////////////////////////////////////

/* structures */

//////////////////////////////////////////////////////////////////////////

/* API functions */

#ifdef __cplusplus
extern "C" {
#endif

/**
 *	Create a new DSR Front-End engine,\n and set-up environments for DSR front-end modules.
 *
 *	@return return POWERDSR_FE_CONNECTED if DSR front-end engine was connected successfully, otherwise return POWERDSR_FE_FAILED.
 */
HCILAB_PUBLIC POWERDSR_FE_API LONG
PowerDSR_FE_Connect(const CHAR* pszASRPath,				///< (i) ASR main path
					const CHAR* pszConfigFile,			///< (i) main ASR configuration file
					int nMaxChannelCount
);

// 해당 채널의 EPD정보 Queue에 쌓여있는 EPD 정보를 얻어온다
// input : 
//  - nChannelID : 채널 정보
// OutPut :
//  - startFrame : EPD Queue에 있는 구간정보중 가장 오래된 구간정보의 Start point
//  - endFrame   : EPD Queue에 있는 구간정보중 가장 오래된 구간정보의 End point
// return : 현재 리턴되는 구간정보(startFrame, endFrame)를 제외하고 남아있는 구간 정보 개수, -1일 경우 아무런 정보가 없음
HCILAB_PUBLIC POWERDSR_FE_API LONG 
PowerDSR_FE_GET_EPD_INFO(const LONG nChannelID,
                                int *startFrame,
                                int *endFrame,
								int *nPaddingSilFrame
								);// KSH_20150917 -> kklee 20151021

HCILAB_PUBLIC POWERDSR_FE_API LONG PowerDSR_FE_GET_CMS_VECTOR_INFO(const LONG nChannelID,FEAT_Normalizer *cmsVector);//KSH_20150917
HCILAB_PUBLIC POWERDSR_FE_API LONG PowerDSR_FE_SET_CMS_VECTOR_INFO(const LONG nChannelID,char *cmsModelName,FEAT_Normalizer *cmsVector);//KSH_20150921

/**
 *	Destroy the DSR Front-End engine.
 *
 *	@return none.
 */
HCILAB_PUBLIC POWERDSR_FE_API VOID
PowerDSR_FE_Disconnect(
);


/**
 *	Open a new DSR front-end channel.
 *
 *	@return return idle DSR front-end channel index. If all DSR front-end channels are busy, return -1.
 */
/*
HCILAB_PUBLIC POWERDSR_FE_API LONG
PowerDSR_FE_OpenChannel(
);
*/
HCILAB_PUBLIC POWERDSR_FE_API LONG // KSH
PowerDSR_FE_OpenChannel(char *cmsModelName, int bUse16k
);


/**
 *	Close an DSR front-end channel.
 *
 *	@return none.
 */
HCILAB_PUBLIC POWERDSR_FE_API VOID
PowerDSR_FE_CloseChannel(const LONG nChannelID				///< (i) DSR front-end channel index
);

/**
 *	Initialize a DSR front-end engine.
 *
 *	- Initialize DSR front-end engine.
 *
 *	@return return 0 if DSR front-end engine was initialized successfully, otherwise return -1.
 */
HCILAB_PUBLIC POWERDSR_FE_API LONG
PowerDSR_FE_InitializeFrontEndEngine(const LONG nChannelID,char *cmsModelName)//KSH				///< (i) DSR front-end channel index
;

/**
 *	Release a DSR front-end engine.
 *
 *	- Release DSR front-end engine.
 *
 *	@return none.
 */
HCILAB_PUBLIC POWERDSR_FE_API VOID
PowerDSR_FE_ReleaseFrontEndEngine(const LONG nChannelID			///< (i) DSR front-end channel index
);

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
											  const LONG bEOS				///< (i) flag to end-of-speech
);
HCILAB_PUBLIC POWERDSR_FE_API LONG
PowerDSR_FE_SpeechStream2FeatureStream(const LONG nChannelID,		///< (i) DSR front-end channel index
											  hci_float32* pOutFeat,				///< (o) new feature stream data
											  LONG* nSizeFeat,				///< (o) size of feature stream data in bytes
											  short* encData,				///< (i) speex encoded speech stream
											  LONG nDataSize,				///< (i) size of encoded speech stream in bytes
											  LONG nMaxFeatFrame,			///< (i) maximum size of feature stream in frame count
											  const LONG bEOS				///< (i) flag to end-of-speech
);

/**
 *	Save EPD wave stream.
 *
 *	@return none.
 */
HCILAB_PUBLIC POWERDSR_FE_API VOID
PowerDSR_FE_SaveEPDWaveStream(const LONG nChannelID,				///< (i) DSR front-end channel index
							  const CHAR *szWaveFile				///< (i) EPD wave file
);

HCILAB_PUBLIC POWERDSR_FE_API LONG PowerDSR_FE_GET_SPEEX_INFO(const LONG nChannelID);

HCILAB_PUBLIC POWERDSR_FE_API LONG SpeechStream2FeatureStream(	const LONG nChannelID,
																hci_float32* pOutFeat,				///< (o) new feature stream data
																LONG* nSizeFeat,					///< (o) size of feature stream data in bytes
																LONG nMaxFeatFrame,					///< (i) maximum size of feature stream in frame count
																const LONG bEOS
																); 



void PowerDSR_FE_SetUpdateFlag(int n,int flag);
int PowerDSR_FE_GetCMSList(char * arr);
int PowerDSR_FE_getCount(int deviceID);
#ifdef __cplusplus
}
#endif

#endif // #ifndef __POWERDSR_FRONTEDN_H__

