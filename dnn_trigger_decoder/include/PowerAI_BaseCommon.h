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

#ifndef __POWERAI_BASECOMMON_H__
#define __POWERAI_BASECOMMON_H__


#include <stdio.h>
#include <math.h>


#define DNN_sigmoid(x) (float)(1.0f / (1.0f + expf((-1.0f)*(x))))
#define DNN_ReLU(x) (float)(((x)>(0.0f)) ? (x) : (0.0f))
#define DNN_max(x,y) (float)(((x)>(y)) ? (x) : (y))
#define DNN_Sqr(x)        (float)((x)*(x))
#define DNN_SIGM_diff(x)  (float)((x)*(1-(x)))
#define DNN_ReLU_diff(x)  (float)(((x)>(0.0f)) ? (1.0f) : (0.0f))
#define DNN_Sqrt(x)	(float)sqrt(x)

#define HCILAB_PUBLIC

#ifdef _WINDLL
#define POWER_DEEPNET_API __declspec(dllexport)
#else  // if not
#define POWER_DEEPNET_API
#endif 


#ifndef __POWERASR_DNNTRAINER__
#define __POWERASR_DNNTRAINER__

#ifdef __cplusplus
extern "C" {
#endif

#include "PowerAI_BaseCommon_Struct.h"

/// Base Function ///
////yowon 2015-03-20 ///////
float base_rand_fio(int fan_in, int fan_out);

HCILAB_PUBLIC POWER_DEEPNET_API
int base_compare(const void *a , const void *b);

//void svd_weight_dcmp(float *w, int m, int n, int svd_k, float *w1, float *w2);
////yowon 2015-03-20 ///////

HCILAB_PUBLIC POWER_DEEPNET_API
DNN_Result base_getArgumentValue(const char* pszArg, char* pValue, FILE* fpConfig);
HCILAB_PUBLIC POWER_DEEPNET_API
DNN_Result base_getArgumentValueWithoutLowCase(char* pszArg, char* pValue, FILE* fpConfig);

HCILAB_PUBLIC POWER_DEEPNET_API
void base_printDeepnet(Deepnet* pDeepnet, FILE* fpLog);
/// CPU Common Function ///

HCILAB_PUBLIC POWER_DEEPNET_API 
Deepnet* DNN_create(const short num_layer, const short num_nodes[], const DNN_NonLinearUnit nonlinear_func[]);
HCILAB_PUBLIC POWER_DEEPNET_API 
DNN_Result DNN_destroy(Deepnet* pDeepnet);
HCILAB_PUBLIC POWER_DEEPNET_API 
DNN_Result DNN_init(Deepnet* pDeepnet, DNN_NonLinearUnit* nonLinearFunc, int bUseFanInOutInit);

HCILAB_PUBLIC POWER_DEEPNET_API 
DNN_Result DNN_load_dnn(Deepnet* pDeepnet, const char szHomeDir[], const char sz_file_name[]);
HCILAB_PUBLIC POWER_DEEPNET_API
DNN_Result DNN_save_dnn(Deepnet* pDeepnet, short n_stages_to_save, char* sz_file_name);

HCILAB_PUBLIC POWER_DEEPNET_API
DNN_Result do_forward_prop(Deepnet* pDeepnet, DNN_LAYER_UNIT* p_dnn_output);

HCILAB_PUBLIC POWER_DEEPNET_API
DNN_LAYER_UNIT* DNN_create_layer_unit(Deepnet* pDeepnet);
HCILAB_PUBLIC POWER_DEEPNET_API
DNN_Result DNN_init_layer_unit(Deepnet* pDeepnet, DNN_LAYER_UNIT* p_dnn_output);
HCILAB_PUBLIC POWER_DEEPNET_API
DNN_Result DNN_destroy_layer_unit(DNN_LAYER_UNIT* p_dnn_output);

HCILAB_PUBLIC POWER_DEEPNET_API 
DNN_Result DNN_attach_new_layer(Deepnet* pDeepnet, short num_output_nodes);

HCILAB_PUBLIC POWER_DEEPNET_API 
DNN_Result DNN_check_equiv(Deepnet* pDeepnet1, Deepnet* pDeepnet2);

HCILAB_PUBLIC POWER_DEEPNET_API 
Deepnet* DNN_SVD_create(Deepnet* pDeepnet, short* svd_k);
HCILAB_PUBLIC POWER_DEEPNET_API 
DNN_Result DNN_SVD_init(Deepnet* pOriDeepnet, Deepnet* pSvdDeepnet, short* svd_k);
HCILAB_PUBLIC POWER_DEEPNET_API 
DNN_Result DNN_SVD_do_dcmp(Deepnet* pOriDeepnet, Deepnet* pSvdDeepnet, short* svd_k);

/// GPU Common Function ///
#if 0
HCILAB_PUBLIC POWER_DEEPNET_API
void do_gpu_vecElementWise_Sigmoid(unsigned int bSize, unsigned int tSize, float *a, unsigned int size);

HCILAB_PUBLIC POWER_DEEPNET_API
void do_gpu_vecElementWise_ReLU(unsigned int bSize, unsigned int tSize, float *a, unsigned int size);

HCILAB_PUBLIC POWER_DEEPNET_API
void do_gpu_vecConst_Sum(unsigned int bSize, unsigned int tSize, float *a, float c, unsigned int size);

HCILAB_PUBLIC POWER_DEEPNET_API
void do_gpu_vecElementWise_Exp(unsigned int bSize, unsigned int tSize, float *a, unsigned int size, float* max_value);

HCILAB_PUBLIC POWER_DEEPNET_API
void do_gpu_matNormWithVec(unsigned int bSize, unsigned int tSize, float *a, float *b, unsigned int size_h, unsigned int size_v);

HCILAB_PUBLIC POWER_DEEPNET_API
void do_gpu_vecInitWithValue(unsigned int bSize, unsigned int tSize, float *a, float value, unsigned int size);

HCILAB_PUBLIC POWER_DEEPNET_API
void do_gpu_vecElementWise_Sqr(unsigned int bSize, unsigned int tSize, float *a, float*b, unsigned int size);

HCILAB_PUBLIC POWER_DEEPNET_API
void do_gpu_vecElement_UpperCheck(unsigned int bSize, unsigned int tSize, float*a, float upper, unsigned int size);

HCILAB_PUBLIC POWER_DEEPNET_API
void do_gpu_matNormWithVec4SoftMax(unsigned int bSize, unsigned int tSize, float*a, float*b, unsigned int size);


HCILAB_PUBLIC POWER_DEEPNET_API
DNN_Result do_start_gpu(void);

HCILAB_PUBLIC POWER_DEEPNET_API
Deepnet* DNN_GPU_create(Deepnet* pDeepnet);
HCILAB_PUBLIC POWER_DEEPNET_API
DNN_Result DNN_GPU_destroy(Deepnet* pDeepnet);
HCILAB_PUBLIC POWER_DEEPNET_API
DNN_Result DNN_GPU_CopytoDev(Deepnet* pDeepnet1, Deepnet* pDeepnet2);
HCILAB_PUBLIC POWER_DEEPNET_API
DNN_Result DNN_GPU_CopytoHost(Deepnet* pDeepnet_h, Deepnet* pDeepnet_d);


HCILAB_PUBLIC POWER_DEEPNET_API
DNN_Result do_gpu_forward_prop(Deepnet* pDeepnet, DNN_LAYER_UNIT* p_dnn_output, float* mini_one_vec, float* class_one_vec, unsigned int chunk_size);

HCILAB_PUBLIC POWER_DEEPNET_API
DNN_LAYER_UNIT* DNN_GPU_create_layer_unit(Deepnet* pDeepnet, unsigned int chunk_size);
HCILAB_PUBLIC POWER_DEEPNET_API
DNN_Result DNN_GPU_init_layer_unit(Deepnet* pDeepnet, DNN_LAYER_UNIT* p_dnn_output, unsigned int chunk_size);
HCILAB_PUBLIC POWER_DEEPNET_API
DNN_Result DNN_GPU_destroy_layer_unit(Deepnet* pDeepnet, DNN_LAYER_UNIT* p_dnn_output);

HCILAB_PUBLIC POWER_DEEPNET_API
float* DNN_GPU_create_vector(unsigned int size);
HCILAB_PUBLIC POWER_DEEPNET_API
DNN_Result DNN_GPU_copy_vector(float* a, float* b, unsigned int size, int direct);
HCILAB_PUBLIC POWER_DEEPNET_API
DNN_Result DNN_GPU_destory_vector(float* a);
#endif

#ifdef __cplusplus
}
#endif


#endif // __POWERASR_DNNTRAINER__

#endif	// __POWERAI_BASECOMMON_H__
