/* ====================================================================
 * Copyright (c) 2014 DIOTEK co., ltd. 
 * ALL RIGHTS RESERVED.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are prohibited provided that permissions by DIOTEK co., ltd.
 * are not given.
 *
 * ====================================================================
 *
 */
#ifndef __DNN_BP_STRUCT_H__
#define __DNN_BP_STRUCT_H__


typedef struct{
	short nStage;						///< # of layer pairs == hidden layer num + output layer
	float* delta[MAX_NUM_LAYER];	///< pointer to delta
} DNN_DELTA;

typedef struct{
	short nStage;								///< # of layers == input layer + hidden layer num + output layer
	float* wgrad[MAX_NUM_LAYER];	///< pointer to output, dnn_output[0] should be the pointer to input data
} DNN_WGRAD;


#endif	// __DNN_BP_STRUCT_H__
