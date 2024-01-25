
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
 *	@file	hci_fx_mfcc2feat.h
 *	@ingroup mfcc2feat_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	PowerASR mfcc-to-feature converter library
 */

#ifndef __HCILAB_FX_MFCC2FEAT_H__
#define __HCILAB_FX_MFCC2FEAT_H__

#include "base/hci_type.h"
#include "basic_op/fixpoint.h"
#include "wave2mfcc/fx_mfcc_common.h"
#include "epd/epd_common.h"

#if defined(HCI_MSC_32)
#ifdef HCI_MFCC2FEAT_EXPORTS
#define HCI_MFCC2FEAT_API __declspec(dllexport)
#elif defined(HCI_MFCC2FEAT_IMPORTS)
#define HCI_MFCC2FEAT_API __declspec(dllimport)
#else	// in case of static library
#define HCI_MFCC2FEAT_API
#endif // #ifdef HCI_MFCC2FEAT_EXPORTS
#elif defined(HCI_OS2)
#define HCI_MFCC2FEAT_API
#else
#define HCI_MFCC2FEAT_API HCI_USER
#endif

/** PowerASR FX post-processor */
typedef struct _PowerASR_FX_Mfcc2Feat{
	void *pInner;
} PowerASR_FX_Mfcc2Feat;


#ifdef __cplusplus
extern "C" {
#endif

/**
 *	create a new mfcc-to-feature converter.
 *
 *	@return Return the pointer to a newly created mfcc-to-feature converter
 */
HCILAB_PUBLIC HCI_MFCC2FEAT_API PowerASR_FX_Mfcc2Feat*
PowerASR_FX_Mfcc2Feat_new(
);

/**
 *	delete the mfcc-to-feature converter.
 */
HCILAB_PUBLIC HCI_MFCC2FEAT_API void
PowerASR_FX_Mfcc2Feat_delete(PowerASR_FX_Mfcc2Feat *pThis
);

/**
 *	set-up environments for mfcc-to-feature converter,
 *	and allocate necessary memories.
 *
 *	@return return 0 if Mfcc2Feature environments are set-up correctly, otherwise return -1.
 */
HCILAB_PUBLIC HCI_MFCC2FEAT_API hci_int32
PowerASR_FX_Mfcc2Feat_openMfcc2FeatConverter(PowerASR_FX_Mfcc2Feat *pThis,	///< (i/o) pointer to the mfcc-to-feature converter
											 const char *pszHomeDir,		///< (i) working directory name
											 const char *pszConfigFile		///< (i) MFCC-to-Feature configuration file
);

/**
 *	free memories allocated to the mfcc-to-feature converter.
 *
 *	@return return 0 if all memories are freed successfully, otherwise return -1.
 */
HCILAB_PUBLIC HCI_MFCC2FEAT_API hci_int32
PowerASR_FX_Mfcc2Feat_closeMfcc2FeatConverter(PowerASR_FX_Mfcc2Feat *pThis	///< (i/o) pointer to the mfcc-to-feature converter
);

/**
 *	initialize data buffers for mfcc-to-feature converter.
 *
 *	@return return 0 if Mfcc2Feature data buffers are initialized successfully, otherwise return -1.
 */
HCILAB_PUBLIC HCI_MFCC2FEAT_API hci_int32
PowerASR_FX_Mfcc2Feat_initializeMfcc2FeatConverter(PowerASR_FX_Mfcc2Feat *pThis,	///< (i) pointer to the mfcc-to-feature converter
												   Feature_UserData *pFeatureData,	///< (i/o) pointer to user-specific feature data struct
												   hci_flag bResetNorm				///< (i) flag to reset feature normalizer
);

/**
 *	free user data buffers for mfcc-to-feature converter.
 *
 *	@return return 0 if Mfcc2Feature data buffers are released successfully, otherwise return -1.
 */
HCILAB_PUBLIC HCI_MFCC2FEAT_API hci_int32
PowerASR_FX_Mfcc2Feat_terminateMfcc2FeatConverter(PowerASR_FX_Mfcc2Feat *pThis,		///< (i) pointer to the mfcc-to-feature converter
												  Feature_UserData *pFeatureData	///< (i/o) pointer to user-specific feature data struct
);

/**
 *	free convert input MFCC stream into feature vectors.
 *
 *	@return Return one of the following values:
 *		- return M2F_FAIL if Mfcc2Feature operation failed
 *		- return M2F_FALSE if returned outputs don't exist
 *		- return M2F_TRUE if returned outputs exist
 *		- return M2F_RESET if previous returned outputs have to be reset
 *		- return M2F_COMPLETE if Mfcc2Feature operation completed
 *
 *	@see FX_Mfcc2Feat_convertMfccStream2FeatureVector()
 */
HCILAB_PUBLIC HCI_MFCC2FEAT_API M2F_Status
PowerASR_FX_Mfcc2Feat_convertMfccStream2FeatureVector(PowerASR_FX_Mfcc2Feat *pThis,		///< (i) pointer to the mfcc-to-feature converter
													  Feature_UserData *pFeatureData,	///< (o) channel-specific output feature data
													  MFCC_UserData *pMfccData,			///< (i) channel-specific MFCC data struct
													  hci_flag bUseEpdResult			///< (i) flag to use frame-by-frame EPD results
);

/**
 *	post-process speech features for use in multi-pass decoding.
 *
 *	@return none.
 */
HCILAB_PUBLIC HCI_MFCC2FEAT_API void
PowerASR_FX_Mfcc2Feat_postprocessSpeechFeature(PowerASR_FX_Mfcc2Feat *pThis,		///< (i) pointer to the mfcc-to-feature converter
											   Feature_UserData *pFeatureData		///< (o) feature user data
);

/**
 *	add a new feature vector into feature stream.
 *
 *	@return return 0 if feature insertion is completed, otherwise return -1.
 */
HCILAB_PUBLIC HCI_MFCC2FEAT_API hci_int32
PowerASR_FX_Mfcc2Feat_addFeatureVector(PowerASR_FX_Mfcc2Feat *pThis,		///< (i) pointer to the mfcc-to-feature converter
									   Feature_UserData *pFeatureData		///< (o) channel-specific output feature data
);

/**
 *	get feature normalization vector.
 *
 *	@return return 0 if feature normalization vectors are copied successfully, otherwise return -1.
 */
HCILAB_PUBLIC HCI_MFCC2FEAT_API hci_int32
PowerASR_FX_Mfcc2Feat_getFeatureNormalizer(PowerASR_FX_Mfcc2Feat *pThis,	///< (i) pointer to the mfcc-to-feature converter
										   FEAT_Normalizer *pOutFeatNorm	///< (o) feature normalization vectors
);

/**
 *	set feature normalization vector.
 *
 *	@return return 0 if feature normalization vectors are set successfully, otherwise return -1.
 */
/*
HCILAB_PUBLIC HCI_MFCC2FEAT_API hci_int32
PowerASR_FX_Mfcc2Feat_setFeatureNormalizer(PowerASR_FX_Mfcc2Feat *pThis,			///< (i) pointer to the mfcc-to-feature converter
										   MFCC_Stream *pMfccStream,				///< (o) pointer to user-specific mfcc data struct
										   const FEAT_Normalizer *pSeedFeatNorm		///< (i) seed feature normalization vectors
);
*/

HCILAB_PUBLIC HCI_MFCC2FEAT_API hci_int32
	PowerASR_FX_Mfcc2Feat_getCMSSeed(int CHID,FEAT_Normalizer *lf);	///< (i) seed feature normalization vectors

HCILAB_PUBLIC HCI_MFCC2FEAT_API hci_int32
PowerASR_FX_Mfcc2Feat_setFeatureNormalizer(PowerASR_FX_Mfcc2Feat *pThis,			///< (i) pointer to the mfcc-to-feature converter
										   MFCC_Stream *pMfccStream,				///< (o) pointer to user-specific mfcc data struct
										   const FEAT_Normalizer *pSeedFeatNorm,		///< (i) seed feature normalization vectors
										   char *cmsModelName,const LONG nChannelID
);

HCILAB_PUBLIC HCI_MFCC2FEAT_API hci_int32
PowerASR_FX_Mfcc2Feat_setClientFeatureNormalizer(PowerASR_FX_Mfcc2Feat *pThis,			///< (i) pointer to the mfcc-to-feature converter
										   MFCC_Stream *pMfccStream,				///< (o) pointer to user-specific mfcc data struct
										   const FEAT_Normalizer *pSeedFeatNorm,		///< (i) seed feature normalization vectors
										   char *cmsModelName,const LONG nChannelID
										   ,FEAT_Normalizer *plSeedFeatNorm
);


/**
 *	update feature normalization vector.
 *
 *	@return return 0 if feature normalization vectors are updated successfully, otherwise return -1.
 */
HCILAB_PUBLIC HCI_MFCC2FEAT_API hci_int32
PowerASR_FX_Mfcc2Feat_updateFeatureNormalizer(PowerASR_FX_Mfcc2Feat *pThis,			///< (i) pointer to the mfcc-to-feature converter
											  MFCC_Stream *pMfccStream				///< (o) pointer to user-specific mfcc data struct
);

/**
 *	get the length of feature frames given speech duration in sec.
 *
 *	@return return maximum length of feature frames.
 */
HCILAB_PUBLIC HCI_MFCC2FEAT_API LONG
PowerASR_FX_Mfcc2Feat_SpeechDuration2FrameLength(PowerASR_FX_Mfcc2Feat *pThis,		///< (i) pointer to the mfcc-to-feature converter
												 const LONG nDurSpeechSec			///< (i) speech duration in sec
);
 
int GetCMSList(char * arr);
#ifdef __cplusplus
}
#endif

#endif // #ifndef __HCILAB_FX_MFCC2FEAT_H__

