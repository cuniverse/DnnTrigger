
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
 *	@file	fx_dropframe.h
 *	@ingroup mfcc2feat_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	silence frame dropping library
 */

#ifndef __FX_DROPFRAME_H__
#define __FX_DROPFRAME_H__

#include "base/hci_type.h"
#include "basic_op/fixpoint.h"
#include "wave2mfcc/fx_mfcc_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 *	drop the frame with long left/right silence periods
 */
MFCC_Cell*
FX_FrameDrop_dropFrameWithLongSilence(MFCC_Stream *pMfccStream,		///< (i) mfcc stream pool
									  hci_int32 cur_mfcc_cell,		///< (i) current mfcc cell position
									  hci_int16 winSilDrop			///< (i) window length for frame dropping
);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef __FX_DROPFRAME_H__
