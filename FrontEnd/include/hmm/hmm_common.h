
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
 *	@file	hmm_common.h
 *	@ingroup hmm_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	constants/structure definition for Acoustic Model resource
 */

#ifndef __HMM_COMMON_H__
#define __HMM_COMMON_H__

#include "base/hci_type.h"
#include "basic_op/fixpoint.h"
#include "common/powerasr_const.h"
#include "wave2mfcc/fx_mfcc_common.h"

//////////////////////////////////////////////////////////////////////////

/* AM type definitions */

/** 
 *	acoustic model size --> select one of following types :
 *		- COMPACT_AM : tied states # < 1500,
 *		- NORMAL_AM : tied states # < 2000,
 *		- LARGE_AM : tied states # < 3000
 */
//#define COMPACT_AM
//#define	NORMAL_AM
#define LARGE_AM

/** 
 *	HMM type -> select one of following types :
 *		- BASE_HMM = base HMM type (single subspace)
 *		- SDCHMM = subspace distribution clustering HMM (N subspace)
 */
#define BASE_HMM
//#define SDCHMM						// subspace distribution clustering HMM

/**
 * HMM modeling type --> select one of following types :
 *		- CDHMM = continuous density HMM,
 *		- SCHMM = semi-continuous HMM (single tied codebook),
 *		- PTM = phonetically tied mixture HMM (different codebooks per base-phone or base-state 
 */
#define CDHMM						// continuous density HMM
//#define SCHMM						// semi-continuous HMM
//#define PTM							// phonetically tied mixture HMM

/**
 * language type --> select one of following languages :
 *		- LAN_CHINESE = Chinese language (larger base-phone size),
 *		- LAN_KOREAN = Korean language (base-phone # = 45),
 *		- LAN_ENGLISH = English (base-phone # < 64)
 */
//#define LAN_CHINESE
#define LAN_KOREAN
//#define LAN_ENGLISH

/** if HMM states are tied by decision tree, define TIED_STATE */
#define TIED_STATE

/** if triphone grammar is used, define USE_TRIP_GRAMMAR */
#define USE_TRIP_GRAMMAR

/** if phone confusion matrix is used, define USE_PHONOMAP */
#define USE_PHONOMAP

/** if decision-tree mapping table is used, define USE_DT_TABLE */
//#define USE_DT_TABLE

/** if logical-hmm-to-physical-hmm mapping table is used, define USE_LHMM2PHMM_TABLE */
#define USE_LHMM2PHMM_TABLE

//////////////////////////////////////////////////////////////////////////

/* constants definitions */

#ifdef LAN_CHINESE
#define	BASEPHONE_SIZE	256						///< maximum number of basephone units (consider Chinese phone set)
#else	// !LAN_CHINESE
//#define	BASEPHONE_SIZE	64						///< maximum number of basephone units (excluding Chinese language)
#define	BASEPHONE_SIZE	48						///< maximum number of basephone units (excluding Chinese language)
#endif	// #ifdef LAN_CHINESE

#define UNIT_STATE		3								///< maximum number of states per subword hmm
#define BASESTATE_SIZE	(UNIT_STATE*BASEPHONE_SIZE)		///< maximum number of base-states

#if defined(TIED_STATE)

#if defined(COMPACT_AM)
#define SUBWORD_SIZE	5000					///< maximum number of subword units
#define STATE_SIZE		(1500+BASESTATE_SIZE)	///< maximum number of distinct HMM states
#elif defined(NORMAL_AM)
#define SUBWORD_SIZE	6000					///< maximum number of subword units
#define STATE_SIZE		(2000+BASESTATE_SIZE)	///< maximum number of distinct HMM states
#else	// LARGE_AM
#define SUBWORD_SIZE	7000					///< maximum number of subword units
#define STATE_SIZE		(2500+BASESTATE_SIZE)	///< maximum number of distinct HMM states
#endif	// #if defined(COMPACT_AM)

#else	// !TIED_STATE

#if defined(COMPACT_AM)
#define SUBWORD_SIZE	1000					///< maximum number of subword units
#define STATE_SIZE		2500					///< maximum number of distinct HMM states
#elif defined(NORMAL_AM)
#define SUBWORD_SIZE	2000					///< maximum number of subword units
#define STATE_SIZE		5000					///< maximum number of distinct HMM states
#else	// LARGE_AM
#define SUBWORD_SIZE	3000					///< maximum number of subword units
#define STATE_SIZE		7500					///< maximum number of distinct HMM states
#endif	// #if defined(COMPACT_AM)

#endif	// #if defined(TIED_STATE)

#if defined(CDHMM)
#define GMM_CB_SIZE		8					///< maximum number of gauss pdfs per GMM
//#define GMM_CB_SIZE		16					///< maximum number of gauss pdfs per GMM
#define TOTAL_GMM_SIZE	STATE_SIZE			///< maximum number of gaussian mixtures
#elif defined(SCHMM)
#define GMM_CB_SIZE		256					///< maximum number of gauss pdfs per GMM
#define	TOTAL_GMM_SIZE	1					///< maximum number of gaussian mixtures
#else	// PTM
#define GMM_CB_SIZE		64					///< maximum number of gauss pdfs per GMM
#define	TOTAL_GMM_SIZE	BASEPHONE_SIZE		///< maximum number of gaussian mixtures
#endif	// #if defined(CDHMM)

#define PDF_SIZE		(GMM_CB_SIZE*TOTAL_GMM_SIZE)	///< maximum number of gaussian pdfs
#define GAUSS_WGT_SIZE	(STATE_SIZE*GMM_CB_SIZE)		///< maximum size of gaussian weight array

#if defined(USE_TRIP_GRAMMAR)
#define PLU_SIZE		SUBWORD_SIZE		///< maximum number of grammar units
#if defined(COMPACT_AM)
#define PLU_PAIR_SIZE	130000				///< maximum triphone-pair count (SrcTS2000-M8)
#elif defined(NORMAL_AM)
#define PLU_PAIR_SIZE	170000				///< maximum triphone-pair count (SrcTS2000-M8)
#else	// LARGE_AM
#define PLU_PAIR_SIZE	210000				///< maximum triphone-pair count (TS2000-M8)
#endif	// #if defined(COMPACT_AM)
#else	// !USE_TRIP_GRAMMAR
#define PLU_SIZE		BASEPHONE_SIZE		///< maximum number of grammar units
#define PLU_PAIR_SIZE	(BASEPHONE_SIZE<<5)	///< maximum basephone-pair count 
#endif	// #if defined(USE_TRIP_GRAMMAR)

#define CXT_TABLE_SIZE	(BASEPHONE_SIZE*BASEPHONE_SIZE*BASEPHONE_SIZE)
#define CXT_PAIR_SIZE	(BASEPHONE_SIZE*BASEPHONE_SIZE)

// constants for phone confusion matrix
#define REC_SYMBOL_SIZE	BASEPHONE_SIZE			///< maximum number of recognition symbols 
#define REF_SYMBOL_SIZE	BASEPHONE_SIZE			///< maximum number of reference symbols 
//#define REC_SYMBOL_SIZE	400					///< maximum number of recognition symbols 
//#define REF_SYMBOL_SIZE	400					///< maximum number of reference symbols 

// constants for SDCHMM
#define SUBSPACE_STREAM	13					///< maximum number of subspaces
#define SUBSPACE_WIDTH	3					///< maximum width of feature stream per subspace
#define SUBSPACE_SIZE	256					///< maximum codebook size per subspace

#ifdef BASE_HMM
#define PDF_DIM			(PDF_SIZE*DIM_FEATURE)			///< maximum total dimension of mean & variance vectors
#else	// defined(SDCHMM)
#define PDF_DIM	(SUBSPACE_STREAM*SUBSPACE_WIDTH*SUBSPACE_SIZE)	///< maximum dimension of codeword vectors in SDCHMM
#endif	// #ifdef BASE_HMM

// constants for decision tree & question set (for state-tying, phone.seq-to-symbol.seq conversion)
#define DT_SIZE_STATETYING	(2*STATE_SIZE)	///< maximum number of nodes in tied-state decision tree 
#define DT_SIZE_PHO2REFSYM	(2*REF_SYMBOL_SIZE)	///< maximum number of nodes in phone-to-ref-symbol decision tree 
#define DT_SIZE_PHO2RECSYM	(2*REC_SYMBOL_SIZE)	///< maximum number of nodes in phone-to-rec-symbol decision tree 
#if defined(LARGE_AM)
#define QUESTION_SIZE	800		///< maximum question set size
#define QUEST_CXT_SIZE	6400	///< maximum context size of question set
#else	// !LARGE_AM
#define QUESTION_SIZE	640		///< maximum question set size
#define QUEST_CXT_SIZE	5120	///< maximum context size of question set
#endif	// #if defined(LARGE_AM)

// Gauss distance table
#define QLEVEL_MEAN_VAR	(64 * 32)
#define SIZE_DIST_TABLE	(QUANT_LEVEL_MFCC*QLEVEL_MEAN_VAR)

//////////////////////////////////////////////////////////////////////////

/* structures */

typedef	hci_int16	hci_hmm16;
typedef	hci_int32	hci_hmm32;
typedef hci_int64	hci_hmm64;

/** Gauss mean/variance quantization level index */
/*typedef struct {
	hci_uint8 mean : 4;		///< quantization level of mean value
	hci_uint8 var : 4;		///< quantization level of variance value
} quant_gauss;

typedef union {
	quant_gauss val;		///< quantization level of (mean+variance) value
	hci_uint8 idQlevel;		///< quantization level of (mean+variance) value
} gauss_data_t;*/

//typedef hci_uint8 gauss_data_t;

typedef hci_uint16 gauss_data_t;

/** Structure holding Gaussian mean/variance vectors. */
typedef struct {
	gauss_data_t pdf_vec[PDF_DIM];		///< gaussian mean/variance vector array
} GAUSS_VECTOR;

/** Structure holding Gaussian weight vectors. */
typedef struct {
	hci_int16 logWgt[GAUSS_WGT_SIZE];	///< log value of gaussian weights
} GAUSS_WEIGHT;

/** Structure holding Gaussian PDF set. */
typedef struct {
	hci_int16 logGConst;				///< logarithmic value of Gaussian constant term
#ifdef BASE_HMM
	hci_uint16 ptrVec;					///< first vector index of current PDF in total mean/variance vector array
#else	// SDCHMM
	hci_uint8 idCW[SUBSPACE_STREAM+1];	///< subspace codeword index array
#endif	// #ifdef BASE_HMM
} GAUSS_PDF;

/** Structure holding Gaussian mixture set. */
typedef struct {
	hci_uint16 numPDF;					///< gaussian mixture size
	hci_uint16 ptrPDF;					///< first pdf index of current GMM in gaussian pdf array
} GMM;

/** Structure holding Subspace set. */
typedef struct {
	hci_uint8 numSubspace;						///< number of subspaces
	hci_uint8 widthSubspace[SUBSPACE_STREAM];	///< subspace stream width array
	hci_uint8 sizeSubspace[SUBSPACE_STREAM];	///< subspace codebook size array
	hci_uint8 orderFeature[DIM_FEATURE];		///< feature dimension index array
} SubSpace;

/** Structure holding distinct HMM state set. */
typedef struct {
	hci_uint16 ptrBaseState;		///< base-state index of current hmm state
	hci_uint16 ptrGMM;				///< GMM index of current hmm state
#ifndef CDHMM
	hci_uint32 ptrWgt;				///< first dimension index of current gaussian weights in total gaussian weight array
#endif	// #ifndef CDHMM
} HMM_STATE;

/** Structure holding base-state set. */
typedef struct {
	hci_uint16 idRootNode;			///< root node index in decision tree
	hci_uint16 ptrEmitState;		///< mapped emitting state index
#ifdef USE_DT_TABLE
	hci_uint8 cxt2state[CXT_PAIR_SIZE];	///< tied-state index mapping table for each acoustic context
#endif	// #ifdef USE_DT_TABLE
} HMM_BaseState;

/** Structure holding PLU property. */
typedef struct {	// caution : 2-byte fixed !!
	hci_uint8 numStates : 4;		///< number of states
	hci_uint8 flagCxtFree : 2;		///< flag to context-free PLU
	hci_uint8 flagCxtIndep : 2;		///< flag to context-independent PLU
	union {
		hci_uint8 basePLU : 8;		///< CDPLU-to-BasePLU mapping index
		hci_uint8 cho2jong : 8;		///< chosung-to-jongsung mapping index for use in symbol matching (language-dependent)
	} pack_t map;
} pack_t PLU_Property;

/** Structure holding subword PLU set. */
typedef struct {
#if (!defined(TIED_STATE) && !defined(USE_TRIP_GRAMMAR))
	char szName[12];					///< subword PLU name
#endif // #if (!defined(TIED_STATE) && !defined(USE_TRIP_GRAMMAR))
	PLU_Property attr;					///< PLU property
	hci_uint16 stateSeq[UNIT_STATE];	///< state index sequence
} PLU_Subword;

/** Structure holding base-phone set. */
typedef struct {
	char szName[4];						///< basephone name
	char szCent[4];						///< center phone name
#if !defined(TIED_STATE)
	char szLeftCxt[4];					///< left context name
	char szRightCxt[4];					///< right context name
#endif	// #if !defined(TIED_STATE)
	PLU_Property attr;					///< PLU property
	hci_uint16 stateSeq[UNIT_STATE];	///< base-state index sequence
#if defined(USE_PHONOMAP)
	hci_uint16 idRecRootNode;			///< root node index in phone-to-recog.symbol DT
	hci_uint16 idRefRootNode;			///< root node index in phone-to-refer.symbol DT
#endif	// #if defined(USE_PHONOMAP)
#if defined(USE_LHMM2PHMM_TABLE)
	hci_uint16 lhmm2phmm[CXT_PAIR_SIZE];	// (logical-HMM)-to-(physical-HMM) mapping table for each base-phone
#endif // #if defined(USE_LHMM2PHMM_TABLE)
} PLU_BasePhone;

/** Structure holding phone context data */
typedef struct {
	hci_uint8 idState[UNIT_STATE+1];	///< state index array for a particular context
} CONTEXT2STATE;

/** Structure holding phone confusion matrix. */
typedef struct {
	hci_uint8 symbol2phone[REF_SYMBOL_SIZE];				///< symbol-to-phone mapping table
	hci_int8 del_prob[REF_SYMBOL_SIZE];						///< deletion probability vector
	hci_int8 ins_prob[REC_SYMBOL_SIZE];						///< insertion probability vector
	hci_int8 sub_prob[REC_SYMBOL_SIZE*REF_SYMBOL_SIZE];		///< substitution probability matrix
	hci_int8 fillerScore[REC_SYMBOL_SIZE];					///< filler score vector
} PhoneConfusMat;

/** Structure holding PLU-pair grammar. */
typedef struct {
	hci_uint32 ptrPair[PLU_SIZE];		///< array index pointer of connected pairs
	hci_uint16 next_plu[PLU_PAIR_SIZE];	///< next-connected plu list
} PLU_Grammar;

/** Structure holding node data in decision tree. */
typedef struct {
	hci_uint16 descentNode;				///< left-side descent node index (right id = left id + 1), [0 --> terminal node]
	union {
		hci_uint16 idQuest;				///< question index iff non-terminal node
		hci_uint16 value;				///< target value iff terminal node
	} attr;
} DT_NODE;

/** Structure holding phonetic question set to use in decision tree. */
typedef struct {
	hci_uint16 numQuest;					///< number of questions
	hci_uint16 sizeQuestCxt;				///< size of total question context array
	hci_uint16 ptrLeftCxt[QUESTION_SIZE];	///< first dimension ptr of left context array for each question
	hci_uint16 ptrRightCxt[QUESTION_SIZE];	///< first dimension ptr of right context array for each question
	hci_uint8 cxtPhone[QUEST_CXT_SIZE];		///< total question context array
} QUEST_SET;

/** Structure holding HMM information. */
typedef struct {
	hci_uint32 numHMMs;						///< number of HMM resources
	hci_uint16 numBasePhones;				///< number of base-phones
	hci_uint16 numSubwords;					///< number of subword PLUs
	hci_uint16 numBaseStates;				///< number of base-states
	hci_uint16 numHmmStates;				///< number of HMM states
	hci_uint16 numGMMs;						///< number of Gaussian mixtures
	hci_uint16 numGaussPDFs;				///< number of Gaussian PDFs
	hci_uint32 sizeGaussVec;				///< size of Gaussian mean/variance vector array
	hci_uint32 sizeGaussWgt;				///< size of Gaussian weight array
	hci_uint32 sizePhonePair;				///< size of phone-pairs
	hci_uint16 featStreamWidth;				///< feature stream width
	hci_uint16 numNodes_StateTying;			///< number of nodes in tied-state decision tree
	hci_uint16 numRecSymbols;				///< number of recognition symbols
	hci_uint16 numRefSymbols;				///< number of reference symbols
	hci_uint16 numNodes_Pho2RecSym;			///< number of nodes in phone-to-rec-symbol decision tree
	hci_uint16 numNodes_Pho2RefSym;			///< number of nodes in phone-to-ref-symbol decision tree
	hci_uint16 idBaseSilence;				///< silence base-phone index
	hci_uint16 idBaseShortPause;			///< short pause base-phone index
	hci_uint16 idSubwdSilence;				///< silence subword model index
	hci_uint16 idSubwdShortPause;			///< short pause subword model index
	hci_uint32 sizeDistTable;				///< size of distance table
	hci_uint32 nBitsGMMPara;				///< count of bits for GMM parameter quantization
} HMM_INFO;

/** Structure holding specific HMM parameters */
typedef struct {

	GAUSS_PDF gaussPDF[PDF_SIZE];				///< Gauss PDF list
	GAUSS_VECTOR gaussVector;					///< Gauss mean/variance vector set
	
#if !defined(CDHMM)
	GAUSS_WEIGHT gaussWeight;					///< Gauss weight set
#endif	// #ifndef CDHMM
	
	hci_int16 distTable[SIZE_DIST_TABLE];		///< distance table for table look-up LL computation

} HMM_PARA;

/** Structure holding acoustic model resource. */
typedef struct {
	HMM_INFO hmm_info;							///< HMM resource summary information

	PLU_BasePhone basePLU[BASEPHONE_SIZE];		///< base-phone list

#if (!defined(TIED_STATE) || defined(USE_TRIP_GRAMMAR))
	PLU_Subword cxtdepPLU[SUBWORD_SIZE];		///< context-dependent PLU list
#endif	// #if (!defined(TIED_STATE) || defined(USE_TRIP_GRAMMAR))

	PLU_Grammar phoneGrammar;					///< triphone-pair grammar

	HMM_BaseState baseState[BASESTATE_SIZE];	///< base states
	HMM_STATE hmmState[STATE_SIZE];				///< HMM states

#if defined(SDCHMM)
	SubSpace subSpace;							///< subspace data in SDCHMM
#endif	// #if defined(SDCHMM)

	GMM gaussMixt[TOTAL_GMM_SIZE];				///< GMM list

#if (defined(TIED_STATE) && !defined(USE_DT_TABLE) && !defined(USE_LHMM2PHMM_TABLE))
	DT_NODE nodeDT_State[DT_SIZE_STATETYING];	///< node list in tied-state decision tree
#endif	// #if (defined(TIED_STATE) && !defined(USE_DT_TABLE) && !defined(USE_LHMM2PHMM_TABLE))

#if defined(USE_PHONOMAP)
	PhoneConfusMat phonoMap;					///< phone confusion matrix
	//DT_NODE nodeDT_Pho2RecSym[DT_SIZE_PHO2RECSYM];	///< node list in phone-to-rec-symbol decision tree
	//DT_NODE nodeDT_Pho2RefSym[DT_SIZE_PHO2REFSYM];	///< node list in phone-to-ref-symbol decision tree
#endif	// #if defined(USE_PHONOMAP)

#if ((defined(USE_PHONOMAP) || defined(TIED_STATE)) && !defined(USE_DT_TABLE) && !defined(USE_LHMM2PHMM_TABLE))
	QUEST_SET questSet;							///< question set
#endif	// #if ((defined(USE_PHONOMAP) || defined(TIED_STATE)) && !defined(USE_DT_TABLE) && !defined(USE_LHMM2PHMM_TABLE))

	HMM_PARA	*multiHMM;						///< multi-HMM parameters

} AM_Resource;

#endif	// #ifndef __HMM_COMMON_H__
