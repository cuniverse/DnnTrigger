
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
 *	@file	fx_cms.h
 *	@ingroup mfcc2feat_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	CMS (cepstral mean subtraction) library
 */

#ifndef __FX_CMS_H__
#define __FX_CMS_H__

#include "base/hci_type.h"
#include "basic_op/fixpoint.h"
#include "wave2mfcc/fx_mfcc_common.h"
#include "fx_mfcc2feat.h"

#ifdef FIXED_POINT_FE
#define MIN_VAR	10486		///< minimum variance in fixed-point
#else	// !FIXED_POINT_FE
#define MIN_VAR	0.01f		///< minimum variance in floating-point
#endif	// #ifdef FIXED_POINT_FE

#ifdef __cplusplus
extern "C" {
#endif

// Load CMS Profiles to buf // KSH
hci_int32
FX_CMS_loadCmsProfileList(
							FEAT_Normalizer *seedFeatNorm_ForCmsProfileType,
							char *szCMS_ProfileListFName,
							const char pszHomeDir[],
							char szCMS_ProfileModelList[][256],
							char szCMS_ProfileCmsVecList[][256],
							int *pNumCmsModels,
							int *pFlagCmsProfile, 
							const hci_int16 dimMFCC, 
							CMS_Type typeCMS);


/**
 *	load seed cepstral mean vector & max log-energy
 */
hci_int32
FX_CMS_loadSeedCepstralMeanVector(FEAT_Normalizer *pSeedFeatNorm,	///< (o) feature normalizer data struct
								  const char *szFeatNormFile,		///< (i) feature normalizer file
								  const hci_int16 dimMFCC,			///< (i) MFCC feature dimension
								  CMS_Type typeCMS					///< (i) feature normalization type
);

/**
 *	save cepstral mean vector & max log-energy
 */
hci_int32
FX_CMS_saveCepstralMeanVector(FEAT_Normalizer *pFeatNorm,			///< (i) feature normalizer
							  const char *pszCepMeanFile			///< (i) feature normalization data file
);

/**
 *	initialize accumulative terms of cepstral mean vectors for current utterance
 */
hci_int32
FX_CMS_initializeCepstrumMean(MFCC_Stream *pMfccStream,		///< (i/o) pointer to mfcc stream
							  const hci_int16 dimMFCC,		///< (i) MFCC feature dimension
							  CMS_Type typeCMS,				///< (i) feature normalization type
							  hci_int16 bCMSwVAD			///< (i) flag to VAD-dependent CMS
);
/**
 *	live-mode CMS
 */
hci_int32
FX_CMS_liveCMS(MFCC_Stream *pMfccStream,		///< (i/o) pointer to mfcc stream
			   MFCC_Cell *pCurCell,				///< (i) input MFCC cell
			   MFCC_Cell *pOutCell,				///< (o) output MFCC cell
			   Mfcc2FeatParameters *pFXVar		///< (i) config. struct for mfcc-to-feature converter
);

/**
 *	batch-mode CMS
 */
hci_int32
FX_CMS_batchCMS(hci_mfcc_t *mfccVec,			///< (i/o) current MFCC input
				FEAT_Normalizer *puserFeatNorm,	///< (i) feature normalizer struct
				FrameClass frame_class,			///< (i) silence/speech class for current frame
				const hci_int16 dimMFCC,		///< (i) MFCC feature dimension
				CMS_Type typeCMS				///< (i) CMS type
);

/**
 *	update cepstral mean vector
 */
hci_int32
FX_CMS_updateCepstralMeanVector(MFCC_Stream *pMfccStream, 		///< (i/o) pointer to mfcc stream
								Mfcc2FeatParameters *pFXVar		///< (i) config. struct for mfcc-to-feature converter
);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef __FX_CMS_H__
