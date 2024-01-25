
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
 *	@file	hci_phone_decoder.h
 *	@ingroup phoneDec_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	PowerASR phonetic decoder library
 */

#ifndef __HCILAB_PHONE_DECODER_H__
#define __HCILAB_PHONE_DECODER_H__

#include "base/hci_type.h"
#include "basic_op/basic_op.h"
#include "basic_op/fixpoint.h"
#include "hmm/hmm_common.h"
#include "state_oprob/oprob_common.h"
#include "phone_decoder/phosearch_common.h"
#include "viterbi/hci_viterbi_scorer.h"

#if defined(HCI_MSC_32)
#ifdef HCI_PHONETICDECODER_EXPORTS
#define HCI_PHONEDEC_API __declspec(dllexport)
#elif defined(HCI_PHONETICDECODER_IMPORTS)
#define HCI_PHONEDEC_API __declspec(dllimport)
#else	// in case of static library
#define HCI_PHONEDEC_API
#endif // #ifdef HCI_PHONETICDECODER_EXPORTS
#elif defined(HCI_OS2)
#define HCI_PHONEDEC_API
#else
#define HCI_PHONEDEC_API HCI_USER
#endif

/** PowerASR Phonetic Decoder */
typedef struct {
	void *pInner;
} PowerASR_PhoneDecoder;

#ifdef __cplusplus
extern "C" {
#endif

/**
 *	create a new phonetic decoder.
 *
 *	@return Return the pointer to a newly created phonetic decoder
 */
HCILAB_PUBLIC HCI_PHONEDEC_API PowerASR_PhoneDecoder*
PowerASR_PhoneDecoder_new();

/**
 *	delete the phonetic decoder.
 */
HCILAB_PUBLIC HCI_PHONEDEC_API void
PowerASR_PhoneDecoder_delete(PowerASR_PhoneDecoder *pThis
);

/**
 *	set-up environments for phonetic decoder,
 *	and allocate necessary memories.
 *
 *	@return Return 0 if phonetic decoder environments are set-up correctly, otherwise return -1.
 */
HCILAB_PUBLIC HCI_PHONEDEC_API hci_int32
PowerASR_PhoneDecoder_openPhoneticDecoder(PowerASR_PhoneDecoder *pThis,		///< (i/o) pointer to the phonetic decoder
										  PowerASR_ViterbiScorer *pViterbi,	///< (i) pointer to the Viterbi scorer
										  const AM_Resource *pResAM,		///< (i) pointer to HMM resource
										  const char *pszConfigFile			///< (i) phonetic decoder configuration file
);

/**
 *	free memories allocated to the phonetic decoder.
 *
 *	@return Return 0 if all memories are freed successfully, otherwise return -1.
 */
HCILAB_PUBLIC HCI_PHONEDEC_API hci_int32
PowerASR_PhoneDecoder_closePhoneticDecoder(PowerASR_PhoneDecoder *pThis		///< (i/o) pointer to the phonetic decoder
);

/**
 *	initialize data buffers for phonetic decoder.
 *
 *	@return Return 0 if user-specific data buffers are initialized successfully, otherwise return -1.
 */
HCILAB_PUBLIC HCI_PHONEDEC_API hci_int32
PowerASR_PhoneDecoder_initializePhoneticDecoder(PowerASR_PhoneDecoder *pThis,	///< (i) pointer to the phonetic decoder
												PhoneSpace *pPhoneSpace			///< (o) channel-specific phone search space
);

/**
 *	initiate phone recognition.
 *
 *	@return Return 0 if phone recognition is initiated successfully, otherwise return -1.
 *
 *	@see PhoneSearch_initiatePhoneRecognition
 */
HCILAB_PUBLIC HCI_PHONEDEC_API hci_int32
PowerASR_PhoneDecoder_initiatePhoneRecognition(PowerASR_PhoneDecoder *pThis,	///< (i) pointer to the phonetic decoder
											   PhoneSpace *pPhoneSpace,			///< (o) channel-specific phone search space
											   OPROB_TABLE *pLLTable			///< (o) channel-specific state LL table
);


/**
 *	proceed frame-synchronous phone recognition.
 *
 *	@return Return 0 if phone recognition is proceeded successfully, otherwise return -1.
 *
 *	@see PhoneSearch_proceedFrameSyncForwardSearch
 */
HCILAB_PUBLIC HCI_PHONEDEC_API hci_int32
PowerASR_PhoneDecoder_framesyncPhoneRecognition(PowerASR_PhoneDecoder *pThis,	///< (i) pointer to the phonetic decoder
												PhoneSpace *pPhoneSpace,		///< (o) channel-specific phone search space
												OPROB_TABLE *pLLTable,			///< (i/o) channel-specific state LL table
												const hci_uint32 nFrameId		///< (i) frame index
);

/**
 *	find N-best phone recognition results by backward A* search.
 *
 *	@return Return 0 if N-best results are found successfully, otherwise return -1.
 *
 *	@see PhoneSearch_findPhoneCandidatesByBackwardSearch
 */
HCILAB_PUBLIC HCI_PHONEDEC_API hci_int32
PowerASR_PhoneDecoder_findNBestPhoneResult(PowerASR_PhoneDecoder *pThis,	///< (i) pointer to the phonetic decoder
										   PhoneSpace *pPhoneSpace,			///< (o) channel-specific phone search space
										   PHONE_RESULT *pResult,			///< (o) phone recognition results
										   const OPROB_TABLE *pLLTable,		///< (i) channel-specific state LL table
										   const hci_uint32 nLenFrames		///< (i) frame length
);

/**
 *	find set phone insertion penalty value as a given value.
 *
 *	@return Return 0.
 */
HCILAB_PUBLIC HCI_PHONEDEC_API hci_int32
PowerASR_PhoneDecoder_setPhoneInsertionPenalty(PowerASR_PhoneDecoder *pThis,	///< (i/o) pointer to the phonetic decoder
											   const hci_int32 nPhoneInstPnt	///< (i) phone insertion penalty value
);

#ifdef __cplusplus
}
#endif

#endif // #ifndef __HCILAB_PHONE_DECODER_H__

