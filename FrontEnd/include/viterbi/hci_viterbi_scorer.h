
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
 *	@file	hci_viterbi_scorer.h
 *	@ingroup viterbi_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	PowerASR Viterbi Scoring library
 */

#ifndef __HCILAB_VITERBI_SCORER_H__
#define __HCILAB_VITERBI_SCORER_H__

#include "base/hci_type.h"
#include "basic_op/basic_op.h"
#include "basic_op/fixpoint.h"
#include "hmm/hmm_common.h"
#include "hmm/hci_resource_hmm.h"
#include "state_oprob/oprob_common.h"
#include "state_oprob/hci_state_oprob.h"
#include "viterbi/viterbi_common.h"

#if defined(HCI_MSC_32)
#ifdef HCI_VITERBISCORER_EXPORTS
#define HCI_VITERBI_API __declspec(dllexport)
#elif defined(HCI_VITERBISCORER_IMPORTS)
#define HCI_VITERBI_API __declspec(dllimport)
#else	// in case of static library
#define HCI_VITERBI_API
#endif // #ifdef HCI_VITERBISCORER_EXPORTS
#elif defined(HCI_OS2)
#define HCI_VITERBI_API
#else
#define HCI_VITERBI_API HCI_USER
#endif

/** PowerASR Viterbi Scorer */
typedef struct {
	void *pInner;
} PowerASR_ViterbiScorer;

#ifdef __cplusplus
extern "C" {
#endif

/**
 *	create a new Viterbi scorer.
 *
 *	@return Return the pointer to a newly created Viterbi Scorer
 */
HCILAB_PUBLIC HCI_VITERBI_API PowerASR_ViterbiScorer*
PowerASR_ViterbiScorer_new();

/**
 *	delete the Viterbi scorer.
 */
HCILAB_PUBLIC HCI_VITERBI_API void
PowerASR_ViterbiScorer_delete(PowerASR_ViterbiScorer *pThis
);

/**
 *	set-up environments for Viterbi scorer,
 *	and allocate necessary memories.
 *
 *	@return Return 0 if Viterbi scorer environments are set-up correctly, otherwise return -1.
 */
HCILAB_PUBLIC HCI_VITERBI_API hci_int32
PowerASR_ViterbiScorer_openViterbiScorer(PowerASR_ViterbiScorer *pThis,			///< (i/o) pointer to the Viterbi scorer
										 PowerASR_Resource_HMM *pResourceHMM,	///< (i) pointer to HMM resource manager
										 PowerASR_State_OProb *pStateOProb,		///< (i) pointer to state LL calculator
										 const char *pszConfigFile				///< (i) Viterbi scorer configuration file
);

/**
 *	free memories allocated to the Viterbi scorer.
 *
 *	@return Return 0 if all memories are freed successfully, otherwise return -1.
 */
HCILAB_PUBLIC HCI_VITERBI_API hci_int32
PowerASR_ViterbiScorer_closeViterbiScorer(PowerASR_ViterbiScorer *pThis			///< (i/o) pointer to the Viterbi scorer
);

/**
 *	initialize data buffers for Viterbi scorer.
 *
 *	@return Return 0 if user-specific data buffers are initialized successfully, otherwise return -1.
 *
 *	@see VITERBI_initializeViterbiSearch
 */
HCILAB_PUBLIC HCI_VITERBI_API hci_int32
PowerASR_ViterbiScorer_initializeViterbiScorer(PowerASR_ViterbiScorer *pThis,	///< (i) pointer to the Viterbi scorer
											   ViterbiSpace *pViterbiSpace,		///< (o) channel-specific Viterbi search space
											   const hci_flag bUseScoreMatrix,	///< (i) flag to use score matrix
											   const hci_uint32 nLenFrame		///< (i) feature frame length
);

/**
 *	compute matching score between a base-phone sequence and feature stream.
 *
 *	@return Return Viterbi alignment score.
 */
HCILAB_PUBLIC HCI_VITERBI_API hci_score_t
PowerASR_ViterbiScorer_computeViterbiMatchScore(PowerASR_ViterbiScorer *pThis,	///< (i) pointer to the Viterbi scorer											   ViterbiSpace *pViterbiSpace,		///< (o) channel-specific Viterbi search space
												const OPROB_TABLE *pLLTable,	///< (i) state log-likelihoods table
												const hci_uint8 *phoneSeq,		///< (i) base-phone index sequence
												const hci_uint32 nLenPhoneSeq,	///< (i) length of base-phone sequence
												hci_score_t *vecMaxFrameScore	///< (i/o) maximum accumulated score vector for each frame
);

/**
 *	compute matching score between a base-phone sequence and feature stream with sorted lexicons.
 *
 *	@return Return Viterbi alignment score.
 */
HCILAB_PUBLIC HCI_VITERBI_API hci_score_t
PowerASR_ViterbiScorer_computeViterbiScoreWithSortedLexicon(PowerASR_ViterbiScorer *pThis,	///< (i) pointer to the Viterbi scorer
															ViterbiSpace *pViterbiSpace,	///< (i/o) channel-specific Viterbi search space
															const OPROB_TABLE *pLLTable,	///< (i) state log-likelihoods table
															const hci_uint32 nLexicalOrder	///< (i) lexicon order/direction (0=normal, 1=reverse)
);

/**
 *	compute matching score between a base-phone sequence and feature stream.
 *
 *	@return Return Viterbi alignment score.
 *
 *	@see PowerASR_Resource_HMM_convertBasePhoneSeq2StateNetwork
 *	@see VITERBI_computeAlignmentScore
 */
HCILAB_PUBLIC HCI_VITERBI_API hci_score_t
PowerASR_ViterbiScorer_computeHMMSpecificMatchScore(PowerASR_ViterbiScorer *pThis,		///< (i) pointer to the Viterbi scorer
													OPROB_TABLE *pLLTable,				///< (o) state log-likelihoods table
													StateNetwork *stateNet,				///< (i/o) state network
													const hci_asr_feat_t *featStream,	///< (i) feature stream
													const hci_uint32 nModelID,			///< (i) HMM index
													hci_score_t *vecMaxFrameScore		///< (i/o) maximum accumulated score vector for each frame
);

/**
 *	insert a new lexicon into Viterbi search space.
 *		- create HMM state network for a new lexicon.
 *		- update a shared node index for fast Viterbi search.
 *
 *	@return Return 0.
 */
HCILAB_PUBLIC HCI_VITERBI_API hci_int32
PowerASR_ViterbiScorer_addLexiconToViterbiSpace(PowerASR_ViterbiScorer *pThis,	///< (i) pointer to the Viterbi scorer
												ViterbiSpace *pViterbiSpace,	///< (i/o) channel-specific Viterbi search space
												const hci_uint32 nLexicalOrder	///< (i) lexicon order/direction (0=normal, 1=reverse)
);

/**
 *	return the beam size value in Viterbi scoring process.
 *
 *	@return Return beam size.
 */
HCILAB_PUBLIC HCI_VITERBI_API hci_score_t
PowerASR_ViterbiScorer_getBeamSize(PowerASR_ViterbiScorer *pThis	///< (i) pointer to the Viterbi scorer
);

#ifdef __cplusplus
}
#endif

#endif // #ifndef __HCILAB_VITERBI_SCORER_H__

