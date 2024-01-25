
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
 *	@file	hci_symbol_match.h
 *	@ingroup symbolmatch_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	PowerASR symbol matching algorithm library
 */

#ifndef __HCILAB_SYMBOL_MATCH_H__
#define __HCILAB_SYMBOL_MATCH_H__

#include "base/hci_type.h"
#include "basic_op/basic_op.h"
#include "basic_op/fixpoint.h"
#include "hmm/hci_resource_hmm.h"
#include "hmm/hmm_common.h"
#include "lexicon/lexicon_common.h"
#include "symbol_match/smatch_common.h"

#if defined(HCI_MSC_32)
#ifdef HCI_SYMBOLMATCH_EXPORTS
#define HCI_SMATCH_API __declspec(dllexport)
#elif defined(HCI_SYMBOLMATCH_IMPORTS)
#define HCI_SMATCH_API __declspec(dllimport)
#else	// in case of static library
#define HCI_SMATCH_API
#endif // #ifdef HCI_SYMBOLMATCH_EXPORTS
#elif defined(HCI_OS2)
#define HCI_SMATCH_API
#else
#define HCI_SMATCH_API HCI_USER
#endif

/** PowerASR Symbol Matcher */
typedef struct {
	void *pInner;
} PowerASR_SymbolMatch;

#ifdef __cplusplus
extern "C" {
#endif

/**
 *	create a new Symbol matching engine.
 *
 *	@return Return the pointer to a newly created Symbol matching engine
 */
HCILAB_PUBLIC HCI_SMATCH_API PowerASR_SymbolMatch*
PowerASR_SymbolMatch_new();

/**
 *	delete the Symbol matching engine.
 */
HCILAB_PUBLIC HCI_SMATCH_API void
PowerASR_SymbolMatch_delete(PowerASR_SymbolMatch *pThis
);

/**
 *	set-up environments for Symbol matching engine,
 *	and allocate necessary memories.
 *
 *	@return Return 0 if Symbol matcher environments are set-up correctly, otherwise return -1.
 */
HCILAB_PUBLIC HCI_SMATCH_API hci_int32
PowerASR_SymbolMatch_openSymbolMatchEngine(PowerASR_SymbolMatch *pThis,		///< (i/o) pointer to the Symbol matching engine
										   PowerASR_Resource_HMM *pResHMM,	///< (i) pointer to acoustic model resource manager
										   const char *pszConfigFile		///< (i) Symbol matcher configuration file
);

/**
 *	free memories allocated to the Symbol matching engine.
 *
 *	@return Return 0 if all memories are freed successfully, otherwise return -1.
 */
HCILAB_PUBLIC HCI_SMATCH_API hci_int32
PowerASR_SymbolMatch_closeSymbolMatchEngine(PowerASR_SymbolMatch *pThis		///< (i/o) pointer to the Symbol matching engine
);

/**
 *	initialize data buffers for Symbol matching engine.
 *
 *	@return Return 0 if user-specific data buffers are initialized successfully, otherwise return -1.
 *
 *	@see SymbolMatch_initializeSymbolMatch
 */
HCILAB_PUBLIC HCI_SMATCH_API hci_int32
PowerASR_SymbolMatch_initializeSymbolMatchEngine(PowerASR_SymbolMatch *pThis,		///< (i) pointer to the Symbol matching engine
												 SymbolSpace *pSymbolSpace,			///< (o) channel-specific symbol search space
												 const hci_uint32 nLexicalType,		///< (i) lexicon data type (FULL_SYMBOL_LEX/PARTIAL_SYMBOL_LEX)
												 const hci_uint32 nLenRecSymSeq		///< (i) length of recognition symbol sequence
);

/**
 *	free memories for Symbol matching space.
 *
 *	@see SymbolMatch_releaseSymbolMatchSpace
 */
HCILAB_PUBLIC HCI_SMATCH_API void
PowerASR_SymbolMatch_releaseSymbolSearchSpace(PowerASR_SymbolMatch *pThis,		///< (i) pointer to the Symbol matching engine
											  SymbolSpace *pSymbolSpace			///< (o) channel-specific symbol search space
);

/**
 *	compute matching score between a recognition symbol sequence and a reference symbol sequence.
 *
 *	@return Return symbol matching score.
 *
 *	@see SymbolMatch_computeFullSymbolMatchingScoreForSortedLexicon
 *	@see SymbolMatch_computeFullSymbolMatchingScoreForUnsortedLexicon
 */
HCILAB_PUBLIC HCI_SMATCH_API hci_int32
PowerASR_SymbolMatch_computeFullMatchScoreOfSymbolSequences(PowerASR_SymbolMatch *pThis,		///< (i) pointer to the Symbol matching engine
															SymbolSpace *pSymbolSpace,			///< (i/o) channel-specific symbol search space
															const hci_uint32 bSortedLexicon		///< (i) flag to indicate whether lexicons are sorted
);

/**
 *	complete symbol matching process.
 *
 *	@return Return 0 if symbol matching was completed successfully.
 *
 *	@see SymbolMatch_completeSymbolMatch
 */
HCILAB_PUBLIC HCI_SMATCH_API hci_int32
PowerASR_SymbolMatch_completeSymbolMatching(PowerASR_SymbolMatch *pThis,		///< (i) pointer to the Symbol matching engine
											SymbolSpace *pSymbolSpace,			///< (i/o) channel-specific symbol search space
											const hci_flag bSortLexicon,		///< (i) flag to indicate whether lexicons are sorted
											const hci_int32 beamSize			///< (i) beam size
);

/**
 *	set a recognition symbol sequence.
 *
 *	@return Return 0 if a recognition symbol sequence was set successfully.
 */
HCILAB_PUBLIC HCI_SMATCH_API hci_int32
PowerASR_SymbolMatch_setRecognitionSymbolSequence(PowerASR_SymbolMatch *pThis,		///< (i) pointer to the Symbol matching engine
												  SymbolSpace *pSymbolSpace,		///< (o) channel-specific symbol search space
												  const hci_uint8 *recSymbolSeq,	///< (i) recognition symbol index sequence
												  const hci_uint32 nLenRecSymbolSeq	///< (i) length of a recognition symbol sequence
);

/**
 *	return the beam size value in symbol match process.
 *
 *	@return Return beam size.
 */
HCILAB_PUBLIC HCI_SMATCH_API hci_int16
PowerASR_SymbolMatch_getBeamSize(PowerASR_SymbolMatch *pThis		///< (i) pointer to the Symbol matching engine
);

/**
 *	set the number of candidates in symbol matching stage.
 *
 *	@return Return beam size.
 */
HCILAB_PUBLIC HCI_SMATCH_API void
PowerASR_SymbolMatch_setHypothesesStatckSize(PowerASR_SymbolMatch *pThis,		///< (i) pointer to the Symbol matching engine
											 SymbolSpace *pSymbolSpace,			///< (o) channel-specific symbol search space
											 const hci_uint32 nLenRecSymSeq		///< (i) length of recognition symbol sequence
);

#ifdef __cplusplus
}
#endif

#endif // #ifndef __HCILAB_SYMBOL_MATCH_H__

