
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
 *	@file	fx_compressframe.h
 *	@ingroup mfcc2feat_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	frame compression library
 */

#ifndef __FX_COMPRESSFRAME_H__
#define __FX_COMPRESSFRAME_H__

#include "base/hci_type.h"
#include "basic_op/fixpoint.h"
#include "wave2mfcc/fx_mfcc_common.h"
#include "fx_mfcc2feat.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 *	compress feature frames
 */
hci_int32
FX_FC_compressFrames(MFCC_Stream *pMfccStream,	///< (i/o) pointer to mfcc stream
					 MFCC_Cell *pCurCell,		///< (i) current MFCC cell
					 const hci_int16 dimMFCC,	///< (i) static MFCC dimension
					 FC_Type typeFC,			///< (i) FC type
					 hci_mfcc32 distThresh,		///< (i) distance threshold in variable frame rate analysis
					 hci_int32 nMaxFrameDrop	///< (i) count of consecutive frame drops in variable frame rate analysis
);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef __FX_COMPRESSFRAME_H__
