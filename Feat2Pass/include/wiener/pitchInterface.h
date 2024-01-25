/*===============================================================================
 *      ETSI ES 202 212   Distributed Speech Recognition
 *      Extended Advanced Front-End Feature Extraction Algorithm & Compression Algorithm
 *      Speech Reconstruction Algorithm.
 *      C-language software implementation                                      
 *      Version 1.1.1   October, 2003                                            
 *===============================================================================*/
/*-------------------------------------------------------------------------------
 *
 * FILE NAME: pitchInterface.c
 * PURPOSE:   This file contains declaration of pitch estimation API.
 *
 *-------------------------------------------------------------------------------*/



#ifndef _PITCH_INTERFACE
#define _PITCH_INTERFACE


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
 * FUNCTION NAME: InitPitchRom
 *
 * PURPOSE:       Allocates and initializes a pitch ROM data structure
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
		 void       **pPitchRom
		 );



/*----------------------------------------------------------------------------
 * FUNCTION NAME: InitPitchEstimator
 *
 * PURPOSE:       Allocates and initializes a pitch estimator data structure
 *                containing RAM and history information used for pitch estimation. 
 *
 * INPUT:
 *   pPitchRom          Output created by InitPitchRom()
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
		       );


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
						);


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
		 );




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
void DeallocatePitchRom(void *PitchRom);


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
void DeallocatePitchEstimator(void *PitchEstimator);
		



#endif //_PITCH_INTERFACE


