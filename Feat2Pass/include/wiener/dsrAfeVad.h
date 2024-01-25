/*===============================================================================
 *      ETSI ES 202 212   Distributed Speech Recognition
 *      Extended Advanced Front-End Feature Extraction Algorithm & Compression Algorithm
 *      Speech Reconstruction Algorithm.
 *      C-language software implementation                                      
 *      Version 1.1.1   October, 2003                                            
 *===============================================================================*/
/*-------------------------------------------------------------------------------
 *
 * FILE NAME: dsrFeVad.h
 * PURPOSE:   Declaration of Voice Activity Detection function
 *
 *-------------------------------------------------------------------------------*/

#ifndef DSR_FE_VAD_H
#define DSR_FE_VAD_H

#include "x_default.h"

#ifndef _defined_AFE_VAD

#define		NUM_VAD_CHAN		23

typedef struct {
  X_INT16 iBurstConst;
  X_INT16 iBurstCount;
  X_INT16 iHangConst;
  X_INT16 iHangCount;
  X_INT16 iVADThld;
  X_INT32 iFrameCount;
  X_INT16 iFUpdateFlag;
  X_INT16 iHysterCount;
  X_INT16 iLastUpdateCount;
  X_INT16 iSigThld;
  X_INT16 iUpdateCount;
  X_FLOAT32 pfChanEnrg[NUM_VAD_CHAN];
  X_FLOAT32 pfChanNoiseEnrg[NUM_VAD_CHAN];
  X_FLOAT32 pfLogSpecEnrgLong[NUM_VAD_CHAN];
  X_FLOAT32 fBeta;
  X_FLOAT32 fSnr;
  X_FLOAT32 fNoiseEnrg;
} AFE_VAD_DATA;

#define _defined_AFE_VAD
#endif

/*----------------------------------------------------------------------------
 * FUNCTION NAME:  dsr_fe_vad
 *
 * PURPOSE:       Detects the Voice Activity at the DSR Front-End
 *
 * INPUT:
 *   pfMFBOutArray[0:NUM_CHAN-1] - Array of Mel-Filter bank outputs
 *
 * OUTPUT
 *   piHangOverFlag - This flag is set if the current frame is
 *                    a hang-over frame
 *   pfSnr - Current estimate of the SNR (filtered)
 *
 * RETURN VALUE
 *   iVad - The VAD value is returned
 *
 *---------------------------------------------------------------------------*/
X_INT16 dsr_afe_vad(AFE_VAD_DATA *pVadData, X_FLOAT32 *pfMFBOutArray,
	       X_INT16 *piHangOverFlag, X_FLOAT32 *pfSnr);


void dsr_afe_vad_init(AFE_VAD_DATA *pVadData);

#endif

