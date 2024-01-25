
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
 *	@file	epd_main.h
 *	@ingroup epd_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	Main modules for energy-based end-point detector
 */

#ifndef __EPD_MAIN_H__
#define __EPD_MAIN_H__

#include "base/hci_type.h"
#include "basic_op/hci_logadd.h"
#include "epd/epd_common.h"
#include "wave2mfcc/fx_mfcc_common.h"

typedef struct
{
//	hci_int16 nMaxSpeechFrame;			///< maximum speech duration in frame count
//	hci_int16 nMinSpeechFrame;			///< minimum speech duration in frame count
	hci_int32 nMaxSpeechFrame;			///< maximum speech duration in frame count yowon 2012-09-01
	hci_int32 nMinSpeechFrame;			///< minimum speech duration in frame count yowon 2012-09-01
	hci_int16 nMinSpeechPulseDur;		///< minimum speech pulse duration in frame count
	hci_int16 nMinVoicedFrame;			///< minimum number of voiced frames
	hci_int16 nMaxWaitFrame;			///< maximum waiting time in frame count
	hci_int16 nBeginMarginFrame;		///< frame margin in starting point of spoken utterance
	hci_int16 nEndMarginFrame;			///< frame margin in ending point of spoken utterance
	hci_int16 nEndPauseFrameTh;			///< threshold of silence frames to verify the ending of utterance
	hci_int16 nMidPauseFrameTh;			///< threshold of silence frames to verify the starting of utterance
	hci_int16 nETUCountTh;				///< threshold of reliable & consecutive voiced frame count
	hci_int16 nSizeMedianFilter;		///< window length of median filter
	hci_int16 bContinuousMode;			///< flag to continuous-mode EPD
	hci_int16 nUselessFrame;			///< number of first frames neglected in EPD process (why? generic delay in noise reducer)
	hci_int16 nInitialFrame;			///< number of first frames discarded by EPD module (only in isolation-mode EPD)
	hci_epd32 loudSpeakCheckThresh;		///< threshold to decide the loudness of input utterance
	hci_epd32 softSpeakCheckThresh;		///< threshold to decide the softness of input utterance
	hci_epd32 constSNRTem;				///< constant term to calculate SNR value
	//klaud 09-03-23
	hci_epd32 EnergyTHforE1;			///< energy threshold in the case of avgNoiseEnrgy is less than T_E1
	hci_epd32 LowerEnergyTHforE1;		///< lower energy threshold in the case of avgNoiseEnrgy is less than T_E1
	hci_epd32 UpperEnergyTHforE1;		///< upper energy threshold in the case of avgNoiseEnrgy is less than T_E1
	hci_epd32 LowerEntropyTHforE1;
	hci_epd32 UpperEntropyTHforE1;
	////
	hci_int32 nFrontSkipFrameSize;
	hci_flag bUseDNNEPD;

} EpdParameters;

//////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

/**
 *  main function to detect start/end points of spoken utterance
 */
hci_flag
EPD_detectSpeechBoundary(EPD_UserData *pEpdData,		///< (i/o) : pointer to EPD data per user (channel)
						 EpdParameters *pEpdVar,		///< (i) : EPD config. variables
						 hci_epd32 specEntropy,			///< (i) : spectral entropy (Q15.32)
						 hci_epd32 frameLogEnergy,		///< (i) : log frame energy (Q15.32)
						 hci_epd32 frameLogPower,		///< (i) : log frame power (Q0.16)
						 hci_int32 flagNRVad,			///< (i) : VAD output in noise-reduction module
						 hci_int32 bSpeechFound			///< (i) flag to speech found
);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef __EPD_MAIN_H__


