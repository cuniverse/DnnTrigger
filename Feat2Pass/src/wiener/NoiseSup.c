/*===============================================================================
 *      ETSI ES 202 212   Distributed Speech Recognition                         
 *      Extended Advanced Front-End Feature Extraction Algorithm & Compression Algorithm
 *      C-language software implementation                                       
 *      Version 1.1.1   October, 2003                                            
 *===============================================================================*/
/*-------------------------------------------------------------------------------
 *
 * FILE NAME: NoiseSup.c
 * PURPOSE: 1) Apply 2-stage Wiener filter on the input frame.
 *          2) Apply DC offset removal on the output of 2-stage 
 *             Wiener filter.
 *          3) Calculate parameters for the frame dropping VAD (see 
 *             SpeechQSpec(), SpeechQMel(), SpeechQVar()).
 *
 *-------------------------------------------------------------------------------*/
/*-----------------
 * File Inclusions
 *-----------------*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "pffft.h"

#include "wiener/ParmInterface.h"
#include "wiener/NoiseSup.h"
#include "wiener/NoiseSupExports.h"
#include "wiener/MelProcExports.h"
#include "wiener/pitchInterface.h"
#include "wiener/classifyFrame.h"
#include "wiener/dsrAfeVad.h"
#include "wiener/preProc.h"

#define MIN_MEL_MEAN	(0.07f)	//(0.10f)	//(0.12f)
#define MAX_MEL_MEAN	(0.15f)	//(0.22f)
#define SPEECH_MEL_TH	(0.7f)
#define MIN_SPEC_H		(0.0001f)
#define SPEECH_SPEC_H	(0.1f)
#define VOICED_SPEC_H	(0.2f)
#define INIT_NS_FRAME	10	// 15

#define ALN_NS 64
#if defined(_MSC_VER)
#define ALIGNED_(x) __declspec(align(x))
#else
#if defined(__GNUC__)
#define ALIGNED_(x) __attribute__ ((aligned(x)))
#endif
#endif

/*------------------------
 * Definitions and Macros
 *------------------------*/
#define etsi_max(a,b) (((a)>(b))?(a):(b))
#define etsi_min(a,b) (((a)>(b))?(b):(a))

typedef struct vad_data_ns VAD_DATA_NS;
typedef struct vad_data_fd VAD_DATA_FD;
typedef struct vad_data_ca VAD_DATA_CA;
typedef struct ns_var      NS_VAR;
typedef struct dc_filter   DC_FILTER;
typedef struct gain_fact   GAIN_FACT;
typedef struct buffers     BUFFERS;
typedef struct spectrum    SPECTRUM;

struct dc_filter
{
  X_FLOAT32 lastSampleIn;                  // last input sample of DC offset compensation
  X_FLOAT32 lastDCOut;                     // last output sample of DC offset compensation
} ;

struct vad_data_ns
{
  X_INT32   nbFrame [2];                   // frame number
  X_INT16   flagVAD;                       // VAD flag (1 == SPEECH, 0 == NON SPEECH)
  X_INT16   hangOver;                      // hangover
  X_INT16   nbSpeechFrames;                // nb speech frames (used to set hangover)
  X_FLOAT32 meanEn;                        // mean energy
};

struct vad_data_ca
{
  X_INT16   flagVAD;                       // VAD flag (1 == SPEECH, 0 == NON SPEECH)
  X_INT16   hangOver;                      // hangover
  X_INT16   nbSpeechFrames;                // nb speech frames (used to set hangover)
  X_FLOAT32 meanCA;                        // mean energy
};

struct vad_data_fd
{
  X_FLOAT32 MelMean;			           //
  X_FLOAT32 VarMean;			           //
  X_FLOAT32 AccTest;			           //
  X_FLOAT32 AccTest2;			           //
  X_FLOAT32 SpecMean;			           //
  X_FLOAT32 MelValues[2];		           //
  X_FLOAT32 SpecValues;		               //
  X_FLOAT32 SpeechInVADQ;		           //
  X_FLOAT32 SpeechInVADQ2;		           //
};

struct gain_fact
{
  X_FLOAT32 denEn1 [3];                    // previous denoised frames energies
  X_FLOAT32 lowSNRtrack;                   // low SNR track
  X_FLOAT32 alfaGF;                        // gain factor applied in 2nd stage
};

struct buffers
{
  X_INT32 nbFramesInFirstStage;            // nb frames in first stage
  X_INT32 nbFramesInSecondStage;           // nb frames in second stage
  X_INT32 nbFramesOutSecondStage;          // nb frames out of second stage

  X_FLOAT32 FirstStageInFloatBuffer  [NS_BUFFER_SIZE]; // first stage buffer
  X_FLOAT32 SecondStageInFloatBuffer [NS_BUFFER_SIZE]; // second stage buffer
};

struct spectrum
{
  X_FLOAT32 nSigSE1 [NS_SPEC_ORDER];       // 1st stage noisy signal spectrum estimation
  X_FLOAT32 nSigSE2 [NS_SPEC_ORDER];       // 2nd stage noisy signal spectrum estimation
  X_FLOAT32 noiseSE1 [NS_SPEC_ORDER];      // 1st stage noise spectrum estimation
  X_FLOAT32 noiseSE2 [NS_SPEC_ORDER];      // 2nd stage noise spectrum estimation
  X_FLOAT32 denSigSE1 [NS_SPEC_ORDER];     // 1st stage denoised signal spectrum estimation
  X_FLOAT32 denSigSE2 [NS_SPEC_ORDER];     // 2nd stage denoised signal spectrum estimation

  X_INT16   indexBuffer1;                  // where to enter new PSD, alternatively 0 and 1
  X_INT16   indexBuffer2;                  // where to enter new PSD, alternatively 0 and 1
  X_FLOAT32 PSDMeanBuffer1 [NS_SPEC_ORDER][NS_PSD_MEAN_ORDER]; // 1st stage PSD Mean buffer
  X_FLOAT32 PSDMeanBuffer2 [NS_SPEC_ORDER][NS_PSD_MEAN_ORDER]; // 2nd stage PSD Mean buffer
};

struct ns_var
{
  X_INT16     SampFreq;                    // sampling frequency
  BUFFERS     buffers;                     // signal buffers
  DC_FILTER   prevSamples;                 // previous samples for DC offset removal
  GAIN_FACT   gainFact;                    // gain factorization variables
  VAD_DATA_NS vadNS;                       // VAD for noise suppression data
  VAD_DATA_FD vadFD;                       // VAD for frame dropping data
  VAD_DATA_CA vadCA;                       // VAD for frame dropping data
  SPECTRUM    spectrum;                    // spectrum data
};

typedef struct ns_tmp
{
  MelFB_Window *FirstWindow;               // chained list for Mel filtering
  X_FLOAT32 **melIDCTbasis;                // mel-frequency inverse DCT basis
  X_FLOAT32 IRWindow [NS_FILTER_LENGTH];   // filter impulse response window
  X_FLOAT32 sigWindow [NS_FRAME_LENGTH];   // signal window
  X_FLOAT32 ALIGNED_(ALN_NS) tmpMem[NS_SCRATCH_MEM_SIZE];  // scratch memory
} NS_TMP;

struct NoiseSupStructX
{
  NS_VAR    nsVar;                         // non sharable data
  NS_TMP    nsTmp;                         // sharable data
};

struct CompCepsStructX
{
  X_INT16   SamplingFrequency;               //
  X_FLOAT32 StartingFrequency;               //
  X_FLOAT32 HammingWindow [NS_FRAME_LENGTH]; //
  X_FLOAT32 *pDCTMatrix;                     //
  MelFB_Window *FirstWindow;                 //
};

/*------------
 * Prototypes
 *------------*/
static void DCOffsetFil (X_FLOAT32 *Data, DC_FILTER *prevSamples, X_INT16 DataLength);
static void DoSigWindowing (X_FLOAT32 *Data, X_FLOAT32 *window, X_INT16 frameLength, X_INT16 FFTLength);
static void FFTtoPSD (const X_FLOAT32 *FFTIn, X_FLOAT32 *PSDOut, X_INT16 FFTLength);
static void PSDMean (X_INT16 *indexBuffer, X_FLOAT32 *PSDIn, X_FLOAT32 *PSDOut, X_FLOAT32 *PSDbuffer);
static void ApplyWF (const X_FLOAT32 *data, const X_FLOAT32 *predata, const X_FLOAT32 *filter, X_FLOAT32 *result, const X_INT16 frameShift, const X_INT16 melOrder);
static void VAD (X_INT16 fstage, VAD_DATA_NS *vadNS, const X_FLOAT32 *newShiftFrame);
static void FilterCalc (X_INT16 fstage, NoiseSupStructX *NSX, X_FLOAT32 *PSDMeaned, X_FLOAT32 *W);
static void DoGainFact (X_INT16 fstage, NoiseSupStructX *NSX, X_FLOAT32 *W);
static void DoFilterWindowing (X_FLOAT32 *filterIRIn, X_FLOAT32 *hanningWindow, X_FLOAT32 *filterIROut);
static X_INT16 SpeechQSpec (FEParamsX *This);
static X_INT16 SpeechQMel (FEParamsX *This);
static X_INT16 SpeechQVar (FEParamsX *This, X_FLOAT32 *W);

/*-----------
 * Functions
 *-----------*/
/*----------------------------------------------------------------------------
 * FUNCTION NAME: DCOffsetFil
 *
 * PURPOSE:       DC offset compensation
 *              
 *
 * INPUT:
 *   *Data        Input frame to filter
 *   *prevSamples Previous samples used in DC offset filtering
 *   DataLength   Number of samples to filter
 *
 * OUTPUT:
 *   *Data        Output filtered frame
 *
 * RETURN VALUE:
 *   none
 *
 *---------------------------------------------------------------------------*/
static void DCOffsetFil (X_FLOAT32 *Data, DC_FILTER *prevSamples, X_INT16 DataLength)
{
  X_INT16 i;
  X_FLOAT32 aux;
  X_FLOAT32 *Prev_x = &(prevSamples->lastSampleIn);
  X_FLOAT32 *Prev_y = &(prevSamples->lastDCOut);

  // y[n] = x[n] - x[n-1] + 0.9990234375 * y[n-1]

  for (i=0 ; i<DataLength ; i++)
	{
	  aux = Data[i];
	  Data[i] = Data[i] - *Prev_x + 0.9990234375 * *Prev_y;
	  *Prev_x = aux;
	  *Prev_y = Data[i];
	}
}

/*----------------------------------------------------------------------------
 * FUNCTION NAME: DoSigWindowing
 *
 * PURPOSE:       Signal windowing
 *              
 * INPUT:
 *   *Data        Input frame
 *   *window      Window
 *   frameLength  Frame Length
 *   FFTLength    FFT Length
 *
 * OUTPUT:
 *   *Data        Output frame
 *
 * RETURN VALUE:
 *   none
 *
 *---------------------------------------------------------------------------*/
static void DoSigWindowing (X_FLOAT32 *Data, X_FLOAT32 *window, X_INT16 frameLength, X_INT16 FFTLength)
{
  X_INT16 i;

  // windowing
  for (i=0 ; i<frameLength ; i++)
    Data [i] = Data[i] * window [i];

  // zero padding
  for (i=frameLength ; i<FFTLength ; i++)
    Data [i] = 0.0;

  return;
}

/*----------------------------------------------------------------------------
 * FUNCTION NAME: FFTtoPSD
 *
 * PURPOSE:       Compute PSD from FFT
 *              
 * INPUT:
 *   *FFTIn       Input Spectrum 
 *   FFTLenth     FFT length
 *
 * OUTPUT:
 *   *PSDOut      Output PSD
 *
 * RETURN VALUE:
 *   none
 *
 *---------------------------------------------------------------------------*/
static void FFTtoPSD (const X_FLOAT32 *FFTIn, X_FLOAT32 *PSDOut, X_INT16 FFTLength)
{
	for (int i = 1; i < NS_SPEC_ORDER-1; i++)
	{
		int re0 = 4 * i;
		int im0 = re0 + 1;
		int re1 = re0 + 2;
		int im1 = re0 + 3;
		X_FLOAT32 spec0 = FFTIn[re0] * FFTIn[re0] + FFTIn[im0] * FFTIn[im0];
		X_FLOAT32 spec1 = FFTIn[re1] * FFTIn[re1] + FFTIn[im1] * FFTIn[im1];
		PSDOut[i] = (spec0 + spec1) / 2.f;
	}
	PSDOut[0] = ((FFTIn[0]*FFTIn[0]) + (FFTIn[2]*FFTIn[2] + FFTIn[3]*FFTIn[3])) / 2.f;
	PSDOut[NS_SPEC_ORDER-1] = FFTIn[1] * FFTIn[1];

  return;
}

/*----------------------------------------------------------------------------
 * FUNCTION NAME: PSDMean
 *
 * PURPOSE:       Smoothing of the PSD
 *              
 * INPUT:         
 *   *indexBuffer Pointer to buffer index
 *   *PSDIn       Pointer to input PSD
 *   *PSDBuffer   Pointer to PSD buffer
 *
 * OUTPUT:
 *   PSDOut       Output PSD
 *
 * RETURN VALUE:
 *   none
 *
 *---------------------------------------------------------------------------*/
static void PSDMean (X_INT16 *indexBuffer, X_FLOAT32 *PSDIn, X_FLOAT32 *PSDOut, X_FLOAT32 *PSDBuffer)
{
  X_INT16 i, index;

  *indexBuffer = 1 - *indexBuffer;

  for (i=0 ; i<NS_SPEC_ORDER ; i++)
	{
	  index = i * NS_PSD_MEAN_ORDER + *indexBuffer;
	  PSDBuffer[index] = PSDIn[i];

	  index += 1 - 2 * *indexBuffer; 
	  PSDOut[i] = (PSDBuffer[index] + PSDIn[i]) / NS_PSD_MEAN_ORDER;
	}
}

/*----------------------------------------------------------------------------
 * FUNCTION NAME: ApplyWF
 *
 * PURPOSE:       Convolving input data with denoised filter impulse response
 *              
 * INPUT:
 *   *data        Pointer to input noisy signal
 *   *predata     Pointer to input noisy signal
 *   *filter      Pointer to denoised filter impulse response
 *   frameShift   Frame shift  
 *   melOrder     Impulse response length
 *
 * OUTPUT:
 *   result       Output denoised data 
 *
 * RETURN VALUE:
 *   none
 *
 *---------------------------------------------------------------------------*/
static void ApplyWF(const X_FLOAT32 *data, const X_FLOAT32 *predata, const X_FLOAT32 *filter, X_FLOAT32 *result, const X_INT16 frameShift, const X_INT16 melOrder)
{
	for (int i = 0; i<frameShift; i++)
	{
		int j_max = i<=melOrder ? i : melOrder;
		float sum = 0.f;

		for (int j = -melOrder; j <= j_max; j++)
			sum += (filter[j + melOrder] * data[i-j]);

		for (int j = i+1; j <= melOrder; j++)
			sum += (filter[j + melOrder] * predata[frameShift - j + i]);

		result[i] = sum;
	}
}

/*----------------------------------------------------------------------------
 * FUNCTION NAME: VAD
 *
 * PURPOSE:       Voice Ativity Detection
 *              
 * INPUT:
 *   fstage       Stage of two stage noise suppression
 *   *vadNS       Pointer to VAD structure
 *   *newShiftFrame Pointer to new input frame    
 *
 * OUTPUT:        VAD for noise suppression structure updated
 *
 *
 * RETURN VALUE:
 *   none
 *
 *---------------------------------------------------------------------------*/
static void VAD (X_INT16 fstage, VAD_DATA_NS *vadNS, const X_FLOAT32 *newShiftFrame)
{
  X_INT16 i;
  X_INT16 flagVAD;
  X_INT16 hangOver;
  X_INT16 nbSpeechFrames;
  X_INT32 nbFrame;
  X_FLOAT32 meanEn;
  X_FLOAT32 frameEn;
  X_FLOAT32 lambdaLTE;

  nbFrame        = vadNS->nbFrame [fstage];
  meanEn         = vadNS->meanEn;
  flagVAD        = vadNS->flagVAD;
  hangOver       = vadNS->hangOver;
  nbSpeechFrames = vadNS->nbSpeechFrames;

  if (nbFrame < 2147483647) nbFrame++;
  vadNS->nbFrame [fstage] = nbFrame;

  if (fstage == 1) return;

  if (nbFrame < NS_NB_FRAME_THRESHOLD_LTE)
    lambdaLTE = 1 - 1 / (X_FLOAT32) nbFrame;
  else
    lambdaLTE = NS_LAMBDA_LTE_LOWER_E;

  frameEn = 64.0;

  for (i=0 ; i<NS_FRAME_SHIFT ; i++)
    frameEn += newShiftFrame[i] * newShiftFrame[i];

  frameEn = (X_FLOAT32) (0.5 + (log (frameEn / 64.0) / log(2.0)) * 16.0);

  if (((frameEn - meanEn) < NS_SNR_THRESHOLD_UPD_LTE) || (nbFrame < NS_MIN_FRAME))
	{
	  if ((frameEn < meanEn) || (nbFrame < NS_MIN_FRAME))
		meanEn += (1 - lambdaLTE) * (frameEn - meanEn);
	  else
		meanEn += (1 - NS_LAMBDA_LTE_HIGHER_E) * (frameEn - meanEn);

	  if (meanEn < NS_ENERGY_FLOOR)
		meanEn = NS_ENERGY_FLOOR;
	}
  if (nbFrame > 4)
  	{
	  if ((frameEn - meanEn) > NS_SNR_THRESHOLD_VAD)
		{
		  flagVAD = 1;
		  nbSpeechFrames++;
		}
	  else
		{
		  if (nbSpeechFrames > NS_MIN_SPEECH_FRAME_HANGOVER)
			hangOver = NS_HANGOVER;
		  nbSpeechFrames = 0;
		  if (hangOver != 0)
			{
			  hangOver--;
			  flagVAD = 1;
			}
		  else
			flagVAD = 0;
		}
	}
  vadNS->meanEn         = meanEn;
  vadNS->flagVAD        = flagVAD;
  vadNS->hangOver       = hangOver;
  vadNS->nbSpeechFrames = nbSpeechFrames;

  return;
}

/*----------------------------------------------------------------------------
 * FUNCTION NAME: FilterCalc
 *
 * PURPOSE:       Computation of noise suppression filter in the frequency domain
 *              
 * INPUT:
 *   fstage       Stage of two stage noise reduction
 *   *NSX         Pointer to noise suppression structure
 *   *PSDMeaned   Pointer to smoothed PSD     
 *
 * OUTPUT:
 *   W            Noise suppression filter
 *
 * RETURN VALUE:
 *   none
 *
 *---------------------------------------------------------------------------*/
static void FilterCalc (X_INT16 fstage, NoiseSupStructX *NSX, X_FLOAT32 *PSDMeaned, X_FLOAT32 *W)
{
  const X_INT16 flagVAD = NSX->nsVar.vadNS.flagVAD;
  const X_INT32 nbFrame = NSX->nsVar.vadNS.nbFrame[fstage];

  X_FLOAT32 lambdaNSE;

  X_FLOAT32 *nSigSE   = NSX->nsVar.spectrum.nSigSE1;
  X_FLOAT32 *noiseSE  = NSX->nsVar.spectrum.noiseSE1;
  X_FLOAT32 *denSigSE = NSX->nsVar.spectrum.denSigSE1;
  if (fstage == 1)
	{
	  nSigSE    = NSX->nsVar.spectrum.nSigSE2;
	  noiseSE   = NSX->nsVar.spectrum.noiseSE2;
	  denSigSE  = NSX->nsVar.spectrum.denSigSE2;
	}


  /*-------------------------------------------------------
   * Choice of the Noise Estimation according to 2WF stage
   * VAD based NE only at 1st stage of 2WF
   *-------------------------------------------------------*/

  /*--------------------------------
   * non VAD based Noise Estimation
   *--------------------------------*/
  if (fstage == 1)
	{
	  // noise estimation in energy
	  for (int i = 0; i < NS_SPEC_ORDER; i++)
		noiseSE[i] *= noiseSE[i];

	  if (nbFrame < 11)
		{
		  lambdaNSE = 1 - 1 / (X_FLOAT32) nbFrame;
		  for (int i = 0; i < NS_SPEC_ORDER; i++)
			{
			  noiseSE[i] = lambdaNSE * noiseSE[i] + (1 - lambdaNSE) * PSDMeaned[i];
			  if (noiseSE[i] < NS_EPS) noiseSE[i] = NS_EPS;
			}
		}
	  else
		{
		  X_FLOAT32 upDate;
		  for (int i = 0; i < NS_SPEC_ORDER; i++)
			{
			  upDate = 0.9 + 0.1 * (PSDMeaned [i] / (PSDMeaned [i] + noiseSE [i])) *
				(1.0 + 1.0 / (1.0 + 0.1 * (PSDMeaned[i] / noiseSE[i])));
			  noiseSE [i] *= upDate;
			}
		}

	  // store noise estimation values in magnitude
	  for (int i = 0; i < NS_SPEC_ORDER; i++)
		{
		  noiseSE [i] = (X_FLOAT32) sqrt (noiseSE[i]);
		  if (noiseSE[i] < NS_EPS) noiseSE[i] = NS_EPS;
		}
	}
  
  /*----------------------------------------------------------------------------
   * Spectral estimations of noisy signal are now stored in magnitude in nSigSE
   *----------------------------------------------------------------------------*/
  for (int i = 0; i < NS_SPEC_ORDER; i++)
	{
	  nSigSE[i]    = (X_FLOAT32) sqrt (nSigSE[i]);
	  PSDMeaned[i] = (X_FLOAT32) sqrt (PSDMeaned[i]);
	}
  
  /*-------------------------------------------
   * VAD based Noise Estimation (in magnitude)
   *-------------------------------------------*/
  if (fstage == 0)
	{
	  if (nbFrame < NS_NB_FRAME_THRESHOLD_NSE)
		lambdaNSE = 1 - 1 / (X_FLOAT32) nbFrame;
	  else
		lambdaNSE =  NS_LAMBDA_NSE;

	  if (flagVAD == 0)
		{
		  for (int i = 0; i < NS_SPEC_ORDER; i++)
			{
			  noiseSE [i] = lambdaNSE * noiseSE [i] + (1 - lambdaNSE) * PSDMeaned[i];
			  if (noiseSE[i] < NS_EPS) noiseSE[i] = NS_EPS;
			}
		}
	}

  /*--------------------------------------
   * noise suppression filter calculation
   *--------------------------------------*/
  for (int i = 0; i < NS_SPEC_ORDER; i++)
	{
	  const X_FLOAT32 SNRpost = (PSDMeaned[i] / noiseSE[i]) - 1;
	  X_FLOAT32 SNRprio = NS_BETA * (denSigSE[i] / noiseSE[i]) + (1 - NS_BETA) * etsi_max(0, SNRpost);
	  W[i] = SNRprio / (1 + SNRprio);
	  SNRprio = W[i] * PSDMeaned[i] / noiseSE[i];
	  SNRprio = etsi_max (SNRprio, NS_RSB_MIN);
	  W[i] = SNRprio / (1 + SNRprio);
	}
   for (int i = 0; i < NS_SPEC_ORDER; i++)
	{
	  denSigSE [i] = W[i] * nSigSE[i];
	}

  return;
}

/*----------------------------------------------------------------------------
 * FUNCTION NAME: DoGainFact
 *
 * PURPOSE:       Apply gain factorization to the noise suppression filter
 *              
 * INPUT:
 *   fstage       Stage of the two stage noise reduction
 *   *NSX         Pointer to noise suppression structure     
 *
 * OUTPUT:
 *   W            Noise suppression filter
 *
 * RETURN VALUE:
 *   none
 *
 *---------------------------------------------------------------------------*/
static void DoGainFact (X_INT16 fstage, NoiseSupStructX *NSX, X_FLOAT32 *W)
{
  X_INT16 i;

  X_FLOAT32 averSNR;
  X_FLOAT32 noiseEn;
  X_FLOAT32 lambdaSNR;
  X_FLOAT32 *noiseSE  = NSX->nsVar.spectrum.noiseSE2;
  X_FLOAT32 *denSigSE = NSX->nsVar.spectrum.denSigSE1;

  GAIN_FACT *gainFact = &(NSX->nsVar.gainFact);

  if (fstage == 0)
	{
	  gainFact->denEn1 [0] = gainFact->denEn1 [1];
	  gainFact->denEn1 [1] = gainFact->denEn1 [2];
	  gainFact->denEn1 [2] = 0.0;
	  for (i=0 ; i<NS_SPEC_ORDER ; i++) gainFact->denEn1 [2] += denSigSE [i]; // new denEn1
	}
  else
	{
	  noiseEn = 0.0;
	  for (i=0 ; i<NS_SPEC_ORDER ; i++) noiseEn += noiseSE [i];
	  averSNR = (gainFact->denEn1 [0] * gainFact->denEn1 [1] * gainFact->denEn1 [2]) / (noiseEn * noiseEn * noiseEn);

	  if (averSNR > 0.00001)
		averSNR = (20 * log10 (averSNR)) / 3.0;
	  else
		averSNR = -100.0 / 3.0;

	  if ( ((averSNR - gainFact->lowSNRtrack) < 10.0) || (NSX->nsVar.vadNS.nbFrame[fstage] < NS_MIN_FRAME) )
		{
		  if (NSX->nsVar.vadNS.nbFrame[fstage] < NS_MIN_FRAME)
			lambdaSNR = 1.0 - 1.0 / (X_FLOAT32) NSX->nsVar.vadNS.nbFrame[fstage];
		  else 
			{
			  if (averSNR < gainFact->lowSNRtrack)
				lambdaSNR = 0.95;
			  else
				lambdaSNR = 0.99;
			}
		  gainFact->lowSNRtrack += (1.0 - lambdaSNR) * (averSNR - gainFact->lowSNRtrack);
		}

	  if (gainFact->denEn1 [2] > 100) // no change if very low signal
		{
		  if (averSNR < (gainFact->lowSNRtrack + 3.5))
			{
			  gainFact->alfaGF += 0.15;
			  if (gainFact->alfaGF > 0.8) gainFact->alfaGF = 0.8;
			}
		  else 
			{
			  gainFact->alfaGF -= 0.3;
			  if (gainFact->alfaGF < 0.1) gainFact->alfaGF = 0.1;
			}
		}

	  for (i=0 ; i<WF_MEL_ORDER ; i++)
		W[i] = gainFact->alfaGF * W[i] + (1.0 - gainFact->alfaGF) * 1.0;
	}
}

/*----------------------------------------------------------------------------
 * FUNCTION NAME: DoFilterWindowing
 *
 * PURPOSE:       Filter windowing
 *              
 * INPUT:
 *   *filterIRIn  Pointer to filter impulse response
 *   *hanningWindow Pointer to Hanning window
 *
 * OUTPUT:
 *   filterIROut  Output windowed filter impulse response
 *
 * RETURN VALUE:
 *   none
 *
 *---------------------------------------------------------------------------*/
static void DoFilterWindowing (X_FLOAT32 *filterIRIn, X_FLOAT32 *hanningWindow, X_FLOAT32 *filterIROut)
{
  X_INT16 i, j;

  for (i=NS_HALF_FILTER_LENGTH,j=0 ; i<NS_FILTER_LENGTH ; i++,j++)
	{
	  filterIROut [NS_HALF_FILTER_LENGTH+j] = filterIRIn [j] * hanningWindow [i];
	  filterIROut [NS_HALF_FILTER_LENGTH-j] = filterIROut [NS_HALF_FILTER_LENGTH+j];
	}
}

/*----------------------------------------------------------------------------
 * FUNCTION NAME: SpeechQSpec
 *
 * PURPOSE:       Voice Activity Detection for Frame Dropping
 *              
 * INPUT:
 *   FEParamsX *  Pointer to frontend parameter structure   
 *
 * OUTPUT:        VAD frame dropping structure updated
 *
 *
 * RETURN VALUE:
 *   none
 *
 *---------------------------------------------------------------------------*/
static X_INT16 SpeechQSpec (FEParamsX *This)
{
  NoiseSupStructX *NSX = This->NSX;
  VAD_DATA_FD *vadFD = &(NSX->nsVar.vadFD);
  X_INT32 FrameCounter = This->NSX->nsVar.vadNS.nbFrame[0];
  X_FLOAT32 Acceleration;					

	if (FrameCounter == 1) {
		vadFD->SpecMean = vadFD->SpecValues;
	}

  if (FrameCounter < INIT_NS_FRAME)
	{
	  vadFD->AccTest = 1.1 * (vadFD->AccTest * (X_FLOAT32)(FrameCounter - 1) + vadFD->SpecValues) / (X_FLOAT32)(FrameCounter);
	  Acceleration = vadFD->SpecValues / vadFD->AccTest;
		vadFD->SpeechInVADQ = 0;	//
	  if (Acceleration > 2.5)
		{
		  vadFD->SpeechInVADQ = 1;	
		}
	  if (vadFD->SpeechInVADQ == 0)
		{
		  vadFD->SpecMean = etsi_max (vadFD->SpecMean, vadFD->SpecValues);
		}
	}
				
  if (vadFD->SpecValues<vadFD->SpecMean*1.5 && vadFD->SpecValues>vadFD->SpecMean*0.75)
	{
	  vadFD->SpecMean = (vadFD->SpecMean * 0.8 + vadFD->SpecValues * 0.2);
	}	

  if (vadFD->SpecValues <= vadFD->SpecMean*0.5)
	{
	  vadFD->SpecMean = (vadFD->SpecMean * 0.97 + vadFD->SpecValues * 0.03);
	}	

  if (vadFD->SpecValues > vadFD->SpecMean*1.65)
	{
	  return 1;
	}
  else
	{
	  return 0;
	}
}

/*----------------------------------------------------------------------------
 * FUNCTION NAME: SpeechQMel
 *
 * PURPOSE:       Voice Activity Detection for Frame Dropping
 *              
 * INPUT:
 *   FEParamsX *  Pointer to frontend parameter structure   
 *
 * OUTPUT:        VAD frame dropping structure updated
 *
 *
 * RETURN VALUE:
 *   none
 *
 *---------------------------------------------------------------------------*/
static X_INT16 SpeechQMel (FEParamsX *This)
{
  NoiseSupStructX *NSX = This->NSX;
  VAD_DATA_FD *vadFD = &(NSX->nsVar.vadFD);
  X_INT32 FrameCounter = This->NSX->nsVar.vadNS.nbFrame[0];
  X_FLOAT32 Acceleration;
  X_FLOAT32 SmoothMel;
  X_INT16 bSpeechFound = 0;
 
  SmoothMel = 0.75 * vadFD->MelValues[1] + 0.25 * vadFD->MelValues[0];
  vadFD->MelValues[0] = vadFD->MelValues[1];

	if (FrameCounter < INIT_NS_FRAME)
	{
		vadFD->MelMean = etsi_max (vadFD->MelMean, SmoothMel);
	}
//  else {
		if (SmoothMel < vadFD->MelMean*1.5 && SmoothMel > vadFD->MelMean*0.75)
		{
//		  vadFD->MelMean = (vadFD->MelMean * 0.8 + SmoothMel * 0.2);
			vadFD->MelMean = (vadFD->MelMean * 0.95 + SmoothMel * 0.05);
		}
// 	  else if ( SmoothMel <= vadFD->MelMean*0.75 && SmoothMel > vadFD->MelMean*0.5 ) {
// 		  vadFD->MelMean = (vadFD->MelMean * 0.9 + SmoothMel * 0.1);
// 	  }
	
	if (SmoothMel <= vadFD->MelMean*0.5)
	{
		vadFD->MelMean = (vadFD->MelMean * 0.97 + SmoothMel * 0.03);
	}	

  vadFD->MelMean = etsi_max(vadFD->MelMean, MIN_MEL_MEAN);
  vadFD->MelMean = etsi_min(vadFD->MelMean, MAX_MEL_MEAN);

  Acceleration = SmoothMel / vadFD->MelMean;

//  if (SmoothMel > vadFD->MelMean * 3.25)
  if (Acceleration > 3.25f || SmoothMel >= SPEECH_MEL_TH || This->FrameClass==3 ||
	  (Acceleration > 2.0f && This->specEntropy > SPEECH_SPEC_H) ||
	  This->specEntropy >= VOICED_SPEC_H )
	{
	  return 1;
	}
  else
	{
	  return 0;
	}
}

/*----------------------------------------------------------------------------
 * FUNCTION NAME: SpeechQVar
 *
 * PURPOSE:       Voice Activity Detection for Frame Dropping
 *              
 * INPUT:
 *   FEParamsX *  Pointer to frontend parameter structure
 *   *W           Pointer to noise suppression filter  
 *
 * OUTPUT:        VAD frame dropping structure updated
 *
 *
 * RETURN VALUE:
 *   none
 *
 *---------------------------------------------------------------------------*/
static X_INT16 SpeechQVar (FEParamsX *This, X_FLOAT32 *W)
{
  NoiseSupStructX *NSX = This->NSX;
  VAD_DATA_FD *vadFD = &(NSX->nsVar.vadFD);
  X_INT16 i; 
  X_INT16 ssize = NS_FFT_LENGTH / 4;
  X_INT32 FrameCounter = This->NSX->nsVar.vadNS.nbFrame[0];
  X_FLOAT32 var = 0.0;
  X_FLOAT32 mean = 0.0;
  X_FLOAT32 specVar = 0.0;
  
  for (i=0 ; i<ssize ; i++)
	{
	  mean += W[i];
	  var  += W[i] * W[i];
	}
  specVar = (var / ssize)  - mean * mean / (ssize * ssize);

  if (FrameCounter < INIT_NS_FRAME)
	{
	  vadFD->VarMean = etsi_max (vadFD->VarMean, specVar);
	}

  if (specVar<vadFD->VarMean*1.5 && specVar>vadFD->VarMean*0.85)
	{
	  vadFD->VarMean = (vadFD->VarMean * 0.8 + specVar * 0.2);
	}

  if (specVar <= vadFD->VarMean*0.25)
	{
	  vadFD->VarMean = (vadFD->VarMean * 0.97 + specVar * 0.03);
	}

  if (specVar > vadFD->VarMean*1.65)
	{
	  return 1;
	}
  else
	{
	  return 0;
	}
}



/*----------------------------------------------------------------------------
 * FUNCTION NAME: EstimateSpectralEntropy
 *
 * PURPOSE:       Compute spectral entropy
 *              
 * INPUT:
 *   FEParamsX *  Pointer to frontend parameter structure
 *   powerSpec    Pointer to spectrum magnitude vector  
 *   LogEnergy	  frame log energy
 *
 * OUTPUT:        spectral entropy
 *
 *
 * RETURN VALUE:
 *   none
 *
 *---------------------------------------------------------------------------*/
static float EstimateSpectralEntropy(FEParamsX *This, float *powerSpec, float LogEnergy)
{
	X_INT32 FrameCounter = This->NSX->nsVar.vadNS.nbFrame[0];
	short nLowerBin = 8;
	short nHigherBin = 120;
	float L_entropy = 0;
	float binProb = 0;
	float *lastEng = 0;
	float *specEng = 0;
	float *avgNoiseSpec = 0;
	float *whiteSpec = 0;
	float minProb = (float)(1.e-4);
	float minEntropy = MIN_SPEC_H;
	float fullBandSpec = 0;
	float minBinSpec = 100.0f;	//60000.0f;
	float minWhiteSpec = 0;
	float whitenSpec[FFT_LENGTH];
	float diffLogE = 0;

	diffLogE = LogEnergy - This->avgNoiseLogE;
	if (FrameCounter < INIT_NS_FRAME) {
		specEng = powerSpec + nLowerBin;
		lastEng = powerSpec + nHigherBin;
		avgNoiseSpec = This->avgNoiseSpec + nLowerBin;
		while ( specEng < lastEng ) {
			*avgNoiseSpec = etsi_max(*avgNoiseSpec, *specEng); avgNoiseSpec++; specEng++;
			*avgNoiseSpec = etsi_max(*avgNoiseSpec, *specEng); avgNoiseSpec++; specEng++;
			*avgNoiseSpec = etsi_max(*avgNoiseSpec, *specEng); avgNoiseSpec++; specEng++;
			*avgNoiseSpec = etsi_max(*avgNoiseSpec, *specEng); avgNoiseSpec++; specEng++;
		}
		This->avgNoiseLogE = etsi_max(This->avgNoiseLogE, LogEnergy);
//		This->avgNoiseLogE = etsi_min(This->avgNoiseLogE, 18.0f);
		return minEntropy;
	}
	else if ( This->SpeechFoundSpec == 0 && diffLogE < 1.0f ) {
		specEng = powerSpec + nLowerBin;
		lastEng = powerSpec + nHigherBin;
		avgNoiseSpec = This->avgNoiseSpec + nLowerBin;
		while ( specEng < lastEng ) {
			*avgNoiseSpec += ( *specEng - *avgNoiseSpec) / 64.0f;
			*avgNoiseSpec = etsi_max(*avgNoiseSpec, minBinSpec); 
			specEng++; avgNoiseSpec++;
			*avgNoiseSpec += ( *specEng - *avgNoiseSpec) / 64.0f;
			*avgNoiseSpec = etsi_max(*avgNoiseSpec, minBinSpec); 
			specEng++; avgNoiseSpec++;
			*avgNoiseSpec += ( *specEng - *avgNoiseSpec) / 64.0f;
			*avgNoiseSpec = etsi_max(*avgNoiseSpec, minBinSpec); 
			specEng++; avgNoiseSpec++;
			*avgNoiseSpec += ( *specEng - *avgNoiseSpec) / 64.0f;
			*avgNoiseSpec = etsi_max(*avgNoiseSpec, minBinSpec); 
			specEng++; avgNoiseSpec++;
		}
	}

	minWhiteSpec = 10.0f;	//(float)30;

	specEng = powerSpec + nLowerBin;
	lastEng = powerSpec + nHigherBin;
	avgNoiseSpec = This->avgNoiseSpec + nLowerBin;
	whiteSpec = whitenSpec + nLowerBin;
	while ( specEng < lastEng ) {
		*whiteSpec = *specEng / *avgNoiseSpec;
		*whiteSpec = etsi_max( *whiteSpec, minWhiteSpec );
		fullBandSpec += *whiteSpec;
		specEng++; avgNoiseSpec++; whiteSpec++;
		*whiteSpec = *specEng / *avgNoiseSpec;
		*whiteSpec = etsi_max( *whiteSpec, minWhiteSpec );
		fullBandSpec += *whiteSpec;
		specEng++; avgNoiseSpec++; whiteSpec++;
		*whiteSpec = *specEng / *avgNoiseSpec;
		*whiteSpec = etsi_max( *whiteSpec, minWhiteSpec );
		fullBandSpec += *whiteSpec;
		specEng++; avgNoiseSpec++; whiteSpec++;
		*whiteSpec = *specEng / *avgNoiseSpec;
		*whiteSpec = etsi_max( *whiteSpec, minWhiteSpec );
		fullBandSpec += *whiteSpec;
		specEng++; avgNoiseSpec++; whiteSpec++;
	}

	whiteSpec = whitenSpec + nLowerBin;
	lastEng   = whitenSpec + nHigherBin;
	while ( whiteSpec < lastEng )
	{
		binProb = *whiteSpec / fullBandSpec;
		binProb = etsi_max(binProb, minProb);
		L_entropy -= binProb * (float)log((double)binProb);
		whiteSpec++;
		binProb = *whiteSpec / fullBandSpec;
		binProb = etsi_max(binProb, minProb);
		L_entropy -= binProb * (float)log((double)binProb);
		whiteSpec++;
		binProb = *whiteSpec / fullBandSpec;
		binProb = etsi_max(binProb, minProb);
		L_entropy -= binProb * (float)log((double)binProb);
		whiteSpec++;
		binProb = *whiteSpec / fullBandSpec;
		binProb = etsi_max(binProb, minProb);
		L_entropy -= binProb * (float)log((double)binProb);
		whiteSpec++;
	}

	L_entropy = 1.0f - L_entropy/(float)log((double)(nHigherBin-nLowerBin));

	diffLogE = LogEnergy - This->avgNoiseLogE;
	if ( L_entropy < minEntropy ) {
		if ( diffLogE < 2.0f ) {
			This->avgNoiseLogE = 0.98f * This->avgNoiseLogE + 0.02f * LogEnergy;
		}
		L_entropy = minEntropy;
	}
	else if ( This->SpeechFoundSpec == 0 ) {
		if ( diffLogE < 2.0f ) {
			This->avgNoiseLogE = 0.98f * This->avgNoiseLogE + 0.02f * LogEnergy;
		}
	}

	return L_entropy;
}

/*------------------------
 * start of Encapsulation
 *------------------------*/
/*----------------------------------------------------------------------------
 * FUNCTION NAME: DoNoiseSupAlloc
 *
 * PURPOSE:       Memory allocation of noise suppression structure
 *              
 * INPUT:
 *   none     
 *
 * OUTPUT:        Noise suppression structure
 *
 *
 * RETURN VALUE:
 *   NSX          Pointer to noise suppression structure
 *
 *---------------------------------------------------------------------------*/
extern NoiseSupStructX *DoNoiseSupAlloc (void)
{
	X_INT16 i;
  NoiseSupStructX *NSX = calloc (1, sizeof (NoiseSupStructX));

  if (NSX == NULL)
    return NULL;
  
  NSX->nsVar.SampFreq    = SAMPLING_FREQ_1 * 1000;

  /*--------
   * ns_tmp
   *--------*/
  // Hanning window for spectrum estimation
  for (i=0 ; i<NS_FRAME_LENGTH ; i++)
	NSX->nsTmp.sigWindow[i] = 0.5 - 0.5 * cos ((PIx2 * ((X_FLOAT32) i + 0.5)) / (X_FLOAT32) (NS_FRAME_LENGTH));
  
  // Hanning window for impulse response windowing
  for (i=0 ; i<NS_FILTER_LENGTH ; i++)
	NSX->nsTmp.IRWindow[i] = 0.5 - 0.5 * cos ((PIx2 * ((X_FLOAT32) i + 0.5)) / (X_FLOAT32) (NS_FILTER_LENGTH));

  // mel FB windows
  NSX->nsTmp.FirstWindow = CMelFBAlloc ();
  if (NSX->nsTmp.FirstWindow == NULL)
	{
	  fprintf (stderr, "ERROR:   Memory allocation error occured!\r\n");
	  DoNoiseSupDelete( NSX );
	  return NULL;
	}
  InitMelFBwindows (NSX->nsTmp.FirstWindow, 0.0, (X_FLOAT32)NSX->nsVar.SampFreq, 2*(NS_SPEC_ORDER-1), WF_MEL_ORDER, 1);

  // mel IDCT
  NSX->nsTmp.melIDCTbasis = (X_FLOAT32 **) malloc (sizeof (X_FLOAT32 *) * WF_MEL_ORDER);
  if (NSX->nsTmp.melIDCTbasis == NULL)
	{
	  fprintf (stderr, "ERROR:   Memory allocation error occured!\r\n");
	  DoNoiseSupDelete( NSX );
	  return NULL;
	}

  for (i=0 ; i<WF_MEL_ORDER ; i++)
	{
	  NSX->nsTmp.melIDCTbasis[i] = (X_FLOAT32 *) malloc (sizeof (X_FLOAT32) * WF_MEL_ORDER);
	  if (NSX->nsTmp.melIDCTbasis[i] == NULL)
		{
		  fprintf (stderr, "ERROR:   Memory allocation error occured!\r\n");
		  DoNoiseSupDelete( NSX );
		  return NULL;
		}
	}

  InitMelIDCTbasis (NSX->nsTmp.melIDCTbasis, NSX->nsTmp.FirstWindow, WF_MEL_ORDER, NSX->nsVar.SampFreq, 2*(NS_SPEC_ORDER-1));

  return NSX;
}

#define LBN_MAX_THR   1.9f
#define LBN_NEUTR_MAX_RATIO  LBN_MAX_THR

/*----------------------------------------------------------------------------
 * FUNCTION NAME: DoNoiseSupInit
 *
 * PURPOSE:       Initialisation of noise suppression structure
 *              
 * INPUT:
 *   FEParamsX *  Pointer to frontend parameter structure
 *        
 * OUTPUT:        Initialized noise suppression structure
 *
 * RETURN VALUE:
 *   None
 *
 *---------------------------------------------------------------------------*/
extern void DoNoiseSupInit (FEParamsX *This)
{
  NoiseSupStructX *NSX = This->NSX;

  This->bApply2ndStage = 1;

  This->fCrit = LBN_NEUTR_MAX_RATIO;
  This->FirstFrameFlag = 1;
  This->LBN_FftBreakPoint = 0;
  memset(This->afPreProcTempBuf, 0, sizeof(This->afPreProcTempBuf));
  memset(This->afPreEmphPowerSpec, 0, sizeof(This->afPreEmphPowerSpec));

  memset(&NSX->nsVar, 0, sizeof(NS_VAR));
  NSX->nsVar.SampFreq    = This->SamplingFrequency;

  /*-----------
   * gain_fact
   *-----------*/
  NSX->nsVar.gainFact.alfaGF = 0.8;

  /*-------------
   * vad_data_ns
   *-------------*/
  dsr_afe_vad_init(&This->dataVAD);


  /*----------
   * spectrum
   *----------*/
  for (int i = 0; i<NS_SPEC_ORDER; i++)
	{
	  NSX->nsVar.spectrum.noiseSE1[i]    = NSX->nsVar.spectrum.noiseSE2[i]    = NS_EPS;
	}
}

/*----------------------------------------------------------------------------
 * FUNCTION NAME: DoNoiseSupDelete
 *
 * PURPOSE:       Memory free of noise suppression structure
 *              
 * INPUT:
 *   *NSX         Pointer to noise suppression structure     
 *
 * OUTPUT:
 *   None
 *
 * RETURN VALUE:
 *   None
 *
 *---------------------------------------------------------------------------*/
extern void DoNoiseSupDelete (NoiseSupStructX *NSX)
{
  X_INT16 i;

  if (NSX != NULL)
	{
	  ReleaseMelFBwindows (NSX->nsTmp.FirstWindow);
	  free (NSX->nsTmp.FirstWindow);

	  for (i=0 ; i<WF_MEL_ORDER ; i++)
		free (NSX->nsTmp.melIDCTbasis[i]);

	  free (NSX->nsTmp.melIDCTbasis);

	  free (NSX); 
	}
}


/*----------------------------------------------------------------------------
 * FUNCTION NAME: IsLowBandNoise
 *
 * PURPOSE:       Performs low-band noise detection
 *
 * INPUT:
 *   pfPowerSpec         Power FFT spectrum
 *   iNoOfDftPoints      Number of FFT bins
 *   iBreakPoint         Low-band cutoff frequency expressed in FFT bins
 *   bVad                VAD flag
 *   fEnergy             Energy level
 *
 * OUTPUT
 *   none
 *
 * RETURN VALUE
 *   TRUE        If low band noise is detected
 *   FALSE         Otherwise
 *---------------------------------------------------------------------------*/
static ETSI_BOOL IsLowBandNoise(
                    X_FLOAT32 *pfPowerSpec,
                    X_INT16   iNoOfDftPoints,
                    X_INT16   iBreakPoint,
                    ETSI_BOOL bVad,
                    X_FLOAT32 fEnergy,
					float	  *fCrit
                    )
{
#define LBN_HIST_WEIGHT  0.99f
#define LBN_CURR_WEIGHT (1.f - LBN_HIST_WEIGHT)

#define LBN_MAX_THR   1.9f

#define LBN_NEUTR_MAX_RATIO  LBN_MAX_THR
#define LBN_LOW_ENR_LEVEL 500.f

    int i;
    float fLowMax, fHighMax, fRatio;

    
    if (bVad == TRUE)
      {
        /* Speech frame - the criterion does not change */
        return (*fCrit > LBN_MAX_THR);
      }


    if (fEnergy/(float)iNoOfDftPoints < LBN_LOW_ENR_LEVEL)
      {
        /*   Silence  */
        fRatio = 0.f;
      }
    else
      {
        /* -------------------------------------------- */
        /*       Background Noise                       */
	/* -------------------------------------------- */
           
        /*  Determine maximum in lower band  */
        fLowMax = 0.f;
        for (i = 1; i <= iBreakPoint; i++)
            if (pfPowerSpec[i] > fLowMax) fLowMax = pfPowerSpec[i];

	/* Determine maximum in upper band */
        fHighMax = 0.f;
        for ( ; i <= iNoOfDftPoints; i++)
            if (pfPowerSpec[i] > fHighMax) fHighMax = pfPowerSpec[i];

	/* Compute the ratio */
        if (fHighMax == 0.f)
            fRatio = 10.f;
        else
            fRatio = fLowMax/fHighMax;

      }

    /* Update the criterion */
    *fCrit = LBN_HIST_WEIGHT*(*fCrit) + LBN_CURR_WEIGHT*fRatio;


    /* Make decision */
    return (*fCrit > LBN_MAX_THR);

}


const static X_FLOAT32 ENERGYFLOOR_logE = -50.0;
const static X_FLOAT32 EnergyFloor_logE = 1.9287E-24;


const static X_FLOAT32 afPePower[NS_FFT_LENGTH/2+1] = {
	0.00090000f, 0.00148429f, 0.00323681f, 0.00615651f, 0.01024163f, 0.01548970f, 0.02189757f, 0.02946137f,
	0.03817656f, 0.04803787f, 0.05903937f, 0.07117443f, 0.08443575f, 0.09881533f, 0.11430451f, 0.13089397f,
	0.14857371f, 0.16733307f, 0.18716077f, 0.20804486f, 0.22997275f, 0.25293124f, 0.27690650f, 0.30188408f,
	0.32784895f, 0.35478546f, 0.38267739f, 0.41150793f, 0.44125972f, 0.47191484f, 0.50345482f, 0.53586066f,
	0.56911284f, 0.60319134f, 0.63807563f, 0.67374468f, 0.71017703f, 0.74735071f, 0.78524335f, 0.82383211f,
	0.86309375f, 0.90300462f, 0.94354068f, 0.98467751f, 1.02639033f, 1.06865402f, 1.11144312f, 1.15473185f,
	1.19849414f, 1.24270363f, 1.28733368f, 1.33235742f, 1.37774773f, 1.42347725f, 1.46951845f, 1.51584359f,
	1.56242478f, 1.60923394f, 1.65624288f, 1.70342329f, 1.75074675f, 1.79818475f, 1.84570871f, 1.89329002f,
	1.94090000f, 1.98850998f, 2.03609129f, 2.08361525f, 2.13105325f, 2.17837671f, 2.22555712f, 2.27256606f,
	2.31937522f, 2.36595641f, 2.41228155f, 2.45832275f, 2.50405227f, 2.54944258f, 2.59446632f, 2.63909637f,
	2.68330586f, 2.72706815f, 2.77035688f, 2.81314598f, 2.85540967f, 2.89712249f, 2.93825932f, 2.97879538f,
	3.01870625f, 3.05796789f, 3.09655665f, 3.13444929f, 3.17162297f, 3.20805532f, 3.24372437f, 3.27860866f,
	3.31268716f, 3.34593934f, 3.37834518f, 3.40988516f, 3.44054028f, 3.47029207f, 3.49912261f, 3.52701454f,
	3.55395105f, 3.57991592f, 3.60489350f, 3.62886876f, 3.65182725f, 3.67375514f, 3.69463923f, 3.71446693f,
	3.73322629f, 3.75090603f, 3.76749549f, 3.78298467f, 3.79736425f, 3.81062557f, 3.82276063f, 3.83376213f,
	3.84362344f, 3.85233863f, 3.85990243f, 3.86631030f, 3.87155837f, 3.87564349f, 3.87856319f, 3.88031571f,
	3.88090000f };

/*----------------------------------------------------------------------------
 * FUNCTION NAME: DoNoiseSup
 *
 * PURPOSE:       Performs noise suppression on input signal frame
 *              
 * INPUT:
 *   FEParamsX *  Pointer to frontend parameter structure
 *   *InData      Pointer to input signal frame     
 *
 * OUTPUT:
 *   Outdata      Output signal frame
 *
 * RETURN VALUE:
 *   TRUE         If a frame is outputed
 *   FALSE        If no frame outputed (happens at the beginning due to the latency)
 *
 *---------------------------------------------------------------------------*/
extern ETSI_BOOL DoNoiseSup(const X_FLOAT32 *InData, X_FLOAT32 *OutData, FEParamsX *This)
{
  X_INT16 i; 
  X_INT16 fstage;                      // noise suppression filter stage

  NoiseSupStructX *NSX = This->NSX;

  X_FLOAT32 *FirstStageInFloatBuffer  = NSX->nsVar.buffers.FirstStageInFloatBuffer;
  X_FLOAT32 *SecondStageInFloatBuffer = NSX->nsVar.buffers.SecondStageInFloatBuffer;

  X_INT32 *nbFramesInFirstStage     = &(NSX->nsVar.buffers.nbFramesInFirstStage);
  X_INT32 *nbFramesInSecondStage    = &(NSX->nsVar.buffers.nbFramesInSecondStage);
  X_INT32 *nbFramesOutSecondStage   = &(NSX->nsVar.buffers.nbFramesOutSecondStage);

  X_INT16 *indexBuffer = NULL;     //

  X_FLOAT32 *prvFrame = NULL;      //
  X_FLOAT32 *curFrame = NULL;      //
  X_FLOAT32 *nSigSE = NULL;        //
  X_FLOAT32 *PSDMeanBuffer = NULL; //

  X_FLOAT32 *W = NSX->nsTmp.tmpMem + NS_SPEC_ORDER;               // scratch memory
  X_FLOAT32 *filterIR = This->NSX->nsTmp.tmpMem;                  // scratch memory
  X_FLOAT32 ALIGNED_(ALN_NS) signalIn[NS_SCRATCH_MEM_SIZE] = { 0 };
  X_FLOAT32 ALIGNED_(ALN_NS) signalFft[NS_SCRATCH_MEM_SIZE] = { 0 };
  X_FLOAT32 *signalOut = This->NSX->nsTmp.tmpMem + NS_SPEC_ORDER; // scratch memory
  X_FLOAT32 *PSDMeaned = This->NSX->nsTmp.tmpMem;                 // scratch memory

  ETSI_BOOL dataToProcess;           // data to process ?

  /*
   * Pitch and Class Estimation
   */
  X_FLOAT32 fEnergy;
  X_FLOAT32 fSum;
  X_FLOAT32 LogEnergy;
  X_FLOAT32 pfPowerSpectrum[NS_FFT_LENGTH/2+1];
  X_FLOAT32 pfPowerSpectrumTemp[NS_FFT_LENGTH/2+1];
  X_FLOAT32 PitchPeriod;

  X_INT16 iVad;
  X_INT16 iHangOverFlag;
  X_INT16 iClass;
  X_FLOAT32 fSnr;

  ETSI_BOOL LowBandNoise;

	This->LBN_FftBreakPoint = (X_INT16)(LBN_UPPER_FREQ*(float)NS_FFT_LENGTH/(float)(SAMPLING_FREQ_1*1000));

	/*---------------------------------------------------
	 * input next shift frame in FirstStageInFloatBuffer
	 *---------------------------------------------------*/
	memcpy(&FirstStageInFloatBuffer[NS_DATA_IN_BUFFER], InData, NS_FRAME_SHIFT * sizeof(X_FLOAT32));
  
	(*nbFramesInFirstStage)++;

	/*------------------------------------
	 * Two-Stage Noise Suppression Filter
	 *------------------------------------*/
	for (fstage=0 ; fstage<2 ; fstage++) 
	{
		dataToProcess = FALSE;

		/*----------------------------------------------------
		 * 1st stage reads data from FirstStageInFloatBuffer
		 * 2nd stage reads data from SecondStageInFloatBuffer
		 *----------------------------------------------------*/
		if ((fstage == 0) && ((*nbFramesInFirstStage - *nbFramesInSecondStage) > NS_NB_FRAMES_LATENCY))
		{
			/*-------------------
			 * process 1st stage
			 *-------------------*/
			dataToProcess = TRUE;

			nSigSE = NSX->nsVar.spectrum.nSigSE1;
			PSDMeanBuffer = &(NSX->nsVar.spectrum.PSDMeanBuffer1 [0][0]);
			indexBuffer = &(NSX->nsVar.spectrum.indexBuffer1);
	  
			for (i=0 ; i<NS_FRAME_LENGTH ; i++)
				signalIn[i] = FirstStageInFloatBuffer [NS_ANALYSIS_WINDOW_8K + i];

			prvFrame = &(NSX->nsVar.buffers.FirstStageInFloatBuffer [NS_PRV_FRAME]);
			curFrame = &(NSX->nsVar.buffers.FirstStageInFloatBuffer [NS_CUR_FRAME]);
		}
		else
		if ((fstage == 1) && ((*nbFramesInSecondStage - *nbFramesOutSecondStage) > NS_NB_FRAMES_LATENCY))
		{
			/*-------------------
			 * process 2nd stage
			 *-------------------*/
			dataToProcess = TRUE;
	    
			nSigSE = NSX->nsVar.spectrum.nSigSE2;
			PSDMeanBuffer = &(NSX->nsVar.spectrum.PSDMeanBuffer2 [0][0]);
			indexBuffer = &(NSX->nsVar.spectrum.indexBuffer2);
	    
			for (i=0 ; i<NS_FRAME_LENGTH ; i++)
				signalIn[i] = SecondStageInFloatBuffer [NS_ANALYSIS_WINDOW_8K + i];
	    
			prvFrame = &(NSX->nsVar.buffers.SecondStageInFloatBuffer [NS_PRV_FRAME]);
			curFrame = &(NSX->nsVar.buffers.SecondStageInFloatBuffer [NS_CUR_FRAME]);
		}

		if (!dataToProcess) continue; // no processing required

		/*
		 * Capture Input Speech Signal for Pitch related pre-processing
		 */

		if ((fstage == 0) && (*nbFramesInFirstStage >= 3))
		{
			for (i = 0; i < NS_FRAME_LENGTH; i++)
			{
				This->pfInpSpeech[i] = signalIn[i];
			}             
		} // endif ((fstage == 0) && (*nbFramesInFirstStage >= 3))


		/*-----------------------------------
		 * signal windowing and zero padding
		 *-----------------------------------*/
		DoSigWindowing (signalIn, NSX->nsTmp.sigWindow, NS_FRAME_LENGTH, NS_FFT_LENGTH);
      

		/*-----
		 * FFT
		 *-----*/
		pffft_transform_ordered(This->fft_setup, signalIn, signalFft, 0, PFFFT_FORWARD);

#if 0	// DNN trigger - Disabled VAD frame classification for speed
		// VAD for frame classification
		if ((fstage == 0) && (*nbFramesInFirstStage >= 3))
		{
			X_FLOAT32 Stft[NS_FFT_LENGTH+2];
			X_FLOAT32 fSpecAverage = 0.f;

			/*
			 * Copy STFT for use by pitch extractor.
			 * Note: pitch-estimator expects to get the STFT values
			 * in the order: real[0]  imag[0]  real[1]  imag[1] ....
			 */
			memcpy(Stft, signalFft, NS_FFT_LENGTH * sizeof(float));
			Stft[0] = 0.f; // Notch DC
			Stft[1] = 0.f;
			Stft[NS_FFT_LENGTH] = signalFft[1];
			Stft[NS_FFT_LENGTH+1] = 0.f;

			for (int idx = 2; idx < NS_FFT_LENGTH; idx+=2)
			{
				fSpecAverage += signalFft[idx];
			}
			fSpecAverage *= 2.f;
			fSpecAverage += signalFft[1];
			fSpecAverage /= (float)NS_FFT_LENGTH;

			/*
			 * Power and Mag. spectra after DC removal
			 */
			pfPowerSpectrum[0] = (X_FLOAT32)((double)Stft[0]*(double)Stft[0]);
			for (i = 1; i < NS_FFT_LENGTH/2; i++)
			{
				pfPowerSpectrum[i] = (X_FLOAT32)((double)Stft[2*i]*(double)Stft[2*i] + (double)Stft[2*i+1]*(double)Stft[2*i+1]);
				This->afPreEmphPowerSpec[i] = pfPowerSpectrum[i]*afPePower[i];
			}
            pfPowerSpectrum[NS_FFT_LENGTH/2] = (X_FLOAT32)((double)Stft[NS_FFT_LENGTH]*(double)Stft[NS_FFT_LENGTH]);
			This->afPreEmphPowerSpec[NS_FFT_LENGTH/2] = pfPowerSpectrum[NS_FFT_LENGTH/2]*afPePower[NS_FFT_LENGTH/2];

			/*
			 * Compute frame energy from input speech
			 */
			fEnergy = (This->pfInpSpeech[0]*This->pfInpSpeech[0]);
			fSum = This->pfInpSpeech[0];
			for (i = 1; i < NS_FRAME_LENGTH; i++)
			{
				fEnergy += (This->pfInpSpeech[i]*This->pfInpSpeech[i]);
				fSum += This->pfInpSpeech[i];
			}
			fEnergy -= ((fSum*fSum) / (float)NS_FRAME_LENGTH);
			if (fEnergy < EnergyFloor_logE)
			{
				fEnergy = EnergyFloor_logE;
				LogEnergy = ENERGYFLOOR_logE;
			}
			else
			{
              LogEnergy = (X_FLOAT32) log((double)fEnergy);
			}

			/*
			 * Voice Activity Detection
			 */

			for (i = 0; i <= NS_FFT_LENGTH/2; i++)
			{
				pfPowerSpectrumTemp[i] = pfPowerSpectrum[i];
			}
    
			DoMelFB(pfPowerSpectrumTemp,This->FirstWindow);

			iVad = dsr_afe_vad(&This->dataVAD, pfPowerSpectrumTemp,&iHangOverFlag,&fSnr);
            
			/*
			 * Pitch extraction
			 */

			LowBandNoise = IsLowBandNoise(This->afPreEmphPowerSpec,
										  NS_FFT_LENGTH/2,
										  This->LBN_FftBreakPoint,
										  iVad,
										  fEnergy,
										  &(This->fCrit) );


			if (This->FirstFrameFlag)
			{
				for (i=0; i<FRAME_SHIFT; i++)
					This->afPreProcTempBuf[i] = 0.f;
				for (i=0; i<FRAME_LENGTH -FRAME_SHIFT; i++)
					This->afPreProcTempBuf[i+FRAME_SHIFT] = This->pfInpSpeech[i];

				pre_process(FRAME_LENGTH,FRAME_SHIFT,HISTORY_LENGTH,DOWN_SAMP_FACTOR,
							This->afPreProcTempBuf,SAMPLING_FREQ_1*1000,This->FirstFrameFlag,
							FALSE,This->pfUBSpeech,This->pfProcSpeech,
							This->pfDownSampledProcSpeech);                 

				This->FirstFrameFlag = 0;
			}
                          
			pre_process(FRAME_LENGTH,FRAME_SHIFT,HISTORY_LENGTH,DOWN_SAMP_FACTOR,
						This->pfInpSpeech,SAMPLING_FREQ_1*1000,This->FirstFrameFlag,
						LowBandNoise,This->pfUBSpeech,This->pfProcSpeech,
						This->pfDownSampledProcSpeech);

			EstimatePitch(This->pPitchRom,
						  This->pPitchEstimator,
						  fSpecAverage,
						  This->pfDownSampledProcSpeech+HISTORY_LENGTH/DOWN_SAMP_FACTOR,
						  DOWN_SAMP_FACTOR,
						  Stft,
						  pfPowerSpectrum,
						  LogEnergy,
						  iVad,
						  LowBandNoise,
						  &PitchPeriod,
						  &This->pcorrData);

 

			/*
			 * Classify frame
			 */

			iClass = classify_frame(iVad,iHangOverFlag,
									PitchPeriod,fEnergy,
									This->pfInpSpeech,This->pfUBSpeech);

			/*
			 * Output class 
			 */

			This->FrameClass = iClass;

		} // endif ((fstage == 0) && (*nbFramesInFirstStage >= 3))
#endif
		/*-------------------------------------------------------------
		 * FFT spectrum (signalFft) -> power spectrum (NSX->nSigSE)
		 *-------------------------------------------------------------*/
		FFTtoPSD(signalFft, nSigSE, NS_FFT_LENGTH);

		/*---------
		 * PSDMean
		 *---------*/
		PSDMean (indexBuffer, nSigSE, PSDMeaned, PSDMeanBuffer);

		/*---------------------------
		 * VAD for Noise Suppression
		 *---------------------------*/
		VAD (fstage, &(NSX->nsVar.vadNS), curFrame); 

		/*--------------------------
		 * filter gains calculation
		 *--------------------------*/
      FilterCalc (fstage, NSX, PSDMeaned, W);

		/*------------------------
		 * VAD for Frame Dropping
		 *------------------------*/
		if (fstage == 0)
		{
			This->SpeechFoundVar = SpeechQVar (This, W);
		}

		/*-----------------
		 * mel filter bank
		 *-----------------*/
		DoMelFB (W, NSX->nsTmp.FirstWindow);

#if 0	// DNN trigger - Disabled VAD frame classification for speed
		/*------------------------
		 * VAD for Frame Dropping
		 *------------------------*/
		if (fstage == 0)
		{
			X_FLOAT32 TempEn = 0.0;
	  
			for(i=0 ; i<WF_MEL_ORDER ; i++) TempEn += (X_FLOAT32)W[i];
			TempEn = etsi_max(TempEn, 2.0f);

			NSX->nsVar.vadFD.SpecValues = TempEn * TempEn - 3.0;
	
			This->SpeechFoundSpec = SpeechQSpec (This);

			/*
			 * Compute spectral entropy
			 */
			This->specEntropy = EstimateSpectralEntropy(This, pfPowerSpectrum, LogEnergy);

			NSX->nsVar.vadFD.MelValues[1] = (X_FLOAT32)(W[1] + W[2] + W[3]) / 3.0;

			This->SpeechFoundMel = SpeechQMel (This);

// 			if ( This->FrameCounter == INIT_NS_FRAME && This->avgNoiseLogE <= 16.01f ) {
// 				This->bApply2ndStage = 0;
// 			}
		}
#endif
		/*--------------------
		 * gain factorization
		 *--------------------*/
		DoGainFact (fstage, NSX, W);

		/*-----------------
		 * mel inverse DCT
		 *-----------------*/
		DoMelIDCT (W, NSX->nsTmp.melIDCTbasis, WF_MEL_ORDER, WF_MEL_ORDER);
		for (i=1 ; i<WF_MEL_ORDER ; i++) W [2 * WF_MEL_ORDER - 1 - i] = W [i];

		/*------------------
		 * filter windowing
		 *------------------*/
		DoFilterWindowing (W, NSX->nsTmp.IRWindow, filterIR);

		/*--------------------------------------------------------------
		 * apply WF to noisy signal, output samples stored in signalOut
		 *--------------------------------------------------------------*/
		ApplyWF (curFrame, prvFrame, filterIR, signalOut, NS_FRAME_SHIFT, NS_HALF_FILTER_LENGTH);

		/*---------------------------------------------------
		 * update buffer with filtered signal for next stage
		 *---------------------------------------------------*/
		if (fstage == 0)
		{
			for (i=0 ; i<NS_FRAME_SHIFT ; i++)
				SecondStageInFloatBuffer[NS_DATA_IN_BUFFER + i] = signalOut[i];

			NSX->nsVar.buffers.nbFramesInSecondStage++;

			if ( This->bApply2ndStage == 0 ) 
			{
				for (i=0 ; i<NS_FRAME_SHIFT ; i++)
					OutData[i] = signalOut[i];
				
				NSX->nsVar.buffers.nbFramesOutSecondStage++;
			}
		}
		else 
		if (fstage == 1)
		{
			for (i=0 ; i<NS_FRAME_SHIFT ; i++)
				OutData[i] = signalOut[i];

			NSX->nsVar.buffers.nbFramesOutSecondStage++;
		}

		/*------------------------
		 * VAD for Frame Dropping
		 *------------------------*/
		if (fstage == 0)
		{
			if (NSX->nsVar.vadNS.nbSpeechFrames > 4)
				This->SpeechFoundVADNS = 1;
			else
				This->SpeechFoundVADNS = 0;	 
		}
		This->FrameCounter = This->NSX->nsVar.vadNS.nbFrame[0];

		if ( This->bApply2ndStage == 0 ) fstage = 1;

	}	// for (fstage=0 ; fstage<2 ; fstage++) 

	/*--------------------------------------------
	 * shift data in NSX->FirstStageInFloatBuffer
	 *--------------------------------------------*/
	if (NSX->nsVar.buffers.nbFramesInFirstStage)
	{
		// copy the content of FirstStageInFloatBuffer [80..319] to [0..239]
		for (i=0 ; i<NS_DATA_IN_BUFFER ; i++)
			FirstStageInFloatBuffer [i] = FirstStageInFloatBuffer [i + NS_FRAME_SHIFT];
	}

	/*---------------------------------------------
	 * shift data in NSX->SecondStageInFloatBuffer
	 *---------------------------------------------*/
	if (NSX->nsVar.buffers.nbFramesInSecondStage && This->bApply2ndStage)
	{
		for (i=0 ; i<NS_DATA_IN_BUFFER ; i++)
			SecondStageInFloatBuffer [i] = SecondStageInFloatBuffer [i + NS_FRAME_SHIFT];
	}

	if (NSX->nsVar.buffers.nbFramesOutSecondStage > 0)
	{
		DCOffsetFil (OutData, &(NSX->nsVar.prevSamples), NS_FRAME_SHIFT);
		return TRUE;
	}
	else
	{
		return FALSE;
	}

}




