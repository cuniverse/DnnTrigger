// dnn_decoder.cpp
// DNN decoder(PowerAI DeepNet) C++ wrapper for DNN trigger
// 2016-09 by mckeum

#include "dnn_decoder.h"

#include <memory.h>
#include <stdio.h>
#include <algorithm>

#include "PowerAI_BaseCommon.h"
#include "bp_train.h"


CDnnDecoder::CDnnDecoder(const char root_path[], const char config_path[])
{
	feat_pool = NULL;
	pDeepnet = NULL;
	p_dnn_output = NULL;

	DNN_Resource dnnResource;       // dnnResource = DNN config ���� ���� (DNN_LoadCibfug �Լ��� ����, DNN config setting )
	auto ret_dnnlc = DNN_LoadConfig(&dnnResource, root_path, config_path);   // .ini �� �������� �ʱ�ȭ, �ʱ�ȭ ��� ���� ��, 'SUCCESS' retrun, 
	if (ret_dnnlc != SUCCESS)
	{
		printf("CONFIG FILE READ FAILED\n");
		err = 1;
		return;
	}

	concat_before = dnnResource.dnnStructParam.concatBefore;                    // frame window (front)
	concat_after = dnnResource.dnnStructParam.concatSize - concat_before - 1;   // frame window (rear)
	feat_dim = dnnResource.dnnStructParam.nFeatDim;                             // feature dimension

	reset();

	pDeepnet = DNN_create(dnnResource.dnnStructParam.numLayer, dnnResource.dnnStructParam.numNodes, dnnResource.dnnStructParam.nonLinearFunc);
	auto ret_dnnl = DNN_load_dnn(pDeepnet, root_path, dnnResource.szSeedDnnFile);    // DNN ������ ����� �̷�������� Ȯ��, ����� ���� ��, 'SUCCES' return
	if (ret_dnnl != SUCCESS)
	{
		printf("DNN MODEL LOAD FAILED\n");
		err = 2;
		return;
	}

	p_dnn_output = DNN_create_layer_unit(pDeepnet);	//DNN �νİ�� ���� ��
	if (!p_dnn_output)	{ err = 3; return; }
	err = 0;
}

CDnnDecoder::~CDnnDecoder()
{
	DNN_destroy_layer_unit(p_dnn_output);
	DNN_destroy(pDeepnet);
	delete feat_pool;
}


// reset DNN decoder without reloading DNN config
int CDnnDecoder::reset()
{
	frame_input = -1;
	delete[] feat_pool;
	feat_pool = new float[feat_dim * (concat_before+1+concat_after)]();

	return 0;
}

// get 1 frame feature input, concatenate frames, decode DNN
// return frame # (if frame# < 0, probability output will be unreliable)
int CDnnDecoder::decode(float* in, float* out)
{
	frame_input++;

	int shift_len = (concat_before+concat_after) * feat_dim;     // window_size * feature_dimension 
	memmove(&feat_pool[0], &feat_pool[feat_dim], shift_len*sizeof(feat_pool[0]));     // memmove(����Ǵ� ������ �����ּ�, ������ ������ �����ּ�, ������ ũ��)
	std::copy_n(in, feat_dim, &feat_pool[shift_len]);   // copy_n(_First, cnt, _Dest) >> _First ��ġ���� cnt ������ŭ _Dest�� ���� , �Ű������� ���� in�� ���� ����

	p_dnn_output->unit[0] = feat_pool;

	auto ret_dfp = do_forward_prop(pDeepnet, p_dnn_output);

	auto output_layer = p_dnn_output->unit[p_dnn_output->n_layer - 1];
	std::copy_n(output_layer, pDeepnet->dnnStage[pDeepnet->nStage-1].nHidNodes, out);

	return frame_input - concat_after;
}

// get number of output nodes
int CDnnDecoder::getNumOutNode()
{
	if (NULL == pDeepnet)
		return 0;
	return pDeepnet->dnnStage[pDeepnet->nStage-1].nHidNodes;
}
