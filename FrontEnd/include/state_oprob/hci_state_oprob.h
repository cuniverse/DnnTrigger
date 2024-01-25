
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
 *	@file	hci_state_oprob.h
 *	@ingroup stateLL_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	PowerASR state observation log-likelihood computation library
 */

#ifndef __HCILAB_STATE_OPROB_H__
#define __HCILAB_STATE_OPROB_H__

#include "base/hci_type.h"
#include "basic_op/basic_op.h"
#include "basic_op/fixpoint.h"
#include "wave2mfcc/fx_mfcc_common.h"
#include "hmm/hmm_common.h"
#include "state_oprob/oprob_common.h"

#if defined(HCI_MSC_32)
#ifdef HCI_STATE_OPROB_EXPORTS
#define HCI_OPROB_API __declspec(dllexport)
#elif defined(HCI_STATE_OPROB_IMPORTS)
#define HCI_OPROB_API __declspec(dllimport)
#else	// in case of static library
#define HCI_OPROB_API
#endif // #ifdef HCI_STATE_OPROB_EXPORTS
#elif defined(HCI_OS2)
#define HCI_OPROB_API
#else
#define HCI_OPROB_API HCI_USER
#endif

/** PowerASR State OProb calculator */
typedef struct {
	void *pInner;
} PowerASR_State_OProb;

#ifdef __cplusplus
extern "C" {
#endif

/**
 *	create a new State observation LL calculator.
 *
 *	@return Return the pointer to a newly created State observation LL calculator
 */
HCILAB_PUBLIC HCI_OPROB_API PowerASR_State_OProb*
PowerASR_State_OProb_new();

/**
 *	delete the State observation LL calculator.
 */
HCILAB_PUBLIC HCI_OPROB_API void
PowerASR_State_OProb_delete(PowerASR_State_OProb *pThis
);

/**
 *	set-up environments for State observation LL calculator,
 *	and allocate necessary memories.
 *
 *	@return Return 0 if State observation LL calculator environments are set-up correctly, otherwise return -1.
 */
HCILAB_PUBLIC HCI_OPROB_API hci_int32
PowerASR_State_OProb_openStateLLCalculator(PowerASR_State_OProb *pThis,	///< (i/o) pointer to the State observation LL calculator
										   const AM_Resource *pResAM,	///< (i) pointer to AM resource
										   const char *pszHomeDir,		///< (i) working directory name
										   const char *pszConfigFile,	///< (i) StateOProb configuration file
										   const hci_flag bFileFormat,	///< (i) flag to ASCII or BINARY format of BBI resource file (0=ascii, 1=binary)
										   hci_int16 *pLogAddTbl		///< (i) pointer to log-addition table
);

/**
 *	free memories allocated to the State observation LL calculator.
 *
 *	@return Return 0 if all memories are freed successfully, otherwise return -1.
 */
HCILAB_PUBLIC HCI_OPROB_API hci_int32
PowerASR_State_OProb_closeStateLLCalculator(PowerASR_State_OProb *pThis	///< (i/o) pointer to the State observation LL calculator
);

/**
 *	load gender-dependent k-d tree data for BBI.
 *
 *	@return Return 0 if BBI resource is loaded successfully, otherwise return -1.
 */
HCILAB_PUBLIC HCI_OPROB_API hci_int32
PowerASR_State_OProb_loadBBIResource(PowerASR_State_OProb *pThis,	///< (i/o) pointer to the State observation LL calculator
									 const char *pszBBIFile,		///< (i) BBI data file
									 const hci_flag bFileFormat		///< (i) flag to ASCII or BINARY format of BBI resource file (0=ascii, 1=binary)
);

/**
 *	initialize data buffers for State observation LL calculator.
 *
 *	@return Return 0 if user-specific data buffers are initialized successfully, otherwise return -1.
 */
HCILAB_PUBLIC HCI_OPROB_API hci_int32
PowerASR_State_OProb_initializeStateLLCalculator(PowerASR_State_OProb *pThis,	///< (i) pointer to the State observation LL calculator
												 OPROB_TABLE *pStateLLTable		///< (o) state LL table
);

/**
 *	compute log-likelihoods of HMM states.
 *
 *	@return Return 0 if state log-likelihood computations are performed successfully, otherwise return -1.
 */
HCILAB_PUBLIC HCI_OPROB_API hci_int32
PowerASR_State_OProb_computeStateLogLikelihoods(PowerASR_State_OProb *pThis,	///< (i) pointer to the State observation LL calculator
												OPROB_TABLE *pStateLLTable,		///< (o) state LL table
												const hci_asr_feat_t *featVec,	///< (i) frame feature vector
												const FrameClass frame_class,	///< (i) frame class
												const hci_uint32 nFrameId		///< (i) frame index
);


/**
 *	get log-likelihood value of a given HMM state index from state LL table.
 *
 *	@return return log-likelihood value of a query state.
 */
HCILAB_PUBLIC HCI_OPROB_API hci_int16
PowerASR_State_OProb_getStateLL(PowerASR_State_OProb *pThis,		///< (i) pointer to the State observation LL calculator
								const hci_optable_t *stateLLVec, 	///< (i) state LL vector for current frame
								const hci_uint32 idHmmState			///< (i) HMM state index
);

/**
*	compute log-likelihoods of a single HMM state.
*
*	@return return log-likelihood value of a query state.
*/
HCILAB_PUBLIC HCI_OPROB_API hci_int16
PowerASR_State_OProb_calculateStateLL(PowerASR_State_OProb *pThis,		///< (i) pointer to the State observation LL calculator
									  OPROB_TABLE *pStateLLTable,		///< (o) state LL table
									  const hci_asr_feat_t *featVec,	///< (i) frame feature vector
									  const hci_uint32 idAM,			///< (i) acoustic model index
									  const hci_uint32 idHmmState,		///< (i) HMM state index
									  const hci_uint32 idFrame			///< (i) frame index
);

#ifdef __cplusplus
}
#endif

#endif // #ifndef __HCILAB_STATE_OPROB_H__

