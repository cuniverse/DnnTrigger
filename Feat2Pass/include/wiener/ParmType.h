/*===============================================================================
 *      ETSI ES 202 212   Distributed Speech Recognition
 *      Extended Advanced Front-End Feature Extraction Algorithm & Compression Algorithm
 *      Speech Reconstruction Algorithm.
 *      C-language software implementation                                      
 *      Version 1.1.1   October, 2003                                            
 *===============================================================================*/
/*-------------------------------------------------------------------------------
 *
 * FILE NAME: ParmType.h
 * PURPOSE: Definition of types.
 *
 *-------------------------------------------------------------------------------*/
#ifndef _PARMTYPE_H
#define _PARMTYPE_H

/*-----------------
 * File Inclusions
 *-----------------*/
#include "x_default.h"

/*---------------------
 * Definition of Types 
 *---------------------*/
typedef X_INT16 ETSI_BOOL;

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE (!FALSE)
#endif

#define FILE_TYPE   X_INT16

#endif
