
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
 *	@file	fx_dropframe.c
 *	@ingroup mfcc2feat_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	silence frame dropping library
 */

#include <stdio.h>
#include <stdlib.h>

#include "base/hci_type.h"
#include "basic_op/basic_op.h"
#include "mfcc2feat/fx_dropframe.h"

/**
 *	drop the frame with long left/right silence periods
 */
MFCC_Cell*
FX_FrameDrop_dropFrameWithLongSilence(MFCC_Stream *pMfccStream,		///< (i) mfcc stream pool
									  hci_int32 cur_mfcc_cell,		///< (i) current mfcc cell position
									  hci_int16 winSilDrop)			///< (i) window length for frame dropping
{
	MFCC_Cell *pCurCell = 0;
	MFCC_Cell *pProcCell = 0;
	hci_int32 proc_cell_id = 0;

	pCurCell = pMfccStream->mfccPool + cur_mfcc_cell;

	if (pCurCell->frame_class == SIL_FRAME) {
        pMfccStream->curLenSilence = PowerASR_BasicOP_add_32_32(pMfccStream->curLenSilence, 1);
	}
	else {
		pMfccStream->curLenSilence = 0;
	}

	if (pMfccStream->curLenSilence > 2*winSilDrop) {
		proc_cell_id = (pMfccStream->maxLenStream+cur_mfcc_cell-winSilDrop)%pMfccStream->maxLenStream;
		pProcCell = pMfccStream->mfccPool + proc_cell_id;
		pProcCell->bActive = DROP_MFCC;
		return pProcCell;
	}
	else {
		return 0;
	}
}


/* end of file */






















