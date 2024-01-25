
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
 *	@file	fx_mfcc_common.h
 *	@ingroup wave2mfcc_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	constants/structure definition for MFCC computation
 */

#ifndef __FX_MFCC_COMMON_H__
#define __FX_MFCC_COMMON_H__

#include "base/hci_type.h"
#include "basic_op/fixpoint.h"
#include "common/powerasr_const.h"

/* Default values */

#define DIM_CEPSTRUM	12
#define NUM_FILTERBANK	23
#define PRE_EMPHASIS_HTK	0.97
#define PRE_EMPHASIS_ETSI	0.9
#define CEP_LIFTER_SIZE	22
#define NUM_BLOCK		3

#define BB_SAMPLING_RATE	16000
#define BB_FFT_SIZE			512
#define BB_FRAME_SIZE		480
#define BB_FRAME_SHIFT		160
#define BB_LOWER_FILT_FREQ	128
#define BB_UPPER_FILT_FREQ	8000

#define NB_SAMPLING_RATE	8000
#define NB_FFT_SIZE			256
#define NB_FRAME_SIZE		240
#define NB_FRAME_SHIFT		80
#define NB_LOWER_FILT_FREQ	64		///< lower frequency at 8kHz, 300 at telephone line
#define NB_UPPER_FILT_FREQ	4000	///< upper frequency at 8kHz, 3400 at telephone line

#define SIZE_MFCC_BUFFER	128//128		///< max buffer size of mfcc stream
#define SIZE_MFCC_HISTORY	8//8
#define DIM_FEATURE			52		///< max feature dimension per frame
//#define QUANT_LEVEL_MFCC	16		///< number of mfcc quantization levels (16~256)
#define QUANT_LEVEL_MFCC	64		///< number of mfcc quantization levels (16~256)

#ifdef FIXED_POINT_FE
typedef	hci_int16	hci_mfcc16;
typedef	hci_int32	hci_mfcc32;
typedef hci_int64	hci_mfcc64;
#else // !FIXED_POINT_FE
typedef hci_float32 hci_mfcc16;
typedef hci_float32 hci_mfcc32;
typedef hci_float32	hci_mfcc64;
#endif // #ifdef FIXED_POINT_FE

#ifndef hci_mfcc_defined
#ifdef FIXED_POINT_FE
typedef hci_fixed32 hci_mfcc_t;
#else	// !FIXED_POINT_FE
typedef	hci_float32	hci_mfcc_t;
#endif	// #ifdef FIXED_POINT_FE
#define hci_mfcc_defined
#endif	// #ifndef hci_mfcc_defined

#ifndef hci_asr_feat_defined

#define SIZE_FEAT_STREAM	(MAX_LEN_FEAT_FRAME*DIM_FEATURE)


//typedef	hci_uint8	hci_asr_feat_t;//original
typedef	hci_float32	hci_asr_feat_t;//for lvcsr


#define hci_asr_feat_defined

#endif	// #ifndef hci_asr_feat_defined

/** MFCC type */
typedef enum {
	HTK_MFCC,						///< HTK MFCCs
	ETSI_MFCC,						///< ETSI standard MFCCs
	DPS_MFCC,						///< cepstrum derived from differential power spectrum
	PDPS_MFCC						///< cepstrum derived from predictive differential power spectrum
} MFCCType;

/** frame class */
typedef enum {
	SIL_FRAME,						///< non-speech frame
	UNVOICED_FRAME,					///< unvoided frame
	MIXED_FRAME,					///< mixed (unvoiced + voice) frame
	VOICED_FRAME					///< voice frame
} FrameClass;

/** mfcc cell status */
typedef enum {
	NULL_MFCC,						///< NULL mfcc cell
	DROP_MFCC,						///< dropped mfcc cell
	SKIP_MFCC,						///< compressed mfcc cell
	ACTIVE_MFCC						///< mfcc cell transfered to back-end
} CellType;

/** mfcc-to-feature conversion status */
typedef enum {
	M2F_FAIL=-1,		///< if mfcc-to-feature operation failed
	M2F_FALSE,			///< in non-speech region
	M2F_TRUE,			///< if a ASR feature vector is returned
	M2F_DROP,			///< dropped frame with long silence in both side
	M2F_SKIP,			///< skipped frame by frame compression
	M2F_RESET,			///< if starting-point of utterance is updated
	M2F_COMPLETE		///< if mfcc-to-feature operation completed !!
} M2F_Status;

/** Structure holding feature normalizer parameters. */
typedef struct {
	hci_mfcc_t cepMeanSilence[DIM_CEPSTRUM];	///< cepstral mean vector for non-speech period
	hci_mfcc_t cepMeanUnvoiced[DIM_CEPSTRUM];	///< cepstral mean vector for unvoiced-speech period
	hci_mfcc_t cepMeanVoiced[DIM_CEPSTRUM];		///< cepstral mean vector for voiced-speech period
	hci_mfcc_t cepVarSilence[DIM_CEPSTRUM];		///< cepstral inverse st-dev vector for non-speech period
	hci_mfcc_t cepVarUnvoiced[DIM_CEPSTRUM];	///< cepstral inverse st-dev vector for unvoiced-speech period
	hci_mfcc_t cepVarVoiced[DIM_CEPSTRUM];		///< cepstral inverse st-dev vector for voiced-speech period
	hci_mfcc_t maxLogE;							///< max log energy
	hci_mfcc_t minLogE;							///< min log energy
} FEAT_Normalizer;

/** Structure holding accumulated feature normalizer parameters. */
typedef struct {
	hci_int32 countUnvoicedFrame;				///< frame length of voiced-speech period for current utterance
	hci_int32 countVoicedFrame;					///< frame length of unvoiced-speech period for current utterance
	hci_int32 countSilenceFrame;				///< frame length of silence period for current utterance
	hci_mfcc_t cepMeanSilence[DIM_CEPSTRUM+1];	///< accumulated cepstral mean vector for non-speech period
	hci_mfcc_t cepMeanUnvoiced[DIM_CEPSTRUM+1];	///< accumulated cepstral mean vector for unvoiced-speech period
	hci_mfcc_t cepMeanVoiced[DIM_CEPSTRUM+1];	///< accumulated cepstral mean vector for voiced-speech period
	hci_mfcc64 cepVarSilence[DIM_CEPSTRUM+1];	///< accumulated cepstral variance vector for non-speech period
	hci_mfcc64 cepVarUnvoiced[DIM_CEPSTRUM+1];	///< accumulated cepstral variance vector for unvoiced-speech period
	hci_mfcc64 cepVarVoiced[DIM_CEPSTRUM+1];	///< accumulated cepstral variance vector for voiced-speech period
	hci_mfcc_t maxLogE;							///< maximum log energy for current utterance
} FEAT_Accumulator;

/** Structure for MFCC data per user (channel) */
typedef struct _MFCC_UserData{
	hci_int32 nInputFrame;					///< input frame index
	hci_int32 nMfccFrame;					///< MFCC frame index
	hci_int16 nNumMelBands;					///< number of mel-frequency filter banks
	hci_int16 nDimMFCC;						///< MFCC feature dimension
	hci_mfcc16 prevBuf[2*BB_FRAME_SHIFT];	///< overlapped sample buffer for next frame processing (Q(sft).16)
	hci_mfcc64 blockPower[2*NUM_BLOCK];		///< power per block to compute frame-power (Q0.64)
	hci_mfcc32 logPower[NUM_BLOCK];			///< log power per block to compute log frame-power (Q15.32)
	hci_mfcc32 subbandFrameEng[NB_FFT_SIZE]; ///< sub-band energy used in entropy computation (Q8.32)
	hci_mfcc32 noiseSpectrum[NB_FFT_SIZE];	///< noise spectrum
	hci_mfcc_t mfccVec[DIM_CEPSTRUM+2];		///< cepstrum vector (Q15.32)
    hci_mfcc_t fbankLogEn[NUM_FILTERBANK];	///< filter bank log energy kklee : 2016.01
	hci_mfcc_t nSpecEntropy;				///< spectral entropy value (Q15.32)
	hci_mfcc_t avgNoiseEnergy;				///< average noise energy value (Q15.32)
	hci_mfcc_t avgNoisePower;				///< average noise power value (Q15.32)
	hci_mfcc16 frameEn;						///< log frame power (Q0.16)
	hci_mfcc16 nPriorSample;				///< prior sample value for pre-emphasis
	hci_int16  biasEntropy;					///< bias constant term in entropy computation
	float averLogEnergy;
	int cnt_EstiFrames;
	hci_mfcc32 minLogEnergy;				/// avergy min LogEnergy KSH
	hci_mfcc32 maxLogEnergy;				/// avergy max LogEnergy KSH
	hci_mfcc32 minLogPower;				/// avergy min LogPower KSH
    hci_mfcc16 xfft[BB_FFT_SIZE];       ///< kklee 20150923 frequency spectrum
	int xfft_len;
} MFCC_UserData;

/** Structure holding frame-by-frame EPD results */
typedef struct {
	hci_int16 epd_state;					///< EPD state
	FrameClass frame_class;					///< non-speech/unvoiced/mixed/voiced class of current frame
	hci_int32 nEpdFrame;					///< current EPD frame index
	hci_int32 nStartFrame;					///< starting point of current utterance
	hci_int32 nEndFrame;					///< ending point of current utterance
} EPD_FrameOut;

/** Structure for a single MFCC cell to extract final feature vectors */
typedef struct {
	hci_mfcc_t mfccVec[DIM_CEPSTRUM+2];		///< cepstrum vector (Q15.32)
	hci_mfcc_t normVec[DIM_CEPSTRUM+2];		///< normalized cepstrum vector (Q15.32)
	hci_mfcc_t featVec[DIM_CEPSTRUM+2];		///< feature vector (Q15.32)
	hci_mfcc_t logEnergy;					///< full-band log energy
	hci_mfcc_t specEntropy;					///< spectral entropy
	hci_int16 bSpeech;						///< flag to indicate whether this mfcc cell is in speech region
	hci_int16 bActive;						///< activity flag
	hci_int32 idInFrame;					///< input frame index
	FrameClass frame_class;					///< frame class
} MFCC_Cell;

/** Structure for the most recent MFCC stream per user (channel) */
typedef struct {
	hci_int32 maxLenStream;						///< maximum length of mfcc stream
	hci_int32 curLenSilence;					///< current length of silence period to drop long silence period
	hci_int32 curLenSpeech;						///< current length of end-point detected speech frames
	hci_int32 epd_state;						///< EPD state
	hci_int32 nCurrentMfccCell;					///< frame length of input MFCC vector from starting-point of utterance
	hci_int32 nOutputMfccCell;					///< frame length of final post-processed feature stream
	hci_int32 nLenOutputMfcc;					///< frame length of output feature stream
	hci_int32 nLenSpeechFrame;					///< length of speech duration
	hci_int32 nLenDropFrame;					///< number of silence-dropped frames
	hci_int32 nLenSkipFrame;					///< number of compressed frames
	FEAT_Normalizer userFeatNorm;				///< user-specific feature normalizer
	FEAT_Normalizer baseFeatNorm;				///< user-specific feature normalizer (back-up for recursive CMN)
	FEAT_Accumulator userFeatAccum;				///< user-specific feature accumulator
	MFCC_Cell mfccPool[SIZE_MFCC_BUFFER];		///< mfcc stream buffer
	MFCC_Cell *preCell[SIZE_MFCC_HISTORY];		///< previous-frame mfcc cell
} MFCC_Stream;

#if 0 //original
/** Structure for intermediate feature stream per user (channel) */
typedef struct {
	hci_int32 *time_stamp;						///< time stamp vector for a spoken utterance
	hci_mfcc_t *sent_feat;						///< feature stream vector for a spoken utterance
	FrameClass *frame_class;					///< frame class vector for a spoken utterance
} UTTER_FEATURE;
#else //yowon dnn
/** Structure for intermediate feature stream per user (channel) */
typedef struct {
	hci_mfcc_t *sent_feat;						///< feature stream vector for a spoken utterance
	hci_mfcc_t *sent_feat_gpu;						///< gpu feature stream vector for a spoken utterance  20141201 bykim
	FrameClass *frame_class;					///< frame class vector for a spoken utterance
} UTTER_FEATURE;
#endif

/** Structure for final feature stream per user (channel) */
typedef struct {
	hci_int16 dimFeat;								///< feature dimension
	hci_asr_feat_t asr_feat[SIZE_FEAT_STREAM];		///< final feature stream
	FrameClass frame_class[MAX_LEN_FEAT_FRAME];		///< frame class vector
	hci_int32 nTimeStamp[MAX_LEN_FEAT_FRAME];		///< time stamp for each frame feature vector in 10msec
} ASR_FEATURE;

/** Structure for MFCC pool */
typedef struct {
	hci_mfcc_t mfccVec[DIM_CEPSTRUM+2];		///< cepstrum vector (Q15.32)
	hci_mfcc_t featVec[DIM_CEPSTRUM+2];		///< feature vector (Q15.32)
	hci_int16 bSpeech;						///< flag to indicate whether this mfcc cell is in speech region
	hci_int16 bActive;						///< activity flag
	hci_int16 frame_class;					///< non-speech/unvoiced/mixed/voiced class of current frame
} MFCC_POOL;

/** Structure for output feature vector per user (channel) */
typedef struct {
	hci_int32 lenFeatStream;					///< length of ASR feature stream
	hci_mfcc_t feat[DIM_FEATURE];				///< output feature vector at current frame
	MFCC_Stream mfccStream;						///< mfcc stream for frame-by-frame processing
	EPD_FrameOut epd_result;					///< frame-by-frame EPD result
	UTTER_FEATURE utterStream;					///< temporary feature stream to use in batch-mode post-processing
	ASR_FEATURE *featStream;						///< final feature stream used in next ASR process
	//MFCC_POOL mfccHist[MAX_LEN_FEAT_FRAME];		///< mfcc stream pool for post-processing
	MFCC_POOL *mfccHist;		///< mfcc stream pool for post-processing
	hci_int32 nSizeMfccHist;					///< size of mfcc stream pool for post-processing
	FEAT_Normalizer gCMSVector;
	int feat_count;//yowon 2016-07-25
} Feature_UserData;

#endif	// #ifndef __FX_MFCC_COMMON_H__
