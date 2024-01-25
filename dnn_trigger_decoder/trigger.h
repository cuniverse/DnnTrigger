#ifndef TRIGGER_H
#define TRIGGER_H


#include <stdint.h>
#include <vector>

#include "feat_2pass.h"
#include "Selvy_Trigger_API.h"

#ifdef TRG_DLLEXPORT

#if defined(_WIN32)
#define POWER_DEEPNET_API __declspec(dllexport)
#elif __GNUC__ >= 4
#define POWER_DEEPNET_API __attribute__ ((visibility ("default")))
#else 
#define POWER_DEEPNET_API
#endif	// _WIN32, __GNUC__

#else  // !TRG_DLLEXPORT
#define POWER_DEEPNET_API
#endif 


class CFeat2pass;
class CDnnDecoder;


class POWER_DEEPNET_API CTrigger : public ITriggerAPI {
protected:
	CFeat2pass* feat_extractor;
	CDnnDecoder* dnn_decoder;
	float *dnn_prob_output;
	float feat_buf[40960 + 10];
	int output_frame;
	int sp_output_frame;
	int err;

public:
	int getError() { return err; }
	int getOutFrame() { return output_frame; }
    virtual int getOutSPFrame() { return sp_output_frame; }

	virtual int detect(const int len_sample, const int16_t pcm_buf[],int *p_st_frame_info=NULL) = 0;
	virtual bool reset() = 0;
	static bool setChannel(int ch) { return CFeat2pass::setChannel(ch); }
};


#endif