/*===============================================================================
 *      ETSI ES 202 212   Distributed Speech Recognition
 *      Extended Advanced Front-End Feature Extraction Algorithm & Compression Algorithm
 *      Speech Reconstruction Algorithm.
 *      C-language software implementation                                      
 *      Version 1.1.1   October, 2003                                            
 *===============================================================================*/
/*-------------------------------------------------------------------------------
 *
 * FILE NAME: dsrAfeVad.c
 * PURPOSE:   Implementation of Voice Activity Detector used for the DSR Extension.
 *
 *-------------------------------------------------------------------------------*/

#include <assert.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif
#include <string.h>

#include "wiener/ParmInterface.h"
#include "wiener/dsrAfeVad.h"



/* ========================================================================
                                  CONSTANTS
   ======================================================================= */

/*
* Define TRUE and FALSE
*/

#ifndef FALSE
#define			FALSE			0
#endif

#ifndef TRUE
#define			TRUE			(!FALSE)
#endif


/*
* Number of channels
*/

#define			NUM_CHAN		23


/*
* Minimum channel energy and Initial signal energy
* (Corresponds roughly to a signal at -36 dB)
*/

#define			MIN_CH_ENRG_8K		5000.0
#define			MIN_CH_ENRG_11K		6400.0
#define			MIN_CH_ENRG_16K		10000.0

#define			INIT_SIG_ENRG_8K	(1.00e+09)
#define			INIT_SIG_ENRG_11K	(1.67e+09)
#define			INIT_SIG_ENRG_16K	(3.00e+09)


/*
* Channel energy and channel noise energy smoothing factors
*/

#define			CE_SM_FAC		0.55
#define			CNE_SM_FAC		0.1


/*
* Low and high values of "gamma" used in filtering
* "long term log-spectral energy"
*/

#define			LO_GAMMA		0.7
#define			HI_GAMMA		0.9


/*
* Low and high values of "beta" used in filtering "snr"
*/

#define			LO_BETA			0.950
#define			HI_BETA			0.998


/*
* Number of initial frames, which are assumed to be non-speech
*/

#define			INIT_FRAMES		10


/*
* Sine start channel and peak-to-average threshold - Used in
* "sine wave detection"
*/

#define			SINE_START_CHAN_8K	4
#define			SINE_START_CHAN_11K	3
#define			SINE_START_CHAN_16K	3
#define			PEAK_TO_AVE_THLD	10.0


/*
* Deviation threshold, hysteresis count threshold, and forced
* update count threshold - Used in "forced update" of channel
* noise energy estimates
*/

#define			DEV_THLD		70.0
#define			HYSTER_CNT_THLD		9
#define			F_UPDATE_CNT_THLD	500


/*
* Non-speech threshold
*/

#define			NON_SPEECH_THLD		32




/* ========================================================================
                                  MACROS
   ======================================================================= */



#define			etsi_min(a,b)		((a)<(b)?(a):(b))
#define			etsi_max(a,b)		((a)>(b)?(a):(b))
#define			square(a)			((a)*(a))




/* ========================================================================
                                INTERNAL FUNCTIONS
   ======================================================================= */


/*----------------------------------------------------------------------------
 * FUNCTION NAME:  get_vm
 *
 * PURPOSE:       Measure speech quality ("voice metric")
 *
 * INPUT:
 *   pfMFBOutArray[0:NUM_CHAN-1] - Array of Mel-Filter bank outputs
 *
 * OUTPUT
 *   pfSnr - Current estimate of the SNR (filtered)
 *
 * RETURN VALUE
 *   iVoiceMetric is returned
 *
 *---------------------------------------------------------------------------*/
static X_INT16 get_vm(AFE_VAD_DATA *pVadData, X_FLOAT32 *pfMFBOutArray, X_FLOAT32 *pfSnr)
{

  /*
   * The voice metric table is defined below.  It is a non-
   * linear table that maps the SNR index (quantized SNR value)
   * to a number that is a measure of voice quality.
   */

  static X_INT16 piVMTable[90] =
  {
    1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3,
    4, 4, 4, 5, 5, 5, 6, 6, 7, 7, 7, 8, 8, 9, 9, 10,
    10, 11, 12, 12, 13, 13, 14, 15, 15, 16, 17, 17,
    18, 19, 20, 20, 21, 22, 23, 24, 24, 25, 26, 27,
    28, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 37,
    38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
    50, 50, 50, 50, 50, 50, 50, 50, 50, 50
  };


  /*
   * The different thresholds (as a function of SNR) are
   * defined below
   */

  static X_INT16 piSigThld[20] = {36, 43, 52, 62, 73, 86, 101, 117, 134, 153,
			      173, 194, 217, 242, 268, 295, 295, 295, 295, 295};

  static X_INT16 piUpdateThld[20] = {31, 32, 33, 34, 35, 36, 37, 37, 37, 37,
				 37, 37, 37, 37, 37, 38, 38, 38, 38, 38};


  /*
   * The shape table is defined below.  It is used to correct
   * the spectral shaping caused by the different channel
   * widths used in DSR-AFE standard.
   */

  static X_FLOAT32 pfShapeTable_8K[NUM_CHAN] =
  {
    0.3333f,
    0.3333f,
    0.2857f,
    0.2857f,
    0.2857f,
    0.2500f,
    0.2500f,
    0.2222f,
    0.2000f,
    0.2000f,
    0.2000f,
    0.1818f,
    0.1667f,
    0.1538f,
    0.1429f,
    0.1429f,
    0.1333f,
    0.1176f,
    0.1111f,
    0.1111f,
    0.1000f,
    0.0909f,
    0.0870f
  };

  
  X_INT16 piChanSnr[NUM_CHAN];
  X_INT16 i,j;
  X_INT16 iUpdateThld;
  X_INT16 iSineStartChan;
  X_INT16 iUpdateFlag;
  X_INT16 iQSnr;
  X_INT16 iVoiceMetric;
  X_FLOAT32 pfLogSpecEnrg[NUM_CHAN];
  X_FLOAT32 *pfShapeTable;

  X_FLOAT32 fAve;
  X_FLOAT32 fPeak;
  X_FLOAT32 fPeak2Ave;

  X_FLOAT32 fAlpha;
  X_FLOAT32 fGamma;
  X_FLOAT32 fEnrg;
  X_FLOAT32 fLogSpecEnrgDev;

  X_FLOAT32 fMinChEnrg;
  X_FLOAT32 fInitSigEnrg;
 
  X_FLOAT32 fSigEnrgInst;
  X_FLOAT32 fSnrInst;
  X_FLOAT32 fTemp;




  pfShapeTable = pfShapeTable_8K;
  fMinChEnrg = MIN_CH_ENRG_8K;
  fInitSigEnrg = INIT_SIG_ENRG_8K;
  iSineStartChan = SINE_START_CHAN_8K;



  /*
   * Increment the frame counter
   */

  pVadData->iFrameCount++;

  if (pVadData->iFrameCount > (INT_MAX-1))
    {
      pVadData->iFrameCount = INT_MAX-1;
    }


  /*
   * Estimate the energy in each channel
   */

  for (i = 0; i < NUM_CHAN; i++)
    {

      fEnrg = pfMFBOutArray[i] * pfShapeTable[i];
      fAlpha = (pVadData->iFrameCount == 1)? 1.0f : (X_FLOAT32)CE_SM_FAC;
      pVadData->pfChanEnrg[i] = (1-fAlpha)*pVadData->pfChanEnrg[i] + fAlpha*fEnrg;
      pVadData->pfChanEnrg[i] = etsi_max(pVadData->pfChanEnrg[i],fMinChEnrg);

    }


  /*
   * Calculate the Peak-to-Average ratio
   */

  fPeak = 0.0;
  fAve = 0.0;

  for (i = 0; i < NUM_CHAN; i++)
    {
      if ((i >= iSineStartChan) && (pVadData->pfChanEnrg[i] > fPeak))
	{
	  fPeak = pVadData->pfChanEnrg[i];
	}

      fAve += pVadData->pfChanEnrg[i];
    }

  fAve /= (X_FLOAT32)NUM_CHAN;

  fPeak2Ave = (X_FLOAT32) ( 10.0 * log10(fPeak/fAve) );


  /*
   * Estimate the channel noise energies from the first
   * INIT_FRAMES frames
   */

  if ((pVadData->iFrameCount <= INIT_FRAMES) || (pVadData->iFUpdateFlag == TRUE))
    {
      
      if (fPeak2Ave < PEAK_TO_AVE_THLD)
	{
	  for (i = 0; i < NUM_CHAN; i++)
	    {
	      if (pVadData->iFrameCount == 1)
		{
		  pVadData->pfChanNoiseEnrg[i] = pVadData->pfChanEnrg[i];
		}
	      else
		{
		  pVadData->pfChanNoiseEnrg[i] = 0.7f * pVadData->pfChanNoiseEnrg[i] + 0.3f * pVadData->pfChanEnrg[i];
		}
	    }
	}
      else
	{
	  for (i = 0; i < NUM_CHAN; i++)
	    {
	      pVadData->pfChanNoiseEnrg[i] = fMinChEnrg;
	    }
	}
      
    }


  /*
   * Compute the channel SNR indices
   */

  for (i = 0; i < NUM_CHAN; i++)
    {
      fTemp = (X_FLOAT32) (10.0 * log10((double)(pVadData->pfChanEnrg[i]/pVadData->pfChanNoiseEnrg[i])) );
      fTemp = etsi_max(fTemp,0.0f);
      piChanSnr[i] = (X_INT16) ( (fTemp+0.1875f) / 0.375f );
    }


  /*
   * Compute the Voice Metric
   */

  iVoiceMetric = 0;
  for (i = 0; i < NUM_CHAN; i++)
    {
      j = etsi_min(piChanSnr[i],89);
      iVoiceMetric += piVMTable[j];
    }


  /*
   * Estimate the log spectral energy deviation
   */

  /*
   * First, compute the log-spectral energy
   */

  for (i = 0; i < NUM_CHAN; i++)
    {
      pfLogSpecEnrg[i] = (X_FLOAT32) ( 10.0*log10(pVadData->pfChanEnrg[i]) );
    }

  if ((pVadData->iFrameCount <= INIT_FRAMES) || (pVadData->iFUpdateFlag == TRUE))
    {
      for (i = 0; i < NUM_CHAN; i++)
	{
	  pVadData->pfLogSpecEnrgLong[i] = pfLogSpecEnrg[i];
	}
    }


  /*
   * Next, compute the log-spectral energy deviation
   */

  fLogSpecEnrgDev = 0.0;
  for (i = 0; i < NUM_CHAN; i++)
    {
      fLogSpecEnrgDev += (X_FLOAT32) fabs(pVadData->pfLogSpecEnrgLong[i]-pfLogSpecEnrg[i]);
    }


  /*
   * Update the long term log-spectral energy
   */

  fGamma = (iVoiceMetric > pVadData->iSigThld)? (X_FLOAT32)HI_GAMMA : (X_FLOAT32)LO_GAMMA;

  for (i = 0; i < NUM_CHAN; i++)
    {
      pVadData->pfLogSpecEnrgLong[i] = fGamma*pVadData->pfLogSpecEnrgLong[i] +
	(1.0f-fGamma)*pfLogSpecEnrg[i];
    }


  /*
   * Estimate the SNR of the speech input
   */

  /*
   * First, estimate the noise energy
   */

  pVadData->fNoiseEnrg = 0.0;
  for (i = 0; i < NUM_CHAN; i++)
    {
      pVadData->fNoiseEnrg += pVadData->pfChanNoiseEnrg[i];
    }


  /*
   * Next, estimate the signal energy
   */

  if ((pVadData->iFrameCount <= INIT_FRAMES) || (pVadData->iFUpdateFlag == TRUE))
    {
      fSigEnrgInst = fInitSigEnrg;
    }
  else
    {
      
      if (iVoiceMetric > pVadData->iSigThld)
	{
	  fSigEnrgInst = 0.0;
	  for (i = 0; i < NUM_CHAN; i++)
	    {
	      if (pVadData->pfChanEnrg[i] > pVadData->pfChanNoiseEnrg[i])
		{
		  fSigEnrgInst += pVadData->pfChanEnrg[i];
		}
	      else
		{
		  fSigEnrgInst += pVadData->pfChanNoiseEnrg[i];
		}
	    }

	}
      else
	{
	  fSigEnrgInst = pVadData->fNoiseEnrg;
	}
    }


  /*
   * Compute the speech SNR
   */

  fSnrInst = (X_FLOAT32) ( 10.0 * log10(fSigEnrgInst/pVadData->fNoiseEnrg) );
  fSnrInst = etsi_max(fSnrInst,0.0f);
  
  if ((pVadData->iFrameCount <= INIT_FRAMES) || (pVadData->iFUpdateFlag == TRUE))
    {
      pVadData->fSnr = fSnrInst;
    }
  else
    {
      if (iVoiceMetric > pVadData->iSigThld)
	{
	  pVadData->fSnr = pVadData->fBeta*pVadData->fSnr + (1.0f-pVadData->fBeta)*fSnrInst;
	  pVadData->fBeta = pVadData->fBeta + 0.003f;
	  pVadData->fBeta = etsi_min(pVadData->fBeta,(X_FLOAT32)HI_BETA);
	}
      else
	{
	  pVadData->fBeta = pVadData->fBeta - 0.003f;
	  pVadData->fBeta = etsi_max(pVadData->fBeta,(X_FLOAT32)LO_BETA);
	}
    }


  /*
   * Quantize the SNR and select the different thresholds
   * based on this value
   */

  iQSnr = etsi_max(0,etsi_min((X_INT16)(pVadData->fSnr/1.5f),19));
  
  pVadData->iSigThld = piSigThld[iQSnr];
  iUpdateThld = piUpdateThld[iQSnr];


  /*
   * Set or reset the update flag and the forced update flag
   */

  iUpdateFlag = FALSE;
  pVadData->iFUpdateFlag = FALSE;
  
  if ((iVoiceMetric < iUpdateThld) && (fPeak2Ave < PEAK_TO_AVE_THLD) &&
      (pVadData->iFrameCount > INIT_FRAMES))
    {
      iUpdateFlag = TRUE;
      pVadData->iUpdateCount = 0;
    }
  else if ((fPeak2Ave < PEAK_TO_AVE_THLD) && (fLogSpecEnrgDev < DEV_THLD))
    {
      pVadData->iUpdateCount++;
      if (pVadData->iUpdateCount >= F_UPDATE_CNT_THLD)
	{
	  iUpdateFlag = TRUE;
	  pVadData->iFUpdateFlag = TRUE;
	}
    }
  else
    {
      ;
    }

  if (pVadData->iUpdateCount == pVadData->iLastUpdateCount)
    {
      pVadData->iHysterCount++;
    }
  else
    {
      pVadData->iHysterCount = 0;
    }
  pVadData->iLastUpdateCount = pVadData->iUpdateCount;

  if (pVadData->iHysterCount > HYSTER_CNT_THLD)
    {
      pVadData->iUpdateCount = 0;
    }


  /*
   * Update the channel noise estimates
   */

  if (iUpdateFlag == TRUE)
    {
      for (i = 0; i < NUM_CHAN; i++)
	{
	  pVadData->pfChanNoiseEnrg[i] = (1.0f-(X_FLOAT32)CNE_SM_FAC)*pVadData->pfChanNoiseEnrg[i] +
	    (X_FLOAT32)CNE_SM_FAC*pVadData->pfChanEnrg[i];
	  pVadData->pfChanNoiseEnrg[i] = etsi_max(pVadData->pfChanNoiseEnrg[i],fMinChEnrg);
	}
    }


  /*
   * Save the output values and return
   */

  *pfSnr = pVadData->fSnr;
  
  if (pVadData->iFrameCount <= INIT_FRAMES)
    {
      iVoiceMetric = NON_SPEECH_THLD;
    }

  return(iVoiceMetric);

}





/* ========================================================================
                                EXTERNAL FUNCTIONS
   ======================================================================= */

   
void dsr_afe_vad_init(AFE_VAD_DATA *pVadData)
{
	pVadData->iBurstConst = 6;
	pVadData->iBurstCount = 0;
	pVadData->iHangConst = 28;
	pVadData->iHangCount = -1;
	pVadData->iVADThld = 56;

	pVadData->iFrameCount = 0;
	pVadData->iFUpdateFlag = FALSE;
	pVadData->iHysterCount = 0;
	pVadData->iLastUpdateCount = 0;
	pVadData->iSigThld = 217;
	pVadData->iUpdateCount = 0;

	memset(pVadData->pfChanEnrg, 0, sizeof(pVadData->pfChanEnrg));
	memset(pVadData->pfChanNoiseEnrg, 0, sizeof(pVadData->pfChanNoiseEnrg));
	memset(pVadData->pfLogSpecEnrgLong, 0, sizeof(pVadData->pfLogSpecEnrgLong));
	pVadData->fBeta = (X_FLOAT32)LO_BETA;
	pVadData->fSnr = 0;
	pVadData->fNoiseEnrg = 0;

}

/*----------------------------------------------------------------------------
 * FUNCTION NAME:  dsr_afe_vad
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
	       X_INT16 *piHangOverFlag, X_FLOAT32 *pfSnr)
{

  static X_INT16 piBurstConst[20] = {2, 2, 3, 3, 4, 4, 4, 4, 5, 5,
				 5, 5, 5, 5, 6, 6, 6, 6, 6, 6};
  
  static X_INT16 piHangConst[20] = {54, 52, 50, 48, 46, 44, 42, 40, 38, 36,
				34, 32, 30, 28, 26, 24, 22, 20, 18, 16};

  static X_INT16 piVADThld[20] = {32, 34, 36, 38, 40, 42, 44, 46, 48, 50,
			      52, 54, 55, 56, 57, 57, 58, 58, 58, 58};

  X_INT16 iQSnr;
  X_INT16 iVad;
  X_INT16 iVoiceMetric;
  X_INT16 iVvad;



  /*
   * Get the "voice metric" for the frame
   */

  iVoiceMetric = get_vm(pVadData,pfMFBOutArray,pfSnr);


  /*
   * Estimate "voice activity" for the frame
   */

  iVvad = (iVoiceMetric > pVadData->iVADThld)? TRUE : FALSE;


  /*
   * Add Hangover
   */

  if (iVvad == TRUE)
    {
      pVadData->iBurstCount++;
    }
  else
    {
      pVadData->iBurstCount = 0;
    }

  if (pVadData->iBurstCount >= pVadData->iBurstConst)
    {
      pVadData->iHangCount = pVadData->iHangConst;
      pVadData->iBurstCount = pVadData->iBurstConst;
    }


  /*
   * Make the "vad" decision for the frame
   */

  iVad = FALSE;
  if ((iVvad == TRUE) || (pVadData->iHangCount >= 0))
    {
      iVad = TRUE;
    }

  *piHangOverFlag = FALSE;
  if ((iVvad == FALSE) && (pVadData->iHangCount >= 0))
    {
      *piHangOverFlag = TRUE;
    }

  if (pVadData->iHangCount >= 0)
    {
      pVadData->iHangCount--;
    }


  /*
   * Update the thresholds and return
   */

  iQSnr = etsi_max(0,etsi_min((X_INT16)(*pfSnr/1.5f),19));
  pVadData->iVADThld = piVADThld[iQSnr];
  pVadData->iBurstConst = piBurstConst[iQSnr];
  pVadData->iHangConst = piHangConst[iQSnr];

  return(iVad);

}


