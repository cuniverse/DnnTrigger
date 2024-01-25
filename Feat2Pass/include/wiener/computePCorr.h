/*===============================================================================
 *      ETSI ES 202 212   Distributed Speech Recognition
 *      Extended Advanced Front-End Feature Extraction Algorithm & Compression Algorithm
 *      Speech Reconstruction Algorithm.
 *      C-language software implementation                                      
 *      Version 1.1.1   October, 2003                                            
 *===============================================================================*/
/*-------------------------------------------------------------------------------
 *
 * FILE NAME: computePCorr.h
 * PURPOSE:   Declaration of function computing correlation measure for pitch candidates.
 *
 *-------------------------------------------------------------------------------*/




#ifndef COMPUTE_PCORR_H
#define COMPUTE_PCORR_H


#ifndef _defined_PCORR_DATA
// Holds scalar products, energies and sums
// for computing correlation at non-integer lag
typedef struct {
    float fX1_X1;
    float fZ1_Z1;
    float fZ2_Z2;
    float fX1_Z1;
    float fX1_Z2;
    float fZ1_Z2;
	
    float fX1_Sum;
    float fZ1_Sum;
    float fZ2_Sum;
} PCORR_STATE;

typedef struct {
	int iOldPitchPeriod;
	int iOldFrameNo;
	PCORR_STATE s;
} PCORR_DATA;

#define _defined_PCORR_DATA
#endif

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
				   PCORR_DATA *pCorrData
                   );



#endif

