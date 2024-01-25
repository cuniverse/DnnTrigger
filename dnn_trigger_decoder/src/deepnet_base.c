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


#include <stdlib.h>
#include <string.h>

#include "PowerAI_BaseCommon_Struct.h"
#include "PowerAI_BaseCommon.h"

float base_rand_fio(int fan_in, int fan_out){
	float range = sqrt(6.0f/(float)(fan_in+fan_out));
	int i_rand = rand(); 
		
	i_rand = i_rand << 15; 
	i_rand = i_rand|rand();
	return range*4.0f*2.0f*( ((float)i_rand)/((float)(RAND_MAX*RAND_MAX))-0.5);
}
void trim(char* string) {
	int i = 0, j = 0, k = 0;

	if (string == NULL)
		return;

	//trim right
	j = strlen(string) - 1;
	while (j >= 0 && (string[j] == ' ' || string[j] == '=' || string[j] == 10 || string[j] == '\n' || string[j] == '\r')){
		j--;
	}
	string[j + 1] = 0;


	//trim left
	while (string[i] == ' ')
		i++;
	if (i != 0)
		strcpy(string, string + i);
}
void trimLowerCase(char* string)
{
    
    int i=0,j=0,k=0;    
    
    if(string == NULL)
        return;

    //trim right
    j = strlen(string)-1;
	while (j >= 0 && (string[j] == ' ' || string[j] == '=' || string[j] == 10 || string[j] == '\n' || string[j] == '\r')){
        j--;
	}
    string[j+1] = 0;
    

    //trim left
    while(string[i]==' ')
        i++;
    if(i != 0)
        strcpy(string,string+i);
		
	//lower case & ignore after #
	while(string[k]!=0){
        if(string[k] >= 'A' && string[k] <= 'Z'){
			string[k] += 32;
		}else if(string[k] == '#'){
			string[k] = 0;
			break;
		}
		k++;
	}
}

DNN_Result splitArg(char* string, char* sAlg, char* sValue){
	int i=0, i_arg=0, i_arg_s=0;
	int flag_put_in=0;
	char* sComp[2];
	sComp[0] = sAlg;
	sComp[1] = sValue;

	if(string == NULL || sComp == NULL)
        return FAIL;

	while(string[i]!=0){
		if(string[i] == ' ' || string[i] == '=' ){
			if(flag_put_in == 1){
				flag_put_in = 0;
				sComp[i_arg][i_arg_s] = 0;
				i_arg = 1;
				i_arg_s = 0;
			}
		}else{
			flag_put_in = 1;
			sComp[i_arg][i_arg_s] = string[i];
			i_arg_s++;
		}
		i++;
	}

	sComp[i_arg][i_arg_s] = 0;

	return SUCCESS;
}

DNN_Result base_getArgumentValueWithoutLowCase(char* pszArg, char* pValue, FILE* fpConfig) {

	char sz_line[MAXSTRLEN] = { 0 };
	char sz_lineOri[MAXSTRLEN] = { 0 };
	char sz_alg[MAXSTRLEN] = { 0 };
	char sz_value[MAXSTRLEN] = { 0 };
	fseek(fpConfig, 0, SEEK_SET);

	trim(pszArg);

	while (fgets(sz_line, MAXSTRLEN, fpConfig) != NULL){
		strcpy(sz_lineOri, sz_line);
		trim(sz_line);
		if (sz_line[0] == 0 || sz_line[0] == 10) continue;
		splitArg(sz_line, sz_alg, sz_value);
		if (!strcmp(pszArg, sz_alg)){
			strcpy(pValue, sz_value);
			return SUCCESS;
		}

	}
	return FAIL;
}

HCILAB_PUBLIC POWER_DEEPNET_API
DNN_Result base_getArgumentValue(const char* pszArg, char* pValue, FILE* fpConfig) {
	char sz_line[MAXSTRLEN] = {0};
	char sz_alg[MAXSTRLEN] = {0};
	char sz_value[MAXSTRLEN] = {0};
	char sz_arg[MAXSTRLEN] = { 0 };

	pValue[0] = '\0';

	fseek(fpConfig, 0, SEEK_SET);

	strcpy(sz_arg, pszArg);
	trimLowerCase(sz_arg);

	while(fgets(sz_line, MAXSTRLEN, fpConfig) != NULL){
		trimLowerCase(sz_line);
		if(sz_line[0] == 0 || sz_line[0] == 10 ) continue;
		splitArg(sz_line, sz_alg, sz_value);
		if (!strcmp(sz_arg, sz_alg)) {
			strcpy(pValue, sz_value);
			return SUCCESS;
		}

	}
	return FAIL;
}


#if 0//yowon 2015-03-20
int base_compare(const void *a , const void *b){

	DNN_ShufRandStruct* tmp1 = (DNN_ShufRandStruct *)a;
	DNN_ShufRandStruct* tmp2 = (DNN_ShufRandStruct *)b;

	return ( tmp1->rand_mat - tmp2->rand_mat );
}
#endif

void base_printDeepnet(Deepnet* pDeepnet, FILE* fpLog){
	int i=0;
		
	if(fpLog != NULL){
		fprintf(fpLog, "===========Deep Neural Network Structure Info.===========\n");
		for(i = 0; i<pDeepnet->nStage; i++){
		
			fprintf(fpLog, "%d",pDeepnet->dnnStage[i].nVisNodes);
		
			fprintf(fpLog, "-");
			switch(pDeepnet->nonLinearFunc[i]){
			case RELU:
				fprintf(fpLog, "(RELU)");
				break;
			case SIGMOID:
				fprintf(fpLog, "(SIGMOID)");
				break;
			case SOFTMAX:
				fprintf(fpLog, "(SOFTMAX)");
				break;
			case LINEAR:
				fprintf(fpLog, "(LINEAR)");
				break;
			}
		}
		fprintf(fpLog, "%d\n",pDeepnet->dnnStage[i-1].nHidNodes);
		fprintf(fpLog, "=========================================================\n");
		fprintf(fpLog, "(Fr-wise Err)\t(Aved Cls Err)\t(Obj Func Val)\t(Learning Rate)\n");
	}else{
		printf("===========Deep Neural Network Structure Info.===========\n");
		for(i = 0; i<pDeepnet->nStage; i++){
		
			printf("%d",pDeepnet->dnnStage[i].nVisNodes);
		
			printf("-");
			switch(pDeepnet->nonLinearFunc[i]){
			case RELU:
				printf("(RELU)");
				break;
			case SIGMOID:
				printf("(SIGMOID)");
				break;
			case SOFTMAX:
				printf("(SOFTMAX)");
				break;
			case LINEAR:
				printf("(LINEAR)");
				break;
			}
		}
		printf("%d\n",pDeepnet->dnnStage[i-1].nHidNodes);
		printf("=========================================================\n");
	}
}
