
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
 *	@file	hci_resource_hmm.h
 *	@ingroup hmm_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	PowerASR acoustic-model resource manager library
 */

#ifndef __HCILAB_RESOURCE_HMM_H__
#define __HCILAB_RESOURCE_HMM_H__

#include "base/hci_type.h"
#include "basic_op/fixpoint.h"
#include "hmm/hmm_common.h"
#include "hmm/hmm_state_net.h"

#if defined(HCI_MSC_32)
#ifdef HCI_RESOURCE_HMM_EXPORTS
#define HCI_HMM_API __declspec(dllexport)
#elif defined(HCI_RESOURCE_HMM_IMPORTS)
#define HCI_HMM_API __declspec(dllimport)
#else	// in case of static library
#define HCI_HMM_API
#endif // #ifdef HCI_RESOURCE_HMM_EXPORTS
#elif defined(HCI_OS2)
#define HCI_HMM_API
#else
#define HCI_HMM_API HCI_USER
#endif

/** PowerASR HMM resource manager */
typedef struct {
	AM_Resource resAM;		///< acoustic model resource
	void *pInner;
} PowerASR_Resource_HMM;

#ifndef FullSymbolLex_defined

/** Structure holding Lexical data for full symbol match */
typedef struct {	// caution : 1-byte fixed !!
	hci_uint8 idSymbol : 7;			///< symbol index
	hci_uint8 flagSilence : 1;		///< flag to preceding short-pause
} pack_t FullSymbolLex;

#define	FullSymbolLex_defined
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 *	create a new HMM resource manager.
 *
 *	@return Return the pointer to a newly created HMM resource manager
 */
HCILAB_PUBLIC HCI_HMM_API PowerASR_Resource_HMM*
PowerASR_Resource_HMM_new(
);

/**
 *	delete the HMM resource manager.
 */
HCILAB_PUBLIC HCI_HMM_API void
PowerASR_Resource_HMM_delete(PowerASR_Resource_HMM *pThis
);

/**
 *	load acoustic model resource.
 *
 *	@return Return 0 if acoustic model resource are loaded successfully, otherwise return -1.
 */
HCILAB_PUBLIC HCI_HMM_API hci_int32
PowerASR_Resource_HMM_openHmmResource(PowerASR_Resource_HMM *pThis,		///< (i/o) pointer to the AM resource manager
									  const char *pszResourceHMMFile	///< (i) HMM resource file
);

/**
 *	free memories allocated to the HMM resource manager.
 *
 *	@return Return 0 if all memories are freed successfully, otherwise return -1.
 */
HCILAB_PUBLIC HCI_HMM_API hci_int32
PowerASR_Resource_HMM_closeHmmResource(PowerASR_Resource_HMM *pThis		///< (i/o) pointer to the AM resource manager
);

/**
 *	initialize data buffers for HMM resource manager.
 *
 *	@return Return 0 if user-specific data buffers are initialized successfully, otherwise return -1.
 */
HCILAB_PUBLIC HCI_HMM_API hci_int32
PowerASR_Resource_HMM_initializeHmmResource(PowerASR_Resource_HMM *pThis		///< (i) pointer to the AM resource manager
);

/**
 *	convert a base-phone sequence into a HMM state sequence.
 *
 *	@return Return 0 if PhoneSeq2StateNet operation is performed successfully, otherwise return -1.
 *
 *	@see AM_PhoSeq2Net_convertBasePhoneSeq2StateNetwork
 */
HCILAB_PUBLIC HCI_HMM_API hci_int32
PowerASR_Resource_HMM_convertBasePhoneSeq2StateNetwork(PowerASR_Resource_HMM *pThis,	///< (i) pointer to the HMM resource manager
													   StateNetwork *stateNet,			///< (o) HMM state network
													   const hci_uint8 *BasePhoneSeq,	///< (i) base-phone index sequence
													   const hci_uint32 lenBasePhoneSeq, ///< (i) length of base-phone sequence
													   const hci_uint32 nLexicalOrder	///< (i) lexical order/direction (normal=0, reverse=1)
);

/**
 *	convert a sub-word PLU sequence into a HMM state sequence.
 *
 *	@return Return 0 if PhoneSeq2StateNet operation is performed successfully, otherwise return -1.
 *
 *	@see AM_PhoSeq2Net_convertSubwordSeq2StateNetwork
 */
HCILAB_PUBLIC HCI_HMM_API hci_int32
PowerASR_Resource_HMM_convertSubwordSeq2StateNetwork(PowerASR_Resource_HMM *pThis,	///< (i) pointer to the HMM resource manager
													 StateNetwork *stateNet,		///< (o) HMM state network
													 const hci_uint16 *SubwordSeq,	///< (i) sub-word PLU index sequence
													 const hci_uint32 lenSubwordSeq	///< (i) length of sub-word PLU sequence
);

/**
 *	convert a base-phone sequence into a recognition symbol sequence.
 *
 *	@return Return the length of recognition symbol sequence.
 *
 *	@see AM_PhonoMap_convertBasePhoneSeq2SymbolSeq
 */
HCILAB_PUBLIC HCI_HMM_API hci_uint32
PowerASR_Resource_HMM_convertBasePhoneSeq2RecSymbolSeq(PowerASR_Resource_HMM *pThis,	///< (i) pointer to the HMM resource manager
													   hci_uint8 *RecSymbolSeq,			///< (o) recognition symbol index sequence
													   const hci_uint8 *BasePhoneSeq,	///< (i) base-phone index sequence
													   const hci_uint32 lenBasePhoneSeq, ///< (i) length of base-phone sequence
													   const hci_flag nLexicalOrder		///< (i) lexical order/direction (normal=0, reverse=1)
);

/**
 *	convert a base-phone sequence into a reference symbol sequence.
 *
 *	@return Return the length of reference symbol sequence.
 */
HCILAB_PUBLIC HCI_HMM_API hci_uint32
PowerASR_Resource_HMM_convertBasePhoneSeq2RefSymbolSeq(PowerASR_Resource_HMM *pThis,	///< (i) pointer to the HMM resource manager
													   hci_uint8 *RefSymbolSeq,			///< (o) reference symbol index sequence
													   hci_uint8 *flagSilence,			///< (o) preceding silence/short-pause flag sequence
													   const hci_uint8 *BasePhoneSeq,	///< (i) base-phone index sequence
													   const hci_uint32 lenBasePhoneSeq,	///< (i) length of base-phone sequence
													   const hci_flag nLexicalOrder		///< (i) lexical order/direction (normal=0, reverse=1)
);

/**
 *	convert symbol sequence into base-phone sequence.
 *
 *	@return Return the length of output base-phone sequence.
 */
HCILAB_PUBLIC HCI_HMM_API hci_uint32
PowerASR_Resource_HMM_convertSymbolSequence2BasePhoneSequence(PowerASR_Resource_HMM *pThis,		///< (i) pointer to the HMM resource manager
															  hci_uint8 *basephoneSeq,			///< (o) base-phone index sequence
															  const FullSymbolLex *symbolSeq,	///< (i) symbol index sequence
															  const hci_flag bReverse			///< (i) flag to reverse phone sequence
);

/**
 *	given phonetic contexts, find a reference symbol index.
 *
 *	@return Return a reference symbol index.
 */
HCILAB_PUBLIC HCI_HMM_API hci_uint16
PowerASR_Resource_HMM_findReferenceSymbol(PowerASR_Resource_HMM *pThis,		///< (i) pointer to the HMM resource manager
										  const hci_uint16 cent_phone,		///< (i) center base-phone index
										  const hci_uint16 left_cxt,		///< (i) left-side phonetic context
										  const hci_uint16 right_cxt		///< (i) right-side phonetic context
);

/**
 *	return base-phone index with the same base-phone string.
 *
 *	@return Return base-phone index.
 */
HCILAB_PUBLIC HCI_HMM_API hci_uint8
PowerASR_Resource_HMM_findBasePhoneIndex(PowerASR_Resource_HMM *pThis,		///< (i) pointer to the HMM resource manager
										 const char *szPhone				///< (i) base-phone string
);

#ifdef __cplusplus
}
#endif

#endif // #ifndef __HCILAB_RESOURCE_HMM_H__

