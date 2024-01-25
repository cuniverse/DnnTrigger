/*===============================================================================
 *      ETSI ES 202 212   Distributed Speech Recognition
 *      Extended Advanced Front-End Feature Extraction Algorithm & Compression Algorithm
 *      Speech Reconstruction Algorithm.
 *      C-language software implementation                                      
 *      Version 1.1.1   October, 2003                                            
 *===============================================================================*/
/*-------------------------------------------------------------------------------
 *
 * FILE NAME: computePCorr.c
 * PURPOSE:   Function computing correlation measure for pitch candidates.
 *
 *-------------------------------------------------------------------------------*/



/*-----------------
 * File Inclusions
 *-----------------*/

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "wiener/computePCorr.h"




/* =================================================================
                              MACROS
   ================================================================= */



#ifndef MIN
#define MIN(x,y)  (((x)>(y)) ? (y) :(x))
#endif

#ifndef MAX
#define MAX(x,y)  (((x)>(y)) ? (x) :(y))
#endif



/* =================================================================
                              DATA TYPES                        
   ================================================================= */

// Holds scalar products, energies and sums
// for computing correlation at non-integer lag
// typedef struct {
//     float fX1_X1;
//     float fZ1_Z1;
//     float fZ2_Z2;
//     float fX1_Z1;
//     float fX1_Z2;
//     float fZ1_Z2;
// 
//     float fX1_Sum;
//     float fZ1_Sum;
//     float fZ2_Sum;
// } PCORR_STATE;



/* =================================================================
                              INTERNAL FUNCTIONS
   ================================================================= */




/*----------------------------------------------------------------------------
 * FUNCTION NAME:  interpolate
 *
 * PURPOSE:       Computes correlation at non-unteger lag
 *                using scalar products, energies and sums
 *                
 * INPUT/OUTPUT:
 *   s - PCORR_STATE instance containing scalar products, energies and sums
 * INPUT:
 *   fAlpha - interpolation factor
 *   fBeta = (1 - fAlpha)
 * OUTPUT
 *   none
 *
 * RETURN VALUE
 *   none
 *   
 *---------------------------------------------------------------------------*/
static float interpolate(
                PCORR_STATE *s,
                float       fAlpha,
                float       fBeta
                )
{
    float fPCorr, fNumer, fDenom;

    fNumer = fBeta*s->fX1_Z1 + fAlpha*s->fX1_Z2;
    fDenom = (float)sqrt((fBeta*fBeta*s->fZ1_Z1+2.0*fBeta*fAlpha*s->fZ1_Z2+
                  fAlpha*fAlpha*s->fZ2_Z2) * s->fX1_X1); 

    if (fDenom > 0.0) {
        fPCorr = fNumer/fDenom;
        if (fPCorr > 1.0) {
            fPCorr = 1.0;
        }
        else if (fPCorr < 0.0) fPCorr = 0.0;
    }
    else
        fPCorr = 0.0;

    return fPCorr;
}
 

/*----------------------------------------------------------------------------
 * FUNCTION NAME:  accumulate
 *
 * PURPOSE:       Computes scalar products, energies and sums,
 *                and add these values to corresponding
 *                entries of PCORR_STATE
 * INPUT/OUTPUT:
 *   s - PCORR_STATE instance
 * INPUT:
 *   pfSignal[] - signal 
 *   iX1Index - offset parameter defining position of the processing window
 *   iPitchPeriod - correlation lag
 *   iWinLength - processing window length
 *
 * OUTPUT
 *   none
 *
 * RETURN VALUE
 *   none
 *   
 *---------------------------------------------------------------------------*/
static void accumulate(
                PCORR_STATE *s,            
                float       *pfSignal,
                int         iX1Index,
                int         iPitchPeriod,
                int         iWinLength
                )
{
    int iZ1Index, iZ2Index, j;
    float fZ1_Sum=0.f, fZ2_Sum=0.f, fZ1_Z1=0.f, fZ2_Z2=0.f; 

    iZ1Index = iX1Index - iPitchPeriod;
    iZ2Index = iZ1Index + 1;
    for (j = 0; j < iWinLength; j++)
        {
          s->fX1_Sum += pfSignal[iX1Index+j];
          fZ1_Sum += pfSignal[iZ1Index+j];

          s->fX1_X1 += pfSignal[iX1Index+j]*pfSignal[iX1Index+j];
          fZ1_Z1 += pfSignal[iZ1Index+j]*pfSignal[iZ1Index+j];

          s->fX1_Z1 += pfSignal[iX1Index+j]*pfSignal[iZ1Index+j];
          s->fX1_Z2 += pfSignal[iX1Index+j]*pfSignal[iZ2Index+j];
          s->fZ1_Z2 += pfSignal[iZ1Index+j]*pfSignal[iZ2Index+j];
        }

        fZ2_Sum = fZ1_Sum + pfSignal[iZ1Index+j] - pfSignal[iZ1Index];
        fZ2_Z2 = fZ1_Z1 + pfSignal[iZ1Index+j]*pfSignal[iZ1Index+j] -
                          pfSignal[iZ1Index]*pfSignal[iZ1Index];
        
        s->fZ1_Sum += fZ1_Sum;
        s->fZ1_Z1 += fZ1_Z1;
        s->fZ2_Sum += fZ2_Sum;
        s->fZ2_Z2 += fZ2_Z2;

}



/*----------------------------------------------------------------------------
 * FUNCTION NAME:  find_most_energetic_window
 *
 * PURPOSE:       Determine the most energetic window position
 *                
 * INPUT:
 *   pfSig[] - signal
 *   iLength - signal length
 *   iWinLength - window length
 *
 * OUTPUT
 *   none
 *
 * RETURN VALUE
 *   offset corresponding to the window left edge
 *   
 *---------------------------------------------------------------------------*/
static int find_most_energetic_window(
                      float  *pfSig,                      
                      int    iLength,
                      int    iWinLength
                      )
{
    float fSum = 0, fMax;
    int i1, i2, iLoc;

    
    for (i2=0; i2 < iWinLength; i2++) {
        fSum += pfSig[i2]*pfSig[i2];
    }    
    fMax = fSum;
    iLoc = 0;

    i1=0;
    for ( ; i2 < iLength; i2++) {
        fSum += pfSig[i2]*pfSig[i2] - pfSig[i1]*pfSig[i1];
        i1++;
        if (fSum > fMax) {
            fMax = fSum;
            iLoc = i1;
        }
    }

    return iLoc;
}




/*----------------------------------------------------------------------------
 * FUNCTION NAME:  find_most_energetic_window2
 *
 * PURPOSE:       Determine the most energetic position  of the window of given
 *                length simultaneously for two periodic signals, each signal
 *                is given by its single period.
 *                
 * INPUT:
 *   pfSig1[] - first signal
 *   pfSig2[] - second signal
 *   iLength  - the period length
 *   iWinLength - window length
 *
 * OUTPUT
 *   none
 *
 * RETURN VALUE
 *   offset corresponding to the window left edge
 *   
 *---------------------------------------------------------------------------*/
static int find_most_energetic_window2(
                      float  *pfSig1,
                      float  *pfSig2,
                      int    iLength,
                      int    iWinLength
                      )
{
    float fSum = 0, fMax;
    int i1, i2, iLoc, iLim = iWinLength;


    /* --------------------------------------------------------
                      Initial window location

     i1             i2
     |                                           |
     |--------------|----------------------------|   
     |                                           | 
   
      -------------------------------------------------------- */

    for (i2=0; i2 < iLim; i2++) {
        fSum += pfSig1[i2]*pfSig1[i2];
        fSum += pfSig2[i2]*pfSig2[i2];
    }    
    fMax = fSum;
    iLoc = 0;

    /*
        Move the window up to the leftmost position
                                 i1             i2
     |         --------->                        |
     |----------------------------|--------------|   
     |                                           | 
   


    */
    i1=0;
    for ( ; i2 < iLength; i2++) {
        fSum += pfSig1[i2]*pfSig1[i2] - pfSig1[i1]*pfSig1[i1];
        fSum += pfSig2[i2]*pfSig2[i2] - pfSig2[i1]*pfSig2[i1];
        i1++;
        if (fSum > fMax) {
            fMax = fSum;
            iLoc = i1;
        }
    }


    /*
        Continue to move the window as if the processing interval
        is expanded cyclically
                                       i1             i2
     |     i2           --------->               |
     |-----|---------------------------|---------|   
     |                                           | 
   

       ... up to this position
                                               i1             i2
     |             i2   --------->               |
     |-------------|----------------------------||   
     |                                           | 
   


    */

    iLim -= 1;    
    for (i2=0; i2 < iLim; i2++) {
        fSum += pfSig1[i2]*pfSig1[i2] - pfSig1[i1]*pfSig1[i1];
        fSum += pfSig2[i2]*pfSig2[i2] - pfSig2[i1]*pfSig2[i1];
        i1++;
        if (fSum > fMax) {
            fMax = fSum;
            iLoc = i1;
        }
    }

    
    return iLoc;

}


/* =================================================================
                           EXTERNAL FUNCTIONS
   ================================================================= */




/*----------------------------------------------------------------------------
 * FUNCTION NAME: compute_pcorr
 *
 * PURPOSE:       Computes correlation measure assiciated with given pitch value
 *                
 * INPUT:
 *   pfSignal - low-pass filtered downsampled speech signal produced by
 *              pre_process() function, see preProc.c
 *   iDownSampFactor - downsampling factor used by pre_process()
 *   fSamplingFreq - sampling frequency in Hz
 *   iFrameLen - frame length in samples
 *   fPitchFreq - pitch frequency in Hz
 *   iFrameNo - frame serial number
 *
 * OUTPUT
 *   pfCorr - correlation measure
 *
 * RETURN VALUE
 *   none
 *   
 *---------------------------------------------------------------------------*/
void compute_pcorr(
                   float   *pfSignal,
                   int     iDownSampFactor,
                   float   fSamplingFreq,
                   int     iFrameLen,
                   float   fPitchFreq,
                   int     iFrameNo,
                   float   *pfCorr,
				   PCORR_DATA			  *pCorrData
                   )
{
    int iPitchPeriod, iWinLength;
  
    int iX1Index;
    

    float fPitchPeriod;
    float fInvWinLength;
    

    float fAlpha;
    float fBeta;

    int iFrameLen_DS = iFrameLen/iDownSampFactor;


    if (fPitchFreq == 0)
    {
        *pfCorr = 0.f;
        return;
    }

    /*
     * Determine the pitch period and interpolation factors fAlpha & fBetta
     */
    
    fPitchPeriod = fSamplingFreq/fPitchFreq;
    fPitchPeriod /= (float)iDownSampFactor;
    iPitchPeriod = (int)(fPitchPeriod) + 1;
    fAlpha = (float)iPitchPeriod - fPitchPeriod;
    fBeta = 1.0f - fAlpha;

      
    if (pCorrData->iOldFrameNo == iFrameNo && iPitchPeriod == pCorrData->iOldPitchPeriod) {
        /*
        * If the current integer pitch period is the same as the old one
        * obtained for the same frame, use the previously calculated scalar products and
        * energies for computing correlation value
        */        
        *pfCorr = interpolate(&pCorrData->s, fAlpha, fBeta);
                
    }
    else {
        
        /*
         * Determine the starting indices
         */
        
        if (iPitchPeriod < iFrameLen_DS/2)
          iX1Index = iFrameLen_DS/2;
        else
          iX1Index = iFrameLen_DS - iPitchPeriod;
          


        /*
         * Compute the scalar products, energies, and sums 
         */

        pCorrData->s.fX1_Sum = 0.0;
        pCorrData->s.fZ1_Sum = 0.0;

        pCorrData->s.fX1_X1 = 0.0;
        pCorrData->s.fZ1_Z1 = 0.0;

        pCorrData->s.fX1_Z1 = 0.0;
        pCorrData->s.fX1_Z2 = 0.0;
        pCorrData->s.fZ1_Z2 = 0.0;

        pCorrData->s.fZ2_Sum = 0.f;
        pCorrData->s.fZ2_Z2 = 0.f;

        // We use the same window length (75 samples for 8kHz speech,
        // i.e. about 9ms) regardless of pitch period
        //         
        iWinLength = (int)(75.f*fSamplingFreq/8000.f/(float)iDownSampFactor);

                           

        if (iPitchPeriod <= iWinLength) {
            // The window conatins more than entire period

            iX1Index = find_most_energetic_window(pfSignal,iFrameLen_DS,
						  iPitchPeriod + iWinLength);
            iX1Index += iPitchPeriod;          
            accumulate(&pCorrData->s, pfSignal,iX1Index,iPitchPeriod,iWinLength);
        }
        else {
            // The window contains a part of the period.
            // Find the most energetic position for the window,
            // Take into account the cyclic nature of the signal buffer - 
            // the window can be splitted into two sub-windows
            int iCyclicWinLoc, iSubWinLength;
            iCyclicWinLoc = find_most_energetic_window2(
                         &pfSignal[iX1Index],                                      
                         &pfSignal[iX1Index-iPitchPeriod],
                         iPitchPeriod,
                         iWinLength);
            if (iCyclicWinLoc + iWinLength <= iPitchPeriod) {
                // Take the whole window and correlate
                iX1Index += iCyclicWinLoc;
                accumulate(&pCorrData->s, pfSignal,iX1Index,iPitchPeriod,iWinLength);
            }
            else {
                // Split the window into two sub-windows
                iSubWinLength = iPitchPeriod - iCyclicWinLoc;
                accumulate(&pCorrData->s, pfSignal,iX1Index + iCyclicWinLoc,
                           iPitchPeriod, iSubWinLength);
                iSubWinLength = iWinLength - iSubWinLength;
                accumulate(&pCorrData->s, pfSignal,iX1Index,
                           iPitchPeriod, iSubWinLength);
            }
        }
                
        /*
         * Adjust for DC levels
         */

        fInvWinLength = 1.0f/(float)iWinLength;

        pCorrData->s.fX1_X1 -= (pCorrData->s.fX1_Sum*pCorrData->s.fX1_Sum*fInvWinLength);
        pCorrData->s.fZ1_Z1 -= (pCorrData->s.fZ1_Sum*pCorrData->s.fZ1_Sum*fInvWinLength);
        pCorrData->s.fZ2_Z2 -= (pCorrData->s.fZ2_Sum*pCorrData->s.fZ2_Sum*fInvWinLength);

        pCorrData->s.fX1_Z1 -= (pCorrData->s.fX1_Sum*pCorrData->s.fZ1_Sum*fInvWinLength);
        pCorrData->s.fX1_Z2 -= (pCorrData->s.fX1_Sum*pCorrData->s.fZ2_Sum*fInvWinLength);
        pCorrData->s.fZ1_Z2 -= (pCorrData->s.fZ1_Sum*pCorrData->s.fZ2_Sum*fInvWinLength);

        *pfCorr = interpolate(&pCorrData->s, fAlpha, fBeta);
        
        pCorrData->iOldPitchPeriod = iPitchPeriod;
        pCorrData->iOldFrameNo = iFrameNo;

    }

}







