
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
 *	@file	hci_wave2mfcc.h
 *	@ingroup wave2mfcc_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	PowerASR wave-to-mfcc converter library
 */

#ifndef __HCILAB_WAVE2MFCC_H__
#define __HCILAB_WAVE2MFCC_H__

#include "base/hci_type.h"
#include "basic_op/fixpoint.h"
#include "basic_op/hci_logadd.h"
#include "wave2mfcc/fx_mfcc_common.h"

#if defined(HCI_MSC_32)
#ifdef HCI_WAVE2MFCC_EXPORTS
#define HCI_WAVE2MFCC_API __declspec(dllexport)
#elif defined(HCI_WAVE2MFCC_IMPORTS)
#define HCI_WAVE2MFCC_API __declspec(dllimport)
#else	// in case of static library
#define HCI_WAVE2MFCC_API
#endif // #ifdef HCI_WAVE2MFCC_EXPORTS
#elif defined(HCI_OS2)
#define HCI_WAVE2MFCC_API
#else
#define HCI_WAVE2MFCC_API HCI_USER
#endif

/**
 *	@enum Wave-to-MFCC status flag
 */
typedef enum {
	W2M_FAIL=-1,	///< wave-to-mfcc operations failed
	W2M_FALSE,		///< there are no emitting MFCC vectors
	W2M_TRUE		///< output a single MFCC vector
} W2M_Status;

/** PowerASR wave-to-mfcc converter */
typedef struct _PowerASR_FX_Wave2Mfcc{
	void *pInner;
} PowerASR_FX_Wave2Mfcc;


#ifdef __cplusplus
extern "C" {
#endif

/**
 *	create a new wave-to-mfcc converter.
 *
 *	@return Return the pointer to a newly created wave-to-mfcc converter
 */
HCILAB_PUBLIC HCI_WAVE2MFCC_API PowerASR_FX_Wave2Mfcc*
PowerASR_FX_Wave2Mfcc_new(
);

/**
 *	delete the wave-to-mfcc converter.
 */
HCILAB_PUBLIC HCI_WAVE2MFCC_API void
PowerASR_FX_Wave2Mfcc_delete(PowerASR_FX_Wave2Mfcc *pThis
);

/**
 *	set-up environments for wave-to-mfcc converter,
 *	and allocate necessary memories.
 *
 *	@return return 0 if Wave2Mfcc environments are set-up correctly, otherwise return -1.
 *
 *	@see FX_Wave2Mfcc_buildFeatureExtractor
 */
HCILAB_PUBLIC HCI_WAVE2MFCC_API hci_int32
PowerASR_FX_Wave2Mfcc_openWave2MfccConverter(PowerASR_FX_Wave2Mfcc *pThis,	///< (i/o) pointer to the wave-to-mfcc converter
											 const char *pszConfigFile,		///< (i) Wave2Mfcc configuration file
											 const hci_int32 nSampleRate,	///< (i) sampling frequency in Hz
											 hci_logadd_t *pLogAddTbl		///< (i) pointer to log-addition table
);

/**
 *	free memories allocated to the wave-to-mfcc converter.
 *
 *	@return return 0 if all memories are freed successfully, otherwise return -1.
 */
HCILAB_PUBLIC HCI_WAVE2MFCC_API hci_int32
PowerASR_FX_Wave2Mfcc_closeWave2MfccConverter(PowerASR_FX_Wave2Mfcc *pThis	///< (i/o) pointer to the wave-to-mfcc converter
);

/**
 *	initialize data buffers for wave-to-mfcc converter.
 *
 *	@return return 0 if Wave2Mfcc data buffers are initialized successfully, otherwise return -1.
 */
HCILAB_PUBLIC HCI_WAVE2MFCC_API hci_int32
PowerASR_FX_Wave2Mfcc_initializeWave2MfccConverter(PowerASR_FX_Wave2Mfcc *pThis,	///< (i) pointer to the wave-to-mfcc converter
												   MFCC_UserData *pMfccData			///< (o) channel-specific Wave2Mfcc data struct
);

/**
 *	convert a single input frame samples to a MFCC vector.
 *
 *	@return return W2M_FAIL, W2M_FALSE, W2M_TRUE.
 *		- return W2M_FAIL if Wave2Mfcc operation failed
 *		- return W2M_FALSE if returned outputs don't exist
 *		- return W2M_TRUE if returned outputs exist
 *
 *	@see FX_Wave2Mfcc_convertSingleFrameToMfccVector
 */
HCILAB_PUBLIC HCI_WAVE2MFCC_API W2M_Status
PowerASR_FX_Wave2Mfcc_convertFrameSample2Mfcc(PowerASR_FX_Wave2Mfcc *pThis,		///< (i) pointer to the wave-to-mfcc converter
											  MFCC_UserData *pMfccData,			///< (o) channel-specific Wave2Mfcc data struct
											  hci_int16 *pFrameBuf,				///< (i/o) an input frame buffer
											  const hci_int16 flagVAD			///< (i) VAD flag (0 = unvoiced, 1 = voiced)
);

#ifdef __cplusplus
}
#endif

#endif // #ifndef __HCILAB_WAVE2MFCC_H__

