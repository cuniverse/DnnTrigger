
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
 *	@file	hci_resource_lexicon.h
 *	@ingroup lexicon_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	PowerASR vocabulary lexicon resource manager library
 */

#ifndef __HCILAB_RESOURCE_LEXICON_H__
#define __HCILAB_RESOURCE_LEXICON_H__

#include "base/hci_type.h"
#include "lexicon/lexicon_common.h"

#if defined(HCI_MSC_32)
#ifdef HCI_RESOURCE_LEXICON_EXPORTS
#define HCI_LEXICON_API __declspec(dllexport)
#elif defined(HCI_RESOURCE_LEXICON_IMPORTS)
#define HCI_LEXICON_API __declspec(dllimport)
#else	// in case of static library
#define HCI_LEXICON_API
#endif // #ifdef HCI_RESOURCE_LEXICON_EXPORTS
#elif defined(HCI_OS2)
#define HCI_LEXICON_API
#else
#define HCI_LEXICON_API HCI_USER
#endif

/** PowerASR Vocabulary Lexicon resource manager */
typedef struct {
	Lexicon_Resource resLexicon[MAX_TASK_COUNT][2];	///< lexicon resource set
	hci_int32 activeLexicon[MAX_TASK_COUNT];		///< active lexicon buffer index (-1 = non-active)
	hci_int32 nCountUsed[MAX_TASK_COUNT][2];		///< count of channels which use a query resource
	void *pInner;
} PowerASR_Resource_Lexicon;

#ifdef __cplusplus
extern "C" {
#endif

/**
 *	create a new Lexicon resource manager.
 *
 *	@return Return the pointer to a newly created Lexicon resource manager
 */
HCILAB_PUBLIC HCI_LEXICON_API PowerASR_Resource_Lexicon*
PowerASR_Resource_Lexicon_new(
);

/**
 *	delete the Lexicon resource manager.
 */
HCILAB_PUBLIC HCI_LEXICON_API void
PowerASR_Resource_Lexicon_delete(PowerASR_Resource_Lexicon *pThis
);

/**
 *	load lexical model resource.
 *
 *	@return Return 0 if lexical model resource are loaded successfully, otherwise return -1.
 */
HCILAB_PUBLIC HCI_LEXICON_API hci_int32
PowerASR_Resource_Lexicon_openLexiconResource(PowerASR_Resource_Lexicon *pThis,		///< (i/o) pointer to the Lexicon resource manager
											  const char *pszHomeDir,				///< (i) working directory name
											  const char *pszConfigFile				///< (i) configuration file
);

/**
 *	free memories allocated to the Lexicon resource manager.
 *
 *	@return Return 0 if all memories are freed successfully, otherwise return -1.
 */
HCILAB_PUBLIC HCI_LEXICON_API hci_int32
PowerASR_Resource_Lexicon_closeLexiconResource(PowerASR_Resource_Lexicon *pThis		///< (i/o) pointer to the Lexicon resource manager
);

/**
 *	build compiled lexicon resource with a given vocabulary file.
 *
 *	@return Return lexicon resource pointer if lexicon resource are built successfully, otherwise return NULL.
 */
HCILAB_PUBLIC HCI_LEXICON_API char*
PowerASR_Resource_Lexicon_buildLexiconResource(PowerASR_Resource_Lexicon *pThis,	///< (i/o) pointer to the Lexicon resource manager
											   const char *szVocabListFile,			///< (i) vocabulary list file
											   const char *szUpdateTime,			///< (i) resource building time
											   const float weightTask				///< (i) QC weight for current ASR task
);


/**
 * 	build compiled lexicon resource with a given vocabulary file, then update lexical resource of query task index.
 *
 * 	@return Return 0 if lexicon resource are updated successfully, otherwise return -1.
 */
HCILAB_PUBLIC HCI_LEXICON_API hci_int32
PowerASR_Resource_Lexicon_updateLexiconResource(PowerASR_Resource_Lexicon *pThis,	///< (i/o) pointer to the lexicon resource manager
											   const char *szVocabListFile,			///< (i) vocabulary list file
											   const char *szUpdateTime,			///< (i) resource building time
												const hci_uint32 idTask				///< (i) ASR task index
);

/**
 * 	load compiled lexicon resource from a given lexicon resource file, then update lexical resource of query task index.
 *
 * 	@return Return 0 if lexicon resource are updated successfully, otherwise return -1.
 */
HCILAB_PUBLIC HCI_LEXICON_API hci_int32
PowerASR_Resource_Lexicon_loadLexiconResource(PowerASR_Resource_Lexicon *pThis,		///< (i/o) pointer to the lexicon resource manager
											  const char *szLexiconResFile,			///< (i) lexicon resource file
											  const hci_uint32 idTask,				///< (i) ASR task index
											  const float weightST					///< (i) weight for short-term query QC
);

/**
 *	initialize user-specific data buffers for Lexicon resource manager.
 *
 *	@return Return 0 if user-specific data buffers are initialized successfully, otherwise return -1.
 */
HCILAB_PUBLIC HCI_LEXICON_API hci_int32
PowerASR_Resource_Lexicon_initializeUserLexiconData(PowerASR_Resource_Lexicon *pThis,	///< (i) pointer to the Lexicon resource manager
													UserLexicon *pUserLexicon,			///< (o) user-specific lexical data
													const hci_uint32 idTask				///< (i) recognition task index
);

/**
 *	release user-specific data buffers for Lexicon resource manager.
 *
 *	@return Return 0 if user-specific data buffers are released successfully, otherwise return -1.
 */
HCILAB_PUBLIC HCI_LEXICON_API hci_int32
PowerASR_Resource_Lexicon_releaseUserLexiconData(PowerASR_Resource_Lexicon *pThis,	///< (i) pointer to the Lexicon resource manager
												 UserLexicon *pUserLexicon			///< (o) user-specific lexical data
);

/**
 *	return vocabulary data corresponding to the query vocabulary lexicon index.
 *
 *	@return Return query vocabulary index.
 */
HCILAB_PUBLIC HCI_LEXICON_API hci_uint32
PowerASR_Resource_Lexicon_getVocabularyData(PowerASR_Resource_Lexicon *pThis,	///< (i) pointer to the Lexicon resource manager
											Lexicon_Resource *pResLexicon,		///< (i) pointer to lexicon resource
											const hci_uint32 idLexicon,			///< (i) vocabulary lexicon index
											char *szRecogWrod					///< (o) recognized word string
);

/**
 *	Get recognition vocabulary size of a given task.
 * 
 *	@return return the number of recognition vocabularies
 */
HCILAB_PUBLIC HCI_LEXICON_API hci_uint32
PowerASR_Resource_Lexicon_getVocabularySize(PowerASR_Resource_Lexicon *pThis,	///< (i) pointer to the Lexicon resource manager
											const hci_uint32 idTask				///< (i) recognition task index
);

/**
 *	return string of query vocabulary index.
 *
 *	@return Return 0 if query vocabulary string is returned successfully, otherwise return -1.
 */
HCILAB_PUBLIC HCI_LEXICON_API hci_int32
PowerASR_Resource_Lexicon_getVocabularyString(PowerASR_Resource_Lexicon *pThis,	///< (i) pointer to the Lexicon resource manager
											  const hci_uint32 idTask,			///< (i) recognition task index
											  const hci_uint32 idVocab,			///< (i) query vocabulary index
											  char *szVocabName					///< (o) vocabulary string
);

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
												const hci_uint32 idTask				///< (i) recognition task index
);

/**
 *	get the number of recognition tasks.
 *
 *	@return Return the number of recognition tasks.
 */
HCILAB_PUBLIC HCI_LEXICON_API hci_uint32
PowerASR_Resource_Lexicon_getTaskCount(PowerASR_Resource_Lexicon *pThis	///< (i) pointer to the Lexicon resource manager
);

/**
 *	reset compiled lexicon resource of query ASR task
 *
 *	@return return 0 if a compiled lexicon resource was reset successfully, otherwise return -1.
 */
HCILAB_PUBLIC HCI_LEXICON_API hci_int32
PowerASR_Resource_Lexicon_resetLexicalData(PowerASR_Resource_Lexicon *pThis,	///< (i) pointer to the Lexicon resource manager
										   const hci_uint32 idTask				///< (i) query recognition task index
);

/**
 *	add compiled lexicon resource of query ASR task
 *
 *	@return return 0 if query lexical data was added successfully, otherwise return -1.
 */
HCILAB_PUBLIC HCI_LEXICON_API hci_int32
PowerASR_Resource_Lexicon_addLexicalData(PowerASR_Resource_Lexicon *pThis,	///< (i) pointer to the Lexicon resource manager
										 const void* LexData,				///< (i) added lexical data
										 const hci_uint32 nSizeLexData,		///< (i) size of added lexical data in bytes
										 const hci_uint32 idTask			///< (i) query recognition task index
);

/**
 *	switch compiled lexicon resource of query ASR task
 *
 *	@return return 0 if a compiled lexicon resource was switched successfully, otherwise return -1.
 */
HCILAB_PUBLIC HCI_LEXICON_API hci_int32
PowerASR_Resource_Lexicon_switchLexicalData(PowerASR_Resource_Lexicon *pThis,	///< (i) pointer to the Lexicon resource manager
											const hci_uint32 idTask,			///< (i) query recognition task index
											const float weightST				///< (i) weight for short-term query QC
);

#ifdef __cplusplus
}
#endif

#endif // #ifndef __HCILAB_RESOURCE_LEXICON_H__
