/*===============================================================================
 *      ETSI ES 202 212   Distributed Speech Recognition
 *      Extended Advanced Front-End Feature Extraction Algorithm & Compression Algorithm
 *      Speech Reconstruction Algorithm.
 *      C-language software implementation                                      
 *      Version 1.1.1   October, 2003                                            
 *===============================================================================*/
/*-------------------------------------------------------------------------------
 *
 * FILE NAME: classifyFrame.h
 * PURPOSE:   Declaration of classify_frame() function
 *
 *-------------------------------------------------------------------------------*/

#ifndef CLASSIFY_FRAME_H
#define CLASSIFY_FRAME_H

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
                   X_FLOAT32 *pfInpSpeech, X_FLOAT32 *pfUBSpeech);

#endif

