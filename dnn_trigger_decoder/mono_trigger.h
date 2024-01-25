// mono_trigger.h
// Voice trigger(word detector) using DNN
// 2016-09 by mckeum, ywjung


#ifndef __TRIGGER_MONO_TRIGGER_H__
#define __TRIGGER_MONO_TRIGGER_H__

#include "trigger.h"

#include <string>


class CDetectorMono;
class SizedQueue;


class POWER_DEEPNET_API CMonoTrigger : public CTrigger
{
private:
	CDetectorMono* detector;
	SizedQueue* pcm_stream;

public:
	CMonoTrigger(const char root_path[], const char config_path[]);
	~CMonoTrigger();
	
	virtual bool reset();
	virtual int detect(const int len_sample, const int16_t pcm_buf[],int* spinfo=NULL);

	int addPhonSeq(const char sequence[]);
	int addPhonSeq(const std::string sequence);
};

#endif	// __TRIGGER_MONO_TRIGGER_H__
