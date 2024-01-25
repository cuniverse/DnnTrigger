// dnn_trigger.cpp
// Voice trigger(word detector) using DNN
// 2016-12 by mckeum, ywjung

#define TRG_DLLEXPORT
#include "dnn_trigger.h"

#include <stdio.h>

#include "clog.h"
#define TRG_CLOG 1

#define MININI_ANSI
#define INI_READONLY 
#include "minIni.h"

#include "SizedQueue.h"

#include "feat_2pass.h"
#include "dnn_decoder.h"
#include "detector_word.h"


#ifndef _MAX_PATH
#define _MAX_PATH 255
#endif


CDnnTrigger::CDnnTrigger(const char root_path[], const char config_path[])    // CDnnTrigger ������, config file ������ �ʱ�ȭ
{
	char tmp_path[_MAX_PATH] = { 0 };
	int tmp_wmax = 0;
	output_frame = -1;

	char ini_config[_MAX_PATH];
	snprintf(ini_config, sizeof(ini_config), "%s/%s", root_path, config_path);

	// log settings
	ini_gets("log", "output", "", tmp_path, _MAX_PATH, ini_config);
	if ('\0' != tmp_path[0])
	{
		clog_init_path(TRG_CLOG, tmp_path);
		clog_set_level(TRG_CLOG, (clog_level)ini_getl("log", "level", CLOG_ERROR, ini_config));
	}
	
	// init feature extractor
	feat_extractor = new CFeat2pass(root_path);
	if (feat_extractor->getError()) { err = 1000 + feat_extractor->getError(); return; }

	// init DNN decoder
	ini_gets("trigger", "dnn_ini", "", tmp_path, _MAX_PATH, ini_config);
	dnn_decoder = new CDnnDecoder(root_path, tmp_path);
	if (dnn_decoder->getError()) { err = 2000 + dnn_decoder->getError(); return; }

	// init word detector
	auto keyword_num = dnn_decoder->getNumOutNode() - 2;
	detector = new CDetectorWord(keyword_num, root_path, config_path);
	if (detector->getError()) { err = 3000 + detector->getError(); return; }
	if (_clog_loggers[TRG_CLOG])
		detector->setClog(TRG_CLOG);


	// memory alloc
	pcm_stream = new SizedQueue(16000);
	dnn_prob_output = new float[dnn_decoder->getNumOutNode()];

	err = 0;
}


CDnnTrigger::~CDnnTrigger()
{
	delete pcm_stream;
	delete[] dnn_prob_output;

	delete feat_extractor;
	delete dnn_decoder;
	delete detector;

	clog_free(TRG_CLOG);
}


bool CDnnTrigger::reset()
{
	pcm_stream->clear();

	if (feat_extractor)	feat_extractor->reset();
	if (dnn_decoder)	dnn_decoder->reset();
	if (detector)	detector->reset();

	return true;
}

int CDnnTrigger::detect(const int len_sample, const int16_t pcm_buf[],int *p_info)
{
	// TODO: input ���� ����, �ʰ��� �����÷ο� ��Ŵ
	pcm_stream->putItems(len_sample, pcm_buf);

	int detected_frame = 0;

	while (160 <= pcm_stream->size())
	{
		int16_t frame_buf[160];
		pcm_stream->getItems(160, frame_buf);

		long len_feat = 0;
		feat_extractor->getFeature(160, frame_buf, &len_feat, feat_buf);   // feat_buf : �� frame�� ���� Ư¡���� �����Ͽ� featu_buf(Queue, FIFO ����)�� ����

		for (int i = 2; i < len_feat; i += 51)    // feat_buf[0]�� Ư¡������ �Ϸ�Ǿ������� ���� info�� , feat_buf[1]�� Ư¡���Ⱚ�� reset �Ǿ������� ���� info�� ����, ���� i=2���� ����!  ( powerdsr_fronted.c ���� ) 
		{
			output_frame = dnn_decoder->decode(&feat_buf[i], dnn_prob_output);   // dnn_prob_output = DNN�� output node���� ��µ� ��, �� class�� ���� Ȯ������ ����, decode�Լ��� ���� �� �޾ƿ�
			auto detected = detector->detect(dnn_prob_output);					 // �� class�� ���� Ȯ����(dnn_prob_output)�� �����Ͽ� detect Ȯ��
			if (0 < detected){
				detected_frame = output_frame;
				sp_output_frame = detector->getTriggerFrameLen();
			}
		}
	}
    if(p_info!=NULL) {
        p_info[0] = output_frame;
        p_info[1] = detected_frame>0?sp_output_frame:0;
    }

	if (!detected_frame)
		return 0;

	// ���� ���������� ����� frame�� return
	return detected_frame;
}