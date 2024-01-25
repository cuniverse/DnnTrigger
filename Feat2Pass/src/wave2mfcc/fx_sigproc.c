
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
 *	@file	fx_sigproc.c
 *	@ingroup wave2mfcc_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	signal processing library for MFCC computation
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "base/hci_type.h"
#include "base/hci_msg.h"
#include "base/hci_malloc.h"

#include "basic_op/fixpoint.h"
#include "basic_op/basic_op.h"
#include "wave2mfcc/fx_mfcc.h"
#include "wave2mfcc/fx_sigproc.h"

// local functions
#ifdef __cplusplus
extern "C" {
#endif 

/**
 *	 return mel-frequency corresponding to given FFT index
 */
HCILAB_PRIVATE hci_mfcc32
_FX_SigProc_MelFreq(hci_int32 nbin,			///< (i) FFT bin index
					hci_mfcc16 fres);		///< (i) frequency resolution

/**
 *	 convert original frequency into mel-frequency
 *		- Mel(f) = 1127 * log(1 + f/700)
 */
HCILAB_PRIVATE hci_mfcc32
_FX_SigProc_Origin2MelFreq(hci_int16 freq);		///< (i) frequency

/**
 *	 compute log frame energy
 */
HCILAB_PRIVATE hci_mfcc16
_FX_SigProc_computeLogPowerOfNewSamples(hci_mfcc64 *blockPower,		///< (i) frame log power per processing block
										hci_int16 nNumBlock);		///< (i) number of processing blocks

/**
 *  apply FFT to complex signal (radix-2 version)
 */
HCILAB_PRIVATE void
_FX_SigProc_FFT(hci_mfcc16 *xfft,		///< (i/o)	: input/output of FFT
				hci_mfcc16 *phs_tbl,	///< (i)	: phase constant table
				hci_int16 nFFTSize,		///< (i/o)	: FFT point
				hci_int16 nHalfFFT,		///< (i/o)	: FFT point divided 2
				hci_int16 stage);		///< (i)	: stage index

#ifdef __cplusplus
}
#endif


/**
 *	create buffer & constants necessary for Fast Fourier Transform 
 */
void
FX_SigProc_createFFTConstants(MfccParameters *pMfccVar)		///< (i/o) MFCC parameters
{
	hci_mfcc32 L_var_deg = 0;
	hci_int16 n = 0;
	hci_int16 var_tmp = 0;

#ifdef FIXED_POINT_FE
	hci_int32 L_var_const = 0;
	hci_int32 L_var_num = 0;
	hci_int16 sps_kHz = 0;
	hci_int16 var_div = 0;
	hci_int16 var_hi = 0;
	hci_int16 var_lo = 0;
	hci_int16 var_shift = 0;
	hci_int16 var_inv_n = 0;
#endif	// #ifdef FIXED_POINT_FE

	/*
	 *  conversion ...
	 *  
	 *	pMfccVar->fres = 1E7/(sampPeriod * pMfccVar->nFFTSize * 700.0)
	 *
	 *	where sampPeriod = 1E7/(pMfccVar->nSampleRate) and 
	 *		  pMfccVar->nSampleRate is sampling rate in H	
	 *
	 *	In fixed-point version, 
	 *	1. it follows an alternative approach
	 *	   due to the term, '1E7'; it's too big !!!
	 *
	 *	therefore, 
	 *	pMfccVar->fres = (pMfccVar->nSampleRate)/(pMfccVar->nFFTSize * 700)
	 *				   = (sps_kHz * 1000)/(pMfccVar->nFFTSize * 700)
	 *				   = (10/7) * (sps_kHz / pMfccVar->nFFTSize)
	 *				   = (L_var_const * sps_kHz) / pMfccVar->nFFTSize
	 *				   = (L_var_const * sps_kHz) >> X
	 *
	 *	where pMfccVar->nFFTSize = 2^X
	 *
	 *	2. 'pGpre->fres' is only used to access the value of
	 *	   'mel function'
	 */

#ifdef FIXED_POINT_FE

	var_shift = 0;
	for (n = 1; n < pMfccVar->nFFTSize; n *= 2) var_shift++;

	sps_kHz = pMfccVar->nSampleRate / 1000;
	L_var_const = 46811;									// Q15.32 (= 10/7)
	L_var_num = sps_kHz * L_var_const;						// Q15.32
	L_var_num += (pMfccVar->nFFTSize>>1);					// Q15.32 (+0.5)
	pMfccVar->fres = (hci_mfcc16) (L_var_num>>var_shift);	// Q15.16

#else	// !FIXED_POINT_FE

	pMfccVar->fres = (hci_mfcc16)pMfccVar->nSampleRate / (hci_mfcc16)(pMfccVar->nFFTSize*700);

#endif	// #ifdef FIXED_POINT_FE

	// number of stages in radix-2 FFT

	pMfccVar->nFFTStage = 0;
	var_tmp = 2;
	while(pMfccVar->nFFTSize > var_tmp) {
		var_tmp <<= 1;
		pMfccVar->nFFTStage += 1;
	}

	/*
	 *	[VERIFIED]
	 *
	 *		value : phs_tbl
	 *	max error : 0.000299
	 *	min error : 0.000001
	 *	avg error : 0.000093
	 */

	for(n = 0; n < pMfccVar->nHalfFFT; n++) {
#ifdef FIXED_POINT_FE
		var_inv_n = PowerASR_BasicOP_divide_16_16(n, pMfccVar->nHalfFFT);			// Q15.16
		L_var_deg = PowerASR_BasicOP_multiply_16_32(180, var_inv_n);				// Q16.32
		L_var_deg = PowerASR_BasicOP_shiftRight_32_32(L_var_deg, 1);				// Q15.32

		pMfccVar->phs_tbl[2*n] = PowerASR_BasicOP_fixedCosine_32_16(L_var_deg, 0);	// Q15.16
		//pMfccVar->phs_tbl[2*n+1] = PowerASR_BasicOP_negate_16_16(PowerASR_BasicOP_fixedCosine_32_16(L_var_deg, 1));	// Q15.16
		pMfccVar->phs_tbl[2*n+1] = PowerASR_BasicOP_fixedCosine_32_16(L_var_deg, 1);	// Q15.16
#else	// !FIXED_POINT_FE
		L_var_deg = (hci_mfcc32)(M_PI*n/pMfccVar->nHalfFFT);
		pMfccVar->phs_tbl[2*n] = (hci_mfcc16)cos((double)L_var_deg);
		//pMfccVar->phs_tbl[2*n+1] = -(hci_mfcc16)sin((double)L_var_deg);
		pMfccVar->phs_tbl[2*n+1] = (hci_mfcc16)sin((double)L_var_deg);
#endif	// #ifdef FIXED_POINT_FE
	}

}


/**
 *	build mel-frequency filter banks
 */
void
FX_SigProc_buildMelFilters(MfccParameters *pMfccVar)		///< (i/o) MFCC parameters
{
	hci_mfcc32 mlo = 0;				// lower mel-frequency Q15.32
	hci_mfcc32 mhi = 0;				// upper mel-frequency Q15.32
	hci_mfcc32 mbin = 0;			// mel-frequency mapped to bin index	Q15.32
	hci_mfcc32 mwidth = 0;			// mel-freq. width	Q15.32
	hci_mfcc32 nCentFreq[NUM_FILTERBANK+1];		// filter-bank center freq. Q15.32
	hci_mfcc32 L_var_num = 0;		// numerator in division operation
	hci_mfcc32 L_var_denom = 0;		// denominator in division operation
	hci_mfcc32 L_var_out = 0;		// output value
	hci_int16 chan = 0;				// channel id
	hci_int16 maxChan = 0;			// max. number of channels
	hci_int16 bin = 0;				// bin index
#ifdef FIXED_POINT_FE
	hci_int16 var_num = 0;
	hci_int16 var_denom = 0;
	hci_int16 mwidth_hi = 0;
	hci_int16 mwidth_lo = 0;
	hci_int16 chan_ratio = 0;
	hci_int32 L_var = 0;
#endif	// #ifdef FIXED_POINT_FE

	 /* 
	 *  conversion ...
	 *
	 *  pMfccVar->pi_factor = (float)(PI/(float)pMfccVar->nNumFilters)
	 *  
	 *	tmp16a : Q6/16bit <- 3.1415927
	 *	tmp16b : Q6/16bit <- 19
	 */

#ifdef FIXED_POINT_FE
	var_num = PowerASR_BasicOP_shiftRight_16_16(FIX_PI, 6);						// Q6.16
	var_denom = PowerASR_BasicOP_shiftLeft_16_16(pMfccVar->nNumFilters, 6);		// Q6.16
	pMfccVar->pi_factor = PowerASR_BasicOP_divide_16_16(var_num, var_denom);	// Q15.16
#else	// !FIXED_POINT_FE
	pMfccVar->pi_factor = (hci_mfcc16)(M_PI/pMfccVar->nNumFilters);
#endif	// #ifdef FIXED_POINT_FE

	/*
	 *  conversion ...
	 *  
	 *  pGpre->mfnorm = (float)sqrt(2.0/(float)pMfccVar->nNumFilters)
	 *
	 *  h_res : it's for higer resolution
	 *			Q30/32bit <- (2/pGpre->pOrder)	 
	 *  s_arg : it's for input argument of Q_sqrt.
	 *			q format of Q_sqrt argument must be odd and be twice 
	 *			in comparison to the value that evaluated.
	 *			ex) sqrt(4) = 2 -> Q_sqrt(8) = 2
	 */

#ifdef FIXED_POINT_FE
	L_var = (hci_int32)PowerASR_BasicOP_divide_16_16(2, pMfccVar->nNumFilters);		// Q15.32
	L_var = PowerASR_BasicOP_shiftLeft_32_32(L_var, 15);							// Q30.32
	pMfccVar->mfnorm = PowerASR_BasicOP_fixedSQRT_32_16(L_var);						// Q15.16
#else	// !FIXED_POINT_FE
	pMfccVar->mfnorm = (hci_mfcc16)sqrt(2.0/(double)pMfccVar->nNumFilters);
#endif	// #ifdef FIXED_POINT_FE

	/* set lo and hi pass cut offs if any */
	pMfccVar->nLowBin = 1;							// low pass filtering
	pMfccVar->nHighBin = pMfccVar->nHalfFFT - 1;	// high pass filtering
	
	mlo = 0;
	mhi = _FX_SigProc_MelFreq(pMfccVar->nHalfFFT+1, pMfccVar->fres);		// Q15.32

	if (pMfccVar->nLowerFiltFreq >= 0) {
		mlo = _FX_SigProc_Origin2MelFreq(pMfccVar->nLowerFiltFreq);		// Q15.32
#ifdef FIXED_POINT_FE
		L_var_num = (hci_int32)pMfccVar->nLowerFiltFreq * (hci_int32)pMfccVar->nFFTSize;
		L_var_denom = (hci_int32)pMfccVar->nSampleRate;
		L_var_out = PowerASR_BasicOP_divideShiftLeft_32_32(L_var_num, L_var_denom, 15);
		L_var_out += FLOAT2FIX32_ANY(1.5,15);
		pMfccVar->nLowBin = (hci_int16)PowerASR_BasicOP_shiftRight_32_32(L_var_out,15);
#else	// !FIXED_POINT_FE
		pMfccVar->nLowBin = (hci_int16) ((pMfccVar->nLowerFiltFreq * pMfccVar->nFFTSize / pMfccVar->nSampleRate) + 1.5);
#endif	// #ifdef FIXED_POINT_FE
		pMfccVar->nLowBin = HCI_MAX(pMfccVar->nLowBin, 1);
	}

	if (pMfccVar->nUpperFiltFreq >= 0) {
		mhi = _FX_SigProc_Origin2MelFreq(pMfccVar->nUpperFiltFreq);		// Q15.32
#ifdef FIXED_POINT_FE
		L_var_num = (hci_int32)pMfccVar->nUpperFiltFreq * (hci_int32)pMfccVar->nFFTSize;
		L_var_denom = (hci_int32)pMfccVar->nSampleRate;
		L_var_out = PowerASR_BasicOP_divideShiftLeft_32_32(L_var_num, L_var_denom, 15);
		L_var_out -= FLOAT2FIX32_ANY(0.5,15);
		pMfccVar->nHighBin = (hci_int16)PowerASR_BasicOP_shiftRight_32_32(L_var_out,15);
#else	// !FIXED_POINT_FE
		pMfccVar->nHighBin = (hci_int16) ((pMfccVar->nUpperFiltFreq * pMfccVar->nFFTSize / pMfccVar->nSampleRate) - 0.5);
#endif	// #ifdef FIXED_POINT_FE
		pMfccVar->nHighBin = HCI_MIN(pMfccVar->nHighBin, pMfccVar->nHalfFFT-1);
	}

	/* Create vector of filter-bank center frequencies */
	memset(nCentFreq, 0, sizeof(nCentFreq));
	mwidth = mhi - mlo;									// Q15.32
#ifdef FIXED_POINT_FE
	PowerASR_BasicOP_separateBits_32_16(mwidth, &mwidth_hi, &mwidth_lo);
#endif // #ifdef FIXED_POINT_FE
	maxChan = pMfccVar->nNumFilters + 1;
	for (chan = 0; chan < maxChan; chan++) {
#ifdef FIXED_POINT_FE
		chan_ratio = PowerASR_BasicOP_divide_16_16((hci_int16)(chan+1), maxChan);	// Q15.16
		nCentFreq[chan] = PowerASR_BasicOP_multiply_32_16_32(mwidth_hi, mwidth_lo, chan_ratio);	// Q15.32
		nCentFreq[chan] += mlo;
#else	// !FIXED_POINT_FE
		nCentFreq[chan] = ((hci_mfcc32)(chan+1)/(hci_mfcc32)maxChan) * mwidth + mlo;
#endif	// #ifdef FIXED_POINT_FE
	}

	/* Create loChan map, loChan[fftindex] -> lower channel index */
	for (bin = 0, chan = 0; bin < pMfccVar->nHalfFFT; bin++) {
		mbin = _FX_SigProc_MelFreq(bin+1, pMfccVar->fres);		// Q15.32
		if (bin < pMfccVar->nLowBin || bin > pMfccVar->nHighBin) {
			pMfccVar->lowerChan[bin] = -1;
		}
		else {
			while (chan < maxChan && nCentFreq[chan] < mbin) ++chan;
			pMfccVar->lowerChan[bin] = chan - 1;
		}
	}

	/* Create vector of lower channel weights */   
	for (bin = 0; bin < pMfccVar->nHalfFFT; bin++) {
		chan = pMfccVar->lowerChan[bin];
		if (bin < pMfccVar->nLowBin || bin > pMfccVar->nHighBin) {
			pMfccVar->lowerWeight[bin] = 0;
		}
		else {
			if (chan >= 0) {
				L_var_num = nCentFreq[chan+1] - _FX_SigProc_MelFreq(bin+1,pMfccVar->fres);			// Q15.32
				L_var_denom = nCentFreq[chan+1] - nCentFreq[chan];									// Q15.32
			}
			else {
				L_var_num = nCentFreq[0] - _FX_SigProc_MelFreq(bin+1,pMfccVar->fres);				// Q15.32
				L_var_denom = nCentFreq[0] - mlo;													// Q15.32
			}
#ifdef FIXED_POINT_FE
			L_var = PowerASR_BasicOP_divideShiftLeft_32_32(L_var_num, L_var_denom, 15);			// Q15.32
			pMfccVar->lowerWeight[bin] = PowerASR_BasicOP_extractLSB_32_16(L_var);				// Q15.16
#else	// !FIXED_POINT_FE
			pMfccVar->lowerWeight[bin] = L_var_num / L_var_denom;
#endif	// #ifdef FIXED_POINT_FE
		}
	}

}


/**
 *	create weight vector for cepstral weighting (liftering)
 *		- w_n  = 1 + (L/2) * sin(n*PI/L);
 */
void
FX_SigProc_createCepstralWindow(hci_mfcc16 *pCepWin,		///< (o) cepstrum weight vector
								hci_int16 nCepLiftOrder,	///< (i) cepstrum liftering order
								hci_int16 nNumCepstra)		///< (i) cepstrum dimension
{
	hci_mfcc16 cnst = 0;
	hci_mfcc16 Lby2 = 0;
	hci_int16 i = 0;

#ifdef FIXED_POINT_FE

	hci_int16 cst_q4_16 = 0;
	hci_int16 cst_q11_16 = 0;
	hci_int16 val_sin = 0;
	hci_int16 tmp16 = 0;
	hci_int32 L_val_deg = 0;

	/*
	 *  conversion ...
	 *   i) pGpre->cepWin[i] = 1.0 + Lby2*sin(i * a)
	 *  
	 *		cst_q15_32 : Q15/32bit <- 1.0
	 *			  Lby2 : Q0/16bit 
	 *  
	 *  ii) sin(i * a)
	 *
	 *      sine argument conversion from radian to degree
	 *		i * a = (PI/cepLiftering) * i
	 *			  = (PI/180) * (180/cepLiftering) * i
	 *
	 *		(180/cepLiftering) * i	<- angle in degree
	 *		cnst = (1/cepLiftering) : Q14/16bit 
	 *
	 *	[VERIFIED]
	 *	
	 *			value : pGpre->cepWin
	 *		max error : 0.003274
	 *		min error : 0.000001
	 *		avg error : 0.001136
	 *	 
  	 */

	if (0 == nCepLiftOrder) {
		return;
	}

	cst_q4_16  = 16;													/*  Q4/16bit <- 1   */
	cst_q11_16 = 2048;													/* Q11/32bit <- 1   */

	Lby2  = PowerASR_BasicOP_shiftLeft_16_16(nCepLiftOrder, 10);		/* = L/2, Q11/16bit */
	cnst = PowerASR_BasicOP_divide_16_16(cst_q4_16, nCepLiftOrder);		/* = 1/L, Q19/16bit */

	for (i = 0; i < nNumCepstra; i++) {
		tmp16 = (hci_int16)(i+1)*180;
		L_val_deg = PowerASR_BasicOP_multiply_16_32(cnst, tmp16);		/* Q20/32bit */
		L_val_deg = PowerASR_BasicOP_shiftRight_32_32(L_val_deg, 5);	/* Q15/32bit */
		val_sin = PowerASR_BasicOP_fixedCosine_32_16(L_val_deg, 1);		/* Q15/16bit */
		tmp16 = PowerASR_BasicOP_multiply_16_16(Lby2, val_sin);			/* Q11/16bit */
		pCepWin[i] = PowerASR_BasicOP_add_16_16(cst_q11_16, tmp16);		/* Q11/16bit */
	}

#else	// !FIXED_POINT_FE

	if (0 == nCepLiftOrder) {
		return;
	}

	cnst = (hci_mfcc16)M_PI/nCepLiftOrder;
	Lby2 = (hci_mfcc16)nCepLiftOrder/2.0f;

	for (i = 0;i < nNumCepstra; i++) {
		pCepWin[i] = (hci_mfcc16)(1.0 + Lby2*sin((double)(i+1) * cnst));
	}

#endif	// #ifdef FIXED_POINT_FE

}


/**
 *	cepstral weighting (liftering)
 *		- c_n' = w_n * c_n
 */
void
FX_SigProc_weightCepstrum(hci_mfcc_t *pCepVec,		///< (i/o) input/output cepstrum vector
						  hci_mfcc16 *pCepWin,		///< (i)   cepstral window
						  hci_int16 nCepDim)		///< (i)   dimension of cepstrum vector
{
	hci_mfcc_t *pMfcc = 0;
	hci_mfcc_t *pLastMfcc = 0;
	hci_mfcc16 *pWeight = 0;

#ifdef FIXED_POINT_FE

	hci_int64 L_var = 0;

	pMfcc = pCepVec;
	pLastMfcc = pMfcc + nCepDim;
	pWeight = pCepWin;
	while (pMfcc < pLastMfcc) {
		L_var = (hci_int64)(*pMfcc) * (hci_int64)(*pWeight);	// Q26.64
		*pMfcc = (hci_int32)(L_var>>11);						// Q15.32
		pMfcc++; pWeight++;
// 		L_var = (hci_int64)(*pMfcc) * (hci_int64)(*pWeight);	// Q26.64
// 		*pMfcc = (hci_int32)(L_var>>11);						// Q15.32
// 		pMfcc++; pWeight++;
	}

#else	// !FIXED_POINT_FE

	pMfcc = pCepVec;
	pLastMfcc = pMfcc + nCepDim;
	pWeight = pCepWin;
	while (pMfcc < pLastMfcc) {
		*pMfcc++ *= (*pWeight++);
// 		*pMfcc++ *= (*pWeight++);
	}

#endif	// #ifdef FIXED_POINT_FE

}


/**
 *	create sine filter for use in predictive differential power spectrum based MFCCs
 *		- h_i = sin(i * 0.5 * PI / W),  0 <= i <= W, (typically W = 6)
 */
void
FX_SigProc_createSineFilter(MfccParameters *pMfccVar)	///< (i) MFCC parameters
{
	hci_mfcc16 cnst = 0;
	hci_int16 i = 0;

#ifdef FIXED_POINT_FE

	hci_int16 tmp16 = 0;
	hci_int32 L_val_deg = 0;

	pMfccVar->nWidthSineFilter = WIDTH_SINE_FILTER;
	cnst = PowerASR_BasicOP_divide_16_16(1, pMfccVar->nWidthSineFilter);				/* = 1/W, Q15/16bit */

	for (i = 0;i < pMfccVar->nWidthSineFilter; i++) {
		tmp16 = (hci_int16)(i+1)*90;
		L_val_deg = PowerASR_BasicOP_multiply_16_32(cnst, tmp16);						/* = 180*i/(2*W), Q16/32bit */
		L_val_deg = PowerASR_BasicOP_shiftRight_32_32(L_val_deg, 1);					/* = 180*i/(2*W), Q15/32bit */
		pMfccVar->sineFilter[i] = PowerASR_BasicOP_fixedCosine_32_16(L_val_deg, 1);		/* = sin(0.5*PI*i/W), Q15/16bit */
	}

#else	// !FIXED_POINT_FE

	pMfccVar->nWidthSineFilter = WIDTH_SINE_FILTER;
	cnst = (hci_mfcc16)M_PI/(hci_mfcc16)pMfccVar->nWidthSineFilter;
	cnst /= 2.0f;

	for (i = 0;i < pMfccVar->nWidthSineFilter; i++) {
		pMfccVar->sineFilter[i] = (hci_mfcc16)sin((double)(i+1) * (double)cnst);
	}

#endif	// #ifdef FIXED_POINT_FE

}


/**
 *	pre-emphasize singal
 *		- c_n' = s_n - alpha * s_(n-1)
 */
void
FX_SigProc_preEmphasize(hci_mfcc16 *pSample,		///< (i/o) : input/output samples
						hci_mfcc16 *priorSample,	///< (i)   : prior sample
						const hci_mfcc16 preE,		///< (i)   : pre-emphasis alpha (Q15.16)
						hci_int16 nLenSample)		///< (i)   : sample length
{
	hci_mfcc16 *pRaw = 0;
	hci_mfcc16 *pFirstRaw = 0;
	hci_mfcc16 new_prior = 0;
	hci_mfcc16 old_prior = (*priorSample);

#ifdef FIXED_POINT_FE
	hci_int16 tmp16 = 0;
	hci_int32 L_var = 0;

	new_prior = pSample[nLenSample-1];

	pRaw = pSample + nLenSample - 1;
	pFirstRaw = pSample;
	while (pRaw > pFirstRaw+3) {
		L_var = PowerASR_BasicOP_multiply_16_32(preE, *(pRaw-1));			// Q16.32
		tmp16 = PowerASR_BasicOP_round_32_16(L_var);						// Q0.16
		*pRaw = PowerASR_BasicOP_subtract_16_16(*pRaw, tmp16);				// Q0.16
		pRaw--;
		L_var = PowerASR_BasicOP_multiply_16_32(preE, *(pRaw-1));			// Q16.32
		tmp16 = PowerASR_BasicOP_round_32_16(L_var);						// Q0.16
		*pRaw = PowerASR_BasicOP_subtract_16_16(*pRaw, tmp16);				// Q0.16
		pRaw--;
		L_var = PowerASR_BasicOP_multiply_16_32(preE, *(pRaw-1));			// Q16.32
		tmp16 = PowerASR_BasicOP_round_32_16(L_var);						// Q0.16
		*pRaw = PowerASR_BasicOP_subtract_16_16(*pRaw, tmp16);				// Q0.16
		pRaw--;
		L_var = PowerASR_BasicOP_multiply_16_32(preE, *(pRaw-1));			// Q16.32
		tmp16 = PowerASR_BasicOP_round_32_16(L_var);						// Q0.16
		*pRaw = PowerASR_BasicOP_subtract_16_16(*pRaw, tmp16);				// Q0.16
		pRaw--;
	}
	while (pRaw > pFirstRaw) {
		L_var = PowerASR_BasicOP_multiply_16_32(preE, *(pRaw-1));			// Q16.32
		tmp16 = PowerASR_BasicOP_round_32_16(L_var);						// Q0.16
		*pRaw = PowerASR_BasicOP_subtract_16_16(*pRaw, tmp16);				// Q0.16
		pRaw--;
	}
	L_var = PowerASR_BasicOP_multiply_16_32(preE, old_prior);			// Q16.32
	tmp16 = PowerASR_BasicOP_round_32_16(L_var);						// Q0.16
	*pRaw = PowerASR_BasicOP_subtract_16_16(*pRaw, tmp16);				// Q0.16

#else	// !FIXED_POINT_FE

	new_prior = pSample[nLenSample-1];

	pRaw = pSample + nLenSample - 1;
	pFirstRaw = pSample;
	while (pRaw > pFirstRaw) {
		*pRaw -= preE * (*(pRaw-1));
		pRaw--;
	}
	*pRaw -= preE * old_prior;

#endif	// #ifdef FIXED_POINT_FE

	*priorSample = new_prior;

}


/**
 *	create cosine matrix table for DCT (Q.16)
 *		- w(i,j)  = sqrt(2/N) * cos(i * PI * (j-0.5) / N),
 *			where, i=0, ..., P-1, j=0, ..., N-1 
 *					P = cepstrum order, 
 *					N = number of bands in mel filter-bank
 */
void
FX_SigProc_createDCTCosineTable(MfccParameters *pMfccVar)	///< (i) MFCC parameters
{
	hci_int16 i = 0;
	hci_int16 j = 0;
	hci_int16 numChan = pMfccVar->nNumFilters;
	hci_int16 dimCep = pMfccVar->nNumCepstra;
	hci_mfcc16 mfnorm = pMfccVar->mfnorm;	// Q15.16
	hci_mfcc16 pi_factor = pMfccVar->pi_factor;	// Q15.16
	hci_mfcc16 *pWgtMat = pMfccVar->dctCosTable; // Q15.16
	hci_mfcc32 cos_factor = 0;

#ifdef FIXED_POINT_FE
	hci_mfcc32 L_val_deg = 0;
	hci_mfcc32 L_tmp32 = 0;
	hci_mfcc16 val_cos = 0;
	hci_mfcc16 tmp16 = 0;

	/*
	 *  conversion ...
	 *   i) cepWgt[i][j] = b * cos(PI * i * (j-0.5) / N)
	 *  
	 *		cst_q15_32 : Q15/32bit <- 1.0
	 *			  Lby2 : Q0/16bit 
	 *  
	 *  ii) cos(PI * i * (j-0.5) / N)
	 *
	 *      cosine argument conversion from radian to degree
	 *		PI * i * (j-0.5) / N = (PI/N) * i * (j-0.5)
	 *							 = (PI/180) * (180/N) * i * (j-0.5)
	 *
	 *		(180/N) * i	<- angle in degree
	 *
	 *	 
  	 */

	if (pMfccVar->nMFCCType == ETSI_MFCC) {
		for (i = 0; i < dimCep; i++) {
			L_tmp32 = 180 * (i+1);
			cos_factor = PowerASR_BasicOP_divideShiftLeft_32_32(L_tmp32, (hci_int32)numChan, 16);
			for (j = 0; j < numChan; j++, pWgtMat++) {
				L_val_deg = ((2*j+1) * cos_factor + 2)>>2;						// Q15.32
				*pWgtMat = PowerASR_BasicOP_fixedCosine_32_16(L_val_deg, 0);	// Q15.16
			}
		}
	}
	else {
		for (i = 0; i < dimCep; i++) {
			L_tmp32 = 180 * (i+1);
			cos_factor = PowerASR_BasicOP_divideShiftLeft_32_32(L_tmp32, (hci_int32)numChan, 16);
			for (j = 0; j < numChan; j++, pWgtMat++) {
				L_val_deg = ((2*j+1) * cos_factor + 2)>>2;						// Q15.32
				val_cos = PowerASR_BasicOP_fixedCosine_32_16(L_val_deg, 0);		// Q15.16
				L_tmp32 = PowerASR_BasicOP_multiply_16_32(mfnorm, val_cos);		// Q31.32
				*pWgtMat = PowerASR_BasicOP_round_32_16(L_tmp32);				// Q15.16
			}
		}
	}

#else	// !FIXED_POINT_FE

	if (pMfccVar->nMFCCType == ETSI_MFCC) {
		for (i = 0; i < dimCep; i++) {
			cos_factor = (hci_mfcc32)(i+1) * pi_factor;
			for (j = 0; j < numChan; j++, pWgtMat++) {
				*pWgtMat = (hci_mfcc16)cos((double)(cos_factor*(j+0.5)));
			}
		}
	}
	else {
		for (i = 0; i < dimCep; i++) {
			cos_factor = (hci_mfcc32)(i+1) * pi_factor;
			for (j = 0; j < numChan; j++, pWgtMat++) {
				*pWgtMat = mfnorm * (hci_mfcc16)cos((double)(cos_factor*(j+0.5)));
			}
		}
	}
	
#endif	// #ifdef FIXED_POINT_FE

}


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
							hci_mfcc16 weightC0)		///< (i) weight value for C0
{
	hci_mfcc_t *pOut = 0;
	hci_mfcc_t *pLastOut = 0;
	hci_mfcc32 *pMSP = 0;
	hci_mfcc32 *pLastMSP = 0;
	hci_mfcc16 *pDCTWgt = 0;
#ifdef FIXED_POINT_FE
	hci_int64 nCepValue = 0;
#endif	// #ifdef FIXED_POINT_FE

	pOut = pCepVec;
	pLastOut = pCepVec + nNumCepstra;
	pLastMSP = melSpecPower + nNumFilters;
	pDCTWgt = dctWgtMat;

#ifdef FIXED_POINT_FE
	while (pOut < pLastOut) {
		nCepValue = 0;
		pMSP = melSpecPower;
		while (pMSP < pLastMSP) {
			nCepValue += (hci_int64)(*pMSP++) * (hci_int64)(*pDCTWgt++);	// Q30.64
		}
		*pOut = (hci_mfcc_t) (nCepValue>>15);		// Q15.32
		pOut++;
	}
	if (weightC0 == 0x7FFF) {
		hci_mfcc_t nLogEnergy = 0;
		pMSP = melSpecPower;
		while (pMSP < pLastMSP) {
			nLogEnergy += *pMSP++;
		}
		nLogEnergy = PowerASR_BasicOP_divideShiftLeft_32_32(nLogEnergy, (hci_int32)nNumFilters, 0);
		*pOut = nLogEnergy;
	}
	else if (weightC0 > 0) {
		hci_mfcc_t nLogEnergy = 0;
		hci_int16 hi_val = 0, lo_val = 0;
		hci_mfcc16 val_complem = 0;
		pMSP = melSpecPower;
		while (pMSP < pLastMSP) {
			nLogEnergy += *pMSP++;
		}
		val_complem = 0x7FFF - weightC0;
		nLogEnergy = PowerASR_BasicOP_divideShiftLeft_32_32(nLogEnergy, (hci_int32)nNumFilters, 0);
		PowerASR_BasicOP_separateBits_32_16(*pOut, &hi_val, &lo_val);
		*pOut = PowerASR_BasicOP_multiply_32_16_32(hi_val, lo_val, val_complem);
		PowerASR_BasicOP_separateBits_32_16(nLogEnergy, &hi_val, &lo_val);
		*pOut += PowerASR_BasicOP_multiply_32_16_32(hi_val, lo_val, weightC0);
	}
#else	// !FIXED_POINT_FE
	while (pOut < pLastOut) {
		*pOut = 0.0f;
		pMSP = melSpecPower;
		while (pMSP < pLastMSP) {
			*pOut += (*pMSP++) * (*pDCTWgt++);
		}
		pOut++;
	}
	if (weightC0 > 0.0f) {
		hci_mfcc_t nLogEnergy = 0;
		pMSP = melSpecPower;
		while (pMSP < pLastMSP) {
			nLogEnergy += *pMSP++;
		}
		nLogEnergy /= (float)nNumFilters;
		*pOut *= (1.0f - weightC0);
		*pOut += weightC0 * nLogEnergy;
	}
#endif	// #ifdef FIXED_POINT_FE

}


#define SQ_INT16_MAX	(32767*32767)

/**
 *	convert FFT outputs into HTK-type mel-frequency power spectrum
 */
void
FX_SigProc_FFT2HTKMelSpectrum(hci_mfcc16 *xfft,			///< (i) : FFT output	(Q(sft).16)
							  hci_mfcc32 *melSpecPower,	///< (o) : mel-freq. spectrum power Q15.32
							  MFCC_UserData *pMfccData,	///< (o) : MFCC data
							  MfccParameters *pMfccVar,	///< (i) : front-end parameters
							  hci_int16 var_shift)		///< (i) : left shift count
{
	hci_mfcc32 *subbandFrameEng = 0;
	hci_mfcc16 *pOutFFT = 0;
	hci_int16 *pChan = 0;
	hci_int16 *pLastChan = 0;
	hci_mfcc16 *pLowWgt = 0;
	hci_mfcc32 *pMelSpec = 0;
	hci_mfcc32 *pLastSpec = 0;
	hci_int16 chan = 0;
	hci_int16 iShift = 0;
	hci_mfcc16 t1 = 0;
	hci_mfcc16 t2 = 0;
	hci_mfcc16 ek = 0;
	hci_mfcc16 ek_compl = 0;
	hci_mfcc32 L_ek = 0;
	hci_mfcc32 L_ek_compl = 0;
	hci_mfcc32 melfloor = 0;
#ifdef FIXED_POINT_FE
	hci_int16 nshifts = 0;
	hci_int16 nSpecShift = 0;
	hci_int32 L_tmp32 = 0;
#endif	// #ifdef FIXED_POINT_FE

	pOutFFT = xfft + 2 * pMfccVar->nLowBin;
	pChan = pMfccVar->lowerChan + pMfccVar->nLowBin;
	pLastChan = pMfccVar->lowerChan + pMfccVar->nHighBin;
	pLowWgt = pMfccVar->lowerWeight + pMfccVar->nLowBin;
	pLastSpec = melSpecPower + pMfccVar->nNumFilters;
	subbandFrameEng = pMfccData->subbandFrameEng + pMfccVar->nLowBin;

#ifdef FIXED_POINT_FE

	if (pMfccVar->nSampleRate == 16000) {
		nSpecShift = 7;
	}
	else {	// 8kHz
		nSpecShift = 6;
	}

	/* Fill filter-bank channels */
	memset(melSpecPower, 0, pMfccVar->nNumFilters*sizeof(hci_mfcc32));
	while (pChan <= pLastChan) {
		t1 = *pOutFFT++;			// Q(sft)/16
		t2 = *pOutFFT++;			// Q(sft)/16

		chan = *pChan;

		/*
		 *	conversion ...   
		 *
		 *	ek = (float)sqrt(t1*t1 + t2*t2)		 
		 */

		L_tmp32 = t1 * t1;						// Q(2*sft)/32
		L_tmp32 += (hci_mfcc32)(t2 * t2);		// Q(2*sft)/32

		*subbandFrameEng++ = PowerASR_BasicOP_shiftRight_32_32(L_tmp32, 8);	// Q(2*sft-8).32

		if (L_tmp32 <= SQ_INT16_MAX) {
			nshifts = 0;
			ek = PowerASR_BasicOP_fixedSQRT_32_16(L_tmp32);		// Q(sft)/16
			L_ek = (hci_mfcc32)ek;
		}
		else {
			nshifts = 0;
			while (L_tmp32 < SQ_INT16_MAX) {
				L_tmp32 >>= 2;
				nshifts++;
			}
			ek = PowerASR_BasicOP_fixedSQRT_32_16(L_tmp32);		// Q(sft)/16
			L_ek = (hci_mfcc32)ek;
			L_ek <<= nshifts;
		}

		L_tmp32 = PowerASR_BasicOP_multiply_16_32(ek, *pLowWgt);	// Q(sft+16)/32
		L_tmp32 = PowerASR_BasicOP_shiftRight_32_32(L_tmp32,(hci_int16)(16-nshifts));	// Q(sft)/32

		L_ek_compl = L_ek - L_tmp32;								// Q(sft)/32
		L_ek -= L_ek_compl;											// Q(sft)/32

		if(chan >= 0) {
			melSpecPower[chan] += L_ek;					// Q(sft)/32
		}

		if (chan < (pMfccVar->nNumFilters-1)) {
			melSpecPower[chan+1] += L_ek_compl;			// Q(sft)/32
		}

		pChan++; pLowWgt++;
	}

	iShift = 2*var_shift - nSpecShift;

	/* Take logs for mel spectral magnitude */
	melfloor = PowerASR_BasicOP_shiftLeft_32_32(1,var_shift);
	pMelSpec = melSpecPower;
	pLastSpec = pMelSpec + pMfccVar->nNumFilters;
	while (pMelSpec < pLastSpec) {
		L_tmp32 = *pMelSpec;					// Q(sft)/32
		if (L_tmp32 < melfloor) {
			*pMelSpec = 0;
		}
		else {
			*pMelSpec = PowerASR_BasicOP_fixedLOG_32_32(L_tmp32,var_shift);		// Q15.32
		}
		pMelSpec++;
	}

#else	// !FIXED_POINT_FE

	/* Fill filter-bank channels */
	memset(melSpecPower, 0, pMfccVar->nNumFilters*sizeof(hci_mfcc32));
	while (pChan <= pLastChan) {
		t1 = *pOutFFT++;
		t2 = *pOutFFT++;
		L_ek = t1*t1 + t2*t2;
		*subbandFrameEng++ = L_ek;
		ek = (hci_mfcc32) sqrt(L_ek);
		ek_compl = ek - (*pLowWgt) * ek;
		ek -= ek_compl;
		chan = *pChan;
		if (chan >= 0) {
			melSpecPower[chan] += ek;
		}
		if (chan < (pMfccVar->nNumFilters-1)) {
			melSpecPower[chan+1] += ek_compl;
		}
		pChan++; pLowWgt++;
	}

	/* Take logs */
	melfloor = 1.0f;
	pMelSpec = melSpecPower;
	while (pMelSpec < pLastSpec) {
		if (*pMelSpec < melfloor) {
			*pMelSpec = 0.0f;
		}
		else {
			*pMelSpec = (hci_mfcc32) log(*pMelSpec);
		}
		pMelSpec++;
	}
#endif	// #ifdef FIXED_POINT_FE

}


/**
 *	convert FFT outputs into ETSI-type mel-frequency power spectrum
 */
void
FX_SigProc_FFT2ETSIMelSpectrum(hci_mfcc16 *xfft,			///< (i) : FFT output	(Q(sft).16)
							   hci_mfcc32 *melSpecPower,	///< (o) : mel-freq. spectrum power Q15.32
							   MfccParameters *pMfccVar,	///< (i) : front-end parameters
							   hci_int16 var_shift)			///< (i) : left shift count
{
	hci_mfcc16 *pOutFFT = 0;
	hci_int16 *pChan = 0;
	hci_int16 *pLastChan = 0;
	hci_mfcc16 *pLowWgt = 0;
	hci_mfcc32 *pMelSpec = 0;
	hci_mfcc32 *pLastSpec = 0;
	hci_int16 chan = 0;
	hci_int16 iShift = 0;
	hci_mfcc16 t1 = 0;
	hci_mfcc16 t2 = 0;
	hci_mfcc32 L_ek = 0;
	hci_mfcc32 L_ek_compl = 0;
	hci_mfcc32 melfloor = 0;
#ifdef FIXED_POINT_FE
	hci_int16 val_hi = 0;
	hci_int16 val_lo = 0;
	hci_mfcc16 ek = 0;
	hci_mfcc16 ek_compl = 0;
	hci_int32 L_tmp32 = 0;
	hci_int32 L_sq = 0;
	hci_int32 L_sq_compl = 0;
	hci_int64 L_tmp64 = 0;
#endif	// #ifdef FIXED_POINT_FE

	pOutFFT = xfft + 2 * pMfccVar->nLowBin;
	pChan = pMfccVar->lowerChan + pMfccVar->nLowBin;
	pLastChan = pMfccVar->lowerChan + pMfccVar->nHighBin;
	pLowWgt = pMfccVar->lowerWeight + pMfccVar->nLowBin;
	pLastSpec = melSpecPower + pMfccVar->nNumFilters;

#ifdef FIXED_POINT_FE

	/* Fill filter-bank channels */
	memset(melSpecPower, 0, pMfccVar->nNumFilters*sizeof(hci_mfcc32));
	while (pChan <= pLastChan) {
		t1 = *pOutFFT++;			// Q(sft)/16
		t2 = *pOutFFT++;			// Q(sft)/16

		chan = *pChan;

		/*
		 *	conversion ...   
		 *
		 *	ek = (float)sqrt(t1*t1 + t2*t2)		 
		 */

		L_tmp32 = t1 * t1;						// Q(2*sft)/32
		L_tmp32 += (hci_mfcc32)(t2 * t2);		// Q(2*sft)/32

		L_sq = L_tmp32 >> 6;					// Q(2*sft-6)/32
		L_tmp64 = (hci_int64) L_sq * (hci_int64)(*pLowWgt);	// Q(2*sft-6+15)/32
		L_tmp64 += (1>>14);
		L_sq_compl = L_sq - (hci_int32)(L_tmp64 >> 15);		// Q(2*sft-6)/32
		L_sq -= L_sq_compl;									// Q(2*sft-6)/32

		L_ek = L_tmp32;
		PowerASR_BasicOP_separateBits_32_16(L_ek, &val_hi, &val_lo);
		L_ek_compl = L_ek - PowerASR_BasicOP_multiply_32_16_32(val_hi, val_lo, *pLowWgt);	// Q(2*sft)/32
		L_ek -= L_ek_compl;																	// Q(2*sft)/32

		if(chan >= 0) {
			//melSpecPower[chan] += PowerASR_BasicOP_shiftRight_32_32(L_ek, 6);				// Q(2*sft-6)/32
			melSpecPower[chan] += L_sq;
		}

		if (chan < (pMfccVar->nNumFilters-1)) {
			//melSpecPower[chan+1] += PowerASR_BasicOP_shiftRight_32_32(L_ek_compl, 6);		// Q(2*sft-6)/32
			melSpecPower[chan+1] += L_sq_compl;
		}

		pChan++; pLowWgt++;
	}

	iShift = 2*var_shift - 6;

	/* Take logs for mel spectral magnitude */
	melfloor = PowerASR_BasicOP_shiftLeft_32_32(1,iShift);
	pMelSpec = melSpecPower;
	pLastSpec = pMelSpec + pMfccVar->nNumFilters;
	while (pMelSpec < pLastSpec) {
		L_tmp32 = *pMelSpec;					// Q(2*sft-6)/32
		if (L_tmp32 < melfloor) {
			*pMelSpec = 0;
		}
		else {
			*pMelSpec = PowerASR_BasicOP_fixedLOG_32_32(L_tmp32,iShift);		// Q15.32
		}
		pMelSpec++;
	}

#else	// !FIXED_POINT_FE

	/* Fill filter-bank channels */
	memset(melSpecPower, 0, pMfccVar->nNumFilters*sizeof(hci_mfcc32));
	while (pChan <= pLastChan) {
		t1 = *pOutFFT++;
		t2 = *pOutFFT++;
		L_ek = t1*t1 + t2*t2;
		L_ek_compl = L_ek - (*pLowWgt) * L_ek;
		L_ek -= L_ek_compl;
		chan = *pChan;
		if (chan >= 0) {
			melSpecPower[chan] += L_ek;
		}
		if (chan < (pMfccVar->nNumFilters-1)) {
			melSpecPower[chan+1] += L_ek_compl;
		}
		pChan++; pLowWgt++;
	}

	/* Take logs */
	melfloor = 1.0f;
	pMelSpec = melSpecPower;
	while (pMelSpec < pLastSpec) {
		if (*pMelSpec < melfloor) {
			*pMelSpec = 0.0f;
		}
		else {
			*pMelSpec = (hci_mfcc32) log(*pMelSpec);
		}
		pMelSpec++;
	}

#endif	// #ifdef FIXED_POINT_FE

}


/**
 *	convert FFT outputs into mel-frequency differential power spectrum
 */
void
FX_SigProc_FFT2MelDiffPowerSpectrum(hci_mfcc16 *xfft,			///< (i) : FFT output	(Q(sft).16)
									hci_mfcc32 *melSpecPower,	///< (o) : mel-freq. spectrum power Q15.32
									MfccParameters *pMfccVar,	///< (i) : front-end parameters
									hci_int16 var_shift)		///< (i) : left shift count
{
	hci_mfcc16 *pOutFFT = 0;
	hci_int16 *pChan = 0;
	hci_int16 *pLastChan = 0;
	hci_mfcc16 *pLowWgt = 0;
	hci_mfcc32 *pMelSpec = 0;
	hci_mfcc32 *pLastSpec = 0;
	hci_mfcc32 prevPowerSpec = 0;
	hci_int16 chan = 0;
	hci_int16 next_chan = 0;
	hci_int16 iShift = 0;
	hci_mfcc16 t1 = 0;
	hci_mfcc16 t2 = 0;
	hci_mfcc16 ek = 0;
	hci_mfcc16 ek_compl = 0;
	hci_mfcc32 L_ek = 0;
	hci_mfcc32 L_ek_compl = 0;
	hci_mfcc32 melfloor = 0;
	hci_mfcc32 L_val_DPS = 0;
#ifdef FIXED_POINT_FE
	hci_int16 val_hi = 0;
	hci_int16 val_lo = 0;
	hci_int32 L_tmp32 = 0;
	hci_int32 L_sq = 0;
	hci_int32 L_sq_compl = 0;
	hci_int64 L_tmp64 = 0;
#endif	// #ifdef FIXED_POINT_FE

	pOutFFT = xfft + 2 * pMfccVar->nLowBin;
	pChan = pMfccVar->lowerChan + pMfccVar->nLowBin;
	pLastChan = pMfccVar->lowerChan + pMfccVar->nHighBin;
	pLowWgt = pMfccVar->lowerWeight + pMfccVar->nLowBin;
	pLastSpec = melSpecPower + pMfccVar->nNumFilters;

#ifdef FIXED_POINT_FE

	/* Fill filter-bank channels */
	memset(melSpecPower, 0, pMfccVar->nNumFilters*sizeof(hci_mfcc32));

	t1 = *pOutFFT++;					// Q(sft)/16
	t2 = *pOutFFT++;					// Q(sft)/16
	L_tmp32 = t1 * t1 + t2 * t2;		// Q(2*sft)/32
	prevPowerSpec = L_tmp32;

	while (pChan < pLastChan) {
		t1 = *pOutFFT++;			// Q(sft)/16
		t2 = *pOutFFT++;			// Q(sft)/16

		chan = *pChan;
		next_chan = *(pChan+1);

		/*
		 *	conversion ...   
		 *
		 *	ek = (float)sqrt(t1*t1 + t2*t2)		 
		 */

		L_tmp32 = t1 * t1;						// Q(2*sft)/32
		L_tmp32 += (hci_mfcc32)(t2 * t2);		// Q(2*sft)/32

		L_val_DPS = PowerASR_BasicOP_subtract_32_32(prevPowerSpec, L_tmp32);		// Q(2*sft)/32
		L_val_DPS = PowerASR_BasicOP_abs_32_32(L_val_DPS);	// Q(2*sft)/32
		L_ek = L_val_DPS;
		PowerASR_BasicOP_separateBits_32_16(L_val_DPS, &val_hi, &val_lo);
		L_ek_compl = L_ek - PowerASR_BasicOP_multiply_32_16_32(val_hi, val_lo, *pLowWgt);	// Q(2*sft)/32
		L_ek -= L_ek_compl;											// Q(2*sft)/32

		if(chan >= 0) {
			melSpecPower[chan] += PowerASR_BasicOP_shiftRight_32_32(L_ek, 6);	// Q(2*sft-6)/32
		}

		if (chan < (pMfccVar->nNumFilters-1)) {
			melSpecPower[chan+1] += PowerASR_BasicOP_shiftRight_32_32(L_ek_compl, 6);	// Q(2*sft-6)/32
		}

		prevPowerSpec = L_tmp32;

		pChan++; pLowWgt++;
	}

	/* Take logs for mel spectral magnitude */
	iShift = 2*var_shift - 6;
	melfloor = PowerASR_BasicOP_shiftLeft_32_32(1,iShift);
	pMelSpec = melSpecPower;
	pLastSpec = pMelSpec + pMfccVar->nNumFilters;
	while (pMelSpec < pLastSpec) {
		L_tmp32 = *pMelSpec;					// Q(2*sft-6)/32
		if (L_tmp32 < melfloor) {
			*pMelSpec = 0;
		}
		else {
			*pMelSpec = PowerASR_BasicOP_fixedLOG_32_32(L_tmp32,iShift);		// Q15.32
		}
		pMelSpec++;
	}

#else	// !FIXED_POINT_FE

	/* Fill filter-bank channels */
	memset(melSpecPower, 0, pMfccVar->nNumFilters*sizeof(hci_mfcc32));
	t1 = *pOutFFT++;
	t2 = *pOutFFT++;
	L_ek = t1*t1 + t2*t2;
	prevPowerSpec = L_ek;

	while (pChan < pLastChan) {
		t1 = *pOutFFT++;
		t2 = *pOutFFT++;
		L_ek = t1*t1 + t2*t2;
		L_val_DPS = prevPowerSpec - L_ek;
		prevPowerSpec = L_ek;
		ek = (hci_mfcc32)HCI_ABS(L_val_DPS);
		ek_compl = ek - (*pLowWgt) * ek;
		ek -= ek_compl;
		chan = *pChan;
		if (chan >= 0) {
			melSpecPower[chan] += ek;
		}
		if (chan < (pMfccVar->nNumFilters-1)) {
			melSpecPower[chan+1] += ek_compl;
		}

		pChan++; pLowWgt++;
	}

	/* Take logs */
	melfloor = 1.0f;
	pMelSpec = melSpecPower;
	while (pMelSpec < pLastSpec) {
		if (*pMelSpec < melfloor) {
			*pMelSpec = 0.0f;
		}
		else {
			*pMelSpec = (hci_mfcc32) log(*pMelSpec);
		}
		pMelSpec++;
	}

#endif	// #ifdef FIXED_POINT_FE

}


/**
 *	convert FFT outputs into mel-frequency predictive differential power spectrum
 */
void
FX_SigProc_FFT2MelPredictiveDiffPowerSpectrum(hci_mfcc16 *xfft,				///< (i) : FFT output	(Q(sft).16)
											  hci_mfcc32 *melSpecPower,		///< (o) : mel-freq. spectrum power Q15.32
											  MfccParameters *pMfccVar,		///< (i) : front-end parameters
											  hci_int16 var_shift)			///< (i) : left shift count
{
	hci_mfcc16 *pOutFFT = 0;
	hci_mfcc16 *pLastOutFFT = 0;
	hci_int16 *pChan = 0;
	hci_mfcc16 *pLowWgt = 0;
	hci_mfcc32 *pMelSpec = 0;
	hci_mfcc32 *pLastSpec = 0;
	hci_mfcc32 *pPowerSpec = 0;
	hci_mfcc32 orgPowerSpec[NB_FFT_SIZE];
	hci_mfcc32 predictedPower[NB_FFT_SIZE];
	hci_mfcc32 outSineFilter = 0;
	hci_mfcc32 prevLeftDiffSpec = 0;
	hci_mfcc32 currLeftDiffSpec = 0;
	hci_mfcc32 prevRightDiffSpec = 0;
	hci_mfcc32 currRightDiffSpec = 0;
	hci_mfcc32 threshSpec = 0;
	hci_int16 chan = 0;
	hci_mfcc16 t1 = 0;
	hci_mfcc16 t2 = 0;
	hci_mfcc16 ek = 0;
	hci_mfcc16 ek_compl = 0;
	hci_mfcc32 L_ek = 0;
	hci_mfcc32 L_ek_compl = 0;
	hci_mfcc32 melfloor = 0;
	hci_mfcc16 fWeight = 0;
	hci_int16 k = 0;
#ifdef FIXED_POINT_FE
	hci_int32 L_tmp32 = 0;
	hci_int32 L_sq = 0;
	hci_int32 L_sq_compl = 0;
	hci_int64 L_tmp64 = 0;
	hci_int16 iShift = 0;
	hci_int16 val_hi = 0;
	hci_int16 val_lo = 0;
#endif	// #ifdef FIXED_POINT_FE

#ifdef FIXED_POINT_FE

	// initialize buffers
	memset(melSpecPower, 0, pMfccVar->nNumFilters*sizeof(hci_mfcc32));
	memset(orgPowerSpec, 0, sizeof(orgPowerSpec));
	memset(predictedPower, 0, sizeof(predictedPower));

	// estimate amplitude spectrum at FFT points
	pPowerSpec = orgPowerSpec;
	pOutFFT = xfft;
	pLastOutFFT = xfft + 2 * pMfccVar->nLowBin;
	while (pOutFFT < pLastOutFFT) {
		t1 = *pOutFFT++;												// Q(sft)/16
		t2 = *pOutFFT++;												// Q(sft)/16
		L_tmp32 = t1 * t1;												// Q(2*sft)/32
		L_tmp32 += (hci_mfcc32)(t2 * t2);								// Q(2*sft)/32
		*pPowerSpec++ = L_tmp32;										// Q(2*sft)/32
	}
	pLastOutFFT = xfft + 2 * pMfccVar->nHighBin;
	pChan = pMfccVar->lowerChan + pMfccVar->nLowBin;
	pLowWgt = pMfccVar->lowerWeight + pMfccVar->nLowBin;
	while (pOutFFT <= pLastOutFFT) {
		t1 = *pOutFFT++;												// Q(sft)/16
		t2 = *pOutFFT++;												// Q(sft)/16
		L_tmp32 = t1 * t1;												// Q(2*sft)/32
		L_tmp32 += (hci_mfcc32)(t2 * t2);								// Q(2*sft)/32
		*pPowerSpec++ = L_tmp32;										// Q(2*sft)/32

		pChan++; pLowWgt++;
	}
	pLastOutFFT = xfft + pMfccVar->nFFTSize;
	while (pOutFFT < pLastOutFFT) {
		t1 = *pOutFFT++;												// Q(sft)/16
		t2 = *pOutFFT++;												// Q(sft)/16
		L_tmp32 = t1 * t1;												// Q(2*sft)/32
		L_tmp32 += (hci_mfcc32)(t2 * t2);								// Q(2*sft)/32
		*pPowerSpec++ = L_tmp32;										// Q(2*sft)/32
	}

	// calculate sine filter outputs
	for (k = 0; k < pMfccVar->nHalfFFT; k++) {
		outSineFilter = 0;
		pPowerSpec = orgPowerSpec + k + 1;
		pLastSpec = orgPowerSpec + HCI_MIN(pMfccVar->nHalfFFT, k+pMfccVar->nWidthSineFilter+1);
		pLowWgt = pMfccVar->sineFilter;
		while(pPowerSpec < pLastSpec) {
			PowerASR_BasicOP_separateBits_32_16(*pPowerSpec, &val_hi, &val_lo);
			L_tmp32 = PowerASR_BasicOP_multiply_32_16_32(val_hi, val_lo, *pLowWgt);	// Q(2*sft)/32
			outSineFilter = HCI_MAX(outSineFilter, L_tmp32);
			pPowerSpec++; pLowWgt++;
		}
		predictedPower[k] = outSineFilter;	// Q(2*sft)/32
	}

	// calculate outputs of mel-freq. filter banks
	fWeight = 589;		// Q9/16, typically 1.15 : range [1.05, 1.20]
	iShift = 2 * var_shift - 6;
	threshSpec = PowerASR_BasicOP_shiftLeft_32_32(1000, iShift);		// Q(2*sft-6)/32, 1000.0f;
	k = pMfccVar->nLowBin;
	prevLeftDiffSpec = prevRightDiffSpec = 0;
	if (orgPowerSpec[k+1] > predictedPower[k+1] && orgPowerSpec[k] < orgPowerSpec[k+1]) {
		PowerASR_BasicOP_separateBits_32_16(orgPowerSpec[k+1], &val_hi, &val_lo);
		L_tmp32 = PowerASR_BasicOP_multiply_32_16_32(val_hi, val_lo, fWeight);		// Q(2*sft-6)/32
		prevRightDiffSpec = PowerASR_BasicOP_subtract_32_32(PowerASR_BasicOP_shiftRight_32_32(orgPowerSpec[k],6), L_tmp32);	// Q(2*sft-6)/32
	}
	else if (orgPowerSpec[k+1] >= predictedPower[k] && orgPowerSpec[k] > orgPowerSpec[k+1]) {
		PowerASR_BasicOP_separateBits_32_16(orgPowerSpec[k], &val_hi, &val_lo);
		L_tmp32 = PowerASR_BasicOP_multiply_32_16_32(val_hi, val_lo, fWeight);	// Q(2*sft-6)/32
		prevRightDiffSpec = PowerASR_BasicOP_subtract_32_32(L_tmp32, PowerASR_BasicOP_shiftRight_32_32(orgPowerSpec[k+1],6));	// Q(2*sft-6)/32
	}
	else {
		prevRightDiffSpec = PowerASR_BasicOP_subtract_32_32(PowerASR_BasicOP_shiftRight_32_32(orgPowerSpec[k],6),
			PowerASR_BasicOP_shiftRight_32_32(orgPowerSpec[k+1],6));	// Q(2*sft-6)/32
	}
	if (k >= 2) {
		if (orgPowerSpec[k-2] > predictedPower[k-2] && orgPowerSpec[k-2] > orgPowerSpec[k-1]) {
			PowerASR_BasicOP_separateBits_32_16(orgPowerSpec[k-2], &val_hi, &val_lo);
			L_tmp32 = PowerASR_BasicOP_multiply_32_16_32(val_hi, val_lo, fWeight);	// Q(2*sft-6)/32
			prevLeftDiffSpec = PowerASR_BasicOP_subtract_32_32(PowerASR_BasicOP_shiftRight_32_32(orgPowerSpec[k-1],6), L_tmp32);	// Q(2*sft-6)/32
		}
		else if (orgPowerSpec[k-1] < predictedPower[k-2] && orgPowerSpec[k-1] >= orgPowerSpec[k-2]) {
			PowerASR_BasicOP_separateBits_32_16(orgPowerSpec[k-1], &val_hi, &val_lo);
			L_tmp32 = PowerASR_BasicOP_multiply_32_16_32(val_hi, val_lo, fWeight);	// Q(2*sft-6)/32
			prevLeftDiffSpec = PowerASR_BasicOP_subtract_32_32(L_tmp32, PowerASR_BasicOP_shiftRight_32_32(orgPowerSpec[k-2],6));	// Q(2*sft-6)/32
		}
		else {
			prevLeftDiffSpec = PowerASR_BasicOP_subtract_32_32(PowerASR_BasicOP_shiftRight_32_32(orgPowerSpec[k-1],6), 
				PowerASR_BasicOP_shiftRight_32_32(orgPowerSpec[k-2],6));	// Q(2*sft-6)/32
		}
	}

	pChan = pMfccVar->lowerChan + pMfccVar->nLowBin;
	pLowWgt = pMfccVar->lowerWeight + pMfccVar->nLowBin;
	for (k = pMfccVar->nLowBin; k <= pMfccVar->nHighBin; k++) {
		// compute right-side differential amplitude
		if ((k+2) < pMfccVar->nHalfFFT) {
			if (orgPowerSpec[k+2] > predictedPower[k+2] && orgPowerSpec[k+1] < orgPowerSpec[k+2]) {
				PowerASR_BasicOP_separateBits_32_16(orgPowerSpec[k+2], &val_hi, &val_lo);
				L_tmp32 = PowerASR_BasicOP_multiply_32_16_32(val_hi, val_lo, fWeight);	// Q(2*sft-6)/32
				currRightDiffSpec = PowerASR_BasicOP_subtract_32_32(PowerASR_BasicOP_shiftRight_32_32(orgPowerSpec[k+1],6), L_tmp32);	// Q(2*sft-6)/32
			}
			else if (orgPowerSpec[k+2] >= predictedPower[k+1] && orgPowerSpec[k+1] > orgPowerSpec[k+2]) {
				PowerASR_BasicOP_separateBits_32_16(orgPowerSpec[k+1], &val_hi, &val_lo);
				L_tmp32 = PowerASR_BasicOP_multiply_32_16_32(val_hi, val_lo, fWeight);	// Q(2*sft-6)/32
				currRightDiffSpec = PowerASR_BasicOP_subtract_32_32(L_tmp32, PowerASR_BasicOP_shiftRight_32_32(orgPowerSpec[k+2],6));	// Q(2*sft-6)/32
			}
			else {
				currRightDiffSpec = PowerASR_BasicOP_subtract_32_32(PowerASR_BasicOP_shiftRight_32_32(orgPowerSpec[k+1],6), 
					PowerASR_BasicOP_shiftRight_32_32(orgPowerSpec[k+2],6));	// Q(2*sft-6)/32
			}
		}
		else {
			currRightDiffSpec = 0;
		}
		// compute left-side differential amplitude
		if (k) {
			if (orgPowerSpec[k-1] > predictedPower[k-1] && orgPowerSpec[k-1] > orgPowerSpec[k]) {
				PowerASR_BasicOP_separateBits_32_16(orgPowerSpec[k-1], &val_hi, &val_lo);
				L_tmp32 = PowerASR_BasicOP_multiply_32_16_32(val_hi, val_lo, fWeight);	// Q(2*sft-6)/32
				currLeftDiffSpec = PowerASR_BasicOP_subtract_32_32(PowerASR_BasicOP_shiftRight_32_32(orgPowerSpec[k],6), L_tmp32);	// Q(2*sft-6)/32
			}
			else if (orgPowerSpec[k] < predictedPower[k-1] && orgPowerSpec[k] >= orgPowerSpec[k-1]) {
				PowerASR_BasicOP_separateBits_32_16(orgPowerSpec[k], &val_hi, &val_lo);
				L_tmp32 = PowerASR_BasicOP_multiply_32_16_32(val_hi, val_lo, fWeight);	// Q(2*sft-6)/32
				currLeftDiffSpec = PowerASR_BasicOP_subtract_32_32(L_tmp32, PowerASR_BasicOP_shiftRight_32_32(orgPowerSpec[k-1],6));	// Q(2*sft-6)/32
			}
			else {
				currLeftDiffSpec = PowerASR_BasicOP_subtract_32_32(PowerASR_BasicOP_shiftRight_32_32(orgPowerSpec[k],6), 
					PowerASR_BasicOP_shiftRight_32_32(orgPowerSpec[k-1],6));	// Q(2*sft-6)/32
			}
		}
		else {
			currLeftDiffSpec = 0;
		}

		// compute restored amplitude spectrum
		L_ek = PowerASR_BasicOP_add_32_32(currLeftDiffSpec, prevLeftDiffSpec);
		L_tmp32 = PowerASR_BasicOP_add_32_32(currRightDiffSpec, prevRightDiffSpec);
		L_ek = PowerASR_BasicOP_shiftRight_32_32(PowerASR_BasicOP_add_32_32(L_ek, L_tmp32), 1);		// Q(2*sft-6)/32

		if (L_ek >= threshSpec) {
			PowerASR_BasicOP_separateBits_32_16(L_ek, &val_hi, &val_lo);
			L_tmp32 = PowerASR_BasicOP_multiply_32_16_32(val_hi, val_lo, *pLowWgt);		// Q(2*sft-6)/32

			L_ek_compl = L_ek - L_tmp32;								// Q(2*sft-6)/32
			L_ek -= L_ek_compl;											// Q(2*sft-6)/32

			chan = *pChan;
			if(chan >= 0) {
				melSpecPower[chan] += L_ek;					// Q(2*sft-6)/32
			}
			if (chan < (pMfccVar->nNumFilters-1)) {
				melSpecPower[chan+1] += L_ek_compl;			// Q(2*sft-6)/32
			}
		}

		pChan++; pLowWgt++;

		prevLeftDiffSpec  = currLeftDiffSpec;
		prevRightDiffSpec = currRightDiffSpec;
	}

	iShift = 2*var_shift - 6;

	/* Take logs for mel spectral magnitude */
	melfloor = PowerASR_BasicOP_shiftLeft_32_32(1,iShift);
	pMelSpec = melSpecPower;
	pLastSpec = pMelSpec + pMfccVar->nNumFilters;
	while (pMelSpec < pLastSpec) {
		L_tmp32 = *pMelSpec;					// Q(sft)/32
		if (L_tmp32 < melfloor) {
			*pMelSpec = 0;
		}
		else {
			*pMelSpec = PowerASR_BasicOP_fixedLOG_32_32(L_tmp32,iShift);		// Q15.32
		}
		pMelSpec++;
	}

#else	// !FIXED_POINT_FE

	/* initialize buffers */
	memset(melSpecPower, 0, pMfccVar->nNumFilters*sizeof(hci_mfcc32));
	memset(orgPowerSpec, 0, sizeof(orgPowerSpec));
	memset(predictedPower, 0, sizeof(predictedPower));

	// estimate amplitude spectrum at FFT points
	pPowerSpec = orgPowerSpec;
	pOutFFT = xfft;
	pLastOutFFT = xfft + 2 * pMfccVar->nLowBin;
	while (pOutFFT < pLastOutFFT) {
		t1 = *pOutFFT++;
		t2 = *pOutFFT++;
		L_ek = t1*t1 + t2*t2;
		*pPowerSpec++ = L_ek;
	}
	pLastOutFFT = xfft + 2 * pMfccVar->nHighBin;
	pChan = pMfccVar->lowerChan + pMfccVar->nLowBin;
	pLowWgt = pMfccVar->lowerWeight + pMfccVar->nLowBin;
	while (pOutFFT <= pLastOutFFT) {
		t1 = *pOutFFT++;
		t2 = *pOutFFT++;
		L_ek = t1*t1 + t2*t2;
		*pPowerSpec++ = L_ek;
		pChan++; pLowWgt++;
	}
	pLastOutFFT = xfft + pMfccVar->nFFTSize;
	while (pOutFFT < pLastOutFFT) {
		t1 = *pOutFFT++;
		t2 = *pOutFFT++;
		L_ek = t1*t1 + t2*t2;
		*pPowerSpec++ = L_ek;
	}

	// calculate sine filter outputs
	for (k = 0; k < pMfccVar->nHalfFFT; k++) {
		outSineFilter = 0;
		pPowerSpec = orgPowerSpec + k + 1;
		pLastSpec = orgPowerSpec + HCI_MIN(pMfccVar->nHalfFFT, k+pMfccVar->nWidthSineFilter+1);
		pLowWgt = pMfccVar->sineFilter;
		while(pPowerSpec < pLastSpec) {
			outSineFilter = HCI_MAX(outSineFilter, (*pPowerSpec)*(*pLowWgt));
			pPowerSpec++; pLowWgt++;
		}
		predictedPower[k] = outSineFilter;
	}

	// calculate outputs of mel-freq. filter banks
	fWeight = 1.15f;		// [1.05, 1.20]
	threshSpec = 1000.0f;
	k = pMfccVar->nLowBin;
	prevLeftDiffSpec = prevRightDiffSpec = 0;
	if (orgPowerSpec[k+1] > predictedPower[k+1] && orgPowerSpec[k] < orgPowerSpec[k+1]) {
		prevRightDiffSpec = orgPowerSpec[k] - fWeight * orgPowerSpec[k+1];
	}
	else if (orgPowerSpec[k+1] >= predictedPower[k] && orgPowerSpec[k] > orgPowerSpec[k+1]) {
		prevRightDiffSpec = fWeight * orgPowerSpec[k] - orgPowerSpec[k+1];
	}
	else {
		prevRightDiffSpec = orgPowerSpec[k] - orgPowerSpec[k+1];
	}
	if (k >= 2) {
		if (orgPowerSpec[k-2] > predictedPower[k-2] && orgPowerSpec[k-2] > orgPowerSpec[k-1]) {
			prevLeftDiffSpec = orgPowerSpec[k-1] - fWeight * orgPowerSpec[k-2];
		}
		else if (orgPowerSpec[k-1] < predictedPower[k-2] && orgPowerSpec[k-1] >= orgPowerSpec[k-2]) {
			prevLeftDiffSpec = fWeight * orgPowerSpec[k-1] - orgPowerSpec[k-2];
		}
		else {
			prevLeftDiffSpec = orgPowerSpec[k-1] - orgPowerSpec[k-2];
		}
	}

	pChan = pMfccVar->lowerChan + pMfccVar->nLowBin;
	pLowWgt = pMfccVar->lowerWeight + pMfccVar->nLowBin;
	for (k = pMfccVar->nLowBin; k <= pMfccVar->nHighBin; k++) {
		// compute right-side differential power
		if ((k+2) < pMfccVar->nHalfFFT) {
			if (orgPowerSpec[k+2] > predictedPower[k+2] && orgPowerSpec[k+1] < orgPowerSpec[k+2]) {
				currRightDiffSpec = orgPowerSpec[k+1] - fWeight * orgPowerSpec[k+2];
			}
			else if (orgPowerSpec[k+2] >= predictedPower[k+1] && orgPowerSpec[k+1] > orgPowerSpec[k+2]) {
				currRightDiffSpec = fWeight * orgPowerSpec[k+1] - orgPowerSpec[k+2];
			}
			else {
				currRightDiffSpec = orgPowerSpec[k+1] - orgPowerSpec[k+2];
			}
		}
		else {
			currRightDiffSpec = 0;
		}
		// compute left-side differential power
		if (k) {
			if (orgPowerSpec[k-1] > predictedPower[k-1] && orgPowerSpec[k-1] > orgPowerSpec[k]) {
				currLeftDiffSpec = orgPowerSpec[k] - fWeight * orgPowerSpec[k-1];
			}
			else if (orgPowerSpec[k] < predictedPower[k-1] && orgPowerSpec[k] >= orgPowerSpec[k-1]) {
				currLeftDiffSpec = fWeight * orgPowerSpec[k] - orgPowerSpec[k-1];
			}
			else {
				currLeftDiffSpec = orgPowerSpec[k] - orgPowerSpec[k-1];
			}
		}
		else {
			currLeftDiffSpec = 0;
		}
		// compute restored power spectrum
		ek = (currLeftDiffSpec + currRightDiffSpec + prevLeftDiffSpec + prevRightDiffSpec) * 0.5f;
		if (ek >= threshSpec) {
			ek_compl = ek - (*pLowWgt) * ek;
			ek -= ek_compl;
			chan = *pChan;
			if (chan >= 0) {
				melSpecPower[chan] += ek;
			}
			if (chan < (pMfccVar->nNumFilters-1)) {
				melSpecPower[chan+1] += ek_compl;
			}
		}

		pChan++; pLowWgt++;

		prevLeftDiffSpec  = currLeftDiffSpec;
		prevRightDiffSpec = currRightDiffSpec;
	}

	/* Take logs */
	melfloor = 1.0f;
	pMelSpec = melSpecPower;
	while (pMelSpec < pLastSpec) {
		if (*pMelSpec < melfloor) {
			*pMelSpec = 0.0f;
		}
		else {
			*pMelSpec = (hci_mfcc32) log(*pMelSpec);
		}
		pMelSpec++;
	}

#endif	// #ifdef FIXED_POINT_FE

}


/**
 *	apply FFT to real-valued signal
 */
void
FX_SigProc_realFFT(hci_mfcc16 *xfft,		///< (i/o)	: input/output of FFT	(Q(sft).16)
				   hci_mfcc16 *phs_tbl,		///< (i)	: phase constant table (Q15.16)
				   hci_int16 nFFTSize,		///< (i)	: FFT point (N)
				   hci_int16 nHalfFFT,		///< (i)	: FFT point divided 2	(N/2)
				   hci_int16 nFFTStage)		///< (i)	: number of stage in radix-2 FFT
{
	hci_mfcc16 *pFwdFFT = 0;
	hci_mfcc16 *pBwdFFT = 0;
	hci_mfcc16 *pLastFFT = 0;
	hci_mfcc16 *pFwdPhs = 0;
	hci_mfcc16 *pBwdPhs = 0;
	//hci_int16 i = 0;
	//hci_int16 j = 0;
	hci_int16 stage = 0;
	hci_mfcc16 ftmp1_real = 0;
	hci_mfcc16 ftmp1_imag = 0;
	hci_mfcc16 ftmp2_real = 0;
	hci_mfcc16 ftmp2_imag = 0;
	hci_mfcc32 Ltmp1 = 0;

#ifdef FIXED_POINT_FE
	hci_mfcc32 Lftmp1_real = 0;
	hci_mfcc32 Lftmp1_imag = 0;
	hci_mfcc32 Lftmp2_real = 0;
	hci_mfcc32 Lftmp2_imag = 0;
	hci_mfcc32 Ltmp2 = 0;
#endif

	stage = nFFTStage;

	_FX_SigProc_FFT(xfft, phs_tbl, nFFTSize, nHalfFFT, stage);		/* Perform the complex FFT */

	/* First, handle the DC and foldover frequencies */
	ftmp1_real = xfft[0];
	ftmp2_real = xfft[1];

	xfft[0] = ftmp1_real + ftmp2_real;
	xfft[1] = ftmp1_real - ftmp2_real;

	/* Now, handle the remaining positive frequencies */
	pFwdFFT = xfft + 2;
	pBwdFFT = xfft + nFFTSize - 2;
	pLastFFT = xfft + nHalfFFT;
	pFwdPhs = phs_tbl + 2;
	pBwdPhs = phs_tbl + nFFTSize - 2;
	while (pFwdFFT < pLastFFT) {

#ifdef FIXED_POINT_FE

		Lftmp1_real = PowerASR_BasicOP_shiftLeft_32_32((hci_int32)(*pFwdFFT + *pBwdFFT), 16);			// Q(sft+16).32
		Lftmp1_imag = PowerASR_BasicOP_shiftLeft_32_32((hci_int32)(*(pFwdFFT+1) - *(pBwdFFT+1)), 16);	// Q(sft+16).32
		Lftmp2_real = PowerASR_BasicOP_shiftLeft_32_32((hci_int32)(*(pFwdFFT+1) + *(pBwdFFT+1)), 16);	// Q(sft+16).32
		Lftmp2_imag = PowerASR_BasicOP_shiftLeft_32_32((hci_int32)(*pBwdFFT - *pFwdFFT), 16);			// Q(sft+16).32

		ftmp1_real = PowerASR_BasicOP_round_32_16(Lftmp1_real);					// Q(sft).16
		ftmp1_imag = PowerASR_BasicOP_round_32_16(Lftmp1_imag);					// Q(sft).16
		ftmp2_real = PowerASR_BasicOP_round_32_16(Lftmp2_real);					// Q(sft).16
		ftmp2_imag = PowerASR_BasicOP_round_32_16(Lftmp2_imag);					// Q(sft).16

		Ltmp1 = PowerASR_BasicOP_multiply_16_32(ftmp2_real, *pFwdPhs);		// Q(sft+16).32
		Ltmp1 = PowerASR_BasicOP_multiplySubtractConst_16_32_32(Ltmp1, ftmp2_imag, *(pFwdPhs+1));	// Q(sft+16).32
		Ltmp2 = PowerASR_BasicOP_add_32_32(Lftmp1_real, Ltmp1);					// Q(sft+16).32
		*pFwdFFT = (hci_mfcc16)PowerASR_BasicOP_shiftRight_32_32(Ltmp2, 17);		// Q(sft).16 (/2)

		Ltmp1 = PowerASR_BasicOP_multiply_16_32(ftmp2_imag, *pFwdPhs);		// Q(sft+16).32
		Ltmp1 = PowerASR_BasicOP_multiplyAddConst_16_32_32(Ltmp1, ftmp2_real, *(pFwdPhs+1));	// Q(sft+16).32
		Ltmp2 = PowerASR_BasicOP_add_32_32(Lftmp1_imag, Ltmp1);					// Q(sft+16).32
		*(pFwdFFT+1) = (hci_mfcc16)PowerASR_BasicOP_shiftRight_32_32(Ltmp2, 17);	// Q(sft).16

		Ltmp1 = PowerASR_BasicOP_multiply_16_32(ftmp2_real, *pBwdPhs);		// Q(sft+16).32
		Ltmp1 = PowerASR_BasicOP_multiplyAddConst_16_32_32(Ltmp1, ftmp2_imag, *(pBwdPhs+1));	// Q(sft+16).32
		Ltmp2 = PowerASR_BasicOP_add_32_32(Lftmp1_real, Ltmp1);					// Q(sft+16).32
		*pBwdFFT = (hci_mfcc16)PowerASR_BasicOP_shiftRight_32_32(Ltmp2, 17);		// Q(sft).16

		Ltmp1 = PowerASR_BasicOP_multiply_16_32(ftmp2_real, *(pBwdPhs+1));	// Q(sft+16).32
		Ltmp1 = PowerASR_BasicOP_multiplySubtractConst_16_32_32(Ltmp1, ftmp2_imag, *pBwdPhs);	// Q(sft+16).32
		Ltmp2 = PowerASR_BasicOP_subtract_32_32(Ltmp1, Lftmp1_imag);			// Q(sft+16).32
		*(pBwdFFT+1) = (hci_mfcc16)PowerASR_BasicOP_shiftRight_32_32(Ltmp2, 17);	// Q(sft).16

#else	// !FIXED_POINT_FE

		ftmp1_real = 0.5f * (*pFwdFFT + *pBwdFFT);
		ftmp1_imag = 0.5f * (*(pFwdFFT+1) - *(pBwdFFT+1));
		ftmp2_real = 0.5f * (*(pFwdFFT+1) + *(pBwdFFT+1));
		ftmp2_imag = 0.5f * (*pBwdFFT - *pFwdFFT);

		Ltmp1 = ftmp2_real * (*pFwdPhs) - ftmp2_imag * (*(pFwdPhs+1));
		*pFwdFFT = ftmp1_real + Ltmp1;

		Ltmp1 = ftmp2_imag * (*pFwdPhs) + ftmp2_real * (*(pFwdPhs+1));
		*(pFwdFFT+1) = ftmp1_imag + Ltmp1;

		Ltmp1 = ftmp2_real * (*pBwdPhs) + ftmp2_imag * (*(pBwdPhs+1));
		*pBwdFFT = ftmp1_real + Ltmp1;

		Ltmp1 = ftmp2_real * (*(pBwdPhs+1)) - ftmp2_imag * (*pBwdPhs);
		*(pBwdFFT+1) = Ltmp1 - ftmp1_imag;

#endif	// #ifdef FIXED_POINT_FE

		pFwdFFT += 2; pBwdFFT -= 2;
		pFwdPhs += 2; pBwdPhs -= 2;
	}
	/*
	for(i = 2, j = nFFTSize - i; i < nHalfFFT; i += 2, j = nFFTSize-i) {

#ifdef FIXED_POINT_FE

		Lftmp1_real = (hci_mfcc32)(xfft[i] + xfft[j]) << 16;					// Q(sft+16).32
		Lftmp1_imag = (hci_mfcc32)(xfft[i+1] - xfft[j+1]) << 16;				// Q(sft+16).32
		Lftmp2_real = (hci_mfcc32)(xfft[i+1] + xfft[j+1]) << 16;				// Q(sft+16).32
		Lftmp2_imag = (hci_mfcc32)(xfft[j] - xfft[i]) << 16;					// Q(sft+16).32

		ftmp1_real = PowerASR_BasicOP_round_32_16(Lftmp1_real);					// Q(sft).16
		ftmp1_imag = PowerASR_BasicOP_round_32_16(Lftmp1_imag);					// Q(sft).16
		ftmp2_real = PowerASR_BasicOP_round_32_16(Lftmp2_real);					// Q(sft).16
		ftmp2_imag = PowerASR_BasicOP_round_32_16(Lftmp2_imag);					// Q(sft).16

		Ltmp1 = PowerASR_BasicOP_multiply_16_32(ftmp2_real, phs_tbl[i]);		// Q(sft+16).32
		Ltmp1 = PowerASR_BasicOP_multiplySubtractConst_16_32_32(Ltmp1, ftmp2_imag, phs_tbl[i + 1]);	// Q(sft+16).32
		Ltmp2 = PowerASR_BasicOP_add_32_32(Lftmp1_real, Ltmp1);					// Q(sft+16).32
		xfft[i] = (hci_mfcc16)PowerASR_BasicOP_shiftRight_32_32(Ltmp2, 17);		// Q(sft).16 (/2)

		Ltmp1 = PowerASR_BasicOP_multiply_16_32(ftmp2_imag, phs_tbl[i]);		// Q(sft+16).32
		Ltmp1 = PowerASR_BasicOP_multiplyAddConst_16_32_32(Ltmp1, ftmp2_real, phs_tbl[i + 1]);	// Q(sft+16).32
		Ltmp2 = PowerASR_BasicOP_add_32_32(Lftmp1_imag, Ltmp1);					// Q(sft+16).32
		xfft[i+1] = (hci_mfcc16)PowerASR_BasicOP_shiftRight_32_32(Ltmp2, 17);	// Q(sft).16

		Ltmp1 = PowerASR_BasicOP_multiply_16_32(ftmp2_real, phs_tbl[j]);		// Q(sft+16).32
		Ltmp1 = PowerASR_BasicOP_multiplyAddConst_16_32_32(Ltmp1, ftmp2_imag, phs_tbl[j + 1]);	// Q(sft+16).32
		Ltmp2 = PowerASR_BasicOP_add_32_32(Lftmp1_real, Ltmp1);					// Q(sft+16).32
		xfft[j] = (hci_mfcc16)PowerASR_BasicOP_shiftRight_32_32(Ltmp2, 17);		// Q(sft).16

		Ltmp1 = PowerASR_BasicOP_multiply_16_32(ftmp2_real, phs_tbl[j + 1]);	// Q(sft+16).32
		Ltmp1 = PowerASR_BasicOP_multiplySubtractConst_16_32_32(Ltmp1, ftmp2_imag, phs_tbl[j]);	// Q(sft+16).32
		Ltmp2 = PowerASR_BasicOP_subtract_32_32(Ltmp1, Lftmp1_imag);			// Q(sft+16).32
		xfft[j+1] = (hci_mfcc16)PowerASR_BasicOP_shiftRight_32_32(Ltmp2, 17);	// Q(sft).16

#else	// !FIXED_POINT_FE

		ftmp1_real = 0.5f * (xfft[i] + xfft[j]);
		ftmp1_imag = 0.5f * (xfft[i+1] - xfft[j+1]);
		ftmp2_real = 0.5f * (xfft[i+1] + xfft[j+1]);
		ftmp2_imag = 0.5f * (xfft[j] - xfft[i]);

		Ltmp1 = ftmp2_real * phs_tbl[i] - ftmp2_imag * phs_tbl[i + 1];
		xfft[i] = ftmp1_real + Ltmp1;

		Ltmp1 = ftmp2_imag * phs_tbl[i] + ftmp2_real * phs_tbl[i + 1];
		xfft[i+1] = ftmp1_imag + Ltmp1;

		Ltmp1 = ftmp2_real * phs_tbl[j] + ftmp2_imag * phs_tbl[j + 1];
		xfft[j] = ftmp1_real + Ltmp1;

		Ltmp1 = ftmp2_real * phs_tbl[j + 1] - ftmp2_imag * phs_tbl[j];
		xfft[j+1] = Ltmp1 - ftmp1_imag;

#endif	// #ifdef FIXED_POINT_FE
	}
	*/

}


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
								hci_int16 nFrameId)			///< (i) : frame index
{
	hci_mfcc16 *pRaw = 0;
	hci_mfcc16 *pLastRaw = 0;
	hci_mfcc64 L_framePower = 0;
	hci_int32 nFrameSize = 0;
	hci_mfcc32 L_blockPowr = 0;
	hci_mfcc32 L_logPower = 0;
	hci_int16 iB = 0;
	hci_int16 nTotalBlock = 0;
	hci_int16 nNewBlock = 0;
	hci_int16 iLogBlock = 0;
	hci_int16 nLenBlockSample = 0;
	hci_int16 var_shift = 0;

	if (nLenSample%80) {
		HCIMSG_ERROR("[ERROR] SampleLength must be a multiple of 80 !!\n");
		return 0;
	}

	nLenBlockSample = 80;
	if (80 == nLenSample) {
		nTotalBlock = 3;
		nNewBlock = 1;
		var_shift = 8;
		nFrameSize = 240;
	}
	else if (160 == nLenSample) {
		nTotalBlock = 6;
		nNewBlock = 2;
		var_shift = 9;
		nFrameSize = 480;
	}

	// update 80-sample block power/log-power buffer
	for (iB = 0; iB < nTotalBlock - nNewBlock; iB++) {
		blockPower[iB] = blockPower[iB+nNewBlock];
		L_framePower += blockPower[iB];
	}
	for (iB = 0; iB < 2; iB++) {
		logPower[iB] = logPower[iB+1];
	}

	// compute power/log-power for new blocks
	pRaw = pSample;
	for (iB = nTotalBlock-nNewBlock; iB < nTotalBlock; iB++) {
		blockPower[iB] = 0;
		pLastRaw = pRaw + nLenBlockSample;
		while (pRaw < pLastRaw) {
			blockPower[iB] += HCI_SQ(*pRaw);
			pRaw++;
			blockPower[iB] += HCI_SQ(*pRaw);
			pRaw++;
			blockPower[iB] += HCI_SQ(*pRaw);
			pRaw++;
			blockPower[iB] += HCI_SQ(*pRaw);
			pRaw++;
		}
		L_framePower += blockPower[iB];
#ifdef FIXED_POINT_FE
		L_blockPowr = (hci_mfcc32)(blockPower[iB]>>7);
		L_logPower = PowerASR_BasicOP_fixedLOG_32_32(L_blockPowr,0);
		L_logPower += 158991;	// [ln(1<<7)] << 15, Q15.32
#else	// !FIXED_POINT_FE
		L_logPower = (hci_mfcc32)log(blockPower[iB]);
#endif	// #ifdef FIXED_POINT_FE
		if (iB == (nTotalBlock-nNewBlock)) {
			logPower[2] = L_logPower;
		}
		else {
			logPower[2] = PowerASR_BasicOP_addLOG_32_32(L_logPower, logPower[2], pLAddTbl);
		}
	}

	// compute log frame power
	if (nFrameId >= 2) {
		L_logPower = PowerASR_BasicOP_addLOG_32_32(logPower[0], logPower[1], pLAddTbl);
		L_logPower = PowerASR_BasicOP_addLOG_32_32(L_logPower, logPower[2], pLAddTbl);
		*frameEn = _FX_SigProc_computeLogPowerOfNewSamples(blockPower+(nTotalBlock-nNewBlock),
														   nNewBlock);
		return L_logPower;
	}
	else {
		return 0;
	}
}


/**
 *	compute log frame energy
 */
hci_mfcc32
FX_SigProc_computeLogFrameEnergy(MFCC_UserData *pMfccData,		///< (i/o) : MFCC data
								 MfccParameters *pMfccVar,		///< (i) : front-end parameters
								 hci_int16 var_shift)			///< (i) : left shift count
{
	hci_mfcc32 *lastEng = 0;
	hci_mfcc32 *specEng = 0;
	hci_int16 nLowBin = 0;
	hci_int16 nHighBin = 0;
	hci_mfcc32 L_energy = 0;
	hci_mfcc32 L_common_energy = 0;
	hci_int16 nLowEnergyBin = 0;
	hci_int16 nHighEnergyBin = 0;
#ifdef FIXED_POINT_FE
	hci_int16 iShift = 0;
#endif	// #ifdef FIXED_POINT_FE

	if ( pMfccVar->nSampleRate == 8000 ) {
		nLowEnergyBin  = 8;
		nHighEnergyBin = 120;
		nLowBin        = 8;
		nHighBin       = 120;
	}
	else {	// 16kHz
		nLowEnergyBin  = 8;
		nHighEnergyBin = 240;
//		nLowBin        = 8;
		nLowBin        = 4;
		nHighBin       = 240;
//		nLowBin        = 16;
//		nHighBin       = 240;
	}

	specEng = pMfccData->subbandFrameEng + HCI_MAX(nLowBin, nLowEnergyBin);
	lastEng = pMfccData->subbandFrameEng + HCI_MIN(nHighBin, nHighEnergyBin);
	while (specEng < lastEng) {
		L_energy += *specEng++;		// Q(2*sft-8).32
		L_energy += *specEng++;		// Q(2*sft-8).32
		L_energy += *specEng++;		// Q(2*sft-8).32
		L_energy += *specEng++;		// Q(2*sft-8).32
	}
	L_common_energy = L_energy;

	if ( nLowBin < nLowEnergyBin || nHighBin > nHighEnergyBin ) {
		L_energy = L_common_energy;
		if ( nLowBin < nLowEnergyBin ) {
			specEng = pMfccData->subbandFrameEng + nLowBin;
			lastEng = pMfccData->subbandFrameEng + nLowEnergyBin;
			while (specEng < lastEng) {
				L_energy += *specEng++;		// Q(2*sft-8).32
			}
		}
		if ( nHighBin > nHighEnergyBin ) {
			specEng = pMfccData->subbandFrameEng + nHighEnergyBin;
			lastEng = pMfccData->subbandFrameEng + nHighBin;
			while (specEng < lastEng) {
				L_energy += *specEng++;		// Q(2*sft-8).32
			}
		}
	pMfccData->nSpecEntropy = L_energy;
	}
	else {
		pMfccData->nSpecEntropy = L_common_energy;
	}

	L_energy = L_common_energy;
	if ( nLowBin > nLowEnergyBin || nHighBin < nHighEnergyBin ) {
		if ( nLowBin > nLowEnergyBin ) {
	specEng = pMfccData->subbandFrameEng + nLowEnergyBin;
	lastEng = pMfccData->subbandFrameEng + nLowBin;
	while (specEng < lastEng) {
		L_energy += *specEng++;		// Q(2*sft-8).32
	}
		}
		if ( nHighBin < nHighEnergyBin ) {
	specEng = pMfccData->subbandFrameEng + nHighBin;
	lastEng = pMfccData->subbandFrameEng + nHighEnergyBin;
	while (specEng < lastEng) {
		L_energy += *specEng++;		// Q(2*sft-8).32
	}
		}
	}

#ifdef FIXED_POINT_FE
	iShift = 2 * var_shift - 8;
	L_energy = PowerASR_BasicOP_fixedLOG_32_32(L_energy,iShift);			// Q15.32
	if (L_energy < 0) L_energy = 0;
#else	// !FIXED_POINT_FE
	if (L_energy < (hci_mfcc32)1) {
		L_energy = (hci_mfcc32)0;
	}
	else {
		L_energy = (hci_mfcc32)log(L_energy);
	}
#endif	// #ifdef FIXED_POINT_FE

	return L_energy;
}


#define INIT_NE_FRAME 4
//#define INIT_NE_FRAME 8

#ifdef FIXED_POINT_FE
#define MIN_LOG_E	(491520)			// Q15.32
#define MAX_LOG_E	(622592)			// Q15.32
#else	// !FIXED_POINT_FE
#define MIN_LOG_E	((hci_mfcc32)15)
#define MAX_LOG_E	((hci_mfcc32)20)	//((hci_mfcc32)19)
#endif	// #ifdef FIXED_POINT_FE


hci_mfcc32
FX_SigProc_computeSpectralEntropy(MFCC_UserData *pMfccData,		///< (i/o) : MFCC data
								  MfccParameters *pMfccVar,		///< (i) : front-end parameters
								  const hci_int16 flagVAD,		///< (i) VAD flag (0 = unvoiced, 1 = voiced)
								  hci_int16 var_shift)			///< (i) : left shift count
{
	hci_int16 nLowerBin = 0;
	hci_int16 nHigherBin = 0;
	hci_mfcc32 totalPower = 0;
	hci_mfcc32 L_entropy = 0;
	hci_mfcc32 binProb = 0;
	hci_mfcc32 nProbConst = 0;
	hci_mfcc32 *lastEng = 0;
	hci_mfcc32 *specEng = 0;
	hci_mfcc32 *avgNoiseSpec = 0;
	hci_mfcc32 *whiteSpec = 0;
	hci_mfcc32 minProb = 0;
	hci_mfcc32 minEntropy = 0;
	hci_mfcc_t frameEnergy = 0;
	hci_mfcc_t framePower = 0;
	hci_mfcc_t threshEnergy = 0;
	hci_mfcc_t fullBandSpec = 0;
	hci_mfcc_t minBinSpec = 0;
	hci_mfcc_t minWhiteSpec = 0;
	hci_mfcc_t whitenSpec[NB_FFT_SIZE];
/*
	hci_mfcc32 minLogE = MIN_LOG_E;
	hci_mfcc32 maxLogE = MAX_LOG_E;
	hci_mfcc32 minLogP = 10.0f;
*/
	hci_mfcc32 minLogE = pMfccData->minLogEnergy;
	hci_mfcc32 maxLogE = pMfccData->maxLogEnergy;
	hci_mfcc32 minLogP = 10.0f;

	hci_mfcc32 diffLogE = 0;
	hci_mfcc32 diffLogP = 0;
	hci_mfcc32 updateWgt = 0;
	hci_flag bUpdateNoiseModel = FALSE;
#ifdef FIXED_POINT_FE
	hci_mfcc64 LL_entropy = 0;
	hci_mfcc32 nLogProb = 0;
	hci_mfcc32 L_temp = 0;
	hci_int16 iShift = 0;	// right shift count
#endif	// #ifdef FIXED_POINT_FE

	if ( pMfccVar->nSampleRate == 8000 ) {
		nLowerBin  = 8;
		nHigherBin = 120;
	}
	else {	// 16khz
//		nLowerBin  = 16;
//		nLowerBin  = 8;
		nLowerBin  = 4;
		nHigherBin = 240;
	}

#ifdef FIXED_POINT_FE
	minBinSpec = 60000;
	threshEnergy = 32768;	// Q15.30(1.0)
//	minProb = 33;			// Q15.32(1e-3)
	minProb = 105;			// Q20.32(1e-3)
	minEntropy = 1;
	iShift = 2 * var_shift - 8;
#else	// !FIXED_POINT_FE
	minBinSpec = (hci_mfcc_t)4;
//	threshEnergy = (hci_mfcc_t)1;
	threshEnergy = (hci_mfcc_t)2;
//	minProb = (hci_mfcc32)(1.e-3);
	minProb = (hci_mfcc32)(1.e-4);
//	minEntropy = (hci_mfcc32)(5.e-3);
//	minEntropy = (hci_mfcc32)(3.e-5);
	minEntropy = (hci_mfcc32)(1.e-2);
#endif	// #ifdef FIXED_POINT_FE

	frameEnergy = pMfccData->mfccVec[pMfccVar->nNumCepstra];
	framePower = pMfccData->frameEn;

	if (pMfccData->nMfccFrame < INIT_NE_FRAME) {
		if ( 0 == pMfccData->nMfccFrame ) {
			memset( pMfccData->noiseSpectrum, 0, sizeof(pMfccData->noiseSpectrum) );
			pMfccData->avgNoiseEnergy = frameEnergy;
			pMfccData->avgNoisePower = framePower;
			bUpdateNoiseModel = TRUE;
		}
//		pMfccData->avgNoiseEnergy += frameEnergy;
		diffLogE = frameEnergy - pMfccData->avgNoiseEnergy;
		diffLogP = framePower - pMfccData->avgNoisePower;
		if ( diffLogE < 3.0f || diffLogP < 3.0f ) {
			bUpdateNoiseModel = TRUE;
			pMfccData->avgNoiseEnergy = HCI_MAX(pMfccData->avgNoiseEnergy, frameEnergy);
			pMfccData->avgNoisePower = HCI_MAX(pMfccData->avgNoisePower, framePower);
		specEng = pMfccData->subbandFrameEng + nLowerBin;
		lastEng = pMfccData->subbandFrameEng + nHigherBin;
		avgNoiseSpec = pMfccData->noiseSpectrum + nLowerBin;
		while ( specEng < lastEng ) {
#ifdef FIXED_POINT_FE
			*avgNoiseSpec += PowerASR_BasicOP_shiftRight_32_32( *specEng, iShift );
			specEng++; avgNoiseSpec++;
			*avgNoiseSpec += PowerASR_BasicOP_shiftRight_32_32( *specEng, iShift );
			specEng++; avgNoiseSpec++;
			*avgNoiseSpec += PowerASR_BasicOP_shiftRight_32_32( *specEng, iShift );
			specEng++; avgNoiseSpec++;
			*avgNoiseSpec += PowerASR_BasicOP_shiftRight_32_32( *specEng, iShift );
			specEng++; avgNoiseSpec++;
#else
// 				*avgNoiseSpec++ += *specEng++;
// 				*avgNoiseSpec++ += *specEng++;
// 				*avgNoiseSpec++ += *specEng++;
// 				*avgNoiseSpec++ += *specEng++;
				*avgNoiseSpec = HCI_MAX(*avgNoiseSpec, *specEng); avgNoiseSpec++; specEng++;
				*avgNoiseSpec = HCI_MAX(*avgNoiseSpec, *specEng); avgNoiseSpec++; specEng++;
				*avgNoiseSpec = HCI_MAX(*avgNoiseSpec, *specEng); avgNoiseSpec++; specEng++;
				*avgNoiseSpec = HCI_MAX(*avgNoiseSpec, *specEng); avgNoiseSpec++; specEng++;
#endif
		}
		}
/*	
		if (pMfccData->nMfccFrame == (INIT_NE_FRAME-1)) {
#ifdef FIXED_POINT_FE
			pMfccData->avgNoiseEnergy >>= 2;
			avgNoiseSpec = pMfccData->noiseSpectrum + nLowerBin;
			lastEng = pMfccData->noiseSpectrum + nHigherBin;
			while ( avgNoiseSpec < lastEng ) {
				*avgNoiseSpec >>= 2; *avgNoiseSpec = HCI_MAX(*avgNoiseSpec, minBinSpec); avgNoiseSpec++;
				*avgNoiseSpec >>= 2; *avgNoiseSpec = HCI_MAX(*avgNoiseSpec, minBinSpec); avgNoiseSpec++;
				*avgNoiseSpec >>= 2; *avgNoiseSpec = HCI_MAX(*avgNoiseSpec, minBinSpec); avgNoiseSpec++;
				*avgNoiseSpec >>= 2; *avgNoiseSpec = HCI_MAX(*avgNoiseSpec, minBinSpec); avgNoiseSpec++;
			}
#else	// !FIXED_POINT_FE
			pMfccData->avgNoiseEnergy /= 4.0f;
			avgNoiseSpec = pMfccData->noiseSpectrum + nLowerBin;
			lastEng = pMfccData->noiseSpectrum + nHigherBin;
			while ( avgNoiseSpec < lastEng ) {
				*avgNoiseSpec /= 4.0f; *avgNoiseSpec = HCI_MAX(*avgNoiseSpec, minBinSpec); avgNoiseSpec++;
				*avgNoiseSpec /= 4.0f; *avgNoiseSpec = HCI_MAX(*avgNoiseSpec, minBinSpec); avgNoiseSpec++;
				*avgNoiseSpec /= 4.0f; *avgNoiseSpec = HCI_MAX(*avgNoiseSpec, minBinSpec); avgNoiseSpec++;
				*avgNoiseSpec /= 4.0f; *avgNoiseSpec = HCI_MAX(*avgNoiseSpec, minBinSpec); avgNoiseSpec++;
			}
#endif	// #ifdef FIXED_POINT_FE
			pMfccData->avgNoiseEnergy = HCI_MAX( pMfccData->avgNoiseEnergy, minLogE );
			pMfccData->avgNoiseEnergy = HCI_MIN( pMfccData->avgNoiseEnergy, maxLogE );
		}
*/	
		pMfccData->avgNoiseEnergy = HCI_MAX( pMfccData->avgNoiseEnergy, minLogE );
		pMfccData->avgNoiseEnergy = HCI_MIN( pMfccData->avgNoiseEnergy, maxLogE );
		pMfccData->avgNoisePower = HCI_MAX( pMfccData->avgNoisePower, minLogP );
	}
	else {
		diffLogE = frameEnergy - pMfccData->avgNoiseEnergy;
#ifdef FIXED_POINT_FE
		if ( !flagVAD && diffLogE < threshEnergy ) {
			pMfccData->avgNoiseEnergy += ((frameEnergy-pMfccData->avgNoiseEnergy) >> 6);
			pMfccData->avgNoiseEnergy = HCI_MAX( pMfccData->avgNoiseEnergy, minLogE );
			pMfccData->avgNoiseEnergy = HCI_MIN( pMfccData->avgNoiseEnergy, maxLogE );
			specEng = pMfccData->subbandFrameEng + nLowerBin;
			lastEng = pMfccData->subbandFrameEng + nHigherBin;
			avgNoiseSpec = pMfccData->noiseSpectrum + nLowerBin;
			while ( specEng < lastEng ) {
				*avgNoiseSpec += PowerASR_BasicOP_shiftRight_32_32( PowerASR_BasicOP_shiftRight_32_32( *specEng, iShift ) - *avgNoiseSpec, 6 );
				*avgNoiseSpec = HCI_MAX(*avgNoiseSpec, minBinSpec); 
				specEng++; avgNoiseSpec++;
				*avgNoiseSpec += PowerASR_BasicOP_shiftRight_32_32( PowerASR_BasicOP_shiftRight_32_32( *specEng, iShift ) - *avgNoiseSpec, 6 );
				*avgNoiseSpec = HCI_MAX(*avgNoiseSpec, minBinSpec); 
				specEng++; avgNoiseSpec++;
				*avgNoiseSpec += PowerASR_BasicOP_shiftRight_32_32( PowerASR_BasicOP_shiftRight_32_32( *specEng, iShift ) - *avgNoiseSpec, 6 );
				*avgNoiseSpec = HCI_MAX(*avgNoiseSpec, minBinSpec); 
				specEng++; avgNoiseSpec++;
				*avgNoiseSpec += PowerASR_BasicOP_shiftRight_32_32( PowerASR_BasicOP_shiftRight_32_32( *specEng, iShift ) - *avgNoiseSpec, 6 );
				*avgNoiseSpec = HCI_MAX(*avgNoiseSpec, minBinSpec); 
				specEng++; avgNoiseSpec++;
			}
			bUpdateNoiseModel = TRUE;
		}
#else	// !FIXED_POINT_FE
/*
		if ( !flagVAD && diffLogE < threshEnergy ) {
			pMfccData->avgNoiseEnergy += (frameEnergy-pMfccData->avgNoiseEnergy) / 64.0f;
			pMfccData->avgNoiseEnergy = HCI_MAX( pMfccData->avgNoiseEnergy, minLogE );
			pMfccData->avgNoiseEnergy = HCI_MIN( pMfccData->avgNoiseEnergy, maxLogE );
			bUpdateNoiseModel = TRUE;
			specEng = pMfccData->subbandFrameEng + nLowerBin;
			lastEng = pMfccData->subbandFrameEng + nHigherBin;
			avgNoiseSpec = pMfccData->noiseSpectrum + nLowerBin;
			while ( specEng < lastEng ) {
				*avgNoiseSpec += ( *specEng - *avgNoiseSpec) / 64.0f;
				*avgNoiseSpec = HCI_MAX(*avgNoiseSpec, minBinSpec); 
				specEng++; avgNoiseSpec++;
				*avgNoiseSpec += ( *specEng - *avgNoiseSpec) / 64.0f;
				*avgNoiseSpec = HCI_MAX(*avgNoiseSpec, minBinSpec); 
				specEng++; avgNoiseSpec++;
				*avgNoiseSpec += ( *specEng - *avgNoiseSpec) / 64.0f;
				*avgNoiseSpec = HCI_MAX(*avgNoiseSpec, minBinSpec); 
				specEng++; avgNoiseSpec++;
				*avgNoiseSpec += ( *specEng - *avgNoiseSpec) / 64.0f;
				*avgNoiseSpec = HCI_MAX(*avgNoiseSpec, minBinSpec); 
				specEng++; avgNoiseSpec++;
			}
		}
*/
		if ( diffLogE < 0.0f || diffLogP < 0.0f ) {
			bUpdateNoiseModel = TRUE;
			updateWgt = 0.02f;
			pMfccData->avgNoiseEnergy += updateWgt * (frameEnergy-pMfccData->avgNoiseEnergy);
			pMfccData->avgNoisePower += updateWgt * (framePower-pMfccData->avgNoisePower);
			specEng = pMfccData->subbandFrameEng + nLowerBin;
			lastEng = pMfccData->subbandFrameEng + nHigherBin;
			avgNoiseSpec = pMfccData->noiseSpectrum + nLowerBin;
			while ( specEng < lastEng ) {
				*avgNoiseSpec += updateWgt * (*specEng - *avgNoiseSpec);
				*avgNoiseSpec = HCI_MAX(*avgNoiseSpec, minBinSpec); 
				specEng++; avgNoiseSpec++;
				*avgNoiseSpec += updateWgt * (*specEng - *avgNoiseSpec);
				*avgNoiseSpec = HCI_MAX(*avgNoiseSpec, minBinSpec); 
				specEng++; avgNoiseSpec++;
				*avgNoiseSpec += updateWgt * (*specEng - *avgNoiseSpec);
				*avgNoiseSpec = HCI_MAX(*avgNoiseSpec, minBinSpec); 
				specEng++; avgNoiseSpec++;
				*avgNoiseSpec += updateWgt * (*specEng - *avgNoiseSpec);
				*avgNoiseSpec = HCI_MAX(*avgNoiseSpec, minBinSpec); 
				specEng++; avgNoiseSpec++;
			}
		}
		else if ( diffLogE < 1.0f && diffLogP < 2.0f ) {
			bUpdateNoiseModel = TRUE;
			updateWgt = 0.03f;
			pMfccData->avgNoiseEnergy += updateWgt * (frameEnergy-pMfccData->avgNoiseEnergy);
			pMfccData->avgNoisePower += updateWgt * (framePower-pMfccData->avgNoisePower);
			specEng = pMfccData->subbandFrameEng + nLowerBin;
			lastEng = pMfccData->subbandFrameEng + nHigherBin;
			avgNoiseSpec = pMfccData->noiseSpectrum + nLowerBin;
			while ( specEng < lastEng ) {
				*avgNoiseSpec += updateWgt * (*specEng - *avgNoiseSpec);
				*avgNoiseSpec = HCI_MAX(*avgNoiseSpec, minBinSpec); 
				specEng++; avgNoiseSpec++;
				*avgNoiseSpec += updateWgt * (*specEng - *avgNoiseSpec);
				*avgNoiseSpec = HCI_MAX(*avgNoiseSpec, minBinSpec); 
				specEng++; avgNoiseSpec++;
				*avgNoiseSpec += updateWgt * (*specEng - *avgNoiseSpec);
				*avgNoiseSpec = HCI_MAX(*avgNoiseSpec, minBinSpec); 
				specEng++; avgNoiseSpec++;
				*avgNoiseSpec += updateWgt * (*specEng - *avgNoiseSpec);
				*avgNoiseSpec = HCI_MAX(*avgNoiseSpec, minBinSpec); 
				specEng++; avgNoiseSpec++;
			}
		}
		else if ( diffLogE < 2.0f && diffLogP < 3.0f ) {
			bUpdateNoiseModel = TRUE;
			updateWgt = 0.02f;
			pMfccData->avgNoiseEnergy += updateWgt * (frameEnergy-pMfccData->avgNoiseEnergy);
			pMfccData->avgNoisePower += updateWgt * (framePower-pMfccData->avgNoisePower);
			specEng = pMfccData->subbandFrameEng + nLowerBin;
			lastEng = pMfccData->subbandFrameEng + nHigherBin;
			avgNoiseSpec = pMfccData->noiseSpectrum + nLowerBin;
			while ( specEng < lastEng ) {
				*avgNoiseSpec += updateWgt * (*specEng - *avgNoiseSpec);
				*avgNoiseSpec = HCI_MAX(*avgNoiseSpec, minBinSpec); 
				specEng++; avgNoiseSpec++;
				*avgNoiseSpec += updateWgt * (*specEng - *avgNoiseSpec);
				*avgNoiseSpec = HCI_MAX(*avgNoiseSpec, minBinSpec); 
				specEng++; avgNoiseSpec++;
				*avgNoiseSpec += updateWgt * (*specEng - *avgNoiseSpec);
				*avgNoiseSpec = HCI_MAX(*avgNoiseSpec, minBinSpec); 
				specEng++; avgNoiseSpec++;
				*avgNoiseSpec += updateWgt * (*specEng - *avgNoiseSpec);
				*avgNoiseSpec = HCI_MAX(*avgNoiseSpec, minBinSpec); 
				specEng++; avgNoiseSpec++;
			}
		}
		pMfccData->avgNoiseEnergy = HCI_MAX( pMfccData->avgNoiseEnergy, minLogE );
		pMfccData->avgNoiseEnergy = HCI_MIN( pMfccData->avgNoiseEnergy, maxLogE );
		pMfccData->avgNoisePower = HCI_MAX( pMfccData->avgNoisePower, minLogP );
#endif	// #ifdef FIXED_POINT_FE
	}

	if ( (pMfccData->nMfccFrame+1) >= INIT_NE_FRAME || !bUpdateNoiseModel ) {

#ifdef FIXED_POINT_FE
		
		iShift = 15 - 2 * var_shift + 8;
		minWhiteSpec = (30 << 15);
		
		specEng = pMfccData->subbandFrameEng + nLowerBin;
		lastEng = pMfccData->subbandFrameEng + nHigherBin;
		avgNoiseSpec = pMfccData->noiseSpectrum + nLowerBin;
		whiteSpec = whitenSpec + nLowerBin;
		while ( specEng < lastEng ) {
			*whiteSpec = PowerASR_BasicOP_divideShiftLeft_32_32( *specEng, *avgNoiseSpec, iShift );		// Q6.32
			*whiteSpec = HCI_MAX( *whiteSpec, minWhiteSpec );
			fullBandSpec += *whiteSpec;
			specEng++; avgNoiseSpec++; whiteSpec++;
			*whiteSpec = PowerASR_BasicOP_divideShiftLeft_32_32( *specEng, *avgNoiseSpec, iShift );		// Q6.32
			*whiteSpec = HCI_MAX( *whiteSpec, minWhiteSpec );
			fullBandSpec += *whiteSpec;
			specEng++; avgNoiseSpec++; whiteSpec++;
			*whiteSpec = PowerASR_BasicOP_divideShiftLeft_32_32( *specEng, *avgNoiseSpec, iShift );		// Q6.32
			*whiteSpec = HCI_MAX( *whiteSpec, minWhiteSpec );
			fullBandSpec += *whiteSpec;
			specEng++; avgNoiseSpec++; whiteSpec++;
			*whiteSpec = PowerASR_BasicOP_divideShiftLeft_32_32( *specEng, *avgNoiseSpec, iShift );		// Q6.32
			*whiteSpec = HCI_MAX( *whiteSpec, minWhiteSpec );
			fullBandSpec += *whiteSpec;
			specEng++; avgNoiseSpec++; whiteSpec++;
		}
		
		whiteSpec = whitenSpec + nLowerBin;
		lastEng   = whitenSpec + nHigherBin;
		while ( whiteSpec < lastEng )
		{
			binProb = PowerASR_BasicOP_divideShiftLeft_32_32( *whiteSpec, fullBandSpec, 20);		// Q20.32
			binProb = HCI_MAX(binProb, minProb);						// Q20.32
			nLogProb = PowerASR_BasicOP_fixedLOG_32_32(binProb,20);		// Q15.32
			LL_entropy -= (hci_mfcc64)binProb * (hci_mfcc64)nLogProb;	// Q35.64
			whiteSpec++;
			binProb = PowerASR_BasicOP_divideShiftLeft_32_32( *whiteSpec, fullBandSpec, 20);		// Q20.32
			binProb = HCI_MAX(binProb, minProb);						// Q20.32
			nLogProb = PowerASR_BasicOP_fixedLOG_32_32(binProb,20);		// Q15.32
			LL_entropy -= (hci_mfcc64)binProb * (hci_mfcc64)nLogProb;	// Q35.64
			whiteSpec++;
			binProb = PowerASR_BasicOP_divideShiftLeft_32_32( *whiteSpec, fullBandSpec, 20);		// Q20.32
			binProb = HCI_MAX(binProb, minProb);						// Q20.32
			nLogProb = PowerASR_BasicOP_fixedLOG_32_32(binProb,20);		// Q15.32
			LL_entropy -= (hci_mfcc64)binProb * (hci_mfcc64)nLogProb;	// Q35.64
			whiteSpec++;
			binProb = PowerASR_BasicOP_divideShiftLeft_32_32( *whiteSpec, fullBandSpec, 20);		// Q20.32
			binProb = HCI_MAX(binProb, minProb);						// Q20.32
			nLogProb = PowerASR_BasicOP_fixedLOG_32_32(binProb,20);		// Q15.32
			LL_entropy -= (hci_mfcc64)binProb * (hci_mfcc64)nLogProb;	// Q35.64
			whiteSpec++;
		}

		L_entropy = (hci_mfcc32)((LL_entropy+(1<<19))>>20);	// Q15.32
		nLogProb = PowerASR_BasicOP_fixedLOG_32_32((hci_int32)(nHigherBin-nLowerBin), 0);	// Q15.32
		nLogProb = PowerASR_BasicOP_divideShiftLeft_32_32(L_entropy, nLogProb, 15);	// Q15.32
		L_entropy = PowerASR_BasicOP_subtract_32_32(32768, nLogProb);

#else	// !FIXED_POINT_FE
		
		minWhiteSpec = (hci_mfcc_t)30;
		
		specEng = pMfccData->subbandFrameEng + nLowerBin;
		lastEng = pMfccData->subbandFrameEng + nHigherBin;
		avgNoiseSpec = pMfccData->noiseSpectrum + nLowerBin;
		whiteSpec = whitenSpec + nLowerBin;
		while ( specEng < lastEng ) {
			*whiteSpec = *specEng / *avgNoiseSpec;
			*whiteSpec = HCI_MAX( *whiteSpec, minWhiteSpec );
			fullBandSpec += *whiteSpec;
			specEng++; avgNoiseSpec++; whiteSpec++;
			*whiteSpec = *specEng / *avgNoiseSpec;
			*whiteSpec = HCI_MAX( *whiteSpec, minWhiteSpec );
			fullBandSpec += *whiteSpec;
			specEng++; avgNoiseSpec++; whiteSpec++;
			*whiteSpec = *specEng / *avgNoiseSpec;
			*whiteSpec = HCI_MAX( *whiteSpec, minWhiteSpec );
			fullBandSpec += *whiteSpec;
			specEng++; avgNoiseSpec++; whiteSpec++;
			*whiteSpec = *specEng / *avgNoiseSpec;
			*whiteSpec = HCI_MAX( *whiteSpec, minWhiteSpec );
			fullBandSpec += *whiteSpec;
			specEng++; avgNoiseSpec++; whiteSpec++;
		}
		
		whiteSpec = whitenSpec + nLowerBin;
		lastEng   = whitenSpec + nHigherBin;
		while ( whiteSpec < lastEng )
		{
			binProb = *whiteSpec / fullBandSpec;
			binProb = HCI_MAX(binProb, minProb);
			L_entropy -= binProb * (hci_mfcc_t)log((double)binProb);
			whiteSpec++;
			binProb = *whiteSpec / fullBandSpec;
			binProb = HCI_MAX(binProb, minProb);
			L_entropy -= binProb * (hci_mfcc_t)log((double)binProb);
			whiteSpec++;
			binProb = *whiteSpec / fullBandSpec;
			binProb = HCI_MAX(binProb, minProb);
			L_entropy -= binProb * (hci_mfcc_t)log((double)binProb);
			whiteSpec++;
			binProb = *whiteSpec / fullBandSpec;
			binProb = HCI_MAX(binProb, minProb);
			L_entropy -= binProb * (hci_mfcc_t)log((double)binProb);
			whiteSpec++;
		}

		L_entropy = (hci_mfcc32)1 - L_entropy/(hci_mfcc32)log((double)(nHigherBin-nLowerBin));

#endif	// #ifdef FIXED_POINT_FE

		L_entropy = HCI_MAX(L_entropy, minEntropy);

	}
	else
	{
		L_entropy = minEntropy;
	}
	
/*
	if ( !bUpdateNoiseModel && !flagVAD && L_entropy <= minEntropy )
	{
#ifdef FIXED_POINT_FE
		iShift = 2 * var_shift - 8;
		pMfccData->avgNoiseEnergy += ((frameEnergy-pMfccData->avgNoiseEnergy) >> 6);
		pMfccData->avgNoiseEnergy = HCI_MAX( pMfccData->avgNoiseEnergy, minLogE );
		pMfccData->avgNoiseEnergy = HCI_MIN( pMfccData->avgNoiseEnergy, maxLogE );
		specEng = pMfccData->subbandFrameEng + nLowerBin;
		lastEng = pMfccData->subbandFrameEng + nHigherBin;
		avgNoiseSpec = pMfccData->noiseSpectrum + nLowerBin;
		while ( specEng < lastEng ) {
			*avgNoiseSpec += PowerASR_BasicOP_shiftRight_32_32( PowerASR_BasicOP_shiftRight_32_32( *specEng, iShift ) - *avgNoiseSpec, 6 );
			*avgNoiseSpec = HCI_MAX(*avgNoiseSpec, minBinSpec); 
			specEng++; avgNoiseSpec++;
			*avgNoiseSpec += PowerASR_BasicOP_shiftRight_32_32( PowerASR_BasicOP_shiftRight_32_32( *specEng, iShift ) - *avgNoiseSpec, 6 );
			*avgNoiseSpec = HCI_MAX(*avgNoiseSpec, minBinSpec); 
			specEng++; avgNoiseSpec++;
			*avgNoiseSpec += PowerASR_BasicOP_shiftRight_32_32( PowerASR_BasicOP_shiftRight_32_32( *specEng, iShift ) - *avgNoiseSpec, 6 );
			*avgNoiseSpec = HCI_MAX(*avgNoiseSpec, minBinSpec); 
			specEng++; avgNoiseSpec++;
			*avgNoiseSpec += PowerASR_BasicOP_shiftRight_32_32( PowerASR_BasicOP_shiftRight_32_32( *specEng, iShift ) - *avgNoiseSpec, 6 );
			*avgNoiseSpec = HCI_MAX(*avgNoiseSpec, minBinSpec); 
			specEng++; avgNoiseSpec++;
		}
#else	// !FIXED_POINT_FE
		pMfccData->avgNoiseEnergy += (frameEnergy-pMfccData->avgNoiseEnergy) / 64.0f;
		pMfccData->avgNoiseEnergy = HCI_MAX( pMfccData->avgNoiseEnergy, minLogE );
		pMfccData->avgNoiseEnergy = HCI_MIN( pMfccData->avgNoiseEnergy, maxLogE );
		specEng = pMfccData->subbandFrameEng + nLowerBin;
		lastEng = pMfccData->subbandFrameEng + nHigherBin;
		avgNoiseSpec = pMfccData->noiseSpectrum + nLowerBin;
		while ( specEng < lastEng ) {
			*avgNoiseSpec += ( *specEng - *avgNoiseSpec) / 64.0f;
			*avgNoiseSpec = HCI_MAX(*avgNoiseSpec, minBinSpec); 
			specEng++; avgNoiseSpec++;
			*avgNoiseSpec += ( *specEng - *avgNoiseSpec) / 64.0f;
			*avgNoiseSpec = HCI_MAX(*avgNoiseSpec, minBinSpec); 
			specEng++; avgNoiseSpec++;
			*avgNoiseSpec += ( *specEng - *avgNoiseSpec) / 64.0f;
			*avgNoiseSpec = HCI_MAX(*avgNoiseSpec, minBinSpec); 
			specEng++; avgNoiseSpec++;
			*avgNoiseSpec += ( *specEng - *avgNoiseSpec) / 64.0f;
			*avgNoiseSpec = HCI_MAX(*avgNoiseSpec, minBinSpec); 
			specEng++; avgNoiseSpec++;
		}
#endif
	}
*/

	return L_entropy;
}


static hci_int32 FixedMultiply_32_32(hci_int32 a, hci_int32 b)
{
	// shift right one less bit (i.e. 15-1)
	hci_int64 c = ((hci_int64)a * (hci_int64)b) >> 14;
	// last bit shifted out = rounding-bit 
	b = (hci_int32)(c & 0x01);
	// last shift + rounding bit 
	a = (hci_int32)(c >> 1) + b;

	return a;
}

/**
 *  apply FFT to complex signal (radix-2 version)
 */
HCILAB_PRIVATE void
_FX_SigProc_FFT(hci_mfcc16 *xfft,		///< (i/o)	: input/output of FFT
				hci_mfcc16 *phs_tbl,	///< (i)	: phase constant table
				hci_int16 nFFTSize,		///< (i/o)	: FFT point
				hci_int16 nHalfFFT,		///< (i/o)	: FFT point divided 2
				hci_int16 stage)		///< (i)	: stage index
{
	//hci_mfcc16 *ffti = 0;
	//hci_mfcc16 *fftj = 0;
	//hci_mfcc16 *lastfft = 0;
	hci_mfcc32 Lftmp_real = 0;
	hci_mfcc32 Lftmp_imag = 0;
	hci_mfcc32 Ltmp32 = 0;
	hci_mfcc16 oddphase = 0;
	hci_mfcc16 evenphase = 0;
	hci_int16 i = 0;
	hci_int16 j = 0;
	hci_int16 k = 0;
	hci_int16 ii = 0;
	hci_int16 jj = 0;
	hci_int16 kk = 0;
	hci_int16 ji = 0;
	hci_mfcc16 tmp1 = 0;

	hci_mfcc32 offt[BB_FFT_SIZE];
	hci_mfcc32 *ffti = 0;
	hci_mfcc32 *fftj = 0;
	hci_mfcc32 *lastfft = 0;


	/*
	 *	[VERIFIED]
	 *
	 *		value : xfft
	 *	max error : 0.082847
	 *	min error : 0.000124
	 *	avg error : 0.018476
	 */
		
	/* Rearrange the input array in bit reversed order */
/*	for (i = 0, j = 0; i < nFFTSize-2; i = i + 2) {
		if (j > i) {
			ffti = xfft + i;
			fftj = xfft + j;
			tmp1 = *ffti;
			*ffti = *fftj;
			*fftj = tmp1;

			ffti++; fftj++;
			tmp1 = *ffti;
			*ffti = *fftj;
			*fftj = tmp1;
		}

		k = nHalfFFT;
		while (j >= k) {
			j = j - k;
			k = k >> 1;
		}
		j += k;
	}

	for (i = 0; i < stage; i++) {
		jj = 2 << i;
		kk = jj << 1;
		ii = nHalfFFT >> i;

		for(j = 0; j < jj; j += 2) {
			ji = j * ii;

			ffti = xfft + j;
			fftj = ffti + jj;
			lastfft = xfft + nFFTSize;
			evenphase = phs_tbl[ji];
			oddphase = phs_tbl[ji+1];

			while (ffti < lastfft) {

#ifdef FIXED_POINT_FE

				Lftmp_real = PowerASR_BasicOP_multiply_16_32(*fftj, evenphase);								// Q(sft+16).32
				Lftmp_real = PowerASR_BasicOP_multiplySubtractConst_16_32_32(Lftmp_real, *(fftj+1), oddphase);	// Q(sft+16).32
				Lftmp_imag = PowerASR_BasicOP_multiply_16_32(*(fftj+1), evenphase);							// Q(sft+16).32
				Lftmp_imag = PowerASR_BasicOP_multiplyAddConst_16_32_32(Lftmp_imag, *fftj, oddphase);		// Q(sft+16).32

				Ltmp32 = PowerASR_BasicOP_depositMSB_16_32(*ffti) - Lftmp_real;
				*fftj = PowerASR_BasicOP_round_32_16(PowerASR_BasicOP_shiftRight_32_32(Ltmp32,1));			// Q(sft).16
				Ltmp32 = PowerASR_BasicOP_depositMSB_16_32(*(ffti+1)) - Lftmp_imag;
				*(fftj+1) = PowerASR_BasicOP_round_32_16(PowerASR_BasicOP_shiftRight_32_32(Ltmp32,1));		// Q(sft).16
				Ltmp32 = PowerASR_BasicOP_depositMSB_16_32(*ffti) + Lftmp_real;
				*ffti = PowerASR_BasicOP_round_32_16(PowerASR_BasicOP_shiftRight_32_32(Ltmp32,1));			// Q(sft).16
				Ltmp32 = PowerASR_BasicOP_depositMSB_16_32(*(ffti+1)) + Lftmp_imag;
				*(ffti+1) = PowerASR_BasicOP_round_32_16(PowerASR_BasicOP_shiftRight_32_32(Ltmp32,1));		// Q(sft).16

#else	// !FIXED_POINT_FE

				Lftmp_real = (((*fftj) * evenphase) - ((*(fftj+1)) * oddphase));
				Lftmp_imag = (((*(fftj+1)) * evenphase) + ((*fftj) * oddphase));

				*fftj = *ffti - Lftmp_real;
				*(fftj+1) = *(ffti+1) - Lftmp_imag;
				*ffti += Lftmp_real;
				*(ffti+1) += Lftmp_imag;

#endif	// #ifdef FIXED_POINT_FE

				ffti += kk;
				fftj += kk;
			}
		}
	}*/

	for (i = 0; i < nFFTSize; i++) {
		offt[i] = xfft[i];
	}
	for (i = 0, j = 0; i < nFFTSize-2; i = i + 2) {
		if (j > i) {
			ffti = offt + i;
			fftj = offt + j;
			Ltmp32 = *ffti;
			*ffti = *fftj;
			*fftj = Ltmp32;

			ffti++; fftj++;
			Ltmp32 = *ffti;
			*ffti = *fftj;
			*fftj = Ltmp32;
		}

		k = nHalfFFT;
		while (j >= k) {
			j = j - k;
			k = k >> 1;
		}
		j += k;
	}

	for (i = 0; i < stage; i++) {
		jj = 2 << i;
		kk = jj << 1;
		ii = nHalfFFT >> i;

		for(j = 0; j < jj; j += 2) {
			ji = j * ii;

			ffti = offt + j;
			fftj = ffti + jj;
			lastfft = offt + nFFTSize;
			evenphase = phs_tbl[ji];
			oddphase = phs_tbl[ji+1];

			while (ffti < lastfft) {

#ifdef FIXED_POINT_FE

				Lftmp_real = FixedMultiply_32_32(*fftj, (hci_int32)evenphase);
				Lftmp_real = PowerASR_BasicOP_subtract_32_32(Lftmp_real,FixedMultiply_32_32(*(fftj+1), oddphase));
				Lftmp_imag = FixedMultiply_32_32(*(fftj+1), (hci_int32)evenphase);
				Lftmp_imag = PowerASR_BasicOP_add_32_32(Lftmp_imag, FixedMultiply_32_32(*fftj, oddphase));

				*fftj = *ffti - Lftmp_real;
				*(fftj+1) = *(ffti+1) - Lftmp_imag;
				*ffti += Lftmp_real;
				*(ffti+1) += Lftmp_imag;

#else	// !FIXED_POINT_FE

				Lftmp_real = (((*fftj) * evenphase) - ((*(fftj+1)) * oddphase));
				Lftmp_imag = (((*(fftj+1)) * evenphase) + ((*fftj) * oddphase));

				*fftj = *ffti - Lftmp_real;
				*(fftj+1) = *(ffti+1) - Lftmp_imag;
				*ffti += Lftmp_real;
				*(ffti+1) += Lftmp_imag;

#endif	// #ifdef FIXED_POINT_FE

				ffti += kk;
				fftj += kk;
			}
		}
	}
#ifdef FIXED_POINT_FE
	for (i = 0; i < nFFTSize; i++) {
		xfft[i] = PowerASR_BasicOP_extractLSB_32_16(PowerASR_BasicOP_shiftRight_32_32(offt[i], stage));
	}
#else	// !FIXED_POINT_FE
	for (i = 0; i < nFFTSize; i++) {
		xfft[i] = offt[i];
	}
#endif	// #ifdef FIXED_POINT_FE

}


/**
 *	 return mel-frequency corresponding to given FFT index
 */
HCILAB_PRIVATE hci_mfcc32
_FX_SigProc_MelFreq(hci_int32 nbin,			///< (i) FFT bin index
					hci_mfcc16 fres)		///< (i) frequency resolution
{
#ifdef FIXED_POINT_FE
	hci_int32	Tmpstp = 0;
	hci_int32	stp = 0;
	hci_int32	LogOut = 0;
	hci_int32	L_var_out = 0;

	/*
	 *	1127 * log [1+(k-1)*fres]
	 *  = 1127 * log{[1+(k-1)*fres]*2^15} - log(2^15)
	 */
	Tmpstp = (nbin-1) * fres;				// Q15/32	[= (k-1)*fres]
	stp = Tmpstp + 32767;					// Q15/32	[= 1+(k-1)*fres]
    LogOut = PowerASR_BasicOP_fixedLOG_32_32(stp, 15);	// Q15/32
	if(LogOut < 0)
		L_var_out = 0;
	else
		L_var_out = 1127 * LogOut;				// Q15/32bit 

	return L_var_out;

#else	// !FIXED_POINT_FE

	hci_mfcc32 melF = 0;
	melF = (hci_mfcc32)(1127 * log(1 + (nbin-1)*fres));
	return melF;

#endif	// #ifdef FIXED_POINT_FE
}


/**
 *	 convert original frequency into mel-frequency
 *		- Mel(f) = 1127 * log(1 + f/700)
 */
HCILAB_PRIVATE hci_mfcc32
_FX_SigProc_Origin2MelFreq(hci_int16 freq)		///< (i) frequency
{
#ifdef FIXED_POINT_FE
	hci_int32	Tmpstp = 0;
	hci_int32	stp = 0;
	hci_int32	LogOut = 0;
	hci_int32	L_var_out = 0;

	/*
	 *	1127 * log [1+f/700]
	 *  = 1127 * log{[1+f/700]*2^15} - log(2^15)
	 */
	Tmpstp = PowerASR_BasicOP_divideShiftLeft_32_32((hci_int32)freq, 700, 15);		// Q15/32	[= freq/700]
	stp = Tmpstp + 32767;					// Q15/32	[= 1+freq/700]
    LogOut = PowerASR_BasicOP_fixedLOG_32_32(stp, 15);
	if(LogOut < 0)
		L_var_out = 0;
	else
		L_var_out = 1127 * LogOut;				// Q15/32bit 

	return L_var_out;

#else	// !FIXED_POINT_FE

	hci_mfcc32 melF = 0;
	melF = (hci_mfcc32)(1127 * log(1 + freq/700.0));
	return melF;

#endif	// #ifdef FIXED_POINT_FE
}

/**
 *	 compute log frame energy
 */
HCILAB_PRIVATE hci_mfcc16
_FX_SigProc_computeLogPowerOfNewSamples(hci_mfcc64 *blockPower,		///< (i) frame log power per processing block
										hci_int16 nNumBlock)		///< (i) number of processing blocks
{
	hci_mfcc64 totalPower = 0;
	hci_mfcc32 framePower = 0;
	hci_mfcc16 frameLogEn = 0;

#ifdef FIXED_POINT_FE
	hci_int16 val_shift = 0;

	if (nNumBlock == 1) {
		val_shift = 6;
		totalPower = blockPower[0] + 64;
	}
	else {
		val_shift = 7;
		totalPower = blockPower[0] + blockPower[1] + 128;
	}

	framePower = (hci_mfcc32)(totalPower>>val_shift);
	frameLogEn = PowerASR_BasicOP_fixedLOG_2(framePower);
	frameLogEn = HCI_MAX(frameLogEn, 64);

#else	// !FIXED_POINT_FE

	if (nNumBlock == 1) {
		totalPower = blockPower[0] + 64.0f;
		framePower = (hci_mfcc16)(log((double)totalPower) - log(64.0));
	}
	else {
		totalPower = blockPower[0] + blockPower[1] + 128.0f;
		framePower = (hci_mfcc16)(log((double)totalPower) - log(128.0));
	}
	frameLogEn = framePower / (hci_mfcc16)log(2.0);
	frameLogEn *= 16.0f;
	frameLogEn += 0.5f;

#endif	// #ifdef FIXED_POINT_FE

	return frameLogEn;
}

/* end of file */






















