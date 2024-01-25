
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
 *	@file	hci_resource_lexicon.c
 *	@ingroup lexicon_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	PowerASR vocabulary lexicon resource manager library
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "base/hci_malloc.h"
#include "base/hci_msg.h"
#include "base/parse_config.h"
#include "base/hci_macro.h"

#include "lexicon/hci_resource_lexicon.h"
#include "lexicon/load_lexicon.h"

/** inner data struct for lexicon resource manager */
typedef struct 
{
	LexiconParameters paraLexicon;			///< parameters for lexicon resource manager
} Resource_Lexicon_Inner;


// local functions
#ifdef __cplusplus
extern "C" {
#endif 

/**
 *  release memories allocated to lexicon resource
 */
HCILAB_PRIVATE hci_int32
_Resource_Lexicon_releaseLexiconResource(Lexicon_Resource *pResLexicon);		///< (i/o) lexicon resource


/**
 * extract recognized vocabulary string and symbol string
 */
HCILAB_PRIVATE hci_int32
_Resource_Lexicon_getWordString(char *szWordName,			///< (o) recognized vocabulary string
								char *szWordSymbol,			///< (o) recognized symbol string
								const char *pSrcVocab,		///< (i) source vocabulary string (name + symbol)
								const hci_uint32 nLenVocab); ///< (i) length of source vocabulary string

/**
 *	allocate memory space of compiled lexicon resource
 */
HCILAB_PRIVATE hci_uint32
_Resource_Lexicon_allocateLexiconResource(Lexicon_Resource *pResLexicon);

/**
 *	copy ASR vocabulary resource
 */
HCILAB_PRIVATE hci_uint32
_Resource_Lexicon_copyAsrVocabulary(Lexicon_Resource *pResLexicon,
									const char* pSrcData,
									const hci_uint32 nDataSize,
									const hci_uint32 nCopyPtr);

/**
 *	copy ASR lexicon list resource
 */
HCILAB_PRIVATE hci_uint32
_Resource_Lexicon_copyAsrLexicon(Lexicon_Resource *pResLexicon,
								 const char* pSrcData,
								 const hci_uint32 nDataSize,
								 const hci_uint32 nCopyPtr);

/**
 *	copy full symbol lexicon resource
 */
HCILAB_PRIVATE hci_uint32
_Resource_Lexicon_copyFullSymbolLexicon(Lexicon_Resource *pResLexicon,
										const char* pSrcData,
										const hci_uint32 nDataSize,
										const hci_uint32 nCopyPtr);

/**
 *	copy vocabulary list string resource
 */
HCILAB_PRIVATE hci_uint32
_Resource_Lexicon_copyVocabularyListString(Lexicon_Resource *pResLexicon,
										   const char* pSrcData,
										   const hci_uint32 nDataSize,
										   const hci_uint32 nCopyPtr);

/**
 * setup default environments for Lexicon resource manager
 */
HCILAB_PRIVATE hci_int32
_Resource_Lexicon_defaultConfigurations(PowerASR_Resource_Lexicon *pThis);


/**
 * setup environments from a given configuration file
 */
HCILAB_PRIVATE hci_int32
_Resource_Lexicon_loadConfigurations(PowerASR_Resource_Lexicon *pThis,
									 const char *pszHomeDir,
									 const char *pszConfigFile);

#ifdef __cplusplus
}
#endif


/**
 *	create a new Lexicon resource manager.
 *
 *	@return Return the pointer to a newly created Lexicon resource manager
 */
HCILAB_PUBLIC HCI_LEXICON_API PowerASR_Resource_Lexicon*
PowerASR_Resource_Lexicon_new()
{
	PowerASR_Resource_Lexicon *pResourceLexicon = 0;
	Resource_Lexicon_Inner *pInner = 0;

	pResourceLexicon = (PowerASR_Resource_Lexicon *) hci_malloc( sizeof(PowerASR_Resource_Lexicon) );

	if ( pResourceLexicon ) {
		memset(pResourceLexicon, 0, sizeof(PowerASR_Resource_Lexicon));

		pInner = (Resource_Lexicon_Inner *) hci_malloc( sizeof(Resource_Lexicon_Inner) );
		if ( pInner ) {
			memset(pInner, 0, sizeof(Resource_Lexicon_Inner));
			pResourceLexicon->pInner = (void *)pInner;
		}
	}
	else {
		HCIMSG_ERROR("cannot create Lexicon resource manager.\n");
	}

	return pResourceLexicon;
}


/**
 *	delete the Lexicon resource manager.
 */
HCILAB_PUBLIC HCI_LEXICON_API void
PowerASR_Resource_Lexicon_delete(PowerASR_Resource_Lexicon *pThis)
{
	Resource_Lexicon_Inner *pInner = 0;

	if (0 == pThis) {
		return;
	}

	pInner = (Resource_Lexicon_Inner *) pThis->pInner;

	if ( pInner ) hci_free(pInner);
	hci_free(pThis);
}


/**
 *	load lexical model resource.
 *
 *	@return Return 0 if lexical model resource are loaded successfully, otherwise return -1.
 */
HCILAB_PUBLIC HCI_LEXICON_API hci_int32
PowerASR_Resource_Lexicon_openLexiconResource(PowerASR_Resource_Lexicon *pThis,		///< (i/o) pointer to the Lexicon resource manager
											  const char *pszHomeDir,				///< (i) working directory name
											  const char *pszConfigFile)			///< (i) configuration file
{
	Resource_Lexicon_Inner *pInner = 0;
	Lexicon_Resource *pResLexicon = 0;
	LexiconParameters *paraLexicon = 0;
	hci_int32 bSetup = 0;
	hci_uint32 idTask = 0;
	hci_uint32 idBuf = 0;

	if (0 == pThis) {
		return -1;
	}

	pInner = (Resource_Lexicon_Inner *) pThis->pInner;

	if (0 == pszConfigFile) {
		bSetup = _Resource_Lexicon_defaultConfigurations(pThis);
	}
	else {
		bSetup = _Resource_Lexicon_loadConfigurations(pThis, pszHomeDir, pszConfigFile);
	}

	if (0 != bSetup) {
		return -1;
	}

	paraLexicon = &pInner->paraLexicon;

	memset( pThis->resLexicon, 0, sizeof(pThis->resLexicon) );

	for (idTask = 0; idTask < MAX_TASK_COUNT; idTask++) {
		for ( idBuf = 0; idBuf < 2; idBuf++) {
				pResLexicon = &(pThis->resLexicon[idTask][idBuf]);
				memset(pResLexicon, 0, sizeof(Lexicon_Resource));
				pThis->nCountUsed[idTask][idBuf] = 0;
		}
		pThis->activeLexicon[idTask] = (-1);
	}

	if ( paraLexicon->szExceptPronDictFile ) {
		RES_LEXICON_loadExceptionalPronDict( paraLexicon->szExceptPronDictFile );
	}

	if ( paraLexicon->szUserDictFile ) {
		RES_LEXICON_loadUserDictFile( paraLexicon->szUserDictFile );
	}

	if ( paraLexicon->szUnitDictFile ) {
		RES_LEXICON_loadUnitDictFile( paraLexicon->szUnitDictFile );	
	}

	if ( paraLexicon->szExceptPronDictFile || paraLexicon->szUserDictFile || paraLexicon->szUnitDictFile ) {
		RES_LEXICON_createExceptDictHashTable();
	}

	return bSetup;
}


/**
 *	free memories allocated to the Lexicon resource manager.
 *
 *	@return Return 0 if all memories are freed successfully, otherwise return -1.
 */
HCILAB_PUBLIC HCI_LEXICON_API hci_int32
PowerASR_Resource_Lexicon_closeLexiconResource(PowerASR_Resource_Lexicon *pThis)		///< (i/o) pointer to the Lexicon resource manager
{
	Resource_Lexicon_Inner *pInner = 0;
	Lexicon_Resource *pResLexicon = 0;
	LexiconParameters *paraLexicon = 0;
	hci_uint32 idTask = 0;
	hci_uint32 idBuf = 0;

	if (0 == pThis) {
		return -1;
	}

	pInner = (Resource_Lexicon_Inner *) pThis->pInner;

	for (idTask = 0; idTask < MAX_TASK_COUNT; idTask++) {
		for ( idBuf = 0; idBuf < 2; idBuf++) {
			pResLexicon = &(pThis->resLexicon[idTask][idBuf]);
			_Resource_Lexicon_releaseLexiconResource(pResLexicon);
			pThis->nCountUsed[idTask][idBuf] = 0;
		}
		pThis->activeLexicon[idTask] = (-1);
	}

	paraLexicon = &pInner->paraLexicon;
	if ( paraLexicon->szExceptPronDictFile ) {
		hci_free(paraLexicon->szExceptPronDictFile);
		paraLexicon->szExceptPronDictFile = 0;
	}
	if ( paraLexicon->szUserDictFile ) {
		hci_free(paraLexicon->szUserDictFile);
		paraLexicon->szUserDictFile = 0;
	}
	if ( paraLexicon->szUnitDictFile ) {
		hci_free(paraLexicon->szUnitDictFile);
		paraLexicon->szUnitDictFile = 0;
	}

	RES_LEXICON_freeExceptionalPronDict();

	return 0;
}


/**
 *	build compiled lexicon resource with a given vocabulary file.
 *
 *	@return Return lexicon resource pointer if lexicon resource are built successfully, otherwise return NULL.
 */
HCILAB_PUBLIC HCI_LEXICON_API char*
PowerASR_Resource_Lexicon_buildLexiconResource(PowerASR_Resource_Lexicon *pThis,	///< (i/o) pointer to the Lexicon resource manager
											   const char *szVocabListFile,			///< (i) vocabulary list file
											   const char *szUpdateTime,			///< (i) resource building time
											   const float weightTask)				///< (i) QC weight for current ASR task
{
	Resource_Lexicon_Inner *pInner = 0;
	Lexicon_Resource *pLexRes = 0;
	LexiconParameters *paraLexicon = 0;
	hci_int32 bSetup = 0;
	char *cResData = 0;
	hci_uint32 sizeResData = 0;

	if (0 == pThis) {
		return cResData;
	}
	if ( 0 == szVocabListFile ) {
		return cResData;
	}

	pInner      = (Resource_Lexicon_Inner *) pThis->pInner;
	paraLexicon = &pInner->paraLexicon;

	pLexRes = (Lexicon_Resource *) hci_calloc( 1, sizeof(Lexicon_Resource) );
	if ( !pLexRes ) return 0;

	bSetup = RES_LEXICON_buildCompiledLexiconResourceFromVocabFile( pLexRes, szVocabListFile, szUpdateTime, weightTask );

	if ( bSetup == 0 ) {

		sizeResData  = 10;	// header: size of compiled lexicon resource in bytes
		sizeResData += sizeof(VOCAB_INFO);
		sizeResData += pLexRes->vocab_info.num_vocabs * sizeof(ASR_VOCAB);
		sizeResData += pLexRes->vocab_info.num_lexicons * sizeof(ASR_LEXICON);
		sizeResData += pLexRes->vocab_info.sizePronLex * sizeof(FullSymbolLex);
		sizeResData += pLexRes->vocab_info.sizeVocabStr * sizeof(hci_int8);

		cResData = (char *) hci_calloc( sizeResData + 1, sizeof(char) );

		if ( cResData ) {
			sprintf(cResData, "%10lu", sizeResData);
			sizeResData = 10;
			memcpy( cResData + sizeResData, &pLexRes->vocab_info, sizeof(VOCAB_INFO) );
			sizeResData += sizeof(VOCAB_INFO);
			memcpy( cResData + sizeResData, pLexRes->vocab, pLexRes->vocab_info.num_vocabs * sizeof(ASR_VOCAB) );
			sizeResData += pLexRes->vocab_info.num_vocabs * sizeof(ASR_VOCAB);
			memcpy( cResData + sizeResData, pLexRes->lexicon, pLexRes->vocab_info.num_lexicons * sizeof(ASR_LEXICON) );
			sizeResData += pLexRes->vocab_info.num_lexicons * sizeof(ASR_LEXICON);
			memcpy( cResData + sizeResData, pLexRes->fullSymbolPronLex, pLexRes->vocab_info.sizePronLex * sizeof(FullSymbolLex) );
			sizeResData += pLexRes->vocab_info.sizePronLex * sizeof(FullSymbolLex);
			memcpy( cResData + sizeResData, pLexRes->szVocabListStr, pLexRes->vocab_info.sizeVocabStr * sizeof(hci_int8) );
			sizeResData += pLexRes->vocab_info.sizeVocabStr * sizeof(hci_int8);
		}
	}

	_Resource_Lexicon_releaseLexiconResource(pLexRes);
	hci_free(pLexRes); pLexRes = 0;

	return cResData;
}


/**
 *	build compiled lexicon resource with a given vocabulary file, then update lexical resource of query task index.
 *
 *	@return Return 0 if lexicon resource are updated successfully, otherwise return -1.
 */
HCILAB_PUBLIC HCI_LEXICON_API hci_int32
PowerASR_Resource_Lexicon_updateLexiconResource(PowerASR_Resource_Lexicon *pThis,	///< (i/o) pointer to the Lexicon resource manager
												const char *szVocabListFile,		///< (i) vocabulary list file
												const char *szUpdateTime,			///< (i) resource building time
												const hci_uint32 idTask)			///< (i) ASR task index
{
	Lexicon_Resource *pResLexicon = 0;
	hci_int32 bSetup = 0;
	hci_int32 idSrcBuf = 0;
	hci_int32 idNewBuf = 0;

	if ( 0 == pThis ) return -1;
	if ( 0 == szVocabListFile ) return -1;
	if ( idTask >= MAX_TASK_COUNT ) return -1;

	idSrcBuf = pThis->activeLexicon[idTask];
	idNewBuf = (idSrcBuf + 1) % 2;

	pResLexicon = &(pThis->resLexicon[idTask][idNewBuf]);

	if ( pResLexicon->fullSymbolPronLex || pResLexicon->vocab ) {
		_Resource_Lexicon_releaseLexiconResource(pResLexicon);
		pThis->nCountUsed[idTask][idNewBuf] = 0;
	}

	bSetup = RES_LEXICON_buildCompiledLexiconResourceFromVocabFile( pResLexicon, szVocabListFile, szUpdateTime, 0.0f );

	if ( bSetup == 0 ) {
		int nCountSec = 0;
		int nWaitTime = 10;

		pThis->activeLexicon[idTask] = idNewBuf;

		// more thinking !!!
		while ( idSrcBuf >= 0 && pThis->nCountUsed[idTask][idSrcBuf] && nCountSec++ < nWaitTime ) {
#ifdef WIN32
			SleepEx(1000, TRUE);
#else
			sleep(1);
#endif
		}

		if ( idSrcBuf >= 0 && pThis->nCountUsed[idTask][idSrcBuf] == 0 ) {
			pResLexicon = &(pThis->resLexicon[idTask][idSrcBuf]);
			_Resource_Lexicon_releaseLexiconResource(pResLexicon);
		}
	}

	return bSetup;
}


/**
 *	Load compiled lexicon resource from a given lexicon resource file, then update lexical resource of query task index.
 *
 *	@return Return 0 if lexicon resource are updated successfully, otherwise return -1.
 */
HCILAB_PUBLIC HCI_LEXICON_API hci_int32
PowerASR_Resource_Lexicon_loadLexiconResource(PowerASR_Resource_Lexicon *pThis,	///< (i/o) pointer to the Lexicon resource manager
											  const char *szLexiconResFile,		///< (i) lexicon resource file
											  const hci_uint32 idTask,			///< (i) ASR task index
											  const float weightST)				///< (i) weight for short-term query QC
{
	Lexicon_Resource *pResLexicon = 0;
	VOCAB_INFO *pVocabInfo = 0;
	hci_int32 bSetup = 0;
	hci_int32 idSrcBuf = 0;
	hci_int32 idNewBuf = 0;
	hci_uint32 n = 0;
	FILE *fpRes = 0;
	hci_uint32 t = 0;
	hci_int32 b = 0;
	hci_uint32 main_task = MAX_TASK_COUNT;
	Lexicon_Resource *pRes = 0, *pMainRes = 0;
	double max_QC = 0;
	double max_vsize = 0;
	float meanQC_LT = 0, meanQC_ST = 0;
	float wgtTask = 0;

	if ( 0 == pThis ) return -1;
	if ( 0 == szLexiconResFile ) return -1;
	if ( idTask >= MAX_TASK_COUNT ) return -1;

	idSrcBuf = pThis->activeLexicon[idTask];
	idNewBuf = (idSrcBuf + 1) % 2;

	pResLexicon = &(pThis->resLexicon[idTask][idNewBuf]);

	if ( pResLexicon->fullSymbolPronLex || pResLexicon->vocab ) {
		_Resource_Lexicon_releaseLexiconResource(pResLexicon);
		pThis->nCountUsed[idTask][idNewBuf] = 0;
	}

	fpRes = fopen(szLexiconResFile, "rb");
	if ( 0 == fpRes ) {
		fprintf(stderr, "[ERROR] cannot access query lexicon resource file ... %s\n", szLexiconResFile);
		return -1;
	}

	fread(&pResLexicon->vocab_info, sizeof(VOCAB_INFO), 1, fpRes);

	pVocabInfo = &pResLexicon->vocab_info;
	pResLexicon->vocab = (ASR_VOCAB *) hci_calloc( pVocabInfo->num_vocabs, sizeof(ASR_VOCAB) );
	pResLexicon->lexicon = (ASR_LEXICON *) hci_calloc( pVocabInfo->num_lexicons, sizeof(ASR_LEXICON) );
	pResLexicon->fullSymbolPronLex = (FullSymbolLex *) hci_calloc( pVocabInfo->sizePronLex, sizeof(FullSymbolLex) );
	pResLexicon->szVocabListStr = (hci_int8 *) hci_calloc( pVocabInfo->sizeVocabStr, sizeof(hci_int8) );

	fread(pResLexicon->vocab, sizeof(ASR_VOCAB), pVocabInfo->num_vocabs, fpRes);
	fread(pResLexicon->lexicon, sizeof(ASR_LEXICON), pVocabInfo->num_lexicons, fpRes);
	fread(pResLexicon->fullSymbolPronLex, sizeof(FullSymbolLex), pVocabInfo->sizePronLex, fpRes);
	fread(pResLexicon->szVocabListStr, sizeof(hci_int8), pVocabInfo->sizeVocabStr, fpRes);

	fclose(fpRes);

	if ( pVocabInfo->weightTask < 0.0f ) {	// auto weighting
		for ( t = 0; t < MAX_TASK_COUNT; t++) {
			b = pThis->activeLexicon[t];
			if ( t != idTask && b >= 0 ) {
				pRes = &(pThis->resLexicon[t][b]);
				if ( pRes->vocab_info.totalQC > max_QC ) {
					max_QC = pRes->vocab_info.totalQC;
					max_vsize = (double)pRes->vocab_info.num_vocabs;
					main_task = t;
					pMainRes = pRes;
				}
			}
		}
		if ( pMainRes && pMainRes->vocab_info.totalQC > pVocabInfo->totalQC && weightST > 0.0f ) {
			if ( pVocabInfo->totalQC > 0 ) {
				meanQC_LT = (float)(max_QC/max_vsize);
				meanQC_ST = (float)pVocabInfo->totalQC / (float)pVocabInfo->num_vocabs;
//				wgtTask = weightST * (float)pMainRes->vocab_info.totalQC / (float)pVocabInfo->totalQC;
				wgtTask = weightST * meanQC_LT / meanQC_ST;
				pVocabInfo->weightTask = 100.0f * (float)log((double)wgtTask);
			}
			else {
				pVocabInfo->weightTask = 0.0f;
				wgtTask = 1.0f;
			}
		}
		else {
			pVocabInfo->weightTask = 0.0f;
			wgtTask = 1.0f;
		}
	}
	else {
		for ( t = 0; t < MAX_TASK_COUNT; t++) {
			b = pThis->activeLexicon[t];
			if ( t != idTask && b >= 0 ) {
				pRes = &(pThis->resLexicon[t][b]);
				if ( pRes->vocab_info.totalQC > max_QC ) {
					max_QC = pRes->vocab_info.totalQC;
					max_vsize = (double)pRes->vocab_info.num_vocabs;
					main_task = t;
					pMainRes = pRes;
				}
			}
		}
		if ( pMainRes && pMainRes->vocab_info.totalQC > pVocabInfo->totalQC && weightST > 0.0f ) {
			pVocabInfo->weightTask += 100.0f * (float)log((double)weightST);
		}
		wgtTask = (float)exp((double)pVocabInfo->weightTask * 0.01);
	}

	if ( bSetup == 0 ) {
		int nCountSec = 0;
		int nWaitTime = 10;

		if ( pThis->activeLexicon[idTask] >= 0 ) {
			fprintf(stdout, "------- Switching Query [%d] -------\n", idTask);
		}
		else {
			fprintf(stdout, "------- Inserting Query [%d] -------\n", idTask);
		}
		fprintf(stdout, "Query Size      = %lu\n", pVocabInfo->num_vocabs);
		fprintf(stdout, "Total QC        = %e\n", pVocabInfo->totalQC);
		fprintf(stdout, "Task QC Weight  = %.2f\n", wgtTask);
		fprintf(stdout, "------------------------------------\n");
		fflush(stdout);

		pThis->activeLexicon[idTask] = idNewBuf;

		// more thinking !!!
		while ( idSrcBuf >= 0 && pThis->nCountUsed[idTask][idSrcBuf] && nCountSec++ < nWaitTime ) {
#ifdef WIN32
			SleepEx(1000, TRUE);
#else
			sleep(1);
#endif
		}

		if ( idSrcBuf >= 0 && pThis->nCountUsed[idTask][idSrcBuf] == 0 ) {
			pResLexicon = &(pThis->resLexicon[idTask][idSrcBuf]);
			_Resource_Lexicon_releaseLexiconResource(pResLexicon);
		}
	}

	return bSetup;
}


/**
 *	initialize user-specific data buffers for Lexicon resource manager.
 *
 *	@return Return 0 if user-specific data buffers are initialized successfully, otherwise return -1.
 */
HCILAB_PUBLIC HCI_LEXICON_API hci_int32
PowerASR_Resource_Lexicon_initializeUserLexiconData(PowerASR_Resource_Lexicon *pThis,	///< (i) pointer to the Lexicon resource manager
													UserLexicon *pUserLexicon,			///< (o) user-specific lexical data
													const hci_uint32 idTask)			///< (i) recognition task index
{
	if (0 == pUserLexicon) {
		return -1;
	}

	memset(pUserLexicon, 0, sizeof(UserLexicon));

	pUserLexicon->idTask = idTask;

	return 0;
}


/**
 *	release user-specific data buffers for Lexicon resource manager.
 *
 *	@return Return 0 if user-specific data buffers are released successfully, otherwise return -1.
 */
HCILAB_PUBLIC HCI_LEXICON_API hci_int32
PowerASR_Resource_Lexicon_releaseUserLexiconData(PowerASR_Resource_Lexicon *pThis,	///< (i) pointer to the Lexicon resource manager
												 UserLexicon *pUserLexicon)			///< (o) user-specific lexical data
{
	return 0;
}


/**
 *	reset compiled lexicon resource of query ASR task
 *
 *	@return return 0 if a compiled lexicon resource was reset successfully, otherwise return -1.
 */
HCILAB_PUBLIC HCI_LEXICON_API hci_int32
PowerASR_Resource_Lexicon_resetLexicalData(PowerASR_Resource_Lexicon *pThis,	///< (i) pointer to the Lexicon resource manager
										   const hci_uint32 idTask)				///< (i) query recognition task index
{
	Lexicon_Resource *pResLexicon = 0;
	hci_int32 idSrcBuf = 0;
	hci_int32 idNewBuf = 0;
	
	if (0 == pThis) {
		return -1;
	}
	if ( idTask >= MAX_TASK_COUNT ) {
		return -1;
	}

	idSrcBuf = pThis->activeLexicon[idTask];
	idNewBuf = (idSrcBuf + 1) % 2;

	pResLexicon = &(pThis->resLexicon[idTask][idNewBuf]);

	if ( pResLexicon->fullSymbolPronLex || pResLexicon->vocab ) {
		_Resource_Lexicon_releaseLexiconResource(pResLexicon);
		pThis->nCountUsed[idTask][idNewBuf] = 0;
	}

	memset( pResLexicon, 0, sizeof(Lexicon_Resource) );
	pResLexicon->nLexiDataSize = 0;
	pResLexicon->nRecvDataSize = 0;
	
	return 0;
}


/**
 *	add compiled lexicon resource of query ASR task
 *
 *	@return return 0 if query lexical data was added successfully, otherwise return -1.
 */
HCILAB_PUBLIC HCI_LEXICON_API hci_int32
PowerASR_Resource_Lexicon_addLexicalData(PowerASR_Resource_Lexicon *pThis,	///< (i) pointer to the Lexicon resource manager
										 const void* LexData,				///< (i) added lexical data
										 const hci_uint32 nSizeLexData,		///< (i) size of added lexical data in bytes
										 const hci_uint32 idTask)			///< (i) query recognition task index
{
	Lexicon_Resource *pResLexicon = 0;
	VOCAB_INFO *pVocabInfo = 0;
	hci_int32 idSrcBuf = 0;
	hci_int32 idNewBuf = 0;
	hci_uint32 nSizeCopy = 0;
	hci_uint32 nRecvSize = 0;
	hci_uint32 nCopyPtr = 0;
	hci_uint32 nSizeTh1 = 0, nSizeTh2 = 0;
	const char* pSrcData = 0;
	char* pOutData = 0;
	
	if (0 == pThis) {
		return -1;
	}
	if ( idTask >= MAX_TASK_COUNT ) {
		return -1;
	}

	if ( !LexData || !nSizeLexData ) return 0;

	idSrcBuf = pThis->activeLexicon[idTask];
	idNewBuf = (idSrcBuf + 1) % 2;

	pResLexicon = &(pThis->resLexicon[idTask][idNewBuf]);
	pVocabInfo  = &pResLexicon->vocab_info;

	if ( pThis->nCountUsed[idTask][idNewBuf] ) return -1;

	pSrcData  = (const char *)LexData;
	nRecvSize = nSizeLexData;

	if ( pResLexicon->nRecvDataSize == 0 )
	{	// header data
		nCopyPtr = 0;
		if ( nSizeLexData >= sizeof(VOCAB_INFO) )
		{
			memcpy( pVocabInfo, pSrcData, sizeof(VOCAB_INFO) );

			pResLexicon->nLexiDataSize = _Resource_Lexicon_allocateLexiconResource( pResLexicon );

			nRecvSize -= sizeof(VOCAB_INFO);
			pSrcData  += sizeof(VOCAB_INFO);

			if ( nRecvSize ) {
				nSizeCopy = _Resource_Lexicon_copyAsrVocabulary( pResLexicon, pSrcData, nRecvSize, nCopyPtr );
				nRecvSize -= nSizeCopy;
				pSrcData  += nSizeCopy;
			}

			if ( nRecvSize ) {
				nSizeCopy = _Resource_Lexicon_copyAsrLexicon( pResLexicon, pSrcData, nRecvSize, nCopyPtr );
				nRecvSize -= nSizeCopy;
				pSrcData  += nSizeCopy;
			}

			if ( nRecvSize ) {
				nSizeCopy = _Resource_Lexicon_copyFullSymbolLexicon( pResLexicon, pSrcData, nRecvSize, nCopyPtr );
				nRecvSize -= nSizeCopy;
				pSrcData  += nSizeCopy;
			}

			if ( nRecvSize ) {
				nSizeCopy = _Resource_Lexicon_copyVocabularyListString( pResLexicon, pSrcData, nRecvSize, nCopyPtr );
				nRecvSize -= nSizeCopy;
				pSrcData  += nSizeCopy;
			}

		}
		else if ( nSizeLexData >= 16U )
		{
			memcpy( pVocabInfo, pSrcData, nSizeLexData );
			pResLexicon->nLexiDataSize = _Resource_Lexicon_allocateLexiconResource( pResLexicon );
		}
		else
		{
			memcpy( pVocabInfo, pSrcData, nSizeLexData );
		}

		pResLexicon->nRecvDataSize = nSizeLexData;
	}
	else
	{
		nSizeTh1 = 0;
		nSizeTh2 = sizeof(VOCAB_INFO);
		if ( pResLexicon->nRecvDataSize < nSizeTh2 ) {
			nCopyPtr  = pResLexicon->nRecvDataSize - nSizeTh1;
			nSizeCopy = nSizeTh2 - nCopyPtr;
			pOutData  = (char *)pVocabInfo;
			memcpy(pOutData + nCopyPtr, pSrcData, nSizeCopy);
			if (pResLexicon->nLexiDataSize == 0) {
				pResLexicon->nLexiDataSize = _Resource_Lexicon_allocateLexiconResource( pResLexicon );
			}
			pSrcData  += nSizeCopy;
			nRecvSize -= nSizeCopy;
			pResLexicon->nRecvDataSize += nSizeCopy;
		}

		nSizeTh1 = nSizeTh2;
		nSizeTh2 += pVocabInfo->num_vocabs * sizeof(ASR_VOCAB);
		if ( nRecvSize > 0 && pResLexicon->nRecvDataSize >= nSizeTh1 && pResLexicon->nRecvDataSize < nSizeTh2 ) {
			nCopyPtr  = pResLexicon->nRecvDataSize - nSizeTh1;
			nSizeCopy = _Resource_Lexicon_copyAsrVocabulary( pResLexicon, pSrcData, nRecvSize, nCopyPtr );
			nRecvSize -= nSizeCopy;
			pSrcData  += nSizeCopy;
			pResLexicon->nRecvDataSize += nSizeCopy;
		}
		
		nSizeTh1 = nSizeTh2;
		nSizeTh2 += pVocabInfo->num_lexicons * sizeof(ASR_LEXICON);
		if ( nRecvSize > 0 && pResLexicon->nRecvDataSize >= nSizeTh1 && pResLexicon->nRecvDataSize < nSizeTh2 ) {
			nCopyPtr  = pResLexicon->nRecvDataSize - nSizeTh1;
			nSizeCopy = _Resource_Lexicon_copyAsrLexicon( pResLexicon, pSrcData, nRecvSize, nCopyPtr );
			nRecvSize -= nSizeCopy;
			pSrcData  += nSizeCopy;
			pResLexicon->nRecvDataSize += nSizeCopy;
		}
		
		nSizeTh1 = nSizeTh2;
		nSizeTh2 += pVocabInfo->sizePronLex * sizeof(FullSymbolLex);
		if ( nRecvSize > 0 && pResLexicon->nRecvDataSize >= nSizeTh1 && pResLexicon->nRecvDataSize < nSizeTh2 ) {
			nCopyPtr  = pResLexicon->nRecvDataSize - nSizeTh1;
			nSizeCopy = _Resource_Lexicon_copyFullSymbolLexicon( pResLexicon, pSrcData, nRecvSize, nCopyPtr );
			nRecvSize -= nSizeCopy;
			pSrcData  += nSizeCopy;
			pResLexicon->nRecvDataSize += nSizeCopy;
		}
		
		nSizeTh1 = nSizeTh2;
		nSizeTh2 += pVocabInfo->sizeVocabStr * sizeof(hci_int8);
		if ( nRecvSize > 0 && pResLexicon->nRecvDataSize >= nSizeTh1 && pResLexicon->nRecvDataSize < nSizeTh2 ) {
			nCopyPtr  = pResLexicon->nRecvDataSize - nSizeTh1;
			nSizeCopy = _Resource_Lexicon_copyVocabularyListString( pResLexicon, pSrcData, nRecvSize, nCopyPtr );
			nRecvSize -= nSizeCopy;
			pSrcData  += nSizeCopy;
			pResLexicon->nRecvDataSize += nSizeCopy;
		}

	}

	return 0;
}


/**
 *	switch compiled lexicon resource of query ASR task
 *
 *	@return return 0 if a compiled lexicon resource was switched successfully, otherwise return -1.
 */
HCILAB_PUBLIC HCI_LEXICON_API hci_int32
PowerASR_Resource_Lexicon_switchLexicalData(PowerASR_Resource_Lexicon *pThis,	///< (i) pointer to the Lexicon resource manager
											const hci_uint32 idTask,			///< (i) query recognition task index
											const float weightST)				///< (i) weight for short-term query QC
{
	Lexicon_Resource *pResLexicon = 0;
	VOCAB_INFO *pVocabInfo = 0;
	hci_int32 idSrcBuf = 0;
	hci_int32 idNewBuf = 0;
	int nCountSec = 0;
	int nWaitTime = 10;

	if (0 == pThis) {
		return -1;
	}
	if ( idTask >= MAX_TASK_COUNT ) {
		return -1;
	}

	idSrcBuf = pThis->activeLexicon[idTask];
	idNewBuf = (idSrcBuf + 1) % 2;

	pResLexicon = &(pThis->resLexicon[idTask][idNewBuf]);
	pVocabInfo  = &pResLexicon->vocab_info;

	if ( pResLexicon->nLexiDataSize && pResLexicon->nRecvDataSize != pResLexicon->nLexiDataSize ) {
		return -1;
	}
	
	if ( pVocabInfo->weightTask < 0.0f ) {	// auto weighting
		hci_uint32 t = 0, b = 0;
		hci_uint32 main_task = MAX_TASK_COUNT;
		Lexicon_Resource *pRes = 0, *pMainRes = 0;
		double max_QC = 0;
		float wgtTask = 0;
		for ( t = 0; t < MAX_TASK_COUNT; t++) {
			if ( t != idTask ) {
			b = pThis->activeLexicon[t];
				pRes = &(pThis->resLexicon[t][b]);
				if ( pRes->vocab_info.totalQC > max_QC ) {
					max_QC = pRes->vocab_info.totalQC;
					main_task = t;
					pMainRes = pRes;
				}
			}
		}
		if ( pMainRes && pMainRes->vocab_info.totalQC > pVocabInfo->totalQC && weightST > 0.0f ) {
			wgtTask = weightST * (float)pMainRes->vocab_info.totalQC / (float)pVocabInfo->totalQC;
			pVocabInfo->weightTask = 100.0f * (float)log((double)wgtTask);
		}
		else {
			pVocabInfo->weightTask = 0.0f;
		}
	}

	pThis->activeLexicon[idTask] = idNewBuf;
#ifdef USE_BE_THREAD
	// more thinking !!!
	while ( idSrcBuf >= 0 && pThis->nCountUsed[idTask][idSrcBuf] && nCountSec++ < nWaitTime ) {
#ifdef WIN32
		SleepEx(1000, TRUE);
#else
		sleep(1);
#endif
	}
#endif
	if ( idSrcBuf >= 0 && pThis->nCountUsed[idTask][idSrcBuf] == 0 ) {
		pResLexicon = &(pThis->resLexicon[idTask][idSrcBuf]);
		_Resource_Lexicon_releaseLexiconResource(pResLexicon);
	}

	return 0;
}


/**
 *	return vocabulary data corresponding to the query vocabulary lexicon index.
 *
 *	@return Return query vocabulary index.
 */
HCILAB_PUBLIC HCI_LEXICON_API hci_uint32
PowerASR_Resource_Lexicon_getVocabularyData(PowerASR_Resource_Lexicon *pThis,	///< (i) pointer to the Lexicon resource manager
											Lexicon_Resource *pResLexicon,		///< (i) pointer to lexicon resource
											const hci_uint32 idLexicon,			///< (i) vocabulary lexicon index
											char *szRecogWrod)					///< (o) recognized word string
{
	VOCAB_INFO *pVocabInfo = 0;
	hci_uint32 queryVocabID = 0;
	char szOutSymbol[256];

	if ( szRecogWrod ) {
		*szRecogWrod = '\0';
	}

	if (0 == pThis) {
		return queryVocabID;
	}
	if (0 == pResLexicon) {
		return queryVocabID;
	}
	if (0 == szRecogWrod) {
		return queryVocabID;
	}

	pVocabInfo  = &pResLexicon->vocab_info;

	if ( pResLexicon->lexicon && pResLexicon->vocab && pVocabInfo->sizeVocabStr ) {
		ASR_VOCAB *pVocab = 0;
		queryVocabID = pResLexicon->lexicon[idLexicon].idVocab;
		pVocab       = pResLexicon->vocab + queryVocabID;

		if ( pVocab->nLenVocabStr ) {
			_Resource_Lexicon_getWordString(szRecogWrod,
											szOutSymbol,
											pResLexicon->szVocabListStr + pVocab->nPtrVocabStr,
											pVocab->nLenVocabStr);
		}
		else {
			strcpy(szRecogWrod, "<UNK>");
		}
	}

	return queryVocabID;
}

/**
 *	Get recognition vocabulary size of a given task.
 * 
 *	@return return the number of recognition vocabularies
 */
HCILAB_PUBLIC HCI_LEXICON_API hci_uint32
PowerASR_Resource_Lexicon_getVocabularySize(PowerASR_Resource_Lexicon *pThis,	///< (i) pointer to the Lexicon resource manager
											const hci_uint32 idTask)			///< (i) recognition task index
{
	Resource_Lexicon_Inner *pInner = 0;
	Lexicon_Resource *pResLexicon = 0;
	LexiconParameters *paraLexicon = 0;
	VOCAB_INFO *pVocabInfo = 0;
	hci_int32 idBuf = 0;

	if (0 == pThis) {
		return 0U;
	}
	if (idTask >= MAX_TASK_COUNT) {
		return 0U;
	}

	pInner = (Resource_Lexicon_Inner *) pThis->pInner;
	paraLexicon = &pInner->paraLexicon;

	idBuf = pThis->activeLexicon[idTask];
	if ( idBuf < 0 ) {
		return 0U;
	}

	pResLexicon = &(pThis->resLexicon[idTask][idBuf]);
	pVocabInfo  = &pResLexicon->vocab_info;

	return pVocabInfo->num_vocabs;
}


/**
 *	return string of query vocabulary index.
 *
 *	@return Return 0 if query vocabulary string is returned successfully, otherwise return -1.
 */
HCILAB_PUBLIC HCI_LEXICON_API hci_int32
PowerASR_Resource_Lexicon_getVocabularyString(PowerASR_Resource_Lexicon *pThis,	///< (i) pointer to the Lexicon resource manager
											  const hci_uint32 idTask,			///< (i) recognition task index
											  const hci_uint32 idVocab,			///< (i) query vocabulary index
											  char *szVocabName)				///< (o) vocabulary string
{
	Resource_Lexicon_Inner *pInner = 0;
	Lexicon_Resource *pResLexicon = 0;
	LexiconParameters *paraLexicon = 0;
	VOCAB_INFO *pVocabInfo = 0;
	char szOutSymbol[512];
	hci_int32 idBuf = 0;

	if (szVocabName) *szVocabName = '\0';

	if (0 == pThis) {
		return -1;
	}
	if (idTask >= MAX_TASK_COUNT) {
		return -1;
	}
	if (0 == szVocabName) {
		return -1;
	}
	
	idBuf = pThis->activeLexicon[idTask];
	if ( idBuf < 0 ) {
		return -1;
	}

	pInner = (Resource_Lexicon_Inner *) pThis->pInner;
	
	paraLexicon = &pInner->paraLexicon;

	pResLexicon = &(pThis->resLexicon[idTask][idBuf]);
	pVocabInfo  = &pResLexicon->vocab_info;

	memset(szOutSymbol, 0, sizeof(szOutSymbol));

	if ( pVocabInfo->num_vocabs == 0U )
	{
		return -1;
	}

	if ( pVocabInfo->sizeVocabStr && pResLexicon->szVocabListStr ) {
		ASR_VOCAB *pVocab = 0;
		pVocab = pResLexicon->vocab + idVocab;
		if ( pVocab->nLenVocabStr ) {
			_Resource_Lexicon_getWordString(szVocabName,
											szOutSymbol,
											pResLexicon->szVocabListStr + pVocab->nPtrVocabStr,
											pVocab->nLenVocabStr);
		}
	}
	else {
		return -1;
	}

	return 0;
}


/**
 *	get lexicon data to be processed.
 *
 *	@return Return one of the following values:
 *		- GET_LEX_FAIL if lexicon resource file cannot be accessed.
 *		- GET_LEX_SUCCESS if lexicon resource data was loaded, but all data was not loaded.
 *		- GET_LEX_COMPLETE if all lexicon data was loaded.
 */
HCILAB_PUBLIC HCI_LEXICON_API LexLoadType
PowerASR_Resource_Lexicon_getProcessingLexicons(PowerASR_Resource_Lexicon *pThis,	///< (i) pointer to the Lexicon resource manager
												UserLexicon *pUserLexicon,			///< (o) user-specific lexical data
												const hci_uint32 idTask)			///< (i) recognition task index
{
	Resource_Lexicon_Inner *pInner = 0;
	Lexicon_Resource *pResLexicon = 0;
	VOCAB_INFO *pVocabInfo = 0;

	if (0 == pThis) {
		return GET_LEX_FAIL;
	}
	if (0 == pUserLexicon) {
		return GET_LEX_FAIL;
	}
	if (0 == pUserLexicon->pResLex) {
		return GET_LEX_FAIL;
	}

	pInner = (Resource_Lexicon_Inner *) pThis->pInner;

	pResLexicon = pUserLexicon->pResLex;
	pVocabInfo  = &pResLexicon->vocab_info;

	if (pVocabInfo->num_lexicons == 0) {
		pUserLexicon->fullSymbolPronLex = 0;
		pUserLexicon->partialSymbolPronLex = 0;
		pUserLexicon->basePronLex = 0;
	}
	else {
		pUserLexicon->fullSymbolPronLex = pResLexicon->fullSymbolPronLex;
		pUserLexicon->partialSymbolPronLex = pResLexicon->partialSymbolPronLex;
		pUserLexicon->basePronLex = pResLexicon->basePronLex;
	}

	return GET_LEX_COMPLETE;
}


/**
 *	get the number of recognition tasks.
 *
 *	@return Return the number of recognition tasks.
 */
HCILAB_PUBLIC HCI_LEXICON_API hci_uint32
PowerASR_Resource_Lexicon_getTaskCount(PowerASR_Resource_Lexicon *pThis)	///< (i) pointer to the Lexicon resource manager
{
	int iTask = 0;
	hci_uint32 nCountActTask = 0;

	if (0 == pThis) {
		return nCountActTask;
	}

	for ( iTask = 0 ; iTask < MAX_TASK_COUNT ; iTask++) {
		if ( pThis->activeLexicon[iTask] == 0 || pThis->activeLexicon[iTask] == 1 ) {
			nCountActTask++;
		}
	}

	return nCountActTask;
}


/**
 *  release memories allocated to lexicon resource
 */
HCILAB_PRIVATE hci_int32
_Resource_Lexicon_releaseLexiconResource(Lexicon_Resource *pResLexicon)		///< (i/o) lexicon resource
{
	if (0 == pResLexicon) {
		return -1;
	}

	if (pResLexicon->vocab) {
		hci_free(pResLexicon->vocab);
		pResLexicon->vocab = 0;
	}

	if (pResLexicon->lexicon) {
		hci_free(pResLexicon->lexicon);
		pResLexicon->lexicon = 0;
	}

	if (pResLexicon->fullSymbolPronLex) {
		hci_free(pResLexicon->fullSymbolPronLex);
		pResLexicon->fullSymbolPronLex = 0;
	}

	if (pResLexicon->partialSymbolPronLex) {
		hci_free(pResLexicon->partialSymbolPronLex);
		pResLexicon->partialSymbolPronLex = 0;
	}

	if (pResLexicon->basePronLex) {
		hci_free(pResLexicon->basePronLex);
		pResLexicon->basePronLex = 0;
	}

	if (pResLexicon->szVocabListStr) {
		hci_free(pResLexicon->szVocabListStr);
		pResLexicon->szVocabListStr = 0;
	}

	return 0;
}


/**
 * extract recognized vocabulary string and symbol string
 */
HCILAB_PRIVATE hci_int32
_Resource_Lexicon_getWordString(char *szWordName,			///< (o) recognized vocabulary string
								char *szWordSymbol,			///< (o) recognized symbol string
								const char *pSrcVocab,		///< (i) source vocabulary string (name + symbol)
								const hci_uint32 nLenVocab)	///< (i) length of source vocabulary string
{
	char *pComma = 0;
	char *pSeparator = 0;

	*szWordName = '\0';
	*szWordSymbol = '\0';

	if ( 0 == nLenVocab ) return 0;

	if (*pSrcVocab == '[') {
		strncpy(szWordName, pSrcVocab+1, nLenVocab-2);
		szWordName[nLenVocab-2] = '\0';
// 		pComma = strchr(szWordName, ',');
// 		if (pComma) {
// 			strcpy(szWordSymbol, pComma+1);
// 			*pComma = '\0';
// 		}
// 		else {
			strcpy(szWordSymbol, szWordName);
//		}
	}
	else if (strchr(pSrcVocab, ';')) {
		strncpy(szWordName, pSrcVocab, nLenVocab-1);
		szWordName[nLenVocab-1] = '\0';
// 		pComma = strchr(szWordName, ',');
// 		if (pComma) {
// 			strcpy(szWordSymbol, pComma+1);
// 			*pComma = '\0';
// 		}
// 		else {
			strcpy(szWordSymbol, szWordName);
//		}
	}
	else {
		strncpy(szWordName, pSrcVocab, nLenVocab);
		szWordName[nLenVocab] = '\0';
	}

	return 0;
}


/**
 *	allocate memory space of compiled lexicon resource
 */
HCILAB_PRIVATE hci_uint32
_Resource_Lexicon_allocateLexiconResource(Lexicon_Resource *pResLexicon)
{
	VOCAB_INFO *pVocabInfo = 0;
	hci_uint32 nLexiDataSize = 0;

	if ( 0 == pResLexicon ) return nLexiDataSize;

	pVocabInfo = &pResLexicon->vocab_info;

	nLexiDataSize  = sizeof(VOCAB_INFO);
	nLexiDataSize += pVocabInfo->num_vocabs * sizeof(ASR_VOCAB);
	nLexiDataSize += pVocabInfo->num_lexicons * sizeof(ASR_LEXICON);
	nLexiDataSize += pVocabInfo->sizePronLex * sizeof(FullSymbolLex);
	nLexiDataSize += pVocabInfo->sizeVocabStr * sizeof(hci_int8);

	if ( pVocabInfo->num_vocabs ) {
		pResLexicon->vocab = (ASR_VOCAB *) hci_calloc( pVocabInfo->num_vocabs, sizeof(ASR_VOCAB) );
	}
	else {
		pResLexicon->vocab = 0;
	}
	if ( pVocabInfo->num_lexicons ) {
		pResLexicon->lexicon = (ASR_LEXICON *) hci_calloc( pVocabInfo->num_lexicons, sizeof(ASR_LEXICON) );
	}
	else {
		pResLexicon->lexicon = 0;
	}
	if ( pVocabInfo->sizePronLex ) {
		pResLexicon->fullSymbolPronLex = (FullSymbolLex *) hci_calloc( pVocabInfo->sizePronLex, sizeof(FullSymbolLex) );
	}
	else {
		pResLexicon->fullSymbolPronLex = 0;
	}
	if ( pVocabInfo->sizeVocabStr ) {
		pResLexicon->szVocabListStr = (hci_int8 *) hci_calloc( pVocabInfo->sizeVocabStr, sizeof(hci_int8) );
	}
	else {
		pResLexicon->szVocabListStr = 0;
	}
	pResLexicon->basePronLex = 0;
	pResLexicon->partialSymbolPronLex = 0;

	return nLexiDataSize;
}


/**
 *	copy ASR vocabulary resource
 */
HCILAB_PRIVATE hci_uint32
_Resource_Lexicon_copyAsrVocabulary(Lexicon_Resource *pResLexicon,
									const char* pSrcData,
									const hci_uint32 nDataSize,
									const hci_uint32 nCopyPtr)
{
	VOCAB_INFO *pVocabInfo = 0;
	hci_uint32 nCopySize = 0;
	hci_uint32 nMemSize = 0;
	char *pOutData = 0;

	if ( 0 == pResLexicon ) return nCopySize;
	if ( 0 == pSrcData || 0 == nDataSize ) return nCopySize;

	pVocabInfo = &pResLexicon->vocab_info;

	nMemSize = pVocabInfo->num_vocabs * sizeof(ASR_VOCAB);
	if ( nCopyPtr >= nMemSize ) return nCopySize;

	pOutData  = (char *) pResLexicon->vocab;
	pOutData += nCopyPtr;

	if ( nDataSize > (nMemSize - nCopyPtr) ) {
		nCopySize = nMemSize - nCopyPtr;
	}
	else {
		nCopySize = nDataSize;
	}
	memcpy( pOutData, pSrcData, nCopySize );

	return nCopySize;
}


/**
 *	copy ASR lexicon list resource
 */
HCILAB_PRIVATE hci_uint32
_Resource_Lexicon_copyAsrLexicon(Lexicon_Resource *pResLexicon,
								 const char* pSrcData,
								 const hci_uint32 nDataSize,
								 const hci_uint32 nCopyPtr)
{
	VOCAB_INFO *pVocabInfo = 0;
	hci_uint32 nCopySize = 0;
	hci_uint32 nMemSize = 0;
	char *pOutData = 0;

	if ( 0 == pResLexicon ) return nCopySize;
	if ( 0 == pSrcData || 0 == nDataSize ) return nCopySize;

	pVocabInfo = &pResLexicon->vocab_info;
	
	nMemSize = pVocabInfo->num_lexicons * sizeof(ASR_LEXICON);
	if ( nCopyPtr >= nMemSize ) return nCopySize;

	pOutData  = (char *) pResLexicon->lexicon;
	pOutData += nCopyPtr;
	
	if ( nDataSize > (nMemSize - nCopyPtr) ) {
		nCopySize = nMemSize - nCopyPtr;
	}
	else {
		nCopySize = nDataSize;
	}
	memcpy( pOutData, pSrcData, nCopySize );

	return nCopySize;
}


/**
 *	copy full symbol lexicon resource
 */
HCILAB_PRIVATE hci_uint32
_Resource_Lexicon_copyFullSymbolLexicon(Lexicon_Resource *pResLexicon,
										const char* pSrcData,
										const hci_uint32 nDataSize,
										const hci_uint32 nCopyPtr)
{
	VOCAB_INFO *pVocabInfo = 0;
	hci_uint32 nCopySize = 0;
	hci_uint32 nMemSize = 0;
	char *pOutData = 0;

	if ( 0 == pResLexicon ) return nCopySize;
	if ( 0 == pSrcData || 0 == nDataSize ) return nCopySize;

	pVocabInfo = &pResLexicon->vocab_info;
	
	nMemSize = pVocabInfo->sizePronLex * sizeof(FullSymbolLex);
	if ( nCopyPtr >= nMemSize ) return nCopySize;

	pOutData  = (char *) pResLexicon->fullSymbolPronLex;
	pOutData += nCopyPtr;
	
	if ( nDataSize > (nMemSize - nCopyPtr) ) {
		nCopySize = nMemSize - nCopyPtr;
	}
	else {
		nCopySize = nDataSize;
	}
	memcpy( pOutData, pSrcData, nCopySize );

	return nCopySize;
}


/**
 *	copy vocabulary list string resource
 */
HCILAB_PRIVATE hci_uint32
_Resource_Lexicon_copyVocabularyListString(Lexicon_Resource *pResLexicon,
										   const char* pSrcData,
										   const hci_uint32 nDataSize,
										   const hci_uint32 nCopyPtr)
{
	VOCAB_INFO *pVocabInfo = 0;
	hci_uint32 nCopySize = 0;
	hci_uint32 nMemSize = 0;
	char *pOutData = 0;

	if ( 0 == pResLexicon ) return nCopySize;
	if ( 0 == pSrcData || 0 == nDataSize ) return nCopySize;

	pVocabInfo = &pResLexicon->vocab_info;
	
	nMemSize = pVocabInfo->sizeVocabStr * sizeof(hci_int8);
	if ( nCopyPtr >= nMemSize ) return nCopySize;

	pOutData  = (char *) pResLexicon->szVocabListStr;
	pOutData += nCopyPtr;
	
	if ( nDataSize > (nMemSize - nCopyPtr) ) {
		nCopySize = nMemSize - nCopyPtr;
	}
	else {
		nCopySize = nDataSize;
	}
	memcpy( pOutData, pSrcData, nCopySize );

	return nCopySize;
}


/**
 * setup default environments for Lexicon resource manager
 */
HCILAB_PRIVATE hci_int32
_Resource_Lexicon_defaultConfigurations(PowerASR_Resource_Lexicon *pThis)
{
	Resource_Lexicon_Inner *pInner = 0;
	LexiconParameters *paraLexicon = 0;

	if (0 == pThis) {
		return -1;
	}

	pInner = (Resource_Lexicon_Inner *) pThis->pInner;
	paraLexicon = &pInner->paraLexicon;

	memset(paraLexicon, 0, sizeof(LexiconParameters));

	return 0;
}


/**
 * setup environments from a given configuration file
 */
HCILAB_PRIVATE hci_int32
_Resource_Lexicon_loadConfigurations(PowerASR_Resource_Lexicon *pThis,
									 const char *pszHomeDir,
									 const char *pszConfigFile)
{
	Resource_Lexicon_Inner *pInner = 0;
	LexiconParameters *paraLexicon = 0;
	char *pszValue = 0;

	if (0 == pThis) {
		return -1;
	}

	pInner = (Resource_Lexicon_Inner *) pThis->pInner;
	paraLexicon = &pInner->paraLexicon;

	if (0 != PowerASR_Base_parseConfigFile(pszConfigFile)) {
		HCIMSG_ERROR("parseConfigFile failed (%s).\n", pszConfigFile);
		return -1;
	}

	// exceptional pronunciation dictionary file
	paraLexicon->szExceptPronDictFile = 0;
	pszValue = PowerASR_Base_getArgumentValue("EXCEPT_PRON_DICT_FILE");
	if (pszValue) {
		paraLexicon->szExceptPronDictFile = hci_salloc(pszValue);
	}

	// user-defined dictionary file
	paraLexicon->szUserDictFile = 0;
	pszValue = PowerASR_Base_getArgumentValue("USER_DICT_FILE");
	if (pszValue) {
		paraLexicon->szUserDictFile = hci_salloc(pszValue);
	}
	
	// English unit dictionary file
	paraLexicon->szUnitDictFile = 0;
	pszValue = PowerASR_Base_getArgumentValue("EN_UNIT_DICT_FILE");
	if (pszValue) {
		paraLexicon->szUnitDictFile = hci_salloc(pszValue);
	}

	PowerASR_Base_closeConfigurations();

	return 0;
}

// end of file
