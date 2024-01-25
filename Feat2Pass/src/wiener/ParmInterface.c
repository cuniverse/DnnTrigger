/*===============================================================================
 *      ETSI ES 202 212   Distributed Speech Recognition
 *      Extended Advanced Front-End Feature Extraction Algorithm & Compression Algorithm
 *      Speech Reconstruction Algorithm.
 *      C-language software implementation                                      
 *      Version 1.1.1   October, 2003                                            
 *===============================================================================*/
/*-------------------------------------------------------------------------------
 *
 * FILE NAME: ParmInterface.c
 * PURPOSE: Processing one input frame in DoAdvProcess():
 *          DoNoiseSup(), DoWaveProc(), DoCompCeps(), DoPostProc(),
 *          and DoVADProc() are called.
 *
 *-------------------------------------------------------------------------------*/
/*-----------------
 * File Inclusions
 *-----------------*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pffft.h"

#include "wiener/ParmInterface.h"
#include "wiener/NoiseSup.h"
#include "wiener/pitchInterface.h"
#include "basic_op/fixpoint.h"

/*--------------------------------------------
 * The followings include the module-specific 
 * and exported methods (or functions).
 *--------------------------------------------*/
#include "wiener/NoiseSupExports.h"
#include "wiener/MelProcExports.h"

/*----------------------------------------------------------------------------
 * FUNCTION NAME: AdvProcessAlloc
 *
 * PURPOSE:       Memory allocation of front end parameter structure     
 *
 * INPUT:
 *   SamplingFrequency     
 *
 * OUTPUT:
 *   pFEParX      Pointer to front end parameter structure
 *
 * RETURN VALUE:
 *   pFEParX      Pointer to front end parameter structure
 *   
 *---------------------------------------------------------------------------*/
FEParamsX* AdvProcessAlloc (int SamplingFrequency)
{
	int rc = 0;
  FEParamsX *pFEParX = (FEParamsX *) calloc (1, sizeof (FEParamsX));

  pFEParX->DoNoiseSupAlloc = DoNoiseSupAlloc;
  pFEParX->DoNoiseSupInit = DoNoiseSupInit;
  pFEParX->DoNoiseSup = DoNoiseSup;
  pFEParX->DoNoiseSupDelete = DoNoiseSupDelete;

  /*----------------------------------------------
   * allocate module-specific expansion structure
   *----------------------------------------------*/
 if (pFEParX->DoNoiseSupAlloc != NULL) pFEParX->NSX = pFEParX->DoNoiseSupAlloc ();

  pFEParX->SamplingFrequency = SamplingFrequency;
  
  if (pFEParX->SamplingFrequency == SAMPLING_FREQ_1 * 1000)
  {
	  pFEParX->FrameLength = FRAME_LENGTH;
	  pFEParX->FrameShift = FRAME_SHIFT;
	  pFEParX->FFTLength = FFT_LENGTH;
	  pFEParX->StartingFrequency = STARTING_FREQ;
  }
  else if (pFEParX->SamplingFrequency == SAMPLING_FREQ_3 * 1000)
  {
	  pFEParX->FrameLength = FRAME_LENGTH;
	  pFEParX->FrameShift = FRAME_SHIFT;
	  pFEParX->FFTLength = FFT_LENGTH;
	  pFEParX->StartingFrequency = STARTING_FREQ;
  }
  else
  {
	  fprintf (stderr, "ERROR:   Invalid sampling frequency '%d'!\r\n",
		  pFEParX->SamplingFrequency);
	  exit(0);
  }

  /* For pitch and class extraction */
  
  pFEParX->pfInpSpeech = (X_FLOAT32*)calloc(sizeof(X_FLOAT32), FRAME_LENGTH);
  
  pFEParX->pfUBSpeech = (X_FLOAT32*)calloc(sizeof(X_FLOAT32), FRAME_LENGTH);
  
  pFEParX->pfProcSpeech = (X_FLOAT32*)calloc(sizeof(X_FLOAT32), (FRAME_LENGTH+HISTORY_LENGTH));

  pFEParX->pfDownSampledProcSpeech = (X_FLOAT32*)
	  calloc(sizeof(X_FLOAT32), (FRAME_LENGTH+HISTORY_LENGTH)/DOWN_SAMP_FACTOR+1);

  pFEParX->CurFrame = (X_FLOAT32*)calloc (1, sizeof (pFEParX->CurFrame[0]) * pFEParX->FrameShift);

  pFEParX->denoisedBuf = 
	  BufInAlloc (pFEParX->FrameLength +                         // at least FrameLength
	  (pFEParX->FrameLength % pFEParX->FrameShift) + // integer number times FrameShift
	  1);                                            // for pre-emphasis
  
  pFEParX->FirstWindow = CMelFBAlloc ();
  InitFFTWindows (pFEParX->FirstWindow, pFEParX->StartingFrequency,
				  (float)pFEParX->SamplingFrequency, FFT_LENGTH, NUM_CHANNELS);
  ComputeTriangle (pFEParX->FirstWindow);  
  
  rc = InitPitchRom(&(pFEParX->pPitchRom));
  if (rc != 0)
  {
	  fprintf(stderr,"ERROR: Can't initialize PITCH ROM, RC = %d\r\n",rc);
	  exit(0) ;
  }
  
  rc = InitPitchEstimator(pFEParX->pPitchRom,&(pFEParX->pPitchEstimator));
  if ( rc!= 0)
  {
	  fprintf (stderr, "ERROR: can't initialize PITCH ESTIMATOR, RC = %d\r\n",rc);
	  exit(0) ;
  }
  
  return pFEParX;
}

/*----------------------------------------------------------------------------
 * FUNCTION NAME: AdvProcessInit
 *
 * PURPOSE:       Initialization of front end parameter structure
 *
 * INPUT:
 *   pFEParX      Pointer to front end parameter structure
 *
 * OUTPUT:
 *   pFEParX      Pointer to front end parameter structure
 *
 * RETURN VALUE:
 *   
 *---------------------------------------------------------------------------*/
void AdvProcessInit (FEParamsX *pFEParX)
{

  if (pFEParX->DoNoiseSupInit != NULL) pFEParX->DoNoiseSupInit (pFEParX);

  pFEParX->FrameCounter = 0;
  pFEParX->NonZeroFrameOnset = 0;
  pFEParX->ZeroFrameCounter = 0;
  pFEParX->NbSamplesToRead = pFEParX->FrameShift;

  ResetPitchEstimator( pFEParX->pPitchEstimator );

  memset(&pFEParX->pcorrData, 0, sizeof(pFEParX->pcorrData));
  pFEParX->pcorrData.iOldFrameNo = -1;
  pFEParX->pcorrData.iOldPitchPeriod = -1;

  memset(pFEParX->GSM_HPF_A_Buf, 0, sizeof(pFEParX->GSM_HPF_A_Buf));
  memset(pFEParX->GSM_HPF_B_Buf, 0, sizeof(pFEParX->GSM_HPF_B_Buf));
  memset(pFEParX->avgNoiseSpec, 0, sizeof(pFEParX->avgNoiseSpec));
  pFEParX->avgNoiseLogE = 0;
  
  pFEParX->fft_setup = pffft_new_setup(NS_FFT_LENGTH, PFFFT_REAL);
}


/*----------------------------------------------------------------------------
 * FUNCTION NAME: Whitening
 *
 * PURPOSE:       apply whitening filter
 *              
 * INPUT:
 *   pFEParX      Pointer to front end parameter structure
 *	 CurrentFrame Current Frame to process 
 *
 * OUTPUT:
 *	 CurrentFrame whitened current frame
 *
 * RETURN VALUE:
 *   none
 *
 *---------------------------------------------------------------------------*/
static void
Whitening(FEParamsX *pFEParX, X_FLOAT32 *CurrentFrame, int dithering_factor)
{
	X_FLOAT32 *pSample = 0;
	X_FLOAT32 *pLastSample = 0;
	X_FLOAT32 valRand = 0;
	int rand_width = 0;
	int rand_swing = 0;

	rand_width = dithering_factor;
	rand_swing = (dithering_factor >> 1);

	// add a randomly generated vector
	pSample     = CurrentFrame;
	pLastSample = CurrentFrame + pFEParX->NbSamplesToRead;
	while (pSample < pLastSample) {
		*pSample += (X_FLOAT32)(rand() % rand_width) - (X_FLOAT32)rand_swing; pSample++;
		*pSample += (X_FLOAT32)(rand() % rand_width) - (X_FLOAT32)rand_swing; pSample++;
		*pSample += (X_FLOAT32)(rand() % rand_width) - (X_FLOAT32)rand_swing; pSample++;
		*pSample += (X_FLOAT32)(rand() % rand_width) - (X_FLOAT32)rand_swing; pSample++;
	}

}

/*
static void
Whitening(FEParamsX *pFEParX, FILE_TYPE *CurrentFrame, int dithering_factor)
{
	FILE_TYPE *pSample = 0;
	FILE_TYPE *pLastSample = 0;
	int valRand = 0;
	int rand_width = 0;
	int rand_swing = 0;

	rand_width = dithering_factor;
	rand_swing = (dithering_factor >> 1);

	// add a randomly generated vector
	pSample     = CurrentFrame;
	pLastSample = CurrentFrame + pFEParX->NbSamplesToRead;
	while (pSample < pLastSample) {
		valRand = *pSample + (rand() % rand_width) - rand_swing;
		if ( valRand > 32767 ) *pSample = 32767;
		else if ( valRand < -32768 ) *pSample = -32768;
		else {
			*pSample = (FILE_TYPE)valRand;
		}
		pSample++;
		valRand = *pSample + (rand() % rand_width) - rand_swing;
		if ( valRand > 32767 ) *pSample = 32767;
		else if ( valRand < -32768 ) *pSample = -32768;
		else {
			*pSample = (FILE_TYPE)valRand;
		}
		pSample++;
		valRand = *pSample + (rand() % rand_width) - rand_swing;
		if ( valRand > 32767 ) *pSample = 32767;
		else if ( valRand < -32768 ) *pSample = -32768;
		else {
			*pSample = (FILE_TYPE)valRand;
		}
		pSample++;
		valRand = *pSample + (rand() % rand_width) - rand_swing;
		if ( valRand > 32767 ) *pSample = 32767;
		else if ( valRand < -32768 ) *pSample = -32768;
		else {
			*pSample = (FILE_TYPE)valRand;
		}
		pSample++;
	}

}
*/

/*----------------------------------------------------------------------------
 * FUNCTION NAME: GSM_HPF
 *
 * PURPOSE:       apply GSM HPF to reduce low-frequency noise components
 *              
 * INPUT:
 *   pFEParX      Pointer to front end parameter structure
 *	 CurrentFrame Current Frame to process 
 *
 * OUTPUT:
 *   pFEParX      Pointer to front end parameter structure
 *	 CurFrame	  float-type HPF current frame
 *
 * RETURN VALUE:
 *   none
 *
 *---------------------------------------------------------------------------*/
static void GSM_HPF (FEParamsX *pFEParX, FILE_TYPE *CurrentFrame, X_FLOAT32 *CurFrame )
{
	FILE_TYPE *pSample = 0;
	FILE_TYPE *pLastSample = 0;
	X_FLOAT32 *pOutput = 0;
	X_FLOAT32 x;
	
	float anB[3]={0.9273f,   -1.8545f,     0.9273f};
	float anA[3]={1.0f,      -1.9059f,     0.9114f};
	
	float sumA  = 0;		// Q15.32
	float sumB  = 0;		// Q15.32
	
	pSample     = CurrentFrame;
	pLastSample = pSample + pFEParX->NbSamplesToRead;
	pOutput     = CurFrame;
	while (pSample < pLastSample) {
		x = (float)(*pSample);

		sumB  = x * anB[0];
		sumB += pFEParX->GSM_HPF_B_Buf[1] * anB[1];
		sumB += pFEParX->GSM_HPF_B_Buf[2] * anB[2];

		sumA  = pFEParX->GSM_HPF_A_Buf[1] * anA[1];
		sumA += pFEParX->GSM_HPF_A_Buf[2] * anA[2];

		*pOutput = sumB - sumA;
			
		pFEParX->GSM_HPF_B_Buf[2] = pFEParX->GSM_HPF_B_Buf[1];
		pFEParX->GSM_HPF_B_Buf[1] = x;
			
		pFEParX->GSM_HPF_A_Buf[2] = pFEParX->GSM_HPF_A_Buf[1];
		pFEParX->GSM_HPF_A_Buf[1] = *pOutput;
			
		pSample++; pOutput++;
	}

  return;
}

/*----------------------------------------------------------------------------
 * FUNCTION NAME: DoAdvProcess
 *
 * PURPOSE:       Processing for a frame 
 *
 * INPUT:
 *   CurrentFrame Current Frame to process     
 *   pFEParX      Pointer to front end parameter structure
 * OUTPUT:
 *   FeatureBuffer Buffer of features
 *
 * RETURN VALUE:
 *   XTRUE        If a feature frame is output
 *   FALSE        If no frame is output (due to delay)
 *
 *---------------------------------------------------------------------------*/
ETSI_BOOL DoAdvProcess (FILE_TYPE *CurrentFrame, 
					    X_INT16 *OutputBuffer,
					    FEParamsX *pFEParX) 
{
  int i;
  int FrameShift;
  int dithering_factor = 256;
  X_FLOAT32 *CurFrame;
  X_FLOAT32 *ptDenoised;

  float FrameCheck;

  long NonZeroFrameOnset;
  long ZeroFrameCounter;

  ZeroFrameCounter = pFEParX->ZeroFrameCounter;
  NonZeroFrameOnset = pFEParX->NonZeroFrameOnset;
  FrameShift  = pFEParX->FrameShift;
  
  CurFrame = pFEParX->CurFrame ;

  /*-------------------------------------------------
   * apply GSM HPF to reduce low-freq. noise components
   *-------------------------------------------------*/

  GSM_HPF( pFEParX, CurrentFrame, CurFrame );

  /*-------------------------------------------------
   * apply whitening filter
   *-------------------------------------------------*/

//  Whitening( pFEParX, CurFrame, dithering_factor );

  /*-----------------------------------------------------------------
   * get the pointer where to input new Frameshift denoised samples
   * shift left data in denoisedBuf by FrameShift
   * and returns pointer to the new available part of
   * denoisedBuf => (denoisedBuf->size - FrameShift + 1)
   *-----------------------------------------------------------------*/
  ptDenoised = BufInShiftToPut (pFEParX->denoisedBuf, pFEParX->FrameShift);

  /*-------------------------------------------------
   * convert input samples from X_INT16 to X_FLOAT32
   *-------------------------------------------------*/
  FrameCheck = 0.0;
  for (i=0 ; i<FrameShift; i++)
	{
//	  CurFrame[i] = (X_FLOAT32)(CurrentFrame[i]);
        FrameCheck += CurFrame[i] * CurFrame[i];
	}

  if (((int)FrameCheck != 0) || (NonZeroFrameOnset != 0))
	{ 
	  pFEParX->NonZeroFrameOnset = 1;

	  if (pFEParX->DoNoiseSup (CurFrame, ptDenoised, pFEParX))
		{

			{
				int d = 0, out = 0;
				
				for ( d = 0; d < pFEParX->FrameShift; d++) {
					out = FLOAT2FIX32_ANY( ptDenoised[d], 0 );
					if ( out > MAX_INT16 ) OutputBuffer[d] = MAX_INT16;
					else if ( out < MIN_INT16 ) OutputBuffer[d] = MIN_INT16;
					else OutputBuffer[d] = (X_INT16)out;
				}
			}

			return TRUE;
		}
	  else
		return FALSE;
	}
  else
	{
	  pFEParX->ZeroFrameCounter++;

	  pFEParX->FrameClass = 0;

	  return FALSE;
	}
}

/*----------------------------------------------------------------------------
 * FUNCTION NAME: AdvProcessDelete
 *
 * PURPOSE:       Memory free of front end parameter structure     
 *
 * INPUT:
 *   ppFEParX     Pointer to pointer to front end parameter structure
 *
 * OUTPUT:
 *
 * RETURN VALUE:
 *   
 *---------------------------------------------------------------------------*/
void AdvProcessDelete (FEParamsX **ppFEParX)
{    
  if ((*ppFEParX)->DoNoiseSupDelete != NULL) (*ppFEParX)->DoNoiseSupDelete ((*ppFEParX)->NSX);

  DeallocatePitchRom((*ppFEParX)->pPitchRom);
  DeallocatePitchEstimator((*ppFEParX)->pPitchEstimator);
  free((*ppFEParX)->pfInpSpeech);
  free((*ppFEParX)->pfUBSpeech);
  free((*ppFEParX)->pfProcSpeech);
  free((*ppFEParX)->pfDownSampledProcSpeech);

  free((*ppFEParX)->CurFrame);
  BufInFree ((*ppFEParX)->denoisedBuf);
  ReleaseMelFBwindows ((*ppFEParX)->FirstWindow);
  free((*ppFEParX)->FirstWindow);

  pffft_destroy_setup((*ppFEParX)->fft_setup);

  free (*ppFEParX);
  *ppFEParX = NULL;
}
