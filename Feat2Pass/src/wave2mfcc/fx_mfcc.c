
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
 *	@file	fx_mfcc.c
 *	@ingroup wave2mfcc_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	main MFCC computation interface
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "pffft.h"

#include "base/hci_type.h"
#include "base/hci_msg.h"
#include "base/hci_malloc.h"

#include "basic_op/fixpoint.h"
#include "basic_op/basic_op.h"
#include "wave2mfcc/fx_hamming.h"
#include "wave2mfcc/fx_sigproc.h"

#define MFCC_ALN 64
#if defined(_MSC_VER)
#define ALIGNED_(x) __declspec(align(x))
#elif defined(__GNUC__)
#define ALIGNED_(x) __attribute__ ((aligned (x)))
#else
#define ALIGNED_(x)
#endif

// local functions
#ifdef __cplusplus
extern "C" {
#endif 

/**
 *	For small-valued frame buffer, add a randomly generated vector (-30~+30)
 */
HCILAB_PRIVATE hci_int32
_FX_Wave2Mfcc_checkAllZeroSamples(hci_int16 *pFrameBuf,		///< In : frame sample buffer
								  hci_int16 nFrameLen		///< In : frame length in sample count
);

#ifdef __cplusplus
}
#endif


/**
 * build constant parameter set necessary for MFCC computation
 */
hci_int32
FX_Wave2Mfcc_buildFeatureExtractor(MfccParameters *pMfccVar)		///< (i/o) structure for feature-extraction environments
{
	FX_SigProc_createHammingWindow(pMfccVar->hamWin, pMfccVar->nFrameSize);

#ifdef FIXED_POINT_FE
	FX_SigProc_createFFTConstants(pMfccVar);
#else	// !FIXED_POINT_FE
	pMfccVar->fftSetup = pffft_new_setup(pMfccVar->nFFTSize, PFFFT_REAL);
	pMfccVar->fres = (hci_mfcc16)pMfccVar->nSampleRate / (pMfccVar->nFFTSize*700);
#endif	// #ifdef FIXED_POINT_FE

	FX_SigProc_buildMelFilters(pMfccVar);

	FX_SigProc_createCepstralWindow(pMfccVar->cepWin, pMfccVar->nCepLiftOrder, pMfccVar->nNumCepstra);

	FX_SigProc_createDCTCosineTable(pMfccVar);

	if (pMfccVar->nMFCCType == PDPS_MFCC) {
		FX_SigProc_createSineFilter(pMfccVar);
	}

	return 0;
}


/**
 * Convert one frame of samples to a cepstrum vector.
 *	- Process only one frame of samples.
 */
hci_int32
FX_Wave2Mfcc_convertSingleFrameToMfccVector(MfccParameters *pMfccVar,	///< (i) structure for feature-extraction environments
											MFCC_UserData *pMfccData,	///< (o) temporary/output data of feature extraction
											hci_int16 *pFrameBuf,		///< (i/o) a single frame sample buffer
											const hci_int16 flagVAD)	///< (i) VAD flag (0 = unvoiced, 1 = voiced)
{
	hci_int16 *pRawBuf = 0;
	hci_int16 *pLastBuf = 0;
	hci_mfcc16 *pInBuf = 0;
	hci_int16 var_shift = 0;
	hci_int16 idFrame = 0;
	hci_mfcc32 nLogPower = 0;
	float minMaxDiffEnergy = 0.0f;
	hci_mfcc16 inBuf[BB_FRAME_SHIFT];
	hci_mfcc16 ALIGNED_(MFCC_ALN) xfft_in[BB_FFT_SIZE];			// input of FFT
	hci_mfcc16 ALIGNED_(MFCC_ALN) xfft_out[BB_FFT_SIZE];		// output of FFT
	hci_mfcc32 melSpecPower[NUM_FILTERBANK];	// mel-frequency spectral power (Q15.32)

	// check all-zero sample frame.
	_FX_Wave2Mfcc_checkAllZeroSamples(pFrameBuf, pMfccVar->nFrameShift);

	// frame buffering
	memset(inBuf, 0, sizeof(inBuf));
#ifdef FIXED_POINT_FE
	memcpy(inBuf, pFrameBuf, (size_t)pMfccVar->nFrameShift*sizeof(hci_int16));
#else	// !FIXED_POINT_FE
	pRawBuf = pFrameBuf;
	pLastBuf = pRawBuf + pMfccVar->nFrameShift;
	pInBuf = inBuf;
	while (pRawBuf < pLastBuf) {
		*pInBuf++ = (hci_mfcc16)(*pRawBuf++);
		*pInBuf++ = (hci_mfcc16)(*pRawBuf++);
		*pInBuf++ = (hci_mfcc16)(*pRawBuf++);
		*pInBuf++ = (hci_mfcc16)(*pRawBuf++);
	}
#endif	// #ifdef FIXED_POINT_FE

	// pre-emphasis
	FX_SigProc_preEmphasize(inBuf,
							&(pMfccData->nPriorSample),
							pMfccVar->PreEmphasis,
							pMfccVar->nFrameShift);

	// compute log frame power
	nLogPower = FX_SigProc_computeLogFramePower(inBuf,
												pMfccData->blockPower,
												pMfccData->logPower,
												&(pMfccData->frameEn),
												pMfccVar->logAddTbl,
												pMfccVar->nFrameShift,
												pMfccData->nInputFrame);

    pMfccData->nInputFrame = PowerASR_BasicOP_add_32_32(pMfccData->nInputFrame, 1); // PowerASR_BasicOP_add_16_16(pMfccData->nInputFrame, 1);
	pMfccData->frameEn = nLogPower;	////

	//if (0 == nLogPower) {	// for initial 2 frames
	if (pMfccData->nInputFrame <= 2) {	// for initial 2 frames
		if (1 == pMfccData->nInputFrame) {
			memcpy(pMfccData->prevBuf, inBuf, (size_t)pMfccVar->nFrameShift*sizeof(hci_mfcc16));
		}
		else {
			memcpy(pMfccData->prevBuf+pMfccVar->nFrameShift, inBuf, (size_t)pMfccVar->nFrameShift*sizeof(hci_mfcc16));
		}
		return -1;
	}

	memset(xfft_in, 0, sizeof(xfft_in));
	memcpy(xfft_in, pMfccData->prevBuf, (size_t)(2*pMfccVar->nFrameShift)*sizeof(hci_mfcc16));
	memcpy(xfft_in+2*pMfccVar->nFrameShift, inBuf, (size_t)pMfccVar->nFrameShift*sizeof(hci_mfcc16));

#ifdef FIXED_POINT_FE
	//_FX_Wave2Mfcc_checkAllZeroSamples(xfft, pMfccVar->nFrameSize);
	var_shift = PowerASR_BasicOP_getNormalShiftCountOfVector_16_16(xfft, pMfccVar->nFrameSize, FFTHEAD);
#endif	// #ifdef FIXED_POINT_FE

	// hamming windowing
	FX_SigProc_applyHamming(xfft_in,
							pMfccVar->hamWin,
							pMfccVar->nFrameSize);

	// update frame buffer for next processing
	memcpy(pMfccData->prevBuf, pMfccData->prevBuf+pMfccVar->nFrameShift, (size_t)pMfccVar->nFrameShift*sizeof(hci_mfcc16));
	memcpy(pMfccData->prevBuf+pMfccVar->nFrameShift, inBuf, (size_t)pMfccVar->nFrameShift*sizeof(hci_mfcc16));


	// FFT
#ifdef FIXED_POINT_FE
	FX_SigProc_realFFT(xfft,
					   pMfccVar->phs_tbl,
					   pMfccVar->nFFTSize,
					   pMfccVar->nHalfFFT,
					   pMfccVar->nFFTStage);
#else	// !FIXED_POINT_FE
	pffft_transform_ordered(pMfccVar->fftSetup, xfft_in, xfft_out, 0, PFFFT_FORWARD);
#endif	// #ifdef FIXED_POINT_FE

	// kklee 20150924
	memcpy(pMfccData->xfft, xfft_out, pMfccVar->nFFTSize*sizeof(xfft_out[0]));
	pMfccData->xfft_len = pMfccVar->nFFTSize;

#ifdef FIXED_POINT_FE
	var_shift += PowerASR_BasicOP_getNormalShiftCountOfVector_16_16(xfft, pMfccVar->nFFTSize, 0);
	var_shift -= pMfccVar->nFFTStage;
#endif	// #ifdef FIXED_POINT_FE

	// FFT bins --> mel power spectrum
	memset(melSpecPower, 0, sizeof(melSpecPower));
	if (pMfccVar->nMFCCType == HTK_MFCC) {
		FX_SigProc_FFT2HTKMelSpectrum(xfft_out,
									  melSpecPower,
									  pMfccData,
									  pMfccVar,
									  var_shift);
	}
	else if (pMfccVar->nMFCCType == ETSI_MFCC) {
		FX_SigProc_FFT2ETSIMelSpectrum(xfft_out,
									   melSpecPower,
									   pMfccVar,
									   var_shift);
	}
	else if (pMfccVar->nMFCCType == DPS_MFCC) {
		FX_SigProc_FFT2MelDiffPowerSpectrum(xfft_out,
											melSpecPower,
											pMfccVar,
											var_shift);
	}
	else {		// PDPS_MFCC
		FX_SigProc_FFT2MelPredictiveDiffPowerSpectrum(xfft_out,
													  melSpecPower,
													  pMfccVar,
													  var_shift);
	}

	// add frame energy into static feature vector
	if (pMfccVar->nEnergyType == TIME_ENERGY) {
		pMfccData->mfccVec[pMfccVar->nNumCepstra] = nLogPower;
	}
	else {		// FREQ_ENERGY
		hci_mfcc32 nLogEnergy = 0;

		// compute log frame energy
		nLogEnergy = FX_SigProc_computeLogFrameEnergy(pMfccData, pMfccVar, var_shift);
		pMfccData->mfccVec[pMfccVar->nNumCepstra] = nLogEnergy;

		//------RevByKSH_2015/04/23----------------//
//		if(pMfccData->nInputFrame >= 3 && pMfccData->nInputFrame <= 12){ //KSH MIN_LOG_E,MAX_LOG_E,MAX_LOG_P 값 초기 10frame ~ 20frame 추정치로 세팅 2015.04.23
		if (pMfccData->nInputFrame <= 12) { //KSH MIN_LOG_E,MAX_LOG_E,MAX_LOG_P 값 초기 2frame ~ 11frame 추정치로 세팅 2015.04.23
			if (nLogEnergy > 19.0 && pMfccData->nInputFrame < 6) {
//				pMfccData->averLogEnergy += 19.0f;
				pMfccData->minLogEnergy = 16.5;
				pMfccData->maxLogEnergy = 20.5;
			}
			else {
				hci_mfcc32 tmpAverLogEnergy;
				pMfccData->cnt_EstiFrames++;
				pMfccData->averLogEnergy += HCI_MIN(19.0f, nLogEnergy);
				tmpAverLogEnergy = pMfccData->averLogEnergy / pMfccData->cnt_EstiFrames;

				if (tmpAverLogEnergy < 6) {
					pMfccData->minLogEnergy = 14.0;
					pMfccData->maxLogEnergy = 17.1;
				}
				else if (tmpAverLogEnergy < 7) {
					pMfccData->minLogEnergy = 14.2;
					pMfccData->maxLogEnergy = 17.2;
				}
				else if (tmpAverLogEnergy < 8) {
					pMfccData->minLogEnergy = 14.5;
					pMfccData->maxLogEnergy = 17.3;
				}
				else if (tmpAverLogEnergy < 9) {
					pMfccData->minLogEnergy = 14.8;
					pMfccData->maxLogEnergy = 17.4;
				}
				else if (tmpAverLogEnergy < 10) {
					pMfccData->minLogEnergy = 15.0;
					pMfccData->maxLogEnergy = 17.8;
				}
				else if (tmpAverLogEnergy < 11) {
					pMfccData->minLogEnergy = 15.2;
					pMfccData->maxLogEnergy = 18;
				}
				else if (tmpAverLogEnergy < 12) {
					pMfccData->minLogEnergy = 15.4;
					pMfccData->maxLogEnergy = 19.9;
				}
				else if (tmpAverLogEnergy < 13) {
					pMfccData->minLogEnergy = 15.7;
					pMfccData->maxLogEnergy = 20;
				}
				else if (tmpAverLogEnergy < 14) {
					pMfccData->minLogEnergy = 16;
					pMfccData->maxLogEnergy = 20.2;
				}
				else if (tmpAverLogEnergy < 15) {
					pMfccData->minLogEnergy = 17.5;
					pMfccData->maxLogEnergy = 20.5;
				}
				else if (tmpAverLogEnergy < 16) {
					pMfccData->minLogEnergy = 17.8;
					pMfccData->maxLogEnergy = 21;
				}
				else if (tmpAverLogEnergy < 17) {
					pMfccData->minLogEnergy = 18;
					pMfccData->maxLogEnergy = 21;
				}
				else if (tmpAverLogEnergy < 18) {
					pMfccData->minLogEnergy = 18.5;
					pMfccData->maxLogEnergy = 21.5;
				}
				else if (tmpAverLogEnergy < 19) {
					pMfccData->minLogEnergy = 19;
					pMfccData->maxLogEnergy = 22;
				}
				else if (tmpAverLogEnergy < 21) {
					pMfccData->minLogEnergy = 19.2;
					pMfccData->maxLogEnergy = 22.2;
				}
				else {
					pMfccData->minLogEnergy = 19.3;
					pMfccData->maxLogEnergy = 22.5;
				}
//				printf("[currentLEnergy]%f [aver]%f [TH] %.3f/%.3f \n",nLogEnergy,tmpAverLogEnergy,pMfccData->minLogEnergy,pMfccData->maxLogEnergy);
			}
		}

		//-----------------------------------------//
//		pMfccVar->minLogEnergy = 15.0f;//KSH_For Debug 2015.09.07
//		pMfccVar->maxLogEnergy = 20.0f;
	}

	// compute spectral entropy used in EPD module
	pMfccData->nSpecEntropy = FX_SigProc_computeSpectralEntropy(pMfccData, pMfccVar, flagVAD, var_shift);
	if (pMfccVar->addEntropy) {
		pMfccData->mfccVec[pMfccData->nDimMFCC - 1] = pMfccData->nSpecEntropy;
	}

    //KKLEE : melscale filter bank 
    memcpy(pMfccData->fbankLogEn, melSpecPower, NUM_FILTERBANK * sizeof(hci_mfcc_t));
	// mel power spectrum --> cepstrum
	FX_SigProc_MelSpectrum2MFCC(pMfccData->mfccVec,
								melSpecPower,
								pMfccVar->dctCosTable,
								pMfccVar->nNumCepstra,
								pMfccVar->nNumFilters,
								pMfccVar->weightC0);

	// cepstral weighting
	if (pMfccVar->nCepLiftOrder > 0) {
		FX_SigProc_weightCepstrum(pMfccData->mfccVec,
								  pMfccVar->cepWin,
								  pMfccVar->nNumCepstra);
	}

	// increment frame index
	pMfccData->nMfccFrame = PowerASR_BasicOP_add_32_32(pMfccData->nMfccFrame, 1);

	return 0;
}


/**
 *	For small-valued frame buffer, add a randomly generated vector (-30~+30)
 */
HCILAB_PRIVATE hci_int32
_FX_Wave2Mfcc_checkAllZeroSamples(hci_int16 *pFrameBuf,		///< In : frame sample buffer
								  hci_int16 nFrameLen)		///< In : frame length in sample count
{
	hci_int16 *pSample = 0;
	hci_int16 *pLastSample = 0;
	hci_int16 threshSample = 0x0000;	//0x0080;	//0x0100;
	hci_int16 valRand = 0;
	hci_int16 rand_width = 0x0020;
	hci_int16 rand_swing = rand_width>>1;

	pSample = pFrameBuf;
	pLastSample = pFrameBuf + nFrameLen;
	while (pSample < pLastSample) {
		if (HCI_ABS(*pSample) > threshSample) return 0;
		pSample++;
		if (HCI_ABS(*pSample) > threshSample) return 0;
		pSample++;
		if (HCI_ABS(*pSample) > threshSample) return 0;
		pSample++;
		if (HCI_ABS(*pSample) > threshSample) return 0;
		pSample++;
		if (HCI_ABS(*pSample) > threshSample) return 0;
		pSample++;
		if (HCI_ABS(*pSample) > threshSample) return 0;
		pSample++;
		if (HCI_ABS(*pSample) > threshSample) return 0;
		pSample++;
		if (HCI_ABS(*pSample) > threshSample) return 0;
		pSample++;
	}

	// for small-valued frame buffer, add a randomly generated vector (-32~+32)
	pSample = pFrameBuf;
	pLastSample = pFrameBuf + nFrameLen;
	while (pSample < pLastSample) {
		valRand = (hci_int16)(rand()%rand_width - rand_swing);
		*pSample = PowerASR_BasicOP_add_16_16(*pSample, valRand);
		pSample++;
		valRand = (hci_int16)(rand()%rand_width - rand_swing);
		*pSample = PowerASR_BasicOP_add_16_16(*pSample, valRand);
		pSample++;
		valRand = (hci_int16)(rand()%rand_width - rand_swing);
		*pSample = PowerASR_BasicOP_add_16_16(*pSample, valRand);
		pSample++;
		valRand = (hci_int16)(rand()%rand_width - rand_swing);
		*pSample = PowerASR_BasicOP_add_16_16(*pSample, valRand);
		pSample++;
		valRand = (hci_int16)(rand()%rand_width - rand_swing);
		*pSample = PowerASR_BasicOP_add_16_16(*pSample, valRand);
		pSample++;
		valRand = (hci_int16)(rand()%rand_width - rand_swing);
		*pSample = PowerASR_BasicOP_add_16_16(*pSample, valRand);
		pSample++;
		valRand = (hci_int16)(rand()%rand_width - rand_swing);
		*pSample = PowerASR_BasicOP_add_16_16(*pSample, valRand);
		pSample++;
		valRand = (hci_int16)(rand()%rand_width - rand_swing);
		*pSample = PowerASR_BasicOP_add_16_16(*pSample, valRand);
		pSample++;
	}

	return 0;
}

/* end of file */






















