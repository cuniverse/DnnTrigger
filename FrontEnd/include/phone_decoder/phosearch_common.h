
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
 *	@file	phosearch_common.h
 *	@ingroup phoneDec_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	constants/structure definition for Phonetic Decoder
 */

#ifndef __PHOSEARCH_COMMON_H__
#define __PHOSEARCH_COMMON_H__

#include "base/hci_type.h"
#include "basic_op/fixpoint.h"
#include "basic_op/basic_op.h"
#include "common/powerasr_const.h"
#include "hmm/hmm_common.h"
#include "hmm/hmm_state_net.h"

//////////////////////////////////////////////////////////////////////////

/* constants definitions */

#if defined(USE_TRIP_GRAMMAR)
#define FRAME_PHONE_BRANCH	100					///< maximum number of phone branches at each frame
#define PHONE_ACT_SIZE	(SUBWORD_SIZE+2)		///< maximum number of active model tokens at each frame
#else	// !USE_TRIP_GRAMMAR
#define FRAME_PHONE_BRANCH	BASEPHONE_SIZE		///< maximum number of phone branches at each frame
#define PHONE_ACT_SIZE	(BASEPHONE_SIZE+2)		///< maximum number of active model tokens at each frame
#endif	// #if defined(USE_TRIP_GRAMMAR)

#define SIZE_PHONE_PATH		200					///< maximum size of partial phone hypotheses

#define SIZE_PHONE_LATTICE	(MAX_DEC_FRAME*FRAME_PHONE_BRANCH)	///< total number of phone branches

#define SIZE_PHONE_CANDIDATE	5				///< maximum number of recognized phone sequences

//////////////////////////////////////////////////////////////////////////

/* structures */

#ifdef USE_SCALED_LL
typedef	hci_uint16	hci_branch_t;
#else
typedef hci_uint32 hci_branch_t;
#endif

/** Structure holding active phone branch */
typedef struct {
	hci_uint16 idPLU;						///< PLU index
	hci_int16 bpFrame;						///< back-pointer frame index
	hci_branch_t bpBranch;					///< back-pointer phone branch index
	hci_score_t accumScore;					///< accumulated score
}PhoneBranch;

/** Structure holding phone lattice */
typedef struct {
	hci_uint32 numPhoneBranch;						///< number of phone branches
	hci_uint32 framePhoneBranch[MAX_DEC_FRAME];		///< phone branch index vector for each frame
	PhoneBranch phoneBranch[SIZE_PHONE_LATTICE];	///< phone branch data
}PhoneLattice;

/** Structure holding active phone node */
typedef struct {
	hci_uint16 idPLU;							///< PLU index
	hci_uint16 nActive;							///< active mode
	hci_score_t accScore[UNIT_STATE+1];			///< accumulated score in each state
	hci_branch_t bpBranch[UNIT_STATE+1];		///< back-pointer branch in each state
} ActPhone;

/** Structure holding n-best phone recognition candidates */
typedef struct {
	hci_score_t	matchScore;						///< matching score
	hci_uint16 nLenPhoneSeq;					///< length of base-phone sequence
	hci_uint8 BasePhoneSeq[MAX_LEN_PHONE_SEQ];	///< base-phone sequence
	hci_uint8 phoneDur[MAX_LEN_PHONE_SEQ];		///< phone duration
	hci_score_t phoneScore[MAX_LEN_PHONE_SEQ];	///< phone score
} PhoneCandidate;

/** Structure holding n-best phone recognition candidates */
typedef struct {
	hci_int32 num_candidates;					///< number of phone recognition candidates
	PhoneCandidate candidate[SIZE_PHONE_CANDIDATE];	///< list of phone recognition candidates
} PHONE_RESULT;

/** Structure holding partial phone hypotheses */
typedef struct {
	hci_uint32 nLenPhoneSeq;					///< length of phone sequence
	hci_score_t	matchScore;						///< estimated matching score
	hci_score_t partialScore;					///< partial matching score to current extended phone
	PhoneBranch *PhoneSeq[MAX_LEN_PHONE_SEQ];	///< phone sequence
} PHONE_PATH;

/** Structure holding active phone search space */
typedef struct {
	hci_score_t maxAccumScore;						///< maximum accumulated score
	hci_score_t threshScore;						///< score threshold
	hci_uint16 countOutputUnchanged;				///< count of output update failure
	hci_uint16 countFullPath;						///< count of output update failure
	hci_uint16 numActPhones;						///< number of active phone nodes
	hci_int32 numHypotheses;						///< number of partial phone hypotheses
	hci_uint16 tokenIndex[PHONE_ACT_SIZE];			///< token index array
	hci_score_t vecMaxFrameScore[MAX_DEC_FRAME];	///< max frame score vector
	ActPhone act_phone[PHONE_ACT_SIZE];				///< active phone nodes
	PhoneLattice phoneLattice;						///< phone lattice
	PHONE_PATH phonePath[SIZE_PHONE_PATH];			///< partial phone hypotheses
} PhoneSpace;

#endif	// #ifndef __PHOSEARCH_COMMON_H__
