
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
 *	@file	phoneseq2symbolseq.h
 *	@ingroup hmm_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	library to convert an input phone sequence into a symbol sequence
 */

#ifndef __PHONESEQ2SYMBOLSEQ_H__
#define __PHONESEQ2SYMBOLSEQ_H__

#include "base/hci_type.h"
#include "basic_op/fixpoint.h"
#include "hmm/hmm_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * convert an input base-phone sequence into a symbol sequence
 */
hci_uint32
AM_PhonoMap_convertBasePhoneSeq2SymbolSeq(const AM_Resource *pResAM,		///< (i) AM resource
										  hci_uint8 *pSymbolSeq,			///< (o) symbol sequence
										  hci_uint8 *flagSilence,			///< (o) preceding silence/short-pause flag sequence
										  const hci_uint8 *pBasePhoneSeq,	///< (i) base-phone sequence
										  const hci_uint32 lenBasePhoneSeq,	///< (i) length of base-phone sequence
										  const hci_flag nLexicalOrder,		///< (i) lexical order/direction (forward, reverse)
										  const hci_flag typeSymbol			///< (i) symbol type (0=recognition, 1=reference)
);


#ifdef __cplusplus
}
#endif

#endif	// #ifndef __PHONESEQ2SYMBOLSEQ_H__
