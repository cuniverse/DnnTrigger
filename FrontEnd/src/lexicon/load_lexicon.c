
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
 *	@file	load_lexicon.c
 *	@ingroup lexicon_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	lexicon resource loading/creating library
 */

#include <stdio.h>
#include <stdlib.h>
#ifdef WIN32
#include <windows.h>
#endif
#include <math.h>
#include <locale.h>

#ifndef WIN32
#include <string.h> // memset(), strcpy() etc.
#endif
#include <wchar.h>

#include "base/hci_macro.h"
#include "base/hci_msg.h"
#include "base/str2words.h"
#include "base/hci_malloc.h"
#include "basic_op/fixpoint.h"
#include "base/utf8.h"

#include "lexicon/load_lexicon.h"
#include "lexicon/text2lex.h"
#include "lexicon/lexicon_common.h"

typedef struct {
	BaseLex *pLex;			///< pointer to base-phone lexicon
	FullSymbolLex *pSymLex;	///< pointer to full symbolic lexicon
	hci_uint32 idVocab;		///< vocabulary index
} SortLexicon;


// local functions
#ifdef __cplusplus
extern "C" {
#endif 

/**
 *	extract word/symbol string from source vocabulary string
 */
HCILAB_PRIVATE hci_int32
_LEXICON_extractWordName(char *szPureVocab,				///< (o) pure vocabulary string
						 char *szFullName,				///< (o) pure vocab string + word symbol string
						 const char *szSourceVocab);	///< (i_ source vocabulary string

/**
 *	compare two vocabulary base lexicons
 */
HCILAB_PRIVATE hci_int32
_LEXICON_compareBaseLexicons(const void *_x, const void *_y);

/**
 *	compare two vocabulary symbol lexicons
 */
HCILAB_PRIVATE hci_int32
_LEXICON_compareSymbolLexicons(const void *_x, const void *_y);

#ifdef __cplusplus
}
#endif


/**
 *	build compiled lexicon resource from vocabulary list file
 */
hci_int32
RES_LEXICON_buildCompiledLexiconResourceFromVocabFile(Lexicon_Resource *pResLexicon,		///< (o) lexicon resource
													  const char *szVocabListFile,			///< (i) vocabulary list file
													  const char *szUpdateTime,				///< (i) resource update time
													  const float weightTask)				///< (i) QC weight for current task
{
	VOCAB_INFO *pVocabInfo = 0;
	ASR_VOCAB *pVocab = 0;
	ASR_VOCAB *pHitVocab = 0;
	ASR_LEXICON *pLexicon = 0;
	SortLexicon *sortLexicon = 0;
	SortLexicon *pVocLex = 0;
	VOC_LEXICON vocabLex;
	FullSymbolLex *pOutLex = 0;
	FullSymbolLex *pCurLex = 0;
	FullSymbolLex *pPrevLex = 0;
	char *pWordSeq[64];
	char szLineStr[1024];
	char *pStr = 0;
	char szVocab[1024];
	char szPureVocab[1024];
	unsigned char ucHeader[256];
	int n = 0;
	int nLenWordSeq = 0;
	hci_uint32 sizeAllocVoc = 0;
	hci_uint32 sizeAllocLex = 0;
	hci_uint32 sizeAllocVocabStr = 0;
	hci_uint32 iLex = 0;
	hci_uint32 sizeSrcLex = 0;
	FILE *fpVoc = 0;
	hci_uint8 nLenPrevSymSeq = 0;
	hci_uint16 i = 0;
	//DWORD eFileType = eUNK;
	unsigned int eFileType = eUNK;
	hci_int32 flagLoad = 0;
	hci_int32 nFreqCount = 0;
	float unigProb = 0;
	
	char szFile[1024];
	FILE *fpSucc = 0;
	FILE *fpFail = 0;
	FILE *fpInva = 0;
	int nFailCount = 0;
	int nSuccCount = 0;
	int nInvalidCount = 0;

	if (0 == pResLexicon) {
		return -1;
	}
	if (0 == szVocabListFile) {
		return -1;
	}

	fpVoc = fopen(szVocabListFile, "r");
	if ( 0 == fpVoc ) {
		HCIMSG_WARN("[Warning] cannot access Vocab List File (%s) !!\n", szVocabListFile);
		return -1;
	}

	sprintf(szFile, "%s.succ.txt", szVocabListFile);
	fpSucc = fopen(szFile, "wt");

	sprintf(szFile, "%s.fail.txt", szVocabListFile);
	fpFail = fopen(szFile, "wt");

	sprintf(szFile, "%s.invalid.txt", szVocabListFile);
	fpInva = fopen(szFile, "wt");

	pVocabInfo = &pResLexicon->vocab_info;

 	sizeAllocVoc	               = 512;
 	sizeAllocLex                   = 2 * sizeAllocVoc;
 	sizeAllocVocabStr              = sizeAllocVoc * 64;

	pResLexicon->vocab             = (ASR_VOCAB *) hci_calloc(sizeAllocVoc, sizeof(ASR_VOCAB));
 	pResLexicon->szVocabListStr    = (hci_int8 *) hci_calloc(sizeAllocVocabStr, sizeof(hci_int8));
	sortLexicon                    = (SortLexicon *) hci_calloc(sizeAllocLex, sizeof(SortLexicon) );

	pVocabInfo->nMaxLenLexicon = 0;
	pVocabInfo->bSortedLexicon = TRUE;
	pVocabInfo->bSortedLexicon = TRUE;
	pVocabInfo->bSortedLength  = TRUE;
	pVocabInfo->nLexicalType   = FULL_SYMBOL_LEX;
	pVocabInfo->num_lexicons   = 0;
	pVocabInfo->num_vocabs     = 0;
	pVocabInfo->sizePronLex    = 0;
	pVocabInfo->sizeVocabStr   = 0;
	pVocabInfo->nLexicalOrder  = FWD_LEXICON;
	memset(pVocabInfo->nFirstLexIdxPerLength, 0, sizeof(pVocabInfo->nFirstLexIdxPerLength));
	memset(pVocabInfo->nFirstLexDataPerLength, 0, sizeof(pVocabInfo->nFirstLexDataPerLength));
	strcpy(pVocabInfo->szVocabFile, szVocabListFile);
	if ( szUpdateTime ) {
		strcpy(pVocabInfo->szBuildTime, szUpdateTime);
	}
	else {
		strcpy(pVocabInfo->szBuildTime, "<UNKNOWN>");
	}
	if ( weightTask <= 0.0f ) {
		pVocabInfo->weightTask = (-1.0f);
	}
	else if ( weightTask > 1.0f ) {
		pVocabInfo->weightTask = 100.0f * (float)log((double)weightTask);
	}
	else {
		pVocabInfo->weightTask = 0.0f;
	}
	pVocabInfo->totalQC = 0;

	memset(ucHeader, 0, sizeof(ucHeader));
	fgets(ucHeader, 256, fpVoc);
	
    if (ucHeader[0] == 0xff && ucHeader[1] == 0xfe)
    {
        eFileType = eUnicode;
		fseek( fpVoc, 2L, SEEK_SET );
		fprintf(stdout, "file type: unicode !!\n"); fflush(stdout);
	}
    // Check for Unicode Big Endian
    else if (ucHeader[0] == 0xfe && ucHeader[1] == 0xff)
    {
        eFileType = eUnicodeBigEndian;
		fseek( fpVoc, 2L, SEEK_SET );
		fprintf(stdout, "file type: unicode big-endian !!\n"); fflush(stdout);
	}
    // Check for UTF-8
    else if (ucHeader[0] == 0xef && ucHeader[1] == 0xbb){
        // UTF-8 has a three char header
        if (ucHeader[2] == 0xbf){
            eFileType = eUTF8;
			fseek( fpVoc, 2L, SEEK_SET );
		}
		else {
			eFileType = eUNK;
			fseek( fpVoc, 0L, SEEK_SET );
		}
    }
    else{
		eFileType = eUNK;
		fseek( fpVoc, 0L, SEEK_SET );
    }

	if ( eFileType == eUnicode || eFileType == eUnicodeBigEndian ) {
		wchar_t *wszLineStr = 0;
		char *szOutStr = 0;
		LONG txtSize = 0;
		char *pMark = 0;

		if ( !setlocale(LC_ALL, "") ) {
			fprintf(stderr, "cannot set the specified locale !!\n");
		}
		else {
			memset(szLineStr, 0, sizeof(szLineStr));
			fseek(fpVoc, 0L, SEEK_END);
			txtSize = (LONG)ftell(fpVoc) - 2;
		}

/*		if ( txtSize > 0L ) {
			int nLineCount = 0;
			wszLineStr = (wchar_t *) hci_calloc( txtSize + 2, sizeof(char) );
			fseek(fpVoc, 2L, SEEK_SET);
			fread(wszLineStr, 1, txtSize, fpVoc);

			if ( eFileType == eUnicodeBigEndian ) {
				UnicodeBigEndianToUnicode( (wchar16_t *)wszLineStr, txtSize );
			}

			szOutStr = UnicodeToAnsi( wszLineStr, -1 );

			free(wszLineStr); wszLineStr = 0;
		}*/
		fprintf(stderr, "[Warning] cannot process unicode-type query file !!\n");
		fprintf(stderr, "At first, change the encoding type of query file into ANSI or UTF-8 !!\n");

		pMark = szOutStr;
		while ( pMark )
		{

			memset(szLineStr, 0, sizeof(szLineStr) );
			strncpy(szLineStr, pMark, 256);

			// remove 'new line' & 'carriage return'
			pStr = strchr(szLineStr, '\n');
			if ( pStr ) *pStr = '\0';
			pStr = strchr(szLineStr, '\r');
			if ( pStr ) *pStr = '\0';

			// update vocabulary string pointer
			pMark = strchr(pMark, '\n');
			if ( pMark ) pMark++;

			memset(szVocab, 0, sizeof(szVocab));

			nLenWordSeq = PowerASR_Base_str2words(szLineStr, pWordSeq, 64 );
			if ( nLenWordSeq <= 1 ) continue;

			if ( !isdigit(pWordSeq[nLenWordSeq-1][0]) ) {
				fprintf(stderr, "[warning] invalid query list format !!\n");
				fprintf(stderr, "[notice] query format (2 fields): query-string [tab] freq-count !!\n");
				break;
			}

			strcpy(szVocab, pWordSeq[0]);
			for ( n = 1 ; (n+1) < nLenWordSeq ; n++ )
				sprintf(szVocab, "%s %s", szVocab, pWordSeq[n]);

			nFreqCount = atoi(pWordSeq[n]);

			if ( strlen(szVocab) >= 64 ) continue;

			if ( pVocabInfo->num_vocabs >= sizeAllocVoc ) {
				sizeAllocVoc += 10;
				pResLexicon->vocab = (ASR_VOCAB *) hci_realloc( pResLexicon->vocab, sizeAllocVoc * sizeof(ASR_VOCAB) );
			}
			pVocab = pResLexicon->vocab + pVocabInfo->num_vocabs;
			pVocab->nPtrVocabStr = pVocabInfo->sizeVocabStr;

			pVocabInfo->totalQC += (double)nFreqCount;

			strcpy(szPureVocab, szVocab);
			strcat(szVocab, ";");
			pVocab->nLenVocabStr = (hci_uint16)strlen(szVocab);
			unigProb = 100.0f * (float)log((double)nFreqCount);
			pVocab->nUniGramProb = FLOAT2FIX16_ANY( unigProb, 0 );
			if ((pVocabInfo->sizeVocabStr+pVocab->nLenVocabStr) >= sizeAllocVocabStr) {
				sizeAllocVocabStr += 256;
				pResLexicon->szVocabListStr = (hci_int8 *) hci_realloc( pResLexicon->szVocabListStr, sizeAllocVocabStr * sizeof(hci_int8) );
			}
			strcpy(pResLexicon->szVocabListStr + pVocabInfo->sizeVocabStr, szVocab);
			pVocabInfo->sizeVocabStr += pVocab->nLenVocabStr;

			memset( &vocabLex, 0, sizeof(vocabLex) );
			if (TEXT2PHONE_convertTextIntoLexicons(szPureVocab, &vocabLex, MAX_NUM_LEXICON) == 0) {
				hci_uint32 iLex = 0;
				PRON_LEXICON *pSrcLex = vocabLex.pronDict;
				if ( (pVocabInfo->num_lexicons + vocabLex.nNumLexicons) > sizeAllocLex ) {
					sizeAllocLex += 32;
					sortLexicon = (SortLexicon *) hci_realloc( sortLexicon, sizeAllocLex * sizeof(SortLexicon) );
				}
				for (iLex = 0; iLex < vocabLex.nNumLexicons; iLex++, pSrcLex++) {
					if (pSrcLex->nLenPhoneSeq) {
						pVocLex           = sortLexicon + pVocabInfo->num_lexicons;
						pVocLex->idVocab  = pVocabInfo->num_vocabs;
						pVocLex->pSymLex  = (FullSymbolLex *) hci_calloc( pSrcLex->nLenPhoneSeq + 1, sizeof(FullSymbolLex) );
						pOutLex           = pVocLex->pSymLex;
						pOutLex->idSymbol = (hci_uint8)pSrcLex->nLenPhoneSeq;
						pOutLex++;
						for (i = 0; i < pSrcLex->nLenPhoneSeq; i++) {
							pOutLex->idSymbol    = pSrcLex->PhoneSeq[i];
							pOutLex->flagSilence = pSrcLex->flagSP[i];
							pOutLex++;
						}
						pVocabInfo->num_lexicons++;
						pVocabInfo->sizePronLex   += pSrcLex->nLenPhoneSeq + 2;
						pVocabInfo->nMaxLenLexicon = HCI_MAX(pVocabInfo->nMaxLenLexicon, pSrcLex->nLenPhoneSeq);
					}
				}
			}
			pVocabInfo->num_vocabs += 1;
			memset(szLineStr, 0, sizeof(szLineStr));
			if ( pVocabInfo->num_vocabs % 20000 == 0 ) {
				fprintf(stdout, "."); fflush(stdout);
			}
			if ( pVocabInfo->num_vocabs % 1000000 == 0 ) {
				fprintf(stdout, "\n"); fflush(stdout);
			}
		}
		if ( szOutStr ) { hci_free(szOutStr); szOutStr = 0; }
	}
	else {
		int bufSize = 0;
		int nLenCopy = 0;
		int nCountField = 0;
		char szOutBuf[1024];
		char szQuery[1024];
		char szQC[1024];
		memset(szLineStr, 0, sizeof(szLineStr));
		while ( fgets(szLineStr, 1024, fpVoc) )
		{
			if ( sscanf(szLineStr, "%s", szVocab) == 0 ) continue;

			strcpy(szQuery, szLineStr);
			pStr = strchr(szQuery, '\t');
			if ( pStr ) *pStr = '\0';
			else continue;

			if ( eFileType == eUTF8 || PowerASR_Base_is_UTF8(szQuery) ) {
				bufSize = (int)strlen(szLineStr) + 1;
				memset(szOutBuf, 0, sizeof(szOutBuf));
				nLenCopy = PowerASR_Base_decode_UTF8(szLineStr, bufSize, szOutBuf, bufSize);
				memset(szLineStr, 0, sizeof(szLineStr));
				strcpy(szLineStr, szOutBuf);
			}

			// remove 'new line' & 'carriage return'
			pStr = strchr(szLineStr, '\n');
			if ( pStr ) *pStr = '\0';
			pStr = strchr(szLineStr, '\r');
			if ( pStr ) *pStr = '\0';

			// count fields
			nCountField = 1;
			pStr = strchr(szLineStr, '\t');
			while ( pStr ) {
				nCountField++;
				pStr = strchr(pStr+1, '\t');
			}

			if ( nCountField <= 1 || nCountField > 3 ) {
				fprintf(fpInva, "%s\n", szOutBuf); fflush(fpInva);
				nInvalidCount++;
				continue;
			}
			else {
				strcpy(szQuery, szLineStr);
				strcpy(szQC, strrchr(szQuery, '\t')+1);
				pStr = strchr(szQuery, '\t');
				*pStr = '\0';
			}

			strcpy(szOutBuf, szLineStr);
//			fprintf(stdout, "query = (%s)\n", szOutBuf); fflush(stdout);

			memset(szVocab, 0, sizeof(szVocab));

			nLenWordSeq = PowerASR_Base_str2words(szQuery, pWordSeq, 64 );
			if ( nLenWordSeq <= 0 ) {
				fprintf(fpInva, "%s\n", szOutBuf); fflush(fpInva);
				nInvalidCount++;
				continue;
			}

			if ( !isdigit(szQC[0]) ) {
				fprintf(stderr, "[warning] invalid query list format !!\n");
				fprintf(stderr, "[notice] query format (2 fields): query-string [tab] freq-count !!\n");
				break;
			}

			strcpy(szVocab, pWordSeq[0]);
			for ( n = 1 ; n < nLenWordSeq ; n++ )
				sprintf(szVocab, "%s %s", szVocab, pWordSeq[n]);

			nFreqCount = atoi(szQC);

			if ( strlen(szVocab) >= 64 ) {
				fprintf(fpInva, "%s\n", szVocab); fflush(fpInva);
				nInvalidCount++;
				continue;
			}

			pVocabInfo->totalQC += (double)nFreqCount;

			memset( &vocabLex, 0, sizeof(vocabLex) );
			if ( TEXT2PHONE_convertTextIntoLexicons(szVocab, &vocabLex, MAX_NUM_LEXICON) == 0 && vocabLex.nNumLexicons > 0 ) {
				hci_uint32 iLex = 0;
				PRON_LEXICON *pSrcLex = vocabLex.pronDict;
				if ( (pVocabInfo->num_lexicons + vocabLex.nNumLexicons) > sizeAllocLex ) {
					sizeAllocLex += 32;
					sortLexicon = (SortLexicon *) hci_realloc( sortLexicon, sizeAllocLex * sizeof(SortLexicon) );
				}
				for (iLex = 0; iLex < vocabLex.nNumLexicons; iLex++, pSrcLex++) {
					if (pSrcLex->nLenPhoneSeq) {
						pVocLex           = sortLexicon + pVocabInfo->num_lexicons;
						pVocLex->idVocab  = pVocabInfo->num_vocabs;
						pVocLex->pSymLex  = (FullSymbolLex *) hci_calloc( pSrcLex->nLenPhoneSeq + 1, sizeof(FullSymbolLex) );
						pOutLex           = pVocLex->pSymLex;
						pOutLex->idSymbol = (hci_uint8)pSrcLex->nLenPhoneSeq;
						pOutLex++;
						for (i = 0; i < pSrcLex->nLenPhoneSeq; i++) {
							pOutLex->idSymbol    = pSrcLex->PhoneSeq[i];
							pOutLex->flagSilence = pSrcLex->flagSP[i];
							pOutLex++;
						}
						pVocabInfo->num_lexicons++;
						pVocabInfo->sizePronLex   += pSrcLex->nLenPhoneSeq + 2;
						pVocabInfo->nMaxLenLexicon = HCI_MAX(pVocabInfo->nMaxLenLexicon, pSrcLex->nLenPhoneSeq);
					}
				}

				fprintf(fpSucc, "%s\n", szVocab); fflush(fpSucc);
				nSuccCount++;

				if ( pVocabInfo->num_vocabs >= sizeAllocVoc ) {
					sizeAllocVoc += 10;
					pResLexicon->vocab = (ASR_VOCAB *) hci_realloc( pResLexicon->vocab, sizeAllocVoc * sizeof(ASR_VOCAB) );
				}
				pVocab = pResLexicon->vocab + pVocabInfo->num_vocabs;
				pVocab->nPtrVocabStr = pVocabInfo->sizeVocabStr;

				strcat(szVocab, ";");
				pVocab->nLenVocabStr = (hci_uint16)strlen(szVocab);
				unigProb = 100.0f * (float)log((double)nFreqCount);
				pVocab->nUniGramProb = FLOAT2FIX16_ANY( unigProb, 0 );
				if ((pVocabInfo->sizeVocabStr+pVocab->nLenVocabStr) >= sizeAllocVocabStr) {
					sizeAllocVocabStr += 256;
					pResLexicon->szVocabListStr = (hci_int8 *) hci_realloc( pResLexicon->szVocabListStr, sizeAllocVocabStr * sizeof(hci_int8) );
				}
				strcpy(pResLexicon->szVocabListStr + pVocabInfo->sizeVocabStr, szVocab);
				pVocabInfo->sizeVocabStr += pVocab->nLenVocabStr;

				pVocabInfo->num_vocabs += 1;
				if ( pVocabInfo->num_vocabs % 20000 == 0 ) {
					fprintf(stdout, "."); fflush(stdout);
				}
				if ( pVocabInfo->num_vocabs % 1000000 == 0 ) {
					fprintf(stdout, "\n"); fflush(stdout);
				}
			}
			else {
				fprintf(fpFail, "%s\n", szVocab); fflush(fpFail);
				nFailCount++;
			}
			memset(szLineStr, 0, sizeof(szLineStr));
		}
	}

	fclose(fpVoc); fpVoc = 0;

	fclose(fpSucc); fpSucc = 0;
	fclose(fpFail); fpFail = 0;
	fclose(fpInva); fpInva = 0;

	fprintf(stdout, "\n"); fflush(stdout);
	fprintf(stdout, "Total Vocabulary Size = %d\n", nSuccCount+nFailCount+nInvalidCount);
	fprintf(stdout, "Lexicon Size = %lu\n", pVocabInfo->num_lexicons);
	fprintf(stdout, "Total QC = %e\n", pVocabInfo->totalQC);
	fprintf(stdout, "Task Weight = %f (%f)\n", weightTask, pVocabInfo->weightTask);
	fprintf(stdout, "G2P Success >> %d\n", nSuccCount);
	fprintf(stdout, "G2P Failure >> %d\n", nFailCount);
	fprintf(stdout, "Invalid Query >> %d\n", nInvalidCount);
	fflush(stdout);

	if ( pVocabInfo->num_vocabs == 0U ) 
	{
		HCIMSG_WARN("[Warning] Empty Valid Vocabulary List (%s) !!\n", szVocabListFile);
		if ( pResLexicon->vocab ) { hci_free(pResLexicon->vocab); pResLexicon->vocab = 0; }
		if ( pResLexicon->szVocabListStr ) { hci_free(pResLexicon->szVocabListStr); pResLexicon->szVocabListStr = 0; }
		if ( sortLexicon ) {
			hci_uint32 u = 0;
			for ( u = 0; u < pVocabInfo->num_lexicons ; u++) {
				if ( sortLexicon[u].pSymLex ) { hci_free(sortLexicon[u].pSymLex); sortLexicon[u].pSymLex = 0; }
			}
			hci_free(sortLexicon); sortLexicon = 0;
		}
		memset(pResLexicon, 0, sizeof(*pResLexicon));
		return -1;
	}

	qsort((char *)sortLexicon, pVocabInfo->num_lexicons, sizeof(SortLexicon), _LEXICON_compareSymbolLexicons);

	pResLexicon->fullSymbolPronLex = (FullSymbolLex *) hci_calloc( pVocabInfo->sizePronLex, sizeof(FullSymbolLex) );
	pResLexicon->lexicon = (ASR_LEXICON *) hci_calloc( pVocabInfo->num_lexicons, sizeof(ASR_LEXICON) );

	pVocabInfo->sizePronLex = 0;
	pOutLex = pResLexicon->fullSymbolPronLex;
/*
	pResLexicon->lexicon[0].idVocab = sortLexicon[0].idVocab;
	pCurLex = sortLexicon[0].pSymLex;
	nLenPrevSymSeq = pCurLex->idSymbol;
	memcpy(pOutLex, pCurLex, sizeof(FullSymbolLex));
	pOutLex++;
	memset(pOutLex, 0, sizeof(FullSymbolLex));
	pOutLex++;
	memcpy(pOutLex, pCurLex+1, (pCurLex->idSymbol)*sizeof(FullSymbolLex));
	pOutLex += pCurLex->idSymbol;
	pVocabInfo->sizePronLex += pCurLex->idSymbol + 2;
	sizeSrcLex += pCurLex->idSymbol + 2;
	pPrevLex = pCurLex;
	for (iLex = 1; iLex < pVocabInfo->num_lexicons; iLex++) {
		pResLexicon->lexicon[iLex].idVocab = sortLexicon[iLex].idVocab;
		pCurLex = sortLexicon[iLex].pSymLex;
		sizeSrcLex += pCurLex->idSymbol + 2;
		memcpy(pOutLex, pCurLex, sizeof(FullSymbolLex));
		pOutLex++;
		if (pCurLex->idSymbol != pPrevLex->idSymbol) {
			pVocabInfo->nFirstLexIdxPerLength[pCurLex->idSymbol]  = iLex;
			pVocabInfo->nFirstLexDataPerLength[pCurLex->idSymbol] = pVocabInfo->sizePronLex;
			for (i = nLenPrevSymSeq + 1; i < pCurLex->idSymbol; i++) {
				pVocabInfo->nFirstLexIdxPerLength[i]  = iLex;
				pVocabInfo->nFirstLexDataPerLength[i] = pVocabInfo->sizePronLex;
			}
			nLenPrevSymSeq = pCurLex->idSymbol;
			memset(pOutLex, 0, sizeof(FullSymbolLex));
			pOutLex++;
			memcpy(pOutLex, pCurLex+1, (pCurLex->idSymbol)*sizeof(FullSymbolLex));
			pOutLex += pCurLex->idSymbol;
			pVocabInfo->sizePronLex += pCurLex->idSymbol + 2;
		}
		else {
			pVocabInfo->sizePronLex++;
			for (i = 0; i < pCurLex->idSymbol; i++) {
				if (memcmp(pCurLex+i+1,pPrevLex+i+1,sizeof(FullSymbolLex))) {
					pOutLex->idSymbol    = (hci_uint8)i;
					pOutLex->flagSilence = 0;
					pOutLex++;
					pVocabInfo->sizePronLex++;
					break;
				}
			}
			if (i == pCurLex->idSymbol) {
				pOutLex->idSymbol    = (hci_uint8)i;
				pOutLex->flagSilence = 0;
				pOutLex++;
				pVocabInfo->sizePronLex++;
			}
			for ( ; i < pCurLex->idSymbol; i++ ) {
				memcpy(pOutLex, pCurLex+i+1, sizeof(FullSymbolLex));
				pOutLex++;
				pVocabInfo->sizePronLex++;
			}
		}
		pPrevLex = pCurLex;
	}
*/
	pResLexicon->lexicon[0].idVocab = sortLexicon[0].idVocab;
	pCurLex = sortLexicon[0].pSymLex;
	nLenPrevSymSeq = pCurLex->idSymbol;
	memset(pOutLex, 0, sizeof(FullSymbolLex));
	pOutLex++;
	memcpy(pOutLex, pCurLex, sizeof(FullSymbolLex));
	pOutLex++;
	memcpy(pOutLex, pCurLex+1, (pCurLex->idSymbol)*sizeof(FullSymbolLex));
	pOutLex += pCurLex->idSymbol;
	pVocabInfo->sizePronLex += pCurLex->idSymbol + 2;
	sizeSrcLex += pCurLex->idSymbol + 2;
	pPrevLex = pCurLex;
	for (iLex = 1; iLex < pVocabInfo->num_lexicons; iLex++) {
		pResLexicon->lexicon[iLex].idVocab = sortLexicon[iLex].idVocab;
		pCurLex = sortLexicon[iLex].pSymLex;
		sizeSrcLex += pCurLex->idSymbol + 2;
//		memcpy(pOutLex, pCurLex, sizeof(FullSymbolLex));
//		pOutLex++;
		if (pCurLex->idSymbol != pPrevLex->idSymbol) {
			pVocabInfo->nFirstLexIdxPerLength[pCurLex->idSymbol]  = iLex;
			pVocabInfo->nFirstLexDataPerLength[pCurLex->idSymbol] = pVocabInfo->sizePronLex;
			for (i = nLenPrevSymSeq + 1; i < pCurLex->idSymbol; i++) {
				pVocabInfo->nFirstLexIdxPerLength[i]  = iLex;
				pVocabInfo->nFirstLexDataPerLength[i] = pVocabInfo->sizePronLex;
			}
			nLenPrevSymSeq = pCurLex->idSymbol;
			memset(pOutLex, 0, sizeof(FullSymbolLex));
			pOutLex++;
			memcpy(pOutLex, pCurLex, (pCurLex->idSymbol+1)*sizeof(FullSymbolLex));
			pOutLex += pCurLex->idSymbol + 1;
			pVocabInfo->sizePronLex += pCurLex->idSymbol + 2;
		}
		else {
			for (i = 0; i < pCurLex->idSymbol; i++) {
				if (memcmp(pCurLex+i+1,pPrevLex+i+1,sizeof(FullSymbolLex))) {
					pOutLex->idSymbol    = (hci_uint8)i;
					pOutLex->flagSilence = 0;
					pOutLex++;
					break;
				}
			}
			if (i == pCurLex->idSymbol) {
				pOutLex->idSymbol    = (hci_uint8)i;
				pOutLex->flagSilence = 0;
				pOutLex++;
			}
			memcpy(pOutLex, pCurLex, (pCurLex->idSymbol+1) * sizeof(FullSymbolLex));
			pOutLex += pCurLex->idSymbol + 1;
			pVocabInfo->sizePronLex += pCurLex->idSymbol + 2;
		}
		pPrevLex = pCurLex;
	}

	for (i = nLenPrevSymSeq + 1; i < MAX_LEN_LEXICON; i++) {
		pVocabInfo->nFirstLexIdxPerLength[i]  = pVocabInfo->num_lexicons;
		pVocabInfo->nFirstLexDataPerLength[i] = pVocabInfo->sizePronLex;
	}

	if ( sortLexicon ) {
		hci_uint32 u = 0;
		for ( u = 0; u < pVocabInfo->num_lexicons ; u++) {
			if ( sortLexicon[u].pSymLex ) { 
				hci_free(sortLexicon[u].pSymLex);
				sortLexicon[u].pSymLex = 0;
			}
		}
		hci_free(sortLexicon); sortLexicon = 0;
	}

	return 0;
}


/**
 *	load exceptional pronunciation dictionary file
 */
hci_int32
RES_LEXICON_loadExceptionalPronDict( const char *szExceptPronFile ) 		///< (i) 예외사전 파일
{
	return TEXT2PHONE_loadExceptionalDict(szExceptPronFile);
}


/**
 *	load user-defined dictionary file
 */
hci_int32
RES_LEXICON_loadUserDictFile( const char *szUserDictFile ) 		///< (i) 사용자 정의 예외사전 파
{
	return TEXT2PHONE_loadExceptionalDict(szUserDictFile);
}


/**
 *	load english unit dictionary file
 */
hci_int32
RES_LEXICON_loadUnitDictFile( const char *szUnitDictFile) 		///< (i) 영어 단위사전 파일
{
	return TEXT2PHONE_loadEnUnitDict(szUnitDictFile);
}

/**
 *	create hash table for exceptional pron-dict
 */
hci_int32
RES_LEXICON_createExceptDictHashTable()
{
	return TEXT2PHONE_createExceptDictHashTable();
}


/**
 *	free memories allocated to exceptional-pronunciation dictionary data
 */
hci_int32
RES_LEXICON_freeExceptionalPronDict()
{
	return TEXT2PHONE_freeExceptionalDict();
}


/**
 *	extract word/symbol string from source vocabulary string
 */
HCILAB_PRIVATE hci_int32
_LEXICON_extractWordName(char *szPureVocab,			///< (o) pure vocabulary string
						 char *szFullName,			///< (o) pure vocab string + word symbol string
						 const char *szSourceVocab)	///< (i_ source vocabulary string
{
	char *pComma = 0;
	const char *pWordEnd = 0;
	char szSourceWord[512];
	char szFrontWord[512];
	char szBackEnd[512];
	char szWordSymbol[512];
	hci_int32 nLenVocab = 0;
	hci_flag bSpotWord = 0;
	hci_int32 numWords = 0, iWord = 0;
	char *pszWordSeq[8];

	if (0 == szSourceVocab || *szSourceVocab == '\0') return -1;

	memset(szSourceWord, 0, sizeof(szSourceWord));
	memset(szWordSymbol, 0, sizeof(szWordSymbol));

	if (*szSourceVocab=='[') {	// [xxx][yyy]...
		pWordEnd = strchr(szSourceVocab, ']');	
		if (pWordEnd) {
			nLenVocab = pWordEnd - szSourceVocab + 1;
			if (nLenVocab > 2) {
				strncpy(szSourceWord, szSourceVocab+1, (hci_uint32)(nLenVocab-2));
				bSpotWord = 1;
			}
		}
	}
	else if (strchr(szSourceVocab, ';')) {	// xxx;yyy;...
		pWordEnd = strchr(szSourceVocab, ';');
		if (pWordEnd) {
			nLenVocab = pWordEnd - szSourceVocab + 1;
			if (nLenVocab > 1) {
				strncpy(szSourceWord, szSourceVocab, (hci_uint32)(nLenVocab-1));
				bSpotWord = 1;
			}
		}
	}
	else if (strchr(szSourceVocab, '|')) {	// xxx|yyy|...
		pWordEnd = strchr(szSourceVocab, '|');
		if (pWordEnd) {
			nLenVocab = pWordEnd - szSourceVocab + 1;
			if (nLenVocab > 1) {
				strncpy(szSourceWord, szSourceVocab, (hci_uint32)(nLenVocab-1));
				bSpotWord = 1;
			}
		}
	}

	if (bSpotWord) {
		memset(szFrontWord, 0, sizeof(szFrontWord));
		strcpy(szFrontWord, szSourceWord);
//		pComma = strchr(szFrontWord, ',');
		pComma = 0;
		if (pComma) {
			memset(szBackEnd, 0, sizeof(szBackEnd));
			strcpy(szBackEnd, pComma+1);
			*pComma = '\0';
			numWords = PowerASR_Base_str2words(szFrontWord, pszWordSeq, 8);
			if (numWords > 0) {
				strcpy(szPureVocab, pszWordSeq[0]);
				for (iWord = 1; iWord < numWords; iWord++) {
					sprintf(szPureVocab, "%s %s", szPureVocab, pszWordSeq[iWord]);
				}
				numWords = PowerASR_Base_str2words(szBackEnd, pszWordSeq, 8);
				if (numWords > 0) {
					strcpy(szWordSymbol, pszWordSeq[0]);
					for (iWord = 1; iWord < numWords; iWord++) {
						sprintf(szWordSymbol, "%s %s", szWordSymbol, pszWordSeq[iWord]);
					}
					if (strcmp(szPureVocab, szWordSymbol)) {
						sprintf(szFullName, "%s,%s;", szPureVocab, szWordSymbol);
					}
					else {
						sprintf(szFullName, "%s;", szPureVocab);
					}
				}
				else {
					sprintf(szFullName, "%s;", szPureVocab);
				}
			}
			else {
				bSpotWord = 0;
			}
		}
		else {
			numWords = PowerASR_Base_str2words(szFrontWord, pszWordSeq, 8);
			if (numWords > 0) {
				strcpy(szPureVocab, pszWordSeq[0]);
				for (iWord = 1; iWord < numWords; iWord++) {
					sprintf(szPureVocab, "%s %s", szPureVocab, pszWordSeq[iWord]);
				}
				sprintf(szFullName, "%s;", szPureVocab);
			}
			else {
				bSpotWord = 0;
			}
		}
	}

	if (bSpotWord==0) {
		nLenVocab = 0;
	}

	return nLenVocab;
}

/**
 *	compare two vocabulary base lexicons
 */
HCILAB_PRIVATE hci_int32
_LEXICON_compareBaseLexicons(const void *_x, const void *_y)
{
	const SortLexicon *x = (const SortLexicon*)_x;
	const SortLexicon *y = (const SortLexicon*)_y;

	const BaseLex *pVocabLex1 = x->pLex;
	const BaseLex *pVocabLex2 = y->pLex;

	const BaseLex *Phone1 = 0;
	const BaseLex *Phone2 = 0;
	hci_uint8 nLenPhoneSeq1 = 0;
	hci_uint8 nLenPhoneSeq2 = 0;
	hci_uint8 n = 0;
	hci_uint8 nMaxLenCompare = 0;
	hci_int32 L_val_out = 0;

	nLenPhoneSeq1 = pVocabLex1[0];
	nLenPhoneSeq2 = pVocabLex2[0];

	Phone1 = pVocabLex1 + 1;
	Phone2 = pVocabLex2 + 1;
	nMaxLenCompare = HCI_MIN(nLenPhoneSeq1, nLenPhoneSeq2);

	for (n = 0; n < nMaxLenCompare; n++) {
		if (*Phone1 != *Phone2) {
			L_val_out = (hci_int32)(*Phone1) - (hci_int32)(*Phone2);
			return L_val_out;
		}
		Phone1++; Phone2++;
	}

	L_val_out = (hci_int32)(nLenPhoneSeq1) - (hci_int32)(nLenPhoneSeq2);

	return L_val_out;
}


/**
 *	compare two vocabulary symbol lexicons
 */
HCILAB_PRIVATE hci_int32
_LEXICON_compareSymbolLexicons(const void *_x, const void *_y)
{
	const SortLexicon *x = (const SortLexicon*)_x;
	const SortLexicon *y = (const SortLexicon*)_y;

	const FullSymbolLex *pVocabLex1 = x->pSymLex;
	const FullSymbolLex *pVocabLex2 = y->pSymLex;

	const FullSymbolLex *Phone1 = 0;
	const FullSymbolLex *Phone2 = 0;
	hci_uint8 nLenPhoneSeq1 = 0;
	hci_uint8 nLenPhoneSeq2 = 0;
	hci_uint8 n = 0;
	hci_int32 L_val_out = 0;

	nLenPhoneSeq1 = pVocabLex1->idSymbol;
	nLenPhoneSeq2 = pVocabLex2->idSymbol;

	if (nLenPhoneSeq1 != nLenPhoneSeq2) {
		L_val_out = (hci_int32)(nLenPhoneSeq1) - (hci_int32)(nLenPhoneSeq2);
		return L_val_out;
	}

	Phone1 = pVocabLex1 + 1;
	Phone2 = pVocabLex2 + 1;

	for (n = 0; n < nLenPhoneSeq1; n++) {
		if (memcmp(Phone1, Phone2, sizeof(FullSymbolLex))) {
			L_val_out = memcmp(Phone1, Phone2, sizeof(FullSymbolLex));
			return L_val_out;
		}
		Phone1++; Phone2++;
	}

	return L_val_out;
}


// end of file
