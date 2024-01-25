/*===============================================================================
 *      ETSI ES 202 212   Distributed Speech Recognition
 *      Extended Advanced Front-End Feature Extraction Algorithm & Compression Algorithm
 *      Speech Reconstruction Algorithm.
 *      C-language software implementation                                      
 *      Version 1.1.1   October, 2003                                            
 *===============================================================================*/
/*-------------------------------------------------------------------------------
 *
 * FILE NAME: pitch.c
 * PURPOSE:   Pitch estimation package implementation.
 *
 *-------------------------------------------------------------------------------*/



/*-----------------
 * File Inclusions
 *-----------------*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>

#include "wiener/ParmInterface.h"
#include "wiener/pitchInterface.h"

#include "wiener/computePCorr.h"






/* =========================================================================
 *
 *        GENERAL TYPES MACROS AND CONSTANTS
 *
 * ========================================================================= */

typedef int BOOL;
#ifndef TRUE
#define TRUE	(BOOL)1
#endif
#ifndef FALSE
#define FALSE	(BOOL)0 
#endif



#define MALLOCATE(pVar, VarType, Number, iReturnCode) \
  if ((VarType *)NULL==(pVar=(VarType *)malloc((Number)*sizeof(VarType)))) {  \
    iReturnCode = 1;   \
  } else {   \
    memset(pVar,0,(Number)*(sizeof(VarType)));   \
    iReturnCode = 0;   \
  }

#define MDEALLOCATE(var) \
 if (NULL!=(var)) {\
  free(var);\
  var=NULL;\
 }


#ifndef MIN
#define MIN(x,y)  (((x)>(y)) ? (y) :(x))
#endif

#ifndef MAX
#define MAX(x,y)  (((x)>(y)) ? (x) :(y))
#endif

#ifndef ABS
#define ABS(x)    (((x)<0) ? -(x) : (x))
#endif


#ifndef M_PI
#define M_PI        3.14159265358979323846f
#endif


#ifndef CMPLX_DEFINED
#define CMPLX_DEFINED
/// Complex Numbers Struct
typedef struct CMPLX_t {
  float fReal;
  float fImag;
} CMPLX;
#endif




/* =========================================================================
 *
 *           PITCH  RELATED CONSTANTS
 *
 * ========================================================================= */



/*
 * Threshold on input log energy. Frames with log energy
 * lower than that will autormaticly be classified as
 * unvoiced.
 */
#define LOG_ENERGY_THRESHOLD 13.6f


/*
 * Dirichlet interpolation kernel width 
 */
#define DIRICHLET_KERNEL_SPAN  8 


/*
 * Sampling rate to express output pitch value (samples per second)
 */
#define REF_SAMPLE_RATE          8000.f 
#define REF_BANDWIDTH           (REF_SAMPLE_RATE/2.f) 
#define THIRD_REF_BANDWIDTH     (REF_SAMPLE_RATE/6.f) 
#define TWO_THIRDS_REF_BANDIWTH (REF_SAMPLE_RATE/3.f)



#define MAX_PEAKS_FOR_SORT  30
#define MAX_AMP_FRACTION  0.001f
#define SUM_FRACTION  0.95f 
#define AMP_FRACTION  0.406f 




/* Constants for power spectrum smoothing */

#define CENTER_WEIGHT 0.6250f
#define SIDE_WEIGHT   0.1875f


/*
 * Constants used to optionaly scale down the spectral peaks
 * amplitude in the second and third thirds of the bandwidth
 */
#define AMP_SCALE_DOWN1 0.65f 
#define AMP_SCALE_DOWN2 0.45f

/* 
 * Constants related to the shape of the utility function
 *
 *   1                +------+------+
 *                    |             |
 *  USTEP     +-------+             +-------+
 *            |                             |
 *   +--------+                             +---------+  
 * -0.5   -UDIST2  -UDIST1   0   +UDIST1  +UDIST2   +0.5
 *    
 *
 */

#define UDIST1 ( 65.f/512.f) 
#define UDIST2 (100.f/512.f)   
#define USTEP  (0.5f)



#define NO_OF_FRACS  (1+(int)(REF_BANDWIDTH/MIN_PITCH_FREQ)) 

/* Ignore spectral peaks below 300 Hz in presence of low band noise   */
#define HIGHPASS_CUTOFF_FREQ   300.f 


#define AMP_MARGIN1  0.06f 
#define AMP_MARGIN2  0.06f  
#define FREQ_MARGIN1 1.17f



/* Constants related to continuous pitch track */

#define STABLE_FREQ_UPPER_MARGIN  1.22f
#define STABLE_FREQ_LOWER_MARGIN  (1.f/1.22f)
#define MIN_STABLE_FRAMES 6  
#define MAX_TRACK_GAP_FRAMES 2


#define UNVOICED 0


/*
 * Pitch Frequency sub-ranges definition (Hz)
 */

#define MAX_PITCH_FREQ 420.f   
#define MIN_PITCH_FREQ  52.f


/*
 *
 *                   +------------+   Short  window
 *          +--------------+          Single window
 *   +----------+                     Double window
 *  Fmin   F1   F2   F3   F4     Fmax
 * (=52 Hz)                      (=420 Hz)
 *
 */


#define SHORT_WIN_START_FREQ  200 /* = F3 */
#define SHORT_WIN_END_FREQ   (MAX_PITCH_FREQ) /* = Fmax */

#define SINGLE_WIN_START_FREQ 100 /* = F1 */
#define SINGLE_WIN_END_FREQ   210 /* = F4 */

#define DOUBLE_WIN_START_FREQ (MIN_PITCH_FREQ) /* = Fmin */
#define DOUBLE_WIN_END_FREQ   120.f /* = F2 */





#define MAX_PEAKS_PRELIM  7
#define MIN_PEAKS  MAX_PEAKS_PRELIM   
#define MAX_PEAKS_FINAL  20


#define MAX_PRELIM_CANDS  4


#define MAX_LOCAL_MAXIMA_ON_SPECTRUM 70


#define CREATE_PIECEWISE_FUNC_LOOP_LIM_SH   20  
#define CREATE_PIECEWISE_FUNC_LOOP_LIM_SNG  30 
#define CREATE_PIECEWISE_FUNC_LOOP_LIM_DBL  60 



    

#define MAX_BEST_CANDS  2
#define N_OF_BEST_CANDS_SHORT  2
#define N_OF_BEST_CANDS_SINGLE 2
#define N_OF_BEST_CANDS_DOUBLE 2
#define N_OF_BEST_CANDS \
 (N_OF_BEST_CANDS_SHORT+N_OF_BEST_CANDS_SINGLE+N_OF_BEST_CANDS_DOUBLE)

 






                           
/* ========================================================================
 *
 *                   PITCH RELATED TYPES
 *
 * ======================================================================== */



/*
 * To hold table of fractions defining the
 * location of each unility function component
 */ 
typedef struct {
 float fFarLow; 
 float fLow;    
 float fHigh;   
 float fFarHigh;
} FRACTIONS;



/*
 * To hold spectral peaks, utlity function edge-points
 * and pitch candidates
 */ 
typedef struct {
  float fFreq;
  float fAmp;
} POINT;

typedef struct {
  int   ind;
  float fAmp;
} IPOINT;

typedef struct xpoint {
  float fFreq;
  float fAmp;
  struct xpoint *next;
} XPOINT;


typedef struct {
	XPOINT  *addr;
	int     length;
} ARRAY_OF_XPOINTS;


typedef struct {
  float fFreq;    
  float fAmp;   
  float fCorr;
} PITCH;
  



/* ---------------------------------------------------------------
 *     ROM object for pitch estimation
 * -------------------------------------------------------------- */

typedef struct _pitch_rom_tag
{

	int iSampleRate;
	int iFrameSize;
	int iWinSize;    
	int iDftSize;    	 
	
	int i4KhzDftBin;
	int    iInterpDftLen;
	
	
	FRACTIONS astFrac[NO_OF_FRACS+1];

	CMPLX     *pstWindowShiftTable; 

    float afDirichletImag[DIRICHLET_KERNEL_SPAN];

    int  iHighPassCutOffDftPoint;
	
} DSR_PITCH_ROM;


/* --------------------------------------------------------------------------
 *      Pitch Estimator object
 * ------------------------------------------------------------------------- */
typedef struct _pitch_estimator_tag{
  
  /* Reusable memory */
  void *pvScratchMemory; 
          
  /* History information */
  int   iStablePitchCount;
  PITCH stPrevPitch;
  PITCH stStableTrackPitch;
  int   iDistFromStableTrack;


  PITCH *pstPitchCand;
  int   iNoOfPitchCand;


  
  CMPLX *pstSingleInterpDft;
  CMPLX *pstDoubleInterpDft;

  int    iInterpDftLen; /* Just a copy of the value containing in DSR_PITCH_ROM */


  /* Reduced list of spectral peaks */
  POINT *pstSpectralPeaks; 
  int    iNoOfSpectralPeaks;

  /* Resulting utility function */
  POINT *pstPoints;
  int   iNoOfPoints;

  /* Pitch frequnecy search interval for the next frame */
  float  fLowPitchFreq;
  float  fHighPitchFreq;

  /* Threshold limitting computational complexity */
  int    iCreatePieceWiseFunc_LoopCount;
  int    iCreatePieceWiseFunc_LoopLimit;

  int    iFrameNo;

  int    iHighPassCutOffDftPoint;
  
} DSR_PITCH_ESTIMATOR;






/* ===============================================================
 *
 *                 INTERNAL FUNCTIONS
 *
 *                Spectrum processing
 * =============================================================== */



/*----------------------------------------------------------------------------
 * FUNCTION NAME:  DirichletInterpolation
 *
 * PURPOSE:      Increases the resolution of a DFT by a factor of two using
 *               the approximated Dirichlet interpolation. This is a frequency
 *               domain equivalent of the time-domain zero padding
 *
 * INPUT
 *   pfInputDft -  complex DFT array ordered as: real[0], imag[0], real[1], imag[1], ...
 *   
 *   fSpecAverage - average value of the complex DFT spectrum, available from
 *                  the FE
 *   iDftSize     - FFT length used to compute pfInputDft
 *   iLastInterpBin - index of the highest bin of the interpolated spectrum to
 *                    be computed
 *   pfDiricFilter - imaginary part of the Dirichlet kernel, see InitPitchRom()
 *
 * OUTPUT
 *   pstOutputDft - interpolated spectrum
 *
 * RETURN VALUE
 *   none
 *
 *---------------------------------------------------------------------------*/
static void DirichletInterpolation(
                                   const float *pfInputDft,
                                   const float fSpecAverage,
                                   const int iDftSize,
                                   const int iLastInterpBin, 
                                   const float *pfDiricFilter,
                                   CMPLX *pstOutputDft
                                   )
{
    int iLastAvailableBin, iLastNeededBin;
    int i,k,j, j_right, j_left;
    int iMaxIndex;
    float fReal,fImag;
 
 //
 // Copy input DFT values to the even locations of the interpolated 
 // output DFT buffer (only the odd locations are calculated).
 //

    for (i=1, j=2; i < DIRICHLET_KERNEL_SPAN; i++) {
        pstOutputDft[-2*i].fReal =  pfInputDft[j++];
        pstOutputDft[-2*i].fImag = -pfInputDft[j++];
    }

    iLastAvailableBin = iDftSize/2;
    iLastNeededBin = iLastInterpBin + DIRICHLET_KERNEL_SPAN -1;
    iMaxIndex = MIN(iLastNeededBin, iLastAvailableBin);

    for (i=0, j=0; i <= iMaxIndex ; i++) {    
        pstOutputDft[2*i].fReal = pfInputDft[j++];
        pstOutputDft[2*i].fImag = pfInputDft[j++];
    }

    
    for ( j=2*(iLastAvailableBin-1); i<= iLastNeededBin; i++, j-=2) {
        pstOutputDft[2*i].fReal = pfInputDft[j];
        pstOutputDft[2*i].fImag = -pfInputDft[j+1];
    }

    
    for (i=1; i < 2*iLastInterpBin; i+=2) {
        fReal = fSpecAverage;
        fImag = 0.f;
        j_right = i + 1;
        j_left  = i - 1;
        for (k=0; k<DIRICHLET_KERNEL_SPAN; k++, j_right+=2, j_left-=2) {
            fReal -= pfDiricFilter[k]*(-pstOutputDft[j_right].fImag + pstOutputDft[j_left].fImag);
            fImag += pfDiricFilter[k]*(-pstOutputDft[j_right].fReal + pstOutputDft[j_left].fReal);            
        }
        pstOutputDft[i].fReal = fReal;
        pstOutputDft[i].fImag = fImag;
    }
            
}


/*----------------------------------------------------------------------------
 * FUNCTION NAME:  CalculateDoubleWindowDft  
 *
 * PURPOSE:        Calculates the DFT of the combined (double) window given the interpolated
 *                 DFTs of two successive (single) windows and a proper table
 *                 holding  the complex exponent values     
 *
 * INPUT:
 *   pstFirstWinDft - DFT of the first single window
 *   pstFirstWinDft - DFT of the second single window
 *   iDftLen        - number of DFT bins
 * OUTPUT
 *   pstDoubleWinDft - DFT of the double window
 * RETURN VALUE
 *   none
 *   
 *---------------------------------------------------------------------------*/
static void CalculateDoubleWindowDft(const CMPLX *pstFirstWinDft,
                                     const CMPLX *pstSecondWinDft,
                                           CMPLX *pstDoubleWinDft,
                                     const int    iDftLen,
                                     const CMPLX *pstWindowShiftTable)
{
 int i;
 CMPLX stTemp;

 for (i=0; i<iDftLen; i++) {	  

  // 
  // Time shift == multiplication with complex exponents
  //
  stTemp.fReal = pstSecondWinDft[i].fReal * pstWindowShiftTable[i].fReal -
                 pstSecondWinDft[i].fImag * pstWindowShiftTable[i].fImag;

  stTemp.fImag = pstSecondWinDft[i].fReal * pstWindowShiftTable[i].fImag +
                 pstSecondWinDft[i].fImag * pstWindowShiftTable[i].fReal;

  //
  // Add result to the DFT of the first frame to get the double-window DFT
  //
  pstDoubleWinDft[i].fReal = pstFirstWinDft[i].fReal + stTemp.fReal ;
  pstDoubleWinDft[i].fImag = pstFirstWinDft[i].fImag + stTemp.fImag ;
 } 

}



/*----------------------------------------------------------------------------
 * FUNCTION NAME:  CalcSpectrumAndFindPeaks
 *
 * PURPOSE:      Determine power spectrum peaks
 *
 * INPUT
 *   pstDft           - complex STFT spectrum after Dirichlet interpolation
 *   pfPowSpecAtEvenPoints - power spectrum associated with even elements of
 *                           the complex spectrum given by pstDft argument, or NULL
 *   iDftLen               - length of pstDft array
 *   iHighPassCutOffDftPoint - the lower limit of the work frequency band
 *   pfTemp                  - work memory, iDftLen floats
 *
 * OUTPUT
 *   pstPeaks  - array of power spectral peaks
 *   piNoOfPeaks - number of peaks
 *   pfSqrAmp - smoothed power spectrum
 *
 * RETURN VALUE
 *   none
 *
 *---------------------------------------------------------------------------*/
static void CalcSpectrumAndFindPeaks(             
				     CMPLX   *pstDft,
				     const float   *pfPowSpecAtEvenPoints,
				     const int     iDftLen,
				     IPOINT   *pstPeaks,
				     int     *piNoOfPeaks,
				     float   *pfSqrAmp,
				     float   *pfTemp,
				     int     iHighPassCutOffDftPoint
									)
{
 int i, j;
 int iMaxCount = 0;

 
 
 //
 // Calculate Spectrum of given DFT (square amplitude)
 //
 
 if (NULL == pfPowSpecAtEvenPoints) {
    for(i=0; i<iDftLen; i++)
         pfTemp[i]= pstDft[i].fReal * pstDft[i].fReal + 
		    		  pstDft[i].fImag * pstDft[i].fImag;
 }
 else {
     // The square amplitudes of the even-location
     // DFT bins was already calculated by the FE
     for(i=0, j=0; i<iDftLen; i+=2, j++)
        pfTemp[i] = pfPowSpecAtEvenPoints[j];

     for(i=1; i<iDftLen; i+=2)
         pfTemp[i]= pstDft[i].fReal * pstDft[i].fReal + 
		    		  pstDft[i].fImag * pstDft[i].fImag;
 }



 //
 // Slightly smooth the spectrum in order to eliminate very close peaks.
 // We use a simple symmetric three-tap weighting filter
 // 

 pfSqrAmp[0] = pfTemp[0];
 pfSqrAmp[iDftLen-1] = pfTemp[iDftLen-1];
 for(i = 1; i < iDftLen-1 ; i++ )
     pfSqrAmp[i] = CENTER_WEIGHT * pfTemp[i] +
                    SIDE_WEIGHT  *(pfTemp[i-1]+pfTemp[i+1]);
 


 
 // Let [y0 y1 y2 y3 y4] be an array of spectral values associated with
 // successive DFT points.
 //
 // Value 'y2' is considered a local maxima if the following is true:
 //
 // (y2>y1) and (y2>y3) and ( (y1>=y0) or (y3>=y4) )
 //   


 for(i=iHighPassCutOffDftPoint+2; i<iDftLen-2; i++) {
 
   float fY0 = pfSqrAmp[i-2] ;
   float fY1 = pfSqrAmp[i-1] ;
   float fY2 = pfSqrAmp[i  ] ;
   float fY3 = pfSqrAmp[i+1] ;
   float fY4 = pfSqrAmp[i+2] ;

   // Check if local maximum
   if (fY2>fY1 && fY2>fY3) {
       if (fY1>=fY0 || fY3>=fY4) {      
           pstPeaks[iMaxCount].ind = i;	  
           pstPeaks[iMaxCount++].fAmp = fY2; 
       }
       i++;
   }

 }

 (*piNoOfPeaks) = iMaxCount; 
}



/* ===============================================================
 *
 *                 INTERNAL FUNCTIONS
 *
 *                Spectral peaks determination and processing
 * =============================================================== */


/*----------------------------------------------------------------------------
 * FUNCTION NAME:     CompareIpointAmp
 *
 * PURPOSE:       Call-back function to be used for sorting array of IPOINT elements  
 *                by associated fAmp values
 * INPUT:
 *   pvValue1     pointer to IPOINT data structure
 *   pvValue2     pointer to IPOINT data structure
 * OUTPUT
 *   none
 * RETURN VALUE
 *   1,-1,0
 *   
 *---------------------------------------------------------------------------*/
static int CompareIpointAmp(const void *pvValue1, const void *pvValue2)
{
 float fAmp1 = ((IPOINT *) pvValue1)->fAmp;
 float fAmp2 = ((IPOINT *) pvValue2)->fAmp;

 if( fAmp1 > fAmp2 )
    return(-1);
 if( fAmp1 < fAmp2)
    return(+1);

 return(0);
}


/*----------------------------------------------------------------------------
 * FUNCTION NAME:    Prelim_ScaleDownAmpsOfHighFreqPeaks  
 *
 * PURPOSE:       The same as Final_ScaleDownAmpsOfHighFreqPeaks but
 *                works on peaks represented by IPOINT struture
 * INPUT:
 *   pstPeaks     array of spectral peaks represented by IPOINT structure
 *   iNoOfPeaks   number of the peaks
 * OUTPUT
 *   none
 * RETURN VALUE
 *   none
 *   
 *---------------------------------------------------------------------------*/
void Prelim_ScaleDownAmpsOfHighFreqPeaks(
					IPOINT       *pstPeaks,
                    const int    iNoOfPeaks,
					const int    iDftLen
                        )
{
 float fMax1, fMax2, fMax3;
 float fMaxAmpAllowed;
 
 int iStart2, iStart3;
 int i;

 // Can be put in ROM 
 int iThirdLim = iDftLen/3, iTwoThirdsLim = 2*iDftLen/3;

 //
 // Find maximum spectral value in the first third of the spectrum.
 //
 fMax1 = 0.0f;
 for (i=0; i<iNoOfPeaks && pstPeaks[i].ind <= iThirdLim ; i++) {
  if (pstPeaks[i].fAmp>fMax1) fMax1 = pstPeaks[i].fAmp;
 }

 // Return if all spectral peaks are in the first third
 if (iNoOfPeaks==i || 0.0f==fMax1) 
  return;

 //
 // Find maximum spectral value in the second third of the spectrum.
 //
 fMax2 =0.0f;
 iStart2 = i;
 for ( ; i < iNoOfPeaks && pstPeaks[i].ind <= iTwoThirdsLim; i++) {
  if (pstPeaks[i].fAmp>fMax2) fMax2 = pstPeaks[i].fAmp;
 }

 //
 // Find maximum spectral value the last third of the spectrum.
 //
 fMax3 = 0.0f;
 iStart3 = i;
 for ( ; i < iNoOfPeaks; i++)   {
  if (pstPeaks[i].fAmp>fMax3) fMax3 = pstPeaks[i].fAmp;
 }

 //
 // Scale down spectral peaks in the [1/3,2/3] bandwidth range, which are
 // above the allowed amplitude. The allowed amplitude in this region
 // is defined as a fraction of the maximum amplitude in the [0,1/3] 
 // bandwidth frequnecy range
 //
 fMaxAmpAllowed = AMP_SCALE_DOWN1*AMP_SCALE_DOWN1 * fMax1;

 if (fMax2>fMaxAmpAllowed) {

  float fScaleFactor =  fMaxAmpAllowed / fMax2 ;

  for(i=iStart2; i < iStart3; i++) {             
   if (pstPeaks[i].fAmp >  fMaxAmpAllowed) pstPeaks[i].fAmp *= fScaleFactor;
  }
 }

 //
 // Scale down spectral peaks in the [2/3,1] bandwidth range, which are
 // above the allowed amplitude
 //
 fMaxAmpAllowed = AMP_SCALE_DOWN2*AMP_SCALE_DOWN2 * fMax1 ;

 if (fMax3>fMaxAmpAllowed) {

  float fScaleFactor = fMaxAmpAllowed / fMax3 ;

  for(i=iStart3; i < iNoOfPeaks; ++i) {
   if (pstPeaks[i].fAmp > fMaxAmpAllowed) pstPeaks[i].fAmp *= fScaleFactor;
  }
 }


}


/*----------------------------------------------------------------------------
 * FUNCTION NAME:    Final_ScaleDownAmpsOfHighFreqPeaks  
 *
 * PURPOSE:       Evaluates relative level of spectral peaks in the middle
 *                and high frequency band and scale down if the levels
 *                exceed predefined thresholds 
 * INPUT:
 *   pstPeaks     array of spectral peaks represented by POINT structure
 *   iNoOfPeaks   number of the peaks
 * OUTPUT
 *   none
 * RETURN VALUE
 *   none
 *   
 *---------------------------------------------------------------------------*/
void Final_ScaleDownAmpsOfHighFreqPeaks(
				POINT       *pstPeaks,
				const int   iNoOfPeaks
					)
 
{
 float fMax1, fMax2, fMax3;
 float fMaxAmpAllowed;
 
 int iStart2, iStart3;
 int i;


 //
 // Find maximum spectral value in the first third of the spectrum.
 //
 fMax1 = 0.0f;
 for (i=0; i<iNoOfPeaks && pstPeaks[i].fFreq<= THIRD_REF_BANDWIDTH ; i++) {
  if (pstPeaks[i].fAmp>fMax1) fMax1 = pstPeaks[i].fAmp;
 }

 // Return if all spectral peaks are in the first third
 if (iNoOfPeaks==i || 0.0f==fMax1) 
  return;

 //
 // Find maximum spectral value in the second third of the spectrum.
 //
 fMax2 =0.0f;
 iStart2 = i;
 for ( ; i < iNoOfPeaks && pstPeaks[i].fFreq <= TWO_THIRDS_REF_BANDIWTH; i++) {
  if (pstPeaks[i].fAmp>fMax2) fMax2 = pstPeaks[i].fAmp;
 }

 //
 // Find maximum spectral value the last third of the spectrum.
 //
 fMax3 = 0.0f;
 iStart3 = i;
 for ( ; i < iNoOfPeaks; i++)   {
  if (pstPeaks[i].fAmp>fMax3) fMax3 = pstPeaks[i].fAmp;
 }

 //
 // Scale down spectral peaks in the [1/3,2/3] bandwidth range, which are
 // above the allowed amplitude. The allowed amplitude in this region
 // is defined as a fraction of the maximum amplitude in the [0,1/3] 
 // bandwidth frequnecy range
 //
 fMaxAmpAllowed = AMP_SCALE_DOWN1 * fMax1;

 if (fMax2>fMaxAmpAllowed) {

  float fScaleFactor =  fMaxAmpAllowed / fMax2 ;

  for(i=iStart2; i < iStart3; i++) {             
   if (pstPeaks[i].fAmp >  fMaxAmpAllowed) pstPeaks[i].fAmp *= fScaleFactor;
  }
 }

 //
 // Scale down spectral peaks in the [2/3,1] bandwidth range, which are
 // above the allowed amplitude
 //
 fMaxAmpAllowed = AMP_SCALE_DOWN2 * fMax1 ;

 if (fMax3>fMaxAmpAllowed) {

  float fScaleFactor = fMaxAmpAllowed / fMax3 ;

  for(i=iStart3; i < iNoOfPeaks; ++i) {
   if (pstPeaks[i].fAmp > fMaxAmpAllowed) pstPeaks[i].fAmp *= fScaleFactor;
  }
 }


}


/*----------------------------------------------------------------------------
 * FUNCTION NAME:  RefineSpectralPeaks
 *
 * PURPOSE:      Refine power spectral peaks location and amplitudes
 *               by parabolic interpolation; apply square root to
 *               represent the amplitude spectrum peaks
 *
 * INPUT
 *   iSampleRate   sampling frequency
 *   iDftSize      number of FFT bins
 *   pfSqrAmp      power spectrum
 *   pstInpPeaks   power spectral peaks
 *   iNoOfPeaks    number of peaks
 *   
 * OUTPUT
 *   pstOutPeaks   output amplitude spectral peaks
 *
 * RETURN VALUE
 *   none
 *
 *---------------------------------------------------------------------------*/
static void RefineSpectralPeaks(
				const int    iSampleRate,
				const int    iDftSize,
				float        *pfSqrAmp,
				IPOINT *pstInpPeaks,
				int    iNoOfPeaks,
				POINT  *pstOutPeaks
				)
{
	int i, j;

	// Frequnecy normalization factor 
	float fFactor = (float)iSampleRate/(float)(iDftSize);
	float fY1, fY2, fY3;
	float f2a, f2b, fc;
	float fX, fY;
		
 
	// If y2 is a local maxima, the values [y1 y2 y3] are used to find
	// precise location and value of the maxima using quadratic interpolation

	for(j=0; j<iNoOfPeaks; j++) {
		i = pstInpPeaks[j].ind;
		fY1 = pfSqrAmp[i-1];
		fY2 = pfSqrAmp[i  ] ;
		fY3 = pfSqrAmp[i+1] ;
   
      //
      // Quadratic interpolation
      //

		f2a=  fY1-2.0f*fY2+fY3; 
		f2b= -fY1         +fY3; 
		fc =        fY2       ; 

		fX = -f2b/(2.0f*f2a) ;  
		fY =  fc + 0.25f * f2b * fX; 

		// Peak location in Hz
		fX = fFactor*(fX+(float)i);
		pstOutPeaks[j].fFreq = fX;	  

		// Convert to amplitude
		pstOutPeaks[j].fAmp = (float)sqrt(fY);

	}

}




/*----------------------------------------------------------------------------
 * FUNCTION NAME:  PrepareSpectralPeaks
 *
 * PURPOSE:      Determines spectral peaks to be used for pitch candidates
 *               generation.
 *
 * INPUT
 *   pstRom                 pitch ROM structure
 *   pstInterpDft           interpolated complex DFT spectrum
 *   pfPowSpecAtEvenPoints  power spectrum computed at front-end or NULL
 *
 * INPUT/OUTPUT
 *   pstEstimator  Pitch estimator data structure. Following entries are modified
 *                 by the function:  pstSpectralPeaks and iNoOfSpectralPeaks
 *
 * RETURN VALUE
 *   none
 *
 *---------------------------------------------------------------------------*/
static void PrepareSpectralPeaks(
		const DSR_PITCH_ROM   *pstRom,
		DSR_PITCH_ESTIMATOR       *pstEstimator,
		CMPLX                 *pstInterpDft,
		const float           *pfPowSpecAtEvenPoints
		)
{
  IPOINT *pstPeakLocs = pstEstimator->pvScratchMemory;
  float  *pfSqrAmp = (float*)((char*)pstPeakLocs + 
	           pstRom->iInterpDftLen/2*sizeof(IPOINT));
  float  *pfTemp = pfSqrAmp + pstRom->iInterpDftLen;
  // Max number of loc maxima on interpolated spectrum is pstRom->iInterpDftLen/2
  // Amount of Scratch memory used = 
  //  pstRom->iInterpDftLen/2*sizeof(IPOINT) + 2*pstRom->iInterpDftLen*sizeof(float)
  int iNoOfPeaks;

  // Calculate power spectrum and find peak
  // locations (pstPeakLocs[].fAmp, pstPeakLocs[].ind)
  CalcSpectrumAndFindPeaks(pstInterpDft,
			   pfPowSpecAtEvenPoints,
			   pstEstimator->iInterpDftLen,                   
			   pstPeakLocs,
			   &iNoOfPeaks,
			   pfSqrAmp,
			   pfTemp,
			   pstEstimator->iHighPassCutOffDftPoint);

  if (iNoOfPeaks > MAX_LOCAL_MAXIMA_ON_SPECTRUM) {
    pstEstimator->iNoOfSpectralPeaks = 0;
    return;
  }


  //
  // Scale down high frequency peaks if thir amplitude is too high
  //
  Prelim_ScaleDownAmpsOfHighFreqPeaks(pstPeakLocs,
				      iNoOfPeaks,
				      pstRom->iInterpDftLen);

  if (iNoOfPeaks > MAX_PEAKS_FOR_SORT) {
    //
    //   Remove low peaks
    //
    float fMax, fAmpThr;
    int i, j;
    fMax = pstPeakLocs[0].fAmp;
    for (i=1; i<iNoOfPeaks; i++)
      if (pstPeakLocs[i].fAmp > fMax) fMax = pstPeakLocs[i].fAmp;
      
    fAmpThr = MAX_AMP_FRACTION*MAX_AMP_FRACTION*fMax;
      
    for (i=0, j=0; i<iNoOfPeaks; i++) {
      if (pstPeakLocs[i].fAmp >= fAmpThr) {
	pstPeakLocs[j] = pstPeakLocs[i];
	j++;
      }
    }
    iNoOfPeaks = j;
      
    if (iNoOfPeaks > MAX_PEAKS_FOR_SORT)
      iNoOfPeaks = MAX_PEAKS_FOR_SORT;
			
  } // endif iNoOfPeaks > MIN_PEAKS
 

  //
  // Sort peaks in decreasing amplitude order
  // 
  qsort(pstPeakLocs, iNoOfPeaks, sizeof(IPOINT), CompareIpointAmp); 
        
  iNoOfPeaks = MIN(iNoOfPeaks,MAX_PEAKS_FINAL); 

  // 
  //  Parabolic interpolation, SQRT 
  //
  RefineSpectralPeaks(pstRom->iSampleRate,
		      2*pstRom->iDftSize,
		      pfSqrAmp,
		      pstPeakLocs,
		      iNoOfPeaks,
		      pstEstimator->pstSpectralPeaks);
  pstEstimator->iNoOfSpectralPeaks = iNoOfPeaks;

  Final_ScaleDownAmpsOfHighFreqPeaks(pstEstimator->pstSpectralPeaks,iNoOfPeaks);
					
  if (iNoOfPeaks > MIN_PEAKS) {
    //
    //   Try to reduce the number of peaks in order to reduce 
    //   the number of iterations over utility function building loops
    //
    float fSum = 0.f, fThr;
    int i;
      
    for (i=0; i<iNoOfPeaks; i++)
      fSum += pstEstimator->pstSpectralPeaks[i].fAmp;
      
    fThr = SUM_FRACTION*fSum;
    fSum = 0;
    for (i=0; i<iNoOfPeaks; i++) {
      fSum += pstEstimator->pstSpectralPeaks[i].fAmp;
      if (fSum >= fThr) {
	i++;
	break;
      }
    }
    if (i < iNoOfPeaks) {
      // the i peaks build up almost all the sum of peak amplitudes
      pstEstimator->iNoOfSpectralPeaks = i;
    }
    else {
      // Take out the peaks which are small relatively to the
      // peak[MIN_PEAKS-1]
      fThr = AMP_FRACTION*pstEstimator->pstSpectralPeaks[MIN_PEAKS-1].fAmp;
      for (i=iNoOfPeaks-1; i>=MIN_PEAKS; i--)
	if (pstEstimator->pstSpectralPeaks[i].fAmp >= fThr)
	  break;
      pstEstimator->iNoOfSpectralPeaks = i+1;
    }
  }


}



/*----------------------------------------------------------------------------
 * FUNCTION NAME:  SumAmplitudes
 *
 * PURPOSE:        Calculates the sum of spectral peak amplitudes
 *
 * INPUT:
 *   pstPeaks       array of peaks
 *   iNoOfPeaks     number of peaks
 * OUTPUT
 *   none
 * RETURN VALUE
 *   sum of the amplitudes
 *   
 *---------------------------------------------------------------------------*/
static float SumAmplitudes(POINT *pstPeaks,
                           const int iNoOfPeaks)
{
 int i;
 float fSum = 0.0f;


 for(i=0;i<iNoOfPeaks;i++)
    fSum += pstPeaks[i].fAmp;
 return fSum;
}

/*----------------------------------------------------------------------------
 * FUNCTION NAME:  NormalizeAmplitudes
 *
 * PURPOSE:        Normalizes spectral peak amplitudes so that their sum is equal
 *                 to 1, works in-place
 * INPUT:
 *   pstPeaks       array of peaks
 *   iNoOfPeaks     number of peaks
 * OUTPUT
 *   pstPeaks       array of peaks containing normalized amplitudes
 * RETURN VALUE
 *   none
 *   
 *---------------------------------------------------------------------------*/
static float NormalizeAmplitudes(POINT *pstPeaks,
                                const int iNoOfPeaks)
{
 int i;
 float fSum;
 float fNorm;

 fSum = SumAmplitudes(pstPeaks,iNoOfPeaks);


 fNorm=1.f/fSum;
 for(i=0;i<iNoOfPeaks;i++) {
    pstPeaks[i].fAmp *= fNorm;
 }
 return fSum;
}


/* ===============================================================
 *
 *                 INTERNAL FUNCTIONS
 *
 *             Utility function related routines 
 * =============================================================== */



/*----------------------------------------------------------------------------
 * FUNCTION NAME:   CreatePieceWiseConstantFunction
 *
 * PURPOSE:        Create differential utility function component generated
 *                 by single given spectral peak within a given frequency band.
 *                 The differential utility function is piecewise linear and
 *                 therefore it is defined by an array of its nodes
 * INPUT:
 *   stSpecPeak    spectral peak
 *   fMinFreq      the lower limit of the frequency band
 *   fNaxFreq      the upper limit of the frequency band
 *   pstFrac       FRACTIONS table, see InitPitchRom() for details
 *   iCreatePieceWiseFunc_LoopLimit   complexity limitation threshold
 * INPUT/OUTPUT
 *   piCreatePieceWiseFunc_LoopCount  number of the iterations over the inner
 *                                    loop (eq. to the number of utility func.
 *                                    shapes) performed since the beginning
 *                                    of the current sub-range processing
 *
 * OUTPUT
 *   pstPoint            array of the differential utility function nodes
 *   piNoOfPoints        number of nodes
 *   piOutOffset         offset of the data irigin in the output array pstPoint   
 * RETURN VALUE
 *   0 - the utility function has been created 
 *   1 - piCreatePieceWiseFunc_LoopCount reached the limit specified by
 *       iCreatePieceWiseFunc_LoopLimit. The utility function has not been
 *       calculated
 *---------------------------------------------------------------------------*/
static int CreatePieceWiseConstantFunction(
		   const POINT      stSpecPeak,
		   const float      fMinFreq,
		   const float      fMaxFreq,
		   const FRACTIONS *pstFrac,
		   XPOINT    *pstPoint,
		   int       *piNoOfPoints,
		   int       *piCreatePieceWiseFunc_LoopCount,
		   const int       iCreatePieceWiseFunc_LoopLimit,
		   int       *piOutOffset
		   )
{  
	int i, iStartIndx, iLastIndx;
	int iIndx, iFrom, iTo, iN;
	float fFrom, fTo, fF0,fF1,fALow,fAHigh;
	float fDaLowPos, fDaLowNeg, fDaHighPos, fDaHighNeg;
	BOOL bZerothShapeFallsInside;

	// Amplitudes of the platos
	fALow = stSpecPeak.fAmp  * USTEP;
	fAHigh = stSpecPeak.fAmp;

	// Differential amplitudes
	fDaLowPos = fALow;
	fDaLowNeg = -fALow;
	fDaHighPos = fAHigh - fALow;
	fDaHighNeg = -fDaHighPos;
		

	// Nmax
	fFrom = stSpecPeak.fFreq/fMinFreq+UDIST2;
	iFrom = (int)fFrom;
	if ((float)iFrom == fFrom) iFrom--;
	// Nmin
	fTo = stSpecPeak.fFreq/fMaxFreq-UDIST2;
	if (fTo < 0.f) {
		bZerothShapeFallsInside = TRUE;
		iTo = 1;
	}
	else {
		bZerothShapeFallsInside = FALSE;
		iTo = (int)fTo +1;
	}


		
	// Check complexity
	iN = iFrom-iTo+1;
	iN = MAX(0,iN);

	if ( (*piCreatePieceWiseFunc_LoopCount += iN) > 
		  iCreatePieceWiseFunc_LoopLimit )
		return 1;


	
	iIndx = iStartIndx = 0;

	for (i=iFrom; i>=iTo; i--) {
			
		pstPoint[iIndx].fFreq   = stSpecPeak.fFreq * pstFrac[i].fFarLow;
		pstPoint[iIndx++].fAmp  = fDaLowPos;
		pstPoint[iIndx].fFreq   = stSpecPeak.fFreq * pstFrac[i].fLow;
		pstPoint[iIndx++].fAmp  = fDaHighPos;
		pstPoint[iIndx].fFreq   = stSpecPeak.fFreq * pstFrac[i].fHigh;
		pstPoint[iIndx++].fAmp  = fDaHighNeg;
		pstPoint[iIndx].fFreq   = stSpecPeak.fFreq * pstFrac[i].fFarHigh;
		pstPoint[iIndx++].fAmp  = fDaLowNeg;

	} // endfor i
		
		

	if (iIndx > 0) { 

		for ( ; iStartIndx <= 3; iStartIndx++) {
			if (pstPoint[iStartIndx].fFreq >= fMinFreq)
				break;
		}
		
		if (iStartIndx > 0) {
			iStartIndx--;
			pstPoint[iStartIndx].fFreq = fMinFreq;
			for (i=0; i<iStartIndx; i++)
				pstPoint[iStartIndx].fAmp += pstPoint[i].fAmp;
		}

		// Check if the fMaxFreq falls into the last shape,
		// and if yes truncate the points list
		iLastIndx = iIndx - 1;
		i = iIndx -4;
		i = MAX(i,iStartIndx);
			
		for ( ; iLastIndx >= i ; iLastIndx--) {
			if (pstPoint[iLastIndx].fFreq < fMaxFreq)
				break;
		}
        
		iIndx = iLastIndx +1;
					

	} // endif non-empty list
			
		
	if (bZerothShapeFallsInside) {
		fF0 = stSpecPeak.fFreq * pstFrac[0].fFarLow;
		fF1 = stSpecPeak.fFreq * pstFrac[0].fLow;

		if (fF0 < fMinFreq) {
			if (fF1 <= fMinFreq) {                    
				pstPoint[iIndx].fFreq   = fMinFreq;			
				pstPoint[iIndx++].fAmp  = fAHigh;
			}
			else {
				pstPoint[iIndx].fFreq   = fMinFreq;			
				pstPoint[iIndx++].fAmp  = fALow;
				if (fF1 <= fMaxFreq) {						
					pstPoint[iIndx].fFreq   = fF1;			
					pstPoint[iIndx++].fAmp  = fDaHighPos;
				}
			}
		}
		else {
			pstPoint[iIndx].fFreq   = fF0;			
			pstPoint[iIndx++].fAmp  = fALow;
			if (fF1 <= fMaxFreq) {					
				pstPoint[iIndx].fFreq   = fF1;			
				pstPoint[iIndx++].fAmp  = fDaHighPos;
			}
		}

	} // endif 0-th shape falls in the frequency interval

	

	*piNoOfPoints = iIndx - iStartIndx;
	*piOutOffset = iStartIndx;

	return 0;


}



/*----------------------------------------------------------------------------
 * FUNCTION NAME:   LinkArrayOfPoints
 *
 * PURPOSE:        Converts array of points to linked list
 *
 * INPUT:
 *   iArrLen  the array length 
 * INPUT/OUTPUT
 *   pstPointArr   array/linked list   
 *   
 * RETURN VALUE
 *   none
 *
 *---------------------------------------------------------------------------*/
static void LinkArrayOfPoints(
				XPOINT  *pstPointArr,
				int     iArrLen
				)
{
	int i;
	#pragma loop(no_vector)	// suppress auto vectorization causing error on runtime (2016-06 mckeum)
	for (i = 0; i<iArrLen-1; i++)
		pstPointArr[i].next = &(pstPointArr[i+1]);
	pstPointArr[i].next = NULL;
}


/*----------------------------------------------------------------------------
 * FUNCTION NAME:   AddSortedArrayOfPoints
 *
 * PURPOSE:      Insert sorted array of points to sorted linked list of points preserving
 *               the sorting order. The points are sorted in ascending order of
 *               "fFreq" data-member 
 *
 * INPUT
 *   iArrLen  the array length 
 * INPUT/OUTPUT
 *   ppstPointHead - the linked list head   
 *   ppstPointTail - the linked list tail
 *   pstPointArr - the array. Its elements are embedded into the linked list
 * RETURN VALUE
 *   none
 *
 *---------------------------------------------------------------------------*/
static void AddSortedArrayOfPoints(
				XPOINT  **ppstPointHead,
				XPOINT  **ppstPointTail,
				XPOINT  *pstPointArr,
				int     iArrLen
				)
{
	XPOINT *pstPointHead = NULL, *pstPointTail = NULL, *pScan = NULL, *pPrev = NULL;
	int i = 0, i1 = 0, i2 = 0;
	float fFreq = 0.0f;
	char* test[256] = { 0, };
	pstPointHead = *ppstPointHead;
	pstPointTail = *ppstPointTail;
	//FILE* testf = fopen("debug.txt", "wt+");

	//
	// What if the input linked list contains only one element
	//
	if (pstPointHead == pstPointTail) {
		LinkArrayOfPoints(pstPointArr,iArrLen);
		fFreq = pstPointHead->fFreq;

		for (i=0; i<iArrLen; i++) {
			if (fFreq < pstPointArr[i].fFreq) break;
		}

		if (i==0) {
			// Insert the single element at the beginning
			pstPointHead->next = pstPointArr;
			*ppstPointTail = &(pstPointArr[iArrLen-1]);
		}
		else if (i==iArrLen) {
			// Insert the single element at the end
			pstPointArr[iArrLen-1].next = pstPointHead;
			pstPointHead->next = NULL;
			*ppstPointTail = pstPointHead;
			*ppstPointHead = pstPointArr;
		}
		else {
			// Insert the single element in between 			
			pstPointArr[i-1].next = pstPointHead;
			pstPointHead->next = &(pstPointArr[i]);
			*ppstPointHead = &(pstPointArr[0]);
			*ppstPointTail = &(pstPointArr[iArrLen-1]);
		}

		return;

	} // end if the input linked list contains only one element

	//
	//   From the current line it's guaranteed that the input linked list
	//   contains at least two elements
	//

	//
	// Check edge conditions in order to reduce the number of
	// compare-operations in the body of the main loop
	//
	if (pstPointArr[0].fFreq < pstPointHead->fFreq) {
		// The first point becomes the list head
		pstPointArr[0].next = pstPointHead;
		*ppstPointHead = &(pstPointArr[0]);
        i1 = 1;
		pScan = pstPointHead;
		pPrev = &(pstPointArr[0]);
	}
	else {
		i1 = 0;
		pScan = pstPointHead->next;
		pPrev = pstPointHead;
	}
	// From the current line it is guaranteed that any point to be checked
	// cannot become the list head


	if (pstPointArr[iArrLen-1].fFreq > pstPointTail->fFreq) {
		pstPointTail->next = &(pstPointArr[iArrLen-1]);
		*ppstPointTail =  &(pstPointArr[iArrLen-1]);
		pstPointArr[iArrLen-1].next = NULL;
		i2 = iArrLen-2;
	}
	else
		i2 = iArrLen-1;
	// From the current line it's guaranteed that any point to be checked
	// cannot become the list tail
		

	//
	//      The main loop
	//
	/*sprintf(test, "LOG : i1 : %d, i2 : %d\n", i1, i2);
	if (testf != NULL) {
		fwrite(test, 1, strlen(test), testf);
	}*/
	for (i=i1; i<=i2; i++) {
		fFreq = pstPointArr[i].fFreq;

		/*sprintf(test, "LOG : fFreq : %f\n", fFreq);
		if (testf != NULL) {
			fwrite(test, 1, strlen(test), testf);
		}*/
		for (; fFreq > pScan->fFreq; pPrev = pScan, pScan = pScan->next) {
			/*sprintf(test, "LOG : pScan->fFreq : %f\n", pScan->fFreq);
			if (testf != NULL) {
				fwrite(test, 1, strlen(test), testf);
			}*/
		}
		pPrev->next = &(pstPointArr[i]);
		pstPointArr[i].next = pScan;
		pPrev = &(pstPointArr[i]);
	}

	/*if (testf != NULL) {
		fclose(testf);

	}*/
}


/*----------------------------------------------------------------------------
 * FUNCTION NAME:  ConvertLinkedListOfDiffPointsToUtilFunc
 *
 * PURPOSE:      Converts differential utility function given by a linked
 *               list to utility function.
 *
 * INPUT
 *   pstPointHead     the linked list head
 * OUTPUT
 *   pstPointArr      array of the utility function nodes
 *   piArrLen         the array length
 * RETURN VALUE
 *   none
 *
 *---------------------------------------------------------------------------*/
static void ConvertLinkedListOfDiffPointsToUtilFunc(
				XPOINT  *pstPointHead,
				POINT   *pstPointArr,
				int     *piArrLen
				)
{
	int i;
	XPOINT *pCurr;


	pstPointArr[0].fAmp = pstPointHead->fAmp;
	pstPointArr[0].fFreq = pstPointHead->fFreq;
	pCurr = pstPointHead->next;
	i = 0;
	for ( ; pCurr!=NULL; pCurr = pCurr->next) {
		if (pCurr->fFreq > pstPointArr[i].fFreq) {
			i++;
			pstPointArr[i].fAmp = pstPointArr[i-1].fAmp + pCurr->fAmp;
			pstPointArr[i].fFreq = pCurr->fFreq;
		}
		else {
			pstPointArr[i].fAmp += pCurr->fAmp;
		}
	}

	*piArrLen = i+1;
}


/*----------------------------------------------------------------------------
 * FUNCTION NAME:    Compare_ARRAY_OF_XPOINTS  
 *
 * PURPOSE:       Call-back function to be used for sorting array of
 *                ARRAY_OF_XPOINTS elements by their length
 * INPUT:
 *   pvValue1     pointer to ARRAY_OF_XPOINTS
 *   pvValue2     pointer to ARRAY_OF_XPOINTS
 * OUTPUT
 *   none
 * RETURN VALUE
 *   1,-1,0
 *   
 *---------------------------------------------------------------------------*/
static int Compare_ARRAY_OF_XPOINTS(const void *pvValue1, const void *pvValue2)
{
	int iLen1 = ((ARRAY_OF_XPOINTS*)pvValue1)->length;
	int iLen2 = ((ARRAY_OF_XPOINTS*)pvValue2)->length;

	if (iLen1 < iLen2)
		return -1;
	if (iLen1 > iLen2)
		return 1;

	return 0;
}



/*----------------------------------------------------------------------------
 * FUNCTION NAME:  CalcUtilityFunction
 *
 * PURPOSE:      Calculates pitch utility function within given frequency
 *               band
 *
 * INPUT
 *   pstRom             Pitch ROM tables and parameters
 *   pvScratchMemory    Work memory
 *   fMinFreq           lower limit of the frequency band
 *   fMaxFreq           upper limit of the frequency band
 *   pstPeaks           array of spectral peaks
 *   iNoOfPeaks         number of spectral peaks
 *   
 * OUTPUT
 *   pstPoints          array of nodes (edge points) defining piecewise
 *                      constant utility function
 *   piNoOfPoints       number of nodes
 * INPUT/OUTPUT
 *   pstEstimator       pitch estimator data structure
 * RETURN VALUE
 *   none
 *
 *---------------------------------------------------------------------------*/
static void CalcUtilityFunction(
		      const DSR_PITCH_ROM *pstRom,
		      void  *pvScratchMemory,
		      const float  fMinFreq,
		      const float  fMaxFreq,
		      const POINT *pstPeaks, 
		      int    iNoOfPeaks,
		      POINT *pstPoints,
		      int   *piNoOfPoints,
		      DSR_PITCH_ESTIMATOR *pstEstimator
		      )									  
{
  int i, rc, iMaxNoOfBreakPoints;
  const FRACTIONS *pstFrac = pstRom->astFrac;

  XPOINT *pstTmpPointsBuf, *pstTmpPoints, *pstPointHead, *pstPointTail;
  ARRAY_OF_XPOINTS  *pstArrayOfXpoints;

  int iTmpOffset;

  iMaxNoOfBreakPoints = 4*pstEstimator->iCreatePieceWiseFunc_LoopLimit +
    2*iNoOfPeaks;
  //
  // Allocate memory from reusable scratch memory space
  //

  pstTmpPointsBuf = (XPOINT*)pvScratchMemory;
  pstArrayOfXpoints = (ARRAY_OF_XPOINTS*)(pstTmpPointsBuf + iMaxNoOfBreakPoints);

  //
  // Build differential utility function component for each peak
  //
  pstTmpPoints = pstTmpPointsBuf;
  for(i=0 ; i<iNoOfPeaks; i++) {
	
    rc = CreatePieceWiseConstantFunction(pstPeaks[i],
					 fMinFreq,
					 fMaxFreq,
					 pstFrac,
					 pstTmpPoints,
					 &(pstArrayOfXpoints[i].length),
					 &(pstEstimator->iCreatePieceWiseFunc_LoopCount),
					 pstEstimator->iCreatePieceWiseFunc_LoopLimit,
					 &iTmpOffset);
    if (rc) 
      break;

    // Save the address of the newly created array of points
    pstArrayOfXpoints[i].addr = pstTmpPoints + iTmpOffset;

    // Calculate output address for the following call to the
    // CreatePieceWiseConstantFunction()
    pstTmpPoints = pstArrayOfXpoints[i].addr + pstArrayOfXpoints[i].length;

  }

  iNoOfPeaks = i; // We could leave the loop upon rc

  //
  // Order the XPoints arrays in ascending order of their lengths
  //
  qsort(pstArrayOfXpoints,
	iNoOfPeaks,
	sizeof(ARRAY_OF_XPOINTS),
	Compare_ARRAY_OF_XPOINTS);


  // 
  //  Skip zero-length arrays if any
  //
  for (i=0; i<iNoOfPeaks && pstArrayOfXpoints[i].length==0; i++);

  if (i == iNoOfPeaks) {
    *piNoOfPoints = 0;
    return;
  }

  //
  //   Merge individual arrays together
  //
  LinkArrayOfPoints(pstArrayOfXpoints[i].addr, pstArrayOfXpoints[i].length);
  pstPointHead = pstArrayOfXpoints[i].addr;
  pstPointTail = pstPointHead + pstArrayOfXpoints[i].length -1;
  for (i++; i < iNoOfPeaks; i++) {
    AddSortedArrayOfPoints(&pstPointHead,
			   &pstPointTail,
			   pstArrayOfXpoints[i].addr,
			   pstArrayOfXpoints[i].length);
  }
				

  //
  //  Convert linked list of differential points
  //  to an array representing piecewise constant utility function
  //
  ConvertLinkedListOfDiffPointsToUtilFunc(pstPointHead,
					  pstPoints,
					  piNoOfPoints);
  // Terminate the utility function
  if (pstPoints[*piNoOfPoints-1].fFreq < fMaxFreq) {
    pstPoints[*piNoOfPoints].fAmp = 0.f;
    pstPoints[*piNoOfPoints].fFreq = fMaxFreq;
    (*piNoOfPoints)++;
  }
  

}


/*----------------------------------------------------------------------------
 * FUNCTION NAME:   FindDominantLocalMaximaInUtilityFunction
 *
 * PURPOSE:        Determine MAX_PRELIM_CANDS "best" local maxima on the
 *                 (piece-wise constant) utility function.  
 *                 The best maxima are either highest ones or those high enough
 *                 located in a vicinity of pre-defined pitch frequency if it is
 *                 specified
 * INPUT:
 *   pstPoints       array of nodes defining the piecewise constant utility function
 *   iNoOfPoints     number of nodes
 *   fStableFreq     pre-define pitch frequency or 0 
 *
 * OUTPUT
 *   pstPitchCand    the best maxima found
 *   piNoOfCand      number of the best maxima found
 * RETURN VALUE
 *   none
 *   
 *---------------------------------------------------------------------------*/
static void FindDominantLocalMaximaInUtilityFunction(	
	        const POINT *pstPoints,
                const int    iNoOfPoints,
                const float  fStableFreq,
                PITCH        *pstPitchCand,
                int          *piNoOfCand
				)
{
	int i, k, j, m;
	int iNoOfCand = 0;
	// The following arrays can be "allocated" on scratch memory
	float afAmpBest[MAX_PRELIM_CANDS];
	int aiIndBest[MAX_PRELIM_CANDS];
 
	if (0==iNoOfPoints) {
	  /* "Empty" utility function */
	  *piNoOfCand = 0;
	  return;
	}
	
	//
	// First, we identify all local maxima in the utility function
	// Since the amplitude of the function is assumed constant until
	// the next edge-point, we do not know exactly where the maxima
	// are. So we take a maxima frequency which is in the middle 
	// between the two edge points.

	// Take care of first point
	if (pstPoints[0].fAmp > pstPoints[1].fAmp ) {
	  afAmpBest[0] = pstPoints[0].fAmp;
	  aiIndBest[0] = 0;
	  iNoOfCand++;
	}
  
 
	// Check for local maxima in amplitude
	for (i=1; i<iNoOfPoints-1; i++) {
	  if (pstPoints[i].fAmp >= pstPoints[i-1].fAmp &&
	      pstPoints[i].fAmp >  pstPoints[i+1].fAmp) {
	    // This is a local maximum - compare to the stored ones
	    for (k=0; k<iNoOfCand; k++) 
	      if (pstPoints[i].fAmp >= afAmpBest[k]) break;
	    if (k == *piNoOfCand) { 
	      // Too small amplitude - it would not fall in the
	      // list of maximal length
	      continue;
	    }
	    // Insert/add at k-th position
	    if (iNoOfCand < *piNoOfCand) {
	      j = iNoOfCand;
	      iNoOfCand++;
	    }
	    else
	      j = iNoOfCand-1;

	    for (m=j-1; j >k; j--,m--) {
	      afAmpBest[j] = afAmpBest[m];
	      aiIndBest[j] = aiIndBest[m];
	    }
	    afAmpBest[k] = pstPoints[i].fAmp;
	    aiIndBest[k] = i;
	    
            i++; // the next point cannot be a local maximum
	  } //endif local maximum
	} // end for (i=1; i<iNoOfPoints-1; i++)
   
	i = iNoOfPoints-1;

	// Take care of the last point. The frequency here is always
	// MAX_PITCH_FREQ, and its amplitude applies to this frequency
	// alone
	if (pstPoints[i].fAmp > pstPoints[i-1].fAmp ) {
	  // This is a local maximum - compare to the stored ones
	  for (k=0; k<iNoOfCand; k++) 
	    if (pstPoints[i].fAmp >= afAmpBest[k]) break;
	  if (k < *piNoOfCand) {
	    if (iNoOfCand < *piNoOfCand) {
	      j = iNoOfCand;
	      iNoOfCand++;
	    }
	    else
	      j = iNoOfCand-1;
	    // Insert/add at k-th position
	    for (m=j-1; j >k; j--,m--) {
	      afAmpBest[j] = afAmpBest[m];
	      aiIndBest[j] = aiIndBest[m];
	    }
	    afAmpBest[k] = pstPoints[i].fAmp;
	    aiIndBest[k] = i;
	  } //endif local maximum
	}


	//
	// Try to find a local maximum in vicinity of the stable track pitch
	//
	if (fStableFreq != UNVOICED) {
	  float fLowFreq  = STABLE_FREQ_LOWER_MARGIN * fStableFreq;
	  float fHighFreq = STABLE_FREQ_UPPER_MARGIN * fStableFreq;
	  float fAmp0 = 0.f;
	  int   iInd0=0; /* Just to calm some compilers nervous */
      
	  // Find the best local maximum in the vicinity of the stable pitch
	  for (i=1; i<iNoOfPoints-1; i++) {
            if (pstPoints[i].fFreq < fLowFreq)  continue;
            if (pstPoints[i].fFreq > fHighFreq) break;
	    if (pstPoints[i].fAmp >= pstPoints[i-1].fAmp &&
		pstPoints[i].fAmp >  pstPoints[i+1].fAmp) {
	      // Local maximum
	      if (pstPoints[i].fAmp > fAmp0) {
		fAmp0 = pstPoints[i].fAmp;
		iInd0 = i;
	      }
            }
	  }

	  if (fAmp0 > 0.f) {
            // Check if the local maximum found is already stored
            for (k=0; k<iNoOfCand; k++)
	      if (iInd0 == aiIndBest[k]) break;
            if (k < iNoOfCand) // it is stored - nothing to be done
	      fAmp0 = 0.f;
	  }

	  // Compare to the stored candidates
	  if (fAmp0 > 0.f) {
            fAmp0 += AMP_MARGIN2;
            for (k=0; k<iNoOfCand; k++) 
	      if (fAmp0 >= afAmpBest[k]) break;
	    if (k < *piNoOfCand) { 				
	      // Insert/add at k-th position
	      if (iNoOfCand < *piNoOfCand) {
		j = iNoOfCand;
		iNoOfCand++;
	      }
	      else j = iNoOfCand-1;
	      for (m=j-1; j >k; j--,m--) {
		afAmpBest[j] = afAmpBest[m];
		aiIndBest[j] = aiIndBest[m];
	      }
	      afAmpBest[k] = pstPoints[iInd0].fAmp;
	      aiIndBest[k] = iInd0;
            } // endif insert
	  } // endif a local maximum found

			    
	} // endif there is a stable pitch to look around it





	//
	//   Fill in the pitch candidate structures
	//
	for (k=0; k<iNoOfCand-1; k++) {
	  i = aiIndBest[k];
	  pstPitchCand[k].fAmp = pstPoints[i].fAmp;
	  pstPitchCand[k].fFreq = 0.5f*(pstPoints[i+1].fFreq + pstPoints[i].fFreq);
	}
	// The last candidate if obtained from the last point requires special
	// care
	i = aiIndBest[k];
	pstPitchCand[k].fAmp = pstPoints[i].fAmp;
	if (i == iNoOfPoints-1) {
	  pstPitchCand[k].fFreq = pstPoints[i].fFreq;
	}
	else {
	  pstPitchCand[k].fFreq = 0.5f*(pstPoints[i+1].fFreq + pstPoints[i].fFreq);
	}
			
	(*piNoOfCand) = iNoOfCand;

}




/*----------------------------------------------------------------------------
 * FUNCTION NAME: UtilityFunctionAtGivenPitchFreq
 *
 * PURPOSE:       Calculate the utility function value at given pitch frequency
 *
 * INPUT:
 *   pstSpectralPeaks            array of spectral peaks
 *   iNoOfSpectralPeaks          number of spectral peaks
 *   fPitchFreq                  pitch frequency
 *   
 * OUTPUT
 *   none
 *
 * RETURN VALUE
 *   U(fPitchFreq) - value of the utility function at given pitch frequency
 *   
 *---------------------------------------------------------------------------*/
static float UtilityFunctionAtGivenPitchFreq(
						const POINT *pstSpectralPeaks,
						const int    iNoOfSpectralPeaks,
						const float  fPitchFreq
						)
{ 
	float fInvPitchFreq = 1.0f/fPitchFreq ;
	float fArg, fAmp = 0.0f;
	int i;

	for(i=0; i<iNoOfSpectralPeaks; i++) {
		fArg = fInvPitchFreq * pstSpectralPeaks[i].fFreq;
		fArg -= (int)(fArg);
		if (fArg > 0.5f) fArg = 1.0f-fArg;
		// At that point fArg = ABS(Fpeak/F0 - ROUND(Fpeak/F0))

		if (fArg < UDIST1)
			fAmp += pstSpectralPeaks[i].fAmp;
		else if (fArg < UDIST2)
			fAmp += USTEP*pstSpectralPeaks[i].fAmp;   
	}

	return fAmp;
}




/* ===============================================================
 *
 *                 INTERNAL FUNCTIONS
 *
 *             Pitch candidates determination and processing 
 * =============================================================== */


/*----------------------------------------------------------------------------
 * FUNCTION NAME:     ComparePitchFreqAscending
 *
 * PURPOSE:       Call-back function to be used for sorting array of PITCH elements  
 *                by associated pitch frequency values
 * INPUT:
 *   pvValue1     pointer to PITCH data structure
 *   pvValue2     pointer to PITCH data structure
 * OUTPUT
 *   none
 * RETURN VALUE
 *   1,-1,0
 *   
 *---------------------------------------------------------------------------*/
static int ComparePitchFreqAscending(const void *pvValue1, const void *pvValue2)
{
 float fFreq1 = ((PITCH *) pvValue1)->fFreq;
 float fFreq2 = ((PITCH *) pvValue2)->fFreq;

 if( fFreq1 < fFreq2 )
    return(-1);
 if( fFreq1 > fFreq2)
    return(+1);

 return(0);
}

/*----------------------------------------------------------------------------
 * FUNCTION NAME:     ComparePitchFreqDescending
 *
 * PURPOSE:       Call-back function to be used for sorting array of PITCH elements  
 *                by associated pitch frequency values
 * INPUT:
 *   pvValue1     pointer to PITCH data structure
 *   pvValue2     pointer to PITCH data structure
 * OUTPUT
 *   none
 * RETURN VALUE
 *   1,-1,0
 *   
 *---------------------------------------------------------------------------*/
static int ComparePitchFreqDescending(const void *pvValue1, const void *pvValue2)
{
 float fFreq1 = ((PITCH *) pvValue1)->fFreq;
 float fFreq2 = ((PITCH *) pvValue2)->fFreq;

 if( fFreq1 > fFreq2 )
    return(-1);
 if( fFreq1 < fFreq2)
    return(+1);

 return(0);
}




/*----------------------------------------------------------------------------
 * FUNCTION NAME:  SelectTopPitchCandidates
 *
 * PURPOSE:        Select a predefined number of the best elements (most likely
 *                 pitch candidates) out of given array of pitch candidates.
 *                 Pitch candidate quality measure is combined from the fAmp
 *                 value and closeness to the previous frame pitch frequency while
 *                 some preference is given to higher pitch frequencies
 * INPUT:
 *   pstPitchCand     array of pitch candidates sorted in ascending order of
 *                    the pitch frequency
 *   iNoOfPitchCand   number of pitch candidates
 *   fPrevPitchFreq   pitch frequency estimate obtained for the previous frame,
 *                    or 0 indicating unvoiced frame
 *   iNoOfOutPitchCand  maximal number of best candidates to find
 * OUTPUT
 *   pstPitch         array of the best pitch candidates
 * RETURN VALUE
 *   none
 *   
 *---------------------------------------------------------------------------*/
static void SelectTopPitchCandidates(
                            const PITCH *pstPitchCand,
                            const int    iNoOfPitchCand,
                            const float  fPrevPitchFreq,
			    const int    iNoOfOutPitchCand,
                            PITCH        *pstPitch
			    )
{
  int i, j, k, m;
  int iIndexBest[MAX_BEST_CANDS];
  float fAmpBest[MAX_BEST_CANDS], fFreqBest[MAX_BEST_CANDS];
  BOOL bCond1, bCond2;
 

  for (i=0; i<iNoOfOutPitchCand; i++) {
    pstPitch[i].fFreq       = UNVOICED ; 
    pstPitch[i].fAmp        = 0 ;
    fAmpBest[i] = -AMP_MARGIN1;
    fFreqBest[i] = 0.f;
    iIndexBest[i] = -1;
  }


  if (0==iNoOfPitchCand)
    return;

  //
  // Select best pitch candidate from maxima list
  // Starting from the highest frequency to the lowest,
  // one of two conditions below has to be true in order to
  // select a new pitch
  //

  for(i=iNoOfPitchCand-1; i>=0; i--) {
    
    for (k=0; k<iNoOfOutPitchCand; k++) {
      bCond1 = (pstPitchCand[i].fAmp > fAmpBest[k] + AMP_MARGIN1);
      bCond2 = (pstPitchCand[i].fAmp > fAmpBest[k])  &&  
	(FREQ_MARGIN1*pstPitchCand[i].fFreq > fFreqBest[k]);
      if (bCond1 || bCond2) break;
    }
    if (k < iNoOfOutPitchCand) {
      // The pstPitchCand[i] should be inserted at k-th position
      for (j=iNoOfOutPitchCand-1; j>k; j--) {
	m = j-1;
	iIndexBest[j] = iIndexBest[m];
	fAmpBest[j] = fAmpBest[m];
	fFreqBest[j] = fFreqBest[m];
      }
      iIndexBest[k] = i;
      fAmpBest[k] = pstPitchCand[i].fAmp;
      fFreqBest[k] = pstPitchCand[i].fFreq;
    }
  }

  //
  // If this frame is part of a stable pitch track,
  // check the possibility to replace the pitch that
  // was selected above by another pitch which is more
  // close to the pitch of the previous frame. This is how
  // we make sure that we continue the pitch track -- by
  // giving a second chance to one of the maxima points
  // to be selected.
  //
  if (fPrevPitchFreq != 0.f) {
    
    // Define a frequency region aroud the pitch of the 
    // previous frame
    float fLowFreq  = STABLE_FREQ_LOWER_MARGIN * fPrevPitchFreq ;
    float fHighFreq = STABLE_FREQ_UPPER_MARGIN * fPrevPitchFreq ;
	
  
    //
    // Go back to the local maxima list, and find the maximum
    // point in this list with frequency that falls in the
    // range defined above
    //	
    float fAmpAlt = 0.0f;
    int iIndexAlt = -1;
		  
    for (i=0; i<iNoOfPitchCand && pstPitchCand[i].fFreq<fHighFreq; i++) {
      if (pstPitchCand[i].fFreq  > fLowFreq &&
	  pstPitchCand[i].fAmp  >= fAmpAlt) {
	fAmpAlt = pstPitchCand[i].fAmp;     
	iIndexAlt       = i;
      }
    }


    if (iIndexAlt >=0) {
      // Check if the alternative candidate already appears in the list
      for (k=0; k<iNoOfOutPitchCand; k++) {
	if (iIndexBest[k] == iIndexAlt) break;
      }
      if (k<iNoOfOutPitchCand) { // Appears - do not insert it twice!
	iIndexAlt = -1;
      }
    }

    if (iIndexAlt >=0) {
      // Check if the alternative candidate is good enough
      for (k=0; k<iNoOfOutPitchCand; k++) {
	if (fAmpAlt + AMP_MARGIN2  > fAmpBest[k]) break;
      }
      if (k<iNoOfOutPitchCand) {
	// Insert the alternative candidate
	for (j=iNoOfOutPitchCand-1; j>k; j--) {
	  m = j-1;
	  iIndexBest[j] = iIndexBest[m];				
	}
	iIndexBest[k] = iIndexAlt;
      } // endif insert alternative pitch
    } // endif alternative pitch found
  } // endif stable pitch contour

   
  // Copy the best candidates
  for (i=0; i<iNoOfOutPitchCand && iIndexBest[i]>=0; i++)
    pstPitch[i] = pstPitchCand[iIndexBest[i]]; 
	
}


/*----------------------------------------------------------------------------
 * FUNCTION NAME:  FindPitchCandidates
 *
 * PURPOSE:    Determine a few most likely pitch candidates within predefined
 *             frequency band. Compute the candidates  spectral and correlation scores
 *
 * INPUT
 *   pstRom      -    Pitch ROM structure
 *   pstEstimator - Pitch estimator data structure
 *   fLowPitchFreq - frequency band lower limit
 *   fHighPitchFreq - frequency band upper limit
 *   pfProcSpeechForCorr - low-pass filtered and downsampled segment of speech
 *                         signal as produced by pre_process() function
 *                         (see preProc.c)
 *   iPSFC_DownSampFactor - downsampling factor passed to pre_process()
 *   iNoOfTopCandidates - maximal number of pitch candidates to be found
 * OUTPUT
 *   pstTopCand - array of pitch candidates
 *
 * RETURN VALUE
 *   none
 *
 * PRE_CONDITION
 *   PrepareSpectralPeaks() has been called before  
 *---------------------------------------------------------------------------*/
static void FindPitchCandidates(
		const DSR_PITCH_ROM   *pstRom,
		DSR_PITCH_ESTIMATOR   *pstEstimator,
		PCORR_DATA			  *pCorrData,
		float                 fLowPitchFreq,
		float                 fHighPitchFreq,
		float                 *pfProcSpeechForCorr,
		int                   iPSFC_DownSampFactor,
		int                   iNoOfTopCandidates,
		PITCH                 *pstTopCand
)
{
  int  iNumOfPeaks, i, iNumOfPeaksFinally;
  float fSum1, fSum2, fNorm2;
  
  POINT *pstSpectralPeaks = (POINT*)pstEstimator->pvScratchMemory;
  void  *pvScratchMemoryLeft = (void*)
    ((char*)pstEstimator->pvScratchMemory + MAX_PEAKS_PRELIM*sizeof(POINT));

  if (pstEstimator->iNoOfSpectralPeaks == 0) {
    pstEstimator->iNoOfPitchCand = 0;
    return;
  }

	
  //
  // Select small number of the highest spectral peaks
  //
  iNumOfPeaks = MIN(pstEstimator->iNoOfSpectralPeaks,MAX_PEAKS_PRELIM);  
  for (i=0; i<iNumOfPeaks; i++)
    pstSpectralPeaks[i] = pstEstimator->pstSpectralPeaks[i];
  fSum1 = NormalizeAmplitudes(pstSpectralPeaks,iNumOfPeaks);
  //
  // Calculate the utility function using this limited set of spectral peaks
  //
  CalcUtilityFunction(pstRom,
		      pvScratchMemoryLeft,
		      fLowPitchFreq,
		      fHighPitchFreq,
		      pstSpectralPeaks,					 
		      iNumOfPeaks,
		      pstEstimator->pstPoints,
		      &pstEstimator->iNoOfPoints,
		      pstEstimator);
	
  if (pstEstimator->iNoOfPoints == 0) {
    pstEstimator->iNoOfPitchCand = 0;
    return;
  }
 
  //
  // Find preliminary pitch candidates
  //
  pstEstimator->iNoOfPitchCand = MAX_PRELIM_CANDS; 
  FindDominantLocalMaximaInUtilityFunction(
					   pstEstimator->pstPoints,
					   pstEstimator->iNoOfPoints,
					   pstEstimator->stStableTrackPitch.fFreq,
					   pstEstimator->pstPitchCand,
					   &(pstEstimator->iNoOfPitchCand));
  
  if (pstEstimator->iNoOfPitchCand == 0)
    return;
	
	
  //
  // Calculate utility function value for the preliminary candidates using
  // all the spectral peaks 
  //
  iNumOfPeaksFinally = MIN(MAX_PEAKS_FINAL,pstEstimator->iNoOfSpectralPeaks);
		

  fSum2 = SumAmplitudes(pstEstimator->pstSpectralPeaks, iNumOfPeaksFinally);
		
  fNorm2 = 1.f/fSum2;	
  for (i=0; i<pstEstimator->iNoOfPitchCand; i++) {
    pstEstimator->pstPitchCand[i].fAmp = fNorm2* 
      UtilityFunctionAtGivenPitchFreq(pstEstimator->pstSpectralPeaks,
				      iNumOfPeaksFinally,
				      pstEstimator->pstPitchCand[i].fFreq);
  }

  
  // Sort pitch candidates in ascending order of frequency
  if (pstEstimator->iNoOfPitchCand > 0) {
    qsort(pstEstimator->pstPitchCand,pstEstimator->iNoOfPitchCand,sizeof(PITCH),
	  ComparePitchFreqAscending);
  }

  // Select small number of final pitch candidates
  SelectTopPitchCandidates(pstEstimator->pstPitchCand,
			   pstEstimator->iNoOfPitchCand,
			   pstEstimator->stStableTrackPitch.fFreq, 
			   iNoOfTopCandidates,
			   pstTopCand);
  // Compute correlation score for each final pitch candidate
  for (i=0; i < iNoOfTopCandidates; i++) {
    if (pstTopCand[i].fFreq != UNVOICED)
      compute_pcorr(pfProcSpeechForCorr,
		    iPSFC_DownSampFactor,
		    (float)pstRom->iSampleRate,
		    pstRom->iWinSize,
		    pstTopCand[i].fFreq,
		    pstEstimator->iFrameNo,
		    &(pstTopCand[i].fCorr),
			pCorrData);				
  }


}


/* ===============================================================
 *
 *                 INTERNAL FUNCTIONS
 *
 *               Miscellaneous routines
 * =============================================================== */


/*----------------------------------------------------------------------------
 * FUNCTION NAME:  IsLowLevelInput
 *
 * PURPOSE:      Compare log-energy value to a pre-defined threshold
 *
 * INPUT
 *   fLogEnergy - log energy
 *
 * OUTPUT
 *   none
 *
 * RETURN VALUE
 *   TRUE log-energy is less than the threshold
 *   FALSE otherwise
 *---------------------------------------------------------------------------*/
static BOOL IsLowLevelInput(const float fLogEnergy)
{
 if (fLogEnergy < LOG_ENERGY_THRESHOLD)
  return (TRUE);
 else
  return (FALSE);
}



/*----------------------------------------------------------------------------
 * FUNCTION NAME:  ClearPitch
 *
 * PURPOSE:    Set PITCH structure to UNVOICED state 
 *
 * INPUT/OUTPUT
 *   pstPitch      -    pitch structure instance
 *
 * RETURN VALUE
 *   none
 *
 *---------------------------------------------------------------------------*/
static void ClearPitch(PITCH *pstPitch)
{
	pstPitch->fAmp = pstPitch->fCorr = 0.f;
	pstPitch->fFreq = UNVOICED;	
}



/*----------------------------------------------------------------------------
 * FUNCTION NAME:  IsContinuousPitch
 *
 * PURPOSE:    Determine if the pitch values associated with two consequent
 *             frames are close enought to each other
 * INPUT
 *   pstPrevPitch  - first frame pitch
 *   pstPitch      -second frame pitch
 * OUTPUT
 *   none
 * RETURN VALUE
 *   TRUE - yes
 *   FALSE - no
 *---------------------------------------------------------------------------*/
static BOOL IsContinuousPitch(const PITCH *pstPrevPitch,
                              PITCH *pstPitch)
{
	BOOL b;

	if (pstPitch->fFreq == UNVOICED || pstPrevPitch->fFreq == UNVOICED)
		return FALSE;
	b = (pstPitch->fFreq > STABLE_FREQ_LOWER_MARGIN*pstPrevPitch->fFreq) && 
		(pstPitch->fFreq < STABLE_FREQ_UPPER_MARGIN*pstPrevPitch->fFreq);
	return b;
}



/*----------------------------------------------------------------------------
 * FUNCTION NAME:  SelectFinalPitch
 *
 * PURPOSE:    Select final pitch estimate out of several candidates
 *
 * INPUT
 *   pPitchCand - pitch candidates list
 *   iNoOfPitchCands -  number of the candidates
 *   pPrevPitch      - pitch estimate obtained for the previous frame     
 *   iStableFrameCount - length of continuous pitch segment, see Finalize()
 *   pLastStableTrackPitch - pitch estimate associated with the last frame
 *                           belonging to the stable track, see Finalize()
 *   bStrictMode - TRUE if the function is called with a partial pitch candidates
 *                 list; FALSE if it is called with the complete list of candidates
 *
 * OUTPUT
 *   pBestPitchCand - final pitch
 * RETURN VALUE
 *   none
 *
 *---------------------------------------------------------------------------*/
static void  SelectFinalPitch( 								  
			      PITCH  *pPitchCand,
			      int    iNoOfPitchCands,
			      PITCH  *pPrevPitch,     
			      int    iStableFrameCount,
			      PITCH  *pLastStableTrackPitch,
			      PITCH  *pBestPitchCand,
			      BOOL   bStrictMode
			      )
{
  //
  // Some macros used exclusively within the function
  //
#define CLOSELY_LOCATED(pit1,pit2) \
((pit1).fFreq*1.20f > (pit2).fFreq && (pit2).fFreq*1.20f > (pit1).fFreq)



#define GOOD_ENOUGH(pit) \
( ((pit).fAmp >= 0.78f && (pit).fCorr >= 0.79f)  || \
  ((pit).fAmp >= 0.68f && (pit).fAmp + (pit).fCorr >= 1.6f) )


#define BETTER(pit1,pit2) \
((pit1).fCorr > (pit2).fCorr && (pit1).fAmp > (pit2).fAmp)

	
  int i, iFirstGood, iGood;
        

  // Sort the candidates in descending frequency order,
  // and determine the actual number of candidates
  qsort(pPitchCand,iNoOfPitchCands,sizeof(PITCH),
	ComparePitchFreqDescending);
  for (i=0; i<iNoOfPitchCands && pPitchCand[i].fFreq != UNVOICED; i++);	        
  iNoOfPitchCands = i;
  if (0 == iNoOfPitchCands) {
    ClearPitch(pBestPitchCand); // Unvoiced        
    return;
  }

  /* -----------------------------------------------------------------------------
     Try to find a pitch candidate with high spectral & high correlation scores
     (high amp & high corr) 
     --------------------------------------------------------------------------- */

  iGood = -1;
  for (i=0; i<iNoOfPitchCands; i++) {				
    if (GOOD_ENOUGH(pPitchCand[i])) {
      iGood = i;
      break;
    }
  }          
    
  if (iGood >= 0) {
    /* High amp & high corr pitch is found */
    float fCrit0, fCrit;
    
    /* Try to find a better candidate in the close vicinity */
    iFirstGood = iGood;
    for (i=iGood+1; i<iNoOfPitchCands; i++) {
      if (!CLOSELY_LOCATED(pPitchCand[iFirstGood],pPitchCand[i]))
	break;
      if (BETTER(pPitchCand[i],pPitchCand[iGood])) 
	iGood = i;
    }
    /* Try to find significantly better candidate */
    fCrit0 = pPitchCand[iGood].fAmp + pPitchCand[iGood].fCorr + 0.18f;
    for ( ; i<iNoOfPitchCands; i++) {				
      if (GOOD_ENOUGH(pPitchCand[i])) { 
	fCrit = pPitchCand[i].fAmp + pPitchCand[i].fCorr;
	if (fCrit >= fCrit0) {
	  iGood = i;
	  break;
	}
      }
    }
    /* Try to find a better candidate in the close vicinity */
    iFirstGood = iGood;
    for (i++; i<iNoOfPitchCands; i++) {
      if (!CLOSELY_LOCATED(pPitchCand[iFirstGood],pPitchCand[i]))
	break;
      if (BETTER(pPitchCand[i],pPitchCand[iGood])) 
	iGood = i;
    }
    
  }


  if (bStrictMode) {
    /* ----------------------------------------------------
       Stric Mode: accept only very high quality candidate
                   otherwise reject
       --------------------------------------------------- */
    if (iGood >= 0 &&
	pPitchCand[iGood].fAmp >= 0.95f &&
	pPitchCand[iGood].fCorr >= 0.95f)
      *pBestPitchCand = pPitchCand[iGood];            
    else
      ClearPitch(pBestPitchCand); // Not found
    
    return;
  }

  /* Not Strict Mode: if an estimate is found then accept and return */
  if (iGood >=0) {
    *pBestPitchCand = pPitchCand[iGood];        
    return;
  }



  if (pLastStableTrackPitch->fFreq!=UNVOICED) {
    
    /* --------------------------------------------------
       We're on stable pitch track or close to it.
       Try to continue the track using less restrictive
       thresholds. Declare UNVOICED if do not manage.
       ------------------------------------------------- */
     
    for (i=0; i<iNoOfPitchCands; i++) {
      if (IsContinuousPitch(pLastStableTrackPitch,&pPitchCand[i]) &&
	  (pPitchCand[i].fAmp > 0.70f || pPitchCand[i].fCorr > 0.70f))
	break;
    }

    if (i < iNoOfPitchCands) {
      /* Try to find a better candidate in the close vicinity */
      iFirstGood = iGood = i;
      for (i=iGood+1; i<iNoOfPitchCands; i++) {
	if (!CLOSELY_LOCATED(pPitchCand[iFirstGood],pPitchCand[i]))
	  break;
	if (BETTER(pPitchCand[i],pPitchCand[iGood])) 
	  iGood = i;
      }

      *pBestPitchCand = pPitchCand[iGood];                                                
    }
    else {
      ClearPitch(pBestPitchCand); // Unvoiced
    }
        
    return;

        
  } // endif we're close from the last stable track


    

  if (pPrevPitch->fFreq!=0 && iStableFrameCount>1) {
    /* ----------------------------------------------------
       At least two previous frames have close pitch.
       Try to find pitch in the vicinity of the previous
       pitch estimate using less limiting requirements
       ---------------------------------------------------- */
    iGood = -1;
    for (i=0; i<iNoOfPitchCands; i++) {
      if (IsContinuousPitch(pPrevPitch,&pPitchCand[i]) &&
	  (pPitchCand[i].fAmp > 0.70f || pPitchCand[i].fCorr > 0.70f)) {
	iGood = i;
	break;       
      }
    }          
    if (iGood >= 0) {
      /* Try to find a better candidate in the close vicinity */
      for (i++; i<iNoOfPitchCands; i++) {
	if (IsContinuousPitch(pPrevPitch,&pPitchCand[i]) &&
	    BETTER(pPitchCand[i],pPitchCand[iGood]))
	  iGood = i;
      }
      *pBestPitchCand = pPitchCand[iGood];            
      return;
    }

  }


  /* ------------------------------------------------------------
     Try to find pitch candidate with very high Amp or very high Corr
     ----------------------------------------------------------- */
  for (i=0; i<iNoOfPitchCands; i++) {				
    if (pPitchCand[i].fAmp >= 0.82f ||			
	pPitchCand[i].fCorr >= 0.85f)
      break;					
  }
  if (i < iNoOfPitchCands) {
    /* Try to find a better candidate in the close vicinity */
    iFirstGood = iGood = i;
    for (i++; i<iNoOfPitchCands; i++) {
      if (!CLOSELY_LOCATED(pPitchCand[iFirstGood],pPitchCand[i]))
	break;
      if (BETTER(pPitchCand[i],pPitchCand[iGood])) 
	iGood = i;
    }
    *pBestPitchCand = pPitchCand[iGood];        
    return;
  }

  ClearPitch(pBestPitchCand); // Unvoiced    
  return;

}


/*----------------------------------------------------------------------------
 * FUNCTION NAME:   Finalize
 *
 * PURPOSE:    Finalize pitch estimation for current frame: set output
 *             pitch value, update pitch estimator for consequent calls
 * INPUT
 *   pstPitch  - final pitch
 * INPUT/OUTPUT
 *   pstEstimator - pitch estimator
 * OUTPUT
 *   pOutPitchPeriod - pitch cycle length as specified in EstimatePitch()
 * RETURN VALUE
 *   none
 *
 *---------------------------------------------------------------------------*/
static void Finalize(
		     DSR_PITCH_ESTIMATOR *pstEstimator,
		     PITCH           *pstPitch,
		     float           *pOutPitchPeriod
		     )
{
  CMPLX *pstDft;
		    	

  // Swap pointers, such that the single-window interpolated input-DFT is saved for
  // next frame. It is required in order to caluclate the DFT of the double-window
  // in the next frame
 
  pstDft                        = pstEstimator->pstDoubleInterpDft;
  pstEstimator->pstDoubleInterpDft = pstEstimator->pstSingleInterpDft;
  pstEstimator->pstSingleInterpDft = pstDft;

		

  //
  // Update return arguments
  //
  if (pstPitch->fFreq == UNVOICED) {
    pstEstimator->iStablePitchCount = 0;
    *pOutPitchPeriod = UNVOICED ;        
  }
  else {
    if (TRUE == IsContinuousPitch(&(pstEstimator->stPrevPitch),pstPitch))
      pstEstimator->iStablePitchCount++;            
    else 
      pstEstimator->iStablePitchCount = 0;
            

    // Convert pitch frequnecy in Hz to period (in non-integer 8 khz samples)
    *pOutPitchPeriod =  REF_SAMPLE_RATE / pstPitch->fFreq ;
  }

  // Save pitch information for next frame (only some of the fields are needed)
  pstEstimator->stPrevPitch = *pstPitch;

  // Update stable pitch mechanism
  if (pstEstimator->iStablePitchCount >= MIN_STABLE_FRAMES) {
    // We're on a stable track
    pstEstimator->iDistFromStableTrack = 0;
    pstEstimator->stStableTrackPitch = *pstPitch;
  }
  else if (pstEstimator->iDistFromStableTrack <= MAX_TRACK_GAP_FRAMES) {
    // We've left the stable track not long ago
    if (IsContinuousPitch(&pstEstimator->stStableTrackPitch,pstPitch)) {
      // We've returned to the stable track soon after we left it
      pstEstimator->iDistFromStableTrack = 0;
      pstEstimator->stStableTrackPitch = *pstPitch;
    }
    else {
      // We did not return to the stable track yet
      pstEstimator->iDistFromStableTrack++;
    }
  }
  else {
    // We're left the stable track finally
    pstEstimator->stStableTrackPitch.fFreq = UNVOICED; // Forget it!
    pstEstimator->iDistFromStableTrack++;
  }

  // Note: stStableTrackPitch.fFreq != 0 indicates that we are on 
  // a stable pitch track - whether original one or renewed one. 

}



/* ===============================================================
 *
 *                 EXTERNAL FUNCTIONS
 *
 * =============================================================== */


/*----------------------------------------------------------------------------
 * FUNCTION NAME: InitPitchRom
 *
 * PURPOSE:       initializes a pitch ROM data structure
 *                containing tables and parameters used for pitch estimation. 
 *
 * INPUT:
 *   none
 *
 * OUTPUT
 *   pPitchRom           Points to a pointer to the pitch ROM data structure
 *
 * RETURN VALUE
 *   0          OK
 *   1          Memory allocation error
 *---------------------------------------------------------------------------*/
int InitPitchRom(
		 void         **pPitchRom
		 )
{
	DSR_PITCH_ROM *pstRom;
	int iMemErr;
	int i;
	float fTeta, fShift, fPiDivN, fInvN;


	*pPitchRom = NULL;


	MALLOCATE(pstRom,DSR_PITCH_ROM,1,iMemErr);
	if (iMemErr) return 1;


	pstRom->iSampleRate = SAMPLING_FREQ_1 * 1000;

	pstRom->iWinSize   = FRAME_LENGTH;
	pstRom->iFrameSize = FRAME_SHIFT;
	pstRom->iDftSize   = FFT_LENGTH;			


	/* DFT bin coresponding to 4kHz */	 
	pstRom->i4KhzDftBin = 
		(int)((REF_BANDWIDTH*pstRom->iDftSize)/(float)pstRom->iSampleRate);

	
	/* Lenght of interpolated DFT */
	pstRom->iInterpDftLen = 2*pstRom->i4KhzDftBin+1;
	


	/* ********************************************* */
	/*    Initialize fractions of untility function  */
	/* ********************************************* */

	for (i=0; i < NO_OF_FRACS; i++) {
	  pstRom->astFrac[i].fFarLow  = 1.0f/( (float)i + UDIST2 );
	  pstRom->astFrac[i].fLow     = 1.0f/( (float)i + UDIST1 ); 
	  pstRom->astFrac[i].fHigh    = 1.0f/( (float)i - UDIST1 );
	  pstRom->astFrac[i].fFarHigh = 1.0f/( (float)i - UDIST2 );
	}

	/* ******************************************************************* */
	/* Initialize complex exponent "shift" factor needed to generate a DFT */
	/*                          the double window                          */
        /* ******************************************************************* */	

	MALLOCATE(pstRom->pstWindowShiftTable,CMPLX,pstRom->iInterpDftLen,iMemErr);
	if (iMemErr) return 1;
  
	fShift = -(M_PI*(float)pstRom->iFrameSize/(float)pstRom->iDftSize);
	for (i = 0; i < pstRom->iInterpDftLen; i++) {
	   fTeta = (fShift * (float)i);
	   pstRom->pstWindowShiftTable[i].fReal = (float)cos(fTeta);
	   pstRom->pstWindowShiftTable[i].fImag = (float)sin(fTeta);
	}

	/* ****************************************** */
	/*     Initialize Dirichlet Interpolation     */
	/* ****************************************** */


	fPiDivN = M_PI/(float)pstRom->iDftSize;
        fInvN = 1.f/(float)pstRom->iDftSize;
	for (i=0; i < DIRICHLET_KERNEL_SPAN; i++) 
            pstRom->afDirichletImag[i] = -fInvN/(float)tan(fPiDivN*(i+0.5f));


	pstRom->iHighPassCutOffDftPoint = (int)(HIGHPASS_CUTOFF_FREQ*
			     2.f*(float)pstRom->iDftSize/(float)pstRom->iSampleRate);
    

	*pPitchRom = (void*)pstRom;
	return 0;
}


/*----------------------------------------------------------------------------
 * FUNCTION NAME: InitPitchEstimator
 *
 * PURPOSE:       Allocates and initializes a pitch estimator data structure
 *                containing RAM and history information used for pitch estimation. 
 *
 * INPUT:
 *   pPitchRom          Output produced by InitPitchRom()
 *
 * OUTPUT
 *   pPitchRomEstimator   Points to a pointer to the pitch estimator data structure
 *
 * RETURN VALUE
 *   0          OK
 *   1          Memory allocation error
 *---------------------------------------------------------------------------*/
int InitPitchEstimator(
		       const void   *PitchRom, 
		       void         **pPitchEstimator			                        
		       )
{
  
  DSR_PITCH_ROM *pstRom;
  DSR_PITCH_ESTIMATOR *pstEstimator;
  int iMemErr, max_spec_peaks, size, size1;
  int iMaxNoOfBreakPoints;
	
  *pPitchEstimator = NULL;

  pstRom = (DSR_PITCH_ROM*)PitchRom;


  MALLOCATE(pstEstimator,DSR_PITCH_ESTIMATOR,1,iMemErr);
  if (iMemErr) return 1;  

  /* Maximal number of break points in derivative of the Utility Function = 
     2 points for each 0-th shape and 4 points x max number of regular
     shapes */
  iMaxNoOfBreakPoints = 4*CREATE_PIECEWISE_FUNC_LOOP_LIM_DBL + 2*MAX_PEAKS_PRELIM;
 	 
  /* ************************************************************************* */
  /*       Allocate RAM scratch memory (reusable memory)                       */
  /* ************************************************************************* */
	
  /* Amount of memory required for FindSpectralPeaks */
  size = 2*pstRom->iInterpDftLen*sizeof(float) + 
    pstRom->iInterpDftLen/2*sizeof(IPOINT);	
  /* Amount of memory required for  CalcUtilityFunction */
  size1 = MAX_PEAKS_PRELIM*sizeof(POINT) + 
    iMaxNoOfBreakPoints*sizeof(XPOINT) + MAX_PEAKS_PRELIM*sizeof(ARRAY_OF_XPOINTS);

  size = MAX(size,size1);	
  MALLOCATE(pstEstimator->pvScratchMemory,char,size,iMemErr);  
  if (iMemErr) return 1;  

	
  /* **************************************************** */
  /*        Allocate single window interpolated DFT       */
  /* **************************************************** */
  MALLOCATE(pstEstimator->pstSingleInterpDft,CMPLX,pstRom->iInterpDftLen+2*2*(DIRICHLET_KERNEL_SPAN-1),iMemErr);
  if (iMemErr) return 1;
  pstEstimator->pstSingleInterpDft += 2*(DIRICHLET_KERNEL_SPAN-1);

  /* **************************************************** */
  /*        Allocate double window interpolated DFT       */
  /* **************************************************** */
  MALLOCATE(pstEstimator->pstDoubleInterpDft,CMPLX,pstRom->iInterpDftLen+2*2*(DIRICHLET_KERNEL_SPAN-1),iMemErr);
  if (iMemErr) return 1;
  pstEstimator->pstDoubleInterpDft += 2*(DIRICHLET_KERNEL_SPAN-1);


  pstEstimator->iInterpDftLen = pstRom->iInterpDftLen;
	

  /* *********************** */
  /*  Allocate other buffers */
  /* *********************** */

  max_spec_peaks = pstRom->iInterpDftLen/2+1;
  MALLOCATE(pstEstimator->pstSpectralPeaks,POINT,max_spec_peaks,iMemErr);
  if (iMemErr) return 1;    
	    
  MALLOCATE(pstEstimator->pstPoints,POINT,(iMaxNoOfBreakPoints+1),iMemErr);	
  if (iMemErr) return 1;
	
  MALLOCATE(pstEstimator->pstPitchCand,PITCH,MAX_PRELIM_CANDS,iMemErr);
  if (iMemErr) return 1;
    		   
 
  /* ********** */
  /*   Reset    */
  /* ********** */

  pstEstimator->iStablePitchCount = 0;
  pstEstimator->stPrevPitch.fAmp  = 0.f;
  pstEstimator->stPrevPitch.fFreq = UNVOICED;
 
  pstEstimator->iDistFromStableTrack = 1000;
  pstEstimator->stStableTrackPitch.fAmp = 0.f;
  pstEstimator->stStableTrackPitch.fFreq = UNVOICED;
 

  memset(pstEstimator->pstDoubleInterpDft,0,sizeof(CMPLX)*pstEstimator->iInterpDftLen);

  pstEstimator->iFrameNo = 0;


  /* Update output */
  *pPitchEstimator = (void*)pstEstimator;

  
  return 0;
}


/*----------------------------------------------------------------------------
 * FUNCTION NAME: ResetPitchEstimator
 *
 * PURPOSE:       initializes a pitch estimator data structure
 *                containing RAM and history information used for pitch estimation. 
 *
 * INPUT:
 *   PitchEstimator         pointer to pitch estimator object created by InitPitchEstimator()
 *
 * OUTPUT
 *   PitchEstimator         pointer to pitch estimator object created by InitPitchEstimator()
 *
 * RETURN VALUE
 *   none
 *---------------------------------------------------------------------------*/
void ResetPitchEstimator(
						void                *PitchEstimator 
						)
{
  
  DSR_PITCH_ESTIMATOR *pstEstimator;

  pstEstimator = (DSR_PITCH_ESTIMATOR*)PitchEstimator;
 
  /* ********** */
  /*   Reset    */
  /* ********** */

  pstEstimator->iStablePitchCount = 0;
  pstEstimator->stPrevPitch.fAmp  = 0.f;
  pstEstimator->stPrevPitch.fFreq = UNVOICED;
 
  pstEstimator->iDistFromStableTrack = 1000;
  pstEstimator->stStableTrackPitch.fAmp = 0.f;
  pstEstimator->stStableTrackPitch.fFreq = UNVOICED;
 

  memset(pstEstimator->pstDoubleInterpDft,0,sizeof(CMPLX)*pstEstimator->iInterpDftLen);

  pstEstimator->iFrameNo = 0;

}



/*----------------------------------------------------------------------------
 * FUNCTION NAME:     EstimatePitch
 *
 * PURPOSE: Estimate pitch of current frame
 *
 * INPUT:
 *
 *   PitchRom               pointer to pitch ROM object created by InitPitchRom()
 *   PitchEstimator         pointer to pitch estimator object created by InitPitchEstimator()
 *   fSpecAverage           average of frame's spectrum
 *   pfProcSpeechForCorr    low-pass downsampled version of speech signal created
 *                          by pre_process()
 *   iPSFC_DownSampFactor   downsampling factor used by pre_process()
 *   Stft                   complex spectrum (Short-Time Fourier transform)
 *   pfPowerSpectrum        power spectrum
 *   LogEnergy              log energy 
 *   iIsSpeech              VAD flag
 *   iLowBandNoise          low band noise detection flag
 *
 * OUTPUT:
 *
 *   pOutPitchPeriod        pitch cycle period expressed in samples
 *                          corresponding to 8 kHz sampling rate,
 *                          or 0 indicating unvoiced frame
 * RETURN VALUE
 *   none
 *---------------------------------------------------------------------------*/

void EstimatePitch(
		 const void          *PitchRom,
		 void                *PitchEstimator,
		 const X_FLOAT32     fSpecAverage,
		 X_FLOAT32           *pfProcSpeechForCorr,
         X_INT16             iPSFC_DownSampFactor,
         const X_FLOAT32     *Stft,
         const X_FLOAT32     *pfPowerSpectrum,
         const X_FLOAT32     LogEnergy, 
         const X_INT16       iIsSpeech,
         const X_INT16       iLowBandNoise,
		 X_FLOAT32           *pOutPitchPeriod,
		 PCORR_DATA			 *pCorrData
		 )
{
  DSR_PITCH_ROM   *pstRom;
  DSR_PITCH_ESTIMATOR *pstEstimator;

  BOOL bSingleWinSpecPeaks_Prepared = FALSE;
 
  // Structures to hold the pitch candidates from each sub-range (analysis window)
  PITCH astPitchCand[N_OF_BEST_CANDS_SHORT+N_OF_BEST_CANDS_SINGLE+N_OF_BEST_CANDS_DOUBLE];
  PITCH *pstShortWinCand, *pstSingleWinCand, *pstDoubleWinCand;

  PITCH stFinalPitch;
			
  int i;

  float fStartFreq, fEndFreq, fShortStartFreq, fShortEndFreq,
    fSingleStartFreq, fSingleEndFreq,
    fDoubleStartFreq, fDoubleEndFreq;

	
  pstRom = (DSR_PITCH_ROM*)PitchRom;
  pstEstimator = (DSR_PITCH_ESTIMATOR*)PitchEstimator;
  
  pstEstimator->iFrameNo++;
		

  pstShortWinCand = astPitchCand;
  pstSingleWinCand = pstShortWinCand + N_OF_BEST_CANDS_SHORT;
  pstDoubleWinCand = pstSingleWinCand + N_OF_BEST_CANDS_SINGLE;

  for (i=0; i<N_OF_BEST_CANDS_SHORT; i++) {
    ClearPitch(&pstShortWinCand[i]);	
  }
  for (i=0; i<N_OF_BEST_CANDS_SINGLE; i++) {
    ClearPitch(&pstSingleWinCand[i]);
  }

  for (i=0; i<N_OF_BEST_CANDS_DOUBLE; i++) {
    ClearPitch(&pstDoubleWinCand[i]);
  }

	
 //
 // Interpolate input DFT of single window by a factor of two,
 // using approximated Dirichlet interpolation.
 // (this must be done even if the frame is below energy threshold,
 //  ,i.e. silience, since it may be needed to calculate a double-window DFT in the
 //  next frame)
 //

  DirichletInterpolation(
			 Stft, // Input complex DFT
			 fSpecAverage,
			 pstRom->iDftSize,
			 pstRom->i4KhzDftBin, // Last point to be interpolated
			 pstRom->afDirichletImag,
			 pstEstimator->pstSingleInterpDft // Output complex interpolated DFT
			 );
                                   
  // If the input has a very low energy declare it UNVOICED
  if(FALSE == iIsSpeech || TRUE==IsLowLevelInput(LogEnergy)) {        
    ClearPitch(&stFinalPitch);				
    Finalize(pstEstimator,
	     &stFinalPitch,
	     pOutPitchPeriod);
    return;
  }
	

  if ( pstEstimator->stStableTrackPitch.fFreq != UNVOICED) {
    //
    //    We're on a stable pitch track - adjust pitch frequency search range
    //
    fStartFreq = pstEstimator->stStableTrackPitch.fFreq*0.666f;
    fStartFreq = MAX(DOUBLE_WIN_START_FREQ, fStartFreq); 
    fEndFreq = pstEstimator->stStableTrackPitch.fFreq*2.2f;
    fEndFreq = MIN(SHORT_WIN_END_FREQ, fEndFreq);

    // Determine pitch frequency search range for double window
    // (low pitch frequency)
    if (fStartFreq < DOUBLE_WIN_END_FREQ) {
      // There is an overlap with DOUBLE window
      fDoubleStartFreq = fStartFreq;
      fDoubleEndFreq = MIN(fEndFreq,DOUBLE_WIN_END_FREQ);
    }
    else 
      fDoubleStartFreq = fDoubleEndFreq = -1.f;

    // Determine pitch frequency search range for single window
    // (middle pitch frequency)
    if (fStartFreq < SINGLE_WIN_END_FREQ &&
	fEndFreq > SINGLE_WIN_START_FREQ) {                
      // There is an overlap with SINGLE window
      fSingleStartFreq = MAX(fStartFreq,SINGLE_WIN_START_FREQ);
      fSingleEndFreq = MIN(fEndFreq,SINGLE_WIN_END_FREQ);
    }
    else
      fSingleStartFreq = fSingleEndFreq = -1.f;

    // Determine pitch frequency search range for short window
    // (high pitch frequency)
    if (fEndFreq > SHORT_WIN_START_FREQ) {
      // There is an overlap with SHORT window
      fShortStartFreq = MAX(fStartFreq,SHORT_WIN_START_FREQ);
      fShortEndFreq = fEndFreq;
    }
    else 
      fShortStartFreq = fShortEndFreq = -1.f;
         
  }
  else {
    //
    // We are not on a stable pitch track - use full search range
    //
    fStartFreq = SHORT_WIN_END_FREQ;
    fEndFreq = DOUBLE_WIN_START_FREQ;
    fShortStartFreq = SHORT_WIN_START_FREQ;
    fShortEndFreq = SHORT_WIN_END_FREQ;
    fSingleStartFreq = SINGLE_WIN_START_FREQ;
    fSingleEndFreq = SINGLE_WIN_END_FREQ;
    fDoubleStartFreq = DOUBLE_WIN_START_FREQ;
    fDoubleEndFreq = DOUBLE_WIN_END_FREQ;        
  }

  if (iLowBandNoise) {
        // Don't use spectral peaks located below pstRom->iHighPassCutOffDftPoint
    pstEstimator->iHighPassCutOffDftPoint = pstRom->iHighPassCutOffDftPoint;
  }
  else {
    // Use spectral peaks wherever they are
    pstEstimator->iHighPassCutOffDftPoint = 0;
  }

 /* ----------------------------------------------------------
  *
  *
  *   SHORT WINDOW - search for high pitch frequency
  *
  *
  * ---------------------------------------------------------- */
  if (fShortStartFreq < fShortEndFreq) {
    // Initialize complexity limitation mechanism
    pstEstimator->iCreatePieceWiseFunc_LoopCount = 0;
    pstEstimator->iCreatePieceWiseFunc_LoopLimit = 
      CREATE_PIECEWISE_FUNC_LOOP_LIM_SH;
						
    PrepareSpectralPeaks(pstRom,
			 pstEstimator,
			 pstEstimator->pstSingleInterpDft,
			 pfPowerSpectrum);		
    bSingleWinSpecPeaks_Prepared = TRUE;
		
        
    FindPitchCandidates(pstRom,
			pstEstimator,
			pCorrData,
			fShortStartFreq,
			fShortEndFreq,
			pfProcSpeechForCorr,
			iPSFC_DownSampFactor,
			N_OF_BEST_CANDS_SHORT,
			pstShortWinCand);

    SelectFinalPitch(astPitchCand,
		     N_OF_BEST_CANDS_SHORT,
		     &(pstEstimator->stPrevPitch),
		     pstEstimator->iStablePitchCount,
		     &(pstEstimator->stStableTrackPitch),
		     &stFinalPitch,
		     TRUE);

        
                			
    if (stFinalPitch.fFreq != UNVOICED) {    				
      Finalize(pstEstimator,
	       &stFinalPitch,
	       pOutPitchPeriod);
      return;
    }					                     		
		        						
			
  } // end of short window anlaysis


 /* ----------------------------------------------------------
  *
  *
  *   SINGLE WINDOW - search for middle pitch frequency
  *
  *
  * ---------------------------------------------------------- */
  if (fSingleStartFreq < fSingleEndFreq) {
    // Initialize complexity limitation mechanism
    pstEstimator->iCreatePieceWiseFunc_LoopCount = 0;
    pstEstimator->iCreatePieceWiseFunc_LoopLimit = 
      CREATE_PIECEWISE_FUNC_LOOP_LIM_SNG;

    // Single window analysis works on the same DFT as short window.
    // Spectral peaks can be already available
    if (!bSingleWinSpecPeaks_Prepared) {			
      PrepareSpectralPeaks(pstRom,
			   pstEstimator,
			   pstEstimator->pstSingleInterpDft,
			   pfPowerSpectrum);			
    }
		        
    FindPitchCandidates(pstRom,
			pstEstimator,
			pCorrData,
			fSingleStartFreq,
			fSingleEndFreq,
			pfProcSpeechForCorr,
			iPSFC_DownSampFactor,
			N_OF_BEST_CANDS_SINGLE,
			pstSingleWinCand);	
		
    SelectFinalPitch(astPitchCand,
		     N_OF_BEST_CANDS_SHORT+N_OF_BEST_CANDS_SINGLE,
		     &(pstEstimator->stPrevPitch),
		     pstEstimator->iStablePitchCount,
		     &(pstEstimator->stStableTrackPitch),
		     &stFinalPitch,
		     TRUE);
        

    if (stFinalPitch.fFreq != UNVOICED) {    				
      Finalize(pstEstimator,
	       &stFinalPitch,
	       pOutPitchPeriod);
      return;
    }
		        
		
  } // end of single window anlaysis


	
  /* ----------------------------------------------------------
   *
   *
   *   DOUBLEE WINDOW - search for low pitch frequency
   *
   *
   * ---------------------------------------------------------- */
  if (fDoubleStartFreq < fDoubleEndFreq) {
    // Initialize complexity limitation mechanism
    pstEstimator->iCreatePieceWiseFunc_LoopCount = 0;
    pstEstimator->iCreatePieceWiseFunc_LoopLimit = 
      CREATE_PIECEWISE_FUNC_LOOP_LIM_DBL;
	
    CalculateDoubleWindowDft(
	  pstEstimator->pstDoubleInterpDft, // On entry holds interpolated DFT of previous frame
	  pstEstimator->pstSingleInterpDft, // Holds interpolated DFT of current frame
	  pstEstimator->pstDoubleInterpDft, // On exit holds double-window DFT
	  pstRom->iInterpDftLen,
	  pstRom->pstWindowShiftTable);
		
    PrepareSpectralPeaks(pstRom,
			 pstEstimator,
			 pstEstimator->pstDoubleInterpDft,
			 NULL);		
    

    FindPitchCandidates(pstRom,
			pstEstimator,
			pCorrData,
			fDoubleStartFreq,
			fDoubleEndFreq,
			pfProcSpeechForCorr,
			iPSFC_DownSampFactor,
			N_OF_BEST_CANDS_DOUBLE,
			pstDoubleWinCand);
						
				
  } // end of double window anlaysis


  /* -----------------------------------------------------------------------
     Select final pitch from the full candidate list
     ----------------------------------------------------------------------- */

					
  SelectFinalPitch(astPitchCand,
		   N_OF_BEST_CANDS,
		   &(pstEstimator->stPrevPitch),
		   pstEstimator->iStablePitchCount,
		   &(pstEstimator->stStableTrackPitch),
		   &stFinalPitch,
		   FALSE);
    	

  Finalize(pstEstimator,
	   &stFinalPitch,
	   pOutPitchPeriod);
  

}





/*----------------------------------------------------------------------------
 * FUNCTION NAME:  DeallocatePitchEstimator
 *
 * PURPOSE:       Free memory allocated for pitch estimator structure
 *
 * INPUT:
 *   PitchRom          Object created by InitPitchEstimator()
 *
 * OUTPUT
 *   none
 *
 * RETURN VALUE
 *   none
 *---------------------------------------------------------------------------*/
void DeallocatePitchEstimator(void *PitchEstimator)
{
  DSR_PITCH_ESTIMATOR *pstEstimator = (DSR_PITCH_ESTIMATOR*)PitchEstimator;
	

  pstEstimator->pstSingleInterpDft -= 2*(DIRICHLET_KERNEL_SPAN-1);
  MDEALLOCATE(pstEstimator->pstSingleInterpDft);	
	
  pstEstimator->pstDoubleInterpDft -= 2*(DIRICHLET_KERNEL_SPAN-1);
  MDEALLOCATE(pstEstimator->pstDoubleInterpDft);

  MDEALLOCATE(pstEstimator->pstPoints);
  MDEALLOCATE(pstEstimator->pstPitchCand);
  MDEALLOCATE(pstEstimator->pvScratchMemory);  
  MDEALLOCATE(pstEstimator->pstSpectralPeaks);

  MDEALLOCATE(pstEstimator);

}

/*----------------------------------------------------------------------------
 * FUNCTION NAME:  DeallocatePitchRom
 *
 * PURPOSE:       Free memory allocated for pitch ROM structure
 *
 * INPUT:
 *   PitchRom          Object created by InitPitchRom()
 *
 * OUTPUT
 *   none
 *
 * RETURN VALUE
 *   none
 *---------------------------------------------------------------------------*/
void DeallocatePitchRom(void *PitchRom)
{
  DSR_PITCH_ROM *pstRom = (DSR_PITCH_ROM*)PitchRom;

  MDEALLOCATE(pstRom->pstWindowShiftTable);
  MDEALLOCATE(pstRom);

}




         




