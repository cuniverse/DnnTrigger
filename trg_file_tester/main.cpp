#include <stdio.h>
#include <string.h>
#include <memory>

#include <sndfile.hh>
#pragma comment(lib, "libsndfile-1")

#include "dnn_trigger_decoder/dnn_trigger.h"
#include "dnn_trigger_decoder/mono_trigger.h"
#pragma comment(lib, "dnn_trigger")

#if defined(unix) || defined(__unix__) || defined(__linux__)
#define _strnicmp strncasecmp
#endif
#ifndef MAX_PATH
#define MAX_PATH 260
#endif

using std::unique_ptr;

char *trimwhitespace(char *str)
{
	// Trim leading space
	while (0 <= *str && isspace(*str)) str++;

	if (*str == 0)  // All spaces?
		return str;

	// Trim trailing space
	char* end = str + strlen(str) - 1;
	while (end > str && 0 <= *end && isspace(*end)) end--;

	// Write new null terminator
	*(end+1) = 0;

	return str;
}


const char *get_filename_ext(const char *filename) {
	const char *dot = strrchr(filename, '.');
	if (!dot) return "";
	return dot;
}


// libsndfile을 사용하여 경로에 있는 소리 파일 오픈
// RAW 파일인 경우 raw_sf를 지정
static SndfileHandle OpenSoundFile(const char fname[], const int raw_sf = 16000)
{
	const char* ext = get_filename_ext(fname);	// 파일 확장자

	int sf_format = 0, sf_channel = 0, sf_samplerate = 0;
	if (!_strnicmp(ext, ".pcm", 5) || !_strnicmp(ext, ".raw", 5) || !_strnicmp(ext, ".dat", 5))
	{	// raw 포맷인 경우 처리
		sf_format = SF_FORMAT_RAW | SF_FORMAT_PCM_16;
		sf_channel = 1;	sf_samplerate = raw_sf;
	}

	SndfileHandle sfh(fname, SFM_READ, sf_format, sf_channel, sf_samplerate);
	if (SF_ERR_NO_ERROR != sfh.error())	printf("%s\n%s\n", fname, sfh.strError());

	return sfh;
}


// 다분할 키워드 테스트
int testSndFile(const char fname[])
{
	SndfileHandle sfh = OpenSoundFile(fname);
	if (SF_ERR_NO_ERROR  != sfh.error()) {
		printf("%s\n%s\n", fname, sfh.strError());
		return -1;
	}

	if (1 != sfh.channels())
	{
		puts("only 1 channel file supported.\n");
		return -2;
	}


	CDnnTrigger dnn_trigger("./", "../conf/diotrg_16k.ini");
	if (dnn_trigger.getError())
	{
		printf("error loading trigger engine\n");
		return -5;
	}

	long feat_size = 0;
	int kw_detected = 0;
	unique_ptr<float[]> feat_out(new float[40960 + 10]());
	short pcm_buf[160];
	auto samp_len = sfh.frames();
	//printf("Sound length: %d samples (%d frames)\n", samp_len, samp_len/160);

	for (int f = 0; f < samp_len - 160; f += 160)
	{
		sfh.readf(pcm_buf, 160);
		int spinfo[2];
		auto ret_dec = dnn_trigger.detect(160, pcm_buf,spinfo);
		if (0 < ret_dec)
		{
			//printf("KW detected at %d frame \n", dnn_trigger.getOutFrame());
			printf("%8d KW detected! -> %8d, %8d\n", kw_detected, dnn_trigger.getOutFrame() - spinfo[1],dnn_trigger.getOutFrame());
			kw_detected++;
		}
	}

	return kw_detected;
}

// monophone 테스트
int testSndFile(const char fname[], const char keyword[])
{
	SndfileHandle sfh = OpenSoundFile(fname);
	if (SF_ERR_NO_ERROR  != sfh.error()) {
		printf("%s\n%s\n", fname, sfh.strError());
		return -1;
	}

	if (1 != sfh.channels())
	{
		puts("only 1 channel file supported.\n");
		return -2;
	}

	//CDnnTrigger dnn_trigger("./", "../conf/diotrg_16k.ini");
	CMonoTrigger dnn_trigger("./", "../conf/diotrg_mono_16k.ini");
	if (dnn_trigger.getError())
	{
		printf("error loading trigger engine\n");
		return -5;
	}
	dnn_trigger.addPhonSeq(keyword);

	long feat_size = 0;
	int kw_detected = 0;
	unique_ptr<float[]> feat_out(new float[40960 + 10]());
	short pcm_buf[160];
	auto samp_len = sfh.frames();
	//printf("Sound length: %d samples (%d frames)\n", samp_len, samp_len/160);

	for (int f = 0; f < samp_len - 160; f += 160)
	{
		sfh.readf(pcm_buf, 160);

		auto ret_dec = dnn_trigger.detect(160, pcm_buf);
		if (0 < ret_dec)
		{
			printf("KW detected at %d frame \n", dnn_trigger.getOutFrame());
			kw_detected++;
		}
	}

	return kw_detected;
}



int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		puts("Usage:\n"
			"trg_file_tester.exe filepath [keyword]\n"
			"trg_file.tester.exe listpath");
		return -1;
	}
	const char* fname = argv[1];
	const char* ext = get_filename_ext(fname);	// 파일 확장자

	// not list file
	if (_strnicmp(ext, ".txt", 5) && _strnicmp(ext, ".list", 6))
	{
		int kw_det;
		if (argc < 3)
			kw_det = testSndFile(fname);
		else
			kw_det = testSndFile(fname, argv[2]);
		printf("%s\t%d\n", fname, kw_det);
		return 0;
	}

	// list file
	int file_count = 0;
	int det_file_count = 0;
	char sz_line[MAX_PATH] = { 0 };
	FILE* list_file = fopen(fname, "r");
	while (NULL != fgets(sz_line, sizeof(sz_line), list_file))
	{
		//char* line_trimmed = trimwhitespace(sz_line);
		//if ('\0' == line_trimmed[0])	continue;

		char string1[255] = { 0 };
		char string2[255] = { 0 };

		sscanf(sz_line, "%[^\t]\t%[^\n]", string1, string2);

		const char* str_path = trimwhitespace(string1);
		const char* str_trn = trimwhitespace(string2);
		std::string snd_path(str_path);


		// check existance of file on both abs and rel paths
		FILE* fp_test = fopen(snd_path.c_str(), "r");
		if (NULL == fp_test)
		{
			std::string parent_path(fname);
			parent_path = parent_path.substr(0, parent_path.find_last_of("/\\")+1);
			snd_path = parent_path + snd_path;

			fp_test = fopen(snd_path.c_str(), "r");
		}

		if (NULL == fp_test)
		{
			printf("file not exists:\n%s\n", snd_path.c_str());
			continue;
		}
		fclose(fp_test);
		file_count++;

		int kw_det;
		if (2 < argc)
		{
			kw_det = testSndFile(snd_path.c_str(), argv[2]);
		}
		else if (str_trn)
		{
			kw_det = testSndFile(snd_path.c_str(), str_trn);
		}
		else
		{
			kw_det = testSndFile(snd_path.c_str());
		}
		printf("%s\t%d\n\n", snd_path.c_str(), kw_det);
		if (kw_det < -2) { puts("critical error!"); break; }

		if (0 < kw_det) det_file_count+= kw_det;
	}

	printf("kw detected: %d / %d (%f%%)\n", det_file_count, file_count, 100.f*det_file_count/file_count);

	return 0;
}
