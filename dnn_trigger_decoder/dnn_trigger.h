// dnn_trigger.h
// Voice trigger(word detector) using DNN
// 2016-09 by mckeum, ywjung


#ifndef __TRIGGER_DNN_TRIGGER_H__
#define __TRIGGER_DNN_TRIGGER_H__

#include "trigger.h"


class CDetectorWord;
class SizedQueue;


class POWER_DEEPNET_API CDnnTrigger : public CTrigger
{
private:

	CDetectorWord* detector;
	SizedQueue* pcm_stream;

public:
	CDnnTrigger(const char root_path[], const char config_path[]);
	~CDnnTrigger();
	virtual bool reset();
	virtual int detect(const int len_sample, const int16_t pcm_buf[],int* spinfo=NULL);
};

#endif	// __TRIGGER_DNN_TRIGGER_H__
