// feat_2pass.cpp
// Feature(MFCC 51-dim) extractor using 2-pass engine
// 2016-11 mckeum

#include "feat_2pass.h"

#include <stdio.h>
#include <stdint.h>

#include "frontend/powerdsr_frontend.h"
#pragma comment(lib, "Feat2Pass")
#pragma comment(lib, "FrontEnd")


bool CFeat2pass::fe_connected = false;
int CFeat2pass::fe_channel = 16;

CFeat2pass::CFeat2pass(const char root_path[])
{
	chan_id = -1;
	err = 0;

	if (!fe_connected)
	{
		auto ret_fec = PowerDSR_FE_Connect(root_path, "../conf/hci_frontend.ini", fe_channel);
		if (POWERDSR_FE_CONNECTED != ret_fec)
		{
			printf("fe connect fail");
			err = 100 + ret_fec;
			return;
		}
		fe_connected = true;
	}

	chan_id = PowerDSR_FE_OpenChannel("unknown", true);
	if (chan_id < 0)
	{
		printf("fe open channel fail");
		err = 2;
		return;
	}
}


CFeat2pass::~CFeat2pass()
{
	PowerDSR_FE_ReleaseFrontEndEngine(chan_id);
	PowerDSR_FE_CloseChannel(chan_id);
	//PowerDSR_FE_Disconnect();
}

// reset and clear buffer
int CFeat2pass::reset()
{
	PowerDSR_FE_ReleaseFrontEndEngine(chan_id);
	PowerDSR_FE_CloseChannel(chan_id);

	chan_id = PowerDSR_FE_OpenChannel("unknown", true);
	if (chan_id < 0)
	{
		printf("fe open channel fail");
		err = 2;
		return -1;
	}

	return 0;
}


int CFeat2pass::getFeature(const int in_samples, const int16_t in_pcm[], long* len_feat, float* out_feat)
{
	auto epd_result = PowerDSR_FE_SpeechStream2FeatureStream(chan_id,
		out_feat, (LONG*)len_feat, (short*)in_pcm, in_samples, 80, 0);

	if (epd_result < 0)	return epd_result;

	return 0;
}
