
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
 *	@file	load_hmm.h
 *	@ingroup hmm_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	aouctic model resource loading library
 */

#ifndef __LOAD_HMM_H__
#define __LOAD_HMM_H__

#include "hmm/hmm_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 *	load binary-format AM resource
 */
hci_int32 
AM_HMM_loadBinaryHMMResource(const char *pszResourceHMMFile, 	///< (i) AM resource file
							 AM_Resource *pResAM				///< (o) pointer to AM resource
);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef __LOAD_HMM_H__
