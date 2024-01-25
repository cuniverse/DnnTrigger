
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
 *	@file	oprob_common.h
 *	@ingroup stateLL_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	constants/structure definition for State Observation LL calculator
 */

#ifndef __OPROB_COMMON_H__
#define __OPROB_COMMON_H__

#include "base/hci_type.h"
#include "basic_op/fixpoint.h"
#include "basic_op/basic_op.h"
#include "common/powerasr_const.h"
#include "hmm/hmm_common.h"

//////////////////////////////////////////////////////////////////////////

/* constants definitions */

#define FRAME_OPROB_SIZE	STATE_SIZE			///< state LL vector size per frame
#define BASE_OPROB_SIZE		BASESTATE_SIZE		///< base-state LL vector size per frame

#define OPROB_TABLE_SIZE	(FRAME_OPROB_SIZE*MAX_DEC_FRAME)	///< total state LL table size

#define BEST_PDF_SIZE	4	///< maximum number of best Gauss PDFs when computing state LLs in tied-mixture HMMs

#define ACTIVATE_STATE		///< if defined, activate HMM states selectively

//////////////////////////////////////////////////////////////////////////

/* structures */

typedef	hci_int16	hci_oprob16;
typedef	hci_int32	hci_oprob32;
typedef hci_int64	hci_oprob64;

#ifndef hci_optable_t
#ifdef USE_SCALED_LL
typedef hci_int8 hci_optable_t;
#else
typedef hci_int16 hci_optable_t;
#endif
#endif	// !hci_optable_t

#ifndef hci_LL_t
#ifdef USE_SCALED_LL
typedef hci_int8 hci_LL_t;
#else
typedef hci_int16 hci_LL_t;
#endif
#endif	// !hci_LL_t

/** Structure holding N-best Gaussian PDFs for tied mixtures */
typedef struct {
	hci_uint32 num_best_pdfs;				///< number of PDFs with best log-likelihoods
	hci_uint8 best_pdf[BEST_PDF_SIZE];		///< n-best Gauss PDf index
	hci_int16 pdf_dist[BEST_PDF_SIZE];		///< n-best inner distance of Gauss PDF
}BestPDF;

/** Structure holding state LL table */
typedef struct {
	hci_uint16 sizeFrame;		///< frame length
	hci_uint16 sizeState;		///< number of HMM states
	hci_int16 maxFrameLL;		///< maximum log-likelihood value at current frame
	hci_uint16 bbi_node;		///< BBI terminal node at current frame
	hci_optable_t tableLL[OPROB_TABLE_SIZE];	///< state LL table for total frames
	hci_int16 stateLL[STATE_SIZE];	///< state LL vector at current frame
#if defined(SDCHMM)
	hci_int16 sdchmm_dist[SUBSPACE_STREAM*SUBSPACE_SIZE];	///< subspace codebook distance matrix in SDCHMM
#endif	// #if defined(SDCHMM)
#if !defined(CDHMM)
	BestPDF best_gauss[TOTAL_GMM_SIZE];		///< best Gauss PDF set in tied-mixture HMMs
#endif	// #if !defined(CDHMM)
	hci_uint8 bBaseStateActive[BASESTATE_SIZE];		///< base-state activity flag vector
	hci_uint8 bStateActive[STATE_SIZE];				///< HMM state activity flag vector
	const hci_int16 *ptrDistTbl[DIM_FEATURE];		///< pointer to distance table for each feature component at current frame
} OPROB_TABLE;

#endif	// #ifndef __OPROB_COMMON_H__
