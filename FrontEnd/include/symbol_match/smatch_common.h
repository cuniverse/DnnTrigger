
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
 *	@file	smatch_common.h
 *	@ingroup symbolmatch_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	constants/structure definition for channel-specific symbol matcher
 */

#ifndef __SMATCH_COMMON_H__
#define __SMATCH_COMMON_H__

#include "base/hci_type.h"
#include "basic_op/fixpoint.h"
#include "basic_op/basic_op.h"
#include "hmm/hmm_common.h"
#include "hmm/hmm_state_net.h"
#include "lexicon/lexicon_common.h"

//////////////////////////////////////////////////////////////////////////

/* constants definitions */

#define SYMBOL_STACK_SIZE	3000	///< maximum number of candidates in symbol match process (1,000~3,000 for 20K vocabularies)

#define SYM_DP_RANGE_3			///< if MAX_SYM_INSERT == 3, define SYM_DP_RANGE_3

#ifdef SYM_DP_RANGE_3
#define MAX_SYM_INSERT	3			///< maximum count of successive symbol insertion/deletions
#else	// SYM_DP_RANGE_2
#define MAX_SYM_INSERT	2			///< maximum count of successive symbol insertion/deletions
#endif	// #ifdef SYM_DP_RANGE_3

#define MAX_LEN_SYMBOL_SEQ	200		///< maximum length of symbol sequence

#define FLOOR_SYM_SCORE	(-10000)	///< lower value of symbol matching score

//////////////////////////////////////////////////////////////////////////

/* structures */

/** Structure holding reference symbol lexicon data */
typedef struct {
	hci_uint32 idLexicon;								///< vocabulary lexicon index
	hci_score_t scoreLM;								///< LM score
//	FullSymbolLex currentFullLex[MAX_LEN_SYMBOL_SEQ];	///< current processing full lexicon
	FullSymbolLex *currentFullLex;						///< current processing full lexicon
	FullSymbolLex prevFullLex[MAX_LEN_SYMBOL_SEQ];		///< previous processing full lexicon
	hci_int16 currentSharedSymbol;						///< current shared symbol index (search starting point)
	hci_uint8 nMinLenSymbolSeq;							///< minimum length of symbol sequence for likely hypotheses
	hci_uint8 nMaxLenSymbolSeq;							///< maximum length of symbol sequence for likely hypotheses
} RefData;

/** Structure holding recognition symbol lexicon data */
typedef struct {
	hci_uint32 nLenRecSymbolSeq;						///< length of recognition symbol sequence
	hci_uint8 recSymbolSeq[MAX_LEN_SYMBOL_SEQ];			///< recognition symbol sequence
	hci_int8 *pSubProb[MAX_LEN_SYMBOL_SEQ];				///< pointer to substitution probability vector
	hci_int8 nInsProb[MAX_LEN_SYMBOL_SEQ];				///< insertion probability vector
	hci_int16 fillerScore;								///< filler score for a recognized symbol sequence
} RecData;

/** Structure holding current symbol search data */
typedef struct {
	hci_uint32 nNumProcLex;								///< number of processed lexicons
	hci_uint32 nStartLexicon;							///< the first lexicon index
	hci_uint32 nEndLexicon;								///< the last lexicon index
	hci_uint32 nCurrentLexicon;							///< current lexicon index
	FullSymbolLex *pFullSymLex;							///< pointer to processing full symbol lexicon data
} SymSearchData;

/** Structure holding symbol match score data */
typedef struct {
	hci_int16 nDimAxisX;								///< maximum symbol length in X axis
	hci_int16 nDimAxisY;								///< maximum symbol length in Y axis
	hci_int16 maxLiveSymbol;							///< the last recognition symbol position index to find active search space
	hci_int16 pruningPoint;								///< pruning length threshold to decide early pruning
	hci_int16 scoreThresh;								///< score threshold to insert into candidate list
	hci_int16 *sharedScoreMat;							///< shared matching score matrix
	hci_int16 *sharedInsertScore;						///< shared insertion path score matrix
	hci_int16 *pathScore;								///< shared path score matrix
	hci_int16 *RecentInsertScore;						///< score array in insertion path
	hci_int16 DeletePathScore[MAX_SYM_INSERT];			///< score array in deletion path
	hci_int16 vecMaxMatchScore[MAX_LEN_SYMBOL_SEQ];		///< maximum matching score vector to prune unlikely symbols
} SymScoreData;

/** Structure holding likely partial/full hits */
typedef struct {
	hci_uint32 idLexicon;							///< vocabulary lexicon index
//	FullSymbolLex symbolSeq[MAX_LEN_SYMBOL_SEQ];	///< symbol sequence in matched segment
	FullSymbolLex *symbolSeq;						///< symbol sequence in matched segment
	hci_score_t matchScore;							///< symbol matching score
	hci_score_t scoreAM;							///< LM score
	hci_score_t scoreLM;							///< LM score
	hci_int8 idTask;								///< ASR task index
	hci_int8 idBuf;									///< resource buffer index
} SymbolHit;

/** Structure holding symbol match results */
typedef struct {
	hci_int32 nNumHits;							///< number of symbol match hits
	SymbolHit SymHit[SYMBOL_STACK_SIZE+1];		///< list of symbol match hits
	SymbolHit *pNBestHit[SYMBOL_STACK_SIZE+1];	///< n-best candidates
} SM_RESULT;

/** configuration struct for symbol matcher */
typedef struct 
{
	hci_int16 beamSize;				///< beam size in Symbol match
	hci_int16 maxCountInsert;		///< maximum count of consecutive symbol insertions/deletions
	hci_int16 nLeftSearchWidth;		///< left-side search width in symbol match
	hci_int16 nRightSearchWidth;	///< right-side search width in symbol match
	hci_int32 sizeCandidates;		///< maximum number of symbol-match candidates
} SM_PARA;

/** Structure holding active Symbol Match space */
typedef struct {
	hci_flag bFullMatch;						///< flag to full symbol match
	RefData dataRefLex;							///< reference symbol lexicon data
	RecData dataRecLex;							///< recognition symbol lexicon data
	SymSearchData dataSearch;					///< symbol search data
	SymScoreData dataScore;						///< symbol match score data
	SM_RESULT smResult;							///< symbol match result
	SM_PARA smPara;								///< configurations for symbol matching
} SymbolSpace;

#endif	// #ifndef __SMATCH_COMMON_H__
