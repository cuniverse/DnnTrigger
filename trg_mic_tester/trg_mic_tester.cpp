// trg_mic_tester.cpp

#include <stdio.h>
#include <string.h>

#include <iostream>

#include <AL/al.h>
#include <AL/alc.h>

#ifdef _WIN32
#pragma comment(lib, "OpenAL32")

#define NOMINMAX
#include <conio.h>
#include <Windows.h>
#include <atlstr.h>

#else
#include <unistd.h>
const int Sleep(int sleepMs) { return usleep(sleepMs * 1000); }

/**
Linux (POSIX) implementation of _kbhit().
Morgan McGuire, morgan@cs.brown.edu
*/
#include <sys/select.h>
#include <sys/ioctl.h>
#include <termios.h>
// #include <stropts.h>

int _kbhit() {
	static const int STDIN = 0;
	static bool initialized = false;

	if (!initialized) {
		// Use termios to turn off line buffering
		termios term;
		tcgetattr(STDIN, &term);
		term.c_lflag &= ~ICANON;
		tcsetattr(STDIN, TCSANOW, &term);
		setbuf(stdin, NULL);
		initialized = true;
	}

	int bytesWaiting;
	ioctl(STDIN, FIONREAD, &bytesWaiting);
	return bytesWaiting;
}

#endif	// _WIN32 else

#include "dnn_trigger_decoder/dnn_trigger.h"
//#include "dnn_trigger_decoder/mono_trigger.h"
#pragma comment(lib, "dnn_trigger")


int main(int argc, char* argv[])
{
#ifdef _WIN32
	char exec_path[MAX_PATH];
	GetModuleFileNameA(NULL, exec_path, sizeof(exec_path) - 1);
	*(strrchr(exec_path, '\\')+1) = '\0';	// parent of excutable
#else
	char exec_path[] = "./";
#endif

	CDnnTrigger dnn_trigger(exec_path, "../conf/diotrg_16k.ini");
	//CMonoTrigger dnn_trigger(exec_path, "../conf/diotrg_mono_16k.ini");
	if (dnn_trigger.getError())
	{
		printf("error loading trigger engine\n");
		return -5;
	}

	//dnn_trigger.addPhonSeq("�����̼���");
	//dnn_trigger.addPhonSeq("�����̱���");


	puts("---------- All Capture Devices list");
	
	const ALchar* devs = alcGetString(NULL, ALC_CAPTURE_DEVICE_SPECIFIER);
	const ALchar* dev_name = alcGetString(NULL, ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER);
	for (int i = 0; '\0' != devs[i]; i += strlen(&devs[i])+1)
	{
#ifdef _WIN32
		std::cout << CW2A(CA2W(&devs[i], CP_UTF8)) << std::endl;
#else
		fprintf(stdout, "%s\n", &devs[i]);
#endif

		if (0 == strncmp(&devs[i], "USB", 3))
			dev_name = &devs[i];
	}
	puts("----------\n");
	puts("Selected recording device:");
#ifdef _WIN32
	std::cout << CW2A(CA2W(dev_name, CP_UTF8)) << std::endl;
#else
	fprintf(stdout, "%s\n", dev_name);
#endif


	int16_t pcm_buf[16000];

	alGetError();
	ALCdevice *device = alcCaptureOpenDevice(dev_name, 16000, AL_FORMAT_MONO16, sizeof(pcm_buf)/sizeof(pcm_buf[0]));
	auto alerror = alcGetError(device);
	if (alerror != AL_NO_ERROR) {
		printf("Sound device error! (%d)\n", alerror);
		return 0;
	}

	alcCaptureStart(device);
	puts("\nListening...");

	FILE* log_pcm = NULL;
	//log_pcm = fopen("log_in.pcm", "wb");	// uncomment to log PCM audio
	FILE* log_feat = NULL;
	//log_feat = fopen("log_feat.mfc", "wb");	// uncomment to log feat data

	int frame_cnt = -1;	// first frame will be frame 0

	int kw_count = 0;

	while (!_kbhit())
	{
		ALCint buf_size;
		alcGetIntegerv(device, ALC_CAPTURE_SAMPLES, 1, &buf_size);
		if (buf_size <= 0)
		{
			Sleep(100);
			continue;
		}

		alcCaptureSamples(device, (ALCvoid *)pcm_buf, buf_size);

		if (log_pcm)
			fwrite(pcm_buf, sizeof(int16_t), buf_size, log_pcm);
		int spinfo[2];
		auto ret_detect = dnn_trigger.detect(buf_size, pcm_buf,spinfo);
		if (0 < ret_detect)
		{
			kw_count++;
			printf("%8d KW detected! -> %8d, %8d\n", kw_count, ret_detect - spinfo[1],ret_detect);
		}
	}

	if (log_pcm)	fclose(log_pcm);
	if (log_feat)	fclose(log_feat);

	alcCaptureStop(device);
	alcCaptureCloseDevice(device);

	return 0;
}
