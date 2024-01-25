#ifndef _COCTOPUS_H_
#define _COCTOPUS_H_

#include <wtypes.h>

#define OCTO_MAX_MIC_NUMBER				8
#define OCTO_BYTES_PER_SAMPLE			2
#define OCTO_MIC0						0
#define OCTO_MIC1						1
#define OCTO_MIC2						2
#define OCTO_MIC3						3
#define OCTO_MIC4						4
#define OCTO_MIC5						5
#define OCTO_MIC6						6
#define OCTO_MIC7						7
#define OCTO_ALL_MICS					8
#define OCTO_FLAG_PREAMPBOOST_OFF		0
#define OCTO_FLAG_PREAMPBOOST_ON		1
#define OCTO_MAX_DECIBEL				12.0
#define OCTO_MIN_DECIBEL				-3.0
#define OCTO_READ_MIC0					0x01
#define OCTO_READ_MIC1					0x02
#define OCTO_READ_MIC2					0x04
#define OCTO_READ_MIC3					0x08
#define OCTO_READ_MIC4					0x10
#define OCTO_READ_MIC5					0x20
#define OCTO_READ_MIC6					0x40
#define OCTO_READ_MIC7					0x80

#define OCTO_SAMPLE_8000HZ				( 8000 )
#define OCTO_SAMPLE_12000HZ				( 12000 )
#define OCTO_SAMPLE_16000HZ				( 16000 )
#define OCTO_SAMPLE_24000HZ				( 24000 )
#define OCTO_SAMPLE_32000HZ				( 32000 )
#define OCTO_SAMPLE_48000HZ				( 48000 )
#define OCTO_SAMPLE_64000HZ				( 64000 )
#define OCTO_SAMPLE_96000HZ				( 96000 )

//err code
#define OCTO_OK							0
#define OCTO_ERR_DO_STREAMING			1
#define OCTO_ERR_OPEN_DEVICE			2
#define OCTO_ERR_NOT_STREAMING			3
#define OCTO_ERR_THREAD_START			4
#define OCTO_ERR_OPEN_MIC				5
#define OCTO_ERR_CREATE_THREAD			6
#define OCTO_ERR_READ					7
#define OCTO_ERR_RESET_EVENT			8
#define OCTO_ERR_WRITE					9
#define OCTO_ERR_CLOSE_MIC				10
#define OCTO_ERR_NULL_POINTER			11
#define OCTO_ERR_ALREADY_OPEN			12
#define OCTO_ERR_INVALID_MIC_INDEX		13
#define OCTO_ERR_NOT_OPEN				14
#define OCTO_ERR_NO_DEVICE_OR_FW		15
#define OCTO_ERR_INVALID_BOOST_VALUE	16
#define OCTO_ERR_INVALID_DECIBEL_VALUE	17


class __declspec(dllexport) COctopus
{
public:
	COctopus();
	BOOL Create(UINT uiSampleRate = OCTO_SAMPLE_16000HZ, INT nSamplesPerFrame = 1600); 	// nSamplesPerFrame은 64의 배수이여야 한다.
	~COctopus();

	BOOL	IsAttached();
	INT		StartStreaming();
	INT		StopStreaming();
	INT		Flush();
	INT		Read(INT nMicIndex, PUCHAR pucBuffer);
	INT		ReadSeletive(INT nSelectedMics, ...);
	INT		ReadAll(PUCHAR pucBuffers[]);
	INT		SetPreAmpBoost(INT nMicIndex = OCTO_ALL_MICS, INT nSwitch = OCTO_FLAG_PREAMPBOOST_ON);
	INT		GetPreAmpBoost(INT nMicIndex, INT* pnSwitch);
	INT		SetPGA(INT nMicIndex = OCTO_ALL_MICS, DOUBLE fDecibel = 0);
	INT		GetPGA(INT nMicIndex, DOUBLE* pfDecibel);
	VOID	GetCodecInfo(INT nCodecIndex, BYTE ucRegAddr, BYTE *pucValue);
	VOID	SetCodecInfo(INT nCodecIndex, BYTE ucRegAddr, BYTE ucValue);
	
	UINT	GetCurrentSampleRate(void);


private:
	LPVOID	m_pHandle;
};

#endif