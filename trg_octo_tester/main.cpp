// HelloDirectionCon.cpp
//
// Octopus, DioDirection 콘솔 테스트
// Win32 Linux 공용 
//

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <memory>

#include <sndfile.hh>

#include "dnn_trigger_decoder/dnn_trigger.h"
#pragma comment(lib, "dnn_trigger")

#ifdef _WIN32
#	pragma comment(lib, "libsndfile-1")

#	include <conio.h>
#	include "Octopus.h"
#	pragma comment(lib, "Octopus")

#else
#	include "Octopus.h"	// Octopus Linux SDK C++ wrapper

/**
 Linux (POSIX) implementation of _kbhit().
 Morgan McGuire, morgan@cs.brown.edu
 */
#	include <sys/select.h>
#	include <sys/ioctl.h>
#	include <termios.h>
#	include <stropts.h>

int _kbhit() {
	static const int STDIN = 0;
	static bool initialized = false;

	if (! initialized) {
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

using std::unique_ptr;

#define D_FX 16000
//#define DD_WRITE_SOUND_FILE	// define to record file

static double getRmsEnergy(const int len, const int16_t buf[]);

//////////////////////////////////////////////////////////////////////////
// Globals
const int nSamplesPerFrame = D_FX / 10;	// 프레임 당 샘플수, 1Sample = 2Bytes
unsigned char* pucMicBuffer[8 * 2];	// 마이크 버퍼 포인터 변수

// 8UMA로부터 얻은 데이터를 저장할 버퍼 메모리를 할당한다.
void InitBoard()
{
	for (int i = 0; i < 8*2; i++)
		pucMicBuffer[i] = new unsigned char[D_FX / 10 * sizeof(int16_t)]();
}

// 메모리 해제
void CloseBoard()
{
	for (int i = 0; i < 8*2; i++)
		delete[] pucMicBuffer[i];
}


int main(int argc, char* argv[])
{
	// Octopus initialization
	COctopus micArr;
	auto nReturn = micArr.Create(D_FX, nSamplesPerFrame);	// 버퍼 크기는 64의 배수여야 함
	if (nReturn < 1)
	{
		printf("Error: Octopus board init failed(%d)!\n", nReturn);
		return -1;
	}
	//micArr.SetPreAmpBoost(OCTO_ALL_MICS, 0);	// Mic Boost On
	micArr.SetPGA(OCTO_ALL_MICS, 10);	// -3 ~ 12

	nReturn = micArr.StartStreaming();
	micArr.Flush();


	CDnnTrigger dnn_trigger("./", "../conf/diotrg_16k.ini");
	if (dnn_trigger.getError())
	{
		printf("error loading trigger engine\n");
		return -5;
	}

#ifdef DD_WRITE_SOUND_FILE	// Open sound log file
	time_t rawtime;	time(&rawtime);
	struct tm* timeinfo = localtime(&rawtime);

	char logfilename[32];
	strftime(logfilename, sizeof(logfilename), "%y%m%d_%H%M%S_0.flac", timeinfo);
	SndfileHandle sfhf(logfilename, SFM_WRITE, SF_FORMAT_FLAC | SF_FORMAT_PCM_16, 8, D_FX);

	unique_ptr<int16_t[]> buf8(new int16_t[8*nSamplesPerFrame]);
#endif

	bool bSecondBuf = false;
	int kw_detected = 0;
	InitBoard();

	// Keyword detection
	printf("Keyword detection running...\n");
	while (!_kbhit())
	{
		bSecondBuf ^= 1;
		unsigned char** crnt_buf = &pucMicBuffer[8*bSecondBuf];	// current buffer
		short** crnt_sbuf = (short**)crnt_buf;

		micArr.ReadAll(crnt_buf);

#ifdef DD_WRITE_SOUND_FILE		
		for (int f = 0; f<nSamplesPerFrame; f++)
		{
			for (int b = 0; b < 8; b++)	// interleaving
				buf8[8*f+b] = crnt_sbuf[b][f];
		}

		sfhf.writef(buf8.get(), nSamplesPerFrame);
#endif

		//printf("%f\n", getRmsEnergy(160, (short*)crnt_buf[0]));

		for (int s = 0; s < nSamplesPerFrame; s += 160)
		{
			auto ret_dec = dnn_trigger.detect(160, &crnt_sbuf[0][s]);
			if (0 < ret_dec)
			{
				printf("Keyword detected at %d frame \n", dnn_trigger.getOutFrame());
				kw_detected++;
			}
		}
	}

	CloseBoard();
	nReturn = micArr.StopStreaming();

	printf("Keyword detection end.\n");

	return 0;
}


static double getRmsEnergy(const int len, const int16_t buf[])
{
	long long sum = 0;
	for (int i = 0; i < len; i++)
		sum += (long long)buf[i] * buf[i];

	long long mean = sum / len;

	return sqrt((double)mean);
}
