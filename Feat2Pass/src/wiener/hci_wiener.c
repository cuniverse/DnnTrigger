
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
 *	@file	hci_wiener.c
 *	@ingroup noise_reducer_src
 *	@date	2010/08/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	Modules for HCILAB noise reducer based on Wiener filtering
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "base/hci_type.h"
#include "base/hci_msg.h"
#include "base/hci_malloc.h"

#include "wiener/ParmInterface.h"
#include "wiener/hci_wiener.h"
#include "base/parse_config.h"
#include "base/case.h"

/** inner data struct for Wiener noise reducer */
typedef struct 
{
	hci_boolean bUseDithering;
	hci_int32 nSampleRate;			///< sampling frequency in Hz
} Wiener_Inner;


// local functions
#ifdef __cplusplus
extern "C" {
#endif 

#ifdef __cplusplus
}
#endif

/**
 *	create a new Wiener noise reducer.
 *
 *	@return Return the pointer to a newly created Wiener noise reducer
 */
HCILAB_PUBLIC HCI_WIENER_API PowerASR_NR_Wiener*
PowerASR_NR_Wiener_new()
{
	PowerASR_NR_Wiener *pNR_Wiener = 0;
	Wiener_Inner *pInner = 0;

	pNR_Wiener = (PowerASR_NR_Wiener *) hci_malloc( sizeof(PowerASR_NR_Wiener) );

	if ( pNR_Wiener ) {
		memset(pNR_Wiener, 0, sizeof(PowerASR_NR_Wiener));

		pInner = (Wiener_Inner *) hci_malloc( sizeof(Wiener_Inner) );
		if ( pInner ) {
			memset(pInner, 0, sizeof(Wiener_Inner));
			pNR_Wiener->pInner = (void *)pInner;
		}
	}
	else {
		HCIMSG_ERROR("cannot create Wiener noise reducer.\n");
	}

	return pNR_Wiener;
}


/**
 *	delete the Wiener noise reducer.
 */
HCILAB_PUBLIC HCI_WIENER_API void
PowerASR_NR_Wiener_delete(PowerASR_NR_Wiener *pThis)
{
	Wiener_Inner *pInner = 0;

	if (0 == pThis) {
		return;
	}

	pInner = (Wiener_Inner *) pThis->pInner;

	if ( pInner ) hci_free(pInner);
	hci_free(pThis);
}


/**
 *	set-up environments for Wiener noise reducer,
 *	and allocate necessary memories.
 *
 *	@return Return 0 if noise reducer environments are set-up correctly, otherwise return -1.
 */
HCILAB_PUBLIC HCI_WIENER_API hci_int32
PowerASR_NR_Wiener_openWienerNR(PowerASR_NR_Wiener *pThis,		///< (i/o) pointer to the Wiener noise reducer
								const char *pszConfigFile,		///< (i) Wiener configuration file
								const hci_int32 nSampleRate)	///< (i) sampling frequency in Hz
{
	Wiener_Inner *pInner = 0;
	char *pszValue = 0;

	if (0 == pThis) {
		return -1;
	}

	pInner = (Wiener_Inner *) pThis->pInner;

	pInner->nSampleRate = nSampleRate;

	
	if (pszConfigFile != NULL) {
		if (0 != PowerASR_Base_parseConfigFile(pszConfigFile)) {
			HCIMSG_ERROR("parseConfigFile failed (%s).\n", pszConfigFile);
			return -1;
		}

		// White Noise Mixer Option 
		pszValue = PowerASR_Base_getArgumentValue("USE_DITHERING");
		if (pszValue != NULL) {
			if (PowerASR_Base_strnocasecmp(pszValue, "yes") == 0) {
				pInner->bUseDithering = TRUE;
			} 
			else {
				pInner->bUseDithering = FALSE;
			}
		} else {
			pInner->bUseDithering = TRUE;
		}
	} else {
		pInner->bUseDithering = TRUE;
	}
	return 0;
}


/**
 *	free memories allocated to the Wiener noise reducer.
 *
 *	@return Return 0 if all memories are freed successfully, otherwise return -1.
 */
HCILAB_PUBLIC HCI_WIENER_API hci_int32
PowerASR_NR_Wiener_closeWienerNR(PowerASR_NR_Wiener *pThis)		///< (i/o) pointer to the Wiener noise reducer
{
	return (pThis ? 0 : -1);
}


/**
 *	initialize data buffers for Wiener noise reducer.
 *
 *	@return Return 0 if noise reducer buffers are initialized successfully, otherwise return -1.
 */
HCILAB_PUBLIC HCI_WIENER_API hci_int32
PowerASR_NR_Wiener_initializeWienerNR(PowerASR_NR_Wiener *pThis,		///< (i) pointer to the Wiener noise reducer
									  Wiener_UserData *pUserData)		///< (o) channel-specific Wiener data
{
	Wiener_Inner *pInner = 0;

	if (0 == pThis) {
		return -1;
	}
	if (0 == pUserData) {
		return -1;
	}

	pInner = (Wiener_Inner *) pThis->pInner;

	if ( pUserData->dataWiener == 0 ) {
		pUserData->dataWiener = (void *)AdvProcessAlloc(8000);
	}

    if ( pUserData->dataWiener ) {
		AdvProcessInit ((FEParamsX *)pUserData->dataWiener);
		return 0;
	}
	else {
		return -1;
	}
}


/**
 *	release memories allocated to data buffers for Wiener noise reducer.
 *
 *	@return Return 0 if memories of Wiener data buffers are released successfully, otherwise return -1.
 */
HCILAB_PUBLIC HCI_WIENER_API hci_int32
PowerASR_NR_Wiener_releaseWienerNR(PowerASR_NR_Wiener *pThis,		///< (i) pointer to the Wiener noise reducer
								   Wiener_UserData *pUserData)		///< (o) channel-specific Wiener data
{
	FEParamsX *dataWiener = 0;

	if (0 == pThis) {
		return -1;
	}
	if (0 == pUserData) {
		return -1;
	}

    if ( pUserData->dataWiener ) {
		dataWiener = (FEParamsX *)pUserData->dataWiener;
		AdvProcessDelete ( &dataWiener );
		pUserData->dataWiener = 0;
	}

	return 0;
}


/**
 *	produce noise-reduced samples from a noisy input frame buffer.
 *
 *	@return Return the length of noise-reduced samples.
 */
HCILAB_PUBLIC HCI_WIENER_API hci_int32
PowerASR_NR_Wiener_procFrameBuffer(PowerASR_NR_Wiener *pThis,		///< (i) pointer to the Wiener noise reducer
								   Wiener_UserData *pUserData,		///< (i/o) channel-specific Wiener data
								   hci_int16 *pFrameBuf)			///< (i/o) input frame buffer, noise-reduced frame buffer
{
	Wiener_Inner *pInner = 0;
	FEParamsX *dataWiener = 0;
	hci_int32 nShiftLen = FRAME_SHIFT;
	hci_int32 nLenOutSample = 0;
	hci_int16 OutputBuffer[2*FRAME_SHIFT];
	hci_int16 *pInput = pFrameBuf;

	if (0 == pThis) {
		return NR_FAIL;
	}
	if (0 == pUserData) {
		return NR_FAIL;
	}
	if (0 == pFrameBuf) {
		return NR_FAIL;
	}

	pInner = (Wiener_Inner *) pThis->pInner;

	dataWiener = (FEParamsX *)pUserData->dataWiener;

	memset( OutputBuffer, 0, sizeof(OutputBuffer) );
	if (pInner->bUseDithering == TRUE) {
		_NR_Wiener_dithering( pInput, (pInner->nSampleRate / 100) );
	}

	if ( DoAdvProcess (pInput, OutputBuffer, dataWiener) ) {
		nLenOutSample += nShiftLen;
		pUserData->flagVAD = dataWiener->FrameClass;
		pUserData->bSpeechFound = dataWiener->SpeechFoundMel;
		pUserData->specEntropy = dataWiener->specEntropy;
	}

	if ( pInner->nSampleRate == 16000 ) {
		pInput = pFrameBuf + nShiftLen;
		if ( DoAdvProcess (pInput, OutputBuffer+nLenOutSample, dataWiener) ) {
			nLenOutSample += nShiftLen;
			pUserData->flagVAD += dataWiener->FrameClass;
			pUserData->flagVAD = (pUserData->flagVAD + 1) / 2;
			pUserData->bSpeechFound += dataWiener->SpeechFoundMel;
			pUserData->bSpeechFound = (pUserData->bSpeechFound + 1) / 2;
			pUserData->specEntropy += dataWiener->specEntropy;
			pUserData->specEntropy *= 0.5f;
		}
	}

	memcpy( pFrameBuf, OutputBuffer, nLenOutSample*sizeof(hci_int16) );

	return nLenOutSample;
}


/**
*	Fixed-point Kalman 버전의 발산 문제를 해결하기 위해 dithering 적용
*/
HCILAB_PRIVATE void
_NR_Wiener_dithering(hci_int16 *pFrameBuf,		///< (i/o) frame sample buffer
hci_int16 nFrameShift)		///< (i) frame shift length
{
	hci_int16 valRand = 0;
	hci_int16 rand_width = 0;
	hci_int16 rand_swing = 0;
	hci_int16 dithering_factor = 128; //4; // 128 => 4      // 160109
	hci_int16 n = 0;

	rand_width = dithering_factor;
	rand_swing = (dithering_factor >> 1);
	//	srand(time(NULL));				// 실험마다 변동 막음 // 160109
	// add a randomly generated vector
	for (n = 0; n < nFrameShift; n++) {
		valRand = (hci_int16)(rand() % rand_width - rand_swing);

		if ((abs(pFrameBuf[n]) + rand_swing) < 32767) {  // 15. 12. 01 신호가 새츄레이션 될 경우 디더링 하지 않음 // 160109
			pFrameBuf[n] -= valRand;
		}
	}

}


// end of file
