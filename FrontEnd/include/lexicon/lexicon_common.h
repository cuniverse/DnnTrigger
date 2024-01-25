
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
 *	@file	lexicon_common.h
 *	@ingroup lexicon_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	constants/structure definition for Vocabulary Lexicon resource
 */

#ifndef __LEXICON_COMMON_H__
#define __LEXICON_COMMON_H__

#include "base/hci_type.h"

//////////////////////////////////////////////////////////////////////////

/* constants definitions */

#define	MAX_TASK_COUNT	4			///< maximum number of recognition tasks

#define MAX_LEN_LEXICON	256			///< maximum length of lexical phone sequence

#ifndef DYNAMIC_VOCAB
#define DYNAMIC_VOCAB	0xFF
#endif

/** ASR task type */
#define IWR_VOCAB			0		///< isolated-word recognition task
#define CWR_VOCAB			1		///< connected-word recognition task with loop grammar
#define KWS_VOCAB			2		///< keyword spotting task
#define TDR_VOCAB			3		///< text document retrieval task by spoken query

//////////////////////////////////////////////////////////////////////////

/* structures */

/**
 * @enum user-specific lexicon data return type
 */
typedef enum {
		GET_LEX_FAIL,			///< failed to get lexical data
		GET_LEX_SUCCESS,		///< success to get lexical data, and remained data
		GET_LEX_COMPLETE		///< completed to get lexical data
} LexLoadType;

/**
 * @enum lexicon data type
 */
typedef enum {
		BASE_LEX,				///< base-phone lexicon
		FULL_SYMBOL_LEX,		///< symbol lexicon for full matching
		PARTIAL_SYMBOL_LEX		///< symbol lexicon for partial matching
} LexDataType;

/**
 * @enum lexical order
 */
typedef enum {
	FWD_LEXICON,				///< normal (forward direction)
	BWD_LEXICON					///< reverse (backward direction)
} LexOrderType;

#ifndef BaseLex_defined
typedef hci_uint8 BaseLex;
#define	BaseLex_defined
#endif

/** Structure holding lexicon resource summary information of a recognition set */
typedef struct {
	hci_uint32 num_vocabs;				///< number of recognition vocabularies
	hci_uint32 num_lexicons;			///< number of vocabulary lexicons
	hci_uint32 sizePronLex;				///< size of pronuncation lexicon data pool
	hci_uint32 sizeVocabStr;			///< size of vocabulary string pool
	hci_uint32 nMaxLenLexicon;			///< max. phone length of vocabulary lexicons
	hci_uint32 nLexicalType;			///< lexicon data type (BASE_LEX, FULL_SYMBOL_LEX, PARTIAL_SYMBOL_LEX)
	hci_uint32 nLexicalOrder;			///< lexical order (FWD_LEXICON, BWD_LEXICON)
	hci_uint32 bSortedLexicon;			///< flag to sorted lexicons
	hci_uint32 bSortedLength;			///< flag to sorted lexicons based on lexicon length
	float	   weightTask;				///< QC weight for current recognition task
	double	   totalQC;					///< total QC for all queries
	hci_uint32 nFirstLexIdxPerLength[MAX_LEN_LEXICON];	///< the first lexicon index per lexical length
	hci_uint32 nFirstLexDataPerLength[MAX_LEN_LEXICON];	///< the first lexicon data index per lexical length
	char szVocabFile[512];				///< vocabulary list file
	char szBuildTime[512];				///< resource building time
} VOCAB_INFO;

#ifndef FullSymbolLex_defined

/** Structure holding Lexical data for full symbol match */
typedef struct {	// caution : 1-byte fixed !!
	hci_uint8 idSymbol : 7;			///< symbol index
	hci_uint8 flagSilence : 1;		///< flag to preceding short-pause
} pack_t FullSymbolLex;

#define	FullSymbolLex_defined
#endif

/** Structure holding Lexical data for partial symbol match */
typedef struct {	// caution : 1-byte fixed !!
	hci_uint8 idSymbol : 6;			///< symbol index
	hci_uint8 bStart : 1;			///< flag to beginning phone of partial entities
	hci_uint8 bEnd : 1;				///< flag to ending phone of partial entities
} pack_t PartialSymbolLex;

/** Structure holding a recognition vocabulary */
typedef struct {
	hci_uint32 nPtrVocabStr;		///< pointer of vocabulary name string in a vocabulary string pool or file
	hci_uint16 nLenVocabStr;		///< length of vocabulary name string
	hci_int16  nUniGramProb;		///< log unigram prob.
} ASR_VOCAB;

/** Structure holding a vocabulary lexicon */
typedef struct {
	hci_uint32 idVocab;					///< vocabulary index
} ASR_LEXICON;

/** Structure holding common vocabulary lexicon resource. */
typedef struct {
	
	VOCAB_INFO vocab_info;							///< Lexicon resource summary information

	ASR_VOCAB *vocab;								///< vocabulary list

	ASR_LEXICON *lexicon;							///< lexicon list

	FullSymbolLex *fullSymbolPronLex;				///< full symbol pronunciation lexicon data

	PartialSymbolLex *partialSymbolPronLex;			///< partial symbol pronunciation lexicon data

	BaseLex *basePronLex;							///< base-phone pronunciation lexicon data

	hci_int8 *szVocabListStr;						///< vocabulary list string

	hci_uint32 nRecvDataSize;						///< received data size
	hci_uint32 nLexiDataSize;						///< estimated lexical data size

} Lexicon_Resource;

/** Structure holding user-specific lexicon resource. */
typedef struct {

	hci_uint32 idTask;								///< recognition task index

	Lexicon_Resource *pResLex;						///< pointer to lexicon resource

	FullSymbolLex *fullSymbolPronLex;				///< full symbol pronunciation lexicon data

	PartialSymbolLex *partialSymbolPronLex;			///< partial symbol pronunciation lexicon data

	BaseLex *basePronLex;							///< base-phone pronunciation lexicon data

} UserLexicon;

#endif	// #ifndef __LEXICON_COMMON_H__
