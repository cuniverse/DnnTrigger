
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
 *	@file	hci_asr_common.h
 *	@ingroup interface_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	Header file to define PowerASR common data structure
 */

#ifndef __HCILAB_ASR_COMMON_H__
#define __HCILAB_ASR_COMMON_H__

#if defined(LINUX) || defined(UNIX)
#include <pthread.h>
#include <unistd.h>
#endif

#include "base/hci_type.h"
#include "basic_op/fixpoint.h"
#include "common/powerasr_const.h"

#include "speex/hci_speex.h"
#include "wiener/wiener_common.h"
#include "wave2mfcc/fx_mfcc_common.h"
#include "epd/epd_common.h"

#include "hmm/hmm_common.h"
#include "lexicon/lexicon_common.h"
#include "state_oprob/oprob_common.h"
#include "viterbi/viterbi_common.h"
#include "phone_decoder/phosearch_common.h"
#include "symbol_match/smatch_common.h"

// kklee 20150922
//#include "AudioRecogAPI.h"
typedef struct __data ARData;

//////////////////////////////////////////////////////////////////////////

/* constants definitions */

/** maximum number of candidate hypotheses */
#define	SIZE_RESULT	200

#ifndef MAX_CMD_GROUP
#define MAX_CMD_GROUP	256	///< maximum number of vocabulary groups
#endif

#define	MAX_ENC_SIZE	480000

/** maximum string length of candidate hypotheses */
#define MAX_LEN_VOCAB_STR	256

/** return constants in case that creating of DSR front-end engine was requested */
#define POWERDSR_FE_CONNECTED	0
#define POWERDSR_FE_FAILED		(-1)
#define POWERDSR_FE_NO_CFG_FILE	(-2)

/** return constants in case that creating of DSR back-end engine was requested */
#define POWERDSR_BE_CONNECTED	0
#define POWERDSR_BE_FAILED		(-1)
#define POWERDSR_BE_NO_CFG_FILE	(-2)

/** wave format */
#define WAVE_PCM8K16		0		///< linear PCM (8kHz, 16bit)
#define WAVE_ALAW			1		///< A-law PCM (8kHz, 8bit)
#define WAVE_MULAW			2		///< mu-law PCM (8kHz, 8bit)
#define WAVE_PCM16K16		3		///< linear PCM (16kHz, 16bit)

/** front-end processing status */
#define FE_WAIT_SPEECH		0		///< speech starting-point was not detected
#define FE_SPEECH_START		1		///< speech starting-point was detected
#define FE_RESET_EPD		2		///< speech starting-point was reset
#define FE_GET_FEAT_DATA	3		///< feature data have to be returned
#define FE_WAIT_FEAT_DATA	4		///< returned feature data don't exist
#define FE_FX_COMPLETE		5		///< feature extraction process was completed
#define FE_TIME_OUT			(-1)	///< speech was not detected within pre-defined duration
#define FE_TOO_LONG_SPEECH	(-2)	///< too long speech duration
#define FE_TOO_LOW_SNR		(-3)	///< SNR of detected speech was too low.
#define FE_SPEECH_CLIPPING	(-4)	///< amplitude of input speech was too high (clipping)
#define FE_UNK_STATE		(-99)	///< unknown feature extraction status 

/** gender type (acoustic model type) */
#define UNKNOWN_GENDER		0		///< unknown gender (speaker-independent) acoustic model
#define MALE_AM				1		///< male-dependent acoustic model
#define FEMALE_AM			2		///< female-dependent acoustic model

/** return constants in opening DSR back-end engine */
#define POWERDSR_BE_SUCCESS			(0)		///< successfully opened DSR back-end engine
#define POWERDSR_BE_CFG_FAIL		(-1)	///< failed in opening back-end configuration files 
#define POWERDSR_BE_CREATE_FAIL		(-2)	///< failed in creating ASR back-end engine
#define POWERDSR_BE_HMM_FAIL		(-3)	///< failed in opening HMM resource
#define POWERDSR_BE_VOCAB_FAIL		(-4)	///< failed in opening lexical resource 
#define POWERDSR_BE_STATELL_FAIL	(-5)	///< failed in opening state log-likelihood calculator
#define POWERDSR_BE_VITERBI_FAIL	(-6)	///< failed in opening Viterbi scorer
#define POWERDSR_BE_PHODEC_FAIL		(-7)	///< failed in opening phonetic decoder 
#define POWERDSR_BE_SYMMATCH_FAIL	(-8)	///< failed in opening symbol matching engine
#define POWERDSR_BE_KWS_FAIL		(-9)	///< failed in opening keyword spotter
#define POWERDSR_BE_VERIFIER_FAIL	(-10)	///< failed in opening result verifier 

/** Back-End (BE) Status */
typedef enum {
	UNK_BE_STATE=-1,			///< unknown Back-End status
	OPEN_BE_CHANNEL,			///< Back-End channel open status
	CLOSE_BE_CHANNEL,			///< Back-End channel close status
	INIT_BE_STATE,				///< Back-End initialization status
	MAIN_BE_STATE,				///< Back-End processing status
	RETURN_BE_RESULT,			///< Back-End result return status
	END_BE_STATE,				///< Back-End completion status
} BE_Status;

//////////////////////////////////////////////////////////////////////////

/* structures */

/**
 *	@struct recognized candidate
 */
typedef struct  
{
	hci_uint32 idVocab;							///< vocabulary index
	hci_uint32 idGender;						///< gender index (0 = unknown, 1 = male, 2 = female)
	hci_score_t matchScore;						///< matching score
	hci_score_t confidScore;					///< confidence score
	hci_score_t scoreAM;						///< AM score
	hci_score_t scoreLM;						///< LM score
	hci_uint16 wAsrTask;						///< 현재 단어의 task index
	hci_uint16 nLenPhoneSeq;					///< length of a recognized phone-sequence
	hci_uint8 phoneSeq[MAX_LEN_PHONE_SEQ];		///< recognized phone-sequence
	char szRecogWord[MAX_LEN_VOCAB_STR];		///< recognized string
} ASR_CANDIDATE;

/**
 *	@struct channel-specific data struct for ASR results
 */
typedef struct  
{
	int nNumCandidates;									///< number of candidate hypotheses
	ASR_CANDIDATE candidate[SIZE_RESULT+2];				///< candidate hypotheses
	char szPhoneResult[512];							///< recognized phone string
	hci_score_t scoreThresh;							///< the last ranking score value
	hci_score_t symbolCM;								///< symbol-level confidence score
	LONG nLenSpeechFrame;								///< 음성의 프레임 길이
	float fScaleLM;										///< 언어모델 확률 값의 scaling factor
} ASR_RESULT;

/**
 *	@struct channel-specific decoding data struct for DSR back-end
 */
typedef struct  
{
	LONG nLenProcFrame;								///< length of processed feature stream
	hci_flag bInitDecoder;							///< flag to indicate that decoding process have to re-start.
	hci_flag bExit;									///< flag to exit decoding thread
	hci_flag bStop;									///< flag to stop current decoding process
	hci_flag bStartRecog;							///< flag to activate decoding thread
	hci_flag bEndRecog;								///< flag to indicate the completeness of decoding process
	hci_flag nBEStatus;								///< back-end status
	hci_asr_feat_t* pAsrFeat;						///< feature stream pointer for current processing frame
} DECODER_DATA;

/**
 *	@struct channel-specific feature data struct for DSR back-end
 */
typedef struct
{
	LONG nLenFeatFrame;								///< length of transmitted feature stream
	LONG nFrameFeatSize;							///< size of a single feature frame in bytes
	LONG nFeatDim;									///< feature dimension
	hci_flag bTransferComplete;						///< flag to indicate the end of feature data transmission
	hci_asr_feat_t asr_feat[SIZE_FEAT_STREAM];		///< final feature stream
} ASR_DATA;

/**
 *	@struct channel-specific DSR feature-extraction data struct for DSR front-end
 */
typedef struct  
{
	LONG nLenTimeOutTh;								///< time-out duration to wait voiced segment in sample count
	LONG nLenSpeechTh;								///< maximum speech duration in sample count
	LONG nStartPoint;								///< starting sample point of speech period
	LONG nEndPoint;									///< ending sample point of speech period
	LONG nFeatDim;									///< feature dimension
	LONG nFrameSampleSize;							///< length of a single frame wave in sample count
	LONG nSizeWaveBuf;								///< size of wave buffer
	LONG nLenProcessWave;							///< length of processed wave in sample count
	LONG nLenSendFrame;								///< size of feature stream transmitted to servers in frame count
	hci_flag bSpeechPeriod;							///< flag to indicate that current processing wave is speech
	hci_flag bDetectStartPoint;						///< flag to indicate the detection of speech start-point
	hci_flag bDetectEndPoint;						///< flag to indicate the detection of speech end-point
	hci_flag bFXComplete;							///< flag to indicate the end of feature extraction process
	hci_flag bUnusualSpeech;						///< flag to unusual speech input (too-long, too-low-snr, clipping, ...)
	hci_flag bTooLongSpeech;						///< flag to too-long speech period
	hci_flag bTooLowSNR;							///< flag to input speech with too low SNR
	hci_flag bClippedSpeech;						///< flag to speech clipping
	hci_flag bTimeOut;								///< flag to TIME_OUT
	hci_flag bExit;									///< thread exit flag
	hci_flag bInitFX;								///< flag to indicate that feature extraction process have to re-start.
	hci_flag bEOS;									///< flag to end-of-speech
	hci_flag nFEStatus;								///< front-end status
	int frameCnt;

	//float *refSilFeat;						/// KSH 15. 10. 19 // LG u+
	hci_flag bReceiveRefSilFeat;					/// KSH 15. 10. 19 // LG u+
	LONG nRefSilFeatSize;							// BJY 15. 11. 30

} DSR_FX_DATA;

/** Structure for output feature vector per user (channel) */
typedef struct {
	SPEEX_DEC_DATA dataSpeex;			///< Speex decoded data struct per channel
	Wiener_UserData dataWiener;			///< Wiener data struct per channel
	MFCC_UserData dataMfcc;				///< MFCC data struct per channel
	EPD_UserData dataEpd;				///< EPD data struct per channel
	Feature_UserData dataASRFeat;		///< feature data struct per channel
	
	DSR_FX_DATA dataFX;					///< DSR feature extraction data

	LONG nRecvDataSize;					///< size of received encoded data stream
	CHAR RecvData[MAX_ENC_SIZE];		///< received encoded data stream
	FEAT_Normalizer gCMSVector;

	hci_uint32 nCountUtter;				///< count of user's speech input event
    ARData *st;                                 ///< DNN EPD
	float *refSilFeat;
	int nSampleRate;
} FrontEnd_UserData;

/**
 *	@struct channel-specific data struct for back-end modules
 */
typedef struct 
{
	ASR_RESULT dataResult;				///< ASR results
	PHONE_RESULT phoneResult;			///< phone recognition results

	OPROB_TABLE dataOProbTable;			///< State log-likelihood table per channel
	PhoneSpace *dataPhoneSpace;			///< phone recognition data struct per channel
	ViterbiSpace dataViterbiSpace;		///< viterbi scoring data struct per channel
	SymbolSpace *dataSymbolSpace;		///< symbol matching data struct per channel
	UserLexicon dataLexicon;			///< lexical data struct per channel

	hci_int32 (*getNextPhoneLexicon)(ViterbiSpace *pSearchSpace, UserLexicon *pUserLexicon);	///< function pointer to get next base-phone lexicon data
	hci_int32 (*getNextSymbolLexicon)(SymbolSpace *pSearchSpace, UserLexicon *pUserLexicon);	///< function pointer to get next symbol lexicon data

	DECODER_DATA dataDecoder;			///< decoding data
	ASR_DATA dataDSRFeat;				///< feature stream for DSR back-end

	float scaleLMConst;					///< LM scale constant
} BackEnd_UserData;

#endif // #ifndef __HCILAB_ASR_COMMON_H__

