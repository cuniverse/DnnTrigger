
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
 *	@file	hci_epd.h
 *	@ingroup epd_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	API functions for PowerASR end-point detector
 */

#ifndef __HCILAB_EPD_H__
#define __HCILAB_EPD_H__

#include "base/hci_type.h"
#include "epd/epd_common.h"

#if defined(HCI_MSC_32)
#ifdef HCI_EPD_EXPORTS
#define HCI_EPD_API __declspec(dllexport)
#elif defined(HCI_EPD_IMPORTS)
#define HCI_EPD_API __declspec(dllimport)
#else	// in case of static library
#define HCI_EPD_API
#endif // #ifdef HCI_EPD_EXPORTS
#elif defined(HCI_OS2)
#define HCI_EPD_API
#else
#define HCI_EPD_API HCI_USER
#endif

/** PowerASR End-Point detector */
typedef struct {
	void *pInner;
} PowerASR_EPD;


#ifdef __cplusplus
extern "C" {
#endif

/**
 *	create a new end-point detector.
 *
 *	@return Return the pointer to a newly created end-point detector
 */
HCILAB_PUBLIC HCI_EPD_API PowerASR_EPD*
PowerASR_EPD_new(
);

/**
 *	delete the end-point detector
 */
HCILAB_PUBLIC HCI_EPD_API void
PowerASR_EPD_delete(PowerASR_EPD *pThis
);

/**
 *	set-up environments for end-point detector,
 *	and allocate necessary memories.
 *
 *	@return Return 0 if EPD environments are set-up correctly, otherwise return -1.
 */
HCILAB_PUBLIC HCI_EPD_API hci_int32
PowerASR_EPD_openEPDetector(PowerASR_EPD *pThis,			///< (i/o) pointer to the end-point detector
							const char *pszConfigFile		///< (i) EPD configuration file
);

/**
 *	free memories allocated to the end-point detector.
 *
 *	@return Return 0 if all memories are freed successfully, otherwise return -1.
 */
HCILAB_PUBLIC HCI_EPD_API hci_int32
PowerASR_EPD_closeEPDetector(PowerASR_EPD *pThis		///< (i/o) pointer to the end-point detector
);

/**
 *	initialize data buffers for end-point detector.
 *
 *	@return Return 0 if EPD user data are initialized successfully, otherwise return -1.
 */
HCILAB_PUBLIC HCI_EPD_API hci_int32
PowerASR_EPD_initializeEPDetector(PowerASR_EPD *pThis,
								  EPD_UserData *pEpdData,			///< (i/o) EPD user data
								  const hci_flag bContinuousEPD		///< (i) flag to continuous-mode EPD
);

/**
 *	Given an input frame buffer, detect end-points of spoken utterance.
 *
 *	@return Return EPD status.
 */
HCILAB_PUBLIC HCI_EPD_API hci_flag
PowerASR_EPD_detectEndPoint(PowerASR_EPD *pThis,
							EPD_UserData *pEpdData,		///< (o) EPD user data
							hci_epd32 frameLogEnergy,	///< (i) frame log energy
							hci_epd16 frameLogPower,	///< (i) frame log power
							hci_epd32 specEntropy,		///< (i) spectral entropy
							hci_int32 flagNRVad,		///< (i) VAD output in noise-reduction module
							hci_int32 bSpeechFound		///< (i) flag to speech found
);

#ifdef __cplusplus
}
#endif

#endif // #ifndef __HCILAB_EPD_H__

