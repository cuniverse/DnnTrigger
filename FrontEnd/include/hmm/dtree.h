
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
 *	@file	dtree.h
 *	@ingroup hmm_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	decision tree library
 */

#ifndef __DTREE_H__
#define __DTREE_H__

#include "base/hci_type.h"
#include "basic_op/fixpoint.h"
#include "hmm/hmm_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Given left/right context values, return a target value of terminal node in decision tree
 */
hci_uint16
AM_DTREE_findTargetValueOfPhoneticContext(const DT_NODE *nodeDT,			///< (i) node list in decision tree
										  const QUEST_SET *pQSet,			///< (i) question set
										  const hci_uint16 idRootNode,		///< (i) root node index
										  const hci_uint16 leftCxt,			///< (i) left phonetic context index
										  const hci_uint16 rightCxt			///< (i) right phonetic context index
);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef __DTREE_H__
