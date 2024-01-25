#ifndef Selvy_Trigger_API_H
#define Selvy_Trigger_API_H
/*
 * Copyright (C) 2017. SELVAS AI. Speech Dev. Dept.
 * All rights reserved.
 * 2017.11   Cuniverse <aaron.d.moon@selvas.com>
 */

#include <stdint.h>

#if defined(_WIN32)
#define EXPORT_SDK_API __declspec(dllexport)
#elif __GNUC__ >= 4
#define EXPORT_SDK_API __attribute__ ((visibility ("default")))
#else
#define EXPORT_SDK_API
#endif	// _WIN32, __GNUC__

class EXPORT_SDK_API ITriggerAPI {

public:
    virtual ~ITriggerAPI(){};
	virtual int detect(const int len_sample, const int16_t pcm_buf[],int *p_trigger_frame_info=NULL) = 0;
	virtual bool reset() = 0;
};

class EXPORT_SDK_API Selvy_DNN_Trigger : public ITriggerAPI{
    int err;
public:
    Selvy_DNN_Trigger(const char root_path[], const char config_path[],const char *license_string=NULL);
    virtual ~Selvy_DNN_Trigger();
    virtual int detect(const int len_sample, const int16_t *pcm_buf,int *p_trigger_frame_info=NULL);
    virtual bool reset();
    int getError() { return err; }

private:
    ITriggerAPI* __impl__;
};


#endif //Selvy_Trigger_API_H