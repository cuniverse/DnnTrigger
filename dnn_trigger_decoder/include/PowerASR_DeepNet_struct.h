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

#ifndef __POWERASR_DEEPNET_STRUCT_H__
#define __POWERASR_DEEPNET_STRUCT_H__


typedef struct Deepnet Deepnet;


/** Error Measure function type Enum. */
typedef enum{
	SQRERR,
	CROSS_ENTROPY
} DNN_ErrMeasureFunc;

typedef enum{
	TRAIN,
	TEST,
	SVD,
	MODEL_MAKE
} DNN_TaskType;

/** Structure holding DNN test info. */
typedef struct{
	int epoch;
//	__int64 iter;//original
	long long iter;//yowon 2015-03-20
	char szTestList[MAXSTRLEN];
	char szConfMatLogFile[MAXSTRLEN];
	char szTotErrLogFile[MAXSTRLEN];	
	float tot_err;
	float tot_class_err;
	float tot_obj_err;
} DNN_TestResource;

typedef struct{
	float lr;
	int lrSustainIter;
	float alpha;
	int maxEpoch;
	char szTrainPath[MAXSTRLEN];
	char szTrainList[MAXSTRLEN];
	char szLabelPath[MAXSTRLEN];
	char szLableExt[MAXSTRLEN];
	char szDnnSaveFileName[MAXSTRLEN];
	int miniBatchSize;
	int bUseDropOut;
	int bShuffle;
	int bPrintTestResultToConsole;
	int numErrChkPeriodEveryNIter;
	float lrReduceRatio;
	float minlr;
	DNN_ErrMeasureFunc errMeasureFunc;
	int bUseGPU;	
	int bMaxNorm;
	float maxNormConst;
	int bGradClassNorm;
	int bUseFanInOutInit;
} DNN_TrainParam;

typedef struct{
	short numClass;
	short nFeatDim;
	short concatSize;	// window total n frames (before + current 1 + after)
	short concatBefore;	// window before n frames
	short numLayer;
	short numNodes[MAX_NUM_LAYER];
	DNN_NonLinearUnit nonLinearFunc[MAX_NUM_STAGE];
} DNN_StructParam;


#if 0
/** Structure holding DNN train info. */
typedef struct{
	DNN_TaskType taskType;
	Deepnet* pDeepnet; 
	Deepnet* pDeepnet_gpu;
	short svd_k[MAX_NUM_STAGE];
	int bUseSeedDnn;
	char szSeedDnnFile[MAXSTRLEN];
	DNN_StructParam dnnStructParam;
	DNN_TrainParam	dnnTrainParam;
	DNN_TestResource testDevResource;
	DNN_TestResource testValiResource;
	DNN_LAYER_UNIT* p_dnn_output;
	float* mini_one_vec;
	float* class_one_vec;
} DNN_Resource;
#else
typedef struct DNN_Resource {
	DNN_TaskType taskType;
	Deepnet* pDeepnet; 
	Deepnet* pDeepnet_gpu;
	short svd_k[MAX_NUM_STAGE];
	int bUseSeedDnn;
	char szSeedDnnFile[MAXSTRLEN];
	DNN_StructParam dnnStructParam;
	DNN_TrainParam	dnnTrainParam;
	DNN_TestResource testDevResource;
	DNN_TestResource testValiResource;
//	DNN_LAYER_UNIT* p_dnn_output;
	float* mini_one_vec;
	float* class_one_vec;
} DNN_Resource;
#endif

#endif	// __POWERASR_DEEPNET_STRUCT_H__
