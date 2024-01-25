#include <stdio.h>
#include <string.h>
#include <memory>

#include <sndfile.hh>
#pragma comment(lib, "libsndfile-1")

#include "Selvy_Trigger_API.h"
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


int testSndFile(const char fname[])
{
	FILE * _f = fopen(fname,"rb");

	Selvy_DNN_Trigger dnn_trigger("./", "../conf/diotrg_16k.ini"); //실행 파일 
	if (dnn_trigger.getError())
	{
		printf("error loading trigger engine\n");
		return -5;
	}
	fseek(_f, 0, SEEK_END);
	long fsize = ftell(_f);
	fseek(_f, 0, SEEK_SET);  //same as rewind(f);
	long feat_size = 0;
	int kw_detected = 0;
	short pcm_buf[160];
	auto samp_len = fsize / sizeof(short);

	//printf("Sound length: %d samples (%d frames)\n", samp_len, samp_len/160);
	int spinfo[2];
	for (int f = 0; f < samp_len - 160; f += 160)
	{
		fread(pcm_buf,sizeof(short),160,_f);
		auto ret_dec = dnn_trigger.detect(160, pcm_buf,spinfo);
		if (0 < ret_dec)
		{
			//printf("KW detected at %d frame \n", f);
			printf("%8d KW detected! -> %8d, %8d, %8d\n", kw_detected, spinfo[0] - spinfo[1],spinfo[0],ret_dec);
			kw_detected++;
		}
	}
	fclose(_f);
	return kw_detected;
}




int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		puts("Usage:\n"
			"trg_file_tester.exe filepath\n"
			"trg_file.tester.exe listpath");
		return -1;
	}
	const char* fname = argv[1];
	const char* ext = get_filename_ext(fname);	// ???? ?????

	if (_strnicmp(ext, ".pcm", 5) != 0)
	{
		printf("PCM FILE Only!\n");
		return 0;
	}
	int kw_det;
	kw_det = testSndFile(fname);
	printf("%s\t%d\n", fname, kw_det);
	return 0;
}
