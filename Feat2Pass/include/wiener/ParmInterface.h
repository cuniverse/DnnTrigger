/*===============================================================================
 *      ETSI ES 202 212   Distributed Speech Recognition
 *      Extended Advanced Front-End Feature Extraction Algorithm & Compression Algorithm
 *      Speech Reconstruction Algorithm.
 *      C-language software implementation                                      
 *      Version 1.1.1   October, 2003                                            
 *===============================================================================*/
/*--------------------------------------------------------------------------
 *
 * FILE NAME: ParmInterface.h
 * PURPOSE: Processing one input frame in DoAdvProcess():
 *          DoNoiseSup(), DoWaveProc(), DoCompCeps(), DoPostProc(),
 *          and DoVADProc() are called.
 *
 *--------------------------------------------------------------------------*/
#ifndef _PARMINTERFACE_H
#define _PARMINTERFACE_H

/*-----------------
 * File Inclusions
 *-----------------*/
#include <stdio.h>
#include "BufferIn.h"
#include "ParmType.h"

/*------------------------------------
 * Definition of Front-end Parameters 
 *------------------------------------*/
#define SAMPLING_FREQ_1    8   // 8kHz
#define SAMPLING_FREQ_2   11   // 11kHz
#define SAMPLING_FREQ_3   16   // 16kHz

#define FRAME_LENGTH     200   // 25ms

#define FRAME_SHIFT       80   // 10ms

#define FFT_LENGTH       256
#define FRAME_BUF_SIZE   241

#define STARTING_FREQ     64.0 // 55.401825

#define NUM_CEP_COEFF     13   //c1...c12 + c0

#define HISTORY_LENGTH   120
#define DOWN_SAMP_FACTOR   4
#define PRE_EMPHASIS     0.9
#define LBN_UPPER_FREQ   380.0f

#define NON_SPEECH_FRAME   0
#define SPEECH_FRAME       1

#define PIx2               6.28318530717958647692

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

#ifndef _defined_FEParamsX

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

/*------------------------------------------------------
 * Private structures for the modules (noise reduction,
 * SNR waveform processing, cepstrum calculation, ...)
 *------------------------------------------------------*/
typedef struct NoiseSupStructX NoiseSupStructX;
typedef struct MelFB_Window    MelFB_Window;

/*------------------------------------------------------
 * FEParamsX must (only) contains:
 *  - initialisation parameters
 *    (like FrameLength, FrameShift, FFTLength)
 *  - buffers to transmit data between modules
 *    (like denoisedBuf)
 *  - specific information which must be remanent
 *    between two frames, and which can't be 
 *    encapsulated in modules
 *  - if a data is not remanent, they must be transmitted
 *    between modules by a GET ans a SET function
 *  - if a data can be encapsulated in a module, it must
 *    be encapsulated to avoid conflict between modules
 *------------------------------------------------------*/

typedef struct FEParamsX FEParamsX;
typedef struct PFFFT_Setup PFFFT_Setup;

struct FEParamsX
{
  int FrameClass;			//
  int CoefNb;				//
  int FFTLength;			//
  int FrameShift;			//
  int FrameLength;			//
  int FrameCounter;         //
  int SpeechFoundMel;		//
  int SpeechFoundVar;		//
  int SpeechFoundSpec;		//
  int SpeechFoundVADNS;     //
  int NbSamplesToRead;		//
  int SamplingFrequency;	//

  int bApply2ndStage;		//

  float StartingFrequency;	//

  float avgNoiseSpec[FFT_LENGTH];		//
  float specEntropy;		//
  float avgNoiseLogE;		//

  long NonZeroFrameOnset;	//
  long ZeroFrameCounter;	//

  BufferIn *denoisedBuf;	//
  X_FLOAT32 *CurFrame;	    //

  MelFB_Window *FirstWindow;       //

  AFE_VAD_DATA dataVAD;

  /*---------------------------
   * module specific expansion
   *---------------------------*/
  NoiseSupStructX *NSX;

  /*-------------------------------------
   * methods: use pointer to function to
   *          emulate generic function
   *-------------------------------------*/

  /*-----------------------
   * for noise suppression 
   *-----------------------*/
  NoiseSupStructX* (*DoNoiseSupAlloc) (void);
  void (*DoNoiseSupInit) (FEParamsX *this);
  ETSI_BOOL (*DoNoiseSup) (X_FLOAT32 *InData,
						   X_FLOAT32 *outData,
						   FEParamsX *this);
  void (*DoNoiseSupDelete) (NoiseSupStructX*);

  float fCrit;
  X_INT16 FirstFrameFlag;
  X_INT16 LBN_FftBreakPoint;
  X_FLOAT32 afPreProcTempBuf[FRAME_LENGTH];
  X_FLOAT32 afPreEmphPowerSpec[129];

  /*
   * For Pitch and Class extraction
   */

  X_FLOAT32 *pfInpSpeech;
  X_FLOAT32 *pfUBSpeech;
  X_FLOAT32 *pfProcSpeech;
  X_FLOAT32 *pfDownSampledProcSpeech;

  void *pPitchRom, *pPitchEstimator;

  PCORR_DATA pcorrData;

  /*
   * For GSM high-pass filtering to remove low-frequency noise components, e.g., car noises
   */
  
  X_FLOAT32 GSM_HPF_A_Buf[3];
  X_FLOAT32 GSM_HPF_B_Buf[3];
  
  PFFFT_Setup* fft_setup;
};

#define _defined_FEParamsX
#endif

/*----------------------------------------------------
 * Global routines : each call the four stages of the
 * front end (noise suppression, wave processing,
 * cepstrum calculation and post processing), and
 * finally call the VAD stage
 *----------------------------------------------------*/

/*------------
 * allocation
 *------------*/
extern FEParamsX* AdvProcessAlloc (int samplingFrequency);

/*----------------
 * initialisation
 *----------------*/
extern void AdvProcessInit (FEParamsX *pComParX);

/*-------------------------------------------------------------
 * Processing "FrameShift" samples
 * Return TRUE if a frame is output, FALSE if no (because some
 * modules may have some latency, hence at the beginning there
 * is no output data for the first input frames
 *-------------------------------------------------------------*/
extern ETSI_BOOL DoAdvProcess (FILE_TYPE *CurrentFrame, 
							   X_INT16 *OutputBuffer,
							   FEParamsX *pComParX);

/*----------
 * deletion
 *----------*/
extern void AdvProcessDelete (FEParamsX **);

#endif
