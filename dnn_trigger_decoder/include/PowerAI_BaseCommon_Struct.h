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

#ifndef __POWERAI_BASECOMMON_STRUCT_H__
#define __POWERAI_BASECOMMON_STRUCT_H__


#define MAX_NUM_LAYER 30
#define MAX_NUM_STAGE MAX_NUM_LAYER-1
#define MAXSTRLEN 1024
#define PI 3.141592
#define MAX_GPU_THREAD 512


typedef enum DNN_Result {
	SUCCESS,
	FAIL,
} DNN_Result;

/** Nonlinear function type Enum. */
typedef enum DNN_NonLinearUnit {
	SIGMOID,
	RELU,
	SOFTMAX,
	LINEAR
} DNN_NonLinearUnit;

typedef struct DNN_LAYER_UNIT {
	short n_layer;				///< # of layers == input layer + hidden layer num + output layer
	float* unit[MAX_NUM_LAYER];	///< pointer to output, dnn_output[0] should be the pointer to input data
} DNN_LAYER_UNIT;

/** Structure holding layer pair info for RBM pre-training. */
typedef struct {
	short nHidNodes;					///< # of hidden nodes
	short nVisNodes;					///< # of visible nodes
	float* dnnHidBias;					///< pointer to hidden bias
	float* dnnVisBias;					///< pointer to visible bias
	float* dnnWeight;					///< pointer to weights
} DNN_Stage;

/** Structure holding entire DBM. */
typedef struct Deepnet {
	short nStage;							///< # of layer pairs
	DNN_Stage dnnStage[MAX_NUM_STAGE];		///< pointer to rbm layer pair
	DNN_NonLinearUnit nonLinearFunc[MAX_NUM_STAGE];
} Deepnet;

#endif	// __POWERAI_BASECOMMON_STRUCT_H__
