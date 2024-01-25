/////////////////////////////////////////////////////////
// Octopus Linux SDK
// Chasy Robotics Inc.
// 2011.08.08 - Alpha version
//

#ifndef OCTOPUS_HH_
#define OCTOPUS_HH_

#include <stdio.h>
#include <string.h>
#include <pthread.h>


#define OCTO_SAMPLE_8000HZ	( 8000 )
#define OCTO_SAMPLE_12000HZ	( 12000 )
#define OCTO_SAMPLE_16000HZ	( 16000 )
#define OCTO_SAMPLE_24000HZ	( 24000 )
#define OCTO_SAMPLE_32000HZ	( 32000 )
#define OCTO_SAMPLE_48000HZ	( 48000 )
#define OCTO_SAMPLE_64000HZ	( 64000 )
#define OCTO_SAMPLE_96000HZ	( 96000 )

#define OCTO_MAX_MIC_NUMBER		( 8 )
#define OCTO_MAX_CODEC_NUMBER	( 4 )

#define OCTO_FLAG_PREAMPBOOST_OFF		( 0 )
#define OCTO_FLAG_PREAMPBOOST_ON		( 1 )

#define OCTO_MIC0		( 0 )
#define OCTO_MIC1		( 1 )
#define OCTO_MIC2		( 2 )
#define OCTO_MIC3		( 3 )
#define OCTO_MIC4		( 4 )
#define OCTO_MIC5		( 5 )
#define OCTO_MIC6		( 6 )
#define OCTO_MIC7		( 7 )
#define OCTO_ALL_MICS	( 8 )

#define OCTO_MAX_DECIBEL				( 12.0 )
#define OCTO_MIN_DECIBEL				( -3.0 )

#define OCTO_READ_MIC0					0x01
#define OCTO_READ_MIC1					0x02
#define OCTO_READ_MIC2					0x04
#define OCTO_READ_MIC3					0x08
#define OCTO_READ_MIC4					0x10
#define OCTO_READ_MIC5					0x20
#define OCTO_READ_MIC6					0x40
#define OCTO_READ_MIC7					0x80


//err code
#define OCTO_OK								( 0 )
#define OCTO_ERR_DO_STREAMING				( 1 )
#define OCTO_ERR_OPEN_DEVICE				( 2 )
#define OCTO_ERR_NOT_STREAMING				( 3 )
#define OCTO_ERR_THREAD_START				( 4 )
#define OCTO_ERR_OPEN_MIC					( 5 )
#define OCTO_ERR_CREATE_THREAD				( 6 )
#define OCTO_ERR_READ						( 7 )
#define OCTO_ERR_RESET_EVENT				( 8 )
#define OCTO_ERR_WRITE						( 9 )
#define OCTO_ERR_CLOSE_MIC					( 10 )
#define OCTO_ERR_NULL_POINTER				( 11 )
#define OCTO_ERR_ALREADY_OPEN				( 12 )
#define OCTO_ERR_INVALID_MIC_INDEX			( 13 )
#define OCTO_ERR_NOT_OPEN					( 14 )
#define OCTO_ERR_NO_DEVICE_OR_FW			( 15 )
#define OCTO_ERR_INVALID_BOOST_VALUE		( 16 )
#define OCTO_ERR_INVALID_DECIBEL_VALUE		( 17 )

#undef TRUE
#define TRUE ( 1 )
#undef FALSE
#define FALSE ( 0 )

#ifdef __cplusplus
extern "C" {
#endif

int OctopusOpen( unsigned int uiSampleRate );
int OctopusClose( void );
int OctopusSetPGA(int nMicIndex, float fGainDecibel);
int OctopusGetPGA(int nMicIndex, float *pfGainDecibel);
int OctopusSetPreAmpBoost(int nMicIndex, int nSwitch);
int OctopusGetPreAmpBoost(int nMicIndex, int *pnSwitch);
int OctopusStartStreaming( void );
int OctopusStopStreaming( void );
int OctopusIsAttached( void );
unsigned int OctopusGetCurrentSampleRate( void );

int OctopusRead(int micIndex, unsigned char buffer[64]);
int OctopusReadSelective(int nSelectedMics, ...);
int OctopusReadAll(unsigned char buffer[8][64]);

#ifdef __cplusplus
};
#endif


#ifdef __cplusplus
/////////////////////////////////////////////////////////
// Octopus Linux SDK
// C++ wrapper
//
// 2015-04 mckeum@diotek.co.kr

const int octo_lnx2win[9] = { 0, 2, 4, 6, 1, 3, 5, 7, 8 };
const int octo_win2lnx[9] = { 0, 4, 1, 5, 2, 6, 3, 7, 8 };	// Linux, Windows 마이크 배열 통일

class COctopus
{
public:
	COctopus();
	bool Create(unsigned int uiSampleRate = OCTO_SAMPLE_16000HZ, int nSamplesPerFrame = 1600); 	// nSamplesPerFrame은 64의 배수이여야 한다.
	~COctopus();

	bool	IsAttached();
	int		StartStreaming();
	int		StopStreaming();
	int		Flush();
	int		Read(int nMicIndex, unsigned char* pucBuffer);
	int		ReadSeletive(int nSelectedMics, ...);
	int		ReadAll(unsigned char* pucBuffers[]);
	int		SetPreAmpBoost(int nMicIndex = OCTO_ALL_MICS, int nSwitch = OCTO_FLAG_PREAMPBOOST_ON);
	int		GetPreAmpBoost(int nMicIndex, int* pnSwitch);
	int		SetPGA(int nMicIndex = OCTO_ALL_MICS, float fDecibel = 0);
	int		GetPGA(int nMicIndex, float* pfDecibel);

	unsigned int	GetCurrentSampleRate(void);


private:
	int byte_per_frame;
	unsigned char** buf;
	unsigned char buf64[8][64];
	int buf_pos;
	int buf_fill;
	int buf_ea;

	pthread_t stream_t;
	pthread_attr_t stream_attr;
	pthread_cond_t stream_buf_filled;
	pthread_mutex_t stream_mutex;
	pthread_mutex_t stream_flag_mutex;
	static void* stream_runner(void* context);
	void stream_run();
	bool stream_stop;
};


COctopus::COctopus()
{
	byte_per_frame = 0;
	buf_pos = 0;
	buf_fill = 0;
	stream_t = 0;
	stream_stop = true;
	
	pthread_attr_init(&stream_attr);
	pthread_attr_setschedpolicy(&stream_attr, SCHED_FIFO);
	
	pthread_cond_init(&stream_buf_filled, NULL);
	pthread_mutex_init(&stream_mutex, NULL);
	pthread_mutex_init(&stream_flag_mutex, NULL);
}

bool COctopus::Create(unsigned int uiSampleRate, int nSamplesPerFrame)
{
	if (0 != nSamplesPerFrame % 64 || 0 == nSamplesPerFrame)
	{
		printf("ERROR: Buffer length should be multiple of 64!\n");
		return false;
	}

	buf_ea = uiSampleRate / nSamplesPerFrame / 2;	// Buffer length = 500ms
	if (buf_ea < 4)	buf_ea = 4;
	
	byte_per_frame = nSamplesPerFrame * sizeof(int16_t);

	// init buffer
	buf = new unsigned char*[buf_ea*8]();
	for (int i = 0; i < buf_ea*8; i++)
		buf[i] = new unsigned char[byte_per_frame]();

	int ret = OctopusOpen(uiSampleRate);
	if (OCTO_OK != ret)	return false;

	return true;
}


COctopus::~COctopus()
{
	OctopusStopStreaming();
	OctopusClose();
	
	pthread_mutex_destroy(&stream_mutex);
	pthread_mutex_destroy(&stream_flag_mutex);
	pthread_cond_destroy(&stream_buf_filled);

	for (int i = 0; i<buf_ea*8; i++)
		delete[] buf[i];
	delete buf;
}


int COctopus::StartStreaming()
{
	StopStreaming();

	int ret = OctopusStartStreaming();
	if (OCTO_OK != ret)	return false;

	stream_stop = false;
	ret = pthread_create(&stream_t, &stream_attr, &COctopus::stream_runner, (void*)this);
	if (0 != ret)	return false;
	
	//usleep(100);

	return true;
}


int COctopus::StopStreaming()
{
	int t_policy;
	stream_stop = true;
	pthread_attr_getdetachstate(&stream_attr, &t_policy);
	if (t_policy == PTHREAD_CREATE_JOINABLE)
	{
		int ret = pthread_join(stream_t, NULL);
		if (0 != ret)	return false;
	}

	int ret = OctopusStopStreaming();
	if (OCTO_OK != ret)	return false;
	
	return true;
}


int COctopus::Flush()
{
	pthread_mutex_lock(&stream_mutex);

	for (int i = 0; i < buf_ea*8; i++)
		memset(buf[i], 0, byte_per_frame);
		
	buf_pos = 0;
	buf_fill = 0;
	pthread_mutex_unlock(&stream_mutex);
	
	return true;
}


int COctopus::ReadAll(unsigned char* pucBuffers[])
{
	unsigned char** crnt_buf = &buf[8*buf_pos];	// current buffer

	if (buf_pos == buf_fill)
	{
		pthread_mutex_lock(&stream_flag_mutex);
		pthread_cond_wait(&stream_buf_filled, &stream_flag_mutex);
		pthread_mutex_unlock(&stream_flag_mutex);
	}

	for (int i = 0; i<8; i++)
	{
		unsigned char* crnt_buf_ch = crnt_buf[octo_win2lnx[i]];
		memcpy(pucBuffers[i], crnt_buf_ch, byte_per_frame);
	}
	
	buf_pos++;
	if (buf_ea <= buf_pos)	buf_pos = 0;
	return true;
}


void COctopus::stream_run()
{
	while (!stream_stop)
	{
		pthread_mutex_lock(&stream_mutex);

		unsigned char** crnt_buf = &buf[8*buf_fill];	// current buffer to fill

		for (int j = 0; j<byte_per_frame; j += 64)
		{
			OctopusReadAll(buf64);

			for (int i = 0; i<8; i++)
				memcpy(&crnt_buf[i][j], buf64[i], 64);
		}

		buf_fill++;
		if (buf_ea <= buf_fill)	buf_fill = 0;

		pthread_mutex_unlock(&stream_mutex);

		pthread_mutex_lock(&stream_flag_mutex);
		pthread_cond_signal(&stream_buf_filled);
		pthread_mutex_unlock(&stream_flag_mutex);
	}
}


bool COctopus::IsAttached() { return OctopusIsAttached(); }
int COctopus::SetPreAmpBoost(int nMicIndex, int nSwitch) { return OctopusSetPreAmpBoost(octo_win2lnx[nMicIndex], nSwitch); }
int COctopus::GetPreAmpBoost(int nMicIndex, int* pnSwitch) { return OctopusGetPreAmpBoost(octo_win2lnx[nMicIndex], pnSwitch); }
int COctopus::SetPGA(int nMicIndex, float fDecibel) { return OctopusSetPGA(octo_win2lnx[nMicIndex], fDecibel); }
int COctopus::GetPGA(int nMicIndex, float* pfDecibel) { return OctopusGetPGA(octo_win2lnx[nMicIndex], pfDecibel); }

void* COctopus::stream_runner(void* context) { ((COctopus*)context)->stream_run();	return NULL; }

#endif	//  __cplusplus

#endif /* OCTOPUS_HH_ */
