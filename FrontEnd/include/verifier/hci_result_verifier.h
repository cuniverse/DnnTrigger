
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
 *	@file	hci_result_verifier.h
 *	@ingroup verifier_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	PowerASR ASR Result verification library
 */

#ifndef __HCILAB_RESULT_VERIFIER_H__
#define __HCILAB_RESULT_VERIFIER_H__

#include "base/hci_type.h"
#include "basic_op/basic_op.h"
#include "basic_op/fixpoint.h"
#include "lexicon/lexicon_common.h"
#include "phone_decoder/phosearch_common.h"
#include "symbol_match/smatch_common.h"

#if defined(HCI_MSC_32)
#ifdef HCI_RESULTVERIFIER_EXPORTS
#define HCI_VERIFIER_API __declspec(dllexport)
#elif defined(HCI_RESULTVERIFIER_IMPORTS)
#define HCI_VERIFIER_API __declspec(dllimport)
#else	// in case of static library
#define HCI_VERIFIER_API
#endif // #ifdef HCI_RESULTVERIFIER_EXPORTS
#elif defined(HCI_OS2)
#define HCI_VERIFIER_API
#else
#define HCI_VERIFIER_API HCI_USER
#endif

/** PowerASR Result Verifier */
typedef struct {
	void *pInner;
} PowerASR_ResultVerifier;

#ifdef __cplusplus
extern "C" {
#endif

/**
 *	create a new ASR result verifier.
 *
 *	@return Return the pointer to a newly created ASR result verifier
 */
HCILAB_PUBLIC HCI_VERIFIER_API PowerASR_ResultVerifier*
PowerASR_ResultVerifier_new();

/**
 *	delete the ASR result verifier.
 */
HCILAB_PUBLIC HCI_VERIFIER_API void
PowerASR_ResultVerifier_delete(PowerASR_ResultVerifier *pThis
);

/**
 *	set-up environments for ASR result verifier,
 *	and allocate necessary memories.
 *
 *	@return Return 0 if ASR result verifier environments are set-up correctly, otherwise return -1.
 */
HCILAB_PUBLIC HCI_VERIFIER_API hci_int32
PowerASR_ResultVerifier_openResultVerifier(PowerASR_ResultVerifier *pThis,	///< (i/o) pointer to the ASR result verifier
										   const char *pszConfigFile		///< (i) result verifier configuration file
);

/**
 *	free memories allocated to the ASR result verifier.
 *
 *	@return Return 0 if all memories are freed successfully, otherwise return -1.
 */
HCILAB_PUBLIC HCI_VERIFIER_API hci_int32
PowerASR_ResultVerifier_closeResultVerifier(PowerASR_ResultVerifier *pThis	///< (i/o) pointer to the ASR result verifier
);

/**
 *	initialize data buffers for ASR result verifier.
 *
 *	@return Return 0 if user-specific data buffers are initialized successfully, otherwise return -1.
 */
HCILAB_PUBLIC HCI_VERIFIER_API hci_int32
PowerASR_ResultVerifier_initializeResultVerifier(PowerASR_ResultVerifier *pThis	///< (i) pointer to the ASR result verifier
);

/**
 *	decide acceptance/rejection status of a given candidate.
 *
 *	@return Return TRUE if a given candidate is accepted, otherwise return FALSE.
 */
HCILAB_PUBLIC HCI_VERIFIER_API hci_flag
PowerASR_ResultVerifier_verifyRecognitionCandidate(PowerASR_ResultVerifier *pThis,	///< (i) pointer to the ASR result verifier
												   hci_score_t *confidScore,		///< (o) confidence score
												   const hci_score_t matchScore,	///< (i) matching score of a given candidate
												   const hci_score_t fillerScore,	///< (i) filler score
												   const hci_uint32 nLenFrame		///< (i) input feature stream length
);

/**
 *	get confidence score threshold value.
 *
 *	@return Return confidence score threshold value.
 */
HCILAB_PUBLIC HCI_VERIFIER_API hci_int32
PowerASR_ResultVerifier_getConfidenceScoreThreshold(PowerASR_ResultVerifier *pThis	///< (i) pointer to the ASR result verifier
);

/**
 *	set confidence score threshold value.
 *
 *	@return Return 0 if confidence score threshold value is set successfully.
 */
HCILAB_PUBLIC HCI_VERIFIER_API hci_int32
PowerASR_ResultVerifier_setConfidenceScoreThreshold(PowerASR_ResultVerifier *pThis,	///< (i/o) pointer to the ASR result verifier
													const hci_int32 confidScoreTh	///< (i) confidence score threshold
);

#ifdef __cplusplus
}
#endif

#endif // #ifndef __HCILAB_RESULT_VERIFIER_H__

