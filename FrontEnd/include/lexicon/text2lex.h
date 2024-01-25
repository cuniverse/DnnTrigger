
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
 *	@file	text2lex.h
 *	@ingroup lexicon_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	Text-to-lexicon conversion library for Korean language
 */

#ifndef __TEXT2LEX_H__
#define __TEXT2LEX_H__

#include "base/hci_type.h"
#include "base/hci_macro.h"
#include "lexicon/lexicon_common.h"

//////////////////////////////////////////////////////////////////////////

/* constants definitions */
#define MAX_NUM_LEXICON	32		///< maximum number of lexicons per vocabulary
//#define MAX_NUM_LEXICON	8		///< maximum number of lexicons per vocabulary

#define OMITH // 0324			///< if OMITH is defined, 'ㅎ' 탈락 규칙이 적용됨.
//#define READ_PER_EUMJOL		///< if READ_PER_EUMJOL is defined, 음절 끊어읽기 규칙이 적용됨.

//////////////////////////////////////////////////////////////////////////

/* structure definitions */

/** pronunciation lexicon */
typedef struct PRON_LEXICON
{
	hci_uint16 nLenPhoneSeq;				///< length of phone sequence
//	char szPhoneSeq[MAX_LEN_LEXICON][4];	///< phone sequence string, i.e. lexical pronunciation string
	hci_uint8 PhoneSeq[MAX_LEN_LEXICON];	///< phone index sequence of lexicon
	hci_uint8 flagSP[MAX_LEN_LEXICON];		///< short-pause-flag sequence
} PRON_LEXICON;

/** output vocabulary lexicons */
typedef struct VOC_LEXICON
{
	hci_uint16 nNumLexicons;				///< number of pronunciation lexicons
	PRON_LEXICON pronDict[MAX_NUM_LEXICON];	///< pronunciation lexicons
} VOC_LEXICON;

/** source vocabulary lexicons */
typedef struct SRC_LEXICON
{
	hci_uint16 nNumMonoLex;								///< number of pronunciation lexicons
	hci_uint16 nLenMonoLex[MAX_NUM_LEXICON];			///< length of a lexical representations
	char szMonoLex[MAX_NUM_LEXICON][MAX_LEN_LEXICON];	///< lexical representation in phone symbol string
} SRC_LEXICON;

/** exceptional dictionary */
typedef struct EXCEPT_DICT
{
	char *szSrcVocab;
	char *szPronDict;
} EXCEPT_DICT;

//////////////////////////////////////////////////////////////////////////

/* API functions */

#ifdef __cplusplus
extern "C" {
#endif 

/**
 *	convert a given text into pronunciation lexicons
 * 
 *	@return Return 0 if pronunciation lexicons are generated successfully, otherwise return -1.
 */
HCILAB_PUBLIC hci_int32
TEXT2PHONE_convertTextIntoLexicons(const char *szQueryText,				///< (i) input query text
								   VOC_LEXICON *pVocabLexicon,			///< (o) pronunciation lexicons
								   const hci_uint32 nMaxLexiconCount	///< (i) maximum count of pronunciation lexicons
);

HCILAB_PUBLIC hci_int32
TEXT2PHONE_loadExceptionalDict(const char *szFile);

HCILAB_PUBLIC hci_int32
TEXT2PHONE_loadEnUnitDict(const char *szUnitDictFile);

HCILAB_PUBLIC hci_int32
TEXT2PHONE_createExceptDictHashTable();

HCILAB_PUBLIC hci_int32
TEXT2PHONE_freeExceptionalDict();

#ifdef __cplusplus
}
#endif

#endif	// #ifndef __TEXT2LEX_H__


