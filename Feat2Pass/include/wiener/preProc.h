/*===============================================================================
 *      ETSI ES 202 212   Distributed Speech Recognition
 *      Extended Advanced Front-End Feature Extraction Algorithm & Compression Algorithm
 *      Speech Reconstruction Algorithm.
 *      C-language software implementation                                      
 *      Version 1.1.1   October, 2003                                            
 *===============================================================================*/
/*-------------------------------------------------------------------------------
 *
 * FILE NAME: preProc.h
 * PURPOSE:   Declaration of speech signal preprocessing function for pitch 
 *            estimation and voicing classification
 *
 *-------------------------------------------------------------------------------*/

#ifndef PRE_PROC_H
#define PRE_PROC_H



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
                 float *pfDownSampledProcSpeech);





#endif

