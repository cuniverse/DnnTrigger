/*===============================================================================
 *      ETSI ES 202 212   Distributed Speech Recognition
 *      Extended Advanced Front-End Feature Extraction Algorithm & Compression Algorithm
 *      Speech Reconstruction Algorithm.
 *      C-language software implementation                                      
 *      Version 1.1.1   October, 2003                                            
 *===============================================================================*/
/*-------------------------------------------------------------------------------
 *
 * FILE NAME: classifyFrame.c
 * PURPOSE:   Implementation of voicing classification algorithm
 *
 *-------------------------------------------------------------------------------*/


#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "wiener/ParmInterface.h"
#include "wiener/classifyFrame.h"





/* -----------------------------------------------------------------
 *                               Constants                             
 * ---------------------------------------------------------------- */
                                                                   

#ifndef FALSE
#define			FALSE			0
#endif

#ifndef TRUE
#define			TRUE			(!FALSE)
#endif


/*
 * Upper-Band energy fraction
 */

#define			UB_ENRG_FRAC_8K		0.0018
#define			UB_ENRG_FRAC_11K	0.0023
#define			UB_ENRG_FRAC_16K	0.0029


/*
 * Zero-crossing measure fraction
 */

#define			ZCM_FRAC		0.4375


/*----------------------------------------------------------------------------
 * FUNCTION NAME:  classify_frame
 *
 * PURPOSE:       Classifies a frame into one of four classes:
 *                         non-speech (0),
 *                         unvoiced speech (1),
 *                         mixed-voiced speech (2),
 *                         voiced speech (3)
 *
 * INPUT:
 *   iVad - VAD output value for the frame
 *   iHangOverFlag - TRUE if this is a hang over frame, FALSE otherwise 
 *   fPeriod - Frame pitch period
 *   fEnergy - Frame energy
 *   pfInpSpeech[] - Input speech for the frame
 *   pfUBSpeech[] - Upper-Band speech for the frame
 *
 * OUTPUT
 *   none
 *
 * RETURN VALUE
 *   The class of the frame is returned
 *
 *---------------------------------------------------------------------------*/
X_INT16 classify_frame(X_INT16 iVad, X_INT16 iHangOverFlag, X_FLOAT32 fPeriod,
                   X_FLOAT32 fEnergy,
                   X_FLOAT32 *pfInpSpeech, X_FLOAT32 *pfUBSpeech)
{
  X_INT16 i;
  X_INT16 iClass;
  X_INT16 iSignPast;
  X_INT16 iSignPres;
  X_INT16 iZCM;

  X_FLOAT32 fUBEnergy;
  X_FLOAT32 fUBEnergyFrac;

  X_INT16 iFrameLength = FRAME_LENGTH;


  fUBEnergyFrac = (X_FLOAT32)UB_ENRG_FRAC_8K;
  


  /*
   * Set the default value
   */

  iClass = 3;


  /*
   * Estimate the class
   */
 
  if (iVad <= 0)
  {
    iClass = 0;
  }
  else if (fPeriod <= 0.0)
  {
    iClass = 1;
  }
  else
  {

    /*
     * Compute the upper-band energy and the
     * zero-crossing measure
     */

    fUBEnergy = (pfUBSpeech[0] * pfUBSpeech[0]);
    iZCM = 0;
    iSignPast = (pfInpSpeech[0] >= 0.0)? +1 : -1;
    for (i = 1; i < iFrameLength; i++)
    {
      fUBEnergy += (pfUBSpeech[i] * pfUBSpeech[i]);
      iSignPres = (pfInpSpeech[i] >= 0.0)? +1 : -1;
      iZCM = ((iSignPres-iSignPast) == 0)? iZCM : iZCM+1;
      iSignPast = iSignPres;
    }

    if ((fUBEnergy <= fUBEnergyFrac*fEnergy) ||
        (iZCM >= (X_INT16)(ZCM_FRAC*(X_FLOAT32)(iFrameLength-1))) ||
        (iHangOverFlag == TRUE))
    {
      iClass = 2;
    }

  }

  return(iClass);

}

