
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
 *	@file	hci_wiener.h
 *	@ingroup noise_reducer_src
 *	@date	2010/08/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	Modules for HCILAB noise reducer based on Wiener filtering
 */

#ifndef __HCILAB_NR_WIENER_H__
#define __HCILAB_NR_WIENER_H__

#include "base/hci_type.h"
#include "wiener/wiener_common.h"

#if defined(HCI_MSC_32)
#ifdef HCI_NR_WIENER_EXPORTS
#define HCI_WIENER_API __declspec(dllexport)
#elif defined(HCI_NR_WIENER_IMPORTS)
#define HCI_WIENER_API __declspec(dllimport)
#else	// in case of static library
#define HCI_WIENER_API
#endif // #ifdef HCI_NR_WIENER_EXPORTS
#elif defined(HCI_OS2)
#define HCI_WIENER_API
#else
#define HCI_WIENER_API HCI_USER
#endif

#ifndef _NR_Status_defined
typedef enum {NR_FAIL=-1, NR_FALSE, NR_TRUE} NR_Status;
#define _NR_Status_defined
#endif

#ifdef __cplusplus
extern "C" {
#endif

/** PowerASR Wiener noise reducer */
typedef struct _PowerASR_NR_Wiener{
	void *pInner;
} PowerASR_NR_Wiener;

/**
 *	create a new Wiener noise reducer.
 *
 *	@return Return the pointer to a newly created Wiener noise reducer
 */
HCILAB_PUBLIC HCI_WIENER_API PowerASR_NR_Wiener*
PowerASR_NR_Wiener_new(
);

/**
 *	delete the Wiener noise reducer.
 */
HCILAB_PUBLIC HCI_WIENER_API void
PowerASR_NR_Wiener_delete(PowerASR_NR_Wiener *pThis
);

/**
 *	set-up environments for Wiener noise reducer,
 *	and allocate necessary memories.
 *
 *	@return Return 0 if noise reducer environments are set-up correctly, otherwise return -1.
 */
HCILAB_PUBLIC HCI_WIENER_API hci_int32
PowerASR_NR_Wiener_openWienerNR(PowerASR_NR_Wiener *pThis,		///< (i/o) pointer to the Wiener noise reducer
								const char *pszConfigFile,		///< (i) Wiener configuration file
								const hci_int32 nSampleRate		///< (i) sampling frequency in Hz
);

/**
 *	free memories allocated to the Wiener noise reducer.
 *
 *	@return Return 0 if all memories are freed successfully, otherwise return -1.
 */
HCILAB_PUBLIC HCI_WIENER_API hci_int32
PowerASR_NR_Wiener_closeWienerNR(PowerASR_NR_Wiener *pThis		///< (i/o) pointer to the Wiener noise reducer
);

/**
 *	initialize data buffers for Wiener noise reducer.
 *
 *	@return Return 0 if noise reducer buffers are initialized successfully, otherwise return -1.
 */
HCILAB_PUBLIC HCI_WIENER_API hci_int32
PowerASR_NR_Wiener_initializeWienerNR(PowerASR_NR_Wiener *pThis,		///< (i) pointer to the Wiener noise reducer
									  Wiener_UserData *pUserData		///< (o) channel-specific Wiener data
);

/**
 *	release memories allocated to data buffers for Wiener noise reducer.
 *
 *	@return Return 0 if memories of Wiener data buffers are released successfully, otherwise return -1.
 */
HCILAB_PUBLIC HCI_WIENER_API hci_int32
PowerASR_NR_Wiener_releaseWienerNR(PowerASR_NR_Wiener *pThis,		///< (i) pointer to the Wiener noise reducer
								   Wiener_UserData *pUserData		///< (o) channel-specific Wiener data
);

/**
 *	produce noise-reduced samples from a noisy input frame buffer.
 *
 *	@return Return the length of noise-reduced samples.
 */
HCILAB_PUBLIC HCI_WIENER_API hci_int32
PowerASR_NR_Wiener_procFrameBuffer(PowerASR_NR_Wiener *pThis,		///< (i) pointer to the Wiener noise reducer
								   Wiener_UserData *pUserData,		///< (i/o) channel-specific Wiener data
								   hci_int16 *pFrameBuf				///< (i/o) input frame buffer, noise-reduced frame buffer
);

/**
*	Fixed-point Kalman 버전의 발산 문제를 해결하기 위해 dithering 적용 
*/
HCILAB_PRIVATE void
_NR_Wiener_dithering(hci_int16 *pFrameBuf,		///< (i/o) frame sample buffer
                     hci_int16 nFrameShift);		///< (i) frame shift length

#ifdef __cplusplus
}
#endif

#endif // #ifndef __HCILAB_NR_WIENER_H__

