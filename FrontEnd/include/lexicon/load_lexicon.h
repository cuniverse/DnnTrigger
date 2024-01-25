
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
 *	@file	load_lexicon.h
 *	@ingroup lexicon_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	lexicon resource loading/creating library
 */

#ifndef __LOAD_LEXICON_H__
#define __LOAD_LEXICON_H__

#include "lexicon/lexicon_common.h"

/**
 * @enum lexicon resource file type
 */
typedef enum {
		ONLY_VOCAB_LIST,		///< only vocabulary list
		VOCAB_LEXICON,			///< vocabulary + lexicon
		COMPILED_LEXICON		///< compiled lexicon resource
} LexType;

/**
 * @enum lexicon resource loading type
 */
typedef enum {
		DEFAULT_LEX,			///< default lexicon resource
		ON_DEMAND_LEX			///< on-demand lexicon resource
} LoadType;

/** Lexicon resource configuration struct */
typedef struct 
{
	char *szExceptPronDictFile;						///< exceptional pronunciation dictionary file
	char *szUserDictFile;							///< user-defined dictionary file
	char *szUnitDictFile;							///< english unit dictionary file
	char szVocabListFile[2*MAX_TASK_COUNT][256];		///< vocabulary name set file
} LexiconParameters;

#ifdef __cplusplus
extern "C" {
#endif

/**
 *	build compiled lexicon resource from vocabulary list file
 */
hci_int32
RES_LEXICON_buildCompiledLexiconResourceFromVocabFile(Lexicon_Resource *pResLexicon,		///< (o) lexicon resource
													  const char *szVocabListFile,			///< (i) vocabulary list file
													  const char *szUpdateTime,				///< (i) resource update time
													  const float weightTask				///< (i) QC weight for current task
);

/**
 *	load exceptional pronunciation dictionary file
 */
hci_int32
RES_LEXICON_loadExceptionalPronDict( const char *szExceptPronFile 		///< (i) 예외사전 파일
);

/**
 *	load user-defined dictionary file
 */
hci_int32
RES_LEXICON_loadUserDictFile( const char *szUserDictFile 		///< (i) 사용자 정의 예외사전 파일
);

/**
 *	load english unit dictionary file
 */
hci_int32
RES_LEXICON_loadUnitDictFile( const char *szUnitDictFile 		///< (i) 영어 단위사전 파일
);

/**
 *	create hash table for exceptional pron-dict
 */
hci_int32
RES_LEXICON_createExceptDictHashTable(
);

/**
 *	free memories allocated to exceptional-pronunciation dictionary data
 */
hci_int32
RES_LEXICON_freeExceptionalPronDict(
);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef __LOAD_LEXICON_H__
