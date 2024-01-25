
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
 *	@file	fx_delta.c
 *	@ingroup mfcc2feat_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	time-derivative feature extraction library
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "base/hci_macro.h"
#include "basic_op/basic_op.h"
#include "mfcc2feat/fx_delta.h"

/**
 * compute time derivative coefficients in live mode
 */
M2F_Status
FX_DELTA_computeLiveDerivatives(MFCC_Stream *pMfccStream,		///< (i/o) pointer to mfcc stream
								hci_mfcc_t *pOutMfccVec,		///< (o) output mfcc vector
								MFCC_Cell *pCurCell,			///< (i) current mfcc cell
								Mfcc2FeatParameters *pFXVar)	///< (i) config. struct for mfcc-to-feature converter
{
	hci_mfcc_t *leftMfcc = 0;
	hci_mfcc_t *rightMfcc = 0;
	hci_mfcc_t *centMfcc = 0;
	hci_mfcc_t *lastMfcc = 0;
	hci_mfcc_t *derivMfcc = 0;
	MFCC_Cell *pFirstCell = 0;
	MFCC_Cell *pLastCell = 0;
	MFCC_Cell *pFrameCell = 0;
	hci_int16 nMove = 0;
	hci_int16 nLenMove = 0;

	if (ACTIVE_MFCC != pCurCell->bActive) {
		if (DROP_MFCC == pCurCell->bActive) {
			return M2F_DROP;
		}
		else {
			return M2F_SKIP;
		}
	}

	memcpy(pOutMfccVec, pCurCell->featVec, sizeof(pCurCell->featVec));

	if (pFXVar->bUseLogE) {
		derivMfcc = pOutMfccVec + pFXVar->dimMFCC;
	}
	else {
		derivMfcc = pOutMfccVec + pFXVar->dimMFCC - 1;
	}
	pFirstCell = pMfccStream->mfccPool;
	pLastCell  = pMfccStream->mfccPool + (pMfccStream->maxLenStream-1);

	// compute differential coefficients ( &C(t) = C(t-2) - C(t+2) )
	if (pFXVar->bUseDif) {
		// find left-side feature vector
		leftMfcc = pCurCell->mfccVec;
		nLenMove = HCI_MIN(pFXVar->winDif, pMfccStream->nLenOutputMfcc);
		if (pCurCell == pFirstCell) pFrameCell = pLastCell;
		else pFrameCell = pCurCell - 1;
		nMove = 0;
		while (TRUE == pFrameCell->bSpeech && nMove < nLenMove) {
			if (pFrameCell->bActive != NULL_MFCC) {
				leftMfcc = pFrameCell->mfccVec;
			}
			nMove++;
			if (pFrameCell == pFirstCell) pFrameCell = pLastCell;
			else pFrameCell = pFrameCell - 1;
		}

		// find right-side feature vector
		nLenMove = pFXVar->winDif;
		rightMfcc = pCurCell->mfccVec;
		if (pCurCell == pLastCell) pFrameCell = pFirstCell;
		else pFrameCell = pCurCell + 1;
		nMove = 0;
		while (nMove < nLenMove && pCurCell->idInFrame < pFrameCell->idInFrame) {
			if (pFrameCell->bActive != NULL_MFCC) {
				rightMfcc = pFrameCell->mfccVec;
			}
			nMove++;
			if (pFrameCell == pLastCell) pFrameCell = pFirstCell;
			else pFrameCell = pFrameCell + 1;
		}

		// compute differential coefficients
		lastMfcc = derivMfcc + pFXVar->dimMFCC;
		while (derivMfcc < lastMfcc) {
			*derivMfcc++ = (*leftMfcc++) - (*rightMfcc++);
		}
	}

	// compute acceleration coefficients ( &&C(t) = C(t-4) - 2C(t) + C(t+4) )
	if (pFXVar->bUseAcc) {
		// assign center feature vector
		centMfcc = pCurCell->mfccVec;

		// find left-side feature vector
		leftMfcc = pCurCell->mfccVec;
		nLenMove = HCI_MIN(pFXVar->winAcc, pMfccStream->nLenOutputMfcc);
		if (pCurCell == pFirstCell) pFrameCell = pLastCell;
		else pFrameCell = pCurCell - 1;
		nMove = 0;
		while (TRUE == pFrameCell->bSpeech && nMove < nLenMove) {
			if (pFrameCell->bActive != NULL_MFCC) {
				leftMfcc = pFrameCell->mfccVec;
			}
			nMove++;
			if (pFrameCell == pFirstCell) pFrameCell = pLastCell;
			else pFrameCell = pFrameCell - 1;
		}

		// find right-side feature vector
		rightMfcc = pCurCell->mfccVec;
		nLenMove = pFXVar->winAcc;
		if (pCurCell == pLastCell) pFrameCell = pFirstCell;
		else pFrameCell = pCurCell + 1;
		nMove = 0;
		while (nMove < nLenMove && pCurCell->idInFrame < pFrameCell->idInFrame) {
			if (pFrameCell->bActive != NULL_MFCC) {
				rightMfcc = pFrameCell->mfccVec;
			}
			nMove++;
			if (pFrameCell == pLastCell) pFrameCell = pFirstCell;
			else pFrameCell = pFrameCell + 1;
		}

		// compute acceleration coefficients
		lastMfcc = derivMfcc + pFXVar->dimMFCC;
		while (derivMfcc < lastMfcc) {
			*derivMfcc++ = (*leftMfcc++) + (*rightMfcc++) - 2 * (*centMfcc++);
		}
	}

	// compute third differential coefficients ( &&&C(t) = C(t-5) - C(t-3) - 2C(t-1) + 2C(t+1) + C(t+3) - C(t+5) )
	if (pFXVar->bUse3rdDif) {
		hci_mfcc_t *mfccContext[6];
		hci_mfcc_t L_val_feat = 0;
		hci_int16 sizeContext = 6;
		hci_int16 iCxt = 0, iDim = 0;

		// initialize
		for ( iCxt = 0; iCxt < sizeContext ; iCxt++ ) {
			mfccContext[iCxt] = pCurCell->mfccVec;
		}

		// find left-side feature vector
		nLenMove = HCI_MIN(pFXVar->win3rdDif, pMfccStream->nLenOutputMfcc);
		if (pCurCell == pFirstCell) pFrameCell = pLastCell;
		else pFrameCell = pCurCell - 1;
		nMove = 0;
		while ( TRUE == pFrameCell->bSpeech && nMove < nLenMove ) {
			if ( pFrameCell->bActive != NULL_MFCC ) {
				if ( nMove <= 0 ) mfccContext[2] = pFrameCell->mfccVec;
				if ( nMove <= 2 ) mfccContext[1] = pFrameCell->mfccVec;
				mfccContext[0] = pFrameCell->mfccVec;
			}
			nMove++;
			if (pFrameCell == pFirstCell) pFrameCell = pLastCell;
			else pFrameCell = pFrameCell - 1;
		}
		
		// find right-side feature vector
		nLenMove = pFXVar->win3rdDif;
		if (pCurCell == pLastCell) pFrameCell = pFirstCell;
		else pFrameCell = pCurCell + 1;
		nMove = 0;
		while ( nMove < nLenMove && pCurCell->idInFrame < pFrameCell->idInFrame ) {
			if ( pFrameCell->bActive != NULL_MFCC ) {
				if ( nMove <= 0 ) mfccContext[3] = pFrameCell->mfccVec;
				if ( nMove <= 2 ) mfccContext[4] = pFrameCell->mfccVec;
				mfccContext[5] = pFrameCell->mfccVec;
			}
			nMove++;
			if (pFrameCell == pLastCell) pFrameCell = pFirstCell;
			else pFrameCell = pFrameCell + 1;
		}
		
		// compute third differential coefficients
		lastMfcc = derivMfcc + pFXVar->dimMFCC;
		iDim     = 0;
		while (derivMfcc < lastMfcc) {
			L_val_feat   = mfccContext[0][iDim] - mfccContext[1][iDim];
			L_val_feat  -= 2 * mfccContext[2][iDim];
			L_val_feat  += 2 * mfccContext[3][iDim];
			L_val_feat  += mfccContext[4][iDim];
			L_val_feat  -= mfccContext[5][iDim];
			*derivMfcc++ = L_val_feat;
			iDim++;
		}

	}

    pMfccStream->nLenOutputMfcc = PowerASR_BasicOP_add_32_32(pMfccStream->nLenOutputMfcc, 1);

	return M2F_TRUE;
}


/* end of file */






















