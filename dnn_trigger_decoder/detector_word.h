#ifndef __TRIGGER_DETECTOR_WORD_H__
#define __TRIGGER_DETECTOR_WORD_H__

#define wsmooth_length 20

class CDnnDecoder;

typedef struct _all_probs {
	float sil_outprob[wsmooth_length];
	float filler_prob[wsmooth_length];
	float** kw_prob;
} all_probs;


class CDetectorWord
{
private:
	int keyword_num;

	float **keyword_prob_ring;
	int proc_count;
	int trigger_word_count;
	// start point detection ( 2018.08.29 )
	int trg_sp_count;
	int sp_detected_frame;
    int frigger_frame_len;

	int wmax;
	int _CM_THRESHOLD2;

	all_probs past_prob;

	float prob_thr;

	int err;
	int clog_id;
	void clear();

public:
	CDetectorWord(const int keyword_num, const char root_path[], const char config_path[]);
	~CDetectorWord();
	int getError();
	int detect(const float prob[]);
	void setClog(int log_id);
	bool reset();

	float posterior_smoothing_ring_buffer(const float dnn_out_prob[], const int frame_idx);
	float calc_confidence_score_ring_buffer(float **post_dnn_outprob, int const frame_idx, const int class_num);
	int getTriggerFrameLen();
};

#endif	// __TRIGGER_DETECTOR_WORD_H__
