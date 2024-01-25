/*===============================================================================
 *      ETSI ES 202 212   Distributed Speech Recognition
 *      Extended Advanced Front-End Feature Extraction Algorithm & Compression Algorithm
 *      Speech Reconstruction Algorithm.
 *      C-language software implementation                                      
 *      Version 1.1.1   October, 2003                                            
 *===============================================================================*/
/*-------------------------------------------------------------------------------
 *
 * FILE NAME: NoiseSupExports.h
 * PURPOSE: 1) Apply 2-stage Wiener filter on the input frame.
 *          2) Apply DC offset removal on the output of 2-stage 
 *             Wiener filter.
 *          3) Calculate parameters for the frame dropping VAD (see 
 *             SpeechQSpec(), SpeechQMel(), SpeechQVar()).
 *
 *-------------------------------------------------------------------------------*/
#ifndef _NOISESUPEXPORTS_H
#define _NOISESUPEXPORTS_H

extern NoiseSupStructX *DoNoiseSupAlloc (void) ;
extern void             DoNoiseSupInit (FEParamsX *This) ;
extern ETSI_BOOL        DoNoiseSup (const X_FLOAT32 *InData, X_FLOAT32 *OutData, FEParamsX *This);
extern void             DoNoiseSupDelete (NoiseSupStructX *NSX) ;

#endif
