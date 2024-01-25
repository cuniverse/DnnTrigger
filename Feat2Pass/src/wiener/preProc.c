/*===============================================================================
 *      ETSI ES 202 212   Distributed Speech Recognition
 *      Extended Advanced Front-End Feature Extraction Algorithm & Compression Algorithm
 *      Speech Reconstruction Algorithm.
 *      C-language software implementation                                      
 *      Version 1.1.1   October, 2003                                            
 *===============================================================================*/
/*-------------------------------------------------------------------------------
 *
 * FILE NAME: preProc.c
 * PURPOSE:   Implementation of speech signal preprocessing for pitch 
 *            estimation and voicing classification
 *
 *-------------------------------------------------------------------------------*/


#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "wiener/preProc.h"






#ifndef FALSE
#define			FALSE			0
#endif

#ifndef TRUE
#define			TRUE			(!FALSE)
#endif


/* =======================================================================

                         FILTER COEFFICIENTS

  ====================================================================== */


#define HPF_ORDER  6
    /*
   * Sixth order Butterworth high-pass filter
   * coefficients (1200 Hz cut-off frequency)
   */

/* Denominator */

  static float pfHpfA_8kHz[7] = {

     1.00000000,
    -2.37972104,
     2.91040657,
    -2.05513144,
     0.87792390,
    -0.20986545,
     0.02183157

  };

  static float pfHpfA_11kHz[7] = {

   1.00000000000000,
  -3.36067979080750,
   5.06982907485034,
  -4.27873732337721,
   2.10853144888207,
  -0.57109866030671,
   0.06610536478028
  };

  static float pfHpfA_16kHz[7] = {
   1.00000000000000,
  -4.18238957916850,
   7.49161108458765,
  -7.31359596689075,
   4.08934993183312,
  -1.23852537177671,
   0.15842763255178
  };


/* Numerator */


  static float pfHpfB_8kHz[7] = {

     0.14773250,
    -0.88639500,
     2.21598750,
    -2.95464999,
     2.21598749,
    -0.88639500,
     0.14773250
  };

  static float pfHpfB_11kHz[7] = {
   0.25710908848444,
  -1.54265453090663,
   3.85663632726659,
  -5.14218176968878,
   3.85663632726659,
  -1.54265453090663,
   0.25710908848444
  };

  static float pfHpfB_16kHz[7] = {
   0.39802968073138,
  -2.38817808438830,
   5.97044521097075,
  -7.96059361462766,
   5.97044521097075,
  -2.38817808438830,
   0.39802968073138
  };

  



  /*
   * Normally used Low-pass filter coefficients.
   * Combination of a
   * sixth order butterworth filter (800 Hz cut-off
   * frequency) and a low frequency emphasis filter
   * with coefficients B = [1.0 -0.6], A = [1.0 -0.9]
   */
#define LPF_ORDER  7

  /* Denominator */

  static float pfLpfA_8kHz[8] = {

     1.00000000,
    -4.47943480,
     8.88015848,
   -10.05821568,
     6.99836861,
    -2.98181953,
     0.71850318,
    -0.07538083

  };

  static float pfLpfA_11kHz[8] = {

    1.00000000000000,
   -5.16457301342956,
   11.60327150757658,
  -14.68045002998683,
   11.28039703154784,
   -5.25795344738947,
    1.37514936680065,
   -0.15553999870817
  };

  static float pfLpfA_16kHz[8] = {

    1.00000000000000,
   -5.73713549885214,
   14.19729645263144,
  -19.63612073482969,
   16.38673682892475,
   -8.24809503698812,
    2.31775924387808,
   -0.28041380978170
  };

    /* Numerator */

  static float pfLpfB_8kHz[8] = {

     0.0003405377,
     0.0018389033,
     0.0038821292,
     0.0037459142,
     0.0010216130,
    -0.0010216130,
    -0.0008853979,
    -0.0002043226

  };

  static float pfLpfB_11kHz[8] = {

   0.00006475945579,
   0.00034263580465,
   0.00069586625626,
   0.00060637516431,
   0.00005297323484,
  -0.00030025721678,
  -0.00021076612482,
  -0.00004592093010
  };

  static float pfLpfB_16kHz[8] = {

   0.00000857655707,
   0.00004459809678,
   0.00008748088215,
   0.00006861245659,
  -0.00000857655707,
  -0.00005145934244,
  -0.00003259091688,
  -0.00000686124566
  };


  /*
   * Low-pass filter coefficients used in presence of 
   * low band noise:
   * sixth order butterworth filter (800 Hz cut-off
   * frequency).
   * No low frequency emphasis.
   */


#define LPF_ORDER_NO_LB_EMPH  6

  /* Denominator */

  static float pfLpfA_8kHz_NoLbEmph[7] = {
    1.00000000,
   -3.57943480,
    5.65866717,
   -4.96541523,
    2.52949491,
   -0.70527411,
    0.08375648
  };

  static float pfLpfA_11kHz_NoLbEmph[7] = {
    1.00000000000000,
   -4.23729801342957,
    7.67413099217370,
   -7.56442021421899,
    4.26609927740795,
   -1.30210623993103,
    0.16773880316861
  };

  static float pfLpfA_16kHz_NoLbEmph[7] = {
    1.00000000000000,
   -4.78713549885213,
    9.64951772872192,
  -10.46907889254388,
    6.44111188100808,
   -2.12903875003046,
    0.29517243134916
  };

  /* Numerator */

  static float pfLpfB_8kHz_NoLbEmph[7] = {
  0.00034054,
  0.00204323,
  0.00510806,
  0.00681075,
  0.00510806,
  0.00204323,
  0.00034054
  };

  static float pfLpfB_11kHz_NoLbEmph[7] = {
    0.00006475945579,
    0.00038855673475,
    0.00097139183688,
    0.00129518911584,
    0.00097139183688,
    0.00038855673475,
    0.00006475945579
  };

  static float pfLpfB_16kHz_NoLbEmph[7] = {
    0.00000857655707,
    0.00005145934244,
    0.00012864835610,
    0.00017153114146,
    0.00012864835610,
    0.00005145934244,
    0.00000857655707
  };
  

/* ====================================================================

                 INTERNAL FUNCTIONS

  ===================================================================== */  


/*----------------------------------------------------------------------------
 * FUNCTION NAME:   filter
 *
 * PURPOSE:       Applies a pole-zero (recursive) filter
 *
 * INPUT:
 *   pfB[0:iOrder] - Coefficients corresponding to the
 *                   numerator polynominal
 *   pfA[0:iOrder] - Coefficients corresponding to the
 *                   denominator polynomial
 *   iOrder - Filter order
 *   pfInp[-iOrder:iLen-1] - Input array
 *   iLen - Input array length (for new samples)
 *
 * INPUT/OUTPUT
 *   pfOut[-iOrder:iLen-1] - Filteres array
 *
 * RETURN VALUE
 *   none
 *
 *---------------------------------------------------------------------------*/
static void filter(float *pfB, float *pfA, int iOrder, float *pfInp,
		   int iLen, float *pfOut)
{
  int i,j;



  for (i = 0; i < iLen; i++)
  {

    pfOut[i] = pfB[0] * pfInp[i];

    for(j = 1; j <= iOrder; j++)
    {
      pfOut[i] += (pfB[j] * pfInp[i-j]);
      pfOut[i] -= (pfA[j] * pfOut[i-j]);
    }

  }

}





/* ====================================================================

                 EXTERNAL FUNCTIONS

  ===================================================================== */  


/*----------------------------------------------------------------------------
 * FUNCTION NAME:   pre_process
 *
 * PURPOSE:       To pre-process a speech frame for pitch extraction and
 *                classification purposes
 *
 * INPUT:
 *   iFrameLength - frame length
 *   iFrameShift  - frame shift
 *   iHistoryLength - offset used to pass the processed signal to
 *                    EstimatePitch() 
 *   iDownSampFactor - downsampling factor
 *   pfInpSpeech - Input speech buffer
 *   iSamplingFrequency - sampling frequency in Hz
 *   iFirstFrameFlag - Flag to indicate the first frame
 *   iLowBandNoiseFlag - low-band noise presence flag
 * OUTPUT
 *   pfUBSpeech - Upper Band speech
 *   pfProcSpeech - Lower Band speech
 *   pfDownSampledProcSpeech - downsampled version of pfProcSpeech
 *
 * RETURN VALUE
 *   none
 *
 *---------------------------------------------------------------------------*/
void pre_process(int iFrameLength,
                 int iFrameShift,
                 int iHistoryLength,
                 int iDownSampFactor,
                 float *pfInpSpeech,                  
                 int iSamplingFrequency,
                 int iFirstFrameFlag,
                 int iLowBandNoiseFlag,
                 float *pfUBSpeech,
                 float *pfProcSpeech,
                 float *pfDownSampledProcSpeech)
{
  int i, j;
  int iOrder, iLpfOrder;
  int iLength;
  float *pfHpfA,*pfHpfB,*pfLpfA,*pfLpfB;



  /* Select filters */
  switch (iSamplingFrequency) {
    case 8000:
        pfHpfA = pfHpfA_8kHz;
        pfHpfB = pfHpfB_8kHz;
        if (iLowBandNoiseFlag) {
            pfLpfA = pfLpfA_8kHz_NoLbEmph;
            pfLpfB = pfLpfB_8kHz_NoLbEmph;
        }
        else {
            pfLpfA = pfLpfA_8kHz;
            pfLpfB = pfLpfB_8kHz;
        }
        break;
    case 11000:
        pfHpfA = pfHpfA_11kHz;
        pfHpfB = pfHpfB_11kHz;
        if (iLowBandNoiseFlag) {
            pfLpfA = pfLpfA_11kHz_NoLbEmph;
            pfLpfB = pfLpfB_11kHz_NoLbEmph;
        }
        else {
            pfLpfA = pfLpfA_11kHz;
            pfLpfB = pfLpfB_11kHz;
        }
        break;
    case 16000:
        pfHpfA = pfHpfA_16kHz;
        pfHpfB = pfHpfB_16kHz;
        if (iLowBandNoiseFlag) {
            pfLpfA = pfLpfA_16kHz_NoLbEmph;
            pfLpfB = pfLpfB_16kHz_NoLbEmph;
        }
        else {
            pfLpfA = pfLpfA_16kHz;
            pfLpfB = pfLpfB_16kHz;
        }
        break;
    default:
        return;
  }

  if (iLowBandNoiseFlag)
      iLpfOrder = LPF_ORDER_NO_LB_EMPH;
  else
      iLpfOrder = LPF_ORDER;


  /*
   * Process the first frame
   */

  if (iFirstFrameFlag == TRUE)
  {
    iLength = iFrameLength-iFrameShift;

    /*
     * Perform HPF to obtain the Upper Band Speech
     */

    iOrder = HPF_ORDER;

    for (i = 0; i < iOrder; i++)
    {
      pfInpSpeech[iFrameLength-iLength-1-i] = 0.0;
      pfUBSpeech[iFrameLength-iLength-1-i] = 0.0;
    }

    filter(pfHpfB,pfHpfA,iOrder,pfInpSpeech+iFrameLength-iLength,
           iLength,pfUBSpeech+iFrameLength-iLength);


    /*
     * Perform LPF to get processed speech
     */

    iOrder = iLpfOrder;

    for (i = 0; i < iOrder; i++)
    {
      pfInpSpeech[iFrameLength-iLength-1-i] = 0.0;
      pfProcSpeech[iHistoryLength+iFrameLength-iLength-1-i] = 0.0;
    }

    filter(pfLpfB,pfLpfA,iOrder,pfInpSpeech+iFrameLength-iLength,
           iLength,pfProcSpeech+iHistoryLength+iFrameLength-iLength);

    
  }

  else
  {

    /*
     * Process the following frames
     */

    iLength = iFrameShift;

    /*
     * Perform HPF to obtain the Upper Band Speech
     */

    iOrder = HPF_ORDER;

    for (i = 0; i < iFrameLength-iLength; i++)
    {
      pfUBSpeech[i] = pfUBSpeech[i+iLength];
    }

    filter(pfHpfB,pfHpfA,iOrder,pfInpSpeech+iFrameLength-iLength,
           iLength,pfUBSpeech+iFrameLength-iLength);


    /*
     * Perform LPF to get processed speech
     */

    iOrder = iLpfOrder;

    for (i = 0; i < iHistoryLength+iFrameLength-iLength; i++)
    {
      pfProcSpeech[i] = pfProcSpeech[i+iLength];
    }

    filter(pfLpfB,pfLpfA,iOrder,pfInpSpeech+iFrameLength-iLength,
           iLength,pfProcSpeech+iHistoryLength+iFrameLength-iLength);

  }

  for (i=0, j=0; i < iHistoryLength+iFrameLength; i+=iDownSampFactor, j++)
        pfDownSampledProcSpeech[j] = pfProcSpeech[i];

  return;

}





