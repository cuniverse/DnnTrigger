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

#ifndef __DNN_BP_TRAIN_H__
#define __DNN_BP_TRAIN_H__


#ifdef DLLEXPORT  // if DLLEXPORT 상수가 선언
#define POWER_DEEPNET_API __declspec(dllexport)
#else  // if not
#define POWER_DEEPNET_API
#endif 


#ifdef __cplusplus
extern "C" {
#endif


#include "PowerAI_BaseCommon_Struct.h"
#include "PowerASR_DeepNet_struct.h"


POWER_DEEPNET_API
DNN_Result DNN_LoadConfig(DNN_Resource* pDnnResource, const char* szHomeDir, const char* szConfig);
//bp training related
#if 0
DNN_Result do_update_deepnet(Deepnet* pDeepnet, DNN_WGRAD* p_dnn_wgrad_curr, DNN_WGRAD* p_dnn_wgrad_prev, DNN_DELTA* p_dnn_bgrad_curr, DNN_DELTA* p_dnn_bgrad_prev, float lr, float alpha);
DNN_Result do_backward_prop(Deepnet* pDeepnet, DNN_LAYER_UNIT* p_dnn_output, DNN_DELTA* p_dnn_delta);
DNN_Result do_update_grad(Deepnet* pDeepnet, DNN_LAYER_UNIT* p_dnn_output, DNN_DELTA* p_dnn_delta, DNN_WGRAD* p_dnn_wgrad_curr, DNN_DELTA* p_dnn_bgrad_curr);
DNN_Result cal_output_layer_delta(float* target, DNN_LAYER_UNIT* p_dnn_output, int num_class, DNN_DELTA* p_dnn_delta, DNN_NonLinearUnit finalRegType,  DNN_ErrMeasureFunc errMeasureFunc);

DNN_DELTA* DNN_create_delta(Deepnet* pDeepnet);
DNN_WGRAD* DNN_create_wgrad(Deepnet* pDeepnet);
DNN_Result DNN_init_delta(Deepnet* pDeepnet, DNN_DELTA* p_dnn_delta);
DNN_Result DNN_init_wgrad(Deepnet* pDeepnet, DNN_WGRAD* p_dnn_wgrad);
DNN_Result DNN_destroy_delta(Deepnet* pDeepnet, DNN_DELTA* p_dnn_delta);
DNN_Result DNN_destroy_wgrad(Deepnet* pDeepnet, DNN_WGRAD* p_dnn_wgrad);

//dropout train related
DNN_LAYER_UNIT* DNN_create_layer_mask(Deepnet* pDeepnet);
DNN_Result DNN_init_mask_with_prob(Deepnet* pDeepnet, DNN_LAYER_UNIT* p_dnn_mask, float inp_layer_prob, float hid_layer_prob);
DNN_Result DNN_init_mask_with_const(Deepnet* pDeepnet, DNN_LAYER_UNIT* p_dnn_mask, float inp_layer_const, float hid_layer_const);
DNN_Result DNN_destroy_layer_mask(Deepnet* pDeepnet, DNN_LAYER_UNIT* p_dnn_output);
int DNN_rand_binary(float ratio);

DNN_Result do_forward_prop_with_dropout_training(Deepnet* pDeepnet, DNN_LAYER_UNIT* p_dnn_mask, DNN_LAYER_UNIT* p_dnn_output);
DNN_Result do_forward_prop_with_dropout_test(Deepnet* pDeepnet, DNN_LAYER_UNIT* p_dnn_output);
DNN_Result do_backward_prop_with_dropout_training(Deepnet* pDeepnet, DNN_LAYER_UNIT* p_dnn_output, DNN_LAYER_UNIT* p_dnn_mask, DNN_DELTA* p_dnn_delta);

//test phase related
float do_test_all(DNN_Resource* pDnnResource, 
				DNN_TestResource* pTestResource, 
				float* frame_err, 
				int** frame_err_cnt, 
				int* frame_cnt);

float cal_err(float* target, float* pp_dnn_output, int num_class);
int check_answer(float* target, float* pp_dnn_output, int num_class);
void get_max_node_idx(float* result, float* pp_dnn_output, int num_class);
DNN_Result target_gen(int class_idx, int num_class, float* target);
#endif

#ifdef __cplusplus
}
#endif

#endif	// __DNN_BP_TRAIN_H__
