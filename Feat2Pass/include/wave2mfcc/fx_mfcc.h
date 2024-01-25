
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
 *	@file	fx_mfcc.h
 *	@ingroup wave2mfcc_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	main MFCC computation interface
 */

#ifndef __FX_MFCC_H__
#define __FX_MFCC_H__

#include "base/hci_type.h"
#include "basic_op/fixpoint.h"
#include "basic_op/hci_logadd.h"
#include "wave2mfcc/fx_mfcc_common.h"

#define	FFTHEAD	0
#define WIDTH_SINE_FILTER	6

typedef struct PFFFT_Setup PFFFT_Setup;

/** energy type */
typedef enum {
	TIME_ENERGY,					///< temporal frame power
	FREQ_ENERGY						///< full-band spectrum energy
} EnergyType;

/** Structure holding front-end parameters. */
typedef struct {
	hci_uint16 nMFCCType;					///< MFCC type (HTK_MFCC, ETSI_MFCC, DPS_MFCC, PDPS_MFCC)
	hci_uint16 nEnergyType;					///< energy type (TIME_ENERGY, FREQ_ENERGY)
	hci_int16 nSampleRate;					///< sampling frequency rate in Hz
	hci_int16 nFrameShift;					///< frame shift length in sample count
	hci_int16 nFrameSize;					///< frame window size in sample count
	hci_int16 nNumCepstra;					///< size of cepstral vector
	hci_int16 nNumFilters;					///< number of filter banks
	hci_int16 nFFTSize;						///< FFT size
	hci_int16 nHalfFFT;						///< = nFFTSize/2
	hci_int16 nFFTStage;					///< number of stage in radix-2 FFT
	hci_int16 nLowerFiltFreq;				///< lower frequency in filter banks
	hci_int16 nUpperFiltFreq;				///< upper frequency in filter banks
	hci_int16 nCepLiftOrder;				///< cepstral liftering order
	hci_int16 nLowBin;						///< lower bin index
	hci_int16 nHighBin;						///< upper bin index
	hci_int16 nWidthSineFilter;				///< the width of sine filter in PDPS-based MFCCs
	hci_flag addEntropy;					///< flag to use spectral entropy as feature
	hci_int16 biasConstantEntropy;			///< constant term in entropy computation
	hci_mfcc16 PreEmphasis;					///< pre-emphasis coefficient (Q15.16)
	hci_mfcc16 weightC0;					///< weight value for C0 coefficient
	hci_mfcc16 fres;						///< FFT frequency resolution (Q15.16)
	hci_mfcc16 pi_factor;					///< PI factor (Q15.16)
	hci_mfcc16 mfnorm;						///< mel-freq. normalization value (Q15.16)
	hci_mfcc16 hamWin[BB_FRAME_SIZE];		///< weights of hamming window
	hci_mfcc16 cepWin[DIM_CEPSTRUM];		///< cepstral weighting vector (Q15.16)
	hci_mfcc16 lowerWeight[NB_FFT_SIZE];	///< lower weight of filter banks (Q15.16)
	hci_int16 lowerChan[NB_FFT_SIZE];		///< lower bin mapping value
	hci_mfcc16 dctCosTable[DIM_CEPSTRUM*NUM_FILTERBANK];	///< cosine table for DCT
	hci_mfcc16 sineFilter[WIDTH_SINE_FILTER];	///< sine filter in PDPS-based MFCC
	hci_mfcc16 phs_tbl[BB_FFT_SIZE];		///< phase table for fast FFT computation 
	hci_mfcc32 weightChannel[NUM_FILTERBANK];	///< weights per channel (Q15.32)
	hci_logadd_t *logAddTbl;				///< pointer to log-addition table (Q15.32)
	PFFFT_Setup* fftSetup;
} MfccParameters;


#ifdef __cplusplus
extern "C" {
#endif

/**
 * build constant parameter set necessary for MFCC computation
 */
hci_int32
FX_Wave2Mfcc_buildFeatureExtractor(MfccParameters *pMfccVar		///< (i/o) structure for feature-extraction environments
);

/**
 * Convert one frame of samples to a cepstrum vector.
 *	- Process only one frame of samples.
 */
hci_int32
FX_Wave2Mfcc_convertSingleFrameToMfccVector(MfccParameters *pMfccVar,	///< (i) structure for feature-extraction environments
											MFCC_UserData *pMfccData,	///< (o) temporary/output data of feature extraction
											hci_int16 *pFrameBuf,		///< (i/o) a single frame sample buffer
											const hci_int16 flagVAD		///< (i) VAD flag (0 = unvoiced, 1 = voiced)
);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef __FX_MFCC_H__
