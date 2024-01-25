
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
 *	@file	phoneseq2statenet.c
 *	@ingroup hmm_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	library to convert an input phone sequence into a state network
 */

#include <stdio.h>
#include <stdlib.h>
#ifndef WIN32
#include <string.h> // memset(), strcpy() etc.
#endif

#include "base/hci_type.h"
#include "base/hci_msg.h"
#include "base/hash_table.h"

#include "hmm/hmm_common.h"
#include "hmm/hmm_state_net.h"
#include "hmm/dtree.h"

#define CONTEXT2PID(c,l,r,n)	((c)*(n)*(n)+(l)*(n)+(r))
#define CONTEXT2PAIR(l,r,n)		((l)*(n)+(r))

// local functions
#ifdef __cplusplus
extern "C" {
#endif 

/**
 *  return a target value of terminal node in decision tree, if left/right context values are given.
 */
HCILAB_PRIVATE PLU_Subword*
_PhoSeq2Net_getContextDependentPLU(AM_Resource *pResAM,			///< (i) AM resource
								   hash_table_t *ht_subword,	///< (i) subword hash table
								   hci_uint16 cent_pid,			///< (i) center PLU index
								   hci_uint16 left_pid,			///< (i) left-side PLU index
								   hci_uint16 right_pid);		///< (i) right-side PLU index

#ifdef __cplusplus
}
#endif


/**
 * convert an input base-phone sequence into a state network
 */
hci_int32
AM_PhoSeq2Net_convertBasePhoneSeq2StateNetwork(AM_Resource *pResAM,				///< (i) AM resource
											   hash_table_t *ht_subword,		///<(i) subword hash table
											   StateNetwork *pStateNet,			///< (o) state network
											   const hci_uint8 *pBasePhoneSeq,	///< (i) base-phone sequence
											   const hci_uint32 lenBasePhoneSeq,	///< (i) length of base-phone sequence
											   const hci_uint32 nLexicalOrder)	///< (i) lexical order/direction (normal=0, reverse=1)
{
	hci_uint16 left_pid = 0, cent_pid = 0, right_pid = 0;
	hci_uint16 numBase = 0;
	hci_uint16 idBaseSilence = 0;
	hci_uint16 idBaseShortPause = 0;
	//hci_uint16 idState = 0;
	HMM_INFO *pHmmInfo = &pResAM->hmm_info;
	PLU_BasePhone *pBasePLU = pResAM->basePLU;
	HMM_BaseState *pBaseState = 0;
#if ( !defined(USE_DT_TABLE) && !defined(USE_LHMM2PHMM_TABLE) )
	DT_NODE *pDTNode = pResAM->nodeDT_State;
	QUEST_SET *pQuestSet = &pResAM->questSet;
#endif // #if !defined(USE_DT_TABLE)
	StateNode *pNode = pStateNet->node;
	PLU_BasePhone *nextPhone = 0;
	PLU_BasePhone *currentPhone = 0;
	//HMM_BaseState *pState = 0;
#if (!defined(TIED_STATE) || defined(USE_LHMM2PHMM_TABLE))
	PLU_Subword *pCDPLU = 0;
#endif	// #if !defined(TIED_STATE)
	const hci_uint8 *curBase = 0;
	const hci_uint8 *nextBase = 0;
	const hci_uint8 *lastBase = 0;
	hci_uint16 *curState = 0;
	hci_uint16 *lastState = 0;
	hci_uint8 numStates = 0;
#if ( defined(USE_DT_TABLE) || defined(USE_LHMM2PHMM_TABLE) )
	hci_uint32 idPLU = 0;
#endif // #if ( defined(USE_DT_TABLE) || defined(USE_LHMM2PHMM_TABLE) )

	numBase = pHmmInfo->numBasePhones;
	idBaseSilence = pHmmInfo->idBaseSilence;
	idBaseShortPause = pHmmInfo->idBaseShortPause;

#if defined(TIED_STATE)

	if (pStateNet->numSharedPhone) {
		const hci_uint8 *prevBase = 0;
		curBase = pBasePhoneSeq + pStateNet->numSharedPhone;
		pNode += pStateNet->numSharedNode;
		left_pid = idBaseSilence;
		prevBase = curBase - 1;
		while (prevBase > pBasePhoneSeq) {
			if (pBasePLU[*prevBase].attr.flagCxtFree==FALSE) {
				left_pid = *prevBase;
				break;
			}
			prevBase--;
		}
		pStateNet->numNodes = pStateNet->numSharedNode;
		//pStateNet->numEmitNodes = pStateNet->numSharedNode;
		if (pStateNet->numSharedPhone < lenBasePhoneSeq) {
			nextBase = curBase + 1;
			nextPhone = pResAM->basePLU + (*nextBase);
			(pNode-1)->attr.bJumpArc = nextPhone->attr.flagCxtFree;
		}
	}
	else {
		curBase = pBasePhoneSeq;
		left_pid = idBaseSilence;
		pStateNet->numNodes = 0;
		pStateNet->numEmitNodes = 0;
	}
	if (nLexicalOrder) {	// with reverse lexicon
		lastBase = pBasePhoneSeq + lenBasePhoneSeq - 1;
		while (curBase < lastBase) {
			currentPhone = pResAM->basePLU + (*curBase);
			nextBase = curBase + 1;
			nextPhone = pResAM->basePLU + (*nextBase);
			right_pid = *(nextBase+nextPhone->attr.flagCxtFree);
			numStates = currentPhone->attr.numStates;
			pStateNet->numNodes += numStates;
			//if (pStateNet->numNodes > STATE_NET_SIZE) {
			//	HCIMSG_WARN("STATE_NET_SIZE overflow.\n");
			//	pStateNet->numNodes -= currentPhone->attr.numStates;
			//	return -1;
			//}
			cent_pid = (*curBase);
#if ( defined(USE_DT_TABLE) || defined(USE_LHMM2PHMM_TABLE) )
			idPLU = CONTEXT2PAIR(right_pid,left_pid,numBase);
#ifdef USE_LHMM2PHMM_TABLE
			pCDPLU = pResAM->cxtdepPLU + currentPhone->lhmm2phmm[idPLU];
#endif	// #ifdef USE_LHMM2PHMM_TABLE
#endif // #if ( defined(USE_DT_TABLE) || defined(USE_LHMM2PHMM_TABLE) )
			if (numStates == 1) {
				if (currentPhone->attr.flagCxtFree==FALSE) {
					pStateNet->numEmitNodes += numStates;
				}
				pBaseState = pResAM->baseState + currentPhone->stateSeq[0];
				pNode->attr.bCrossModel = TRUE;
				pNode->attr.bActive = FALSE;
				pNode->attr.bJumpArc = nextPhone->attr.flagCxtFree;
#ifdef USE_DT_TABLE
				pNode->attr.idState = pBaseState->idRootNode + pBaseState->cxt2state[idPLU];
#elif defined(USE_LHMM2PHMM_TABLE)
				pNode->attr.idState = pCDPLU->stateSeq[0];
#else	// !USE_DT_TABLE && !USE_LHMM2PHMM_TABLE
				if (currentPhone->attr.flagCxtFree || currentPhone->attr.flagCxtIndep) {
					pNode->attr.idState = pBaseState->ptrEmitState;
				}
				else {
					pNode->attr.idState = AM_DTREE_findTargetValueOfPhoneticContext(pDTNode,
																					pQuestSet,
																					pBaseState->idRootNode,
																					right_pid,
																					left_pid);
				}
#endif // #ifdef USE_DT_TABLE
				pNode++;
			}
			else if (numStates == 3) {
				pStateNet->numEmitNodes += numStates;
				pBaseState = pResAM->baseState + currentPhone->stateSeq[2];
				pNode->attr.bCrossModel = FALSE;
				pNode->attr.bActive = FALSE;
				pNode->attr.bJumpArc = FALSE;
#ifdef USE_DT_TABLE
				pNode->attr.idState = pBaseState->idRootNode + pBaseState->cxt2state[idPLU];
#elif defined(USE_LHMM2PHMM_TABLE)
				pNode->attr.idState = pCDPLU->stateSeq[2];
#else	// !USE_DT_TABLE && !USE_LHMM2PHMM_TABLE
				pNode->attr.idState = AM_DTREE_findTargetValueOfPhoneticContext(pDTNode,
																				pQuestSet,
																				pBaseState->idRootNode,
																				right_pid,
																				left_pid);
#endif // #ifdef USE_DT_TABLE
				pNode++;
				pBaseState = pResAM->baseState + currentPhone->stateSeq[1];
				pNode->attr.bCrossModel = FALSE;
				pNode->attr.bActive = FALSE;
				pNode->attr.bJumpArc = FALSE;
#ifdef USE_DT_TABLE
				pNode->attr.idState = pBaseState->idRootNode + pBaseState->cxt2state[idPLU];
#elif defined(USE_LHMM2PHMM_TABLE)
				pNode->attr.idState = pCDPLU->stateSeq[1];
#else	// !USE_DT_TABLE
				pNode->attr.idState = AM_DTREE_findTargetValueOfPhoneticContext(pDTNode,
																				pQuestSet,
																				pBaseState->idRootNode,
																				right_pid,
																				left_pid);
#endif // #ifdef USE_DT_TABLE
				pNode++;
				pBaseState = pResAM->baseState + currentPhone->stateSeq[0];
				pNode->attr.bCrossModel = TRUE;
				pNode->attr.bActive = FALSE;
				pNode->attr.bJumpArc = nextPhone->attr.flagCxtFree;
#ifdef USE_DT_TABLE
				pNode->attr.idState = pBaseState->idRootNode + pBaseState->cxt2state[idPLU];
#elif defined(USE_LHMM2PHMM_TABLE)
				pNode->attr.idState = pCDPLU->stateSeq[0];
#else	// !USE_DT_TABLE && !USE_LHMM2PHMM_TABLE
				pNode->attr.idState = AM_DTREE_findTargetValueOfPhoneticContext(pDTNode,
																				pQuestSet,
																				pBaseState->idRootNode,
																				right_pid,
																				left_pid);
#endif // #ifdef USE_DT_TABLE
				pNode++;
			}
			else {
				;
			}
			if (currentPhone->attr.flagCxtFree==FALSE) {
				left_pid = cent_pid;
			}
			curBase++;
		}
		currentPhone = pResAM->basePLU + (*curBase);
		nextPhone = pResAM->basePLU + idBaseSilence;
		right_pid = idBaseSilence;
		numStates = currentPhone->attr.numStates;
		pStateNet->numNodes += numStates;
		//if (pStateNet->numNodes > STATE_NET_SIZE) {
		//	HCIMSG_WARN("STATE_NET_SIZE overflow.\n");
		//	pStateNet->numNodes -= currentPhone->attr.numStates;
		//	return -1;
		//}
		cent_pid = (*curBase);
#if ( defined(USE_DT_TABLE) || defined(USE_LHMM2PHMM_TABLE) )
		idPLU = CONTEXT2PAIR(right_pid,left_pid,numBase);
#ifdef USE_LHMM2PHMM_TABLE
		pCDPLU = pResAM->cxtdepPLU + currentPhone->lhmm2phmm[idPLU];
#endif	// #ifdef USE_LHMM2PHMM_TABLE
#endif // #if ( defined(USE_DT_TABLE) || defined(USE_LHMM2PHMM_TABLE) )
		if (numStates == 1) {
			if (currentPhone->attr.flagCxtFree==FALSE) {
				pStateNet->numEmitNodes += numStates;
			}
			pBaseState = pResAM->baseState + currentPhone->stateSeq[0];
			pNode->attr.bCrossModel = TRUE;
			pNode->attr.bActive = FALSE;
			pNode->attr.bJumpArc = nextPhone->attr.flagCxtFree;
#ifdef USE_DT_TABLE
			pNode->attr.idState = pBaseState->idRootNode + pBaseState->cxt2state[idPLU];
#elif defined(USE_LHMM2PHMM_TABLE)
			pNode->attr.idState = pCDPLU->stateSeq[0];
#else	// !USE_DT_TABLE && !USE_LHMM2PHMM_TABLE
			if (currentPhone->attr.flagCxtFree || currentPhone->attr.flagCxtIndep) {
				pNode->attr.idState = pBaseState->ptrEmitState;
			}
			else {
				pNode->attr.idState = AM_DTREE_findTargetValueOfPhoneticContext(pDTNode,
																				pQuestSet,
																				pBaseState->idRootNode,
																				right_pid,
																				left_pid);
			}
#endif // #ifdef USE_DT_TABLE
			pNode++;
		}
		else if (numStates == 3) {
			pStateNet->numEmitNodes += numStates;
			pBaseState = pResAM->baseState + currentPhone->stateSeq[2];
			pNode->attr.bCrossModel = FALSE;
			pNode->attr.bActive = FALSE;
			pNode->attr.bJumpArc = FALSE;
#ifdef USE_DT_TABLE
			pNode->attr.idState = pBaseState->idRootNode + pBaseState->cxt2state[idPLU];
#elif defined(USE_LHMM2PHMM_TABLE)
			pNode->attr.idState = pCDPLU->stateSeq[2];
#else	// !USE_DT_TABLE && !USE_LHMM2PHMM_TABLE
			pNode->attr.idState = AM_DTREE_findTargetValueOfPhoneticContext(pDTNode,
																			pQuestSet,
																			pBaseState->idRootNode,
																			right_pid,
																			left_pid);
#endif // #ifdef USE_DT_TABLE
			pNode++;
			pBaseState = pResAM->baseState + currentPhone->stateSeq[1];
			pNode->attr.bCrossModel = FALSE;
			pNode->attr.bActive = FALSE;
			pNode->attr.bJumpArc = FALSE;
#ifdef USE_DT_TABLE
			pNode->attr.idState = pBaseState->idRootNode + pBaseState->cxt2state[idPLU];
#elif defined(USE_LHMM2PHMM_TABLE)
			pNode->attr.idState = pCDPLU->stateSeq[1];
#else	// !USE_DT_TABLE && !USE_LHMM2PHMM_TABLE
			pNode->attr.idState = AM_DTREE_findTargetValueOfPhoneticContext(pDTNode,
																			pQuestSet,
																			pBaseState->idRootNode,
																			right_pid,
																			left_pid);
#endif // #ifdef USE_DT_TABLE
			pNode++;
			pBaseState = pResAM->baseState + currentPhone->stateSeq[0];
			pNode->attr.bCrossModel = TRUE;
			pNode->attr.bActive = FALSE;
			pNode->attr.bJumpArc = nextPhone->attr.flagCxtFree;
#ifdef USE_DT_TABLE
			pNode->attr.idState = pBaseState->idRootNode + pBaseState->cxt2state[idPLU];
#elif defined(USE_LHMM2PHMM_TABLE)
			pNode->attr.idState = pCDPLU->stateSeq[0];
#else	// !USE_DT_TABLE && !USE_LHMM2PHMM_TABLE
			pNode->attr.idState = AM_DTREE_findTargetValueOfPhoneticContext(pDTNode,
																			pQuestSet,
																			pBaseState->idRootNode,
																			right_pid,
																			left_pid);
#endif // #ifdef USE_DT_TABLE
			pNode++;
		}
		else {
			;
		}
	}
	else {	// with normal lexicon
		lastBase = pBasePhoneSeq + lenBasePhoneSeq - 1;
		while (curBase < lastBase) {
			currentPhone = pResAM->basePLU + (*curBase);
			nextBase = curBase + 1;
			nextPhone = pResAM->basePLU + (*nextBase);
			right_pid = *(nextBase+nextPhone->attr.flagCxtFree);
			numStates = currentPhone->attr.numStates;
			pStateNet->numNodes += numStates;
			//if (pStateNet->numNodes > STATE_NET_SIZE) {
			//	HCIMSG_WARN("STATE_NET_SIZE overflow.\n");
			//	pStateNet->numNodes -= currentPhone->attr.numStates;
			//	return -1;
			//}
			cent_pid = (*curBase);
#if ( defined(USE_DT_TABLE) || defined(USE_LHMM2PHMM_TABLE) )
			idPLU = CONTEXT2PAIR(left_pid,right_pid,numBase);
#ifdef USE_LHMM2PHMM_TABLE
			pCDPLU = pResAM->cxtdepPLU + currentPhone->lhmm2phmm[idPLU];
#endif	// #ifdef USE_LHMM2PHMM_TABLE
#endif // #if ( defined(USE_DT_TABLE) || defined(USE_LHMM2PHMM_TABLE) )
			if (numStates == 1) {
				if (currentPhone->attr.flagCxtFree==FALSE) {
					pStateNet->numEmitNodes += numStates;
				}
				pBaseState = pResAM->baseState + currentPhone->stateSeq[0];
				pNode->attr.bCrossModel = TRUE;
				pNode->attr.bActive = FALSE;
				pNode->attr.bJumpArc = nextPhone->attr.flagCxtFree;
#ifdef USE_DT_TABLE
				pNode->attr.idState = pBaseState->idRootNode + pBaseState->cxt2state[idPLU];
#elif defined(USE_LHMM2PHMM_TABLE)
				pNode->attr.idState = pCDPLU->stateSeq[0];
#else	// !USE_DT_TABLE && !USE_LHMM2PHMM_TABLE
				if (currentPhone->attr.flagCxtFree || currentPhone->attr.flagCxtIndep) {
					pNode->attr.idState = pBaseState->ptrEmitState;
				}
				else {
					pNode->attr.idState = AM_DTREE_findTargetValueOfPhoneticContext(pDTNode,
																					pQuestSet,
																					pBaseState->idRootNode,
																					left_pid,
																					right_pid);
				}
#endif // #ifdef USE_DT_TABLE
				pNode++;
			}
			else if (numStates == 3) {
				pStateNet->numEmitNodes += numStates;
				pBaseState = pResAM->baseState + currentPhone->stateSeq[0];
				pNode->attr.bCrossModel = FALSE;
				pNode->attr.bActive = FALSE;
				pNode->attr.bJumpArc = FALSE;
#ifdef USE_DT_TABLE
				pNode->attr.idState = pBaseState->idRootNode + pBaseState->cxt2state[idPLU];
#elif defined(USE_LHMM2PHMM_TABLE)
				pNode->attr.idState = pCDPLU->stateSeq[0];
#else	// !USE_DT_TABLE && !USE_LHMM2PHMM_TABLE
				pNode->attr.idState = AM_DTREE_findTargetValueOfPhoneticContext(pDTNode,
																				pQuestSet,
																				pBaseState->idRootNode,
																				left_pid,
																				right_pid);
#endif // #ifdef USE_DT_TABLE
				pNode++;
				pBaseState = pResAM->baseState + currentPhone->stateSeq[1];
				pNode->attr.bCrossModel = FALSE;
				pNode->attr.bActive = FALSE;
				pNode->attr.bJumpArc = FALSE;
#ifdef USE_DT_TABLE
				pNode->attr.idState = pBaseState->idRootNode + pBaseState->cxt2state[idPLU];
#elif defined(USE_LHMM2PHMM_TABLE)
				pNode->attr.idState = pCDPLU->stateSeq[1];
#else	// !USE_DT_TABLE && !USE_LHMM2PHMM_TABLE
				pNode->attr.idState = AM_DTREE_findTargetValueOfPhoneticContext(pDTNode,
																				pQuestSet,
																				pBaseState->idRootNode,
																				left_pid,
																				right_pid);
#endif // #ifdef USE_DT_TABLE
				pNode++;
				pBaseState = pResAM->baseState + currentPhone->stateSeq[2];
				pNode->attr.bCrossModel = TRUE;
				pNode->attr.bActive = FALSE;
				pNode->attr.bJumpArc = nextPhone->attr.flagCxtFree;
#ifdef USE_DT_TABLE
				pNode->attr.idState = pBaseState->idRootNode + pBaseState->cxt2state[idPLU];
#elif defined(USE_LHMM2PHMM_TABLE)
				pNode->attr.idState = pCDPLU->stateSeq[2];
#else	// !USE_DT_TABLE && !USE_LHMM2PHMM_TABLE
				pNode->attr.idState = AM_DTREE_findTargetValueOfPhoneticContext(pDTNode,
																				pQuestSet,
																				pBaseState->idRootNode,
																				left_pid,
																				right_pid);
#endif // #ifdef USE_DT_TABLE
				pNode++;
			}
			else {
				;
			}
			if (currentPhone->attr.flagCxtFree==FALSE) {
				left_pid = cent_pid;
			}
			curBase++;
		}
		currentPhone = pResAM->basePLU + (*curBase);
		nextPhone = pResAM->basePLU + idBaseSilence;
		right_pid = pHmmInfo->idBaseSilence;
		numStates = currentPhone->attr.numStates;
		pStateNet->numNodes += numStates;
		//if (pStateNet->numNodes > STATE_NET_SIZE) {
		//	HCIMSG_WARN("STATE_NET_SIZE overflow.\n");
		//	pStateNet->numNodes -= currentPhone->attr.numStates;
		//	return -1;
		//}
		cent_pid = (*curBase);
#if ( defined(USE_DT_TABLE) || defined(USE_LHMM2PHMM_TABLE) )
		idPLU = CONTEXT2PAIR(left_pid,right_pid,numBase);
#ifdef USE_LHMM2PHMM_TABLE
		pCDPLU = pResAM->cxtdepPLU + currentPhone->lhmm2phmm[idPLU];
#endif	// #ifdef USE_LHMM2PHMM_TABLE
#endif // #if ( defined(USE_DT_TABLE) || defined(USE_LHMM2PHMM_TABLE) )
		if (numStates == 1) {
			if (currentPhone->attr.flagCxtFree==FALSE) {
				pStateNet->numEmitNodes += numStates;
			}
			pBaseState = pResAM->baseState + currentPhone->stateSeq[0];
			pNode->attr.bCrossModel = TRUE;
			pNode->attr.bActive = FALSE;
			pNode->attr.bJumpArc = nextPhone->attr.flagCxtFree;
#ifdef USE_DT_TABLE
			pNode->attr.idState = pBaseState->idRootNode + pBaseState->cxt2state[idPLU];
#elif defined(USE_LHMM2PHMM_TABLE)
			pNode->attr.idState = pCDPLU->stateSeq[0];
#else	// !USE_DT_TABLE && !USE_LHMM2PHMM_TABLE
			if (currentPhone->attr.flagCxtFree || currentPhone->attr.flagCxtIndep) {
				pNode->attr.idState = pBaseState->ptrEmitState;
			}
			else {
				pNode->attr.idState = AM_DTREE_findTargetValueOfPhoneticContext(pDTNode,
																				pQuestSet,
																				pBaseState->idRootNode,
																				left_pid,
																				right_pid);
			}
#endif // #ifdef USE_DT_TABLE
			pNode++;
		}
		else if (numStates == 3) {
			pStateNet->numEmitNodes += numStates;
			pBaseState = pResAM->baseState + currentPhone->stateSeq[0];
			pNode->attr.bCrossModel = FALSE;
			pNode->attr.bActive = FALSE;
			pNode->attr.bJumpArc = FALSE;
#ifdef USE_DT_TABLE
			pNode->attr.idState = pBaseState->idRootNode + pBaseState->cxt2state[idPLU];
#elif defined(USE_LHMM2PHMM_TABLE)
			pNode->attr.idState = pCDPLU->stateSeq[0];
#else	// !USE_DT_TABLE && !USE_LHMM2PHMM_TABLE
			pNode->attr.idState = AM_DTREE_findTargetValueOfPhoneticContext(pDTNode,
																			pQuestSet,
																			pBaseState->idRootNode,
																			left_pid,
																			right_pid);
#endif // #ifdef USE_DT_TABLE
			pNode++;
			pBaseState = pResAM->baseState + currentPhone->stateSeq[1];
			pNode->attr.bCrossModel = FALSE;
			pNode->attr.bActive = FALSE;
			pNode->attr.bJumpArc = FALSE;
#ifdef USE_DT_TABLE
			pNode->attr.idState = pBaseState->idRootNode + pBaseState->cxt2state[idPLU];
#elif defined(USE_LHMM2PHMM_TABLE)
			pNode->attr.idState = pCDPLU->stateSeq[1];
#else	// !USE_DT_TABLE && !USE_LHMM2PHMM_TABLE
			pNode->attr.idState = AM_DTREE_findTargetValueOfPhoneticContext(pDTNode,
																			pQuestSet,
																			pBaseState->idRootNode,
																			left_pid,
																			right_pid);
#endif // #ifdef USE_DT_TABLE
			pNode++;
			pBaseState = pResAM->baseState + currentPhone->stateSeq[2];
			pNode->attr.bCrossModel = TRUE;
			pNode->attr.bActive = FALSE;
			pNode->attr.bJumpArc = nextPhone->attr.flagCxtFree;
#ifdef USE_DT_TABLE
			pNode->attr.idState = pBaseState->idRootNode + pBaseState->cxt2state[idPLU];
#elif defined(USE_LHMM2PHMM_TABLE)
			pNode->attr.idState = pCDPLU->stateSeq[2];
#else	// !USE_DT_TABLE && !USE_LHMM2PHMM_TABLE
			pNode->attr.idState = AM_DTREE_findTargetValueOfPhoneticContext(pDTNode,
																			pQuestSet,
																			pBaseState->idRootNode,
																			left_pid,
																			right_pid);
#endif // #ifdef USE_DT_TABLE
			pNode++;
		}
		else {
			;
		}
	}
/*
	lastBase = pBasePhoneSeq + lenBasePhoneSeq;
	while (curBase < lastBase) {
		if ((*curBase) == pHmmInfo->idBaseShortPause) {
			pNode->attr.bCrossModel = TRUE;
			pNode->attr.bActive = FALSE;
			pNode->attr.bJumpArc = nextPhone->attr.flagCxtFree;
			pState = pResAM->baseState + currentPhone->stateSeq[0];
			pNode->attr.idState = pState->ptrEmitState;
			pNode++;
		}
		else if ((*curBase) == pHmmInfo->idBaseSilence) {
			pStateNet->numEmitNodes += currentPhone->attr.numStates;
			curState = currentPhone->stateSeq;
			lastState = curState + currentPhone->attr.numStates;
			while ((curState+1) < lastState) {
				pNode->attr.bCrossModel = FALSE;
				pNode->attr.bActive = FALSE;
				pNode->attr.bJumpArc = FALSE;
				pState = pResAM->baseState + (*curState);
				pNode->attr.idState = pState->ptrEmitState;
				pNode++;
				curState++;
			}
			pNode->attr.bCrossModel = TRUE;
			pNode->attr.bActive = FALSE;
			pNode->attr.bJumpArc = nextPhone->attr.flagCxtFree;
			pState = pResAM->baseState + (*curState);
			pNode->attr.idState = pState->ptrEmitState;
			pNode++;
			left_pid = (*curBase);
		}
		else {
			pStateNet->numEmitNodes += currentPhone->attr.numStates;
			cent_pid = (*curBase);
			right_pid = pHmmInfo->idBaseSilence;
			nextBase = curBase + 1;
			while (nextBase < lastBase) {
				if ((*nextBase) != pHmmInfo->idBaseShortPause) {
					right_pid = (*nextBase);
					break;
				}
				nextBase++;
			}
			idPLU = CONTEXT2PID(cent_pid,left_pid,right_pid,numBase);
			curState = currentPhone->stateSeq;
			lastState = curState + currentPhone->attr.numStates;
			idState = 0;
			while ((curState+1) < lastState) {
				pNode->attr.bCrossModel = FALSE;
				pNode->attr.bActive = FALSE;
				pNode->attr.bJumpArc = FALSE;
				//pState = pResAM->baseState + (*curState);
				//pNode->attr.idState = AM_DTREE_findTargetValueOfPhoneticContext(pResAM->nodeDT_State,
				//																&pResAM->questSet,
				//																pState->idRootNode,
				//																left_pid,
				//																right_pid);
				pNode->attr.idState = cxt2state[idPLU].idState[idState++];
				pNode++;
				curState++;
			}
			pNode->attr.bCrossModel = TRUE;
			pNode->attr.bActive = FALSE;
			pNode->attr.bJumpArc = nextPhone->attr.flagCxtFree;
			//pState = pResAM->baseState + (*curState);
			//pNode->attr.idState = AM_DTREE_findTargetValueOfPhoneticContext(pResAM->nodeDT_State,
			//																&pResAM->questSet,
			//																pState->idRootNode,
			//																left_pid,
			//																right_pid);
			pNode->attr.idState = cxt2state[idPLU].idState[idState];
			pNode++;
			left_pid = cent_pid;
		}
		curBase++;
	}
*/

#else	// !TIED_STATE

	curBase = pBasePhoneSeq;
	lastBase = pBasePhoneSeq + lenBasePhoneSeq;
	while (curBase < lastBase) {
		currentPhone = pResAM->basePLU + (*curBase);
		nextBase = curBase + 1;
		if ((curBase+1) == lastBase) {
			nextPhone = pResAM->basePLU + pHmmInfo->idBaseSilence;
		}
		else {
			nextPhone = pResAM->basePLU + (*nextBase);
		}
		pStateNet->numNodes += currentPhone->attr.numStates;
		if (pStateNet->numNodes > STATE_NET_SIZE) {
			HCIMSG_WARN("STATE_NET_SIZE overflow.\n");
			pStateNet->numNodes -= currentPhone->attr.numStates;
			return -1;
		}
		//if ((*curBase) == pHmmInfo->idBaseShortPause) {
		if (pBasePLU[*curBase].attr.flagCxtFree==TRUE) {
			pNode->attr.bCrossModel = TRUE;
			pNode->attr.bActive = FALSE;
			pNode->attr.bJumpArc = nextPhone->attr.flagCxtFree;
			pState = pResAM->baseState + currentPhone->stateSeq[0];
			pNode->attr.idState = pState->ptrEmitState;
			pNode++;
		}
		else if ((*curBase) == pHmmInfo->idBaseSilence) {
			pStateNet->numEmitNodes += currentPhone->attr.numStates;
			curState = currentPhone->stateSeq;
			lastState = curState + currentPhone->attr.numStates;
			while ((curState+1) < lastState) {
				pNode->attr.bCrossModel = FALSE;
				pNode->attr.bActive = FALSE;
				pNode->attr.bJumpArc = FALSE;
				pState = pResAM->baseState + (*curState);
				pNode->attr.idState = pState->ptrEmitState;
				pNode++;
				curState++;
			}
			pNode->attr.bCrossModel = TRUE;
			pNode->attr.bActive = FALSE;
			pNode->attr.bJumpArc = nextPhone->attr.flagCxtFree;
			pState = pResAM->baseState + (*curState);
			pNode->attr.idState = pState->ptrEmitState;
			pNode++;
			left_pid = (*curBase);
		}
		else {
			pStateNet->numEmitNodes += currentPhone->attr.numStates;
			cent_pid = (*curBase);
			right_pid = pHmmInfo->idBaseSilence;
			nextBase = curBase + 1;
			while (nextBase < lastBase) {
				if ((*nextBase) != pHmmInfo->idBaseShortPause) {
					right_pid = (*nextBase);
					break;
				}
				nextBase++;
			}
#if defined(USE_LHMM2PHMM_TABLE)
			pCDPLU = pResAM->cxtdepPLU + currentPhone->lhmm2phmm[CONTEXT2PAIR(left_pid,right_pid,numBase)];
#else	// !USE_LHMM2PHMM_TABLE
			pCDPLU = _PhoSeq2Net_getContextDependentPLU(pResAM,
														ht_subword,
														cent_pid,
														left_pid,
														right_pid);
#endif // #if defined(USE_LHMM2PHMM_TABLE)
			curState = pCDPLU->stateSeq;
			lastState = curState + currentPhone->attr.numStates;
			while ((curState+1) < lastState) {
				pNode->attr.bCrossModel = FALSE;
				pNode->attr.bActive = FALSE;
				pNode->attr.bJumpArc = FALSE;
				pNode->attr.idState = (*curState);
				pNode++;
				curState++;
			}
			pNode->attr.bCrossModel = TRUE;
			pNode->attr.bActive = FALSE;
			pNode->attr.bJumpArc = nextPhone->attr.flagCxtFree;
			pNode->attr.idState = (*curState);
			pNode++;
			left_pid = cent_pid;
		}
		curBase++;
	}

#endif	// #if defined(TIED_STATE)

	return 0;
}

/**
 * convert an input subword sequence into a state network
 */
hci_int32
AM_PhoSeq2Net_convertSubwordSeq2StateNetwork(const AM_Resource *pResAM,			///< (i) AM resource
											 StateNetwork *pStateNet,			///< (o) state network
											 const hci_uint16 *pSubwordSeq,		///< (i) subword sequence
											 const hci_uint32 lenSubwordSeq)	///< (i) length of subword sequence
{
	hci_uint32 i = 0;
	hci_uint32 n = 0;
	const HMM_INFO *pHmmInfo = &pResAM->hmm_info;
	StateNode *pNode = pStateNet->node;
	const PLU_Subword *nextPLU = 0;
	const PLU_Subword *currentPLU = 0;

	memset(pStateNet, 0, sizeof(StateNetwork));

#if (!defined(TIED_STATE) && defined(USE_TRIP_GRAMMAR))

	for (i = 0; i < lenSubwordSeq; i++) {
		currentPLU = pResAM->cxtdepPLU + pSubwordSeq[i];
		if ((i+1) == lenSubwordSeq) {
			nextPLU = pResAM->cxtdepPLU + pHmmInfo->idSubwdSilence;
		}
		else {
			nextPLU = pResAM->cxtdepPLU + pSubwordSeq[i+1];
		}
		pStateNet->numNodes += currentPLU->attr.numStates;
		if (pStateNet->numNodes > STATE_NET_SIZE) {
			HCIMSG_WARN("STATE_NET_SIZE overflow.\n");
			pStateNet->numNodes -= currentPLU->attr.numStates;
			return -1;
		}
		if (currentPLU->attr.flagCxtFree) {
			pNode->attr.bCrossModel = TRUE;
			pNode->attr.bActive = FALSE;
			pNode->attr.bJumpArc = nextPLU->attr.flagCxtFree;
			pNode->attr.idState = currentPLU->stateSeq[0];
			pNode++;
		}
		else if (currentPLU->attr.flagCxtIndep) {
			pStateNet->numEmitNodes += currentPLU->attr.numStates;
			for (n = 0; (n+1) < currentPLU->attr.numStates; n++) {
				pNode->attr.bCrossModel = FALSE;
				pNode->attr.bActive = FALSE;
				pNode->attr.bJumpArc = FALSE;
				pNode->attr.idState = currentPLU->stateSeq[n];
				pNode++;
			}
			pNode->attr.bCrossModel = TRUE;
			pNode->attr.bActive = FALSE;
			pNode->attr.bJumpArc = nextPLU->attr.flagCxtFree;
			pNode->attr.idState = currentPLU->stateSeq[n];
			pNode++;
		}
		else {
			pStateNet->numEmitNodes += currentPLU->attr.numStates;
			for (n = 0; (n+1) < currentPLU->attr.numStates; n++) {
				pNode->attr.bCrossModel = FALSE;
				pNode->attr.bActive = FALSE;
				pNode->attr.bJumpArc = FALSE;
				pNode->attr.idState = currentPLU->stateSeq[n];
				pNode++;
			}
			pNode->attr.bCrossModel = TRUE;
			pNode->attr.bActive = FALSE;
			pNode->attr.bJumpArc = nextPLU->attr.flagCxtFree;
			pNode->attr.idState = currentPLU->stateSeq[n];
			pNode++;
		}
	}
#endif // #if !defined(TIED_STATE)

	return 0;
}


/**
 * create base-phone hash table
 */
hash_table_t*
AM_PhoSeq2Net_createBasePhoneHashTable(AM_Resource *pResAM)
{
	HMM_INFO *pHmmInfo = &pResAM->hmm_info;
	PLU_BasePhone *pBasePLU = 0;
	PLU_BasePhone *pLastPLU = 0;
	hash_table_t *basephone_ht = 0;
	hci_uint32 n = 0;

    basephone_ht = PowerASR_Base_newHashTable((hci_int32)pHmmInfo->numBasePhones, 0);

	pBasePLU = pResAM->basePLU;
	pLastPLU = pResAM->basePLU + pHmmInfo->numBasePhones;
	while (pBasePLU < pLastPLU) {
		if (PowerASR_Base_enterHashTable(basephone_ht, pBasePLU->szName, (void *)n) != (void *)n) {
			HCIMSG_ERROR("Insertion of base-phone [%s] failed\n", pBasePLU->szName);
			PowerASR_Base_freeHashTable(basephone_ht);
			return 0;
		}
		pBasePLU++; n++;
	}

	return basephone_ht;
}


/**
 * create subword hash table
 */
hash_table_t*
AM_PhoSeq2Net_createSubwordHashTable(AM_Resource *pResAM)
{
	HMM_INFO *pHmmInfo = &pResAM->hmm_info;
	PLU_Subword *pCDPLU = 0;
	hash_table_t *subword_ht = 0;
	hci_uint32 n = 0;

#if (!defined(TIED_STATE) && !defined(USE_TRIP_GRAMMAR))

    subword_ht = PowerASR_Base_newHashTable((hci_int32)pHmmInfo->numSubwords, 0);

	pCDPLU = pResAM->cxtdepPLU;
	for (n = 0; n < pHmmInfo->numSubwords; n++, pCDPLU++) {
		if (PowerASR_Base_enterHashTable(subword_ht, pCDPLU->szName, (void *)n) != (void *)n) {
			HCIMSG_ERROR("Insertion of subword [%s] failed\n", pCDPLU->szName);
			PowerASR_Base_freeHashTable(subword_ht);
			return 0;
		}
	}

#endif	// #if (!defined(TIED_STATE) && !defined(USE_TRIP_GRAMMAR))

	return subword_ht;
}


/**
 * delete subword hash table
 */
hci_int32
AM_PhoSeq2Net_deleteSubwordHashTable(hash_table_t *subword_ht)
{

	PowerASR_Base_freeHashTable(subword_ht);
    subword_ht = 0;

	return 0;
}

/**
 *  return a target value of terminal node in decision tree, if left/right context values are given.
 */
HCILAB_PRIVATE PLU_Subword*
_PhoSeq2Net_getContextDependentPLU(AM_Resource *pResAM,			///< (i) AM resource
								   hash_table_t *ht_subword,	///< (i) subword hash table
								   hci_uint16 cent_pid,			///< (i) center PLU index
								   hci_uint16 left_pid,			///< (i) left-side PLU index
								   hci_uint16 right_pid)		///< (i) right-side PLU index
{
	PLU_Subword *pCDPLU = 0;

#if (!defined(TIED_STATE) && !defined(USE_TRIP_GRAMMAR))

	PLU_BasePhone *pCentPLU = 0;
	PLU_BasePhone *pLeftPLU = 0;
	PLU_BasePhone *pRightPLU = 0;
	void *val = 0;
	char szSubword[128];

	pCentPLU  = pResAM->basePLU + cent_pid;
	pLeftPLU  = pResAM->basePLU + left_pid;
	pRightPLU = pResAM->basePLU + right_pid;

	pCDPLU = pResAM->cxtdepPLU;

	sprintf(szSubword, "%s-%s+%s", pLeftPLU->szLeftCxt, pCentPLU->szCent, pRightPLU->szRightCxt);
	if (PowerASR_Base_lookupHashTable(ht_subword, szSubword, &val) == 0) {
		pCDPLU = pResAM->cxtdepPLU + (hci_uint32)val;
        return pCDPLU;
	}

	sprintf(szSubword, "%s+%s", pCentPLU->szCent, pRightPLU->szRightCxt);
	if (PowerASR_Base_lookupHashTable(ht_subword, szSubword, &val) == 0) {
		pCDPLU = pResAM->cxtdepPLU + (hci_uint32)val;
        return pCDPLU;
	}

	sprintf(szSubword, "%s-%s", pLeftPLU->szLeftCxt, pCentPLU->szCent);
	if (PowerASR_Base_lookupHashTable(ht_subword, szSubword, &val) == 0) {
        pCDPLU = pResAM->cxtdepPLU + (hci_uint32)val;
        return pCDPLU;
	}

	if (PowerASR_Base_lookupHashTable(ht_subword, pCentPLU->szName, &val) == 0) {
        pCDPLU = pResAM->cxtdepPLU + (hci_uint32)val;
        return pCDPLU;
	}

#endif	// #if (!defined(TIED_STATE) && !defined(USE_TRIP_GRAMMAR))

	return pCDPLU;
}

/* end of file */
