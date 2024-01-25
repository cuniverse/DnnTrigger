
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
 *	@file	phoneseq2statenet.h
 *	@ingroup hmm_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	library to convert an input phone sequence into a state network
 */

#ifndef __PHONESEQ2STATENET_H__
#define __PHONESEQ2STATENET_H__

#include "base/hci_type.h"
#include "base/hash_table.h"
#include "basic_op/fixpoint.h"
#include "hmm/hmm_common.h"
#include "hmm/hmm_state_net.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * convert an input base-phone sequence into a state network
 */
hci_int32
AM_PhoSeq2Net_convertBasePhoneSeq2StateNetwork(AM_Resource *pResAM,				///< (i) AM resource
											   hash_table_t *ht_subword,		///< (i) subword hash table
											   StateNetwork *pStateNet,			///< (o) state network
											   const hci_uint8 *pBasePhoneSeq,	///< (i) base-phone sequence
											   const hci_uint32 lenBasePhoneSeq,	///< (i) length of base-phone sequence
											   const hci_uint32 nLexicalOrder	///< (i) lexical order/direction (normal=0, reverse=1)
);

/**
 * convert an input subword sequence into a state network
 */
hci_int32
AM_PhoSeq2Net_convertSubwordSeq2StateNetwork(const AM_Resource *pResAM,			///< (i) AM resource
											 StateNetwork *pStateNet,			///< (o) state network
											 const hci_uint16 *pSubwordSeq,		///< (i) subword sequence
											 const hci_uint32 lenSubwordSeq		///< (i) length of subword sequence
);

/**
 * create base-phone hash table
 */
hash_table_t*
AM_PhoSeq2Net_createBasePhoneHashTable(AM_Resource *pResAM
);

/**
 * create subword hash table
 */
hash_table_t*
AM_PhoSeq2Net_createSubwordHashTable(AM_Resource *pResAM
);

/**
 * delete subword hash table
 */
hci_int32
AM_PhoSeq2Net_deleteSubwordHashTable(hash_table_t *subword_ht
);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef __PHONESEQ2STATENET_H__
