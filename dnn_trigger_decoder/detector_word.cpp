// detector_word.cpp
// Keyword detector for word DNN model
// 2016-09 by ywjung, mckeum

//#define FILE_LOG

#include "detector_word.h"

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <algorithm>

#include "clog.h"

#define MININI_ANSI
#define INI_READONLY
#include "minIni.h"


#ifndef _MAX_PATH
#define _MAX_PATH 255
#endif

//#define wmax 50
//#define _CM_THRESHOLD2 10 // _CM_THRESHOLD 보다 큰 frame 수가 이보다 많으면 검출

float CDetectorWord::posterior_smoothing_ring_buffer(const float dnn_out_prob[], const int frame_idx)//frame_idx 는 0보다 커야함.
{
	float accum_post_dnnprob = 0;
	const int hsmooth_length = std::max(1, frame_idx - wsmooth_length + 1);
	const float hsmooth_value = 1 / (float)(frame_idx - hsmooth_length + 1);

	for (int k = hsmooth_length - 1; k < frame_idx; k++)
	{
		accum_post_dnnprob += dnn_out_prob[k%wsmooth_length];
	}

	float post_dnn_outprob = hsmooth_value*accum_post_dnnprob;

	return post_dnn_outprob;
}


float CDetectorWord::calc_confidence_score_ring_buffer(float **post_dnn_outprob, int const frame_idx, const int class_num)
{
	float cm_score = 1.f;
	const int hmax = std::max(1, frame_idx - wmax + 1);

	for (int i = 0; i < class_num; i++)
	{
		float max_class_prob = 0.f;
		for (int k = hmax - 1; k < frame_idx; k++)
			max_class_prob = std::max(max_class_prob, post_dnn_outprob[i][k%wmax]);

		cm_score = cm_score * max_class_prob;
	}
	if (cm_score < FLT_MIN)	return 0.f;
	return powf(cm_score, 1.f/class_num);
}


CDetectorWord::CDetectorWord(const int keyword_num, const char root_path[], const char config_path[])
{
	this->keyword_num = keyword_num;
	clog_id = 0;

	char ini_config[_MAX_PATH];
	snprintf(ini_config, sizeof(ini_config), "%s/%s", root_path, config_path);


	/* ** tigger settings ** */
	prob_thr = ini_getf("trigger", "prob_threshold", -1.f, ini_config);
	if (prob_thr <= 0.f) { err = 4; return; }

	// init wmax value
	wmax = ini_getINT("trigger", "w_max", 50, ini_config);
	if (wmax <= 0) { err = 4; return; }

	// init _CM_THRESHLOD2 value   : _CM_THRESHOLD 보다 큰 frame 수가 이보다 많으면 검출
	_CM_THRESHOLD2 = ini_getINT("trigger", "_CM_THRESHOLD2", 10, ini_config);
	if (_CM_THRESHOLD2 <= 0) { err = 4; return; }

	// memory alloc
	past_prob.kw_prob = new float*[keyword_num]();
	for (int i = 0; i < keyword_num; i++)
		past_prob.kw_prob[i] = new float[wsmooth_length]();

	keyword_prob_ring = (float **)(calloc(keyword_num, sizeof(float *)));
	for (int i = 0; i < keyword_num; i++)
		keyword_prob_ring[i] = (float *)calloc(wmax, sizeof(float));
	clear();

	err = 0;
}


CDetectorWord::~CDetectorWord()
{
	for (int i = 0; i < keyword_num; i++)
		delete[] past_prob.kw_prob[i];
	delete[] past_prob.kw_prob;

	for (int i = 0; i < keyword_num; i++)
		free(keyword_prob_ring[i]);
	free(keyword_prob_ring);
}

void CDetectorWord::setClog(int log_id)
{
	clog_id = log_id;
}

void CDetectorWord::clear()
{
	proc_count = -1;
	trigger_word_count = 0;
	trg_sp_count = 0;
	sp_detected_frame = -1;


	memset(past_prob.sil_outprob, 0, sizeof(past_prob.sil_outprob));
	memset(past_prob.filler_prob, 0, sizeof(past_prob.filler_prob));

	for (int k = 0; k < keyword_num; k++)
		memset(past_prob.kw_prob[k], 0, sizeof(float)*wsmooth_length);


	for (int k = 0; k < keyword_num; k++)
		memset(keyword_prob_ring[k], 0x00, sizeof(float)*wmax);
}

bool CDetectorWord::reset()
{
	clear();

	return true;
}

int CDetectorWord::getError() { return err; }
// #define _CM_THRESHOLD2 10 // _CM_THRESHOLD 보다 큰 frame 수가 이보다 많으면 검출

int CDetectorWord::detect(const float prob[])
{
	proc_count++;

	// fill ring buffer  (원형 linkedList 개념)
	const int idx = proc_count % wsmooth_length;
#ifdef FILE_LOG
//	FILE *fp = fopen("keyword_posterior.txt", "a+");
//	FILE *cm_fp = fopen("cm_score.txt", "a+");
	//fprintf(fp, "kw_prob >> ");
#endif
	past_prob.sil_outprob[idx] = prob[0];
	past_prob.filler_prob[idx] = prob[1];
	
#ifdef FILE_LOG
//	fprintf(fp, "%dclass : %f ", 0, (float)prob[0]);  fprintf(fp, "%dclass : %f ", 1, (float)prob[1]);
//	fprintf(cm_fp, "%dclass : %f ", 0, (float)prob[0]);  fprintf(cm_fp, "%dclass : %f ", 1, (float)prob[1]);
#endif
	for (int k = 0; k < keyword_num; k++){       //keyword_num = class number - 2 (2 = silence + filler)
		past_prob.kw_prob[k][idx] = prob[k + 2];
		// Detect Starting point
		if (past_prob.kw_prob[0][idx] > 0.8){
			trg_sp_count++;
			if (trg_sp_count == 10){
				sp_detected_frame = proc_count-10-10 ;   // -10 : trg_starting point frame , -10 : concat_after_frame
#ifdef FILE_LOG
//				printf("Proc_count = %d \n",proc_count);
//				printf("sp_detected_frame = %d \n", sp_detected_frame);
#endif
			}
		}else{
			trg_sp_count = 0;
		}
#ifdef FILE_LOG
//		fprintf(fp, "%dclass : %f ", k+2, (float)prob[k+2]);
#endif
	}
#ifdef FILE_LOG
//	fprintf(fp, "\n");
#endif

	const int idx_key_prob = proc_count % wmax;

	for (int k = 0; k < keyword_num; k++){
		keyword_prob_ring[k][idx_key_prob] = posterior_smoothing_ring_buffer(past_prob.kw_prob[k], proc_count + 1);
#ifdef FILE_LOG
		//fprintf(fp, "%dclass : %f ", k + 2, (float)prob[k + 2]);
//		fprintf(cm_fp, "%dclass : %f ", k + 2, (float)prob[k + 2]);
#endif
	}
#ifdef FILE_LOG
	//fprintf(fp, "\n");
//	fprintf(cm_fp, "\n");
#endif
	
	float kw_cm_score = calc_confidence_score_ring_buffer(keyword_prob_ring, proc_count + 1, keyword_num);
#ifdef FILE_LOG
//	if (kw_cm_score >= 0.7)
//		fprintf(cm_fp, "kw_cm_score = %f\n", (float)kw_cm_score);
#endif


	// trigger word 발성 유/무 체크
	int detected = 0;
	if (sp_detected_frame >= 0 && prob_thr < kw_cm_score)
	{
		trigger_word_count++;
		if (trigger_word_count > _CM_THRESHOLD2                                                                   // cm_socre  (confidence measure score)가 prob_thr 보다 큰 frame이 지속적으로 _CM_THRESHOLD2(10) 번 이상이어야 함
			&& keyword_prob_ring[keyword_num-2][idx_key_prob] < keyword_prob_ring[keyword_num-1][idx_key_prob]    // 마지막 2 class 에 대해서 가장 마지막 class의 posterior smoothing 값이 마지막에서 두번째 posterior smoothing 값 보다 커야 함
			)
		{
			detected = 1;
		}
	}
	else
	{
		trigger_word_count = 0;
	}
#ifdef FILE_LOG
//	if (kw_cm_score >= 0.7  && detected == 1)
//		fprintf(fp, "final kw_cm_score = %f\n", (float)kw_cm_score);
//	
//	fclose(fp);
//	fclose(cm_fp);
#endif

	//printf("frame = %d : sil_prob = %f : hi_prob = %f : dio_prob = %f : filler_prob = %f : past_hi_prob = %f : past_dio_prob = %f : kw_cm_score = %f\n",
	//	proc_count, past_prob.sil_outprob[proc_count%wsmooth_length], past_prob.kw_prob[0][proc_count%wsmooth_length],
	//	past_prob.kw_prob[1][proc_count%wsmooth_length], past_prob.filler_prob[proc_count%wsmooth_length], 
	//	keyword_prob_ring[0][idx_key_prob], keyword_prob_ring[1][idx_key_prob], kw_cm_score);
	//printf("wcount = %d\n", trigger_word_count);

	if (clog_id)
		clog_debug(CLOG(clog_id), "%d\t%d\t%f\t%f\t%f", proc_count, trigger_word_count, kw_cm_score,
			keyword_prob_ring[0][idx_key_prob], keyword_prob_ring[1][idx_key_prob]);

    frigger_frame_len = proc_count - sp_detected_frame;

	if (!detected)
		return 0;

	this->clear();
	return 1;
}

int CDetectorWord::getTriggerFrameLen(){
    return (frigger_frame_len);
}