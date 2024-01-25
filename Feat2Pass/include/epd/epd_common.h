
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
 *	@file	epd_common.h
 *	@ingroup epd_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	constants/type/structure definitions for end-point detector
 */

#ifndef __EPD_COMMON_H__
#define __EPD_COMMON_H__

#include "base/hci_type.h"
#include "basic_op/fixpoint.h"
#include "common/powerasr_const.h"
#include "wave2mfcc/fx_mfcc_common.h"

/* constant definitions */

#define		MAX_SPEECH_DUR	MAX_LEN_FEAT_FRAME		///< maximum speech duration in frame count (typically 10sec)
#define		MIN_SPEECH_DUR	20			///< minimum speech duration in frame count (typically 0.2sec)
#define		MAX_WAIT_TIME	1000		///< maximum waiting time in frame count (typically 10sec)
#define		MAX_TOTAL_DUR	(MAX_SPEECH_DUR+MAX_WAIT_TIME)	///< maximum total recording time in frame count

#define		MIN_SPEECH_PULSE_DUR	10	///< minimum speech pulse duration in frame Count

#define		LEN_MEDIAN		8			///< length of median filter (typically 3 ~ 11)

#define		SIZE_VAD_BUFFER	128			///< size of previous VAD flag buffer

/** constants definition to indicate EPD status */

#define	EPD_STATUS_DETECTED			1	///< indicate that speech end-point was detected
#define EPD_STATUS_NOT_DETECTED		0	///< indicate that speech start-point or end-point were not detected

#define EPD_STATUS_SYSTEM_ERROR		-1	///< indicate that audio device had some problems
#define	EPD_STATUS_TIMEOUT_FAIL		-2	///< indicate that none speech start-points were not detected within pre-defined time-out period
#define EPD_STATUS_LONG_SPEECH_FAIL	-3	///< indicate that detected speech duration was too long
#define	EPD_STATUS_LOW_FAIL			-4	///< indicate that SNR value of detected speech was too low
#define	EPD_STATUS_HIGH_FAIL		-5	///< indicate that SNR value of detected speech was too high

#define CHECK_SNR
#ifdef CHECK_SNR
	#define LOW_SNR	 1		///< low-valued SNR
	#define HIGH_SNR 0		///< high-valued SNR
	#define GOOD_SNR 2		///< SNR values appropriate for speech recognition
#endif

/**
 * @enum current EPD state
 */
typedef enum {
	UTTER_START,			///< initial state
	SIL_SPEECH,				///< front silence state to transit speech state
	CORE_SPEECH,			///< core speech state
	SPEECH_SIL,				///< back silence state to detect the end of utterance
	UTTER_END				///< final state to confirm the speech detection
} EpdState;

/* variable definitions */

#ifdef FIXED_POINT_FE
typedef	hci_int16	hci_epd16;
typedef	hci_int32	hci_epd32;
typedef hci_int64	hci_epd64;
#else // !FIXED_POINT_FE
typedef hci_float32 hci_epd16;
typedef hci_float32 hci_epd32;
typedef hci_float32	hci_epd64;
#endif // #ifdef FIXED_POINT_FE


/*
**********************************************************************
*                                                                    *
*                              Structures                            *
*                                                                    *
**********************************************************************
*/

/** EPD data structure per user (channel) */
typedef struct
{
	FrameClass frame_class;				///< frame class(NON-SPEECH/UNVOICED/MIXED/VOICED)
	hci_int32 bFoundSpeech;				///< flag to speech found
	hci_int32 nEpdFrame;				///< EPD frame count
	hci_int32 nRealBeginPt;				///< real start point without margin
	hci_int32 nRealEndPt;				///< real end point without margin
	hci_int32 nStartFrame;				///< start point with margin
	hci_int32 nEndFrame;				///< end point with margin
	hci_int32 nStartMargin;				///< SNR-dependent starting margin in frame count
	hci_int32 nEndMargin;				///< SNR-dependent ending margin in frame count
	hci_int16 nCurrentState;			///< current EPD state
	hci_int16 nPrevState;				///< previous EPD state
	hci_int32 nLenSpeech;				///< speech frame count
	hci_int32 nLenSilence;				///< silence frame count
	hci_int16 nNumPause;				///< length of consecutive pause
	hci_int16 nSpeechDur;				///< speech duration
	hci_int16 nNumVoiceFrame;			///< number of voiced frames
	hci_int32 nNumUtter;				///< number of processing utterances up to date
	hci_int16 sizeBuf;					///< buffer size
	hci_int16 bSpeechStarted;			///< flag to indicate which speech starting-point was detected
	hci_epd32 medianValue;				///< median value used in EPD process
	hci_epd32 recentValue[LEN_MEDIAN];	///< buffer used in the computation of median value
	hci_epd32 speechEn;					///< average log power for speech frames
	hci_epd32 noiseEn;					///< average log power for noise frames
	hci_epd32 valueSNR;					///< SNR value
	hci_epd32 avgNoiseEnergy;			///< average background noise log-energy
	hci_epd32 avgNoisePower;			///< average background noise log-power
	hci_epd32 lowerVadThresh;			///< lower threshold to decide speech/non-speech frame
	hci_epd32 upperVadThresh;			///< upper threshold to decide speech/non-speech frame
	hci_epd32 maxSpecEntropy;			///< maximum spectral entropy value
	hci_int32 nCountVoiced;				///< frame count of successive voiced frames
	hci_flag bDetectStartPt;			///< flag to indicate the detection of speech starting point
	hci_flag bDetectEndPt;				///< flag to indicate the detection of speech ending point
	hci_flag bResetStartPt;				///< flag to reset the detection status of speech starting point
	hci_int32 nCountEndUpdate;			///< end-point update count
	hci_uint8 vadHist[SIZE_VAD_BUFFER];	///< recent VAD flag buffer
	hci_epd32 entropy[SIZE_VAD_BUFFER];	///< recent spectral entropy buffer
	hci_int32 maxVoicedDur;				///< max. duration of consecutive voiced frames
	hci_int32 curVoicedDur;				///< current duration of consecutive voiced frames
	hci_int32 maxEndVoiceDur;			///< max. duration of consecutive voiced frames in speech-silence period
	hci_flag bNonSpeechFrame;			///< flag to non-speech frame
	hci_int32 nFrontSkipFrameSize;
	hci_flag bUseDNNEPD;
} EPD_UserData;

#endif	// #ifndef __EPD_COMMON_H__
