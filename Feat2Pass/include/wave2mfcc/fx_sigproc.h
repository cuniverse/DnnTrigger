
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
 *	@file	fx_sigproc.h
 *	@ingroup wave2mfcc_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	signal processing library for MFCC computation
 */

#ifndef __FX_SIGPROC_H__
#define __FX_SIGPROC_H__

#include "base/hci_type.h"
#include "basic_op/fixpoint.h"
#include "fx_mfcc.h"

#ifndef	M_PI
#define M_PI	(3.14159265358979323846)
#endif	/* M_PI */

#ifndef FIX_PI
#define	FIX_PI	12868			/* Q12/16bit <- 3.1415927 */
#endif	/* FIX_PI */

#ifdef __cplusplus
extern "C" {
#endif

/**
 *	create buffer & constants necessary for Fast Fourier Transform 
 */
void
FX_SigProc_createFFTConstants(MfccParameters *pMfccVar		///< (i/o) MFCC parameters
);

/**
 *	build mel-frequency filter banks
 */
void
FX_SigProc_buildMelFilters(MfccParameters *pMfccVar		///< (i/o) MFCC parameters
);

/**
 *	create weight vector for cepstral weighting (liftering)
 *		- w_n  = 1 + (L/2) * sin(n*PI/L);
 */
void
FX_SigProc_createCepstralWindow(hci_mfcc16 *pCepWin,		///< (o) cepstrum weight vector
								hci_int16 nCepLiftOrder,	///< (i) cepstrum liftering order
								hci_int16 nNumCepstra		///< (i) cepstrum dimension
);

/**
 *	cepstral weighting (liftering)
 *		- c_n' = w_n * c_n
 */
void
FX_SigProc_weightCepstrum(hci_mfcc_t *pCepVec,		///< (i/o) input/output cepstrum vector
						  hci_mfcc16 *pCepWin,		///< (i)   cepstral window
						  hci_int16 nCepDim			///< (i)   dimension of cepstrum vector
);

/**
 *	create sine filter for use in predictive differential power spectrum based MFCCs
 *		- h_i = sin(i * 0.5 * PI / W),  0 <= i <= W, (typically W = 6)
 */
void
FX_SigProc_createSineFilter(MfccParameters *pMfccVar	///< (i) MFCC parameters
);

/**
 *	create cosine matrix table for DCT (Q.16)
 *		- w(i,j)  = sqrt(2/N) * cos(i * PI * (j-0.5) / N),
 *			where, i=0, ..., P-1, j=0, ..., N-1 
 *					P = cepstrum order, 
 *					N = number of bands in mel filter-bank
 */
void
FX_SigProc_createDCTCosineTable(MfccParameters *pMfccVar	///< (i) MFCC parameters
);

/**
 *	perform discrete cosine transform (DCT) to convert mel-spectrum into cepstrum vector.
 *		- c'_i = sum_j { w(i,j) * bin_j }
 */
void
FX_SigProc_MelSpectrum2MFCC(hci_mfcc_t *pCepVec,		///< (o) cepstrum vector Q15.32
							hci_mfcc32 *melSpecPower,	///< (i) mel-freq. spectrum power Q15.32
							hci_mfcc16 *dctWgtMat,		///< (i) DCT cosine weight matrix Q15.16
							hci_int16 nNumCepstra,		///< (i) dimension of cepstrum vector
							hci_int16 nNumFilters,		///< (i) number of bands in filter banks
							hci_mfcc16 weightC0			///< (i) weight value for C0
);

/**
 *	convert FFT outputs into HTK-type mel-frequency power spectrum
 */
void
FX_SigProc_FFT2HTKMelSpectrum(hci_mfcc16 *xfft,			///< (i) : FFT output	(Q(sft).16)
							  hci_mfcc32 *melSpecPower,	///< (o) : mel-freq. spectrum power Q15.32
							  MFCC_UserData *pMfccData,	///< (o) : MFCC data
							  MfccParameters *pMfccVar,	///< (i) : front-end parameters
							  hci_int16 var_shift		///< (i) : left shift count
);

/**
 *	convert FFT outputs into ETSI-type mel-frequency power spectrum
 */
void
FX_SigProc_FFT2ETSIMelSpectrum(hci_mfcc16 *xfft,			///< (i) : FFT output	(Q(sft).16)
							   hci_mfcc32 *melSpecPower,	///< (o) : mel-freq. spectrum power Q15.32
//							   hci_mfcc32 *epdSpectrum,		///< (o) : mel-freq. spectrum for EPD Q15.32
							   MfccParameters *pMfccVar,	///< (i) : front-end parameters
							   hci_int16 var_shift			///< (i) : left shift count
);

/**
 *	convert FFT outputs into mel-frequency differential power spectrum
 */
void
FX_SigProc_FFT2MelDiffPowerSpectrum(hci_mfcc16 *xfft,			///< (i) : FFT output	(Q(sft).16)
									hci_mfcc32 *melSpecPower,	///< (o) : mel-freq. spectrum power Q15.32
//									hci_mfcc32 *epdSpectrum,	///< (o) : mel-freq. spectrum for EPD Q15.32
									MfccParameters *pMfccVar,	///< (i) : front-end parameters
									hci_int16 var_shift			///< (i) : left shift count
);

/**
 *	convert FFT outputs into mel-frequency predictive differential power spectrum
 */
void
FX_SigProc_FFT2MelPredictiveDiffPowerSpectrum(hci_mfcc16 *xfft,				///< (i) : FFT output	(Q(sft).16)
											  hci_mfcc32 *melSpecPower,		///< (o) : mel-freq. spectrum power Q15.32
//											  hci_mfcc32 *epdSpectrum,		///< (o) : mel-freq. spectrum for EPD Q15.32
											  MfccParameters *pMfccVar,		///< (i) : front-end parameters
											  hci_int16 var_shift			///< (i) : left shift count
);

/**
 *	pre-emphasize singal
 *		- c_n' = s_n - alpha * s_(n-1)
 */
void
FX_SigProc_preEmphasize(hci_mfcc16 *pSample,		///< (i/o) : input/output samples
						hci_mfcc16 *priorSample,	///< (i)   : prior sample
						const hci_mfcc16 preE,		///< (i)   : pre-emphasis alpha (Q15.16)
						hci_int16 nLenSample		///< (i)   : sample length
);

/**
 *	apply FFT to real-valued signal
 */
void
FX_SigProc_realFFT(hci_mfcc16 *xfft,		///< (i/o)	: input/output of FFT	(Q(sft).16)
				   hci_mfcc16 *phs_tbl,		///< (i)	: phase constant table (Q15.16)
				   hci_int16 nFFTSize,		///< (i)	: FFT point (N)
				   hci_int16 nHalfFFT,		///< (i)	: FFT point divided 2	(N/2)
				   hci_int16 nFFTStage		///< (i)	: number of stage in radix-2 FFT
);

/**
 *	compute log value of frame power
 */
hci_mfcc32
FX_SigProc_computeLogFramePower(hci_mfcc16 *pSample,		///< (i) : input samples	(Q0.16)
								hci_mfcc64 *blockPower,		///< (o) : power per block (Q0.64)
								hci_mfcc32 *logPower,		///< (o) : log pwer per block (Q15.32)
								hci_mfcc16 *frameEn,		///< (o) : frame power (Q0.16)
								hci_logadd_t *pLAddTbl,		///< (i) : log-addition table (Q15.32)
								hci_int16 nLenSample,		///< (i) : length of input samples
								hci_int16 nFrameId			///< (i) : frame index
);

/**
 *	compute log frame energy
 */
hci_mfcc32
FX_SigProc_computeLogFrameEnergy(MFCC_UserData *pMfccData,		///< (i/o) : MFCC data
								 MfccParameters *pMfccVar,		///< (i) : front-end parameters
								 hci_int16 var_shift			///< (i) : left shift count
);

/**
 *	compute spectral entropy
 */
hci_mfcc32
FX_SigProc_computeSpectralEntropy(MFCC_UserData *pMfccData,		///< (i/o) : MFCC data
								  MfccParameters *pMfccVar,		///< (i) : front-end parameters
								  const hci_int16 flagVAD,		///< (i) VAD flag (0 = unvoiced, 1 = voiced)
								  hci_int16 var_shift			///< (i) : left shift count
);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef __FX_SIGPROC_H__
