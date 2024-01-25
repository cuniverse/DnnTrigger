
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
 *	@file	text2lex.c
 *	@ingroup lexicon_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	Text-to-lexicon conversion library for Korean language
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "base/str2words.h"
#include "base/hci_malloc.h"
#include "base/hash.h"
#include "base/case.h"
#include "base/hci_macro.h"

#include "lexicon/text2lex.h"
#include "lexicon/kor_g2p_res.h"

#define KR_MORPH		0
#define ALPHA_MORPH		1
#define DIGIT_MORPH		2
#define SYMBOL_MORPH	3
#define UNK_MORPH		(-1)

typedef struct EXCEPT_PRON {
	int nCountDict;
	char szPron[16][128];
} EXCEPT_PRON;

typedef struct MORPH_SEQ {
	int nCountMorph;
	char szMorph[16][128];
	short morphType[16];
} MORPH_SEQ;

int g_CountExceptDict = 0;
EXCEPT_DICT *g_ExceptDict = 0;

int g_CountUnitDict = 0;
EXCEPT_DICT *g_UnitDict = 0;

static hash_t g_exceptDict_ht;			// Hash table for exceptional pronunciation dictionary
static hash_t g_unitDict_ht;			// Hash table for English unit dictionary

// local functions
#ifdef __cplusplus
extern "C" {
#endif 

/*
 * find exceptional pronunciation dictionary matched with a given vocabulary string
 */
HCILAB_PRIVATE hci_flag
_find_exceptional_dict(EXCEPT_PRON *exceptDict,		///< (o) exceptional dictionary data
					   const char *szVocabName );	///< (i) query vocabulary string

/*
 * process alphabetic & numerical query
 */
HCILAB_PRIVATE hci_flag
_process_alphanumeric_word(EXCEPT_PRON *exceptDict,		///< (o) exceptional dictionary data
						   const char *szVocabName );	///< (i) query vocabulary string

/*
 * convert an input word into pronunciation lexicons
 */
HCILAB_PRIVATE hci_flag
MakeMonoDic(SRC_LEXICON *pWordLex,				///< (o) pronunciation lexicons
			const char *word,					///< (i) vocabulary string
			const hci_uint32 nMaxLexiconCount,	///< (i) maximum number of pronunciation lexicons
			const hci_flag bSegmRead);			///< (i) flag to segmental pronunciation

HCILAB_PRIVATE int
GetCode(const char *s1,
		char *res);

void	write_pause(char *psym, char *psym_sp);

HCILAB_PRIVATE hci_uint32
Eng2Kor(const char c, char *des);

HCILAB_PRIVATE hci_uint32
Digit2Kor(const char c, char *des);

HCILAB_PRIVATE hci_uint32
Digit2Eng(const char c, char *des);

int		CheckException(char *wordcode, int *ex, int *ex_pos);
int		CheckException4(char *wordcode, int *ex, int *ex_pos);
int		reading_by_rule(char *word, int *ex, int *cpos, int num_ex, char *psym);
int		reading_per_eumjol(char *wordcode, char *psym);
int		IsOptRule(char *wordcode);
int		IsOmitH(char *wordcode);

#ifdef __cplusplus
}
#endif

/**
 *	convert a given text into pronunciation lexicons
 * 
 *	@return Return 0 if pronunciation lexicons are generated successfully, otherwise return -1.
 */
HCILAB_PUBLIC hci_int32
TEXT2PHONE_convertTextIntoLexicons(const char *szQueryText,				///< (i) input query text
								   VOC_LEXICON *pVocabLexicon,			///< (o) pronunciation lexicons
								   const hci_uint32 nMaxLexiconCount)	///< (i) maximum count of pronunciation lexicons
{
	hci_uint16 i = 0, j = 0, k = 0;
	hci_uint16 nLen = 0;
	hci_int32 n = 0;
	hci_int32 numWords = 0;
	char *pszWordSeq[32];
	char szVocabName[1024];
	char szSrcText[1024];
	char szCompound[1024];
	char *pStr = 0;
	char *pSpecChar = 0;
	PRON_LEXICON *pPronLex = 0;
	SRC_LEXICON wordLexicon;
	hci_uint8 flagSp = 0;
	EXCEPT_PRON exceptDict;

	memset(pVocabLexicon, 0, sizeof(VOC_LEXICON));

	if (0 == pVocabLexicon) {
		return -1;
	}
	if (0 == szQueryText) {
		return -1;
	}
	if (0 == nMaxLexiconCount) {
		return -1;
	}

	strcpy(szSrcText, szQueryText);
	pStr = szSrcText;
	while ( *pStr != '\0' ) {
		if ( (*pStr) & 0x80 ) pStr += 2;
		else if ( isalpha(*pStr) ) {
			*pStr = toupper(*pStr);
			pStr++;
		}
		else {
			pStr++;
		}
	}

	while ( (pSpecChar = strchr(szSrcText, ':') ) ) {
		*pSpecChar = ' ';
		pSpecChar = strchr( szSrcText, ':' );
	}

	numWords = PowerASR_Base_str2words(szSrcText, pszWordSeq, 32);
	if (numWords <= 0) {
		return -1;
	}

	for (n = 0; n < numWords; n++) {
		sscanf(pszWordSeq[n], "%s", szVocabName);

		memset(&wordLexicon, 0, sizeof(wordLexicon));

		memset(&exceptDict, 0, sizeof(exceptDict));
		_find_exceptional_dict( &exceptDict, szVocabName );
#if 0 //발음 예외 사전 사용안함
		if ( 0 == exceptDict.nCountDict ) {
			if ( _process_alphanumeric_word( &exceptDict, szVocabName ) == FALSE ) {
				return -1;
			}
		}
#endif
		if ( n ) {
			if ( exceptDict.nCountDict ) {
				strcat(szCompound, exceptDict.szPron[0]);
			}
			else {
				strcat(szCompound, szVocabName);
			}
		}
		else {
			if ( exceptDict.nCountDict ) {
				strcpy(szCompound, exceptDict.szPron[0]);
			}
			else {
				strcpy(szCompound, szVocabName);
			}
		}

		if ( 0 == exceptDict.nCountDict ) {
			if (MakeMonoDic(&wordLexicon, szVocabName, nMaxLexiconCount, 0) == FALSE) {
				return -1;
			}
		}
		else {
			SRC_LEXICON exceptLexicon;
			int iE = 0, iL = 0, nCountL = 0;
			for ( iE = 0; iE < exceptDict.nCountDict ; iE++) {
				memset(&exceptLexicon, 0, sizeof(exceptLexicon));
				if (MakeMonoDic(&exceptLexicon, exceptDict.szPron[iE], nMaxLexiconCount, 0) == TRUE) {
					nCountL = wordLexicon.nNumMonoLex;
					for ( iL = 0; iL < exceptLexicon.nNumMonoLex && wordLexicon.nNumMonoLex < nMaxLexiconCount; iL++) {
						wordLexicon.nLenMonoLex[nCountL] = exceptLexicon.nLenMonoLex[iL];
						strcpy(wordLexicon.szMonoLex[nCountL], exceptLexicon.szMonoLex[iL]);
						wordLexicon.nNumMonoLex++; nCountL++;
					}
				}
			}
		}

		if (wordLexicon.nNumMonoLex == 0) {
			continue;
		}

		if ( pVocabLexicon->nNumLexicons == 0 ) {
			pVocabLexicon->nNumLexicons = wordLexicon.nNumMonoLex;
			pPronLex = pVocabLexicon->pronDict;
			for (i = 0; i < pVocabLexicon->nNumLexicons; i++, pPronLex++) {
				nLen = 0;
				flagSp = 0;
				for (j = 0; j < wordLexicon.nLenMonoLex[i]; j++) {
					pStr = strchr(g_phoneset, wordLexicon.szMonoLex[i][j]);
					if ( pStr ) {
						k = (hci_uint16)(pStr - g_phoneset);
						if ( Phoneme_Table[k].symbolID >= 0 ) {
							pPronLex->PhoneSeq[nLen] = (hci_uint8)Phoneme_Table[k].symbolID;
							pPronLex->flagSP[nLen]   = flagSp;
							nLen++;
							flagSp = 0;
						}
						else {
							flagSp = 1;
						}
					}
				}
				pPronLex->nLenPhoneSeq = nLen;
			}
		}
		else {
			if (pVocabLexicon->nNumLexicons == wordLexicon.nNumMonoLex) {
				pPronLex = pVocabLexicon->pronDict;
				for (i = 0; i < pVocabLexicon->nNumLexicons; i++, pPronLex++) {
					nLen = pPronLex->nLenPhoneSeq;
					flagSp = 1;
					for (j = 0; j < wordLexicon.nLenMonoLex[i]; j++) {
						pStr = strchr(g_phoneset, wordLexicon.szMonoLex[i][j]);
						if ( pStr ) {
							k = (hci_uint16)(pStr - g_phoneset);
							if ( Phoneme_Table[k].symbolID >= 0 ) {
								pPronLex->PhoneSeq[nLen] = (hci_uint8)Phoneme_Table[k].symbolID;
								pPronLex->flagSP[nLen]   = flagSp;
								nLen++;
								flagSp = 0;
							}
							else {
								flagSp = 1;
							}
						}
					}
					pPronLex->nLenPhoneSeq = nLen;
				}
			}
			else if (pVocabLexicon->nNumLexicons > wordLexicon.nNumMonoLex) {
				hci_uint16 ii = 0;
				pPronLex = pVocabLexicon->pronDict;
				for (i = 0; i < pVocabLexicon->nNumLexicons; i++, pPronLex++) {
					nLen = pPronLex->nLenPhoneSeq;
					flagSp = 1;
					ii = i%wordLexicon.nNumMonoLex;
					for (j = 0; j < wordLexicon.nLenMonoLex[ii]; j++) {
						pStr = strchr(g_phoneset, wordLexicon.szMonoLex[ii][j]);
						if ( pStr ) {
							k = (hci_uint16)(pStr - g_phoneset);
							if ( Phoneme_Table[k].symbolID >= 0 ) {
								pPronLex->PhoneSeq[nLen] = (hci_uint8)Phoneme_Table[k].symbolID;
								pPronLex->flagSP[nLen]   = flagSp;
								nLen++;
								flagSp = 0;
							}
							else {
								flagSp = 1;
							}
						}
					}
					pPronLex->nLenPhoneSeq = nLen;
				}
			}
			else {
				PRON_LEXICON *pSrcLex = 0;
				pSrcLex = pVocabLexicon->pronDict;
				pPronLex = pVocabLexicon->pronDict + pVocabLexicon->nNumLexicons;
				for (i = pVocabLexicon->nNumLexicons; i < wordLexicon.nNumMonoLex; i++) {
					memcpy(pPronLex, pSrcLex, sizeof(PRON_LEXICON));
					pSrcLex++; pPronLex++;
				}
				pVocabLexicon->nNumLexicons = wordLexicon.nNumMonoLex;
				pPronLex = pVocabLexicon->pronDict;
				for (i = 0; i < pVocabLexicon->nNumLexicons; i++, pPronLex++) {
					nLen = pPronLex->nLenPhoneSeq;
					flagSp = 1;
					for (j = 0; j < wordLexicon.nLenMonoLex[i]; j++) {
						pStr = strchr(g_phoneset, wordLexicon.szMonoLex[i][j]);
						if ( pStr ) {
							k = (hci_uint16)(pStr - g_phoneset);
							if ( Phoneme_Table[k].symbolID >= 0 ) {
								pPronLex->PhoneSeq[nLen] = (hci_uint8)Phoneme_Table[k].symbolID;
								pPronLex->flagSP[nLen]   = flagSp;
								nLen++;
								flagSp = 0;
							}
							else {
								flagSp = 1;
							}
						}
					}
					pPronLex->nLenPhoneSeq = nLen;
				}
			}
		}

	}

	if ( numWords > 1 && pVocabLexicon->nNumLexicons && strlen(szCompound) <= 12 ) {

		strcpy(szVocabName, szCompound);
		memset(&wordLexicon, 0, sizeof(wordLexicon));

		memset(&exceptDict, 0, sizeof(exceptDict));
		_find_exceptional_dict( &exceptDict, szVocabName );

		if ( 0 == exceptDict.nCountDict ) {
			_process_alphanumeric_word( &exceptDict, szVocabName );
		}

		if ( exceptDict.nCountDict ) {
			SRC_LEXICON exceptLexicon;
			int iE = 0, iL = 0, nCountL = 0;
			for ( iE = 0; iE < exceptDict.nCountDict ; iE++) {
				memset(&exceptLexicon, 0, sizeof(exceptLexicon));
				if (MakeMonoDic(&exceptLexicon, exceptDict.szPron[iE], nMaxLexiconCount, 0) == TRUE) {
					nCountL = wordLexicon.nNumMonoLex;
					for ( iL = 0; iL < exceptLexicon.nNumMonoLex && wordLexicon.nNumMonoLex < nMaxLexiconCount; iL++) {
						wordLexicon.nLenMonoLex[nCountL] = exceptLexicon.nLenMonoLex[iL];
						strcpy(wordLexicon.szMonoLex[nCountL], exceptLexicon.szMonoLex[iL]);
						wordLexicon.nNumMonoLex++; nCountL++;
					}
				}
			}

			if (wordLexicon.nNumMonoLex) {

				pPronLex = pVocabLexicon->pronDict + pVocabLexicon->nNumLexicons;
				for (i = 0; i < wordLexicon.nNumMonoLex && pVocabLexicon->nNumLexicons < nMaxLexiconCount; i++, pPronLex++) {
					nLen = 0;
					flagSp = 0;
					for (j = 0; j < wordLexicon.nLenMonoLex[i]; j++) {
						pStr = strchr(g_phoneset, wordLexicon.szMonoLex[i][j]);
						if ( pStr ) {
							k = (hci_uint16)(pStr - g_phoneset);
							if ( Phoneme_Table[k].symbolID >= 0 ) {
								pPronLex->PhoneSeq[nLen] = (hci_uint8)Phoneme_Table[k].symbolID;
								pPronLex->flagSP[nLen]   = flagSp;
								nLen++;
								flagSp = 0;
							}
							else {
								flagSp = 1;
							}
						}
					}
					pPronLex->nLenPhoneSeq = nLen;
					pVocabLexicon->nNumLexicons++;
				}

			}

		}

	}
	
	if ( pVocabLexicon->nNumLexicons && strlen(szCompound) <= 8 ) {
		
		strcpy(szVocabName, szCompound);
		memset(&wordLexicon, 0, sizeof(wordLexicon));
		
		memset(&exceptDict, 0, sizeof(exceptDict));
		_find_exceptional_dict( &exceptDict, szVocabName );
		
		if ( 0 == exceptDict.nCountDict ) {
			_process_alphanumeric_word( &exceptDict, szVocabName );
		}
		
		if ( exceptDict.nCountDict ) {
			SRC_LEXICON exceptLexicon;
			int iE = 0, iL = 0, nCountL = 0;
			for ( iE = 0; iE < exceptDict.nCountDict ; iE++) {
				memset(&exceptLexicon, 0, sizeof(exceptLexicon));
				if (MakeMonoDic(&exceptLexicon, exceptDict.szPron[iE], nMaxLexiconCount, 1) == TRUE) {
					nCountL = wordLexicon.nNumMonoLex;
					for ( iL = 0; iL < exceptLexicon.nNumMonoLex && wordLexicon.nNumMonoLex < nMaxLexiconCount; iL++) {
						wordLexicon.nLenMonoLex[nCountL] = exceptLexicon.nLenMonoLex[iL];
						strcpy(wordLexicon.szMonoLex[nCountL], exceptLexicon.szMonoLex[iL]);
						wordLexicon.nNumMonoLex++; nCountL++;
					}
				}
			}
			
			if (wordLexicon.nNumMonoLex) {
				
				pPronLex = pVocabLexicon->pronDict + pVocabLexicon->nNumLexicons;
				for (i = 0; i < wordLexicon.nNumMonoLex && pVocabLexicon->nNumLexicons < nMaxLexiconCount; i++, pPronLex++) {
					nLen = 0;
					flagSp = 0;
					for (j = 0; j < wordLexicon.nLenMonoLex[i]; j++) {
						pStr = strchr(g_phoneset, wordLexicon.szMonoLex[i][j]);
						if ( pStr ) {
							k = (hci_uint16)(pStr - g_phoneset);
							if ( Phoneme_Table[k].symbolID >= 0 ) {
								pPronLex->PhoneSeq[nLen] = (hci_uint8)Phoneme_Table[k].symbolID;
								pPronLex->flagSP[nLen]   = flagSp;
								nLen++;
								flagSp = 0;
							}
							else {
								flagSp = 1;
							}
						}
					}
					pPronLex->nLenPhoneSeq = nLen;
					pVocabLexicon->nNumLexicons++;
				}
				
			}
			
		}
		
	}
	
	// adjust short-pause flag
	if ( pVocabLexicon->nNumLexicons ) {
		unsigned int prev_symbol = 0, curr_symbol = 0;
		for ( n = 0; n < pVocabLexicon->nNumLexicons; n++) {
			pPronLex = pVocabLexicon->pronDict + n;
			prev_symbol = Symbol_Table[pPronLex->PhoneSeq[0]].classID;
			for ( j = 1 ; j < pPronLex->nLenPhoneSeq; j++) {
				curr_symbol = Symbol_Table[pPronLex->PhoneSeq[j]].classID;
				if ( curr_symbol <= prev_symbol ) pPronLex->flagSP[j] = 1;
				prev_symbol = curr_symbol;
			}
		}
	}

	// remove equal lexicons
	if ( pVocabLexicon->nNumLexicons > 1 ) {
		hci_flag *bDisableLex = 0;
		PRON_LEXICON *pSrcLex = 0;
		int nCountOutLex = 0;
		bDisableLex = (hci_flag *) hci_calloc( pVocabLexicon->nNumLexicons, sizeof(hci_flag) );
		for ( n = 0; n < pVocabLexicon->nNumLexicons; n++) {
			if ( bDisableLex[n] ) continue;
			pSrcLex = pVocabLexicon->pronDict + n;
			for ( i = n + 1; i < pVocabLexicon->nNumLexicons; i++) {
				if ( bDisableLex[i] ) continue;
				pPronLex = pVocabLexicon->pronDict + i;
				if ( pSrcLex->nLenPhoneSeq == pPronLex->nLenPhoneSeq ) {
					for ( j = 0 ; j < pSrcLex->nLenPhoneSeq; j++) {
						if ( pSrcLex->PhoneSeq[j] != pPronLex->PhoneSeq[j] ) break;
					}
					if ( j == pSrcLex->nLenPhoneSeq ) {	// equal lexicon
// 						for ( j = 0 ; j < pSrcLex->nLenPhoneSeq; j++) {
// 							pSrcLex->flagSP[j] = HCI_MAX(pSrcLex->flagSP[j], pPronLex->flagSP[j]);
// 						}
						bDisableLex[i] = 1;
					}
				}
			}
		}
		for ( n = 0; n < pVocabLexicon->nNumLexicons; n++) {
			if ( bDisableLex[n] == 0 ) {
				if ( n != nCountOutLex ) {
					pPronLex = pVocabLexicon->pronDict + nCountOutLex;
					pSrcLex  = pVocabLexicon->pronDict + n;
					pPronLex->nLenPhoneSeq = pSrcLex->nLenPhoneSeq;
					memcpy(pPronLex->PhoneSeq, pSrcLex->PhoneSeq, pSrcLex->nLenPhoneSeq);
					memcpy(pPronLex->flagSP, pSrcLex->flagSP, pSrcLex->nLenPhoneSeq);
				}
				nCountOutLex++;
			}
		}
		pVocabLexicon->nNumLexicons = nCountOutLex;
	}

/*	{
		char szTemp[128];
		fprintf(stdout, "\nQuery = %s\n", szQueryText);
		for ( n = 0; n < pVocabLexicon->nNumLexicons; n++) {
			pPronLex = pVocabLexicon->pronDict + n;
			fprintf(stdout, "[%03d/%03d]", n, pPronLex->nLenPhoneSeq);
			for ( j = 0; j < pPronLex->nLenPhoneSeq; j++) {
				if ( pPronLex->flagSP[j] ) fprintf(stdout, " sp");
				fprintf(stdout, " %s", Symbol_Table[pPronLex->PhoneSeq[j]].symbol);
			}
			fprintf(stdout, "\n"); fflush(stdout);
		}
		scanf("%s", szTemp);
	}*/

	return 0;
}


/*
 *
 */
HCILAB_PRIVATE hci_int32
_compare_VocabString(const void *_x, const void *_y)
{
	const EXCEPT_DICT *x = (const EXCEPT_DICT*)_x;
	const EXCEPT_DICT *y = (const EXCEPT_DICT*)_y;

	return strcmp(x->szSrcVocab, y->szSrcVocab);
}


/*
 *
 */
HCILAB_PUBLIC hci_int32
TEXT2PHONE_loadExceptionalDict(const char *szFile)
{
	FILE *fpDict = 0;
	hci_int32 numWords = 0, iWord = 0;
	char szLineStr[1024];
	char *pszWordSeq[16];
	char szNewExcept[1024];
	char szVocab[1024];
	char szTemp[1024];
	char *pStr = 0;

	if ( szFile == 0 ) return 0;

	fpDict = fopen(szFile, "rt" );
	if ( 0 == fpDict ) return -1;

	while ( fgets(szLineStr, 1024, fpDict) ) {
		if ( strchr(szLineStr, '\t') ) {
			strcpy(szTemp, szLineStr);
			pStr = strchr(szTemp, '\t');
			if ( pStr ) *pStr = '\0';
			numWords = PowerASR_Base_str2words( szTemp, pszWordSeq, 16 );
			if ( numWords <= 0 ) continue;
			strcpy(szVocab, pszWordSeq[0]);
			for ( iWord = 1; iWord < numWords; iWord++)
				sprintf(szVocab, "%s %s", szVocab, pszWordSeq[iWord]);
			pStr = szVocab;
			while ( *pStr != '\0' ) {
				if ( (*pStr) & 0x80 ) pStr += 2;
				else if ( isalpha(*pStr) ) {
					*pStr = toupper(*pStr);
					pStr++;
				}
				else pStr++;
			}
			memset(szNewExcept, 0, sizeof(szNewExcept) );
			strcpy(szTemp, strchr(szLineStr, '\t')+1);
			numWords = PowerASR_Base_str2words( szTemp, pszWordSeq, 16 );
			if ( numWords <= 0 ) continue;
			strcpy(szNewExcept, pszWordSeq[0]);
			for ( iWord = 1; iWord < numWords; iWord++) 
				sprintf(szNewExcept, "%s;%s", szNewExcept, pszWordSeq[iWord]);
			sprintf(szNewExcept, "%s;", szNewExcept);
		}
		else {
			numWords = PowerASR_Base_str2words( szLineStr, pszWordSeq, 16 );
			if ( numWords <= 1 ) continue;
			strcpy(szVocab, pszWordSeq[0]);
			pStr = szVocab;
			while ( *pStr != '\0' ) {
				if ( (*pStr) & 0x80 ) pStr += 2;
				else if ( isalpha(*pStr) ) {
					*pStr = toupper(*pStr);
					pStr++;
				}
				else pStr++;
			}
			memset(szNewExcept, 0, sizeof(szNewExcept) );
			strcpy(szNewExcept, pszWordSeq[1]);
			for ( iWord = 2; iWord < numWords; iWord++) 
				sprintf(szNewExcept, "%s;%s", szNewExcept, pszWordSeq[iWord]);
			sprintf(szNewExcept, "%s;", szNewExcept);
		}

		if ( g_CountExceptDict ) {
			g_ExceptDict = (EXCEPT_DICT *) hci_realloc( g_ExceptDict, (g_CountExceptDict + 1) * sizeof(EXCEPT_DICT) );
		}
		else {
			g_ExceptDict = (EXCEPT_DICT *) hci_calloc( 1, sizeof(EXCEPT_DICT) );
		}

		g_ExceptDict[g_CountExceptDict].szSrcVocab = hci_salloc(szVocab);
		g_ExceptDict[g_CountExceptDict].szPronDict = hci_salloc(szNewExcept);
		g_CountExceptDict++;

	}

	fclose(fpDict); fpDict = 0;

// 	if ( g_CountExceptDict ) {
// 		qsort((char *)g_ExceptDict, (size_t)g_CountExceptDict, sizeof(EXCEPT_DICT), _compare_VocabString);
// 	}

	return 0;
}


HCILAB_PUBLIC hci_int32
TEXT2PHONE_loadEnUnitDict(const char *szUnitDictFile)
{
	FILE *fpDict = 0;
	hci_int32 numWords = 0, iWord = 0;
	char szLineStr[1024];
	char *pszWordSeq[16];
	char szNewExcept[1024];
	char szVocab[1024];
	char szTemp[1024];
	char *pStr = 0;
	
	if ( szUnitDictFile == 0 ) return 0;
	
	fpDict = fopen(szUnitDictFile, "rt" );
	if ( 0 == fpDict ) return -1;
	
	while ( fgets(szLineStr, 1024, fpDict) ) {
		if ( strchr(szLineStr, '\t') ) {
			strcpy(szTemp, szLineStr);
			pStr = strchr(szTemp, '\t');
			if ( pStr ) *pStr = '\0';
			numWords = PowerASR_Base_str2words( szTemp, pszWordSeq, 16 );
			if ( numWords <= 0 ) continue;
			strcpy(szVocab, pszWordSeq[0]);
			for ( iWord = 1; iWord < numWords; iWord++)
				sprintf(szVocab, "%s %s", szVocab, pszWordSeq[iWord]);
			pStr = szVocab;
			while ( *pStr != '\0' ) {
				if ( (*pStr) & 0x80 ) pStr += 2;
				else if ( isalpha(*pStr) ) {
					*pStr = toupper(*pStr);
					pStr++;
				}
				else pStr++;
			}
			memset(szNewExcept, 0, sizeof(szNewExcept) );
			strcpy(szTemp, strchr(szLineStr, '\t')+1);
			numWords = PowerASR_Base_str2words( szTemp, pszWordSeq, 16 );
			if ( numWords <= 0 ) continue;
			strcpy(szNewExcept, pszWordSeq[0]);
			for ( iWord = 1; iWord < numWords; iWord++) 
				sprintf(szNewExcept, "%s;%s", szNewExcept, pszWordSeq[iWord]);
			sprintf(szNewExcept, "%s;", szNewExcept);
		}
		else {
			numWords = PowerASR_Base_str2words( szLineStr, pszWordSeq, 16 );
			if ( numWords <= 1 ) continue;
			strcpy(szVocab, pszWordSeq[0]);
			pStr = szVocab;
			while ( *pStr != '\0' ) {
				if ( (*pStr) & 0x80 ) pStr += 2;
				else if ( isalpha(*pStr) ) {
					*pStr = toupper(*pStr);
					pStr++;
				}
				else pStr++;
			}
			memset(szNewExcept, 0, sizeof(szNewExcept) );
			strcpy(szNewExcept, pszWordSeq[1]);
			for ( iWord = 2; iWord < numWords; iWord++) 
				sprintf(szNewExcept, "%s;%s", szNewExcept, pszWordSeq[iWord]);
			sprintf(szNewExcept, "%s;", szNewExcept);
		}
		
		if ( g_CountUnitDict ) {
			g_UnitDict = (EXCEPT_DICT *) hci_realloc( g_UnitDict, (g_CountUnitDict + 1) * sizeof(EXCEPT_DICT) );
		}
		else {
			g_UnitDict = (EXCEPT_DICT *) hci_calloc( 1, sizeof(EXCEPT_DICT) );
		}
		
		g_UnitDict[g_CountUnitDict].szSrcVocab = hci_salloc(szVocab);
		g_UnitDict[g_CountUnitDict].szPronDict = hci_salloc(szNewExcept);
		g_CountUnitDict++;
		
	}
	
	fclose(fpDict); fpDict = 0;

	return 0;
}


HCILAB_PUBLIC hci_int32
TEXT2PHONE_createExceptDictHashTable()
{
	int n = 0;

	if ( g_CountExceptDict ) {
		hash_init( &g_exceptDict_ht, g_CountExceptDict );
		for ( n = 0; n < g_CountExceptDict; n++) {
			if ( hash_insert( &g_exceptDict_ht, g_ExceptDict[n].szSrcVocab, n ) != HASH_FAIL ) {
				fprintf(stderr, "[warning] duplicative exceptional dict: %s\n", g_ExceptDict[n].szSrcVocab);
			}
		}
		fprintf(stdout, "Exceptional Pron-Dict # = %d\n", g_CountExceptDict);
	}

	if ( g_CountUnitDict ) {
		hash_init( &g_unitDict_ht, g_CountUnitDict );
		for ( n = 0; n < g_CountUnitDict; n++) {
			if ( hash_insert( &g_unitDict_ht, g_UnitDict[n].szSrcVocab, n ) != HASH_FAIL ) {
				fprintf(stderr, "[warning] duplicative en-unit dict: %s\n", g_UnitDict[n].szSrcVocab);
			}
		}
		fprintf(stdout, "English-Uint # = %d\n", g_CountUnitDict);
	}

	return 0;
}

/*
 *
 */
HCILAB_PUBLIC hci_int32
TEXT2PHONE_freeExceptionalDict()
{
	if ( g_ExceptDict ) {
		int n = 0;
		for ( n = 0; n < g_CountExceptDict; n++) {
			if ( g_ExceptDict[n].szSrcVocab ) {
				hci_free(g_ExceptDict[n].szSrcVocab);
				g_ExceptDict[n].szSrcVocab = 0;
			}
			if ( g_ExceptDict[n].szPronDict ) {
				hci_free(g_ExceptDict[n].szPronDict);
				g_ExceptDict[n].szPronDict = 0;
			}
		}
		hci_free(g_ExceptDict);
		g_ExceptDict = 0;

		hash_destroy( &g_exceptDict_ht );
		g_CountExceptDict = 0;
	}

	if ( g_UnitDict ) {
		int n = 0;
		for ( n = 0; n < g_CountUnitDict; n++) {
			if ( g_UnitDict[n].szSrcVocab ) {
				hci_free(g_UnitDict[n].szSrcVocab);
				g_UnitDict[n].szSrcVocab = 0;
			}
			if ( g_UnitDict[n].szPronDict ) {
				hci_free(g_UnitDict[n].szPronDict);
				g_UnitDict[n].szPronDict = 0;
			}
		}
		hci_free(g_UnitDict);
		g_UnitDict = 0;
		
		hash_destroy( &g_unitDict_ht );
		g_CountUnitDict = 0;
	}

	return 0;
}


/*
 * find exceptional pronunciation dictionary matched with a given vocabulary string
 */
HCILAB_PRIVATE hci_flag
_find_exceptional_dict(EXCEPT_PRON *exceptDict,		///< (o) exceptional dictionary data
					   const char *szVocabName )	///< (i) query vocabulary string
{
	char *pStr = 0;
	int	iL = 0, iS = 0;
	int mid = 0;

	if ( 0 == g_ExceptDict || 0 == szVocabName ) return FALSE;
	if ( !g_CountExceptDict ) return FALSE;

	exceptDict->nCountDict = 0;

	mid = hash_lookup( &g_exceptDict_ht, szVocabName );

	if ( mid != HASH_FAIL ) {
		iL = iS = 0;
		pStr = g_ExceptDict[mid].szPronDict;
		while ( *pStr != '\0' ) {
			if ( *pStr == ':' ) pStr++;
			else if ( *pStr == ';' ) {
				exceptDict->szPron[iL][iS] = '\0';
				exceptDict->nCountDict++;
				if ( exceptDict->nCountDict >= 8 ) break;
				iL++; iS = 0;
				pStr++;
			}
			else {
				exceptDict->szPron[iL][iS++] = *pStr++;
			}
		}
		return TRUE;
	}
	else {
		return FALSE;
	}
}


/*
 * find english-unit pronunciation dictionary matched with a given vocabulary string
 */
HCILAB_PRIVATE hci_flag
_find_english_unit_dict(EXCEPT_PRON *exceptDict,		///< (o) exceptional dictionary data
						const char *szVocabName )		///< (i) query vocabulary string
{
	char *pStr = 0;
	int	iL = 0, iS = 0;
	int mid = 0;

	if ( 0 == g_UnitDict || 0 == szVocabName ) return FALSE;
	if ( !g_CountUnitDict ) return FALSE;

	exceptDict->nCountDict = 0;

	mid = hash_lookup( &g_unitDict_ht, szVocabName );

	if ( mid != HASH_FAIL ) {
		iL = iS = 0;
		pStr = g_UnitDict[mid].szPronDict;
		while ( *pStr != '\0' ) {
			if ( *pStr == ':' ) pStr++;
			else if ( *pStr == ';' ) {
				exceptDict->szPron[iL][iS] = '\0';
				exceptDict->nCountDict++;
				if ( exceptDict->nCountDict >= 8 ) break;
				iL++; iS = 0;
				pStr++;
			}
			else {
				exceptDict->szPron[iL][iS++] = *pStr++;
			}
		}
		return TRUE;
	}
	else {
		return FALSE;
	}
}

const char *g_szDigit2Hangul2 = "영일이삼사오육칠팔구";

HCILAB_PRIVATE int
_convertDigitString2HangulString( char *szDigit )
{
	char *pStr1 = 0, *pStr2 = 0, *pStr3 = 0, *pStr4 = 0, *pStr5 = 0;
	hci_int32 nLenDigit = (hci_int32)strlen(szDigit);
	hci_int32 i = 0, n = 0;
	hci_int32 iDigit = 0;
	hci_int32 nLenHangulStr = 0;
	char szHangulDigit[256];
	char *pOutStr = 0;

	if (0 == szDigit) return -1;
	if (0 == isdigit(szDigit[0])) return -1;
	if ( nLenDigit == 0 ) return -1;

	pStr1 = strchr(szDigit, '\n');
	if ( pStr1 ) *pStr1 = '\0';
	pStr2 = strchr(szDigit, '\r');
	if ( pStr2 ) *pStr2 = '\0';

	pOutStr  = szHangulDigit;
	*pOutStr = '\0';
	
	if ( nLenDigit == 1 ) {
		iDigit = (hci_int32)(szDigit[0] - '0');
		strncpy(pOutStr, g_szDigit2Hangul2+2*iDigit, 2);
		nLenHangulStr += 2;
	}
	else {
		for ( i = nLenDigit - 1, n = 0 ; i >= 0 && nLenHangulStr < 64; i--, n++) {
			iDigit = (hci_int32)(szDigit[n] - '0');
			if ( iDigit || (iDigit==0 && i%4==0) ) {
				if ( i == 0 && iDigit ) {	// 일의 자리
					strncpy(pOutStr+nLenHangulStr, g_szDigit2Hangul2+2*iDigit, 2);
					nLenHangulStr += 2;
				}
				else {	// 십의 자리 이상
					if ( iDigit > 1 ) {
						strncpy(pOutStr+nLenHangulStr, g_szDigit2Hangul2+2*iDigit, 2);
						nLenHangulStr += 2;
					}
					switch( i ) {
						case 1:
						case 5:
						case 9:
						case 13:	
							strcpy(pOutStr+nLenHangulStr, "십");
							nLenHangulStr += 2;
							break;
						case 2:
						case 6:
						case 10:
						case 14:	
							strcpy(pOutStr+nLenHangulStr, "백");
							nLenHangulStr += 2;
							break;
						case 3:
						case 7:
						case 11:
						case 15:	
							strcpy(pOutStr+nLenHangulStr, "천");
							nLenHangulStr += 2;
							break;
						case 4:
							strcpy(pOutStr+nLenHangulStr, "만");
							nLenHangulStr += 3;
							break;
						case 8:
							strcpy(pOutStr+nLenHangulStr, "억");
							nLenHangulStr += 3;
							break;
						case 12:
							strcpy(pOutStr+nLenHangulStr, "조");
							nLenHangulStr += 3;
							break;
					}
				}
			}
		}
	}
	
	szHangulDigit[nLenHangulStr] = '\0';

	while ( 1 ) {
		pStr1 = strstr(szHangulDigit, "십육");
		if ( pStr1 ) strncpy( pStr1, "심뉵", 4);
		pStr2 = strstr(szHangulDigit, "백육");
		if ( pStr2 ) strncpy( pStr2, "뱅뉵", 4);
		pStr3 = strstr(szHangulDigit, "천육");
		if ( pStr3 ) strncpy( pStr3, "천뉵", 4);
		pStr4 = strstr(szHangulDigit, "만육");
		if ( pStr4 ) strncpy( pStr4, "만뉵", 4);
		pStr4 = strstr(szHangulDigit, "만육");
		if ( pStr4 ) strncpy( pStr4, "만뉵", 4);
		pStr5 = strstr(szHangulDigit, "억육");
		if ( pStr5 ) strncpy( pStr5, "엉뉵", 4);
		if ( !pStr1 && !pStr2 && !pStr3 && !pStr4 && !pStr5 ) break;
	}

	strcpy( szDigit, szHangulDigit );

	return 0;
}

HCILAB_PRIVATE int
_convertDigitString2CountalbeHangulString( char *szDigit, int nDType )
{
	char *pStr1 = 0, *pStr2 = 0, *pStr3 = 0, *pStr4 = 0, *pStr5 = 0;
	hci_int32 nLenDigit = (hci_int32)strlen(szDigit);
	hci_int32 i = 0, n = 0;
	hci_int32 iDigit = 0;
	hci_int32 nLenHangulStr = 0;
	char szHangulDigit[256];
	char *pOutStr = 0;

	if (0 == szDigit) return -1;
	if (0 == isdigit(szDigit[0])) return -1;
	if ( nLenDigit == 0 ) return -1;

	pStr1 = strchr(szDigit, '\n');
	if ( pStr1 ) *pStr1 = '\0';
	pStr2 = strchr(szDigit, '\r');
	if ( pStr2 ) *pStr2 = '\0';

	pOutStr  = szHangulDigit;
	*pOutStr = '\0';

	if ( nLenDigit == 1 ) {
		iDigit = (hci_int32)(szDigit[0] - '0');
		if ( iDigit == 1 && nDType == 2 ) {
			strcpy(pOutStr, "첫");
			nLenHangulStr += strlen("첫");
		}
		else {
			strcpy(pOutStr, PRON_D2CK[iDigit]);
			nLenHangulStr += strlen(PRON_D2CK[iDigit]);
		}
	}
	else {
		for ( i = nLenDigit - 1, n = 0 ; i >= 2 ; i--, n++) {
			iDigit = (hci_int32)(szDigit[n] - '0');
			if ( iDigit || (iDigit==0 && i%4==0) ) {
				if ( i == 0 && iDigit ) {	// 일의 자리
					strncpy(pOutStr+nLenHangulStr, g_szDigit2Hangul2+2*iDigit, 2);
					nLenHangulStr += 2;
				}
				else {	// 십의 자리 이상
					if ( iDigit > 1 ) {
						strncpy(pOutStr+nLenHangulStr, g_szDigit2Hangul2+2*iDigit, 2);
						nLenHangulStr += 2;
					}
					switch( i ) {
						case 1:
						case 5:
						case 9:
						case 13:	
							strcpy(pOutStr+nLenHangulStr, "십");
							nLenHangulStr += 2;
							break;
						case 2:
						case 6:
						case 10:
						case 14:	
							strcpy(pOutStr+nLenHangulStr, "백");
							nLenHangulStr += 2;
							break;
						case 3:
						case 7:
						case 11:
						case 15:	
							strcpy(pOutStr+nLenHangulStr, "천");
							nLenHangulStr += 2;
							break;
						case 4:
							strcpy(pOutStr+nLenHangulStr, "만");
							nLenHangulStr += 3;
							break;
						case 8:
							strcpy(pOutStr+nLenHangulStr, "억");
							nLenHangulStr += 3;
							break;
						case 12:
							strcpy(pOutStr+nLenHangulStr, "조");
							nLenHangulStr += 3;
							break;
					}
				}
			}
		}
		if ( i == 1 ) {
			iDigit = (hci_int32)(szDigit[n] - '0');
			if ( iDigit ) {
				if ( iDigit==2 && szDigit[n+1] == '0' ) {
					strcpy(pOutStr+nLenHangulStr, "스무");
					nLenHangulStr += 4;
				}
				else {
					strcpy(pOutStr+nLenHangulStr, PRON_DD2CK[iDigit]);
					nLenHangulStr += strlen(PRON_DD2CK[iDigit]);
				}
			}
			i--; n++;
		}
		if ( i == 0 && szDigit[n] != '0' ) {
			iDigit = (hci_int32)(szDigit[n] - '0');
			strcpy(pOutStr+nLenHangulStr, PRON_D2CK[iDigit]);
			nLenHangulStr += strlen(PRON_D2CK[iDigit]);
		}
	}
	
	szHangulDigit[nLenHangulStr] = '\0';

	while ( 1 ) {
		pStr1 = strstr(szHangulDigit, "십육");
		if ( pStr1 ) strncpy( pStr1, "심뉵", 4);
		pStr2 = strstr(szHangulDigit, "백육");
		if ( pStr2 ) strncpy( pStr2, "뱅뉵", 4);
		pStr3 = strstr(szHangulDigit, "천육");
		if ( pStr3 ) strncpy( pStr3, "천뉵", 4);
		pStr4 = strstr(szHangulDigit, "만육");
		if ( pStr4 ) strncpy( pStr4, "만뉵", 4);
		pStr4 = strstr(szHangulDigit, "만육");
		if ( pStr4 ) strncpy( pStr4, "만뉵", 4);
		pStr5 = strstr(szHangulDigit, "억육");
		if ( pStr5 ) strncpy( pStr5, "엉뉵", 4);
		if ( !pStr1 && !pStr2 && !pStr3 && !pStr4 && !pStr5 ) break;
	}

	strcpy( szDigit, szHangulDigit );

	if ( nLenHangulStr ) return 0;
	else return -1;
}


/*
 *	한글 단위형 명사인지를 체크.
 *		- "-사람", "-가지", "-갈래", "-개", "-시", "-단계', "-살", "-줄", "-평", "-번째", "-마리", "-명"
 */
HCILAB_PRIVATE int
CheckHangulUnit( char *szKrUnit )
{
	if ( strncmp(szKrUnit, "개", 2) == 0 ) return 1;
	else if ( strncmp(szKrUnit, "시", 2) == 0 ) return 1;
	else if ( strncmp(szKrUnit, "살", 2) == 0 ) return 1;
	else if ( strncmp(szKrUnit, "줄", 2) == 0 ) return 1;
	else if ( strncmp(szKrUnit, "평", 2) == 0 ) return 1;
	else if ( strncmp(szKrUnit, "명", 2) == 0 ) return 1;
	else if ( strncmp(szKrUnit, "사람", 4) == 0 ) return 1;
	else if ( strncmp(szKrUnit, "가지", 4) == 0 ) return 1;
	else if ( strncmp(szKrUnit, "갈래", 4) == 0 ) return 1;
	else if ( strncmp(szKrUnit, "단계", 4) == 0 ) return 1;
	else if ( strncmp(szKrUnit, "번째", 4) == 0 ) return 2;
	else if ( strncmp(szKrUnit, "마리", 4) == 0 ) return 1;
	else if ( strncmp(szKrUnit, "시간", 4) == 0 ) return 1;
	else return 0;
}

/*
 * process alphabetic & numerical query
 */
_process_alphanumeric_word(EXCEPT_PRON *exceptDict,		///< (o) exceptional dictionary data
						   const char *szVocabName )	///< (i) query vocabulary string
{
	char szQuery[256];
	char *pStr = 0;
	char *pOut = 0;
	int n = 0, i = 0, j = 0;
	hci_uint32 k = 0;
	int nCountAlphaNum = 0;
	int nCountCompound = 0;
	MORPH_SEQ morphSeq;
	int prevType = UNK_MORPH;
	int countMorph = 0;
	hci_flag bAlphaDigit = 0;
	EXCEPT_PRON morphExcept;
	char szCompound[256];
	
	if ( 0 == g_ExceptDict || 0 == szVocabName ) return FALSE;

	if ( PowerASR_Base_strnocasencmp(szVocabName, "0x", 2) == 0 ) return FALSE;

	memset( &morphSeq, 0, sizeof(morphSeq) );

	memset(szQuery, 0, sizeof(szQuery));
	strcpy(szQuery, szVocabName);

	pStr = szQuery;
	morphSeq.morphType[countMorph] = 0;
	pOut = &(morphSeq.szMorph[countMorph][0]);
	while ( *pStr != '\0' ) {
		if ( (*pStr) & 0x80 ) {
			if ( prevType == UNK_MORPH ) {
				morphSeq.morphType[countMorph] = KR_MORPH;
			}
			else if ( prevType != KR_MORPH ) {
				*pOut = '\0';
				countMorph++;
				if ( countMorph >= 16 ) {
					exceptDict->nCountDict = 0;
					return FALSE;
				}
				morphSeq.morphType[countMorph] = KR_MORPH;
				pOut = &(morphSeq.szMorph[countMorph][0]);
			}
			*pOut++ = *pStr++;
			*pOut++ = *pStr++;
			prevType = KR_MORPH;
		}
		else if ( isalpha(*pStr) ) {
			if ( prevType == UNK_MORPH ) {
				morphSeq.morphType[countMorph] = ALPHA_MORPH;
				bAlphaDigit = 1;
			}
			else if ( prevType != ALPHA_MORPH ) {
				*pOut = '\0';
				countMorph++;
				if ( countMorph >= 16 ) {
					exceptDict->nCountDict = 0;
					return FALSE;
				}
				bAlphaDigit = 1;
				morphSeq.morphType[countMorph] = ALPHA_MORPH;
				pOut = &(morphSeq.szMorph[countMorph][0]);
			}
			*pOut++ = *pStr++;
			prevType = ALPHA_MORPH;
		}
		else if ( isdigit(*pStr) ) {
			if ( prevType == UNK_MORPH ) {
//				bAlphaDigit = 1;
				morphSeq.morphType[countMorph] = DIGIT_MORPH;
			}
			else if ( prevType != DIGIT_MORPH ) {
				*pOut = '\0';
				countMorph++;
				if ( countMorph >= 16 ) {
					exceptDict->nCountDict = 0;
					return FALSE;
				}
//				bAlphaDigit = 1;
				morphSeq.morphType[countMorph] = DIGIT_MORPH;
				pOut = &(morphSeq.szMorph[countMorph][0]);
			}
			*pOut++ = *pStr++;
			prevType = DIGIT_MORPH;
		}
		else {
			if ( (*pStr == '.' || *pStr == '-') && prevType == DIGIT_MORPH ) {
				*pOut++ = *pStr++;
			}
			else {
				if ( prevType != UNK_MORPH ) {
					*pOut = '\0';
					countMorph++;
					if ( countMorph >= 16 ) {
						exceptDict->nCountDict = 0;
						return FALSE;
					}
				}
				morphSeq.morphType[countMorph] = SYMBOL_MORPH;
				pOut = &(morphSeq.szMorph[countMorph][0]);
				prevType = SYMBOL_MORPH;
				*pOut++ = *pStr++;
			}
		}
	}
	if ( prevType != UNK_MORPH ) {
		*pOut = '\0';
		countMorph++;
	}
	morphSeq.nCountMorph = countMorph;

//	if ( !countMorph || morphSeq.morphType[0] == SYMBOL_MORPH ) return FALSE;
	if ( !countMorph ) return FALSE;

	exceptDict->nCountDict = 0;
	for ( n = 0; n < countMorph; n++) {
		memset(&morphExcept, 0, sizeof(morphExcept));
		if ( (n+3) < countMorph ) {
			nCountCompound = 3;
			sprintf(szCompound, "%s%s", morphSeq.szMorph[n], morphSeq.szMorph[n+1]);
			strcat(szCompound, morphSeq.szMorph[n+2]);
			strcat(szCompound, morphSeq.szMorph[n+3]);
			_find_exceptional_dict( &morphExcept, szCompound );
		}
		if ( morphExcept.nCountDict == 0 && (n+2) < countMorph ) {
			nCountCompound = 2;
			sprintf(szCompound, "%s%s", morphSeq.szMorph[n], morphSeq.szMorph[n+1]);
			strcat(szCompound, morphSeq.szMorph[n+2]);
			_find_exceptional_dict( &morphExcept, szCompound );
		}
		if ( morphExcept.nCountDict == 0 && (n+1) < countMorph ) {
			nCountCompound = 1;
			sprintf(szCompound, "%s%s", morphSeq.szMorph[n], morphSeq.szMorph[n+1]);
			_find_exceptional_dict( &morphExcept, szCompound );
		}
		if ( morphExcept.nCountDict == 0 ) {
			nCountCompound = 0;
			strcpy(szCompound, morphSeq.szMorph[n]);
			_find_exceptional_dict( &morphExcept, szCompound );
		}
		if ( morphExcept.nCountDict ) {
			if ( exceptDict->nCountDict ) {
				int num_out = exceptDict->nCountDict * morphExcept.nCountDict;
				if ( num_out > 16 ) {
					num_out = 16;
				}
				for ( i = exceptDict->nCountDict; i < num_out; i++) {
					strcpy(exceptDict->szPron[i], exceptDict->szPron[i%exceptDict->nCountDict]);
				}
				j = 0;
				for ( i = 0; i < num_out; i++, j++) {
					if ( (strlen(exceptDict->szPron[i]) + strlen(morphExcept.szPron[j%morphExcept.nCountDict])) >= 64 ) {
						memset(exceptDict, 0, sizeof(EXCEPT_PRON));
						return FALSE;
					}
					strcat(exceptDict->szPron[i], morphExcept.szPron[j%morphExcept.nCountDict]);
				}
				exceptDict->nCountDict = num_out;
			}
			else {
				memcpy(exceptDict, &morphExcept, sizeof(morphExcept));
			}
			n += nCountCompound;
		}
		else {
			if ( morphSeq.morphType[n] == KR_MORPH ) {
				if ( exceptDict->nCountDict ) {
					for ( i = 0; i < exceptDict->nCountDict; i++) {
						if ( (strlen(exceptDict->szPron[i]) + strlen(morphSeq.szMorph[n])) >= 64 ) {
							memset(exceptDict, 0, sizeof(EXCEPT_PRON));
							return FALSE;
						}
						strcat(exceptDict->szPron[i], morphSeq.szMorph[n]);
					}
				}
				else {
					exceptDict->nCountDict = 1;
					strcpy( exceptDict->szPron[0], morphSeq.szMorph[n] );
				}
			}
			else if ( morphSeq.morphType[n] == ALPHA_MORPH ) {	
				char szAlphaStr[128];
				memset(szAlphaStr, 0, sizeof(szAlphaStr));
				if ( n && morphSeq.morphType[n-1] == DIGIT_MORPH && _find_english_unit_dict(&morphExcept, morphSeq.szMorph[n]) ) {
					pStr = &(morphSeq.szMorph[n][0]);
					pOut = szAlphaStr;
					while ( *pStr != '\0' ) {
						k = Eng2Kor(*pStr, pOut);
						pStr++; pOut += k;
						if ( (pOut-szAlphaStr) >= 64 ) {
							memset(exceptDict, 0, sizeof(EXCEPT_PRON));
							return FALSE;
						}
					}
					strcpy(morphExcept.szPron[morphExcept.nCountDict], szAlphaStr);
					morphExcept.nCountDict += 1;
				}
				else {
					pStr = &(morphSeq.szMorph[n][0]);
					pOut = szAlphaStr;
					while ( *pStr != '\0' ) {
						k = Eng2Kor(*pStr, pOut);
						pStr++; pOut += k;
						if ( (pOut-szAlphaStr) >= 64 ) {
							memset(exceptDict, 0, sizeof(EXCEPT_PRON));
							return FALSE;
						}
					}
					morphExcept.nCountDict = 1;
					strcpy(morphExcept.szPron[0], szAlphaStr);
				}
				if ( exceptDict->nCountDict ) {
					int num_out = exceptDict->nCountDict * morphExcept.nCountDict;
					if ( num_out > 16 ) {
						num_out = 16;
					}
					for ( i = exceptDict->nCountDict; i < num_out; i++) {
						strcpy(exceptDict->szPron[i], exceptDict->szPron[i%exceptDict->nCountDict]);
					}
					j = 0;
					for ( i = 0; i < num_out; i++, j++) {
						if ( (strlen(exceptDict->szPron[i]) + strlen(morphExcept.szPron[j%morphExcept.nCountDict])) >= 64 ) {
							memset(exceptDict, 0, sizeof(EXCEPT_PRON));
							return FALSE;
						}
						strcat(exceptDict->szPron[i], morphExcept.szPron[j%morphExcept.nCountDict]);
					}
					exceptDict->nCountDict = num_out;
				}
				else {
					memcpy(exceptDict, &morphExcept, sizeof(morphExcept));
				}
				nCountAlphaNum++;
			}
			else if ( morphSeq.morphType[n] == DIGIT_MORPH ) {
				char szDigit[128];
				char szDigitStr[128];
				strcpy(szDigit, morphSeq.szMorph[n]);
				pStr = strchr(szDigit, '.');
				if ( pStr ) {
					char szFront[128];
					strcpy(szFront, szDigit);
					pStr = strchr(szFront, '.'); *pStr = '\0';
					if ( strcmp(szFront, "0") == 0 ) {
						strcpy(szFront, "영");
					}
					else {
						_convertDigitString2HangulString(szFront);
					}
					if ( exceptDict->nCountDict ) {
						for ( i = 0; i < exceptDict->nCountDict; i++) {
							if ( (strlen(exceptDict->szPron[i]) + strlen(szFront)) >= 64 ) {
								memset(exceptDict, 0, sizeof(EXCEPT_PRON));
								return FALSE;
							}
							strcat(exceptDict->szPron[i], szFront);
						}
					}
					else {
						exceptDict->nCountDict = 1;
						strcpy( exceptDict->szPron[0], szFront );
					}
					for ( i = 0; i < exceptDict->nCountDict; i++) {
						if ( (strlen(exceptDict->szPron[i]) + strlen("쩜")) >= 64 ) {
							memset(exceptDict, 0, sizeof(EXCEPT_PRON));
							return FALSE;
						}
						strcat(exceptDict->szPron[i], "쩜");
					}
					pStr = strchr(szDigit, '.');
					strcpy(szFront, pStr+1);
					memset(szDigitStr, 0, sizeof(szDigitStr));
					pStr = szFront;
					pOut = szDigitStr;
					while ( *pStr != '\0' ) {
						if ( *pStr == '.' ) {
							strcpy(pOut, "쩜");
							pOut += 2;
						}
						else {
							k = Digit2Kor(*pStr, pOut);
							pOut += k;
						}
						if ( (pOut-szDigitStr) > 64 ) {
							memset(exceptDict, 0, sizeof(EXCEPT_PRON));
							return FALSE;
						}
						pStr++;
					}
					for ( i = 0; i < exceptDict->nCountDict; i++) {
						if ( (strlen(exceptDict->szPron[i]) + strlen(szDigitStr)) >= 64 ) {
							memset(exceptDict, 0, sizeof(EXCEPT_PRON));
							return FALSE;
						}
						strcat(exceptDict->szPron[i], szDigitStr);
					}
				}
				else if ( strchr(szDigit, '-') ) {
					char szFront[128];
					memset(szDigitStr, 0, sizeof(szDigitStr));
					strcpy(szFront, szDigit);
					pStr = strchr(szFront, '-'); *pStr = '\0';
					if ( strcmp(szFront, "0") == 0 ) {
						strcpy(szFront, "영");
					}
					else {
						_convertDigitString2HangulString(szFront);
					}
					if ( exceptDict->nCountDict ) {
						for ( i = 0; i < exceptDict->nCountDict; i++) {
							if ( (strlen(exceptDict->szPron[i]) + strlen(szFront)) >= 64 ) {
								memset(exceptDict, 0, sizeof(EXCEPT_PRON));
								return FALSE;
							}
							strcat(exceptDict->szPron[i], szFront);
						}
					}
					else {
						exceptDict->nCountDict = 1;
						strcpy( exceptDict->szPron[0], szFront );
					}
					for ( i = 0; i < exceptDict->nCountDict; i++) {
						strcat(exceptDict->szPron[i], "다시");
					}
					pStr = strchr(szDigit, '-'); pStr++;
					memset(szFront, 0, sizeof(szFront));
					pOut = szFront;	
					while ( *pStr != '\0' ) {
						if ( isdigit(*pStr) ) {
							*pOut++ = *pStr;
						}
						else if ( *pStr == '-' ) {
							_convertDigitString2HangulString(szFront);
							for ( i = 0; i < exceptDict->nCountDict; i++) {
								if ( (strlen(exceptDict->szPron[i]) + strlen(szFront) + 4) >= 64 ) {
									memset(exceptDict, 0, sizeof(EXCEPT_PRON));
									return FALSE;
								}
								strcat(exceptDict->szPron[i], szFront);
								strcat(exceptDict->szPron[i], "다시");
							}
							memset(szFront, 0, sizeof(szFront));
							pOut = szFront;	
						}
						else {
							_convertDigitString2HangulString(szFront);
							for ( i = 0; i < exceptDict->nCountDict; i++) {
								if ( (strlen(exceptDict->szPron[i]) + strlen(szFront)) >= 64 ) {
									memset(exceptDict, 0, sizeof(EXCEPT_PRON));
									return FALSE;
								}
								strcat(exceptDict->szPron[i], szFront);
							}
							memset(szFront, 0, sizeof(szFront));
							pOut = szFront;	
						}
						pStr++;
					}
					if ( strlen(szFront) ) {
						_convertDigitString2HangulString(szFront);
						for ( i = 0; i < exceptDict->nCountDict; i++) {
							if ( (strlen(exceptDict->szPron[i]) + strlen(szFront)) >= 64 ) {
								memset(exceptDict, 0, sizeof(EXCEPT_PRON));
								return FALSE;
							}
							strcat(exceptDict->szPron[i], szFront);
						}
					}
				}
				else if ( szDigit[0] != '0' ) {
					int iDType = 0;
					if ( (strcmp(szDigit,"6") == 0 || strcmp(szDigit,"10")==0) && (n+1) < countMorph &&
						strncmp(morphSeq.szMorph[n+1], "월", 2)==0 ) {	// "6월", "10월"
						if ( strcmp(szDigit, "6") == 0 ) {
							strcpy(morphExcept.szPron[0], "유");
							morphExcept.nCountDict = 1;
						}
						else {
							strcpy(morphExcept.szPron[0], "시");
							morphExcept.nCountDict = 1;
						}
					}
					else if ( (n+1) < countMorph && morphSeq.morphType[n+1] == KR_MORPH && 
						(iDType=CheckHangulUnit(morphSeq.szMorph[n+1])) ) {
						strcpy(szDigitStr, szDigit);
						_convertDigitString2HangulString(szDigitStr);
						strcpy(morphExcept.szPron[0], szDigitStr);
						morphExcept.nCountDict = 1;
						memset(szDigitStr, 0, sizeof(szDigitStr));
						strcpy(szDigitStr, szDigit);
						if ( _convertDigitString2CountalbeHangulString(szDigitStr, iDType) >= 0 ) {
							strcpy(morphExcept.szPron[morphExcept.nCountDict], szDigitStr);
							morphExcept.nCountDict += 1;
						}
						if ( strlen(szDigit) == 1 ) {
							pStr = szDigit;
							pOut = szDigitStr;
							k = Digit2Eng(*pStr, pOut);
							pOut[k] = '\0';
							strcpy(morphExcept.szPron[morphExcept.nCountDict], szDigitStr);
							morphExcept.nCountDict += 1;
						}
					}
					else {
						strcpy(szDigitStr, szDigit);
						_convertDigitString2HangulString(szDigitStr);
						strcpy(morphExcept.szPron[0], szDigitStr);
						morphExcept.nCountDict = 1;
						memset(szDigitStr, 0, sizeof(szDigitStr));
						if ( strlen(szDigit) == 1 ) {
							pStr = szDigit;
							pOut = szDigitStr;
							k = Digit2Eng(*pStr, pOut);
							pOut[k] = '\0';
						}
						else {
							pStr = szDigit;
							pOut = szDigitStr;
							while ( *pStr != '\0' ) {
								if ( isdigit(*pStr) ) {
									k = Digit2Kor(*pStr, pOut);
									pOut += k;
								}
								if ( (pOut-szDigitStr) > 64 ) {
									memset(exceptDict, 0, sizeof(EXCEPT_PRON));
									return FALSE;
								}
								pStr++;
							}
							*pOut = '\0';
						}
						if ( strlen(szDigitStr) ) {
							strcpy(morphExcept.szPron[1], szDigitStr);
							morphExcept.nCountDict = 2;
						}
					}
					if ( exceptDict->nCountDict ) {
						int num_out = exceptDict->nCountDict * morphExcept.nCountDict;
						if ( num_out > 16 ) {
							num_out = 16;
						}
						for ( i = exceptDict->nCountDict; i < num_out; i++) {
							strcpy(exceptDict->szPron[i], exceptDict->szPron[i%exceptDict->nCountDict]);
						}
						j = 0;
						for ( i = 0; i < num_out; i++, j++) {
							if ( (strlen(exceptDict->szPron[i]) + strlen(morphExcept.szPron[j%morphExcept.nCountDict])) >= 64 ) {
								memset(exceptDict, 0, sizeof(EXCEPT_PRON));
								return FALSE;
							}
							strcat(exceptDict->szPron[i], morphExcept.szPron[j%morphExcept.nCountDict]);
						}
						exceptDict->nCountDict = num_out;
					}
					else {
						memcpy(exceptDict, &morphExcept, sizeof(morphExcept));
					}
				}
				else {
					memset(szDigitStr, 0, sizeof(szDigitStr));
					pStr = &(morphSeq.szMorph[n][0]);
					pOut = szDigitStr;
					while ( *pStr != '\0' ) {
						if ( isdigit(*pStr) ) {
							k = Digit2Kor(*pStr, pOut);
							pOut += k;
						}
						pStr++;
					}
					if ( exceptDict->nCountDict ) {
						for ( i = 0; i < exceptDict->nCountDict; i++) {
							if ( (strlen(exceptDict->szPron[i]) + strlen(szDigitStr)) >= 64 ) {
								memset(exceptDict, 0, sizeof(EXCEPT_PRON));
								return FALSE;
							}
							strcat(exceptDict->szPron[i], szDigitStr);
						}
					}
					else {
						exceptDict->nCountDict = 1;
						strcpy( exceptDict->szPron[0], szDigitStr );
					}
				}
			}
			else {	// special symbol
				if ( strcmp(morphSeq.szMorph[n], "%") == 0 ) {
					morphExcept.nCountDict = 2;
					strcpy(morphExcept.szPron[0], "퍼센트");
					strcpy(morphExcept.szPron[1], "프로");
				}
				else {
					morphExcept.nCountDict = 0;
				}
				if ( morphExcept.nCountDict ) {
					if ( exceptDict->nCountDict ) {
						int num_out = exceptDict->nCountDict * morphExcept.nCountDict;
						if ( num_out > 16 ) {
							num_out = 16;
						}
						for ( i = exceptDict->nCountDict; i < num_out; i++) {
							strcpy(exceptDict->szPron[i], exceptDict->szPron[i%exceptDict->nCountDict]);
						}
						j = 0;
						for ( i = 0; i < num_out; i++, j++) {
							if ( (strlen(exceptDict->szPron[i]) + strlen(morphExcept.szPron[j%morphExcept.nCountDict])) >= 64 ) {
								memset(exceptDict, 0, sizeof(EXCEPT_PRON));
								return FALSE;
							}
							strcat(exceptDict->szPron[i], morphExcept.szPron[j%morphExcept.nCountDict]);
						}
						exceptDict->nCountDict = num_out;
					}
					else {
						memcpy(exceptDict, &morphExcept, sizeof(morphExcept));
					}
				}
			}
		}
	}
/*
//	if ( bAlphaDigit && exceptDict->nCountDict ) {
	if ( exceptDict->nCountDict ) {
 		for ( i = 0; i < countMorph; i++) {
 			fprintf(stdout, "morph[%d] %s (%d)\n", i+1, morphSeq.szMorph[i], morphSeq.morphType[i]);
 		}
 		fprintf(stdout, "%s\t%s", szVocabName, exceptDict->szPron[0]);
		for ( i = 1; i < exceptDict->nCountDict; i++) {
			fprintf(stdout, " %s", exceptDict->szPron[i]);
		}
 		fprintf(stdout, "\n"); fflush(stdout);
	}
*/
	return TRUE;
}


/*
 * convert an input word into pronunciation lexicons
 */
HCILAB_PRIVATE hci_flag
MakeMonoDic(SRC_LEXICON *pWordLex,				///< (o) pronunciation lexicons
			const char *word,					///< (i) vocabulary string
			const hci_uint32 nMaxLexiconCount,	///< (i) maximum number of pronunciation lexicons
			const hci_flag bSegmRead)			///< (i) flag to segmental pronunciation
{
	int nCount;
	char wordcode[1024], tmps[1024], tmps2[1024];
//	int ex, ex_pos;
	int ex[1024];
	int ex_pos[1024];
	int num_ex = 0;
	int num_ex2 = 0;
	int i = 0, j = 0, k = 0;
	hci_uint32 n = 0;

	int cur = 0, num_boots = 0;
	int bFind = 0;
	char words2[MAX_LEN_LEXICON];
	int  bOmitH = 0;
	int word_len = 0;
	hci_uint16 nlex = 0;

	memset(wordcode, 0, sizeof(wordcode));
	memset(tmps, 0, sizeof(tmps));
	memset(tmps2, 0, sizeof(tmps2));
	memset(words2, 0, sizeof(words2));

	if ((word_len = GetCode(word, wordcode)) == -1) {
		return FALSE;
	}

	// 메모리 할당 및 초기화
	num_ex = 0;
	if (num_ex = CheckException(wordcode, ex, ex_pos)) 
		nlex = num_ex + 1;
	else
		nlex = 1;

#ifdef OMITH
	bOmitH = 0;
	if (IsOmitH(wordcode)) {
		bOmitH = 1;
		nlex++;
	}
#endif

// #ifdef READ_PER_EUMJOL
// 	if (word_len > 2)
// 		pWordLex->nNumMonoLex = nlex + 1; // 1: 끊어읽기 (끊어읽기는 opt_rule, omit_h엔 적용안됨)
// 	else 
// 		pWordLex->nNumMonoLex = nlex;		
// #else
// 	pWordLex->nNumMonoLex = nlex;
// #endif

	if ( bSegmRead ) {
		if (word_len > 2)
			pWordLex->nNumMonoLex = nlex + 1; // 1: 끊어읽기 (끊어읽기는 opt_rule, omit_h엔 적용안됨)
		else 
			pWordLex->nNumMonoLex = nlex;		
	}
	else {
		pWordLex->nNumMonoLex = nlex;
	}

	nCount = 0;

	/////////// 연속으로 읽기

	//exclusive lexicon - 기본과 ㄴ첨가 순서 바꿈

	// 1. 기본
	num_ex = CheckException4(wordcode, ex, ex_pos); // 'ㄹ' 경음화

	if (!reading_by_rule(wordcode, ex, ex_pos, num_ex, tmps)) {
		return FALSE;
	}

	if ( strlen(tmps) == 0 ) return FALSE;

	write_pause(tmps, tmps2);
	//sunhci 20060119 S_SIL
	sprintf(tmps, "%s", tmps2);
	if ((n=strlen(tmps)) > MAX_LEN_LEXICON) {
		tmps[MAX_LEN_LEXICON-1] = '\0';
		pWordLex->nLenMonoLex[nCount] = MAX_LEN_LEXICON - 1;
	} else {
		pWordLex->nLenMonoLex[nCount] = (hci_uint16)n;
	}
	strcpy(pWordLex->szMonoLex[nCount++], tmps);

	// 2. 'ㄴ' 첨가
	num_ex2 = 0;
	if (num_ex2 = CheckException(wordcode, ex+num_ex, ex_pos+num_ex)) 
	{
		num_ex += num_ex2;
		if (!reading_by_rule(wordcode, ex, ex_pos, num_ex, tmps)) {
			return FALSE;
		}

		write_pause(tmps, tmps2); 
        //sunhci 20060119 S_SIL
		sprintf(tmps, "%s", tmps2); // 030104:sil->sp (name_sub)
		if ((n=strlen(tmps)) > MAX_LEN_LEXICON) {
			tmps[MAX_LEN_LEXICON-1] = '\0'; // 030109
			pWordLex->nLenMonoLex[nCount] = MAX_LEN_LEXICON - 1;
		} else {
			pWordLex->nLenMonoLex[nCount] = (hci_uint16)n;
		}
		strcpy(pWordLex->szMonoLex[nCount++], tmps);
	} 

	// 4. 'ㅎ'탈락
#ifdef OMITH
	if (bOmitH) {
		cur = nCount;
		for (i=0; i<cur; i++) {
			bFind = 0;
			num_boots = (int)pWordLex->nLenMonoLex[i];
			for (k=1; k<num_boots; k++) {
				if (pWordLex->szMonoLex[i][k] == 'h') {
					if (pWordLex->szMonoLex[i][k-1] == 'N') {
						bFind = 1;
						words2[k-1] = 'n';
					}
					else if (pWordLex->szMonoLex[i][k-1] == 'L') {
						bFind = 1;
						words2[k-1] = 'r';
					}
					else if (pWordLex->szMonoLex[i][k-1] == 'M') {
						bFind = 1;
						words2[k-1] = 'm';
					} else { // OO, 모음 case
						bFind = 1;
						words2[k-1] = pWordLex->szMonoLex[i][k-1];
					}
					words2[k] = '*';
				} else
					words2[k] = pWordLex->szMonoLex[i][k];
			}
			words2[0] = pWordLex->szMonoLex[i][0];
			if (bFind) {
				for (k=0, j=0; k<num_boots; k++) {
					if (words2[k] != '*')
						pWordLex->szMonoLex[nCount][j++] = words2[k];
				}
				pWordLex->szMonoLex[nCount][j] = '\0'; 
				pWordLex->nLenMonoLex[nCount] = (hci_uint16)j;
				nCount++;
			} /*else { // (정상적으론 이곳에 들어오면 안되는데 코드에 문제가 있음)
				pWordLex->nNumMonoLex--;
			}*/
		}
	}
#endif

		/////////// 끊어서 읽기
// #ifdef READ_PER_EUMJOL
// 	if (word_len > 2) {
// 		reading_per_eumjol(wordcode, tmps);
// 		//sunhci 20060119 S_SIL
// 		sprintf(tmps2, "%s", tmps);
// 		if ((n=strlen(tmps2)) > MAX_LEN_LEXICON) {
// 			tmps2[MAX_LEN_LEXICON-1] = '\0'; // 030109
// 			pWordLex->nLenMonoLex[nCount] = MAX_LEN_LEXICON - 1;
// 			//err_ret("error: exceed max boots");
// 			//return FALSE;
// 		} else
// 			pWordLex->nLenMonoLex[nCount] = (hci_uint16)n;
// 
// 		strcpy(pWordLex->szMonoLex[nCount++], tmps2);
// 	}
// #endif // READ_PER_EUMJOL

	if ( bSegmRead && word_len > 2 ) {
		reading_per_eumjol(wordcode, tmps);
		sprintf(tmps2, "%s", tmps);
		if ((n=strlen(tmps2)) > MAX_LEN_LEXICON) {
			tmps2[MAX_LEN_LEXICON-1] = '\0'; // 030109
			pWordLex->nLenMonoLex[nCount] = MAX_LEN_LEXICON - 1;
		} else {
			pWordLex->nLenMonoLex[nCount] = (hci_uint16)n;
		}
		strcpy(pWordLex->szMonoLex[nCount++], tmps2);
	}

	pWordLex->nNumMonoLex = nCount;

	return TRUE;
}

///////////////////////// GetCode //////////////////////////////

// chk:1105:etc
HCILAB_PRIVATE int
GetAlphaIndex(const char c)
{
	char *pStr = 0;

	pStr = strchr(g_alpha,c);
	if (!pStr) {
		return -1;
	}

	return (int)(strlen(pStr)-1);
}

// chk:1105:etc
HCILAB_PRIVATE int
GetDigitIndex(const char c)
{
	char *pStr = 0;

	pStr = strchr(g_digit,c);
	if (!pStr) {
		return -1;
	}

	return (int)(strlen(pStr)-1);
}

// chk:1105:etc
HCILAB_PRIVATE hci_uint32
Eng2Kor(const char c, char *des)
{
	int i = GetAlphaIndex((char)tolower(c));

	if (i < 0) {
		return 0U;
	}
	else {
		strcpy(des, PRON_E2K[i]);
		return strlen(des);
	}
}

// chk:1105:etc
HCILAB_PRIVATE hci_uint32
Digit2Kor(const char c, char *des)
{
	int i = GetDigitIndex(c);

	if (i < 0) {
		return 0U;
	}
	else {
		strcpy(des, PRON_D2K[i]);
		return strlen(des);
	}
}

HCILAB_PRIVATE hci_uint32
Digit2Eng(const char c, char *des)
{
	int i = GetDigitIndex(c);
	
	if (i < 0) {
		return 0U;
	}
	else {
		strcpy(des, PRON_D2E[i]);
		return strlen(des);
	}
}


HCILAB_PRIVATE int
Convert2Kor(const char *src,
			char *des)
{
	hci_uint32 i = 0, j = 0, k = 0;
	hci_uint32 nLenVocabStr = strlen(src);
	hci_uint32 maxLenOutStr = 64;
	const char *pSrcStr = 0;
	char *pOutStr = 0;

	if (0 == src) {
		return 0;
	}

	pSrcStr = src;
	pOutStr = des;

	for (i=0, j=0; i < nLenVocabStr && j < maxLenOutStr;) {
		if ((*pSrcStr) & 0x80) {	// 한글 (2byte)
			*pOutStr++ = *pSrcStr++;
			*pOutStr++ = *pSrcStr++;
			i += 2; j += 2;
		} else if (isalpha(*pSrcStr)) {	// 영어
			k = Eng2Kor(*pSrcStr, pOutStr);
			pSrcStr++; i++;
			if (k) {
				pOutStr += k;
				j += k;
			}
		} else if (isdigit(src[i])) {	// 숫자
			k = Digit2Kor(*pSrcStr, pOutStr);
			pSrcStr++; i++;
			if (k) {
				pOutStr += k;
				j += k;
			}
		} else if (strncmp(pSrcStr,"ㆍ",2) == 0) {
			pSrcStr += 2; i += 2;
		} else if ((*pSrcStr)=='_') {
			pSrcStr++;
			i++;
		} else { // 그외 상황 => 포기
			pSrcStr++;
			i++;
			//return 0;
		}
	}
	*pOutStr = '\0';

	if ( j < maxLenOutStr ) return 1;
	else return 0;
}

static char GetUmso(char *s)
{
	char *p;
	char Jaso[]="ㄱAㄲBㄳCㄴDㄵEㄶFㄷGㄸHㄹIㄺJㄻKㄼLㄽMㄾNㄿOㅀPㅁQㅂRㅃSㅄTㅅUㅆVㅇWㅈXㅉYㅊZㅋ[ㅌ\\ㅍ]ㅎ^";
	if(s==NULL || *s==0) return 0;
	p = strstr(Jaso,s);

	return p ? *(p+2) : 0;
}

HCILAB_PRIVATE int
GetCode(const char *s1,
		char *res)
{

	int len = 0;
	char *p, tmp[3], t1, t2[1024], s[1024];

	extern int c2_to_n(char *c2, char *n);

	if (Convert2Kor(s1, s) == 0)
		return -1;

	len = (int)strlen(s);

	if((*s & 0x80) && (len >= 2)) {
		tmp[0] = s[0];
		tmp[1] = s[1];
		tmp[2] = '\0';
		if(t1 = GetUmso(tmp)){
			p = s + 2;
			if (c2_to_n(p,t2) == 0)
				return -1;
			res[0] = t1;
			res[1] = '\0';
			strcat(res, t2);
		}
		else {
			if (c2_to_n(s,res) == 0)
				return -1;
		}
	}

	return len;
}

/////////////////////// write_pause ////////////////////////////

static int is_stop(char p1, char p2)
{
	if( strchr(stop_jong,p1) && strchr(stop_cho, p2))
		return 1;
	else if(strchr(stop_cho,p2))
		return 2;
	else if(strchr(stop_jong,p2))
		return 3;
	else
		return 0;
}

void write_pause(char *psym, char *psym_sp)
{
	int i,j;
	char p;

	psym_sp[0] = psym[0];
	j = 1;
	if (strchr(stop_jong, psym[0]))
		psym_sp[j++] = '#';

	for( i=1; i<(int)strlen(psym); i++) {
		if (psym[i] == ':' || psym[i] == '*' || psym[i] == '~') continue;
		p = psym[i-1];

		switch (is_stop(p, psym[i])){
			
			case 1: psym_sp[j++] = psym[i]; // 이미 앞에서 했기 때문에 붙이지 않는다. (stop_jong + stop_cho)
					break;

			case 2: if (p != '#' && p != '$') // (+stop_cho)
						psym_sp[j++] = '#';
					psym_sp[j++] = psym[i];
					break;
	
			case 3: psym_sp[j++] = psym[i]; // (+stop_jong)
					if (i < ((int)strlen(psym)-1) && psym[i+1] != '#' && psym[i+1] != '$')
						psym_sp[j++] = '#';		
					break;
			case 0: psym_sp[j++] = psym[i]; // else
					break;

		}
	}
	psym_sp[j]='\0';
}


// from MD_KSCode.c
//	kscode.c
//	prototype
void	s2_to_n(unsigned short s2, char *n);
unsigned short	c2_to_s2(unsigned short c2);


int c2_to_n(char *c2, char *n)
{
	unsigned char	b[2];
	unsigned short	c;

	unsigned short	a;
	char		n1[4];
	int		i, ci=0, ni=0;

  	while (c2[ci] != '\0') 

	{
		b[0] = c2[ci++]; 
		b[1] = c2[ci++];
		c = (b[0] << 8) | b[1];
		if((a=c2_to_s2(c))==0)
			return 0;

		s2_to_n(a,n1);
		for (i=0; n1[i] != '\0'; i++)
			n[ni++] = n1[i];
	}
	n[ni] = '\0';



	return 1;

}

/*
 * 완성 -> 조합
 */
unsigned short c2_to_s2(unsigned short c2)
{
	register int		l, u, m;
	register unsigned short	tmp;

	l = 0; u = NumOfC2H - 1;
	while (l <= u) 

	{
		m = (l+u)/2;
	   	tmp = S2C2HC[m][1];
    	if ( c2 > tmp )
			l = m + 1;
    	else if ( c2 == tmp )
			return(S2C2HC[m][0]);
		else if ( c2 < tmp )
			u = m - 1;
	}
	l = 0; u = NumOfC2C - 1;
	while ( l <= u ) 

	{
		m = (l+u)/2;
	 	tmp = S2C2CC[m][1];
    	if (c2 > tmp)
			l = m + 1;
    	else if (c2 == tmp)

		{			/*음절을 이루지 못하는 문자*/
			return(S2C2CC[m][0]);

			//return 0;

		}
		else if (c2 < tmp)
			u = m - 1;
	}

	return 0;
}

void s2_to_n(unsigned short s2, char *n)
{
//sunhci 20061013 11172
/*
	int	ci, vi, fi;
	ci = ((unsigned short)(s2 & 0x7C00) >> 10);
	vi = ((unsigned short)(s2 & 0x03E0) >>  5);
	fi =  (s2 & 0x001F);
	if (S2NC[ci]) *n++ = S2NC[ci];
	if (S2NV[vi]) *n++ = S2NV[vi];
	if (S2NF[fi]) *n++ = S2NF[fi];
	*n = '\0';
*/
	int	ci, vi, fi;
	register int m = 0x00;

	if ((s2 >= 0xDAA1) && (s2 <=0xDAD3)) {  /* 낱자 */
		while(s2 != S2C2CC[m][0]) m++;
		if (s2 < 0xDABF) {  /* 자음 */
			ci = S2C2CC[m][2]; vi=0x00; fi=0x00;
		} else {			/* 모음 */
			vi = S2C2CC[m][2]; ci=0x00; fi=0x00;
		}
	} else {
		ci = ((unsigned short)(s2 & 0x7C00) >> 10); /* 0111 1100 0000 0000 */
		vi = ((unsigned short)(s2 & 0x03E0) >>  5);	/* 0000 0011 1110 0000 */
		fi =  (s2 & 0x001F);						/* 0000 0000 0001 1111 */
	}
	if (S2NC[ci]) *n++ = S2NC[ci];
	if (S2NV[vi]) *n++ = S2NV[vi];
	if (S2NF[fi]) *n++ = S2NF[fi];
	*n = '\0';
}

//////////////////////// reading_by_rule /////////////////////////////////
static void JAEUM_rule(char *psym, char ch, char ch1, int exception)
{
	int	gb_remove_n = 0;

	switch (ch) {
	case 'A':  /* ㄱ   */
	case 'B':  /* ㄲ   */
	case 'C':  /* ㄱㅅ */
		switch (ch1) {
		case 'A': case 'G': case 'R': case 'U': case 'X': // 경음화
			strcat(psym,NPSC[ch1-0x3f]);
			break;
		case 'D': case 'I':
			psym[strlen(psym)-1] = 'O';
			strcat(psym,"n");
			break;
		case 'Q':
			psym[strlen(psym)-1] = 'O';
			strcat(psym,"m");
			break;
		case '^':	/* 격음화 */
			switch (exception) {
			case EX_DOUBLE_C:
				strcat(psym, "k");
				break;
			default:
				psym[strlen(psym)-1] = 'k';
				break;
			}
			break;
		case 'W':
			switch (exception) {
			case EX_N_ADD:
				psym[strlen(psym)-1] = 'O';
				if (!gb_remove_n)
					strcat(psym,"n");
				break;
			default:
				psym[strlen(psym)-1] = EOS;
				strcat(psym,NPSC[ch-0x40]);
				break;
			}
			break;
		default :
			strcat(psym,NPSC[ch1-0x40]);
			break;
		}
		break;
	case 'D':  /* ㄴ */
		switch (ch1) {
		case 'A': case 'G': case 'R': case 'U': case 'X':
			if ((strlen(psym) == 1) && (psym[0] == 'n')) {
				psym[0] = 'N';
			}
			switch (exception) {
			case 1:
				strcat(psym,NPSC[ch1-0x3f]);
				break;
			default:
				strcat(psym,NPSC[ch1-0x40]);
				break;
			}
			break;
		case 'I':
			switch (exception) {
			case EX_N_ADD:
				if (!gb_remove_n)
					strcat(psym,"n");
				break;
			default:
				psym[strlen(psym)-1] = 'L';
				strcat(psym,"r");
				break;
			}
			break;
		/*
		case '^':  ** ㅎ 탈락 **
			psym[strlen(psym)-1] = 'n';
			break;
		*/
		case 'W':
			switch (exception) {
			case EX_N_ADD:
				if (!gb_remove_n)
					strcat(psym,"n");
				break;
			default:
				psym[strlen(psym)-1] = 'n';
				break;
			}
			break;
		default :
			strcat(psym,NPSC[ch1-0x40]);
			break;
		}	
		break;
	case 'E':  /* ㄴㅈ */
		switch (ch1) {
		case 'A': case 'G': case 'U': case 'X':	/* 어미의 경음화 */
			strcat(psym,NPSC[ch1-0x3f]);
			break;
		case '^':	/* 격음화 */	
			strcat(psym,"c");
			break;
		case 'W':
			strcat(psym,"j");
			break;
		default :
			strcat(psym,NPSC[ch1-0x40]);
			break;
	  	}	
		break;
	case 'F':  /* ㄴㅎ */
		switch (ch1) {
		case 'A':
			strcat(psym,"k");
			break;
		case 'G': /* 어미의 격음화 */
			strcat(psym,"t");
			break;
		case 'I':
			psym[strlen(psym)-1] = 'L';
			strcat(psym,"r");
			break;
		case 'W':
			psym[strlen(psym)-1] = 'n';
			break;
		case 'X': /* 어미의 격음화 */
			strcat(psym,"c");
			break;
		default :
			strcat(psym,NPSC[ch1-0x40]);
			break;
		}	
		break;
	case 'G' :  /* ㄷ */
		switch (ch1) {
		case 'A': case 'G': case 'R': case 'U': case 'X': // 경음화
			strcat(psym,NPSC[ch1-0x3f]);
			break;
		case 'D':
			psym[strlen(psym)-1] = 'N';
			strcat(psym,"n");
			break;
		case 'Q':
			psym[strlen(psym)-1] = 'N';
			strcat(psym,"m");
			break;
		case '^': 
			if (exception)
				psym[strlen(psym)-1] = 't';
			else psym[strlen(psym)-1] = 'c';
			break;
		case 'W':
			switch (exception) {
			
			case EX_N_ADD:	/* 꽃잎 ??? ㅊ이 아닌데*/
				psym[strlen(psym)-1] = 'N';
				if (!gb_remove_n)
					strcat(psym,"n");
				break;
			case 3:	/* 풍년걷이 */
				psym[strlen(psym)-1] = 'j';
				break;

			default:
				psym[strlen(psym)-1] = EOS;
				strcat(psym,NPSC[ch-0x40]);
				break;
			}
			break;
		default :
			strcat(psym,NPSC[ch1-0x40]);
			break;
		}
		break;
	case 'H' :  /* ㄸ */
	case 'U' :  /* ㅅ */
	case 'V' :  /* ㅆ */
	case 'Y' :  /* ㅉ */
	case 'Z' :  /* ㅊ */
		switch (ch1) {
		case 'A': case 'G': case 'R': case 'U': case 'X':
			strcat(psym,NPSC[ch1-0x3f]);
			break;
		case 'D':
			psym[strlen(psym)-1] = 'N';
			strcat(psym,"n");
			break;
		case 'Q':
			psym[strlen(psym)-1] = 'N';
			strcat(psym,"m");
			break;
		case '^': 
			if (exception)
				psym[strlen(psym)-1] = 'c';
			else psym[strlen(psym)-1] = 't';
			break;
		case 'W':
			switch (exception) {
	
			case EX_N_ADD:	/* 꽃잎 */
				psym[strlen(psym)-1] = 'N';
				if (!gb_remove_n)
					strcat(psym,"n");
				break;
			case 4:
				psym[strlen(psym)-1] = 'd';
				break;
			default:
				psym[strlen(psym)-1] = EOS;
				strcat(psym,NPSC[ch-0x40]);
				break;
			}
			break;
		default :
			strcat(psym,NPSC[ch1-0x40]);
			break;
		}
		break;
	case '\\':  /* ㅌ */
		switch (ch1) {
		case 'A': case 'G': case 'R': case 'U': case 'X':
			strcat(psym,NPSC[ch1-0x3f]);
			break;
		case 'D':
			psym[strlen(psym)-1] = 'N';
			strcat(psym,"n");
			break;
		case 'Q':
			psym[strlen(psym)-1] = 'N';
			strcat(psym,"m");
			break;
		case '^': 
			if (exception)
				psym[strlen(psym)-1] = 'c';
			else psym[strlen(psym)-1] = 't';
			break;
		case 'W':
			switch (exception) {
	
			case EX_N_ADD:
				psym[strlen(psym)-1] = 'N';
				if (!gb_remove_n)
					strcat(psym,"n");
				break;
			case 3:
				psym[strlen(psym)-1] = 'c';
				break;

			default:
	
				psym[strlen(psym)-1] = EOS;
				strcat(psym,NPSC[ch-0x40]);
				break;
			}
			break;
		default :
			strcat(psym,NPSC[ch1-0x40]);
			break;
		}
		break;
	case 'X' :  /* ㅈ */
		switch (ch1) {
		case 'A': case 'G': case 'R': case 'U': case 'X':
			strcat(psym,NPSC[ch1-0x3f]);
			break;
		case 'D':
			psym[strlen(psym)-1] = 'N';
			strcat(psym,"n");
			break;
		case 'Q':
			psym[strlen(psym)-1] = 'N';
			strcat(psym,"m");
			break;
		case '^':
			if (exception)
				psym[strlen(psym)-1] = 't';
			else psym[strlen(psym)-1] = 'c';
			break;
		case 'W':
			switch (exception) {

			case EX_N_ADD:
				psym[strlen(psym)-1] = 'N';
				if (!gb_remove_n)
					strcat(psym,"n");
				break;
			default:
				psym[strlen(psym)-1] = EOS;
				strcat(psym,NPSC[ch-0x40]);
				break;
			}
			break;
		default :
			strcat(psym,NPSC[ch1-0x40]);
			break;
		}
		break;
	case 'I':  /* ㄹ 경음화 by shcha  */
		switch(ch1){
		case 'A': case 'G': case 'R': case 'X': case 'U':
			switch (exception) {
			case EX_STRONG:
				strcat(psym, NPSC[ch1-0x3f]);
				break;
			default:
				strcat(psym, NPSC[ch1-0x40]);
				break;
			}
			break;
		case 'W':

			switch (exception) {
			case EX_N_ADD:
				if (!gb_remove_n)
					strcat(psym, "r");
				break;
			default:
				psym[strlen(psym)-1] = EOS;
				strcat(psym,NPSC[ch-0x40]);
				break;
			}
			break;
		default :
			strcat(psym,NPSC[ch1-0x40]);
				break;
		}
		break;
	case 'M':  /* ㄹㅅ */
		switch (ch1) {
		case 'A': case 'G': case 'R': case 'U': case 'X':
			if ((strlen(psym) == 1) && (psym[0] == 'r')) {
				psym[0] = 'L';
				strcat(psym,NPSC[ch1-0x3f]);
			} else 
			strcat(psym,NPSC[ch1-0x3f]); 
			break;

		case 'W':
		
			psym[strlen(psym)-1] = EOS; // 돐에 -> 도레
			strcat(psym,"r");
			break;
		default :
			strcat(psym,NPSC[ch1-0x40]);
			break;
		}
		break;
	case 'N':  /* ㄹㅌ */
		switch (ch1) {
		case 'A': case 'G': case 'R': case 'U': case 'X':
			if ((strlen(psym) == 1) && (psym[0] == 'r')) {
				psym[0] = 'L';
				strcat(psym,NPSC[ch1-0x3f]);
			} else 
			strcat(psym,NPSC[ch1-0x3f]); 
			break;
	
		case 'D':
			if ((strlen(psym) == 1) && (psym[0] == 'r'))
				psym[0] = 'L';
			strcat(psym,"r");
			break;
	
		case 'W':

			switch (exception) {
			case 3:
				strcat(psym,"c");
				break;
			default:
				psym[strlen(psym)-1] = EOS;
				strcat(psym,NPSC[ch-0x40]);
				break;
			}
			break;
		default :
			strcat(psym,NPSC[ch1-0x40]);
			break;
		}
		break;
	case 'L':  /* ㄹㅂ */
		switch (ch1) {
		case 'A': case 'G': case 'R': case 'X': case 'U':
			strcat(psym,NPSC[ch1-0x3f]);
			break;
		case 'D':
			switch (exception) {
			case 5:
				psym[strlen(psym)-1] = 'M';
				strcat(psym,"n");
				break;
			default:
				strcat(psym,"r");
				break;
			}
			break;
		case '^':
			psym[strlen(psym)-1] = 'L';
			strcat(psym,"p");
			break;
		case 'W':
			psym[strlen(psym)-1] = EOS;
			strcat(psym,NPSC[ch-0x40]);
			break;
		default :
			strcat(psym,NPSC[ch1-0x40]);
			break;
		}
		break;
	case 'P':  /* ㄹㅎ */
		switch (ch1) {
		case 'A':  /* 어미의 격음화 */
			strcat(psym,"k");
			break;
		case 'U':
			strcat(psym,NPSC[ch1-0x3f]);
			break;
		case 'D':
			strcat(psym,"r");
			break;
		case 'G':  /* 어미의 격음화 */
			strcat(psym,"t");
			break;
		case 'X':  /* 어미의 격음화 */
			strcat(psym,"c");
			break;
		case 'W':
			psym[strlen(psym)-1] = 'r';
			break;
		default :
			strcat(psym,NPSC[ch1-0x40]);
			break;
		}
		break;
	case 'J':  /* ㄹㄱ */
		switch (ch1) {
		case 'A': case 'G': case 'R': case 'U': case 'X': 
			switch (exception) {
			case 4:
				psym[strlen(psym)-1] = 'L';
				strcat(psym,NPSC[ch1-0x3f]);
				break;
			default:
				strcat(psym,NPSC[ch1-0x3f]);
				break;
			}
			break;
	
		case 'D': case 'I':
			switch (exception) { 
			case 4:
				psym[strlen(psym)-1] = 'L';
				strcat(psym,"r");
				break;
			default:
				psym[strlen(psym)-1] = 'O';
				strcat(psym,"n");
				break;
			}
			break;
		case 'Q':
			psym[strlen(psym)-1] = 'O';
			strcat(psym,"m");
			break;
		case '^':
		
			switch (exception) {
			case 4:
				psym[strlen(psym)-1] = 'K';
				strcat(psym, "k");
				break;
			default:
				psym[strlen(psym)-1] = 'L';
				strcat(psym,"k");
				break;
			}
			break;
		case 'W':
			
			switch (exception) {
			case 4:
				psym[strlen(psym)-1] = 'g';
				break;
			default:
				psym[strlen(psym)-1] = EOS;
				strcat(psym,NPSC[ch-0x40]);
				break;
			}
			break;
		default :
			strcat(psym,NPSC[ch1-0x40]);
			break;
		}
		break;
	case 'K':  /* ㄹㅁ */
	case 'Q':  /* ㅁ   */
		switch (ch1) {
		case 'A': case 'G': case 'R': case 'U': case 'X':
			if (exception)
				strcat(psym,NPSC[ch1-0x3f]);
			else strcat(psym,NPSC[ch1-0x40]);
			break;
		case 'I':
			strcat(psym,"n");
			break;
	
		case 'W':
			switch (exception) {
			case EX_N_ADD:
				if (!gb_remove_n)
					strcat(psym,"n");
				break;
			default:
				psym[strlen(psym)-1] = EOS;
				strcat(psym,NPSC[ch-0x40]);
				break;
			}
			break;
		default :
			strcat(psym,NPSC[ch1-0x40]);
			break;
		}
		break;
	case 'O':  /* ㄹㅍ */
	case 'R':  /* ㅂ   */ 
	case 'S':  /* ㅃ   */
	case 'T':  /* ㅂㅅ */
	case ']':  /* ㅍ   */
		switch (ch1) {
		case 'A': case 'G': case 'R': case 'U': case 'X':
			if ((strlen(psym) == 1) && (psym[0] == 'b')) {
				psym[0] = 'P';
			}
			strcat(psym,NPSC[ch1-0x3f]);
			break;
		case 'D': case 'I':
			/*
			printf("-->\n");
			*/
			psym[strlen(psym)-1] = 'M';
			strcat(psym,"n");
			break;
		case 'Q':
			psym[strlen(psym)-1] = 'M';
			strcat(psym,"m");
			break;
		case '^':	/* 격음화 */
			psym[strlen(psym)-1] = 'p';
			break;
		case 'W':
			switch (exception) {
			case EX_N_ADD:
				psym[strlen(psym)-1] = 'M';
				if (!gb_remove_n)
					strcat(psym,"n");
				break;
			case 4:
				psym[strlen(psym)-1] = 'b';
				break;
			default:
				psym[strlen(psym)-1] = EOS;
				strcat(psym,NPSC[ch-0x40]);
				break;
			}
		default :
			strcat(psym,NPSC[ch1-0x40]);
			break;
		}
		break;
	case 'W':  /* ㅇ */
		switch (ch1) {
		case 'A': case 'G': case 'R': case 'U': case 'X':
			if (exception)
				strcat(psym,NPSC[ch1-0x3f]);
			else strcat(psym,NPSC[ch1-0x40]);
			break;
		case 'I':
			strcat(psym,"n");
			break;
		case 'W':
			switch (exception) {
			case EX_N_ADD:
				if (!gb_remove_n)
					strcat(psym,"n");
				break;
			default:
				strcat(psym,NPSC[ch1-0x40]);
				break;
			}
			break;
		default :
			strcat(psym,NPSC[ch1-0x40]);
			break;
		}
		break;
	case '[':  /* ㅋ */
		switch (ch1) {
		case 'A': case 'G': case 'R': case 'U': case 'X':
			strcat(psym,NPSC[ch1-0x3f]);
			break;
		case 'D': case 'I':
			psym[strlen(psym)-1] = 'O';
			strcat(psym,"n");
			break;
		case 'Q':
			psym[strlen(psym)-1] = 'O';
			strcat(psym,"m");
			break;
		case '^':	/* 격음화 */
			psym[strlen(psym)-1] = 'k';
			break;
		case 'W':
			switch (exception) {
			case EX_N_ADD:
				if (!gb_remove_n)
					strcat(psym,"n");
				break;
			default:
				psym[strlen(psym)-1] = 'g';
				break;
			}
			break;
		default :
			strcat(psym,NPSC[ch1-0x40]);
			break;
		}
		break;
	case '^':  /* ㅎ */
		switch (ch1) {
		case 'A':
			switch (exception) {
			case EX_DOUBLE_C:
				psym[strlen(psym)-1] = 'K';
				strcat(psym, "k");
				break;
			default:
				psym[strlen(psym)-1] = 'k';
				break;
			}
			break;
		case 'D':
			psym[strlen(psym)-1] = 'N';
			strcat(psym,"n");
			break;
		case 'G': case '^':
			switch (exception) {
			case EX_DOUBLE_C:
				psym[strlen(psym)-1] = 'T';
				strcat(psym, "t");
				break;
			default:
				psym[strlen(psym)-1] = 't';
				break;
			}
			break;
		case 'Q':
			psym[strlen(psym)-1] = 'N';
			strcat(psym,"m");
			break;
		case 'R': case 'U':
			strcat(psym,NPSC[ch1-0x3f]);
			break;
		case 'X':
			switch (exception) {
			case EX_DOUBLE_C:
				psym[strlen(psym)-1] = 'T';
				strcat(psym, "c");
				break;
			default:
				psym[strlen(psym)-1] = 'c';
				break;
			}
			break;
		case 'W':
			psym[strlen(psym)-1] = EOS;
			break;
		default :
			strcat(psym,NPSC[ch1-0x40]);
			break;
		}
		break;
	default :
		switch (exception) {
		case 1:
			strcat(psym,NPSC[ch1-0x3f]);
			break;
		default:
			strcat(psym,NPSC[ch1-0x40]);
			break;
		}
		break;
	}
}



static void MOEUM_rule(char *psym, char ch1, char ch2, int *lvs)
{
	int	i;
	char	lastp; //, lv;

	//ch1 = ch;
	if (ch1 == '{' && ch2 != 'W') {  /* 의: 앞에 'ㅇ'이 없이 'ㅢ'만 있는 경우 */ 
		if (psym[0] != EOS) {
			lastp = psym[strlen(psym)-1];
			for (i=0; i<32; i++) {
				if (strlen(NPSC[i]) == 1)
					if (lastp == NPSC[i][0]) {
						ch1 = '|';  /* 이 */
						break;
					}
			}
		}
	} 

	// '이' 선행 모음 처리 ( '여' 이후는 추가된것)
	// 1. single: 여,야,요,유 (ㅈ,ㅊ,ㅉ)

	else if (ch1 == 'j') {	// 여 -> 어
		if (psym[0] != EOS) {
			lastp = psym[strlen(psym)-1];
			if ((lastp == 'j') || (lastp == 'c') || (lastp == 'z')) // ㅈ, ㅊ, ㅉ
				ch1 = 'f';
		}
	} else if (ch1 == 'd') { // 야 -> 아
		if (psym[0] != EOS) {
			lastp = psym[strlen(psym)-1];
			if ((lastp == 'j') || (lastp == 'c') || (lastp == 'z'))
				ch1 = 'b';
		}
	} else if (ch1 == 'r') { // 요 -> 오 
		if (psym[0] != EOS) {
			lastp = psym[strlen(psym)-1];
			if ((lastp == 'j') || (lastp == 'c') || (lastp == 'z'))
				ch1 = 'l';
		}
	} else if (ch1 == 'w') { // 유 -> 우
		if (psym[0] != EOS) {
			lastp = psym[strlen(psym)-1];
			if ((lastp == 'j') || (lastp == 'c') || (lastp == 'z'))
				ch1 = 's';
		}
	} 

	// 2. double: 얘,예 (ㄴ,ㅅ,ㅆ 만 제외)
	else if (ch1 == 'e') { // 얘 -> 애
		if (psym[0] != EOS) {
			lastp = psym[strlen(psym)-1];
			if ((lastp != 'n') && (lastp != 's') && (lastp != 'x'))
				ch1 = 'c';
		}

	} else if (ch1 == 'k') { // 예 -> 에
		if (psym[0] != EOS) {
			lastp = psym[strlen(psym)-1];
			if ((lastp != 'n') && (lastp != 's') && (lastp != 'x'))
				ch1 = 'g';
		}
	} 

	strcat(psym,NPSV[ch1-0x60]);
	
	//*lvs = *lvs << 1;
	//lv = *lvs & 0x80;
	//if (lv)
	//	strcat(psym,":");
}

int i_pre_vowel(char v)
{
	int	i;

	if (v == '\0')
		return 0;

	for (i=0; IPV[i]!='\0'; i++)
		if (v == IPV[i])
			return 1;

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////

int reading_by_rule(char *word, int *ex, int *cpos, int num_ex, char *psym)
{
	int		i = 0, cur = 0, next = 0;
	//unsigned char	lvs;
	//int		cpos1;
	//int		last;
	int     ex_id = 0;
	int		tmpd = 0; // 기존것을 유지하기 위해 (long vowel)

	//lvs = longv;
	//cpos1 = (int)(cpos >> 4);
	//cpos2 = (int)(cpos & 0x0F);
	//cpos1 = cpos;
  	while (word[i] != EOS) 
	{
		if (word[i]-0x40 < 0) return 0;

		if ( cur < 0 ) cur = 0;

		next = STT[cur][CCT[word[i]-0x40]];
		if (next < 0 ) next = 0;

		switch (AT[cur][next]) 
		{
		case 0 : /* 초성 */
			if ((int)strlen(word) == 1 || strchr(Chosung_Code_O, word[i+1])) {
				strcpy(psym,NPSF[word[i]-0x40]);
			}
			else {
				strcpy(psym,NPSC[word[i]-0x40]);
			}
			break;
		case 1 : /* 중성 */
			MOEUM_rule(psym,word[i],word[i-1],&tmpd); // word[i-1]을 파라메타로 줘서 모음룰에서 '의'를 체크할 수 있도록함
			break;
		case 2 : /* 종성 */
		
			strcat(psym,NPSF[word[i]-0x40]);
			break;
		case 3 : /* 한 글자 완성, 초성 */
			if (num_ex && i == cpos[ex_id]) // 이게 아마 예외규정이 들어가는 곳이고 꾸며주는 일이 있는 곳 같군!!
			{
			
				JAEUM_rule(psym,word[i-1],word[i], ex[ex_id]);
				ex_id++;
			} else {
				// 추가: 구개음화
				if (i_pre_vowel(word[i+1]) && (word[i] == 'W') && (word[i-1] == 'G' || word[i-1] == '\\' || word[i-1] == 'N'))
					JAEUM_rule(psym,word[i-1],word[i],3);
				else
					JAEUM_rule(psym,word[i-1],word[i],0);
			}
			break;
		case 4 : /* 종성 -> 초성, 한 글자 완성, 초성, 중성 */
			psym[strlen(psym)-1] = EOS;
		
			JAEUM_rule(psym,word[i-2],word[i-1],0);
			MOEUM_rule(psym,word[i],word[i-1],&tmpd);
			break;
		}
		cur = next;
		i += 1;
	}

	return 1;
}

////////////////// IsOptRule ///////////////////////
int IsOptRule(char *wordcode)
{
	// 와->아, 워->어, 위->이 ///, 예->이에 ('예'는 제외)
	int i, len = strlen(wordcode);
	//char str[] = "mtvk"; // 와,워,위,예
	char str[] = "mtv"; // 와,워,위

	for (i=0; i<len; i++) {
		if (strchr(str, wordcode[i]))
			return 1;
	}
	
	return 0;
}

////////////////// IsOmitH ///////////////////////
int IsOmitH(char *wordcode)
{
	// ㄴ,ㄹ,ㅁ,ㅇ,모음 + ㅎ -> ㅎ탈락 경우 가능
	int i, len = strlen(wordcode);
	char str[] = "DIQWbcdefgjklmnorstuvwz|{"; // ㄴ,ㄹ,ㅁ,ㅇ,모음

	for (i=1; i<len; i++) {
		if (wordcode[i] == '^') {
			if (strchr(str, wordcode[i-1]))
				return 1;
		}
	}
	return 0;
}

////////////////// IsDAUM ///////////////////////
int IsDAUM(char *wordcode)
{
	if (strstr(wordcode, "GbWzQ"))
		return 1;
	return 0;
}

////////////////// CheckException ////////////////////////

int CheckException(char *wordcode, int *ex, int *ex_pos)
{
	int	i, n;
	char *IVowel = "djrwek"; // '이'를 제외한 '이'모음 계열
	int	cur=0, next;
	int num_ex = 0;
	
	// 2음절 이후에서 ㄱ,ㄴ,ㅁ,ㄹ,ㅂ,ㅇ + '이'를 제외한 '이'모음 계열이 오면 음소 삽입 예외 적용 (3음절이후 -> 2음절이후)
	// 예) 김유경

	i = n = 0;
	//while (n < 2 && wordcode[i] != EOS) { // 2글자를 우선 완성시킴
	while (n < 1 && wordcode[i] != EOS) { // 1글자를 우선 완성시킴
		if (wordcode[i]-0x40 < 0) return 0;

		if ( cur < 0 ) cur = 0;
		next = STT[cur][CCT[wordcode[i]-0x40]];
		if ( next < 0 ) next = 0;

		switch (AT[cur][next]) {
		case 3 : /* 한 글자 완성, 초성 */
		case 4 : /* 종성 -> 초성, 한 글자 완성, 초성, 중성 */
			n++;
			break;
		default:
			break;
		}
		cur = next;
		i++;
	}

	if (wordcode[i] == EOS) return 0;

	i -= 2; // 두개 뒤로 보냄
	
	while (wordcode[i] != EOS) {
		// ckh:1104:reduce time
		//if (wordcode[i] == 'A' || wordcode[i] == 'D' || wordcode[i] == 'Q' || wordcode[i] == 'I' || wordcode[i] == 'W' || wordcode[i] == 'R') { // 선행음이 ㄱ,ㄴ,ㄹ,ㅂ,ㅇ 이고
//		if (strchr("ADQIWR",wordcode[i])!=NULL){
		if (strchr("ADQIWRGZURX\\",wordcode[i])!=NULL){
			i++;
			if (wordcode[i] == EOS) return num_ex;
			if (wordcode[i++] == 'W') { // 현재음이 'ㅇ'이고
				if (wordcode[i] == EOS) return num_ex;
				if (strchr(IVowel, wordcode[i++])) { // '이'모음 계열이 뒤따르면
					//return 'i'; // 음소 삽입 예외 발생
					//*ex = EX_N_ADD; // 'ㄴ 첨가 예외 코드'
					//return i-2;
					ex[num_ex] = EX_N_ADD;
					ex_pos[num_ex] = i-2;
					num_ex++;
				}
			}
		} else
			i++;
	}

	return num_ex;
}

////////////////// CheckException4 ////////////////////////
int CheckException4(char *wordcode, int *ex, int *ex_pos)
{
	int	i, n;
	int mode=0;
	int	cur=0, next;
	char	*dsz = "GUX";
	int num_ex = 0;

	i = n = 0;
//	while (n < start && wordcode[i] != EOS) { // start까지 글자를 우선 완성시킴
	while ( wordcode[i] != EOS ) {
		if (wordcode[i]-0x40 < 0) return 0;

		next = STT[cur][CCT[wordcode[i]-0x40]];
		switch (AT[cur][next]) {
		case 3 : /* 한 글자 완성, 초성 */
			n++;
			mode = 1;
			if ( wordcode[i-1] == 'I' && strchr(dsz, wordcode[i])) {
				ex[num_ex] = EX_STRONG;
				ex_pos[num_ex] = i;
				num_ex++;
			}
			break;
		case 4 : /* 종성 -> 초성, 한 글자 완성, 초성, 중성 */
			n++;
			mode = 2;
			if ( wordcode[i-2] == 'I' && strchr(dsz, wordcode[i-1]) ) {
				ex[num_ex] = EX_STRONG;
				ex_pos[num_ex] = i-1;
				num_ex++;
			}
			break;
		default:
			break;
		}
		cur = next;
		i++;
	}

	return num_ex;
}

//////////// reading_per_eumjol ////////////////

int reading_per_eumjol(char *wordcode, char *psym)
{
	int i, cur, next, tmpd;

	i = 0;
	cur = 0;
	psym[0] = '\0';

	while (wordcode[i] != EOS) {
		if (wordcode[i]-0x40 < 0) return 0;

		next = STT[cur][CCT[wordcode[i]-0x40]];
		switch (AT[cur][next]) {
		case 0 : /* 초성 */
			strcpy(psym,NPSC[wordcode[i]-0x40]);
			break;
		case 1 : /* 중성 */
			MOEUM_rule(psym,wordcode[i],wordcode[i-1],&tmpd); // word[i-1]을 파라메타로 줘서 모음룰에서 '의'를 체크할 수 있도록함
			break;
		case 2 : /* 종성 */
			strcat(psym,NPSF[wordcode[i]-0x40]);
			break;
		case 3 : /* 한 글자 완성, 초성 */
			strcat(psym, "#");
			strcat(psym, NPSC[wordcode[i]-0x40]);
			break;
		case 4 : /* 종성 -> 초성, 한 글자 완성, 초성, 중성 */
			psym[strlen(psym)-1] = EOS;
			strcat(psym, "#");
			strcat(psym, NPSC[wordcode[i-1]-0x40]);
			MOEUM_rule(psym,wordcode[i],wordcode[i-1],&tmpd);
			break;
		}
		cur = next;
		i++;
	}

	return 1;
}

/////////////// skip initial stop eumjol ////////////////////
/*
BOOL skip_initial_stop_eumjol(char *in, char *out)
{
	int pos, i, n;
	char *pron_vowel ="a@e^o%u+_=iAEZYIUQFWV";

	// 첫 음절만 따지므로 반드시 2,3,4에 sp가 있어야 한다.
	if (strlen(in) < 4) return FALSE;
	if (in[1] == '#')
		pos = 1;
	else if (in[2] == '#')
		pos = 2;
	else if (in[3] == '#')
		pos = 3;
	else
		return FALSE;

	// 받침 ㄱ,ㄷ,ㅂ이 오는 경우 (박지영 -> 찌영)
	if (strchr("KTP", in[pos-1])) 
	{
		// 모음이 한개만 있어야 함 (싸이트 같은 건 제외되도록)
		for (i=0, n=0; i<pos; i++) {
			if (strchr(pron_vowel, in[i])) {
				if (n) return FALSE;
				n++;
			}
		}
		strcpy(out, &(in[pos+1]));
	}
	// 받침 ㄱ,ㄷ,ㅂ + ㅎ 으로 인해 받침이 초성 격음으로 변한 경우 (백현택 -> 켠택)
	else if (strchr("KTP", in[pos-1]) || strchr("ktp", in[pos+1])) 
	{
		// 반드시 앞이 모음이어야 함
		if (!strchr(pron_vowel, in[pos-1]))
			return FALSE;

		// 모음이 한개만 있어야 함 (싸이트 같은 건 제외되도록)
		for (i=0, n=0; i<pos; i++) {
			if (strchr(pron_vowel, in[i])) {
				if (n) return FALSE;
				n++;
			}
		}
		strcpy(out, &(in[pos+1]));
	} 
	else
		return FALSE;

	return TRUE;
}
*/
void MOEUM_rule2(char *psym, char ch1, char ch2, unsigned char *lvs, int ex)
{
	int	i;
	char	lastp, lv;

	//ch1 = ch;
	if (ch1 == '{' && ch2 != 'W') {  // 의: 앞에 'ㅇ'이 없이 'ㅢ'만 있는 경우  
		if (psym[0] != EOS) {
			lastp = psym[strlen(psym)-1];
			for (i=0; i<32; i++) {
				if (strlen(NPSC[i]) == 1)
					if (lastp == NPSC[i][0]) {
						ch1 = '|';  // 이 
						break;
					}
			}
		}

	// '이' 선행 모음 처리 ( '여' 이후는 추가된것)
	// 1. single: 여,야,요,유 (ㅈ,ㅊ,ㅉ)
	} 

	else if (ch1 == 'j') {	// 여 -> 어
		if (psym[0] != EOS) {
			lastp = psym[strlen(psym)-1];
			if ((lastp == 'j') || (lastp == 'c') || (lastp == 'z')) // ㅈ, ㅊ, ㅉ
				ch1 = 'f';
		}
	} else if (ch1 == 'd') { // 야 -> 아
		if (psym[0] != EOS) {
			lastp = psym[strlen(psym)-1];
			if ((lastp == 'j') || (lastp == 'c') || (lastp == 'z'))
				ch1 = 'b';
		}
	} else if (ch1 == 'r') { // 요 -> 오 
		if (psym[0] != EOS) {
			lastp = psym[strlen(psym)-1];
			if ((lastp == 'j') || (lastp == 'c') || (lastp == 'z'))
				ch1 = 'l';
		}
	} else if (ch1 == 'w') { // 유 -> 우
		if (psym[0] != EOS) {
			lastp = psym[strlen(psym)-1];
			if ((lastp == 'j') || (lastp == 'c') || (lastp == 'z'))
				ch1 = 's';
		}
	} 
	// 2. double: 얘,예 (ㄴ,ㅅ,ㅆ 만 제외)
	else if (ch1 == 'e') { // 얘 -> 애
		if (psym[0] != EOS) {
			lastp = psym[strlen(psym)-1];
			if ((lastp != 'n') && (lastp != 's') && (lastp != 'x'))
				ch1 = 'c';
		}

	} else if (ch1 == 'k') { // 예 -> 에
		if (psym[0] != EOS) {
			lastp = psym[strlen(psym)-1];
			if ((lastp != 'n') && (lastp != 's') && (lastp != 'x'))
				ch1 = 'g';
		}
	} 

	// 모음 + 육 예외처리 (26)
	if (ex == EX_N_ADD)
		strcat(psym, "r");

	strcat(psym,NPSV[ch1-0x60]);
	*lvs = *lvs << 1;
	lv = *lvs & 0x80;
	if (lv)
		strcat(psym,":");
}


// end of file
