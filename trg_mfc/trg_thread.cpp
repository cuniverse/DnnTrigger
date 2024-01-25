#include "stdafx.h"
#include "trg_thread.h"

#include <iostream>
#include <memory>

#include <AL/al.h>
#include <AL/alc.h>
#pragma comment(lib, "OpenAL32")

#include "dnn_trigger_decoder/dnn_trigger.h"
#include "dnn_trigger_decoder/mono_trigger.h"
#pragma comment(lib, "dnn_trigger")

#include "trg_mfcDlg.h"


using std::unique_ptr;

// RMS energy를 구함
double getRmsEnergy(const int len, const short buf[])
{
	long long sum = 0;
	for (int i = 0; i<len; i++)
		sum += (long)buf[i] * buf[i];

	long long mean = sum/len;

	return sqrt((double)mean);
}


DWORD WINAPI WordTriggerThread(void *lParam)
{
	Ctrg_mfcDlg *pDlg = (Ctrg_mfcDlg*)lParam;

	char exec_path[MAX_PATH];
	GetModuleFileNameA(NULL, exec_path, sizeof(exec_path) - 1);
	*(strrchr(exec_path, '\\')+1) = '\0';	// parent of excutable

	CDnnTrigger dnn_trigger(exec_path, "../conf/diotrg_16k.ini");	
	if (dnn_trigger.getError())
	{
		printf("error loading trigger engine\n");
		pDlg->InterfaceEnable(true);
		CloseHandle(pDlg->recordThread);	pDlg->recordThread = NULL;
		return -5;
	}


	puts("---------- All Capture Devices list");

	const ALchar* devs = alcGetString(NULL, ALC_CAPTURE_DEVICE_SPECIFIER);
	const ALchar* dev_name = alcGetString(NULL, ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER);
	for (int i = 0; '\0' != devs[i]; i += strlen(&devs[i]) + 1)
	{
		std::cout << CW2A(CA2W(&devs[i], CP_UTF8)) << std::endl;
		if (0 == strncmp(&devs[i], "USB", 3))
			dev_name = &devs[i];
	}
	puts("----------\n");
	puts("Selected recording device:");
	std::cout << CW2A(CA2W(dev_name, CP_UTF8)) << std::endl;
	pDlg->m_ctrShowStat.InsertString(0, CA2W(dev_name, CP_UTF8));


	int16_t pcm_buf[16000];

	alGetError();
	ALCdevice *device = alcCaptureOpenDevice(dev_name, 16000, AL_FORMAT_MONO16, sizeof(pcm_buf) / sizeof(pcm_buf[0]));
	auto alerror = alcGetError(device);
	if (alerror != AL_NO_ERROR) {
		printf("Sound device error! (%d)\n", alerror);
		return 0;
	}

	alcCaptureStart(device);


	FILE* log_pcm = NULL;
	//fopen("log_in.pcm", "wb");	// uncomment to log PCM audio

	puts("\nListening...");
	pDlg->ShowStat("Listening...\n");

	long feat_size = 0;
	int frame_cnt = -1;	// first frame will be frame 0

	int trigger_word_detect_count = 0;

	int last_out = 0;
	while (pDlg->recordingFlag)
	{
		if (33 == frame_cnt - last_out)
			pDlg->ShowTrigger("검출중");

		ALCint buf_size;
		alcGetIntegerv(device, ALC_CAPTURE_SAMPLES, 1, &buf_size);
		if (buf_size < 160)
		{
			Sleep(33);
			continue;
		}

		alcCaptureSamples(device, (ALCvoid *)pcm_buf, buf_size);

		pDlg->mCtrProgIn.SetPos(getRmsEnergy(160, pcm_buf));

		if (log_pcm)
			fwrite(pcm_buf, sizeof(short), buf_size, log_pcm);


		frame_cnt++;
		auto ret_dec = dnn_trigger.detect(buf_size, pcm_buf);

		if (0 < ret_dec)
		{
			last_out = frame_cnt;

			trigger_word_detect_count++;
			printf("Keyword detected at %d frame.\n", dnn_trigger.getOutFrame());
			pDlg->ShowStat("Keyword detected at %d frame.\n", dnn_trigger.getOutFrame());
			pDlg->ShowTrigger("오케이 셀바");
		}
	}

	if (log_pcm)	fclose(log_pcm);
	
	pDlg->InterfaceEnable(true);
	CloseHandle(pDlg->recordThread);	pDlg->recordThread = NULL;

	return 13579;
}


DWORD WINAPI MonoTriggerThread(void *lParam)
{
	Ctrg_mfcDlg *pDlg = (Ctrg_mfcDlg*)lParam;

	char exec_path[MAX_PATH];
	GetModuleFileNameA(NULL, exec_path, sizeof(exec_path) - 1);
	*(strrchr(exec_path, '\\')+1) = '\0';	// parent of excutable

	std::string keyword = CStringA(pDlg->mEditKeyword);
	if (keyword.length() < 2)
	{
		pDlg->ShowStat("Too short keyword");
		pDlg->InterfaceEnable(true);
		CloseHandle(pDlg->recordThread);	pDlg->recordThread = NULL;
		return -7;
	}

	CMonoTrigger dnn_trigger(exec_path, "../conf/diotrg_mono_16k.ini");
	//CDnnTrigger dnn_trigger(exec_path, "../conf/diotrg_16k.ini");
	if (dnn_trigger.getError())
	{
		printf("error loading trigger engine\n");
		pDlg->InterfaceEnable(true);
		CloseHandle(pDlg->recordThread);	pDlg->recordThread = NULL;
		return -5;
	}
	dnn_trigger.addPhonSeq(keyword);


	puts("---------- All Capture Devices list");

	const ALchar* devs = alcGetString(NULL, ALC_CAPTURE_DEVICE_SPECIFIER);
	const ALchar* dev_name = alcGetString(NULL, ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER);
	for (int i = 0; '\0' != devs[i]; i += strlen(&devs[i]) + 1)
	{
		std::cout << CW2A(CA2W(&devs[i], CP_UTF8)) << std::endl;
		if (0 == strncmp(&devs[i], "USB", 3))
			dev_name = &devs[i];
	}
	puts("----------\n");
	puts("Selected recording device:");
	std::cout << CW2A(CA2W(dev_name, CP_UTF8)) << std::endl;
	pDlg->m_ctrShowStat.InsertString(0, CA2W(dev_name, CP_UTF8));


	int16_t pcm_buf[16000];

	alGetError();
	ALCdevice *device = alcCaptureOpenDevice(dev_name, 16000, AL_FORMAT_MONO16, sizeof(pcm_buf) / sizeof(pcm_buf[0]));
	auto alerror = alcGetError(device);
	if (alerror != AL_NO_ERROR) {
		printf("Sound device error! (%d)\n", alerror);
		pDlg->ShowStat("Sound device error! (%d)\n", alerror);
		return 0;
	}

	alcCaptureStart(device);


	FILE* log_pcm = NULL;
	log_pcm = fopen("log_in.pcm", "wb");	// uncomment to log PCM audio


	puts("Listening...\n");
	pDlg->ShowStat("Listening...\n");
	pDlg->ShowTrigger("검출중");

	long feat_size = 0;
	int frame_cnt = -1;	// first frame will be frame 0

	int trigger_word_detect_count = 0;

	int last_out = 0;
	while (pDlg->recordingFlag)
	{
		if (33 == frame_cnt - last_out)
			pDlg->ShowTrigger("검출중");

		ALCint buf_size;
		alcGetIntegerv(device, ALC_CAPTURE_SAMPLES, 1, &buf_size);
		if (buf_size < 160)
		{
			Sleep(33);
			continue;
		}

		alcCaptureSamples(device, (ALCvoid *)pcm_buf, buf_size);

		pDlg->mCtrProgIn.SetPos(getRmsEnergy(160, pcm_buf));

		if (log_pcm)
			fwrite(pcm_buf, sizeof(short), buf_size, log_pcm);

		frame_cnt++;
		auto ret_dec = dnn_trigger.detect(buf_size, pcm_buf);

		if (0 < ret_dec)
		{
			last_out = frame_cnt;

			trigger_word_detect_count++;
			printf("Keyword detected at %d frame.\n", dnn_trigger.getOutFrame());
			pDlg->ShowStat("Keyword detected at %d frame.\n", dnn_trigger.getOutFrame());
			pDlg->ShowTrigger(keyword.c_str());
		}
	}

	if (log_pcm)	fclose(log_pcm);

	pDlg->InterfaceEnable(true);
	CloseHandle(pDlg->recordThread);	pDlg->recordThread = NULL;

	return 13579;
}

