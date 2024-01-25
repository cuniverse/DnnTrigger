
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
 *	@file	load_hmm.c
 *	@ingroup hmm_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	aouctic model resource loading library
 */

#include <stdio.h>
#include <stdlib.h>

#include "base/hci_msg.h"
#include "base/hci_macro.h"
#include "base/hci_malloc.h"

#include "hmm/hmm_common.h"

// local functions
#ifdef __cplusplus
extern "C" {
#endif 

/**
 *	check the overflow status of AM resource
 */
HCILAB_PRIVATE hci_int32
_HMM_check_AM_Resource(HMM_INFO *pHmmInfo);		///< (i) HMM header data

#ifdef __cplusplus
}
#endif

/**
 *	load binary-format AM resource
 */
hci_int32 
AM_HMM_loadBinaryHMMResource(const char *pszResourceHMMFile, 	///< (i) AM resource file
							 AM_Resource *pResAM)				///< (o) pointer to AM resource
{
	FILE *fpHMM = 0;
	HMM_PARA *pHmmPara = 0;
	hci_uint32 n = 0;
	hci_uint32 nLoadSize = 0;

	fpHMM = fopen(pszResourceHMMFile, "rb");
	if (0 == fpHMM) {
		HCIMSG_ERROR("cannot open HMM resource file [rb] : %s\n", pszResourceHMMFile);
		return -1;
	}

// 	nLoadSize = fread(pResAM, sizeof(AM_Resource), 1, fpHMM);
// 	fclose(fpHMM);
// 
// 	if (0 == nLoadSize) {
// 		HCIMSG_WARN("loading size of AM_Resource mismatched.");
// 		return -1;
// 	}

	nLoadSize += sizeof(HMM_INFO);
	nLoadSize += BASEPHONE_SIZE * sizeof(PLU_BasePhone);
#if (!defined(TIED_STATE) || defined(USE_TRIP_GRAMMAR))
	nLoadSize += SUBWORD_SIZE * sizeof(PLU_Subword);
#endif	// #if (!defined(TIED_STATE) || defined(USE_TRIP_GRAMMAR))
	nLoadSize += sizeof(PLU_Grammar);
	nLoadSize += BASESTATE_SIZE * sizeof(HMM_BaseState);
	nLoadSize += STATE_SIZE * sizeof(HMM_STATE);
#if defined(SDCHMM)
	nLoadSize += sizeof(SubSpace);
#endif	// #if defined(SDCHMM)
	nLoadSize += TOTAL_GMM_SIZE * sizeof(GMM);
#if (defined(TIED_STATE) && !defined(USE_DT_TABLE) && !defined(USE_LHMM2PHMM_TABLE))
	nLoadSize += DT_SIZE_STATETYING * sizeof(DT_NODE);
#endif	// #if (defined(TIED_STATE) && !defined(USE_DT_TABLE) && !defined(USE_LHMM2PHMM_TABLE))
#if defined(USE_PHONOMAP)
	nLoadSize += sizeof(PhoneConfusMat);
#endif	// #if defined(USE_PHONOMAP)
#if ((defined(USE_PHONOMAP) || defined(TIED_STATE)) && !defined(USE_DT_TABLE) && !defined(USE_LHMM2PHMM_TABLE))
	nLoadSize += sizeof(QUEST_SET);
#endif	// #if (defined(USE_PHONOMAP) || defined(TIED_STATE))

	fread(pResAM, sizeof(char), nLoadSize, fpHMM);

	if ( pResAM->hmm_info.numHMMs ) {
		pResAM->multiHMM = (HMM_PARA *) hci_calloc( pResAM->hmm_info.numHMMs, sizeof(HMM_PARA) );
	}

	for ( n = 0 ; n < pResAM->hmm_info.numHMMs; n++) {
		pHmmPara = pResAM->multiHMM + n;
		fread(pHmmPara, sizeof(HMM_PARA), 1, fpHMM);
	}

	fclose(fpHMM); fpHMM = 0;
	
	// check the overflow status of AM resource
	if (-1 == _HMM_check_AM_Resource(&pResAM->hmm_info)) {
		return -1;
	}

	{
		PhoneConfusMat *pConfusMat = 0;
		hci_uint32 n = 0;
		hci_int16 nPenalty = 0;
		hci_int16 nProb = 0;
		hci_int16 nMinProb = (-100);
		hci_int16 nMaxProb = 0;

		pConfusMat = &pResAM->phonoMap;

		nPenalty = (-10);
		for (n=0; n < pResAM->hmm_info.numRecSymbols; n++) {
			nProb = pConfusMat->ins_prob[n] + nPenalty;
			if (nProb < nMinProb) {
				pConfusMat->ins_prob[n] = (hci_int8)nMinProb;
			}
			else if(nProb > nMaxProb ) {
				pConfusMat->ins_prob[n] = (hci_int8)nMaxProb;
			}
			else {
				pConfusMat->ins_prob[n] = (hci_int8)nProb;
			}
		}

		nPenalty = 10;
		for (n=0; n < pResAM->hmm_info.numRefSymbols; n++) {
			nProb = pConfusMat->del_prob[n] + nPenalty;
			if (nProb < nMinProb) {
				pConfusMat->del_prob[n] = (hci_int8)nMinProb;
			}
			else if (nProb > nMaxProb) {
				pConfusMat->del_prob[n] = (hci_int8)nMaxProb;
			}
			else {
				pConfusMat->del_prob[n] = (hci_int8)nProb;
			}
		}

	}

	return 0;
}


/**
 *	check the overflow status of AM resource
 */
HCILAB_PRIVATE hci_int32
_HMM_check_AM_Resource(HMM_INFO *pHmmInfo)		///< (i) HMM header data
{

	if (pHmmInfo->numBasePhones > BASEPHONE_SIZE) {
		HCIMSG_WARN("BASEPHONE_SIZE overflow (%hu/%hu).\n", pHmmInfo->numBasePhones, BASEPHONE_SIZE);
		return -1;
	}

#if (!defined(TIED_STATE) || defined(USE_TRIP_GRAMMAR))
	if (pHmmInfo->numSubwords > SUBWORD_SIZE) {
		HCIMSG_WARN("SUBWORD_SIZE overflow (%hu/%hu).\n", pHmmInfo->numSubwords, SUBWORD_SIZE);
		return -1;
	}
#endif	// #if (!defined(TIED_STATE) || defined(USE_TRIP_GRAMMAR))

	if (pHmmInfo->numBaseStates > BASESTATE_SIZE) {
		HCIMSG_WARN("BASESTATE_SIZE overflow (%hu/%hu).\n", pHmmInfo->numBaseStates, BASESTATE_SIZE);
		return -1;
	}

	if (pHmmInfo->numHmmStates > STATE_SIZE) {
		HCIMSG_WARN("STATE_SIZE overflow (%hu/%hu).\n", pHmmInfo->numHmmStates, STATE_SIZE);
		return -1;
	}

	if (pHmmInfo->numGMMs > TOTAL_GMM_SIZE) {
		HCIMSG_WARN("TOTAL_GMM_SIZE overflow (%hu/%hu).\n", pHmmInfo->numGMMs, TOTAL_GMM_SIZE);
		return -1;
	}

	if (pHmmInfo->numGaussPDFs > PDF_SIZE) {
		HCIMSG_WARN("PDF_SIZE overflow (%hu/%hu).\n", pHmmInfo->numGaussPDFs, PDF_SIZE);
		return -1;
	}

	if (pHmmInfo->sizeGaussVec > PDF_DIM) {
		HCIMSG_WARN("PDF_DIM overflow (%lu/%lu).\n", pHmmInfo->sizeGaussVec, PDF_DIM);
		return -1;
	}

#if !defined(CDHMM)
	if (pHmmInfo->sizeGaussWgt > GAUSS_WGT_SIZE) {
		HCIMSG_WARN("GAUSS_WGT_SIZE overflow (%lu/%lu).\n", pHmmInfo->sizeGaussWgt, GAUSS_WGT_SIZE);
		return -1;
	}
#endif	// #ifndef CDHMM

#if defined(USE_TRIP_GRAMMAR)
	if (pHmmInfo->sizePhonePair > PLU_PAIR_SIZE) {
		HCIMSG_WARN("PLU_PAIR_SIZE overflow (%lu/%lu).\n", pHmmInfo->sizePhonePair, PLU_PAIR_SIZE);
		return -1;
	}
#endif	// #if defined(USE_TRIP_GRAMMAR)

	if (pHmmInfo->featStreamWidth > DIM_FEATURE) {
		HCIMSG_WARN("DIM_FEATURE overflow (%hu/%hu).\n", pHmmInfo->featStreamWidth, DIM_FEATURE);
		return -1;
	}

#if defined(TIED_STATE)
	//if (pHmmInfo->numNodes_StateTying > DT_SIZE_STATETYING) {
	//	HCIMSG_WARN("DT_SIZE_STATETYING overflow (%hu/%hu).\n", pHmmInfo->numNodes_StateTying, DT_SIZE_STATETYING);
	//	return -1;
	//}
#endif	// #if defined(TIED_STATE)

#if defined(USE_PHONOMAP)
	if (pHmmInfo->numRecSymbols > REC_SYMBOL_SIZE) {
		HCIMSG_WARN("REC_SYMBOL_SIZE overflow (%hu/%hu).\n", pHmmInfo->numRecSymbols, REC_SYMBOL_SIZE);
		return -1;
	}
	if (pHmmInfo->numRefSymbols > REF_SYMBOL_SIZE) {
		HCIMSG_WARN("REF_SYMBOL_SIZE overflow (%hu/%hu).\n", pHmmInfo->numRefSymbols, REF_SYMBOL_SIZE);
		return -1;
	}
	//if (pHmmInfo->numNodes_Pho2RecSym > DT_SIZE_PHO2RECSYM) {
	//	HCIMSG_WARN("DT_SIZE_PHO2RECSYM overflow (%hu/%hu).\n", pHmmInfo->numNodes_Pho2RecSym, DT_SIZE_PHO2RECSYM);
	//	return -1;
	//}
	//if (pHmmInfo->numNodes_Pho2RefSym > DT_SIZE_PHO2REFSYM) {
	//	HCIMSG_WARN("DT_SIZE_PHO2REFSYM overflow (%hu/%hu).\n", pHmmInfo->numNodes_Pho2RefSym, DT_SIZE_PHO2REFSYM);
	//	return -1;
	//}
#endif	// #if defined(USE_PHONOMAP)

	if (pHmmInfo->sizeDistTable > SIZE_DIST_TABLE) {
		HCIMSG_WARN("SIZE_DIST_TABLE overflow (%lu/%lu).\n", pHmmInfo->sizeDistTable, SIZE_DIST_TABLE);
		return -1;
	}

	return 0;
}


// end of file
