
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
 *	@file	wiener_common.h
 *	@ingroup noise_reducer_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	constants/structure definition for Wiener filtering
 */

#ifndef __WIENER_COMMON_H__
#define __WIENER_COMMON_H__

#include "base/hci_type.h"

// cannot support fixed-point wiener filter !!
#ifndef hci_wie16_defined
typedef float		hci_wie16;
typedef float		hci_wie32;
typedef	double		hci_wie64;
#define	hci_wie16_defined
#endif	// #ifndef hci_wie16_defined

/** Wiener data struct for each user/channel */
typedef struct _Wiener_UserData
{
	void *dataWiener;				///< intermediate data for Wiener filtering
	hci_int32 flagVAD;				///< VAD flag (0 == silence, 1 = unvoiced, 2 = mixed, 3 = voiced)
	hci_int32 bSpeechFound;			///< flag to speech found
	hci_float32 specEntropy;		///< mel spectral entropy
} Wiener_UserData;

#endif // #ifndef __WIENER_COMMON_H__
