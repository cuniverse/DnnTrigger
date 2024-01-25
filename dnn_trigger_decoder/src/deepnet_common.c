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

#include <malloc.h>
#include <memory.h>
#include <stdlib.h>

#include "PowerAI_BaseCommon_Struct.h"
#include "PowerAI_BaseCommon.h"


#define DNN_ALN 64

#if __STDC_VERSION__ >= 201112L && defined(_ISOC11_SOURCE)	// after C11
#include <stdalign.h>
#define ALIGNED_(x) alignas(x)
#define ALIGNED_ALLOC(a, s) aligned_alloc(a, s)
#define ALIGNED_FREE(x) free(x)

#elif defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__)) && !defined(__ANDROID__)	// x86 Linux
#include <immintrin.h>
#define ALIGNED_(x) __attribute__ ((aligned (x)))
#define ALIGNED_ALLOC(a, s) _mm_malloc(s, a)
#define ALIGNED_FREE(x) _mm_free(x)

#elif _POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600	// POSIX
#define ALIGNED_(x) __attribute__ ((aligned (x)))
static inline void* ALIGNED_ALLOC(size_t a, size_t s) { void* p; posix_memalign(&p, a, s); return p; }
#define ALIGNED_FREE(x) free(x)

#elif defined(_MSC_VER)	// MSVC
#define ALIGNED_(x) __declspec(align(x))
#define ALIGNED_ALLOC(a, s) _aligned_malloc(s, a)
#define ALIGNED_FREE(x) _aligned_free(x)

#else
#pragma message("No memory align supported? " __FILE__)
#if defined(__GNUC__)
#define ALIGNED_(x) __attribute__ ((aligned (x)))
#else
#define ALIGNED_(x)
#endif	// __GNUC__
#define ALIGNED_ALLOC(a, s) malloc(s)
#define ALIGNED_FREE(x) free(x)

#endif

// set random seed
HCILAB_PUBLIC POWER_DEEPNET_API
DNN_Result DNN_init(Deepnet* pDeepnet, DNN_NonLinearUnit* nonLinearFunc, int bUseFanInOutInit) {
	for (int idx = 0; idx < pDeepnet->nStage; idx++) {
		pDeepnet->nonLinearFunc[idx] = nonLinearFunc[idx];	// TODO: 중복 코드 제거 (DNN_create 함수에서도 수행)

		int n_hid = pDeepnet->dnnStage[idx].nHidNodes;
		int n_vis = pDeepnet->dnnStage[idx].nVisNodes;

		for (int idx_h = 0; idx_h < n_hid; idx_h++) {
			for (int idx_v = 0; idx_v < n_vis; idx_v++) {
				size_t idx_n = idx_h*n_vis + idx_v;
				if (bUseFanInOutInit) {
					pDeepnet->dnnStage[idx].dnnWeight[idx_n] = base_rand_fio(n_vis, n_hid);
				} else {
					int i_rand = rand(); i_rand = i_rand << 15; i_rand = i_rand|rand();
					pDeepnet->dnnStage[idx].dnnWeight[idx_n] = 0.02f * (i_rand / ((float)RAND_MAX*RAND_MAX) - 0.5f);
				}
			}
		}
	}
	return SUCCESS;
}

HCILAB_PUBLIC POWER_DEEPNET_API
DNN_Result DNN_destroy(Deepnet* pDeepnet) {
	if (!pDeepnet)	return FAIL;

	ALIGNED_FREE(pDeepnet->dnnStage[0].dnnVisBias);

	for (int idx = 0; idx<pDeepnet->nStage; idx++){
		ALIGNED_FREE(pDeepnet->dnnStage[idx].dnnHidBias);		
		ALIGNED_FREE(pDeepnet->dnnStage[idx].dnnWeight);
	}
	
	free(pDeepnet);
	return SUCCESS;
}

HCILAB_PUBLIC POWER_DEEPNET_API
DNN_Result DNN_attach_new_layer(Deepnet* pDeepnet, short num_output_nodes) {
	int idx_h=0, idx_v=0;
	short diff_nodes=0;
	DNN_Stage* pDnnStage = &(pDeepnet->dnnStage[pDeepnet->nStage]);
	
	diff_nodes = pDnnStage->nHidNodes-num_output_nodes;
		
	pDnnStage->nHidNodes = num_output_nodes;
	pDnnStage->nVisNodes = pDeepnet->dnnStage[pDeepnet->nStage-1].nHidNodes;
	pDnnStage->dnnVisBias = pDeepnet->dnnStage[pDeepnet->nStage-1].dnnHidBias;
		
	pDnnStage->dnnHidBias = (float*)ALIGNED_ALLOC(DNN_ALN, pDnnStage->nHidNodes*sizeof(float));
	memset(pDnnStage->dnnHidBias, 0, pDnnStage->nHidNodes*sizeof(float));

	pDnnStage->dnnWeight = (float*)ALIGNED_ALLOC(DNN_ALN, pDnnStage->nHidNodes*pDnnStage->nVisNodes*sizeof(float));
	for(idx_h = 0; idx_h< pDnnStage->nHidNodes; idx_h++){	
		for(idx_v =0;idx_v < pDnnStage->nVisNodes; idx_v++){
			pDnnStage->dnnWeight[idx_h*pDnnStage->nVisNodes + idx_v] = base_rand_fio(pDnnStage->nVisNodes, pDnnStage->nHidNodes);
		}
	}
	
	pDeepnet->nStage++;

	return SUCCESS;
}

HCILAB_PUBLIC POWER_DEEPNET_API
DNN_LAYER_UNIT* DNN_create_layer_unit(Deepnet* pDeepnet) {
	int nStage = pDeepnet->nStage;

	DNN_LAYER_UNIT *p_dnn_output = (DNN_LAYER_UNIT*)calloc(1, sizeof(DNN_LAYER_UNIT));
	if (!p_dnn_output)	return NULL;

	p_dnn_output->n_layer = nStage+1;
	
	for(int i = 0; i<nStage; i++){
		p_dnn_output->unit[i+1] = (float*)ALIGNED_ALLOC(DNN_ALN, pDeepnet->dnnStage[i].nHidNodes*sizeof(float));
	}
	return p_dnn_output;
}

DNN_Result DNN_init_layer_unit(Deepnet* pDeepnet, DNN_LAYER_UNIT* p_dnn_output){

	int nStage = pDeepnet->nStage;
	int dnn_output_idx = 0;
	int i = 0;

	//init
	dnn_output_idx++;
	for(i = 0; i<nStage; i++){
		memset(p_dnn_output->unit[dnn_output_idx++],0,pDeepnet->dnnStage[i].nHidNodes*sizeof(float));
	}
	
	return SUCCESS;
}

HCILAB_PUBLIC POWER_DEEPNET_API
DNN_Result DNN_destroy_layer_unit(DNN_LAYER_UNIT* p_dnn_output) {
	for (int i = 1; i < p_dnn_output->n_layer; i++){
		ALIGNED_FREE(p_dnn_output->unit[i]);
	}
	free(p_dnn_output);
	p_dnn_output = NULL;
	return SUCCESS;
}


HCILAB_PUBLIC POWER_DEEPNET_API
DNN_Result do_forward_prop(Deepnet* pDeepnet, DNN_LAYER_UNIT* p_dnn_output) {
	const short n_stage = pDeepnet->nStage;
	DNN_NonLinearUnit* nonLinearFunc = pDeepnet->nonLinearFunc;

	for (int i = 0; i < n_stage; i++) {
		const DNN_Stage* pDnnStage = &pDeepnet->dnnStage[i];
		const int n_hid = (int)pDnnStage->nHidNodes;
		const int n_vis = (int)pDnnStage->nVisNodes;
		const float *input_alt = p_dnn_output->unit[i];
		float *output_alt = p_dnn_output->unit[i+1];

		// 수동 SIMD 최적화 코드 삭제 (2016-06 mckeum) 삭제된 코드는 SVN @6325 이전판을 참조
		for (int idx_h = 0; idx_h < n_hid; idx_h++) {
			float temp = 0.f;
			for (int idx_v = 0; idx_v < n_vis; idx_v++){
				temp += pDnnStage->dnnWeight[idx_h * n_vis + idx_v] * input_alt[idx_v];
			}
			temp += pDnnStage->dnnHidBias[idx_h];

			output_alt[idx_h] = temp;
		}

		switch (nonLinearFunc[i]) {
			case SIGMOID: {
				for (int idx_h = 0; idx_h < n_hid; idx_h++) {
					output_alt[idx_h] = DNN_sigmoid(output_alt[idx_h]);
				}
			} break;
			case RELU: {
				for (int idx_h = 0; idx_h < n_hid; idx_h++) {
					output_alt[idx_h] = DNN_ReLU(output_alt[idx_h]);
				}
			} break;
			case LINEAR: {
				// do nothing
			} break;
			case SOFTMAX: {
				float f_max = 0.f;	//yowon 2015-03-23
				for (int idx_h = 0; idx_h < n_hid; idx_h++) {
					if (output_alt[idx_h] > f_max)
						f_max = output_alt[idx_h];
				}

				float denom = 0.f;
				for (int idx_h = 0; idx_h < n_hid; idx_h++) {
					if (output_alt[idx_h] > f_max-10.0f) {
						output_alt[idx_h] = expf(output_alt[idx_h]-f_max);
					}
					else {
						output_alt[idx_h] = 0;
					}
					denom += output_alt[idx_h];
				}
				denom = 1/denom;
				for (int idx_h = 0; idx_h < n_hid; idx_h++) {
					output_alt[idx_h] *= denom;
				}
			} break;
			default: {
				return FAIL;
			}
		}

	}
	return SUCCESS;
}


DNN_Result DNN_save_dnn(Deepnet* pDeepnet, short n_stages_to_save, char* sz_file_name){

	
	FILE* fpDeepnet;
	int i=0, hid_idx=0;
	short num_stage = n_stages_to_save;
	//file open
	
	fpDeepnet = fopen(sz_file_name,"wb");
	if(!fpDeepnet){
		printf("[ERROR] Cannot open file %s to save DNN data!!!\n",sz_file_name);
		return FAIL;
	}

	//layer pair num save
	fwrite(&(num_stage),sizeof(short),1,fpDeepnet);

	//vbias node num save
	fwrite(&(pDeepnet->dnnStage[0].nVisNodes),sizeof(short),1,fpDeepnet);
	for(i=0;i<num_stage;i++){
		//hbias node num save
		fwrite(&(pDeepnet->dnnStage[i].nHidNodes),sizeof(short),1,fpDeepnet);
	}


	//vbias node save	
	fwrite(pDeepnet->dnnStage[0].dnnVisBias,sizeof(float),pDeepnet->dnnStage[0].nVisNodes,fpDeepnet);
	//1st layer pair weight save
	fwrite(pDeepnet->dnnStage[0].dnnWeight,sizeof(float),pDeepnet->dnnStage[0].nVisNodes*pDeepnet->dnnStage[0].nHidNodes,fpDeepnet);
	//hbias node save
	fwrite(pDeepnet->dnnStage[0].dnnHidBias,sizeof(float),pDeepnet->dnnStage[0].nHidNodes,fpDeepnet);
	
	//second layer pair ...
	for(i=1;i<num_stage;i++){
		//1st layer pair weight save
		
		fwrite(pDeepnet->dnnStage[i].dnnWeight,sizeof(float),pDeepnet->dnnStage[i].nVisNodes*pDeepnet->dnnStage[i].nHidNodes,fpDeepnet);
		
		//hbias node save
		fwrite(pDeepnet->dnnStage[i].dnnHidBias,sizeof(float),pDeepnet->dnnStage[i].nHidNodes,fpDeepnet);
	
	}
	fclose(fpDeepnet);
	printf("\nDNN Save completed! : %s\n",sz_file_name);
	
	return SUCCESS;
}


// Load DNN .dat file
HCILAB_PUBLIC POWER_DEEPNET_API
DNN_Result DNN_load_dnn(Deepnet* pDeepnet, const char szHomeDir[], const char sz_file_name[]) {
	FILE* fpDeepnet;
	short tmp = 0;

	// file open
	fpDeepnet = fopen(sz_file_name, "rb");
	if (!fpDeepnet) {	// retry with relative path
		char temp_path[512];
		sprintf(temp_path, "%s/%s", szHomeDir, sz_file_name);
		fpDeepnet = fopen(temp_path, "rb");
	}

	if (!fpDeepnet) {
		printf("[ERROR] Cannot open file %s to load DNN data!!!\n",sz_file_name);
		return FAIL;
	}

	//layer pair num load
	fread(&(tmp),sizeof(short),1,fpDeepnet);
	if (tmp != pDeepnet->nStage)
	{
		printf("[ERROR] Mismatch between layer nStage setting  %d and %d\n", pDeepnet->nStage, tmp);
		return FAIL;
	}
	//vbias node num load
	fread(&(tmp),sizeof(short),1,fpDeepnet);
	if (tmp != pDeepnet->dnnStage[0].nVisNodes)
	{
		printf("[ERROR] Mismatch between vbias setting %d and %d\n", pDeepnet->dnnStage[0].nVisNodes, tmp);
		return FAIL;
	}
	for (int i = 0; i < pDeepnet->nStage; i++) {
		//hbias node num load
		fread(&(tmp),sizeof(short),1,fpDeepnet);
		if (tmp != pDeepnet->dnnStage[i].nHidNodes)
		{
			printf("[ERROR] Mismatch between hbias setting %d and %d\n", pDeepnet->dnnStage[i].nHidNodes, tmp);
			return FAIL;
		}
	}
	//check duplicated num of nodes
	for (int i = 1; i < pDeepnet->nStage; i++) {
		if (pDeepnet->dnnStage[i].nVisNodes != pDeepnet->dnnStage[i - 1].nHidNodes)
		{
			printf("[ERROR] Duplicated num of nodes nVisNodes %d and i-1 nHidNodes %d\n", pDeepnet->dnnStage[i].nVisNodes, pDeepnet->dnnStage[i - 1].nHidNodes);
			return FAIL;
		}
	}

	//vbias node load	
	fread(pDeepnet->dnnStage[0].dnnVisBias,sizeof(float),pDeepnet->dnnStage[0].nVisNodes,fpDeepnet);
	//1st layer pair weight load
	fread(pDeepnet->dnnStage[0].dnnWeight,sizeof(float),pDeepnet->dnnStage[0].nVisNodes*pDeepnet->dnnStage[0].nHidNodes,fpDeepnet);	
	//hbias node load
	fread(pDeepnet->dnnStage[0].dnnHidBias,sizeof(float),pDeepnet->dnnStage[0].nHidNodes,fpDeepnet);
	
	//second layer pair ...
	for (int i = 1; i < pDeepnet->nStage; i++) {
		//중복 레이어 포인터 연결
		pDeepnet->dnnStage[i].dnnVisBias = pDeepnet->dnnStage[i-1].dnnHidBias;

		//1st layer pair weight load
		fread(pDeepnet->dnnStage[i].dnnWeight,sizeof(float),pDeepnet->dnnStage[i].nVisNodes*pDeepnet->dnnStage[i].nHidNodes,fpDeepnet);
		
		//hbias node load
		fread(pDeepnet->dnnStage[i].dnnHidBias,sizeof(float),pDeepnet->dnnStage[i].nHidNodes,fpDeepnet);
	}
	fclose(fpDeepnet);
	
	return SUCCESS;
}


DNN_Result DNN_check_equiv(Deepnet* pDeepnet1, Deepnet* pDeepnet2){
	
	int i=0;
	int hid_idx=0;

	// num check
	if(pDeepnet1->nStage != pDeepnet2->nStage) {		
		return FAIL;
	}
	for(i=0;i<pDeepnet1->nStage;i++){
		if(pDeepnet1->dnnStage[i].nVisNodes != pDeepnet2->dnnStage[i].nVisNodes)
			return FAIL;
		if(pDeepnet1->dnnStage[i].nHidNodes != pDeepnet2->dnnStage[i].nHidNodes)
			return FAIL;
	}

	//value check
	if(memcmp(pDeepnet1->dnnStage[0].dnnVisBias,pDeepnet2->dnnStage[0].dnnVisBias,pDeepnet1->dnnStage[0].nVisNodes)){
		return FAIL;
	}
	
	if(memcmp(pDeepnet1->dnnStage[0].dnnWeight,pDeepnet2->dnnStage[0].dnnWeight,pDeepnet1->dnnStage[0].nVisNodes*pDeepnet1->dnnStage[0].nHidNodes)){
		return FAIL;
	}
	
	//hbias node save
	if(memcmp(pDeepnet1->dnnStage[0].dnnHidBias,pDeepnet2->dnnStage[0].dnnHidBias,pDeepnet1->dnnStage[0].nHidNodes)){
		return FAIL;
	}

	//second layer pair ...
	for(i=1;i<pDeepnet1->nStage;i++){
		
		//1st layer pair weight check
		for(int kkk=0;kkk<pDeepnet1->dnnStage[i].nVisNodes*pDeepnet1->dnnStage[i].nHidNodes;kkk++){
			if(pDeepnet1->dnnStage[i].dnnWeight[kkk] != pDeepnet2->dnnStage[i].dnnWeight[kkk]){
				return FAIL;
			}
		}

		if(memcmp(pDeepnet1->dnnStage[i].dnnWeight,pDeepnet2->dnnStage[i].dnnWeight,pDeepnet1->dnnStage[i].nVisNodes*pDeepnet1->dnnStage[i].nHidNodes)){
			return FAIL;
		}
		
		//hbias node check
		if(memcmp(pDeepnet1->dnnStage[i].dnnHidBias,pDeepnet2->dnnStage[i].dnnHidBias,pDeepnet1->dnnStage[i].nHidNodes)){
			return FAIL;
		}
	}
	
	return SUCCESS;
}

// allocate memory and set base parameter for DNN model
HCILAB_PUBLIC POWER_DEEPNET_API
Deepnet* DNN_create(const short num_layer, const short num_nodes[], const DNN_NonLinearUnit nonlinear_func[]) {
	Deepnet* pDeepnet = (Deepnet*)calloc(1, sizeof(Deepnet));

	pDeepnet->nStage = num_layer - 1;

	pDeepnet->dnnStage[0].nVisNodes = num_nodes[0];
	pDeepnet->dnnStage[0].nHidNodes = num_nodes[1];

	pDeepnet->dnnStage[0].dnnVisBias = (float*)ALIGNED_ALLOC(DNN_ALN, sizeof(float) * num_nodes[0]);
	pDeepnet->dnnStage[0].dnnHidBias = (float*)ALIGNED_ALLOC(DNN_ALN, sizeof(float) * num_nodes[1]);
	pDeepnet->dnnStage[0].dnnWeight = (float*)ALIGNED_ALLOC(DNN_ALN, sizeof(float) * num_nodes[0] * num_nodes[1]);

	for (int idx = 1; idx < num_layer-1; idx++){

		pDeepnet->dnnStage[idx].nVisNodes = num_nodes[idx];
		pDeepnet->dnnStage[idx].nHidNodes = num_nodes[idx+1];

		pDeepnet->dnnStage[idx].dnnVisBias = pDeepnet->dnnStage[idx-1].dnnHidBias;
		pDeepnet->dnnStage[idx].dnnHidBias = (float*)ALIGNED_ALLOC(DNN_ALN, sizeof(float) * num_nodes[idx+1]);
		pDeepnet->dnnStage[idx].dnnWeight = (float*)ALIGNED_ALLOC(DNN_ALN, sizeof(float) * num_nodes[idx] * num_nodes[idx+1]);
	}

	for (int i = 0; i < pDeepnet->nStage; i++)
		pDeepnet->nonLinearFunc[i] = nonlinear_func[i];

	return pDeepnet;
}

// allocate memory and set base parameter for SVD DNN model
HCILAB_PUBLIC POWER_DEEPNET_API
Deepnet* DNN_SVD_create(Deepnet* pDeepnet, short* svd_k){
	short svd_num_nodes[MAX_NUM_LAYER] = { 0 };
	DNN_NonLinearUnit svd_nonlinear_func[MAX_NUM_LAYER];

	int idx_svd_layer = 0;
	for (int idx_stage = 0; idx_stage < pDeepnet->nStage; idx_stage++) {	//The number of parameters changes from mn to (m+n)k
		svd_num_nodes[idx_svd_layer++] = pDeepnet->dnnStage[idx_stage].nVisNodes;

		if (svd_k[idx_stage] != 0)
			svd_num_nodes[idx_svd_layer++] = svd_k[idx_stage];

		if (idx_stage == pDeepnet->nStage-1)
			svd_num_nodes[idx_svd_layer++] = pDeepnet->dnnStage[idx_stage].nHidNodes;
	}
	short svd_num_layer = idx_svd_layer;

	idx_svd_layer = 0;
	for (int idx_stage = 0; idx_stage < pDeepnet->nStage; idx_stage++) {	//The number of parameters changes from mn to (m+n)k
		if (svd_k[idx_stage] != 0)
			svd_nonlinear_func[idx_svd_layer++] = LINEAR;

		svd_nonlinear_func[idx_svd_layer++] = pDeepnet->nonLinearFunc[idx_stage];
	}

	Deepnet* pSvdDeepnet = DNN_create(svd_num_layer, svd_num_nodes, svd_nonlinear_func);
	return pSvdDeepnet; 
}

DNN_Result DNN_SVD_init(Deepnet* pOriDeepnet, Deepnet* pSvdDeepnet, short* svd_k){
	
	int idx_svd = 0, idx_ori = 0;
	
	memset(pSvdDeepnet->dnnStage[0].dnnVisBias, 0, sizeof(float)*pSvdDeepnet->dnnStage[0].nVisNodes);
	

	for(idx_svd=0;idx_svd<pSvdDeepnet->nStage;){
		
		if(svd_k[idx_ori] == 0){		
			memcpy(pSvdDeepnet->dnnStage[idx_svd].dnnHidBias,pOriDeepnet->dnnStage[idx_ori].dnnHidBias,sizeof(float)*pSvdDeepnet->dnnStage[idx_svd].nHidNodes);
			memset(pSvdDeepnet->dnnStage[idx_svd].dnnWeight, 0, sizeof(float)*pSvdDeepnet->dnnStage[idx_svd].nHidNodes*pSvdDeepnet->dnnStage[idx_svd].nVisNodes);
			
		}else{
			memset(pSvdDeepnet->dnnStage[idx_svd].dnnHidBias, 0, sizeof(float)*pSvdDeepnet->dnnStage[idx_svd].nHidNodes);
			memset(pSvdDeepnet->dnnStage[idx_svd].dnnWeight, 0, sizeof(float)*pSvdDeepnet->dnnStage[idx_svd].nHidNodes*pSvdDeepnet->dnnStage[idx_svd].nVisNodes);
			idx_svd++;
			memcpy(pSvdDeepnet->dnnStage[idx_svd].dnnHidBias,pOriDeepnet->dnnStage[idx_ori].dnnHidBias,sizeof(float)*pSvdDeepnet->dnnStage[idx_svd].nHidNodes);
			memset(pSvdDeepnet->dnnStage[idx_svd].dnnWeight, 0, sizeof(float)*pSvdDeepnet->dnnStage[idx_svd].nHidNodes*pSvdDeepnet->dnnStage[idx_svd].nVisNodes);
		}
		idx_ori++;idx_svd++;
	}

	return SUCCESS;
}

#if 0
DNN_Result DNN_SVD_do_dcmp(Deepnet* pOriDeepnet, Deepnet* pSvdDeepnet, short* svd_k){

	int idx_ori = 0, idx_svd = 0;
	
	idx_ori = 0;
	for(idx_svd=0; idx_svd<pSvdDeepnet->nStage;){
		if(svd_k[idx_ori]==0){
			
			float *pW = pOriDeepnet->dnnStage[idx_ori].dnnWeight;
			float *pW1 = pSvdDeepnet->dnnStage[idx_svd].dnnWeight;
			int m = pOriDeepnet->dnnStage[idx_ori].nVisNodes;
			int n = pOriDeepnet->dnnStage[idx_ori].nHidNodes;
			memcpy(pW1, pW, sizeof(float)*m*n);
			idx_svd++;idx_ori++;
			printf("[Stage %d W %dx%d] -> [Stage %d W %dx%d]\n",idx_ori,m,n,idx_svd,m,n);
			printf("Copy stage %d... [OK]\n\n",idx_svd);
		}else{
			float *pW = pOriDeepnet->dnnStage[idx_ori].dnnWeight;
			float *pW1 = pSvdDeepnet->dnnStage[idx_svd++].dnnWeight;
			float *pW2 = pSvdDeepnet->dnnStage[idx_svd++].dnnWeight;
			int m = pOriDeepnet->dnnStage[idx_ori].nVisNodes;
			int n = pOriDeepnet->dnnStage[idx_ori].nHidNodes;
			int svd_k_ele = svd_k[idx_ori++];
			printf("[Stage %d W %dx%d] -> [Stage %d and %d W %dx%d and %dx%d]\n",idx_ori,m,n,idx_svd-1,idx_svd,m,svd_k_ele,svd_k_ele,n);		
			svd_weight_dcmp(pW, m, n, svd_k_ele, pW1, pW2);
			printf("SVD stage %d to stage %d & stage %d... [OK]\n\n",idx_ori, idx_svd-1, idx_svd);
		}
	}


	return SUCCESS;
}
#else
DNN_Result DNN_SVD_do_dcmp(Deepnet* pOriDeepnet, Deepnet* pSvdDeepnet, short* svd_k){
;
}
#endif
