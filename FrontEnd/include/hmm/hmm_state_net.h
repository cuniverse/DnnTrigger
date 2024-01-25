
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
 *	@file	hmm_state_net.h
 *	@ingroup hmm_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	MM state network struct
 */

#ifndef __HMM_STATE_NET_H__
#define __HMM_STATE_NET_H__

#include "base/hci_type.h"
#include "basic_op/fixpoint.h"
#include "common/powerasr_const.h"
#include "hmm/hmm_common.h"

// constants definition

#define MAX_LEN_PHONE_SEQ	256								///< maximum length of phone sequence (about 40~50 syllables)

#define STATE_NET_SIZE		(UNIT_STATE*MAX_LEN_PHONE_SEQ)	///< maximum number of state nodes in sentence network

/** Structure holding attributes of state node in sentence network. */
typedef struct {		// 2-byte fixed
	hci_uint16 idState : 13;		///< HMM state index
	hci_uint16 bCrossModel: 1;		///< flag to cross-model transition
	hci_uint16 bJumpArc : 1;		///< flag to jump arc
	hci_uint16 bActive : 1;			///< flag to node activity
} pack_t SNode_Attr;

/** Structure holding state node in sentence network. */
typedef struct {
	SNode_Attr attr;				///< state node attributes
	hci_score_t score;				///< accumulated score
} StateNode;

/** Structure holding sentence state network. */
typedef struct {
	hci_uint16 numNodes;			///< number of state nodes
	hci_uint16 numEmitNodes;		///< number of non-skipped state nodes
	hci_uint16 numSharedPhone;		///< length of shared phone sequence
	hci_uint16 numSharedNode;		///< length of shared nodes
	StateNode node[STATE_NET_SIZE];	///< state node list
} StateNetwork;

#endif	// #ifndef __HMM_STATE_NET_H__
