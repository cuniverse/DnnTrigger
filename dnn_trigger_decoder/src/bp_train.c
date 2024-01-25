#include <stdlib.h>
#include <string.h>

#include "PowerAI_BaseCommon_Struct.h"
#include "PowerAI_BaseCommon.h"

#include "PowerASR_DeepNet_struct.h"
//#include "PowerASR_TrainDeepNet.h"
#include "bp_struct.h"

//CPU
#include "bp_train.h"


POWER_DEEPNET_API
DNN_Result DNN_LoadConfig(DNN_Resource* pDnnResource, const char* szHomeDir, const char* szConfig)       // .ini 설정 초기화
{
	int idx_layer = 0, idx_stage = 0;
	FILE* fpConfig = 0;
	char szArg[MAXSTRLEN] = {0};
	char szValue[MAXSTRLEN] = {0};

	if (!pDnnResource) { puts("Null DNN resource!"); return FAIL; }

	fpConfig = fopen(szConfig, "rt");
	if (!fpConfig) {	// try to open at absolute path
		sprintf(szValue, "%s/%s", szHomeDir, szConfig);
		fpConfig = fopen(szValue, "rt");
	}

	if (!fpConfig) { printf(" (Cannot open config file: %s) ", szConfig); return FAIL; }
	
	sprintf(szArg,"TASK_TYPE");
	if(base_getArgumentValue(szArg,szValue,fpConfig) != SUCCESS) goto CONFIG_FAIL;
	if(!strcmp(szValue,"train")) pDnnResource->taskType = TRAIN;
	else if(!strcmp(szValue,"test")) pDnnResource->taskType = TEST;
	else if(!strcmp(szValue,"svd")) pDnnResource->taskType = SVD;
	else if(!strcmp(szValue,"model_make")) pDnnResource->taskType = MODEL_MAKE;//yowon 2015-05-12
	else {goto CONFIG_FAIL;}

	
	sprintf(szArg,"FEAT_DIM");
	if(base_getArgumentValue(szArg,szValue,fpConfig) != SUCCESS) goto CONFIG_FAIL;
	{
		int tmp = atoi(szValue);
		if(tmp == 0) goto CONFIG_FAIL;
		pDnnResource->dnnStructParam.nFeatDim = tmp;
	}
		

	sprintf(szArg,"FRAME_CONCAT_BEFORE");
	if(base_getArgumentValue(szArg,szValue,fpConfig) != SUCCESS) goto CONFIG_FAIL;
	{
		int tmp = atoi(szValue);
		if(tmp == 0) goto CONFIG_FAIL;
		pDnnResource->dnnStructParam.concatBefore = tmp;
	}
		
	sprintf(szArg, "FRAME_CONCAT_AFTER");
	if (base_getArgumentValue(szArg, szValue, fpConfig) != SUCCESS) goto CONFIG_FAIL;
	{
		int tmp = atoi(szValue);
		if (tmp == 0) goto CONFIG_FAIL;
		pDnnResource->dnnStructParam.concatSize = pDnnResource->dnnStructParam.concatBefore + tmp + 1;

		// input node
		pDnnResource->dnnStructParam.numNodes[idx_layer++] = pDnnResource->dnnStructParam.nFeatDim * pDnnResource->dnnStructParam.concatSize;
	}
		
	sprintf(szArg,"NUM_HID_NODES");
	if(base_getArgumentValue(szArg,szValue,fpConfig) != SUCCESS) goto CONFIG_FAIL;
	{
		char sz_Num[10] = {0};
		char* psz_Val = szValue;
		int idx = 0, idx_in=0;
		for(idx = 0; idx<=strlen(szValue); idx++){
			if(szValue[idx] >= 48 && szValue[idx] <= 57){
				sz_Num[idx_in++] = szValue[idx];
			}else{
				int tmp = 0;
				sz_Num[idx_in] = 0;
				idx_in = 0;
				tmp = atoi(sz_Num);
				if(tmp == 0) goto CONFIG_FAIL;
				pDnnResource->dnnStructParam.numNodes[idx_layer++] = tmp;
			}
		}


	}
		
	sprintf(szArg,"NUM_CLASS");
	if(base_getArgumentValue(szArg,szValue,fpConfig) != SUCCESS) goto CONFIG_FAIL;
	{
		int tmp = atoi(szValue);
		if(tmp == 0) goto CONFIG_FAIL;
		pDnnResource->dnnStructParam.numClass = tmp;
		pDnnResource->dnnStructParam.numNodes[idx_layer++] = tmp;
	}
		
	sprintf(szArg,"NUM_LAYER");
	if(base_getArgumentValue(szArg,szValue,fpConfig) != SUCCESS) goto CONFIG_FAIL;
	{
		int tmp = atoi(szValue);
		if(tmp == 0) goto CONFIG_FAIL;
		if(idx_layer != tmp) goto CONFIG_FAIL;
	}

	sprintf(szArg,"NON_LINEAR_FUNC");
	if(base_getArgumentValue(szArg,szValue,fpConfig) != SUCCESS) goto CONFIG_FAIL;
	{
		char sz_Func[10] = {0};
		char* psz_Val = szValue;
		int idx = 0, idx_in=0;
		int softmax_flag = 0;
		for(idx = 0; idx<=strlen(szValue); idx++){
			if(szValue[idx] >= 97 && szValue[idx] <= 122){
				sz_Func[idx_in++] = szValue[idx];
			}else{
				int tmp = 0;
				sz_Func[idx_in] = 0;
				idx_in = 0;
				if(!strcmp(sz_Func, "sigmoid")){						
					if(softmax_flag == 0) pDnnResource->dnnStructParam.nonLinearFunc[idx_stage++] = SIGMOID;
					else goto CONFIG_FAIL;						
				}else if(!strcmp(sz_Func, "relu")){
					if(softmax_flag == 0) pDnnResource->dnnStructParam.nonLinearFunc[idx_stage++] = RELU;
					else goto CONFIG_FAIL;
					if(idx_stage == idx_layer-1) goto CONFIG_FAIL;
				}else if(!strcmp(sz_Func, "softmax")){
					pDnnResource->dnnStructParam.nonLinearFunc[idx_stage++] = SOFTMAX;
					softmax_flag = 1;
				}else if(!strcmp(sz_Func, "linear")){						
					if(softmax_flag == 0) pDnnResource->dnnStructParam.nonLinearFunc[idx_stage++] = LINEAR;
					else goto CONFIG_FAIL;
					if(idx_stage == idx_layer-1) goto CONFIG_FAIL;
				}else{
					goto CONFIG_FAIL;
				}					
					
			}
		}
		if(idx_stage + 1 != idx_layer){
			goto CONFIG_FAIL;
		}
		pDnnResource->dnnStructParam.numLayer = idx_layer;

	}



	switch (pDnnResource->taskType){
	case TRAIN:
#if 0		
		sprintf(szArg,"USE_GPU");
		if(base_getArgumentValue(szArg,szValue,fpConfig) != SUCCESS) 
		{printf("Default %s option = yes\n",szArg); pDnnResource->dnnTrainParam.bUseGPU = 1;}
		else{
			if(!strcmp(szValue,"yes")) {
				pDnnResource->dnnTrainParam.bUseGPU = 1;
			}else if(!strcmp(szValue,"no")){
				pDnnResource->dnnTrainParam.bUseGPU = 0;
			}else{
				goto CONFIG_FAIL;
			}
		}

	
		sprintf(szArg,"MOMENTUM_ALPHA");
		if(base_getArgumentValue(szArg,szValue,fpConfig) != SUCCESS) goto CONFIG_FAIL;
		{
			float tmp = atof(szValue);		
			pDnnResource->dnnTrainParam.alpha = tmp;
		}

		sprintf(szArg,"USE_FANINOUT_INIT");
		if(base_getArgumentValue(szArg,szValue,fpConfig) != SUCCESS)
		{printf("Default %s option = yes\n",szArg); pDnnResource->dnnTrainParam.bUseFanInOutInit = 1;}
		else{		
			if(!strcmp(szValue,"yes")) {
				pDnnResource->dnnTrainParam.bUseFanInOutInit = 1;
			}else if(!strcmp(szValue,"no")){
				pDnnResource->dnnTrainParam.bUseFanInOutInit = 0;
			}else{
				goto CONFIG_FAIL;
			}
		}
	
		sprintf(szArg,"DO_SHUFFLE");
		if(base_getArgumentValue(szArg,szValue,fpConfig) != SUCCESS) 
		{printf("Default %s option = yes\n",szArg); pDnnResource->dnnTrainParam.bShuffle = 1;}
		else{		
			if(!strcmp(szValue,"yes")) {
				pDnnResource->dnnTrainParam.bShuffle = 1;
			}else if(!strcmp(szValue,"no")){
				pDnnResource->dnnTrainParam.bShuffle = 0;
			}else{
				goto CONFIG_FAIL;
			}
		}
	
		sprintf(szArg,"USE_DROPOUT");
		if(base_getArgumentValue(szArg,szValue,fpConfig) != SUCCESS) 
		{printf("Default %s option = no\n",szArg); pDnnResource->dnnTrainParam.bUseDropOut = 0;}
		else{
			if(!strcmp(szValue,"yes")) {
				pDnnResource->dnnTrainParam.bUseDropOut = 1;
			}else if(!strcmp(szValue,"no")){
				pDnnResource->dnnTrainParam.bUseDropOut = 0;
			}else{
				goto CONFIG_FAIL;
			}
		}
	
		sprintf(szArg,"ERR_CHK_LR_CHANGE_ITER");
		if(base_getArgumentValue(szArg,szValue,fpConfig) != SUCCESS) goto CONFIG_FAIL;
		{
			int tmp = atoi(szValue);
			if(tmp == 0) goto CONFIG_FAIL;
			pDnnResource->dnnTrainParam.numErrChkPeriodEveryNIter = tmp;
		}
	
		sprintf(szArg,"LR");
		if(base_getArgumentValue(szArg,szValue,fpConfig) != SUCCESS) goto CONFIG_FAIL;
		{
			float tmp = atof(szValue);
			if(tmp < 0 || tmp > 1) goto CONFIG_FAIL;
			pDnnResource->dnnTrainParam.lr = tmp;
		}
	
		sprintf(szArg,"LR_SUSTAIN_ITER");
		if(base_getArgumentValue(szArg,szValue,fpConfig) != SUCCESS) goto CONFIG_FAIL;
		{
			int tmp = atoi(szValue);
			if(tmp < 0) goto CONFIG_FAIL;
			pDnnResource->dnnTrainParam.lrSustainIter = tmp;
		}
	
		sprintf(szArg,"LR_REDUCE_RATIO");
		if(base_getArgumentValue(szArg,szValue,fpConfig) != SUCCESS) goto CONFIG_FAIL;
		{
			float tmp = atof(szValue);
			if(tmp < 0 || tmp > 1) goto CONFIG_FAIL;
			pDnnResource->dnnTrainParam.lrReduceRatio = tmp;
		}
	
		sprintf(szArg,"MIN_LR");
		if(base_getArgumentValue(szArg,szValue,fpConfig) != SUCCESS) goto CONFIG_FAIL;
		{
			float tmp = atof(szValue);
			if(tmp < 0 || tmp > 1) goto CONFIG_FAIL;
			pDnnResource->dnnTrainParam.minlr = tmp;
		}
	
		sprintf(szArg,"MAX_EPOCH");
		if(base_getArgumentValue(szArg,szValue,fpConfig) != SUCCESS) goto CONFIG_FAIL;
		{
			int tmp = atoi(szValue);
			if(tmp == 0) goto CONFIG_FAIL;
			pDnnResource->dnnTrainParam.maxEpoch = tmp;
		}
	
		sprintf(szArg,"MINI_BATCH_SIZE");
		if(base_getArgumentValue(szArg,szValue,fpConfig) != SUCCESS) goto CONFIG_FAIL;
		{
			int tmp = atoi(szValue);
			if(tmp == 0) goto CONFIG_FAIL;
			pDnnResource->dnnTrainParam.miniBatchSize = tmp;
		}
		
		if(pDnnResource->dnnTrainParam.bUseGPU == 1){
			sprintf(szArg,"USE_W_MAX_NORM");
			if(base_getArgumentValue(szArg,szValue,fpConfig) != SUCCESS) goto CONFIG_FAIL;
			{
				if(!strcmp(szValue,"yes")) {
					pDnnResource->dnnTrainParam.bMaxNorm = 1;
					sprintf(szArg,"W_MAX_NORM_CONST");
					if(base_getArgumentValue(szArg,szValue,fpConfig) != SUCCESS) goto CONFIG_FAIL;
					{
						float tmp = atof(szValue);
						if(tmp < 0) goto CONFIG_FAIL;
						pDnnResource->dnnTrainParam.maxNormConst = tmp;
					}

				}else if(!strcmp(szValue,"no")){
					pDnnResource->dnnTrainParam.bMaxNorm = 0;
				}else{
					goto CONFIG_FAIL;
				}
			}
		}
		
		if(pDnnResource->dnnTrainParam.bUseGPU == 1){
			sprintf(szArg,"USE_GRAD_CLASS_NORM");
			if(base_getArgumentValue(szArg,szValue,fpConfig) != SUCCESS) goto CONFIG_FAIL;
			{
				if(!strcmp(szValue,"yes")) {
					pDnnResource->dnnTrainParam.bGradClassNorm = 1;					
				}else if(!strcmp(szValue,"no")){
					pDnnResource->dnnTrainParam.bGradClassNorm = 0;
				}else{
					goto CONFIG_FAIL;
				}
			}
		}
		
		sprintf(szArg,"DATA_PATH");
		if(base_getArgumentValue(szArg,szValue,fpConfig) != SUCCESS) goto CONFIG_FAIL;
		{
			char tmp[MAXSTRLEN] = {0};
			sprintf(tmp,"%s",szValue);
			if(tmp[0] == 0) goto CONFIG_FAIL;
			if(tmp[strlen(tmp)-1] != '\\' && tmp[strlen(tmp)-1] != '/') {tmp[strlen(tmp)] = '/'; tmp[strlen(tmp)+1] = 0;}
			sprintf(pDnnResource->dnnTrainParam.szTrainPath, "%s", tmp);
		}
	
		sprintf(szArg,"TRAIN_LIST_FILE");
		if(base_getArgumentValue(szArg,szValue,fpConfig) != SUCCESS) goto CONFIG_FAIL;
		{
			if(szValue[0] == 0) goto CONFIG_FAIL;
			sprintf(pDnnResource->dnnTrainParam.szTrainList, "%s", szValue);
		}
	
		sprintf(szArg,"LABEL_PATH");
		if(base_getArgumentValue(szArg,szValue,fpConfig) != SUCCESS) goto CONFIG_FAIL;
		{
			char tmp[MAXSTRLEN] = {0};
			sprintf(tmp,"%s",szValue);
			if(tmp[0] == 0) goto CONFIG_FAIL;
			if(tmp[strlen(tmp)-1] != '\\' && tmp[strlen(tmp)-1] != '/') {tmp[strlen(tmp)] = '/'; tmp[strlen(tmp)+1] = 0;}
			sprintf(pDnnResource->dnnTrainParam.szLabelPath, "%s", tmp);
		}
	
		sprintf(szArg,"LABEL_EXT");
		if(base_getArgumentValue(szArg,szValue,fpConfig) != SUCCESS) goto CONFIG_FAIL;
		{
			if(szValue[0] == 0) goto CONFIG_FAIL;
			sprintf(pDnnResource->dnnTrainParam.szLableExt, "%s", szValue);
		}
	
		sprintf(szArg,"DNN_SAVE_FILE_NAME");
		if(base_getArgumentValue(szArg,szValue,fpConfig) != SUCCESS) goto CONFIG_FAIL;
		{
			if(szValue[0] == 0) goto CONFIG_FAIL;
			sprintf(pDnnResource->dnnTrainParam.szDnnSaveFileName, "%s", szValue);
		}
	
		sprintf(szArg,"DO_PRINT_TEST_RESULT_TO_CONSOLE");
		if(base_getArgumentValue(szArg,szValue,fpConfig) != SUCCESS) 
		{printf("Default %s option = no\n",szArg); pDnnResource->dnnTrainParam.bPrintTestResultToConsole = 0;}
		else{
			if(!strcmp(szValue,"yes")) {
				pDnnResource->dnnTrainParam.bPrintTestResultToConsole = 1;
			}else if(!strcmp(szValue,"no")){
				pDnnResource->dnnTrainParam.bPrintTestResultToConsole = 0;
			}else{
				goto CONFIG_FAIL;
			}
		}
	
		sprintf(szArg,"ERR_MEASURE_FUNC");
		if(base_getArgumentValue(szArg,szValue,fpConfig) != SUCCESS) goto CONFIG_FAIL;
		{
			if(!strcmp(szValue,"sqrerr")) {
				pDnnResource->dnnTrainParam.errMeasureFunc = SQRERR;
			}else if(!strcmp(szValue,"cross_entropy")){
				pDnnResource->dnnTrainParam.errMeasureFunc = CROSS_ENTROPY;
			}else{
				goto CONFIG_FAIL;
			}
		}
#endif
/*
		sprintf(szArg,"USE_SEED_DNN");
		if(base_getArgumentValue(szArg,szValue,fpConfig) != SUCCESS) goto CONFIG_FAIL;
		{
			if(!strcmp(szValue,"yes")) {
				pDnnResource->bUseSeedDnn = 1;
			}else if(!strcmp(szValue,"no")){
				pDnnResource->bUseSeedDnn = 0;
			}else{
				goto CONFIG_FAIL;
			}
		}

		base_getArgumentValue("SEED_DNN_FILE", szValue, fpConfig);
		if (pDnnResource->bUseSeedDnn && szValue[0] == '\0')	goto CONFIG_FAIL;
		sprintf(pDnnResource->szSeedDnnFile, "%s", szValue);
*/
		sprintf(szArg,"USE_SEED_DNN");
		if(base_getArgumentValue(szArg,szValue,fpConfig) != SUCCESS) goto CONFIG_FAIL;
		{
			if(!strcmp(szValue,"yes")) {
				pDnnResource->bUseSeedDnn = 1;

			}else if(!strcmp(szValue,"no")){
				pDnnResource->bUseSeedDnn = 0;
			}else{
				goto CONFIG_FAIL;
			}
		}

		sprintf(szArg,"SEED_DNN_FILE");
		if (base_getArgumentValueWithoutLowCase(szArg, szValue, fpConfig) != SUCCESS) goto CONFIG_FAIL;
		{
			sprintf(pDnnResource->szSeedDnnFile, "%s", szValue);
			if(pDnnResource->bUseSeedDnn && szValue[0] == 0) goto CONFIG_FAIL;
		}

		sprintf(szArg,"DEV_TEST_LIST");
		if(base_getArgumentValue(szArg,szValue,fpConfig) != SUCCESS) goto CONFIG_FAIL;
		{
			if(szValue[0] == 0) goto CONFIG_FAIL;
			sprintf(pDnnResource->testDevResource.szTestList, "%s", szValue);
		}
		sprintf(szArg,"DEV_CONFMAT");
		if(base_getArgumentValue(szArg,szValue,fpConfig) != SUCCESS) goto CONFIG_FAIL;
		{
			if(szValue[0] == 0) goto CONFIG_FAIL;
			sprintf(pDnnResource->testDevResource.szConfMatLogFile, "%s", szValue);
		}
		sprintf(szArg,"DEV_TOT_ERR_LOG");
		if(base_getArgumentValue(szArg,szValue,fpConfig) != SUCCESS) goto CONFIG_FAIL;
		{
			if(szValue[0] == 0) goto CONFIG_FAIL;
			sprintf(pDnnResource->testDevResource.szTotErrLogFile, "%s", szValue);
		}
		sprintf(szArg,"VALI_TEST_LIST");
		if(base_getArgumentValue(szArg,szValue,fpConfig) != SUCCESS) goto CONFIG_FAIL;
		{
			if(szValue[0] == 0) goto CONFIG_FAIL;
			sprintf(pDnnResource->testValiResource.szTestList, "%s", szValue);
		}
		sprintf(szArg,"VALI_CONFMAT");
		if(base_getArgumentValue(szArg,szValue,fpConfig) != SUCCESS) goto CONFIG_FAIL;
		{
			if(szValue[0] == 0) goto CONFIG_FAIL;
			sprintf(pDnnResource->testValiResource.szConfMatLogFile, "%s", szValue);
		}
		sprintf(szArg,"VALI_TOT_ERR_LOG");
		if(base_getArgumentValue(szArg,szValue,fpConfig) != SUCCESS) goto CONFIG_FAIL;
		{
			if(szValue[0] == 0) goto CONFIG_FAIL;
			sprintf(pDnnResource->testValiResource.szTotErrLogFile, "%s", szValue);
		}
		break;
	case TEST:
		sprintf(szArg,"USE_DROPOUT");
		if(base_getArgumentValue(szArg,szValue,fpConfig) != SUCCESS) goto CONFIG_FAIL;
		{
			if(!strcmp(szValue,"yes")) {
				pDnnResource->dnnTrainParam.bUseDropOut = 1;
			}else if(!strcmp(szValue,"no")){
				pDnnResource->dnnTrainParam.bUseDropOut = 0;
			}else{
				goto CONFIG_FAIL;
			}
		}

		sprintf(szArg,"DATA_PATH");
		if(base_getArgumentValue(szArg,szValue,fpConfig) != SUCCESS) goto CONFIG_FAIL;
		{
			char tmp[MAXSTRLEN] = {0};
			sprintf(tmp,"%s",szValue);
			if(tmp[0] == 0) goto CONFIG_FAIL;
			if(tmp[strlen(tmp)-1] != '\\' && tmp[strlen(tmp)-1] != '/') {tmp[strlen(tmp)] = '/'; tmp[strlen(tmp)+1] = 0;}
			sprintf(pDnnResource->dnnTrainParam.szTrainPath, "%s", tmp);
		}
	
		sprintf(szArg,"LABEL_PATH");
		if(base_getArgumentValue(szArg,szValue,fpConfig) != SUCCESS) goto CONFIG_FAIL;
		{
			char tmp[MAXSTRLEN] = {0};
			sprintf(tmp,"%s",szValue);
			if(tmp[0] == 0) goto CONFIG_FAIL;
			if(tmp[strlen(tmp)-2] != '\\' ||tmp[strlen(tmp)-2] != '/') {tmp[strlen(tmp)-1] = '/'; tmp[strlen(tmp)] = 0;}
			sprintf(pDnnResource->dnnTrainParam.szLabelPath, "%s", tmp);
		}
	
		sprintf(szArg,"LABEL_EXT");
		if(base_getArgumentValue(szArg,szValue,fpConfig) != SUCCESS) goto CONFIG_FAIL;
		{
			if(szValue[0] == 0) goto CONFIG_FAIL;
			sprintf(pDnnResource->dnnTrainParam.szLableExt, "%s", szValue);
		}
	
		sprintf(szArg,"DO_PRINT_TEST_RESULT_TO_CONSOLE");
		if(base_getArgumentValue(szArg,szValue,fpConfig) != SUCCESS) goto CONFIG_FAIL;
		{
			if(!strcmp(szValue,"yes")) {
				pDnnResource->dnnTrainParam.bPrintTestResultToConsole = 1;
			}else if(!strcmp(szValue,"no")){
				pDnnResource->dnnTrainParam.bPrintTestResultToConsole = 0;
			}else{
				goto CONFIG_FAIL;
			}
		}
	
		sprintf(szArg,"USE_GPU");
		if(base_getArgumentValue(szArg,szValue,fpConfig) != SUCCESS) goto CONFIG_FAIL;
		{
			if(!strcmp(szValue,"yes")) {
				pDnnResource->dnnTrainParam.bUseGPU = 1;
			}else if(!strcmp(szValue,"no")){
				pDnnResource->dnnTrainParam.bUseGPU = 0;
			}else{
				goto CONFIG_FAIL;
			}
		}

		sprintf(szArg,"USE_SEED_DNN");
		if(base_getArgumentValue(szArg,szValue,fpConfig) != SUCCESS) goto CONFIG_FAIL;
		{
			if(!strcmp(szValue,"yes")) {
				pDnnResource->bUseSeedDnn = 1;
				sprintf(szArg,"SEED_DNN_FILE");
				if(base_getArgumentValue(szArg,szValue,fpConfig) != SUCCESS) goto CONFIG_FAIL;
				{
					if(szValue[0] == 0) goto CONFIG_FAIL;
					sprintf(pDnnResource->szSeedDnnFile, "%s", szValue);
				}
			}else if(!strcmp(szValue,"no")){				
				goto CONFIG_FAIL;				
			}else{
				goto CONFIG_FAIL;
			}
		}

		sprintf(szArg,"VALI_TEST_LIST");
		if(base_getArgumentValue(szArg,szValue,fpConfig) != SUCCESS) goto CONFIG_FAIL;
		{
			if(szValue[0] == 0) goto CONFIG_FAIL;
			sprintf(pDnnResource->testValiResource.szTestList, "%s", szValue);
		}
		sprintf(szArg,"VALI_CONFMAT");
		if(base_getArgumentValue(szArg,szValue,fpConfig) != SUCCESS) goto CONFIG_FAIL;
		{
			if(szValue[0] == 0) goto CONFIG_FAIL;
			sprintf(pDnnResource->testValiResource.szConfMatLogFile, "%s", szValue);
		}
		sprintf(szArg,"VALI_TOT_ERR_LOG");
		if(base_getArgumentValue(szArg,szValue,fpConfig) != SUCCESS) goto CONFIG_FAIL;
		{
			if(szValue[0] == 0) goto CONFIG_FAIL;
			sprintf(pDnnResource->testValiResource.szTotErrLogFile, "%s", szValue);
		}

		break;
	case SVD:
//		int idx_layer_svd = 1, idx_stage_svd = 0;//org
		sprintf(szArg,"SVD_K");		
		if(base_getArgumentValue(szArg,szValue,fpConfig) != SUCCESS) goto CONFIG_FAIL;
		{
			int idx_layer_svd = 1, idx_stage_svd = 0;//yowon 2015-05-12

			char sz_Num[10] = {0};
			char* psz_Val = szValue;
			int idx = 0, idx_in=0;
			for(idx = 0; idx<=strlen(szValue); idx++){
				if(szValue[idx] >= 48 && szValue[idx] <= 57){
					sz_Num[idx_in++] = szValue[idx];
				}else{
					int tmp = 0;
					sz_Num[idx_in] = 0;
					idx_in = 0;
					tmp = atoi(sz_Num);
					if(idx_layer_svd > idx_layer) goto CONFIG_FAIL;
					if(tmp > pDnnResource->dnnStructParam.numNodes[idx_layer_svd++]) goto CONFIG_FAIL;
					pDnnResource->svd_k[idx_stage_svd++] = tmp;
				}
			}

			if(idx_layer_svd != idx_layer) goto CONFIG_FAIL;
		}

		
		sprintf(szArg,"DNN_SAVE_FILE_NAME");
		if(base_getArgumentValue(szArg,szValue,fpConfig) != SUCCESS) goto CONFIG_FAIL;
		{
			if(szValue[0] == 0) goto CONFIG_FAIL;
			sprintf(pDnnResource->dnnTrainParam.szDnnSaveFileName, "%s", szValue);
		}
		
		sprintf(szArg,"USE_SEED_DNN");
		if(base_getArgumentValue(szArg,szValue,fpConfig) != SUCCESS) goto CONFIG_FAIL;
		{
			if(!strcmp(szValue,"yes")) {
				pDnnResource->bUseSeedDnn = 1;
				sprintf(szArg,"SEED_DNN_FILE");
				if(base_getArgumentValue(szArg,szValue,fpConfig) != SUCCESS) goto CONFIG_FAIL;
				{
					if(szValue[0] == 0) goto CONFIG_FAIL;
					sprintf(pDnnResource->szSeedDnnFile, "%s", szValue);
				}
			}else if(!strcmp(szValue,"no")){				
				goto CONFIG_FAIL;				
			}else{
				goto CONFIG_FAIL;
			}

			
		}
		break;

//yowon 2015-05-12 기존 dnn 모델에서 output 노드 갯수만 변경된 경우
	case MODEL_MAKE: 
		sprintf(szArg,"USE_GPU");
		if(base_getArgumentValue(szArg,szValue,fpConfig) != SUCCESS) 
		{printf("Default %s option = yes\n",szArg); pDnnResource->dnnTrainParam.bUseGPU = 1;}
		else{
			if(!strcmp(szValue,"yes")) {
				pDnnResource->dnnTrainParam.bUseGPU = 1;
			}else if(!strcmp(szValue,"no")){
				pDnnResource->dnnTrainParam.bUseGPU = 0;
			}else{
				goto CONFIG_FAIL;
			}
		}
	
		if(pDnnResource->dnnTrainParam.bUseGPU == 1){
			sprintf(szArg,"USE_W_MAX_NORM");
			if(base_getArgumentValue(szArg,szValue,fpConfig) != SUCCESS) goto CONFIG_FAIL;
			{
				if(!strcmp(szValue,"yes")) {
					pDnnResource->dnnTrainParam.bMaxNorm = 1;
					sprintf(szArg,"W_MAX_NORM_CONST");
					if(base_getArgumentValue(szArg,szValue,fpConfig) != SUCCESS) goto CONFIG_FAIL;
					{
						float tmp = atof(szValue);
						if(tmp < 0) goto CONFIG_FAIL;
						pDnnResource->dnnTrainParam.maxNormConst = tmp;
					}

				}else if(!strcmp(szValue,"no")){
					pDnnResource->dnnTrainParam.bMaxNorm = 0;
				}else{
					goto CONFIG_FAIL;
				}
			}
		}
		
		if(pDnnResource->dnnTrainParam.bUseGPU == 1){
			sprintf(szArg,"USE_GRAD_CLASS_NORM");
			if(base_getArgumentValue(szArg,szValue,fpConfig) != SUCCESS) goto CONFIG_FAIL;
			{
				if(!strcmp(szValue,"yes")) {
					pDnnResource->dnnTrainParam.bGradClassNorm = 1;					
				}else if(!strcmp(szValue,"no")){
					pDnnResource->dnnTrainParam.bGradClassNorm = 0;
				}else{
					goto CONFIG_FAIL;
				}
			}
		}
		
		sprintf(szArg,"DNN_SAVE_FILE_NAME");
		if(base_getArgumentValue(szArg,szValue,fpConfig) != SUCCESS) goto CONFIG_FAIL;
		{
			if(szValue[0] == 0) goto CONFIG_FAIL;
			sprintf(pDnnResource->dnnTrainParam.szDnnSaveFileName, "%s", szValue);
		}
	
		sprintf(szArg,"DO_PRINT_TEST_RESULT_TO_CONSOLE");
		if(base_getArgumentValue(szArg,szValue,fpConfig) != SUCCESS) 
		{printf("Default %s option = no\n",szArg); pDnnResource->dnnTrainParam.bPrintTestResultToConsole = 0;}
		else{
			if(!strcmp(szValue,"yes")) {
				pDnnResource->dnnTrainParam.bPrintTestResultToConsole = 1;
			}else if(!strcmp(szValue,"no")){
				pDnnResource->dnnTrainParam.bPrintTestResultToConsole = 0;
			}else{
				goto CONFIG_FAIL;
			}
		}
	
		sprintf(szArg,"USE_SEED_DNN");
		if(base_getArgumentValue(szArg,szValue,fpConfig) != SUCCESS) goto CONFIG_FAIL;
		{
			if(!strcmp(szValue,"yes")) {
				pDnnResource->bUseSeedDnn = 1;
				sprintf(szArg,"SEED_DNN_FILE");
				if(base_getArgumentValue(szArg,szValue,fpConfig) != SUCCESS) goto CONFIG_FAIL;
				{
					if(szValue[0] == 0) goto CONFIG_FAIL;
					sprintf(pDnnResource->szSeedDnnFile, "%s", szValue);
				}
			}else if(!strcmp(szValue,"no")){
				pDnnResource->bUseSeedDnn = 0;
			}else{
				goto CONFIG_FAIL;
			}
		}
		break;
	}

	fclose(fpConfig);
	return SUCCESS;

CONFIG_FAIL:
	fclose(fpConfig);
	printf(" (Argument error: %s) ",szArg);
	return FAIL;
}


#if 0
DNN_Result DNN_printDeepnet(DNN_Resource* pDnnResource){

	FILE* fpLog = 0;
	if(pDnnResource->pDeepnet == NULL) {printf("No deepnet allocated!!\n"); return FAIL;}

	base_printDeepnet(pDnnResource->pDeepnet, NULL);
	fpLog = fopen(pDnnResource->testDevResource.szTotErrLogFile,"a+");
	if(!fpLog){
		if(pDnnResource->taskType == TRAIN) printf("[WARNING] Cannot open development set log file! %s\n",pDnnResource->testDevResource.szTotErrLogFile);
	}else{
		base_printDeepnet(pDnnResource->pDeepnet, fpLog);
		fclose(fpLog);
	}
	fpLog = fopen(pDnnResource->testValiResource.szTotErrLogFile,"a+");
	if(!fpLog){
		printf("[WARNING] Cannot open validation set log file! %s\n",pDnnResource->testValiResource.szTotErrLogFile);
	}else{
		base_printDeepnet(pDnnResource->pDeepnet, fpLog);
		fclose(fpLog);
	}

	return SUCCESS;

}


DNN_LAYER_UNIT* DNN_create_layer_mask(Deepnet* pDeepnet){
	DNN_LAYER_UNIT* p_dnn_output=NULL;
	int nStage = pDeepnet->nStage;
	int dnn_output_idx = 0;
	int i = 0;
	p_dnn_output = (DNN_LAYER_UNIT*)malloc(sizeof(DNN_LAYER_UNIT));

	p_dnn_output->n_layer = nStage+1;
	
	//alloc
	p_dnn_output->unit[dnn_output_idx++] = (float*)malloc(pDeepnet->dnnStage[0].nVisNodes*sizeof(float));
	p_dnn_output->unit[dnn_output_idx++] = (float*)malloc(pDeepnet->dnnStage[0].nHidNodes*sizeof(float));
	for(i = 1; i<nStage; i++){
		p_dnn_output->unit[dnn_output_idx++] = (float*)malloc(pDeepnet->dnnStage[i].nHidNodes*sizeof(float));
	}

	

	return p_dnn_output;
}


DNN_DELTA* DNN_create_delta(Deepnet* pDeepnet){
	DNN_DELTA* p_dnn_delta=NULL;
	int nStage = pDeepnet->nStage;
	int i = 0;
	p_dnn_delta = (DNN_DELTA*)malloc(sizeof(DNN_DELTA));

	p_dnn_delta->nStage = nStage;
	
	//alloc
	for(i = 0; i<nStage; i++){
		p_dnn_delta->delta[i] = (float*)malloc(pDeepnet->dnnStage[i].nHidNodes*sizeof(float));
	}
	
	return p_dnn_delta;
}

DNN_WGRAD* DNN_create_wgrad(Deepnet* pDeepnet){
	DNN_WGRAD* p_dnn_wgrad=NULL;
	int nStage = pDeepnet->nStage;
	int i = 0;
	int idx_h=0, idx_v=0;
	p_dnn_wgrad = (DNN_WGRAD*)malloc(sizeof(DNN_WGRAD));

	p_dnn_wgrad->nStage = nStage;
	
	//alloc
	for(i = 0; i<nStage; i++){
		p_dnn_wgrad->wgrad[i] = (float*)malloc(pDeepnet->dnnStage[i].nHidNodes*pDeepnet->dnnStage[i].nVisNodes*sizeof(float));		
	}
	
	return p_dnn_wgrad;
}

DNN_Result DNN_init_mask_with_prob(Deepnet* pDeepnet, DNN_LAYER_UNIT* p_dnn_mask, float inp_layer_prob, float hid_layer_prob){

	int nStage = pDeepnet->nStage;
	int dnn_output_idx = 0;
	int i = 0, j = 0;

	//init
	//20% drop out for input layer
	for(j = 0; j<pDeepnet->dnnStage[0].nVisNodes;j++){
		p_dnn_mask->unit[dnn_output_idx][j] = (float) DNN_rand_binary(inp_layer_prob);
	}
	dnn_output_idx++;
	
	//50% drop out for hidden layer
	for(i = 0; i<nStage-1; i++){
		for(j = 0; j<pDeepnet->dnnStage[i].nHidNodes;j++){
			p_dnn_mask->unit[dnn_output_idx][j] = (float) DNN_rand_binary(hid_layer_prob);
		}
		dnn_output_idx++;	
	}
	
	//no drop out for output layer
	for(j = 0; j<pDeepnet->dnnStage[nStage-1].nHidNodes;j++){
		p_dnn_mask->unit[dnn_output_idx][j] = (float) 1.0;
	}
	

	return SUCCESS;

}


DNN_Result DNN_init_mask_with_const(Deepnet* pDeepnet, DNN_LAYER_UNIT* p_dnn_mask, float inp_layer_const, float hid_layer_const){

	int nStage = pDeepnet->nStage;
	int dnn_output_idx = 0;
	int i = 0, j = 0;

	//init
	//20% drop out for input layer
	for(j = 0; j<pDeepnet->dnnStage[0].nVisNodes;j++){
		p_dnn_mask->unit[dnn_output_idx][j] = inp_layer_const;
	}
	dnn_output_idx++;
	
	//50% drop out for hidden layer
	for(i = 0; i<nStage-1; i++){
		for(j = 0; j<pDeepnet->dnnStage[i].nHidNodes;j++){
			p_dnn_mask->unit[dnn_output_idx][j] = hid_layer_const;
		}
		dnn_output_idx++;	
	}
	
	//no drop out for output layer
	for(j = 0; j<pDeepnet->dnnStage[nStage-1].nHidNodes;j++){
		p_dnn_mask->unit[dnn_output_idx][j] = (float) 1.0;
	}
	

	return SUCCESS;

}

DNN_Result DNN_init_delta(Deepnet* pDeepnet, DNN_DELTA* p_dnn_delta){

	int nStage = pDeepnet->nStage;
	int i = 0;
	//init
	for(i = 0; i<nStage; i++){
		memset(p_dnn_delta->delta[i],0,pDeepnet->dnnStage[i].nHidNodes*sizeof(float));
	}

	return SUCCESS;
}

DNN_Result DNN_init_wgrad(Deepnet* pDeepnet, DNN_WGRAD* p_dnn_wgrad){

	int nStage = pDeepnet->nStage;
	int i = 0;
	int idx_h=0, idx_v=0;
	//init
		
	for(i =0;i<nStage;i++){
		memset(p_dnn_wgrad->wgrad[i],0,pDeepnet->dnnStage[i].nHidNodes*pDeepnet->dnnStage[i].nVisNodes*sizeof(float));
	}
	
	return SUCCESS;
}


DNN_Result DNN_destroy_layer_mask(Deepnet* pDeepnet, DNN_LAYER_UNIT* p_dnn_output){

	int nStage = pDeepnet->nStage;
	int i = 0;

	//free	
	for(i = 0; i<nStage+1; i++){
		free(p_dnn_output->unit[i]);		
	}
	free(p_dnn_output);
	p_dnn_output=NULL;
	return SUCCESS;
}

DNN_Result DNN_destroy_delta(Deepnet* pDeepnet, DNN_DELTA* p_dnn_delta){

	int nStage = pDeepnet->nStage;
	int i = 0;
	//free
	for(i = 0; i<nStage; i++){
		free(p_dnn_delta->delta[i]);
	}
	free(p_dnn_delta);
	p_dnn_delta=NULL;
	return SUCCESS;
}

DNN_Result DNN_destroy_wgrad(Deepnet* pDeepnet, DNN_WGRAD* p_dnn_wgrad){

	int nStage = pDeepnet->nStage;
	int i = 0;
	int idx_h=0;
	//free
	for(i = 0; i<nStage; i++){
		
		free(p_dnn_wgrad->wgrad[i]);
	}
	free(p_dnn_wgrad);
	p_dnn_wgrad=NULL;
	return SUCCESS;
}



DNN_Result do_forward_prop_with_dropout_training(Deepnet* pDeepnet, DNN_LAYER_UNIT* p_dnn_mask, DNN_LAYER_UNIT* p_dnn_output) {
	DNN_Stage* pDnnStage;
	int i;
	float* input_alt, *output_alt;
	float* input_mask, *output_mask;
	short n_stage = pDeepnet->nStage;
	int idx_h, idx_v;
	float temp = .0f;
	float denom = .0f;
	DNN_NonLinearUnit* nonLinearFunc = pDeepnet->nonLinearFunc;

	for(i=0; i < n_stage; i++){
		input_alt = p_dnn_output->unit[i];
		output_alt = p_dnn_output->unit[i+1];
		input_mask = p_dnn_mask->unit[i];
		output_mask = p_dnn_mask->unit[i+1];
		pDnnStage = &(pDeepnet->dnnStage[i]);		
		
		
		switch (nonLinearFunc[i]){
		case SIGMOID:
			for (idx_h = 0; idx_h < pDnnStage->nHidNodes; idx_h++) {
				temp = 0;
				if(output_mask[idx_h] > 0.001f){
					for(idx_v = 0; idx_v< pDnnStage->nVisNodes; idx_v++){			
						temp += pDnnStage->dnnWeight[idx_h*pDnnStage->nVisNodes + idx_v]* input_alt[idx_v]* input_mask[idx_v];			
					}
					temp += pDnnStage->dnnHidBias[idx_h];

					output_alt[idx_h] = DNN_sigmoid(temp) * output_mask[idx_h];
				}else{
					output_alt[idx_h] = .0f;
				}
			}
			break;
		case RELU:
			for (idx_h = 0; idx_h < pDnnStage->nHidNodes; idx_h++) {
				temp = 0;
				if(output_mask[idx_h] > 0.001f){
					for(idx_v = 0; idx_v< pDnnStage->nVisNodes; idx_v++){			
						temp += pDnnStage->dnnWeight[idx_h*pDnnStage->nVisNodes + idx_v]* input_alt[idx_v]* input_mask[idx_v];			
					}
					temp += pDnnStage->dnnHidBias[idx_h];

					output_alt[idx_h] = DNN_ReLU(temp) * output_mask[idx_h];
				}else{
					output_alt[idx_h] = .0f;
				}
						
			}
			break;
		case LINEAR:
			for (idx_h = 0; idx_h < pDnnStage->nHidNodes; idx_h++) {
				temp = 0;
				if(output_mask[idx_h] > 0.001f){
					for(idx_v = 0; idx_v< pDnnStage->nVisNodes; idx_v++){			
						temp += pDnnStage->dnnWeight[idx_h*pDnnStage->nVisNodes + idx_v]* input_alt[idx_v]* input_mask[idx_v];			
					}
					temp += pDnnStage->dnnHidBias[idx_h];

					output_alt[idx_h] = temp * output_mask[idx_h];
				}else{
					output_alt[idx_h] = .0f;
				}
			}
			break;
		case SOFTMAX:
			float f_max = 0;
			for (idx_h = 0; idx_h < pDnnStage->nHidNodes; idx_h++) {
				temp = 0;

				for(idx_v = 0; idx_v< pDnnStage->nVisNodes; idx_v++){			
					temp += pDnnStage->dnnWeight[idx_h*pDnnStage->nVisNodes + idx_v]* input_alt[idx_v]* input_mask[idx_v];			
				}
				temp += pDnnStage->dnnHidBias[idx_h];	
				
				output_alt[idx_h] = temp;
				
				if(temp > f_max)
					f_max = temp;

			}
			for (idx_h = 0; idx_h < pDnnStage->nHidNodes; idx_h++) {
				if(output_alt[idx_h] > f_max-10.0f){
					output_alt[idx_h] = expf(output_alt[idx_h]-f_max);
				}else{
					output_alt[idx_h] = .0f;
				}
				denom += output_alt[idx_h];
			}
			for (idx_h = 0; idx_h < pDnnStage->nHidNodes; idx_h++) {
				output_alt[idx_h] /= denom;
			}
			break;
		}

	}

	return SUCCESS;
}


DNN_Result do_forward_prop_with_dropout_test(Deepnet* pDeepnet, DNN_LAYER_UNIT* p_dnn_output) {
	DNN_Stage* pDnnStage;
	int i;
	float* input_alt, *output_alt;	
	short n_stage = pDeepnet->nStage;
	int idx_h, idx_v;
	float temp = .0f;
	float denom = .0f;
	DNN_NonLinearUnit* nonLinearFunc = pDeepnet->nonLinearFunc;

	for(i=0; i < n_stage; i++){
		float input_mask = .0f;
		input_alt = p_dnn_output->unit[i];
		output_alt = p_dnn_output->unit[i+1];
		pDnnStage = &(pDeepnet->dnnStage[i]);		

		if(i==0) {input_mask = 0.8f;}
		else if(i < n_stage) {input_mask = 0.5f;}
		else return FAIL;

		switch (nonLinearFunc[i]){
		case SIGMOID:
			for (idx_h = 0; idx_h < pDnnStage->nHidNodes; idx_h++) {
				temp = 0;
			
				for(idx_v = 0; idx_v< pDnnStage->nVisNodes; idx_v++){			
					temp += pDnnStage->dnnWeight[idx_h*pDnnStage->nVisNodes + idx_v]* input_alt[idx_v] * input_mask;			
				}
				temp += pDnnStage->dnnHidBias[idx_h];

				output_alt[idx_h] = DNN_sigmoid(temp);			
							
			}
			break;
		case RELU:
			for (idx_h = 0; idx_h < pDnnStage->nHidNodes; idx_h++) {
				temp = 0;
			
				for(idx_v = 0; idx_v< pDnnStage->nVisNodes; idx_v++){			
					temp += pDnnStage->dnnWeight[idx_h*pDnnStage->nVisNodes + idx_v]* input_alt[idx_v] * input_mask;			
				}
				temp += pDnnStage->dnnHidBias[idx_h];

				output_alt[idx_h] = DNN_ReLU(temp);
						
			}
			break;
		case LINEAR:
			for (idx_h = 0; idx_h < pDnnStage->nHidNodes; idx_h++) {
				temp = 0;
			
				for(idx_v = 0; idx_v< pDnnStage->nVisNodes; idx_v++){			
					temp += pDnnStage->dnnWeight[idx_h*pDnnStage->nVisNodes + idx_v]* input_alt[idx_v] * input_mask;			
				}
				temp += pDnnStage->dnnHidBias[idx_h];

				output_alt[idx_h] = temp;
						
			}
			break;
		case SOFTMAX:
			float f_max = 0;
			for (idx_h = 0; idx_h < pDnnStage->nHidNodes; idx_h++) {
				temp = 0;

				for(idx_v = 0; idx_v< pDnnStage->nVisNodes; idx_v++){			
					temp += pDnnStage->dnnWeight[idx_h*pDnnStage->nVisNodes + idx_v]* input_alt[idx_v] * input_mask;			
				}
				temp += pDnnStage->dnnHidBias[idx_h];	
				
				output_alt[idx_h] = temp;
				
				if(temp > f_max)
					f_max = temp;

			}
			for (idx_h = 0; idx_h < pDnnStage->nHidNodes; idx_h++) {
				if(output_alt[idx_h] > f_max-10.0f){
					output_alt[idx_h] = expf(output_alt[idx_h]-f_max);
				}else{
					output_alt[idx_h] = .0f;
				}
				denom += output_alt[idx_h];
			}
			for (idx_h = 0; idx_h < pDnnStage->nHidNodes; idx_h++) {
				output_alt[idx_h] /= denom;
			}
			break;
		}

	}

	return SUCCESS;

}


DNN_Result do_backward_prop_with_dropout_training(Deepnet* pDeepnet, DNN_LAYER_UNIT* p_dnn_output, DNN_LAYER_UNIT* p_dnn_mask, DNN_DELTA* p_dnn_delta){
	DNN_Stage* pDnnStage;
	float* pp_dnn_weight=0;
	float* pp_dnn_output=0;
	float* pp_dnn_delta_hid=0;
	float* pp_dnn_bgrad_curr_hid=0;
	float* pp_dnn_delta_vis=0;
	float* pp_dnn_mask_hid=0;
	float* pp_dnn_mask_vis=0;
	float weighted_sum = 0;
	int i = 0, idx_h=0, idx_v=0;
	short n_stage = pDeepnet->nStage;
	DNN_NonLinearUnit* nonLinearFunc = pDeepnet->nonLinearFunc;

	for(i=n_stage-1; i > 0; i--){
		pDnnStage = &(pDeepnet->dnnStage[i]);
		pp_dnn_weight = pDnnStage->dnnWeight;
		pp_dnn_output = p_dnn_output->unit[i];
		pp_dnn_delta_vis = p_dnn_delta->delta[i-1];
		pp_dnn_delta_hid = p_dnn_delta->delta[i];
		pp_dnn_mask_hid = p_dnn_mask->unit[i+1];
		pp_dnn_mask_vis = p_dnn_mask->unit[i];
		//delta back propagation
		for(idx_v=0; idx_v<pDnnStage->nVisNodes; idx_v++){
			weighted_sum = 0;
			for(idx_h=0; idx_h<pDnnStage->nHidNodes; idx_h++){
				weighted_sum += pp_dnn_delta_hid[idx_h]*pp_dnn_weight[idx_h*pDnnStage->nVisNodes + idx_v]*pp_dnn_mask_hid[idx_h];				
			}
			
			if(nonLinearFunc[i-1] == SIGMOID){
				
				pp_dnn_delta_vis[idx_v] = DNN_max(0.01f,DNN_SIGM_diff(pp_dnn_output[idx_v]))*weighted_sum*pp_dnn_mask_vis[idx_v];
				
			}else if(nonLinearFunc[i-1] == RELU){
				
				if(pp_dnn_output[idx_v] <= 0){
					pp_dnn_delta_vis[idx_v] = 0;
				}else{
					pp_dnn_delta_vis[idx_v] = weighted_sum*pp_dnn_mask_vis[idx_v];
				}
				
			}else if(nonLinearFunc[i-1] == LINEAR){
				
				pp_dnn_delta_vis[idx_v] = weighted_sum*pp_dnn_mask_vis[idx_v];
				
				
			}else{
				return FAIL; //non linear unit type error
			}
			
		}

		
	}


	return SUCCESS;
}

DNN_Result do_update_deepnet(Deepnet* pDeepnet, DNN_WGRAD* p_dnn_wgrad_curr, DNN_WGRAD* p_dnn_wgrad_prev, DNN_DELTA* p_dnn_bgrad_curr, DNN_DELTA* p_dnn_bgrad_prev, float lr, float alpha){
	
	int nStage = pDeepnet->nStage;
	int i = 0;
	int idx_h=0, idx_v=0;
	DNN_Stage* pDnnStage;
	float* pp_dnn_weight=0;
	float* pp_dnn_hbias=0;
	float* pp_dnn_Wgrad_curr=0;
	float* pp_dnn_Wgrad_prev=0;
	float* pp_dnn_bgrad_curr=0;
	float* pp_dnn_bgrad_prev=0;
	// w update
	for(i =0;i<nStage;i++){
		pDnnStage = &(pDeepnet->dnnStage[i]);
		pp_dnn_weight = pDnnStage->dnnWeight;
		pp_dnn_hbias = pDnnStage->dnnHidBias;
		pp_dnn_Wgrad_curr = p_dnn_wgrad_curr->wgrad[i];
		pp_dnn_Wgrad_prev = p_dnn_wgrad_prev->wgrad[i];
		pp_dnn_bgrad_curr = p_dnn_bgrad_curr->delta[i];
		pp_dnn_bgrad_prev = p_dnn_bgrad_prev->delta[i];
		
		for(idx_h =0;idx_h<pDeepnet->dnnStage[i].nHidNodes;idx_h++){
			for(idx_v =0;idx_v < pDeepnet->dnnStage[i].nVisNodes; idx_v++){
				//weight update
				pp_dnn_Wgrad_prev[idx_h*pDnnStage->nVisNodes+idx_v] = lr*pp_dnn_Wgrad_curr[idx_h*pDnnStage->nVisNodes+idx_v] + alpha*pp_dnn_Wgrad_prev[idx_h*pDnnStage->nVisNodes+idx_v];
				pp_dnn_weight[idx_h*pDnnStage->nVisNodes + idx_v] += pp_dnn_Wgrad_prev[idx_h*pDnnStage->nVisNodes+idx_v];
				//for Max-norm regularization : lr*(p_dnn_wgrad_curr->wgrad[i][idx_h*pDnnStage->nVisNodes+idx_v]/mini_batch - lamda*pp_dnn_weight[idx_h*pDnnStage->nVisNodes + idx_v]) + alpha*p_dnn_wgrad_prev->wgrad[i][idx_h*pDnnStage->nVisNodes+idx_v];

				// curr w 초기화
				pp_dnn_Wgrad_curr[idx_h*pDnnStage->nVisNodes+idx_v] = 0;
			}
			//bias update
			pp_dnn_bgrad_prev[idx_h] = lr*pp_dnn_bgrad_curr[idx_h] + alpha*pp_dnn_bgrad_prev[idx_h];
			pp_dnn_hbias[idx_h] += pp_dnn_bgrad_prev[idx_h];
			pp_dnn_bgrad_curr[idx_h] = 0;
		}
	}
	return SUCCESS;
}


DNN_Result target_gen(int class_idx, int num_class, float* target){
	int i=0;
	for(i = 0; i<num_class; i++){
		if(i==class_idx){
			target[i] = 1;
		}else{
			target[i] = 0;
		}
	}
	return SUCCESS;
}

float cal_err(float* target, float* pp_dnn_output, int num_class){
	
	int i=0;
	float err = 0;
	
	for(i = 0; i < num_class; i++){
		err += (target[i] - pp_dnn_output[i])*(target[i] - pp_dnn_output[i]);
		
	}

	return 0.5*(float)err/(float)num_class;
}

DNN_Result cal_output_layer_delta(float* target, DNN_LAYER_UNIT* p_dnn_output, int num_class, DNN_DELTA* p_dnn_delta, DNN_NonLinearUnit finalRegType,  DNN_ErrMeasureFunc errMeasureFunc){

	int i=0;
	float* pp_dnn_output = p_dnn_output->unit[p_dnn_output->n_layer-1];
	float* pp_dnn_delta = p_dnn_delta->delta[p_dnn_delta->nStage-1];
	
	switch(finalRegType){
	case SIGMOID:
		if(errMeasureFunc == SQRERR){
			for(i=0;i<num_class;i++) pp_dnn_delta[i] = (target[i] - pp_dnn_output[i])*DNN_max(0.01f,DNN_SIGM_diff(pp_dnn_output[i]));
		}else if(errMeasureFunc == CROSS_ENTROPY){
			for(i=0;i<num_class;i++) pp_dnn_delta[i] = (target[i] - pp_dnn_output[i]);
		}
		break;
	case RELU:
		if(errMeasureFunc == SQRERR){
			for(i=0;i<num_class;i++) pp_dnn_delta[i] = (target[i] - pp_dnn_output[i])*DNN_ReLU_diff(pp_dnn_output[i]);
		}else if(errMeasureFunc == CROSS_ENTROPY){
			for(i=0;i<num_class;i++) pp_dnn_delta[i] = ((target[i] - pp_dnn_output[i])*DNN_ReLU_diff(pp_dnn_output[i]))/(DNN_max(0.01f,(pp_dnn_output[i]*(1-pp_dnn_output[i]))));
		}
		break;
	case SOFTMAX:
		if(errMeasureFunc == SQRERR){
			for(i=0;i<num_class;i++) pp_dnn_delta[i] = (target[i] - pp_dnn_output[i])*DNN_max(0.01f,DNN_SIGM_diff(pp_dnn_output[i]));
		}else if(errMeasureFunc == CROSS_ENTROPY){
			for(i=0;i<num_class;i++) pp_dnn_delta[i] = (target[i] - pp_dnn_output[i]);
		}
		break;
	}


	return SUCCESS;
}


void get_max_node_idx(float* result, float* pp_dnn_output, int num_class){
	int i=0;
	int n_o_max_idx=0;
	float o_max=-9999;
	int max_idx=0;
	//max check
	for(i = 0; i < num_class; i++){
		if( pp_dnn_output[i] > o_max){
			o_max = pp_dnn_output[i];
			max_idx = i;
		}				
	}
	for(i = 0; i < num_class; i++){
		//if( fabs(p_dnn_output->unit[p_dnn_output->n_layer-1][i] - o_max) < 0.001){
		if(i == max_idx){
			result[i] = 1.0;
		}else{
			result[i] = 0;
		}
	}

}

int check_answer(float* target, float* pp_dnn_output, int num_class){
	int n_t_max_idx=0;
	int n_o_max_idx=0;
	int i=0;
	float t_max=0;
	float o_max=0;
	//for 2class problem
	/*if(p_dnn_output->unit[p_dnn_output->n_layer-1][0] == p_dnn_output->unit[p_dnn_output->n_layer-1][1]){
		return 1;
	}*/

	for(i = 0; i < num_class; i++){
		if( target[i] > t_max){
			t_max = pp_dnn_output[i];
			n_t_max_idx = i;
		}

		if( pp_dnn_output[i] > o_max){
			o_max = pp_dnn_output[i];
			n_o_max_idx = i;
		}				
	}
	if(n_o_max_idx == n_t_max_idx){
		return 0;
	}else{
		return 1;
	}	
}

int DNN_rand_binary(float ratio){

	if(ratio <0 || ratio >1){
		return FAIL;
	}else if(ratio == 0 || ratio ==1){
		return (int)ratio;
	}else{
		if((float)rand()/RAND_MAX < ratio){
			return 1;
		}else{
			return 0;
		}
	}

}

DNN_Result do_backward_prop(Deepnet* pDeepnet, DNN_LAYER_UNIT* p_dnn_output, DNN_DELTA* p_dnn_delta){
	DNN_Stage* pDnnStage;
	float* pp_dnn_weight=0;
	float* pp_dnn_output=0;
	float* pp_dnn_delta_hid=0;
	float* pp_dnn_delta_vis=0;
	float weighted_sum = 0;
	int i = 0, idx_h=0, idx_v=0;
	short n_stage = pDeepnet->nStage;
	DNN_NonLinearUnit* nonLinearFunc = pDeepnet->nonLinearFunc;

	for(i=n_stage-1; i > 0; i--){
		pDnnStage = &(pDeepnet->dnnStage[i]);
		pp_dnn_weight = pDnnStage->dnnWeight;
		pp_dnn_output = p_dnn_output->unit[i];
		pp_dnn_delta_vis = p_dnn_delta->delta[i-1];
		pp_dnn_delta_hid = p_dnn_delta->delta[i];
		
		//delta back propagation
		for(idx_v=0; idx_v<pDnnStage->nVisNodes; idx_v++){
			weighted_sum = 0;
			for(idx_h=0; idx_h<pDnnStage->nHidNodes; idx_h++){
				weighted_sum += pp_dnn_delta_hid[idx_h]*pp_dnn_weight[idx_h*pDnnStage->nVisNodes + idx_v];
			}

			if(nonLinearFunc[i-1] == SIGMOID){
				
				pp_dnn_delta_vis[idx_v] = DNN_max(0.01f,DNN_SIGM_diff(pp_dnn_output[idx_v]))*weighted_sum;
				
			}else if(nonLinearFunc[i-1] == RELU){
				
				if(pp_dnn_output[idx_v] <= 0){
					pp_dnn_delta_vis[idx_v] = 0;
				}else{
					pp_dnn_delta_vis[idx_v] = weighted_sum;
				}
				
			}else if(nonLinearFunc[i-1] == LINEAR){

				pp_dnn_delta_vis[idx_v] = weighted_sum;
				
			}else{
				return FAIL; //non linear unit type error
			}
			
		}

		
	}
	

	return SUCCESS;
}

DNN_Result do_update_grad(Deepnet* pDeepnet, DNN_LAYER_UNIT* p_dnn_output, DNN_DELTA* p_dnn_delta, DNN_WGRAD* p_dnn_wgrad_curr, DNN_DELTA* p_dnn_bgrad_curr){

	DNN_Stage* pDnnStage;
	float* pp_dnn_output=0;
	float* pp_dnn_delta_hid=0;
	float* pp_dnn_bgrad_curr_hid=0;
	float* pp_dnn_delta_vis=0;
	int i = 0, idx_h=0, idx_v=0;
	short n_stage = pDeepnet->nStage;

	for(i=n_stage-1; i >= 0; i--){
		pDnnStage = &(pDeepnet->dnnStage[i]);
		pp_dnn_delta_hid = p_dnn_delta->delta[i];
		pp_dnn_bgrad_curr_hid = p_dnn_bgrad_curr->delta[i];
		pp_dnn_output = p_dnn_output->unit[i];
		//Weight grad update
		for(idx_v=0; idx_v<pDnnStage->nVisNodes; idx_v++){
			for(idx_h=0; idx_h<pDnnStage->nHidNodes; idx_h++){
				p_dnn_wgrad_curr->wgrad[i][idx_h*pDnnStage->nVisNodes+idx_v] += pp_dnn_delta_hid[idx_h]*pp_dnn_output[idx_v];				
			}
		}

		for(idx_h=0; idx_h<pDnnStage->nHidNodes; idx_h++){
			pp_dnn_bgrad_curr_hid[idx_h] += pp_dnn_delta_hid[idx_h];

		}
		
	}

	return SUCCESS;
}
#endif