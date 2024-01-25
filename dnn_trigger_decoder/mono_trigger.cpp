// mono_trigger.cpp
// Voice trigger(word detector) using DNN
// 2016-12 by mckeum

#define TRG_DLLEXPORT
#include "mono_trigger.h"

#include <stdio.h>

#include "clog.h"
#define TRG_CLOG 1

#define MININI_ANSI
#define INI_READONLY 
#include "minIni.h"

#include "SizedQueue.h"

#include "feat_2pass.h"
#include "dnn_decoder.h"
#include "detector_mono.h"


#ifndef _MAX_PATH
#define _MAX_PATH 255
#endif


CMonoTrigger::CMonoTrigger(const char root_path[], const char config_path[])
{
	char tmp_path[_MAX_PATH] = { 0 };
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
	ini_gets("mono", "dnn_ini", "", tmp_path, _MAX_PATH, ini_config);
	dnn_decoder = new CDnnDecoder(root_path, tmp_path);
	if (dnn_decoder->getError()) { err = 2000 + dnn_decoder->getError(); return; }

	// init word detector
	auto phon_num = dnn_decoder->getNumOutNode();
	detector = new CDetectorMono(phon_num, root_path, config_path);
	if (detector->getError()) { err = 3000 + detector->getError(); return; }
	if (_clog_loggers[TRG_CLOG])
		detector->setClog(TRG_CLOG);


	// memory alloc
	dnn_prob_output = new float[phon_num];
	pcm_stream = new SizedQueue(16000);

	err = 0;
}


CMonoTrigger::~CMonoTrigger()
{
	delete pcm_stream;
	delete[] dnn_prob_output;

	delete feat_extractor;
	delete dnn_decoder;
	delete detector;

	clog_free(TRG_CLOG);
}


// 검출할 phone열 추가 (0-terminate된 C style 문자열)
int CMonoTrigger::addPhonSeq(const char sequence[])
{
	return detector->addPhonSeq(sequence);
}


// 검출할 phone열 추가 (std::string)
int CMonoTrigger::addPhonSeq(const std::string sequence)
{
	return detector->addPhonSeq(sequence);
}


// clear detector to be ready for another sound stream
bool CMonoTrigger::reset()
{
	if (feat_extractor)	feat_extractor->reset();
	if (dnn_decoder)	dnn_decoder->reset();
	if (detector)	detector->reset();

	return true;
}


int CMonoTrigger::detect(const int len_sample, const int16_t pcm_buf[],int* spinfo)
{
	pcm_stream->putItems(len_sample, pcm_buf);

	int detected_kw = 0;

	while (160*sizeof(int16_t) <= pcm_stream->size())
	{
		int16_t frame_buf[160];
		pcm_stream->getItems(160, frame_buf);

		long len_feat = 0;
		feat_extractor->getFeature(160, frame_buf, &len_feat, feat_buf);

		for (int i = 2; i < len_feat; i += 51)
		{
			output_frame = dnn_decoder->decode(&feat_buf[i], dnn_prob_output);
			auto frame_detected = detector->detect(dnn_prob_output);
			if (frame_detected)
				detected_kw = frame_detected;
		}
	}

	return detected_kw;
}
