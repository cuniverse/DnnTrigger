
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
 *	@file	phoneseq2symbolseq.c
 *	@ingroup hmm_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	library to convert an input phone sequence into a symbol sequence
 */

#include <stdio.h>
#include <stdlib.h>
#ifndef WIN32
#include <string.h> // memset(), strcpy() etc.
#endif

#include "base/hci_type.h"
#include "base/hci_msg.h"

#include "hmm/hmm_common.h"
#include "hmm/dtree.h"

/**
 * convert an input base-phone sequence into a symbol sequence
 *	- Return value = length of output symbol sequence
 */
hci_uint32
AM_PhonoMap_convertBasePhoneSeq2SymbolSeq(const AM_Resource *pResAM,		///< (i) AM resource
										  hci_uint8 *pSymbolSeq,			///< (o) symbol sequence
										  hci_uint8 *flagSilence,			///< (o) preceding silence/short-pause flag sequence
										  const hci_uint8 *pBasePhoneSeq,	///< (i) base-phone sequence
										  const hci_uint32 lenBasePhoneSeq,	///< (i) length of base-phone sequence
										  const hci_flag nLexicalOrder,		///< (i) lexical order/direction (forward, reverse)
										  const hci_flag typeSymbol)		///< (i) symbol type (0=recognition, 1=reference)
{
#if defined(USE_PHONOMAP)
	//const DT_NODE *pDTNode = 0;
	//const QUEST_SET *pQSet = 0;
	const PLU_BasePhone *pCentPLU = 0;
	const hci_uint8 *pCurrBase = 0;
	//const hci_uint8 *pNextBase = 0;
	const hci_uint8 *pLastBase = 0;
	hci_uint8 *pOutSym = 0;
	hci_uint8 *pOutFlag = 0;
	hci_uint32 nLenOutSymbolSeq = 0;
	hci_uint16 cent_pid = 0, left_pid = 0, right_pid = 0;
	hci_uint16 idSilence = pResAM->hmm_info.idBaseSilence;
	hci_uint16 idShortPause = pResAM->hmm_info.idBaseShortPause;
	hci_uint16 idRootNode = 0;

	//if (0 == typeSymbol) {
	//	pDTNode = pResAM->nodeDT_Pho2RecSym;
	//}
	//else {
	//	pDTNode = pResAM->nodeDT_Pho2RefSym;
	//}
	//pQSet = &pResAM->questSet;

	if (flagSilence) {
		memset(flagSilence, 0, lenBasePhoneSeq*sizeof(hci_uint8));
	}

	left_pid = idSilence;
	pCurrBase = pBasePhoneSeq;
	pLastBase = pBasePhoneSeq + lenBasePhoneSeq;
	pOutSym = pSymbolSeq;
	if (flagSilence) {
		pOutFlag = flagSilence;
		while (pCurrBase < pLastBase) {
			if ((*pCurrBase) == idShortPause) {	// skip
				if (typeSymbol) {
					*pOutFlag = 1;
				}
			}
			else if ((*pCurrBase) == idSilence) { // skip, but contexts updated
				//if (typeSymbol) {
				//	*pOutFlag = 1;
				//}
				left_pid = idSilence;
			}
			else {
				cent_pid = (*pCurrBase);
				/*right_pid = idSilence;
				pNextBase = pCurrBase + 1;
				while (pNextBase < pLastBase) {
					if ((*pNextBase) != idShortPause) {
						right_pid = (*pNextBase);
						break;
					}
					pNextBase++;
				}*/
				pCentPLU = pResAM->basePLU + cent_pid;
				if (0 == typeSymbol) {
					idRootNode = pCentPLU->idRecRootNode;
				}
				else {
					idRootNode = pCentPLU->idRefRootNode;
				}
				/*if (nLexicalOrder) {
					*pOutSym = AM_DTREE_findTargetValueOfPhoneticContext(pDTNode,
																		 pQSet,
																		 idRootNode,
																		 right_pid,
																		 left_pid);
				}
				else {
					*pOutSym = AM_DTREE_findTargetValueOfPhoneticContext(pDTNode,
																		 pQSet,
																		 idRootNode,
																		 left_pid,
																		 right_pid);
				}*/
				*pOutSym = (hci_uint8)idRootNode;
				left_pid = cent_pid;
				pOutSym++;
				pOutFlag++;
			}
			pCurrBase++;
		}
	}
	else if (typeSymbol) {
		while (pCurrBase < pLastBase) {
			if ((*pCurrBase) == idShortPause) {	// skip
				;
			}
			else if ((*pCurrBase) == idSilence) { // skip, but contexts updated
				left_pid = idSilence;
			}
			else {
				cent_pid = (*pCurrBase);
				/*right_pid = idSilence;
				pNextBase = pCurrBase + 1;
				if (pNextBase < pLastBase) {
					if ((*pNextBase) != idShortPause) {
						right_pid = (*pNextBase);
					}
					else if (pNextBase+1 < pLastBase) {
						right_pid = *(pNextBase+1);
					}
				}*/
				pCentPLU = pResAM->basePLU + cent_pid;
				idRootNode = pCentPLU->idRefRootNode;
				/*if (nLexicalOrder) {
					*pOutSym = AM_DTREE_findTargetValueOfPhoneticContext(pDTNode,
																		 pQSet,
																		 idRootNode,
																		 right_pid,
																		 left_pid);
				}
				else {
					*pOutSym = AM_DTREE_findTargetValueOfPhoneticContext(pDTNode,
																		 pQSet,
																		 idRootNode,
																		 left_pid,
																		 right_pid);
				}*/
				*pOutSym = (hci_uint8)idRootNode;
				left_pid = cent_pid;
				pOutSym++;
			}
			pCurrBase++;
		}
	}
	else {
		while (pCurrBase < pLastBase) {
			if ((*pCurrBase) == idShortPause) {	// skip
				;
			}
			else if ((*pCurrBase) == idSilence) { // skip, but contexts updated
				left_pid = idSilence;
			}
			else {
				cent_pid = (*pCurrBase);
				/*right_pid = idSilence;
				pNextBase = pCurrBase + 1;
				while (pNextBase < pLastBase) {
					if ((*pNextBase) != idShortPause) {
						right_pid = (*pNextBase);
						break;
					}
					pNextBase++;
				}*/
				pCentPLU = pResAM->basePLU + cent_pid;
				idRootNode = pCentPLU->idRecRootNode;
				/*if (nLexicalOrder) {
					*pOutSym = AM_DTREE_findTargetValueOfPhoneticContext(pDTNode,
																		 pQSet,
																		 idRootNode,
																		 right_pid,
																		 left_pid);
				}
				else {
					*pOutSym = AM_DTREE_findTargetValueOfPhoneticContext(pDTNode,
																		 pQSet,
																		 idRootNode,
																		 left_pid,
																		 right_pid);
				}*/
				*pOutSym = (hci_uint8)idRootNode;
				left_pid = cent_pid;
				pOutSym++;
			}
			pCurrBase++;
		}
	}
	nLenOutSymbolSeq = pOutSym - pSymbolSeq;

	return nLenOutSymbolSeq;

#else	// !USE_PHONOMAP

	return 0;

#endif	// #if defined(USE_PHONOMAP)
}


/* end of file */






















