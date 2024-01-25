
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
 *	@file	hci_resource_hmm.c
 *	@ingroup hmm_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	PowerASR acoustic-model resource manager library
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "base/hci_malloc.h"
#include "base/hci_msg.h"
#include "base/parse_config.h"
#include "base/case.h"
#include "base/hash_table.h"

#include "hmm/hci_resource_hmm.h"
#include "hmm/load_hmm.h"
#include "hmm/dtree.h"
#include "hmm/phoneseq2statenet.h"
#include "hmm/phoneseq2symbolseq.h"

/** inner data struct for acoustic model resource manager */
typedef struct 
{
	hash_table_t *ht_basephone;		///< hash table for base-phone list
	hash_table_t *ht_subword;		///< hast table for sub-word PLU list
} Resource_HMM_Inner;


/**
 *	create a new HMM resource manager.
 *
 *	@return Return the pointer to a newly created HMM resource manager
 */
HCILAB_PUBLIC HCI_HMM_API PowerASR_Resource_HMM*
PowerASR_Resource_HMM_new()
{
	PowerASR_Resource_HMM *pResourceHMM = 0;
	Resource_HMM_Inner *pInner = 0;

	pResourceHMM = (PowerASR_Resource_HMM *) hci_malloc( sizeof(PowerASR_Resource_HMM) );

	if ( pResourceHMM ) {
		memset(pResourceHMM, 0, sizeof(PowerASR_Resource_HMM));

		pInner = (Resource_HMM_Inner *) hci_malloc( sizeof(Resource_HMM_Inner) );
		if ( pInner ) {
			memset(pInner, 0, sizeof(Resource_HMM_Inner));
			pResourceHMM->pInner = (void *)pInner;
		}
	}
	else {
		HCIMSG_ERROR("cannot create HMM resource manager.\n");
	}

	return pResourceHMM;
}


/**
 *	delete the HMM resource manager.
 */
HCILAB_PUBLIC HCI_HMM_API void
PowerASR_Resource_HMM_delete(PowerASR_Resource_HMM *pThis)
{
	Resource_HMM_Inner *pInner = 0;

	if (0 == pThis) {
		return;
	}

	pInner = (Resource_HMM_Inner *) pThis->pInner;

	if ( pInner ) hci_free(pInner);
	hci_free(pThis);
}


/**
 *	load acoustic model resource.
 *
 *	@return Return 0 if acoustic model resource are loaded successfully, otherwise return -1.
 */
HCILAB_PUBLIC HCI_HMM_API hci_int32
PowerASR_Resource_HMM_openHmmResource(PowerASR_Resource_HMM *pThis,		///< (i/o) pointer to the AM resource manager
									  const char *pszResourceHMMFile)	///< (i) HMM resource file
{
	Resource_HMM_Inner *pInner = 0;
	hci_int32 bSetup = 0;

	if (0 == pThis) {
		return -1;
	}
	if (0 == pszResourceHMMFile) {
		return -1;
	}

	pInner = (Resource_HMM_Inner *) pThis->pInner;

	pInner->ht_basephone = 0;
	pInner->ht_subword = 0;

	bSetup = AM_HMM_loadBinaryHMMResource(pszResourceHMMFile, &pThis->resAM);

	if (!bSetup) {
		pInner->ht_basephone = AM_PhoSeq2Net_createBasePhoneHashTable(&pThis->resAM);
#if (!defined(TIED_STATE) && !defined(USE_TRIP_GRAMMAR))
		pInner->ht_subword = AM_PhoSeq2Net_createSubwordHashTable(&pThis->resAM);
#endif // #ifndef TIED_STATE
	}

	return bSetup;
}


/**
 *	free memories allocated to the HMM resource manager.
 *
 *	@return Return 0 if all memories are freed successfully, otherwise return -1.
 */
HCILAB_PUBLIC HCI_HMM_API hci_int32
PowerASR_Resource_HMM_closeHmmResource(PowerASR_Resource_HMM *pThis)		///< (i/o) pointer to the AM resource manager
{
	Resource_HMM_Inner *pInner = 0;

	if (0 == pThis) {
		return -1;
	}

	pInner = (Resource_HMM_Inner *) pThis->pInner;

	if ( pThis->resAM.multiHMM ) {
		hci_free(pThis->resAM.multiHMM );
		pThis->resAM.multiHMM = 0;
	}

	if (pInner->ht_basephone) {
		AM_PhoSeq2Net_deleteSubwordHashTable(pInner->ht_basephone);
	}
	if (pInner->ht_subword) {
		AM_PhoSeq2Net_deleteSubwordHashTable(pInner->ht_subword);
	}

	return 0;
}


/**
 *	initialize data buffers for HMM resource manager.
 *
 *	@return Return 0 if user-specific data buffers are initialized successfully, otherwise return -1.
 */
HCILAB_PUBLIC HCI_HMM_API hci_int32
PowerASR_Resource_HMM_initializeHmmResource(PowerASR_Resource_HMM *pThis)		///< (i) pointer to the AM resource manager
{
	return (pThis ? 0 : -1);
}


/**
 *	convert a base-phone sequence into a HMM state sequence.
 *
 *	@return Return 0 if PhoneSeq2StateNet operation is performed successfully, otherwise return -1.
 *
 *	@see AM_PhoSeq2Net_convertBasePhoneSeq2StateNetwork
 */
HCILAB_PUBLIC HCI_HMM_API hci_int32
PowerASR_Resource_HMM_convertBasePhoneSeq2StateNetwork(PowerASR_Resource_HMM *pThis,	///< (i) pointer to the HMM resource manager
													   StateNetwork *stateNet,			///< (o) HMM state network
													   const hci_uint8 *BasePhoneSeq,	///< (i) base-phone index sequence
													   const hci_uint32 lenBasePhoneSeq,	///< (i) length of base-phone sequence
													   const hci_uint32 nLexicalOrder)	///< (i) lexical order/direction (normal=0, reverse=1)
{
	hci_int32 flag_Phone2Net = 0;
	Resource_HMM_Inner *pInner = 0;

	if (0 == pThis) {
		return -1;
	}

	if (0 == BasePhoneSeq || 0 == lenBasePhoneSeq) {
		return -1;
	}

	pInner = (Resource_HMM_Inner *) pThis->pInner;

	flag_Phone2Net = AM_PhoSeq2Net_convertBasePhoneSeq2StateNetwork(&pThis->resAM,
																	pInner->ht_subword,
																	stateNet,
																	BasePhoneSeq,
																	lenBasePhoneSeq,
																	nLexicalOrder);

	return flag_Phone2Net;
}


/**
 *	convert a sub-word PLU sequence into a HMM state sequence.
 *
 *	@return Return 0 if PhoneSeq2StateNet operation is performed successfully, otherwise return -1.
 *
 *	@see AM_PhoSeq2Net_convertSubwordSeq2StateNetwork
 */
HCILAB_PUBLIC HCI_HMM_API hci_int32
PowerASR_Resource_HMM_convertSubwordSeq2StateNetwork(PowerASR_Resource_HMM *pThis,	///< (i) pointer to the HMM resource manager
													 StateNetwork *stateNet,		///< (o) HMM state network
													 const hci_uint16 *SubwordSeq,	///< (i) sub-word PLU index sequence
													 const hci_uint32 lenSubwordSeq)	///< (i) length of sub-word PLU sequence
{
	hci_int32 flag_Phone2Net = 0;

	if (0 == pThis) {
		return -1;
	}

	if (0 == SubwordSeq || 0 == lenSubwordSeq) {
		return -1;
	}

	flag_Phone2Net = AM_PhoSeq2Net_convertSubwordSeq2StateNetwork(&pThis->resAM,
																  stateNet,
																  SubwordSeq,
																  lenSubwordSeq);

	return flag_Phone2Net;
}


/**
 *	convert a base-phone sequence into a recognition symbol sequence.
 *
 *	@return Return the length of recognition symbol sequence.
 *
 *	@see AM_PhonoMap_convertBasePhoneSeq2SymbolSeq
 */
HCILAB_PUBLIC HCI_HMM_API hci_uint32
PowerASR_Resource_HMM_convertBasePhoneSeq2RecSymbolSeq(PowerASR_Resource_HMM *pThis,	///< (i) pointer to the HMM resource manager
													   hci_uint8 *RecSymbolSeq,			///< (o) recognition symbol index sequence
													   const hci_uint8 *BasePhoneSeq,	///< (i) base-phone index sequence
													   const hci_uint32 lenBasePhoneSeq,	///< (i) length of base-phone sequence
													   const hci_flag nLexicalOrder)	///< (i) lexical order/direction (normal=0, reverse=1)
{
	hci_uint32 lenSymbolSeq = 0;

	if (0 == pThis) {
		return lenSymbolSeq;
	}

	if (0 == BasePhoneSeq || 0 == lenBasePhoneSeq) {
		return lenSymbolSeq;
	}

	lenSymbolSeq = AM_PhonoMap_convertBasePhoneSeq2SymbolSeq(&pThis->resAM,
															 RecSymbolSeq,
															 0,
															 BasePhoneSeq,
															 lenBasePhoneSeq,
															 nLexicalOrder,
															 0);

	return lenSymbolSeq;
}


/**
 *	convert a base-phone sequence into a reference symbol sequence.
 *
 *	@return Return the length of reference symbol sequence.
 *
 *	@see AM_PhonoMap_convertBasePhoneSeq2SymbolSeq
 */
HCILAB_PUBLIC HCI_HMM_API hci_uint32
PowerASR_Resource_HMM_convertBasePhoneSeq2RefSymbolSeq(PowerASR_Resource_HMM *pThis,	///< (i) pointer to the HMM resource manager
													   hci_uint8 *RefSymbolSeq,			///< (o) reference symbol index sequence
													   hci_uint8 *flagSilence,			///< (o) preceding silence/short-pause flag sequence
													   const hci_uint8 *BasePhoneSeq,	///< (i) base-phone index sequence
													   const hci_uint32 lenBasePhoneSeq,	///< (i) length of base-phone sequence
													   const hci_flag nLexicalOrder)	///< (i) lexical order/direction (normal=0, reverse=1)
{
	hci_uint32 lenSymbolSeq = 0;

	if (0 == pThis) {
		return lenSymbolSeq;
	}

	if (0 == BasePhoneSeq || 0 == lenBasePhoneSeq) {
		return lenSymbolSeq;
	}

	lenSymbolSeq = AM_PhonoMap_convertBasePhoneSeq2SymbolSeq(&pThis->resAM,
															 RefSymbolSeq,
															 flagSilence,
															 BasePhoneSeq,
															 lenBasePhoneSeq,
															 nLexicalOrder,
															 1);

	return lenSymbolSeq;
}


/**
 *	convert symbol sequence into base-phone sequence.
 *
 *	@return Return the length of output base-phone sequence.
 */
HCILAB_PUBLIC HCI_HMM_API hci_uint32
PowerASR_Resource_HMM_convertSymbolSequence2BasePhoneSequence(PowerASR_Resource_HMM *pThis,		///< (i) pointer to the HMM resource manager
															  hci_uint8 *basephoneSeq,			///< (o) base-phone index sequence
															  const FullSymbolLex *symbolSeq,	///< (i) symbol index sequence
															  const hci_flag bReverse)			///< (i) flag to reverse phone sequence
{
	hci_uint32 nLenPhoneSeq = 0;

#if defined(USE_PHONOMAP)

	AM_Resource *pResAM = 0;
	const hci_uint8 *pSymbol2Phone = 0;
	const FullSymbolLex *pSym = symbolSeq + 1;
	const FullSymbolLex *pLastSym = pSym + symbolSeq->idSymbol;
	hci_uint8 *pOutPhone = basephoneSeq;
	hci_uint8 idBaseSilence = 0;
	hci_uint8 idBaseShortPause = 0;

	if (0 == pThis) {
		return nLenPhoneSeq;
	}

	pResAM           = &pThis->resAM;
	idBaseSilence    = (hci_uint8)pResAM->hmm_info.idBaseSilence;
	idBaseShortPause = (hci_uint8)pResAM->hmm_info.idBaseShortPause;
	pSymbol2Phone    = pResAM->phonoMap.symbol2phone;

	*pOutPhone++ = idBaseSilence;
	*pOutPhone++ = pSymbol2Phone[pSym->idSymbol];
	pSym++;
	while (pSym < pLastSym) {
		if (pSym->flagSilence) {
			*pOutPhone++ = idBaseShortPause;
			*pOutPhone++ = pSymbol2Phone[pSym->idSymbol];
		}
		else {
			*pOutPhone++ = pSymbol2Phone[pSym->idSymbol];
		}
		pSym++;
	}
	*pOutPhone++ = idBaseSilence;

	nLenPhoneSeq = pOutPhone - basephoneSeq;

	if (bReverse && nLenPhoneSeq) {
		hci_uint8 *phoneSeq = basephoneSeq;
		hci_uint8 utemp = 0;
		hci_uint32 iPho = 0;
		hci_uint32 iRevPho = nLenPhoneSeq - 1;
		hci_uint32 nLenHalf = (nLenPhoneSeq>>1);
		for (iPho = 0; iPho < nLenHalf; iPho++, iRevPho--) {
			utemp = *(phoneSeq+iPho);
			*(phoneSeq+iPho) = *(phoneSeq+iRevPho);
			*(phoneSeq+iRevPho) = utemp;
		}
	}

#endif // #if defined(USE_PHONOMAP)

	return nLenPhoneSeq;

}


/**
 *	given phonetic contexts, find a reference symbol index.
 *
 *	@return Return a reference symbol index.
 */
HCILAB_PUBLIC HCI_HMM_API hci_uint16
PowerASR_Resource_HMM_findReferenceSymbol(PowerASR_Resource_HMM *pThis,		///< (i) pointer to the HMM resource manager
										  const hci_uint16 cent_phone,		///< (i) center base-phone index
										  const hci_uint16 left_cxt,		///< (i) left-side phonetic context
										  const hci_uint16 right_cxt)		///< (i) right-side phonetic context
{
#if defined(USE_PHONOMAP)
	AM_Resource *pResAM = 0;
	//DT_NODE *pDTNode = 0;
	//QUEST_SET *pQSet = 0;
	PLU_BasePhone *pCentPLU = 0;
	hci_uint16 idRootNode = 0;

	pResAM = &pThis->resAM;
	//pDTNode = pResAM->nodeDT_Pho2RefSym;
	//pQSet = &pResAM->questSet;

	pCentPLU = pResAM->basePLU + cent_phone;
	idRootNode = pCentPLU->idRefRootNode;

	//return AM_DTREE_findTargetValueOfPhoneticContext(pDTNode,
	//												 pQSet,
	//												 idRootNode,
	//												 left_cxt,
	//												 right_cxt);

	return idRootNode;

#else	// !USE_PHONOMAP

	return 0u;

#endif	// #if defined(USE_PHONOMAP)
}


/**
 *	return base-phone index with the same base-phone string.
 *
 *	@return Return base-phone index.
 */
HCILAB_PUBLIC HCI_HMM_API hci_uint8
PowerASR_Resource_HMM_findBasePhoneIndex(PowerASR_Resource_HMM *pThis,		///< (i) pointer to the HMM resource manager
										 const char *szPhone)				///< (i) base-phone string
{
	Resource_HMM_Inner *pInner = 0;
	void *val = 0;

	if (0 == pThis) {
		return 0u;
	}

	if (0 == szPhone) {
		return 0u;
	}

	pInner = (Resource_HMM_Inner *) pThis->pInner;

	if (PowerASR_Base_lookupHashTable(pInner->ht_basephone, szPhone, &val) == 0) {
		return (hci_uint8)val;
	}

	HCIMSG_WARN("cannot find base-phone (%s).\n", szPhone);
	return 0u;
}


// end of file
