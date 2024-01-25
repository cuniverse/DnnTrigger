#include "detector_mono.h"

#include <float.h>
#include <math.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

#include <algorithm>
#include <iostream>
#include <string>

#include "clog.h"

#define MININI_ANSI
#define INI_READONLY 
#include "minIni.h"

#include "lexicon/text2lex.h"
#pragma comment(lib, "Feat2Pass")
#pragma comment(lib, "FrontEnd")

#ifndef _MAX_PATH
#define _MAX_PATH 255
#endif

//#define wsmooth_length 10
#define wsmooth_length 30
#define wmax 300

#include <sstream>
namespace std
{
	template < typename T > std::string to_string( const T& n )
	{
		std::ostringstream stm ;
		stm << n ;
		return stm.str() ;
	}
}


const static char* mono_hangul[] = {
	  "#",  "ㅏ", " ㅂ", " ㅃ", " ㅊ", " ㄷ", " ㄸ",  "ㅔ", " ㄱ", " ㄲ",
	" ㅎ",  "ㅣ", " ㅈ", " ㅉ", " ㅋ", "ㄱ ", "ㄹ ",  "ㅁ",  "ㄴ",  "ㅗ",
	"ㅇ ", " ㅍ", "ㅂ ", " ㄹ", " ㅅ", " ㅆ", " ㅌ", "ㄷ ",  "ㅜ",  "ㅟ",
	 "ㅓ",  "ㅡ",  "ㅘ",  "ㅞ",  "ㅢ",  "ㅝ",  "ㅑ",  "ㅖ",  "ㅛ",  "ㅠ",
	 "ㅕ",
};

typedef struct ex_phon {
	bool alive = true;
	char phon = 0;
	float max_prob = 0.f;
} ex_phon;

typedef struct phoneseq_worker {
	std::string history;	// 검출된 kw phone열 기록 (한글) (디버깅용)
	std::string all_hist;	// 검출된 ex+kw phone열

	int frm_begin;
	int frm_end;	// 지금까지 검출된 kw의 끝 위치
	int window_len;	// look-foward window size

	int phon_begin;	// 검출된 마지막 phone의 시작 위치

	int detected_phon;
	int all_phon;

	std::vector<char> detected_phons;
	std::vector<ex_phon> ex_pool;	// out of interest phones
} phoneseq_worker;


static const char symbol2phoneidx[42] = {
	15, 16, 17, 18, 20, 22,
	27, 31, 34,  1,  2,  3,
	 4,  5,  6,  7,  8,  9,
	10, 11, 12, 13, 14, 17,
	18, 19, 21, 23, 24, 25,
	26, 28, 29, 30, 32, 33,
	35, 36, 37, 38, 39, 40
};


const static char* mono_hangul_merged[] = {
	  "#",  "ㅏ", " ㅂ", " ㅃ", " ㅊ", " ㄷ", " ㄸ",  "ㅔ", " ㄱ", " ㄲ",
	" ㅎ",  "ㅣ", " ㅈ", " ㅉ", " ㅋ", "ㄱ ", "ㄹ ",  "ㅁ",  "ㄴ",  "ㅗ",
	"ㅇ ", " ㅍ", "ㅂ ", " ㄹ", " ㅅ", " ㅆ", " ㅌ", "ㄷ ",  "ㅜ",  "ㅑ",
	 "ㅓ",  "ㅡ",  "ㅛ",  "ㅠ", " ㅕ",
};

static const char sym2phon_merged[42] = {	// 이중모음 병합된 테이블
	15, 16, 17, 18, 20, 22,
	27, 31, 11,  1,  2,  3,
	 4,  5,  6,  7,  8,  9,
	10, 11, 12, 13, 14, 17,
	18, 19, 21, 23, 24, 25,
	26, 28, 11, 30,  1,  7,
	30, 29,  7, 32, 33, 34
};

static const float prob_smooth_len[] = {
	27.023861,  7.815500, 11.749150, 11.070766, 13.338693,
	10.842304,  9.695746,  8.333520, 12.009899,  8.531265,
	10.237376,  9.326945, 11.882619,  8.824042, 11.530241,
	 8.505893,  7.932478,  8.956997,  8.119765,  8.413376,
	 7.241573, 13.268901,  8.277426,  7.700892, 11.858909,
	 6.657240, 10.834931,  7.039269,  8.972683, 10.817489,
	 8.507745,  7.443124, 13.952400, 14.276915, 10.119273,
};

CDetectorMono::CDetectorMono(const int phon_num, const char root_path[], const char config_path[])
{
	this->phon_num = phon_num;
	clog_id = 0;

	char ini_config[_MAX_PATH];
	snprintf(ini_config, sizeof(ini_config), "%s/%s", root_path, config_path);


	// tigger settings
	prob_thr = ini_getf("mono", "prob_threshold", -1.f, ini_config);
	if (prob_thr <= 0.f) { err = 4; return; }

	pause_thr = ini_getl("mono", "pause_threshold", -1, ini_config) / 10;
	if (pause_thr < 0) { err = 5; return; }

	score_thr = ini_getf("mono", "score_threshold", -1.f, ini_config);
	if (pause_thr < 0.f) { err = 5; return; }

	// memory alloc
	past_prob = new float*[phon_num]();
	for (int i = 0; i < phon_num; i++)
		past_prob[i] = new float[wsmooth_length]();

	phon_prob_ring = new float*[phon_num]();
	for (int i = 0; i < phon_num; i++)
		phon_prob_ring[i] = new float[wmax]();
	clear();

	if (41 == phon_num)
	{
		sym2phon = symbol2phoneidx;
		hangul_table = mono_hangul;
	}
	else if (35 == phon_num)
	{
		sym2phon = sym2phon_merged;
		hangul_table = mono_hangul_merged;
	}
	else
	{
		err = 6;
		return;
	}

	err = 0;
}


CDetectorMono::~CDetectorMono()
{
	int seqs = phon_seqs.size();
	for (int s = 0; s < seqs; s++) {
		std::vector<phoneseq_worker*>& work_seq = work_seq_pool[s];
		int seq_len = work_seq.size();
		for (int p = 0; p < seq_len; p++)	// phone
		{
			delete work_seq[p];
			work_seq[p] = NULL;
		}
	}

	for (int i = 0; i < phon_num; i++)
		delete[] past_prob[i];
	delete[] past_prob;

	for (int i = 0; i < phon_num; i++)
		delete[] phon_prob_ring[i];
	delete[] phon_prob_ring;
}


void CDetectorMono::setClog(int log_id)
{
	clog_id = log_id;
}


// make hangul word into monophone sequence
std::string CDetectorMono::word2phone(const char word[])
{
	VOC_LEXICON test_voc;
	int max_lex_cnt = 4;	//다중 발음열 개수

	printf("트리거워드 = %s\n", word);

	TEXT2PHONE_convertTextIntoLexicons(word, &test_voc, max_lex_cnt);

	std::string phone_seq(test_voc.pronDict[0].nLenPhoneSeq, '\0');
	for (int i = 0; i < test_voc.pronDict[0].nLenPhoneSeq; i++)
		phone_seq[i] = sym2phon[test_voc.pronDict[0].PhoneSeq[i]];

	return phone_seq;
}


static float prob_variable_smooth(const float prob[], const int frame_idx, const int smooth_len)
{
	const int hsmooth_start = std::max(0, frame_idx - smooth_len);
	const int hsmooth_len = frame_idx - hsmooth_start;
	if (hsmooth_len <= 0)	return 0.f;

	float accum_post_prob = 0.f;
	for (int k = hsmooth_start; k < frame_idx; k++)
		accum_post_prob += prob[k%wsmooth_length];

	float post_outprob = accum_post_prob / hsmooth_len;

	return post_outprob;
}


static float calc_prob_max_sum(float** const post_dnn_outprob, const int start_idx, const int end_idx, std::vector<char> phon_set)
{
	if (start_idx < 0
		|| end_idx < 0
		|| end_idx - start_idx <= 0
		|| 0 == phon_set.size())
		return 0.f;

	float cm_score = 0.f;
	for (auto i = phon_set.begin(); i != phon_set.end(); i++)
	{
		float max_class_prob = 0.f;
		for (int k = start_idx; k < end_idx; k++)
			max_class_prob = std::max(max_class_prob, post_dnn_outprob[*i][k%wmax]);

		cm_score += max_class_prob;
	}
	if (cm_score < FLT_MIN)	return 0.f;
	return cm_score;
}


// 검출할 phone열 추가 (0-terminate된 C style 문자열)
int CDetectorMono::addPhonSeq(const char sequence[])
{
	if (!sequence)	return -1;
	if (strnlen(sequence, 30) <= 0)	return -2;

	std::string phone_seq = word2phone(sequence);

	phon_seqs.push_back(phone_seq);
	work_seq_pool.push_back(std::vector<phoneseq_worker*>(phone_seq.length(), nullptr));
	return phon_seqs.size();
}


// 검출할 phone열 추가 (std::string)
int CDetectorMono::addPhonSeq(const std::string sequence)
{
	if (sequence.length() <= 0)	return -2;

	std::string phone_seq = word2phone(sequence.c_str());

	phon_seqs.push_back(phone_seq);
	work_seq_pool.push_back(std::vector<phoneseq_worker*>(phone_seq.length(), nullptr));
	return phon_seqs.size();
}


// clear buffer to continue word detection
void CDetectorMono::clear()
{
	proc_count = -1;

	int seqs = phon_seqs.size();
	for (int s = 0; s < seqs; s++) {
		std::vector<phoneseq_worker*>& work_seq = work_seq_pool[s];
		int seq_len = work_seq.size();
		for (int p = 0; p < seq_len; p++)	// phone
		{
			delete work_seq[p];
			work_seq[p] = NULL;
		}
	}

	for (int k = 0; k < phon_num; k++)
		memset(past_prob[k], 0, sizeof(float)*wsmooth_length);

	for (int k = 0; k < phon_num; k++)
		memset(phon_prob_ring[k], 0, sizeof(float)*wmax);
}


// clear detector to be ready for another sound stream
// TODO: clear와 reset 명확히 정의
bool CDetectorMono::reset()
{
	clear();

	phon_seqs.clear();
	work_seq_pool.clear();

	return true;
}


int CDetectorMono::detect(const float prob[])
{
	proc_count++;

	// fill ring buffer
	const int idx = proc_count % wsmooth_length;
	for (int k = 0; k < phon_num; k++)
		past_prob[k][idx] = prob[k];

	const int idx_key_prob = proc_count % wmax;
	for (int k = 0; k < phon_num; k++)
		phon_prob_ring[k][idx_key_prob] = prob_variable_smooth(past_prob[k], proc_count, (int)round(prob_smooth_len[k]/2));

#if 0
	if (_clog_loggers[clog_id] 
		&& CLOG_DEBUG == _clog_loggers[clog_id]->level)
	{

		std::string prob_str;
		for (int k = 0; k < phon_num; k++)
		{
			prob_str += "\t";
			prob_str += std::to_string(phon_prob_ring[k][idx_key_prob]);
		}
		clog_debug(CLOG(clog_id), prob_str.c_str());


		const int idx_key_prob1 = (proc_count-1) % wmax;
		for (int k = 0; k < phon_num; k++)
		{
			if (phon_prob_ring[k][idx_key_prob1] < prob_thr
				&& prob_thr < phon_prob_ring[k][idx_key_prob])
				printf("%s", hangul_table[k]);
		}
	}
#endif

	int detected = 0;
	const int seqs = phon_seqs.size();
	for (int s = 0; s < seqs; s++) {	// phone sequence
		const std::string& seq = phon_seqs[s];
		std::vector<phoneseq_worker*>& work_seq = work_seq_pool[s];

		int seq_len = seq.size();
		for (int p = seq_len-1; 0 <= p; p--)	// phone
		{
			phoneseq_worker* w = work_seq[p];
			if (!w)	continue;

			char last_phon = seq[p];
			if (prob_thr < phon_prob_ring[last_phon][idx_key_prob])
				w->frm_end = proc_count;

			for (int i = 0; i < w->window_len; i++)	// phone window
			{
				if (seq_len <= p+i+1)	// word detected
				{
					float sum_target = calc_prob_max_sum(phon_prob_ring, w->frm_begin, proc_count, w->detected_phons);

					// ooc 확률 갱신
					float sum_ex = 0.f;
					for (auto i = w->ex_pool.begin(); i != w->ex_pool.end(); i++)
					{
						sum_ex += (*i).max_prob;
					}
#ifndef __ANDROID__	// for Android build (Android toolchain does not support to_string)
					if (_clog_loggers[clog_id])
					{
						std::string log_str;
						log_str += "\t" + std::to_string(w->frm_end - w->frm_begin);
						log_str += "\t" + std::to_string(w->detected_phon);
						log_str += "\t" + std::to_string(w->all_phon);
						log_str += "\t" + w->history;
						log_str += "\t" + w->all_hist;
						log_str += "\t" + std::to_string(sum_target);
						log_str += "\t" + std::to_string(sum_ex);
						log_str += "\t" + std::to_string(sum_target/sum_ex);

						std::cout << log_str << std::endl;
						clog_debug(CLOG(clog_id), log_str.c_str());
					}
#endif	// !_GLIBCXX_HAVE_BROKEN_VSWPRINTF
					if (sum_ex <= 0.f || score_thr < sum_target/sum_ex)	// 마지막으로 확률 판정을 실시
						detected = s+1;

					// 확률 판정에 통과 못하더라도 계속 남는다.
					// 이후 phone 검출에 의해 확률이 초과되어 valid되거나, timeout되면 사라진다.
					break;
				}

				char phon = seq[p+i+1];

				// 윈도우 안에 같은 phone가 중복된 경우 예외처리
				//if (last_phon == phon)	continue;

				// 최소 phone길이를 구한다
				// 최소 phone길이에 미달하는 경우 pass
				int len_min_phon = 0;
				for (int m = 0; m <= i; m++)
				{
					char min_phon = seq[p+m];
					len_min_phon += (int)round(prob_smooth_len[min_phon]/2);
				}

				if (proc_count - w->phon_begin < len_min_phon)
					continue;

				if (phon_prob_ring[phon][idx_key_prob] < prob_thr)	continue;	// phone 미검출

				// phone detected
				phoneseq_worker* w_new = new phoneseq_worker(*w);	// copy
				w_new->frm_end = proc_count;
				w_new->phon_begin = proc_count;
				w_new->window_len -= i;
				w_new->history += hangul_table[phon];

				w_new->detected_phon += 1;
				w_new->all_phon += 1;
				w_new->all_hist += hangul_table[phon];

				w_new->detected_phons.push_back(phon);

				phoneseq_worker* w_toreplace = work_seq[p+i+1];
				if (w_toreplace && w_new->window_len <= w_toreplace->window_len)	// 윈도우가 더 큰게 남아있다면 병합
				{
					if (0 == i)
					{
						delete work_seq[p];
						work_seq[p] = NULL;
					}
					delete w_new;
					break;
				}

				delete work_seq[p+i+1];	// delete existing worker
				work_seq[p+i+1] = w_new;	// put worker to new position
				if (0 == i)
				{
					delete work_seq[p];
					work_seq[p] = NULL;
				}
				break;
			}	// i = phone window


		}	// p = phone
		if (detected)	break;


		// 시작부분 phone 검출
		int win = seq_len / 3;
		for (int i = 0; i < win; i++)	// phone window
		{
			if (seq_len < i)	return -3;	// sequence length보다 window 크기가 크다, 뭔가 이상한 경우임!

			if (work_seq[i])	continue;	// 이미 해당 phone이 검출된 경우 skip

			char phon = seq[i];
			if (phon_prob_ring[phon][idx_key_prob] < prob_thr)	continue;	// phone 미검출

			// phone detected
			phoneseq_worker* w = new phoneseq_worker();
			w->frm_begin = proc_count;
			w->frm_end = proc_count;
			w->phon_begin = proc_count;
			w->window_len = win - i;
			w->history += hangul_table[phon];
			w->all_hist += hangul_table[phon];

			w->detected_phon = 1;
			w->all_phon = 1;

			w->detected_phons.push_back(phon);

			work_seq[i] = w;	// move worker to new position
			break;
		}

		for (int p = 0; p < seq_len; p++)	// phone
		{
			phoneseq_worker* w = work_seq[p];
			if (!w)	continue;

			// remove timeout
			if (w->frm_end + pause_thr < proc_count)
			{
				//if (CLOG_DEBUG == _clog_loggers[clog_id]->level)
				//{
				//	printf("\nt\t");
				//	puts(w->history.c_str());
				//}
				delete work_seq[p];
				work_seq[p] = NULL;
				continue;
			}

			// ex phone 확률 갱신
			for (auto i = w->ex_pool.begin(); i != w->ex_pool.end(); i++)
			{
				if (!(*i).alive)
					continue;

				char k = (*i).phon;
				if (k == seq[p])
				{
					(*i).alive = false;
					continue;
				}

				(*i).max_prob = std::max((*i).max_prob, phon_prob_ring[k][idx_key_prob]);
			}

			// detect non-kw phone(ex phone)
			const int idx_key_prob1 = (proc_count-1) % wmax;
			for (int k = 0; k < phon_num; k++)
			{
				if (!(
					phon_prob_ring[k][idx_key_prob1] < prob_thr
					&& prob_thr < phon_prob_ring[k][idx_key_prob])
					)
					continue;

				// phone prob이 threshold를 넘긴 경우

				if (k == seq[p]) {	// 마지막으로 검출된 phone와 같은 경우
					continue;
				}

				// 검출중인 phone은 제외
				bool detecting_phone = false;
				for (int i = 0; i < w->window_len; i++)	// phone window
				{
					char phon = seq[p+i+1];
					if (k == phon)
					{
						detecting_phone = true;
						break;
					}
				}

				if (detecting_phone)
					continue;


				if (0 != w->ex_pool.size()
					&& k == w->ex_pool.back().phon)	// 이전 OOC phone와 같은 경우
					continue;

				w->all_phon += 1;
				w->all_hist += hangul_table[k];
				
				ex_phon exp;
				exp.phon = k;
				exp.max_prob = phon_prob_ring[k][idx_key_prob];
				w->ex_pool.push_back(exp);
			}
		}
	}

	if (detected)	clear();

	return detected;
}
