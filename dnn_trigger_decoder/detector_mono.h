// detector_mono.h
// Word detector using monophone model
// 2016-12 mckeum


#ifndef __TRIGGER_DETECTOR_MONO_H__
#define __TRIGGER_DETECTOR_MONO_H__

#include <string>
#include <vector>


typedef struct phoneseq_worker phoneseq_worker;


class CDetectorMono
{
	int proc_count;	// 내부 frame counter, 실제 출력하고 있는 frame count와는 차이 있음

	const char* sym2phon;
	const char** hangul_table;

	int phon_num;
	float** phon_prob_ring;	// after smoothing
	float** past_prob;	// prob history
	void clear();

	std::vector<std::string> phon_seqs;
	std::vector<std::vector<phoneseq_worker*>> work_seq_pool;

	float prob_thr;
	float score_thr;
	int pause_thr;

	int err;
	int clog_id;

	std::string word2phone(const char word[]);

public:
	CDetectorMono(const int keyword_num, const char root_path[], const char config_path[]);
	~CDetectorMono();
	int addPhonSeq(const char sequence[]);
	int addPhonSeq(const std::string sequence);
	int getError() { return err; }
	int detect(const float prob[]);
	bool reset();

	void setClog(int log_id);
};

#endif	// __TRIGGER_DETECTOR_MONO_H__
