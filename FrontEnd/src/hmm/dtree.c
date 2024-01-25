
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
 *	@file	dtree.c
 *	@ingroup hmm_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	decision tree library
 */

#include <stdio.h>
#include <stdlib.h>

#include "base/hci_type.h"
#include "base/hci_msg.h"

#include "hmm/hmm_common.h"

// local functions
#ifdef __cplusplus
extern "C" {
#endif 

/**
 * given left/right phonetic contexts, evaluate a node question
 */
HCILAB_PRIVATE hci_flag
_DTREE_evaluateNodeQuestion(const QUEST_SET *pQSet,					///< (i) question set
							const hci_uint32 idQuest,				///< (i) question index
							const hci_uint16 leftCxt,				///< (i) left phonetic context index
							const hci_uint16 rightCxt);				///< (i) right phonetic context index

/**
 * given left/right phonetic contexts, evaluate a node question
 */
HCILAB_PRIVATE hci_flag
_DTREE_isMemberOfPhoneticContexts(const hci_uint8 *paContext,		///< (i) phonetic context array
								  const hci_uint16 inputContext,	///< (i) query context
								  const hci_uint16 numContext);		///< (i) size of phonetic context array

#ifdef __cplusplus
}
#endif


/**
 * Given left/right context values, return a target value of terminal node in decision tree
 */
hci_uint16
AM_DTREE_findTargetValueOfPhoneticContext(const DT_NODE *nodeDT,				///< (i) node list in decision tree
										  const QUEST_SET *pQSet,				///< (i) question set
										  const hci_uint16 idRootNode,			///< (i) root node index
										  const hci_uint16 leftCxt,				///< (i) left phonetic context index
										  const hci_uint16 rightCxt)			///< (i) right phonetic context index
{
	const DT_NODE *pNode = 0;
	hci_uint16 nTargetValue = 0;

	pNode = nodeDT + idRootNode;
	while (pNode->descentNode) {
		if (_DTREE_evaluateNodeQuestion(pQSet,pNode->attr.idQuest,leftCxt,rightCxt)) {	// 'yes' descent node, or left-descent node
			pNode = nodeDT + pNode->descentNode;
		}
		else {				// 'no' descent node, or right-descent node
			pNode = nodeDT + pNode->descentNode + 1;
		}
	}

	nTargetValue = pNode->attr.value;

	return nTargetValue;
}


/**
 * given left/right phonetic contexts, evaluate a node question
 */
HCILAB_PRIVATE hci_flag
_DTREE_evaluateNodeQuestion(const QUEST_SET *pQSet,					///< (i) question set
							const hci_uint32 idQuest,				///< (i) question index
							const hci_uint16 inputLeftCxt,			///< (i) left phonetic context index
							const hci_uint16 inputRightCxt)			///< (i) right phonetic context index
{
	hci_uint16 numLeftCxt = 0;
	hci_uint16 numRightCxt = 0;
	const hci_uint8 *paContext = 0;
	hci_flag bResponse = 1;

	numLeftCxt = pQSet->ptrRightCxt[idQuest] - pQSet->ptrLeftCxt[idQuest];
	//if ((idQuest+1) == pQSet->numQuest) {
	//	numRightCxt = pQSet->sizeQuestCxt - pQSet->ptrRightCxt[idQuest];
	//}
	//else {
	//	numRightCxt = pQSet->ptrLeftCxt[idQuest+1] - pQSet->ptrRightCxt[idQuest];
	//}
	numRightCxt = pQSet->ptrLeftCxt[idQuest+1] - pQSet->ptrRightCxt[idQuest];

	if (numLeftCxt) {
		paContext = pQSet->cxtPhone + pQSet->ptrLeftCxt[idQuest];
		bResponse = _DTREE_isMemberOfPhoneticContexts(paContext, inputLeftCxt, numLeftCxt);
	}
	if (numRightCxt && bResponse) {
		paContext = pQSet->cxtPhone + pQSet->ptrRightCxt[idQuest];
		bResponse = _DTREE_isMemberOfPhoneticContexts(paContext, inputRightCxt, numRightCxt);
	}

	return bResponse;
}


/**
 * given left/right phonetic contexts, evaluate a node question
 */
HCILAB_PRIVATE hci_flag
_DTREE_isMemberOfPhoneticContexts(const hci_uint8 *paContext,		///< (i) phonetic context array
								  const hci_uint16 inputContext,	///< (i) query context
								  const hci_uint16 numContext)		///< (i) size of phonetic context array
{
	hci_int16 lower = 0, mid = 0;
	hci_int16 upper = numContext - 1;
	hci_int16 diff = 0;

	while (lower <= upper) {
		mid = ((lower+upper)>>1);
		diff = (hci_int16)inputContext - (hci_int16)paContext[mid];
		if (diff > 0) lower = mid + 1;
		else if (diff < 0) upper = mid - 1;
		else {
			return TRUE;
		}
	}

	return FALSE;
}


/* end of file */






















