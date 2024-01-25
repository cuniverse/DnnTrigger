
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
 *	@file	viterbi_common.h
 *	@ingroup viterbi_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	constants/structure definition for channel-specific Viterbi Scorer
 */

#ifndef __VITERBI_COMMON_H__
#define __VITERBI_COMMON_H__

#include "base/hci_type.h"
#include "basic_op/fixpoint.h"
#include "basic_op/basic_op.h"
#include "common/powerasr_const.h"
#include "hmm/hmm_common.h"
#include "hmm/hmm_state_net.h"

//////////////////////////////////////////////////////////////////////////

/* constants definitions */

//////////////////////////////////////////////////////////////////////////

/* structures */

/** Structure holding active Viterbi search space */
typedef struct {
	hci_uint32 nNumProcLex;							///< number of processed lexicons
	hci_uint32 nLenCurrentPhoneSeq;					///< length of current phone sequence
	hci_uint32 nLenPrevPhoneSeq;					///< length of previous phone sequence
	hci_int16 minLiveFrame;							///< the first frame index to find active search space
	hci_int16 maxLiveFrame;							///< the last frame index to find active search space
	hci_int32 pruningPoint;							///< pruning length threshold to decide early pruning
	hci_int16 prevSharedNode;						///< previous shared node index (search starting point)
	hci_int16 currentSharedNode;					///< current shared node index (score keeping point)
	hci_uint8 currentPhoneSeq[MAX_LEN_PHONE_SEQ];	///< current phone sequence
	hci_uint8 prevPhoneSeq[MAX_LEN_PHONE_SEQ];		///< previous phone sequence
	hci_score_t vecMaxFrameScore[MAX_DEC_FRAME];	///< maximum frame score vector to prune unlikely nodes
	hci_score_t *scoreMat;							///< score matrix
	StateNetwork prevStateNet;						///< state network for processing lexicon
	StateNetwork currentStateNet;					///< state network for next lexicon
	hci_uint32 idLexicon;							///< current vocabulary lexicon index
	hci_uint32 nStartLexicon;						///< the first lexicon index
	hci_uint32 nEndLexicon;							///< the last lexicon index
	hci_uint32 nCurrentLexicon;						///< current processing lexicon index
	hci_uint8 *pBasePhoneLex;						///< pointer to processing base-phone lexicon data
} ViterbiSpace;

#endif	// #ifndef __VITERBI_COMMON_H__
